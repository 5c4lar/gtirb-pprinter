#ifndef AUXDATALOADER_HPP
#define AUXDATALOADER_HPP

#include <gtirb/gtirb.hpp>
#include <optional>
#include <type_traits>

#include "AuxDataSchema.hpp"

/*
 * What goes here?

 * 1. Validation
 * -- Check AuxData for all needed tables
 *    mandatory:
 *      all:
 *          FunctionEntries
 *          FunctionBlocks
 *      elf:
 *           elfSymbolInfo
 *          elfSectionProperties
 *      Pe:
 *          PeSectionProperties
 *    Optional:
 *      all:
 *          Encodings
 *          Comments
 *          CfiDirectives
 *          SymbolForwarding
 *          SymbolicExpressionSizes
 *          Libraries
 *          LibraryPaths
 *          BinaryType
 *      pe:
 *          PeImportedSymbols
 *          PeExportedSymbols
 *          ImportEntries
 *          ExportEntries
 *          PEResources
 *
 * - each block in FunctionBlocks must have positive size
 *
 * 2. structure
 * -- structure the auxdata if there's additional logic
 *    to it
 *
 */

namespace aux_data {

namespace util {

// Dereference an AuxData table or initialize a default value.
template <typename Schema>
typename Schema::Type getOrDefault(const typename Schema::Type* SchemaPtr) {
  if (SchemaPtr) {
    return *SchemaPtr;
  }
  return {};
}

// Load an AuxData table from a Module or a default value.
template <typename Schema>
typename Schema::Type getOrDefault(const gtirb::Module& Module) {
  return getOrDefault<Schema>(Module.getAuxData<Schema>());
}

// Access a map-typed AuxData schema by key value.
template <typename Schema, typename KeyType>
std::optional<typename Schema::Type::mapped_type>
getByKey(KeyType K, const typename Schema::Type* SchemaPtr) {
  if (SchemaPtr) {
    if (auto Val = SchemaPtr->find(K); Val != SchemaPtr->end()) {
      return Val->second;
    }
  }
  return std::nullopt;
}

// Access a map-typed AuxData schema by key value.
template <typename Schema>
std::optional<typename Schema::Type::mapped_type>
getByOffset(const gtirb::Offset Offset, const gtirb::Module& Mod) {
  return getByKey<Schema, gtirb::Offset>(Offset, Mod.getAuxData<Schema>());
}

// Access a map-typed AuxData keyed by gtirb::Offset.
template <typename Schema>
std::optional<typename Schema::Type::mapped_type>
getByNode(const gtirb::Node& Node, const gtirb::Module& Mod) {
  return getByKey<Schema, gtirb::UUID>(Node.getUUID(),
                                       Mod.getAuxData<Schema>());
}

}; // namespace util

namespace elf {

// Table maps ELF flag labels to assembly keywords.
static const std::unordered_map<std::string, std::string> TypeNameConversion = {
    {"FUNC", "function"},  {"OBJECT", "object"},
    {"NOTYPE", "notype"},  {"NONE", "notype"},
    {"TLS", "tls_object"}, {"GNU_IFUNC", "gnu_indirect_function"},
};

}; // namespace elf

// Type wrapper for ELF symbol properties stored in the `elfSymbolInfo' table.
struct ElfSymbolInfo {
  using AuxDataType =
      std::tuple<uint64_t, std::string, std::string, std::string, uint64_t>;

  uint64_t Size;
  std::string Type;
  std::string Binding;
  std::string Visibility;
  uint64_t SectionIndex;

  ElfSymbolInfo(const AuxDataType& Tuple)
      : Size(std::get<0>(Tuple)), Type(std::get<1>(Tuple)),
        Binding(std::get<2>(Tuple)), Visibility(std::get<3>(Tuple)),
        SectionIndex(std::get<4>(Tuple)) {}

  AuxDataType asAuxData() {
    return AuxDataType{Size, Type, Binding, Visibility, SectionIndex};
  }

  std::optional<std::string> convertType() {
    if (auto Converted = elf::TypeNameConversion.find(Type);
        Converted != elf::TypeNameConversion.end()) {
      return Converted->second;
    }
    return std::nullopt;
  }
};

// Type wrapper for CFI directives stored in the `.cfiDirectives' table.
struct CFIDirective {
  using AuxDataType =
      std::tuple<std::string, std::vector<int64_t>, gtirb::UUID>;

  std::string Directive;
  std::vector<int64_t> Operands;
  gtirb::UUID Uuid;

  CFIDirective(const AuxDataType& Auxdata)
      : Directive(std::get<0>(Auxdata)), Operands(std::get<1>(Auxdata)),
        Uuid(std::get<2>(Auxdata)){};

  AuxDataType asAuxData() { return AuxDataType{Directive, Operands, Uuid}; }
};

std::optional<std::string> getEncodingType(const gtirb::DataBlock& DataBlock);

// Find CFI directives for some location in a byte interval in the
// `cfiDirectives' AuxData table.
std::optional<std::vector<CFIDirective>>
getCFIDirectives(const gtirb::Offset& Offset, const gtirb::Module& Mod);

// Load all function entry nodes from the `functionEntries' Auxdata table.
gtirb::schema::FunctionEntries::Type
getFunctionEntries(const gtirb::Module& Mod);

// Load all function block UUIDs from the `functionBlocks' AuxData table.
std::map<gtirb::UUID, std::set<gtirb::UUID>>
getFunctionBlocks(const gtirb::Module& Mod);

// Find the size of a symbolic expression by offset (`symbolicExpressionSizes').
std::optional<uint64_t> getSymbolicExpressionSize(const gtirb::Offset& Offset,
                                                  const gtirb::Module& Mod);

// Load all alignment entries from the `alignment' AuxData table.
gtirb::schema::Alignment::Type getAlignments(const gtirb::Module& Mod);

std::optional<uint64_t> getAlignment(const gtirb::UUID& Uuid,
                                     const gtirb::Module& Mod);

// Find a mapping of one symbol to another in the `symbolForwarding' AuxData
// table.
std::optional<gtirb::UUID> getForwardedSymbol(const gtirb::Symbol* Symbol);

// Load all library names from the `libraries' AuxData table.
std::vector<std::string> getLibraries(const gtirb::Module& Module);

// Load all library path names from the `libraryPaths' AuxData table.
std::vector<std::string> getLibraryPaths(const gtirb::Module& Module);

// Load all binary type specifiers from the `binaryType' AuxData table.
std::vector<std::string> getBinaryType(const gtirb::Module& Module);

// Load symbol forwarding mapping from the `symbolForwarding' AuxData table.
std::map<gtirb::UUID, gtirb::UUID>
getSymbolForwarding(const gtirb::Module& Module);

// Load all comments for instructions from the `comments' AuxData table.
const gtirb::schema::Comments::Type* getComments(const gtirb::Module& Module);

// Check that a Module's AuxData contains all tables required for printing.
bool validateAuxData(const gtirb::Module& Mod, std::string TargetFormat);

// Load the properties of a symbol from the `elfSymbolInfo' AuxData table.
std::optional<aux_data::ElfSymbolInfo>
getElfSymbolInfo(const gtirb::Symbol& Sym);

// Store the properties of a symbol to the `elfSymbolInfo' AuxData table.
void setElfSymbolInfo(gtirb::Symbol& Sym, aux_data::ElfSymbolInfo& Info);

// Load the section properties of an ELF binary section from the
// `elfSectionProperties' AuxData tables.
std::optional<std::tuple<uint64_t, uint64_t>>
getElfSectionProperties(const gtirb::Section& Section);

// Load the section properties of a PE binary section from the
// `elfSectionProperties' AuxData tables.
std::optional<uint64_t> getPeSectionProperties(const gtirb::Section& Section);

// Load list UUIDs for symbols imported by a PE binary from the
// `peImportedSymbols' AuxData table.
gtirb::schema::ImportEntries::Type getImportEntries(const gtirb::Module& M);

// Load list UUIDs for symbols exported by a PE binary from the
// `peExportedSymbols' AuxData table.
gtirb::schema::ExportEntries::Type getExportEntries(const gtirb::Module& M);

// Load all PE resources from the `peResources' AuxData table.
gtirb::schema::PEResources::Type getPEResources(const gtirb::Module& M);

// Load all imported symbol properties for a PE binary from the
// `peImportEntries' AuxData table.
gtirb::schema::PeImportedSymbols::Type
getPeImportedSymbols(const gtirb::Module& M);

// Load all exported symbol properties for a PE binary from the
// `peExportEntries' AuxData table.
gtirb::schema::PeExportedSymbols::Type
getPeExportedSymbols(const gtirb::Module& M);

} // namespace aux_data

#endif
