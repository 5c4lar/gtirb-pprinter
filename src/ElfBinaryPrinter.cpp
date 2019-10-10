//===- ElfBinaryPrinter.cpp -------------------------------------*- C++ -*-===//
//
//  Copyright (C) 2018 GrammaTech, Inc.
//
//  This code is licensed under the MIT license. See the LICENSE file in the
//  project root for license terms.
//
//  This project is sponsored by the Office of Naval Research, One Liberty
//  Center, 875 N. Randolph Street, Arlington, VA 22203 under contract #
//  N68335-17-C-0700.  The content of the information does not necessarily
//  reflect the position or policy of the Government and no official
//  endorsement should be inferred.
//
//===----------------------------------------------------------------------===//
#include "ElfBinaryPrinter.hpp"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wshadow"
#pragma GCC diagnostic ignored "-Wc++11-compat"
#pragma GCC diagnostic ignored "-Wpessimizing-move"
#pragma GCC diagnostic ignored "-Wdeprecated-copy"
#endif // __GNUC__
#include <boost/process/search_path.hpp>
#include <boost/process/system.hpp>
#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif // __GNUC__
#include <experimental/filesystem>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

#ifdef _WIN32
#include <fcntl.h>
#include <io.h>
#include <sys/stat.h>

/* From libgw32 misc/mkstemps.c */

/* Generate a unique temporary file name from TEMPLATE.

   TEMPLATE has the form:

   <path>/ccXXXXXX<suffix>

   SUFFIX_LEN tells us how long <suffix> is (it can be zero length).

   The last six characters of TEMPLATE before <suffix> must be "XXXXXX";
   they are replaced with a string that makes the filename unique.

   Returns a file descriptor open on the file for reading and writing.  */
int mkstemps(char* template_, int suffix_len) {
  static const char letters[] =
      "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
  static uint64_t value;
#ifdef HAVE_GETTIMEOFDAY
  struct timeval tv;
#endif
  char* XXXXXX;
  size_t len;
  int count;

  len = strlen(template_);

  if ((int)len < 6 + suffix_len ||
      strncmp(&template_[len - 6 - suffix_len], "XXXXXX", 6)) {
    return -1;
  }

  XXXXXX = &template_[len - 6 - suffix_len];

#ifdef HAVE_GETTIMEOFDAY
  /* Get some more or less random data.  */
  gettimeofday(&tv, NULL);
  value += ((uint64_t)tv.tv_usec << 16) ^ tv.tv_sec ^ getpid();
#else
  value += getpid();
#endif

  for (count = 0; count < TMP_MAX; ++count) {
    uint64_t v = value;
    int fd;

    /* Fill in the random bits.  */
    XXXXXX[0] = letters[v % 62];
    v /= 62;
    XXXXXX[1] = letters[v % 62];
    v /= 62;
    XXXXXX[2] = letters[v % 62];
    v /= 62;
    XXXXXX[3] = letters[v % 62];
    v /= 62;
    XXXXXX[4] = letters[v % 62];
    v /= 62;
    XXXXXX[5] = letters[v % 62];

    fd = open(template_, O_RDWR | O_CREAT | O_EXCL | O_TEMPORARY,
              _S_IREAD | _S_IWRITE);
    if (fd >= 0)
      /* The file does not exist.  */
      return fd;

    /* This is a random value.  It is only necessary that the next
       TMP_MAX values generated by adding 7777 to VALUE are different
       with (module 2^32).  */
    value += 7777;
  }

  /* We return the null string if we can't find a unique file name.  */
  template_[0] = '\0';
  return -1;
}
#endif // _WIN32

namespace bp = boost::process;
namespace fs = std::experimental::filesystem;

namespace gtirb_bprint {

std::optional<std::string>
ElfBinaryPrinter::getInfixLibraryName(const std::string& library) const {
  std::regex libsoRegex("^lib(.*)\\.so.*");
  std::smatch m;
  if (std::regex_match(library, m, libsoRegex)) {
    return m.str(1);
  }
  return std::nullopt;
}

std::optional<std::string>
ElfBinaryPrinter::findLibrary(const std::string& library,
                              const std::vector<std::string>& paths) const {
  for (const auto& path : paths) {
    fs::path filePath(path);
    filePath.append(library);
    // check that if filePath is a symbolic link, it eventually leads to a
    // regular file.
    fs::path resolvedFilePath(filePath);
    while (fs::is_symlink(resolvedFilePath)) {
      resolvedFilePath = fs::read_symlink(resolvedFilePath);
    }
    if (fs::is_regular_file(resolvedFilePath)) {
      return filePath.string();
    }
  }
  return std::nullopt;
}

std::vector<std::string> ElfBinaryPrinter::buildCompilerArgs(
    std::string outputFilename, std::string asmPath,
    const std::vector<std::string>& userLibraryPaths, gtirb::IR& ir) const {
  std::vector<std::string> args;
  // Start constructing the compile arguments, of the form
  // -o <output_filename> fileAXADA.s
  args.insert(args.end(), {"-o", outputFilename, std::string(asmPath)});

  const auto* libraries =
      ir.modules().begin()->getAuxData<std::vector<std::string>>("libraries");
  const auto* binaryLibraryPaths =
      ir.modules().begin()->getAuxData<std::vector<std::string>>(
          "libraryPaths");

  // collect all the library paths
  std::vector<std::string> allBinaryPaths = userLibraryPaths;
  if (binaryLibraryPaths)
    allBinaryPaths.insert(allBinaryPaths.end(), binaryLibraryPaths->begin(),
                          binaryLibraryPaths->end());
  // add needed libraries
  if (libraries) {
    for (const auto& library : *libraries) {
      // if they match the lib*.so pattern we let the compiler look for them
      auto infixLibraryName = getInfixLibraryName(library);
      if (infixLibraryName) {
        args.push_back("-l" + *infixLibraryName);
      } else {
        // otherwise we try to find them here
        auto libraryLocation = findLibrary(library, allBinaryPaths);
        if (libraryLocation) {
          args.push_back(*libraryLocation);
        } else {
          std::cerr << "ERROR: Could not find library " << library << std::endl;
        }
      }
    }
  }
  // add user library paths
  for (const auto& libraryPath : userLibraryPaths) {
    args.push_back("-L" + libraryPath);
  }
  // add binary library paths (add them to rpath as well)
  if (binaryLibraryPaths) {
    for (const auto& libraryPath : *binaryLibraryPaths) {
      args.push_back("-L" + libraryPath);
      args.push_back("-Wl,-rpath," + libraryPath);
    }
  }

  if (debug) {
    std::cout << "Compiler arguments: ";
    for (auto i : args)
      std::cout << i << ' ';
    std::cout << std::endl;
  }
  return args;
}

/// Auxiliary class to make sure we delete the temporary assembly file at the
/// end
class TempFile {
public:
  std::string name;
  std::ofstream fileStream;
  TempFile() {
#ifdef _WIN32
    char temp_location[MAX_PATH];
    GetTempPathA(MAX_PATH, temp_location);
    std::string tmpFileName{temp_location};
    tmpFileName.append("fileXXXXXX.s");
#else
    char tmpFileName[] = "/tmp/fileXXXXXX.s";
#endif                                   // _WIN32
    close(mkstemps(&tmpFileName[0], 2)); // Create tmp file
    name = tmpFileName;
    fileStream.open(name);
  };
  ~TempFile() {
    if (fs::exists(name))
      fs::remove(name);
  };
};

int ElfBinaryPrinter::link(std::string outputFilename,
                           const std::vector<std::string>& userLibraryPaths,
                           const gtirb_pprint::PrettyPrinter& pp,
                           gtirb::Context& ctx, gtirb::IR& ir) const {
  if (debug)
    std::cout << "Generating binary file" << std::endl;
  TempFile tempFile;
  if (tempFile.fileStream) {
    if (debug)
      std::cout << "Printing assembly to temporary file " << tempFile.name
                << std::endl;
    pp.print(tempFile.fileStream, ctx, ir);
    tempFile.fileStream.close();
  } else {
    std::cerr << "ERROR: Could not write assembly into a temporary file.\n";
    return -1;
  }

  auto compilerPath = bp::search_path(this->compiler);
  if (compilerPath.empty()) {
    std::cerr << "ERROR: Could not find compiler" << this->compiler;
    return -1;
  }
  if (debug)
    std::cout << "Calling compiler" << std::endl;
  return bp::system(
      compilerPath,
      buildCompilerArgs(outputFilename, tempFile.name, userLibraryPaths, ir));
}

} // namespace gtirb_bprint
