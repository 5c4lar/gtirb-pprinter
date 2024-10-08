#
# Cmake Configuration
#

# Need 3.12.1 to support FindPython3 and the correct cpack semantics for version
# numbers.
cmake_minimum_required(VERSION 3.12.1)

project(gtirb_pprinter)
set(PACKAGE_BRANCH master)

# get version information
file(READ "${CMAKE_CURRENT_SOURCE_DIR}/version.txt" ver)

string(REGEX MATCH "VERSION_MAJOR ([0-9]*)" _ ${ver})
set(GTIRB_PPRINTER_MAJOR_VERSION ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_MINOR ([0-9]*)" _ ${ver})
set(GTIRB_PPRINTER_MINOR_VERSION ${CMAKE_MATCH_1})

string(REGEX MATCH "VERSION_PATCH ([0-9]*)" _ ${ver})
set(GTIRB_PPRINTER_PATCH_VERSION ${CMAKE_MATCH_1})

set(GTIRB_PPRINTER_VERSION
    "${GTIRB_PPRINTER_MAJOR_VERSION}.${GTIRB_PPRINTER_MINOR_VERSION}.${GTIRB_PPRINTER_PATCH_VERSION}"
    CACHE STRING "version of gtirb-pprinter" FORCE)

if(NOT DEFINED GTIRB_PPRINTER_BUILD_REVISION)
  execute_process(
    COMMAND git log --pretty=format:%h -n 1
    WORKING_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}
    OUTPUT_VARIABLE GTIRB_PPRINTER_BUILD_REVISION
    ERROR_QUIET)
endif()
if(GTIRB_PPRINTER_BUILD_REVISION STREQUAL "")
  set(GTIRB_PPRINTER_BUILD_REVISION "UNKNOWN")
endif()
option(GTIRB_PPRINTER_RELEASE_VERSION
       "Whether or not to build Python package versions without dev suffixes."
       OFF)

string(TIMESTAMP GTIRB_PPRINTER_BUILD_DATE "%Y-%m-%d")

# includes
include(CheckFunctionExists)
include(CheckCXXSourceCompiles)
include(CheckIncludeFile)

option(ENABLE_CONAN "Use Conan to inject dependencies" OFF)
if(ENABLE_CONAN)
  set(CONAN_SYSTEM_INCLUDES ON)
  include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
  conan_basic_setup()
endif()

# ---------------------------------------------------------------------------
# Global settings
# ---------------------------------------------------------------------------

option(GTIRB_PPRINTER_ENABLE_TESTS "Enable building and running tests." ON)

# The libraries can be static while the drivers can link in other things in a
# shared manner. This option allows for this possibility.
option(
  GTIRB_PPRINTER_STATIC_DRIVERS
  "Attempt to make any driver executables as statically-linked as possible.
Implies GTIRB_PPRINTER_BUILD_SHARED_LIBS=OFF."
  OFF)

if(GTIRB_PPRINTER_STATIC_DRIVERS)
  set(Boost_USE_STATIC_LIBS ON)

  if(WIN32)
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".lib")
  else()
    set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
  endif()
endif()

# This just sets the builtin BUILD_SHARED_LIBS, but if defaults to ON instead of
# OFF.
option(GTIRB_PPRINTER_BUILD_SHARED_LIBS "Build shared libraries." ON)

if(GTIRB_PPRINTER_STATIC_DRIVERS OR NOT GTIRB_PPRINTER_BUILD_SHARED_LIBS)
  set(BUILD_SHARED_LIBS OFF)
else()
  set(BUILD_SHARED_LIBS ON)
endif()

option(GTIRB_PPRINTER_CODE_COVERAGE
       "Build with instrumentation for collecting code coverage" OFF)

if(GTIRB_PPRINTER_CODE_COVERAGE)
  if(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU OR ${CMAKE_CXX_COMPILER_ID} STREQUAL
                                              Clang)
    add_compile_options(--coverage)
    # Unfortunately using link_libraries for this triggers a warning about
    # CMP0022 not being set inside of googletest. add_link_options does not
    # trigger this warning, but is only available in CMake 3.13+.
    if(${CMAKE_VERSION} GREATER_EQUAL "3.13")
      add_link_options(--coverage)
    else()
      link_libraries(--coverage)
    endif()
  else()
    message(FATAL_ERROR "no support for code coverage on this target")
  endif()
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/src)
if(WIN32)
  set(CMAKE_DEBUG_POSTFIX
      "d"
      CACHE STRING "add a postfix, usually d on windows")
endif()
set(CMAKE_RELEASE_POSTFIX
    ""
    CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_RELWITHDEBINFO_POSTFIX
    ""
    CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_MINSIZEREL_POSTFIX
    ""
    CACHE STRING "add a postfix, usually empty on windows")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Determine whether or not to strip debug symbols and set the build-id. This is
# only really needed when we are building ubuntu *-dbg packages
option(GTIRB_PPRINTER_STRIP_DEBUG_SYMBOLS
       "Whether or not to strip debug symbols and set the build-id." OFF)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
# Error if it's not available
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Specifically check for gcc-7 or later. gcc-5 is installed on many systems and
# will accept -std=c++17, but does not fully support the standard.
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  if(CMAKE_CXX_COMPILER_VERSION VERSION_LESS "7.0.0")
    message(FATAL_ERROR "gcc 7 or later is required to build gtirb")
  endif()
endif()

# If we're using libc++, we need to manually include libc++abi (unlike with
# using libstdc++, which automatically does this)
include(CheckCXXSourceCompiles)

check_cxx_source_compiles(
  "
  #include <ciso646>
  int main() {
    return _LIBCPP_VERSION;
  }
"
  USING_LIBCPP)

if(USING_LIBCPP)
  if(BUILD_SHARED_LIBS)
    find_library(LIBCPP_ABI NAMES c++abi)
  else()
    find_library(LIBCPP_ABI NAMES libc++abi.a)
  endif()

  if(NOT LIBCPP_ABI)
    message(FATAL_ERROR "libc++abi not found")
  endif()
endif()

# Required warning suppression (TODO: Remove!)
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  # add_compile_options(-wd4251)  # Non-exportable template classes.
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
  add_compile_options(-mtune=generic)
  add_compile_options(-pthread)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  add_compile_options(-mtune=generic)
  add_compile_options(-pthread)
endif()

# If Unix, but not Cygwin: Find system libs we need to link with.
if(UNIX)
  if(NOT WIN32)
    set(SYSLIBS ${SYSLIBS} dl pthread)
  endif()
else()
  set(SYSLIBS)
endif()

# global MSVC options
if(${CMAKE_CXX_COMPILER_ID} STREQUAL MSVC)
  add_compile_options(-D_CRT_SECURE_NO_WARNINGS)
  add_compile_options(-D_MBCS)
  add_compile_options(-D_SCL_SECURE_NO_WARNINGS)
  add_compile_options(-D_WIN32)
  add_compile_options(-D_WINDOWS)
  add_compile_options(-DMBCS)
  add_compile_options(-DNOMINMAX)
  add_compile_options(-EHsc)
  add_compile_options(-GR)
  add_compile_options(-MP)
  add_compile_options(-nologo)
  add_compile_options(-W4)
  add_compile_options(-WX)

  add_compile_options(
    -wd4267) # Conversion warning.  Appears in gtirb's proto headers, but only
             # when building with conan. Unfortunately, this is the only way to
             # get it to go away it seems.
  add_compile_options(
    -wd4251) # 'identifier' : class 'type' needs to have dll- interface to be
             # used by clients of class 'type2'
  add_compile_options(-wd4275) # Non-dll interface base classes.
  add_compile_options(-wd4996) # VC8: Deprecated libc functions.
  add_compile_options(
    -wd4351) # This is a warning about a change in behavior from old versions of
             # visual c++.  We want the new (standard-compliant) behavior, so we
             # don't want the warning.  The warning is that using an array in a
             # class initializer list will cause its elements to be default
             # initialized.

  # Per-configuration options
  add_compile_options($<$<CONFIG:Debug>:-D_DEBUG>)
  add_compile_options($<$<CONFIG:Debug>:-DDEBUG>)
  add_compile_options($<$<CONFIG:Debug>:-MDd>)
  add_compile_options($<$<CONFIG:Debug>:-Ob0>) # Disables inline expansion
  add_compile_options(
    $<$<CONFIG:Debug>:-Od>) # Disables optimization, speeding compilation and
                            # simplifying debugging.
                            # https://msdn.microsoft.com/en-
                            # us/library/k1ack8f1.aspx
  add_compile_options($<$<CONFIG:Debug>:-RTC1>) # Enables run-time error
                                                # checking.
  add_compile_options($<$<CONFIG:Debug>:-W4>) # Sets warning level.
  add_compile_options($<$<CONFIG:Debug>:-Zi>) # Generates complete debugging
                                              # information.
  add_compile_options($<$<CONFIG:Debug>:-bigobj>) # enables big obj compilation
                                                  # option

  add_compile_options($<$<CONFIG:RelWithDebInfo>:-D_NDEBUG>)
  add_compile_options($<$<CONFIG:RelWithDebInfo>:-DNDEBUG>)
  add_compile_options($<$<CONFIG:RelWithDebInfo>:-MD>)
  add_compile_options($<$<CONFIG:RelWithDebInfo>:-O2>) # Creates fast code.
  add_compile_options(
    $<$<CONFIG:RelWithDebInfo>:-Ob2>) # The default value. Allows expansion of
                                      # functions marked as inline, __inline, or
                                      # __forceinline, and any other function
                                      # that the compiler chooses.
  add_compile_options($<$<CONFIG:RelWithDebInfo>:-Oi>) # Generates intrinsic
                                                       # functions.
  add_compile_options($<$<CONFIG:RelWithDebInfo>:-Ot>) # Favors fast code.
  add_compile_options($<$<CONFIG:RelWithDebInfo>:-W4>) # Sets warning level.
  add_compile_options($<$<CONFIG:RelWithDebInfo>:-Zi>) # Generates complete
                                                       # debugging information.
  add_compile_options($<$<CONFIG:RelWithDebInfo>:-bigobj>) # enables big obj
                                                           # compilation option

  add_compile_options($<$<CONFIG:Release>:-D_NDEBUG>)
  add_compile_options($<$<CONFIG:Release>:-DNDEBUG>)
  add_compile_options($<$<CONFIG:Release>:-MD>)
  add_compile_options($<$<CONFIG:Release>:-O2>) # Creates fast code.
  add_compile_options(
    $<$<CONFIG:Release>:-Ob2>) # The default value. Allows expansion of
                               # functions marked as inline, __inline, or
                               # __forceinline, and any other function that the
                               # compiler chooses.
  add_compile_options($<$<CONFIG:Release>:-Oi>) # Generates intrinsic functions.
  add_compile_options($<$<CONFIG:Release>:-Ot>) # Favors fast code.
  add_compile_options($<$<CONFIG:Release>:-W4>) # Sets warning level.
  add_compile_options($<$<CONFIG:Release>:-bigobj>) # enables big obj
                                                    # compilation option

  # Shove in some linker flags to support using 64-bit memory. 4099 -PDB
  # 'filename' was not found with 'object/library' or at 'path'; linking object
  # as if no debug info
  set(CMAKE_SHARED_LINKER_FLAGS
      "${CMAKE_SHARED_LINKER_FLAGS} /LARGEADDRESSAWARE")

elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL GNU)
  add_compile_options(-Wall -Wextra -Wpointer-arith -Wshadow)
  add_compile_options(-Werror)
  add_compile_options(-fPIC)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL Clang)
  add_compile_options(-Wall -Wextra -Wpointer-arith -Wshadow)
  add_compile_options(-Werror)
  add_compile_options(-fPIC)
endif()

# ---------------------------------------------------------------------------
# gtirb
# ---------------------------------------------------------------------------
find_package(gtirb 2.2.0 REQUIRED)

add_definitions(-DGTIRB_WRAP_UTILS_IN_NAMESPACE)

# ---------------------------------------------------------------------------
# Boost
# ---------------------------------------------------------------------------
#
# Note: we would like to use std::filsystem. However, currently, std::filesystem
# is not provided in clang 6 or gcc 7, both of which are the default installed
# versions for Ubuntu 18. Instead in that context, one can use the
# "experimental" version fo filesystem. But we've decided that it's simpler to
# just use boost::filesystem instead until we (eventually) drop support for
# Ubuntu 18.
set(BOOST_COMPONENTS filesystem program_options system)
find_package(Boost 1.67 REQUIRED COMPONENTS ${BOOST_COMPONENTS})

add_compile_options(-DBOOST_CONFIG_SUPPRESS_OUTDATED_MESSAGE)
add_compile_options(-DBOOST_SYSTEM_NO_DEPRECATED)

# Boost versions 1.70.0+ may use Boost's provided CMake support rather than
# CMake's internal Boost support. The former uses "Boost::boost" and so on,
# while the latter uses "Boost_BOOST" and so on. This normalizes the two cases
# to use Boost_INCLUDE_DIRS and Boost_LIBRARIES.

if(TARGET Boost::headers)
  get_target_property(Boost_INCLUDE_DIRS Boost::headers
                      INTERFACE_INCLUDE_DIRECTORIES)
  foreach(BOOST_COMPONENT ${BOOST_COMPONENTS})
    list(APPEND Boost_LIBRARIES Boost::${BOOST_COMPONENT})
  endforeach()
endif()

include_directories(SYSTEM ${Boost_INCLUDE_DIRS})

# ---------------------------------------------------------------------------
# capstone
# ---------------------------------------------------------------------------
if(BUILD_SHARED_LIBS)
  find_library(CAPSTONE NAMES capstone)
else()
  find_library(CAPSTONE NAMES libcapstone.a)
endif()

if(CAPSTONE)
  get_filename_component(CAPSTONE_DIR ${CAPSTONE} DIRECTORY)
  find_program(CSTOOL "cstool" HINTS ${CAPSTONE_DIR} ${CAPSTONE_DIR}/bin
                                     ${CAPSTONE_DIR}/../bin)
  # When using conan to install the dependencies but CMake to do the build, the
  # capstone libraries may not be findable by the loader. Set LD_LIBRARY path to
  # fix this.
  execute_process(
    COMMAND ${CMAKE_COMMAND} -E env
            "LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH}:${CAPSTONE_DIR}" #
            "${CSTOOL}" "-v" OUTPUT_VARIABLE CSTOOL_VERSION)
  string(REGEX MATCH "v([0-9]+\.[0-9]+\.[0-9]+)" CSTOOL_VERSION
               "${CSTOOL_VERSION}")
  set(CSTOOL_VERSION "${CMAKE_MATCH_1}")
endif()

if(NOT CAPSTONE OR "${CSTOOL_VERSION}" VERSION_LESS "5.0.1")
  message(
    FATAL_ERROR
      " No Capstone installation found.\n"
      " - If Capstone is not installed, install it from souce.\n"
      "   You can get the latest version of Capstone at:\n"
      "       http://www.capstone-engine.org/\n"
      " - If Capstone is installed, make sure the installation location is in your PATH,\n"
      "   and it is at least version 5.0.1.\n")
else()
  # CMake's IF EXISTS functionality requires a full path rather than a relative
  # path, so get the full path in case we need it.
  get_filename_component(CAPSTONE_INCLUDE_DIR1 ${CAPSTONE_DIR}/include ABSOLUTE)
  get_filename_component(CAPSTONE_INCLUDE_DIR2 ${CAPSTONE_DIR}/../include
                         ABSOLUTE)
  if(EXISTS ${CAPSTONE_INCLUDE_DIR1})
    # cmake-format: off
    # root/
    #   include/
    #   library.a
    # cmake-format: on
    include_directories(SYSTEM ${CAPSTONE_INCLUDE_DIR1})
  elseif(EXISTS ${CAPSTONE_INCLUDE_DIR2})
    # cmake-format: off
    # root/
    #   include/
    #   lib/
    #     library.a
    # cmake-format: on
    include_directories(SYSTEM ${CAPSTONE_INCLUDE_DIR2})
  endif()
endif()

# ---------------------------------------------------------------------------
# Google Test
# ---------------------------------------------------------------------------

if(GTIRB_PPRINTER_ENABLE_TESTS)
  enable_testing()

  # Pull in Google Test
  # https://github.com/google/googletest/blob/master/googletest#incorporating-
  # into-an-existing-cmake-project

  # Download and unpack googletest at configure time
  configure_file(CMakeLists.googletest googletest-download/CMakeLists.txt)

  execute_process(
    COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)

  if(result)
    message(FATAL_ERROR "CMake step for googletest failed: ${result}")
  endif()

  execute_process(
    COMMAND ${CMAKE_COMMAND} --build .
    RESULT_VARIABLE result
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/googletest-download)

  if(result)
    message(FATAL_ERROR "Build step for googletest failed: ${result}")
  endif()

  # Prevent overriding the parent project's compiler/linker settings on Windows
  set(gtest_force_shared_crt
      ON
      CACHE BOOL "" FORCE)

  # Add googletest directly to our build. This defines the gtest and gtest_main
  # targets.
  add_subdirectory(${CMAKE_BINARY_DIR}/googletest-src
                   ${CMAKE_BINARY_DIR}/googletest-build EXCLUDE_FROM_ALL)
endif()

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------
function(install_linux_debug_info TARGET COMPONENT_NAME)
  if(UNIX
     AND NOT CYGWIN
     AND ("${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo"
          OR "${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
     AND ${GTIRB_PPRINTER_STRIP_DEBUG_SYMBOLS})
    string(
      RANDOM
      LENGTH 32
      ALPHABET "abcdef0123456789" BUILD_ID)
    string(SUBSTRING "${BUILD_ID}" 0 2 BUILD_ID_PREFIX)
    string(SUBSTRING "${BUILD_ID}" 2 32 BUILD_ID_SUFFIX)
    target_link_libraries(${TARGET} PRIVATE "-Wl,--build-id=0x${BUILD_ID}")
    add_custom_command(
      TARGET ${TARGET}
      POST_BUILD
      COMMAND objcopy --only-keep-debug $<TARGET_FILE:${TARGET}>
              ${CMAKE_BINARY_DIR}/bin/${BUILD_ID_SUFFIX}.debug
      COMMAND objcopy --strip-debug $<TARGET_FILE:${TARGET}>)
    install(
      FILES "${CMAKE_BINARY_DIR}/bin/${BUILD_ID_SUFFIX}.debug"
      COMPONENT "${COMPONENT_NAME}"
      DESTINATION "lib/debug/.build-id/${BUILD_ID_PREFIX}")
  endif()
endfunction()

add_subdirectory(src)

# ---------------------------------------------------------------------------
# Python wheel
# ---------------------------------------------------------------------------
option(GTIRB_PPRINTER_RELEASE_VERSION "Build Python gtirb-pprinter package."
       OFF)
if(GTIRB_PPRINTER_BUILD_PYTHON_PACKAGE)
  add_subdirectory(python)
endif()

# ---------------------------------------------------------------------------
# Export config for use by other CMake projects
# ---------------------------------------------------------------------------
export(TARGETS gtirb_pprinter gtirb_layout
       FILE "${CMAKE_CURRENT_BINARY_DIR}/gtirb_pprinterTargets.cmake")
configure_file("${CMAKE_CURRENT_LIST_DIR}/gtirb_pprinterConfig.cmake.in"
               "${CMAKE_CURRENT_BINARY_DIR}/gtirb_pprinterConfig.cmake" @ONLY)
file(
  APPEND "${CMAKE_CURRENT_BINARY_DIR}/gtirb_pprinterConfig.cmake"
  "
            set_property(
                TARGET gtirb_pprinter
                APPEND PROPERTY
                    INTERFACE_INCLUDE_DIRECTORIES \"${CMAKE_BINARY_DIR}/include\"
            )
        ")
export(PACKAGE gtirb_pprinter)

# --- For the installed copy ---
# Main config file for find_package, just includes the targets file.
configure_file(
  "${CMAKE_CURRENT_LIST_DIR}/gtirb_pprinterConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/export/gtirb_pprinterConfig.cmake" @ONLY)
# In this mode, find_package also seems to require a version file
set(version_file
    "${CMAKE_CURRENT_BINARY_DIR}/gtirb_pprinterConfig-version.cmake")
include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${version_file}
  VERSION ${GTIRB_PPRINTER_VERSION}
  COMPATIBILITY AnyNewerVersion)

# Copy the config files to the install location
install(
  FILES ${CMAKE_CURRENT_BINARY_DIR}/export/gtirb_pprinterConfig.cmake
        ${version_file}
  DESTINATION lib/gtirb_pprinter
  COMPONENT cmake_config)

install(
  EXPORT gtirb_pprinterTargets
  DESTINATION lib/gtirb_pprinter
  COMPONENT cmake_target)

# ---------------------------------------------------------------------------
# Tests
# ---------------------------------------------------------------------------

if(GTIRB_PPRINTER_ENABLE_TESTS)
  find_package(Python3 REQUIRED COMPONENTS Interpreter)

  add_test(
    NAME python_tests
    COMMAND Python3::Interpreter -m unittest discover tests "*_test.py"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/")
  set_tests_properties(
    python_tests PROPERTIES ENVIRONMENT
                            "PPRINTER_PATH=$<TARGET_FILE:gtirb-pprinter>")
endif()

# ---------------------------------------------------------------------------
# Package policy enforcement
# ---------------------------------------------------------------------------

if(GTIRB_PPRINTER_PACKAGE_POLICY)
  set(PACKAGE_POLICY ${GTIRB_PPRINTER_PACKAGE_POLICY})
elseif(ENABLE_CONAN OR WIN32)
  set(PACKAGE_POLICY conan)
else()
  set(PACKAGE_POLICY unix)
endif()

if(PACKAGE_POLICY STREQUAL "unix")

  # Provides copyright file for Unix packages.
  install(
    FILES ${CMAKE_SOURCE_DIR}/LICENSE.txt
    COMPONENT license
    DESTINATION share/doc/gtirb-pprinter
    RENAME copyright)

elseif(PACKAGE_POLICY STREQUAL "conan")

  # Provides LICENSE.txt for Conan packages
  install(
    FILES ${CMAKE_SOURCE_DIR}/LICENSE.txt
    COMPONENT license
    DESTINATION licenses)

endif()
# ---------------------------------------------------------------------------
# Package generation with cpack
# ---------------------------------------------------------------------------
set(CPACK_PROJECT_CONFIG_FILE ${CMAKE_CURRENT_SOURCE_DIR}/cpack-config.cmake)

set(CMAKE_PROJECT_HOMEPAGE_URL https://github.com/GrammaTech/gtirb-pprinter)
set(CPACK_PACKAGE_VERSION_MAJOR ${GTIRB_PPRINTER_MAJOR_VERSION})
set(CPACK_PACKAGE_VERSION_MINOR ${GTIRB_PPRINTER_MINOR_VERSION})
set(CPACK_PACKAGE_VERSION_PATCH ${GTIRB_PPRINTER_PATCH_VERSION})
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "A pretty printer from the GTIRB intermediate representation for binary analysis and reverse engineering to gas-syntax assembly code."
)
set(CPACK_PACKAGE_VEDOR "GrammaTech Inc.")
set(CPACK_PACKAGE_CONTACT gtirb@grammatech.com)
set(CPACK_PACKAGE_DESCRIPTION_FILE ${CMAKE_CURRENT_SOURCE_DIR}/README.md)
set(CPACK_PACKAGE_RESOURCE_FILE_LICENSE ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt)

set(CPACK_DEBIAN_PACKAGE_SECTION devel)

set(CPACK_GTIRB_VERSION "${gtirb_VERSION}")
set(CPACK_GTIRB_PPRINTER_VERSION "${GTIRB_PPRINTER_VERSION}")

include(CPack)
