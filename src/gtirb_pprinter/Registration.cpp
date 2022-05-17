//===- Registration.cpp -----------------------------------------*- C++ -*-===//
//
//  Copyright (C) 2020 GrammaTech, Inc.
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

#include "Arm64PrettyPrinter.hpp"
#include "ArmPrettyPrinter.hpp"
#include "AttPrettyPrinter.hpp"
#include "AuxDataSchema.hpp"
#include "IntelIcxPrettyPrinter.hpp"
#include "IntelPrettyPrinter.hpp"
#include "MasmPrettyPrinter.hpp"
#include "Mips32PrettyPrinter.hpp"
#include "PrettyPrinter.hpp"
#include <gtirb/gtirb.hpp>

namespace gtirb_pprint {
void registerAuxDataTypes() {
  using namespace gtirb::schema;
  using namespace gtirb::provisional_schema;
  gtirb::AuxDataContainer::registerAuxDataType<Comments>();
  gtirb::AuxDataContainer::registerAuxDataType<FunctionEntries>();
  gtirb::AuxDataContainer::registerAuxDataType<FunctionBlocks>();
  gtirb::AuxDataContainer::registerAuxDataType<SymbolForwarding>();
  gtirb::AuxDataContainer::registerAuxDataType<Encodings>();
  gtirb::AuxDataContainer::registerAuxDataType<SectionProperties>();
  gtirb::AuxDataContainer::registerAuxDataType<CfiDirectives>();
  gtirb::AuxDataContainer::registerAuxDataType<Libraries>();
  gtirb::AuxDataContainer::registerAuxDataType<LibraryPaths>();
  gtirb::AuxDataContainer::registerAuxDataType<PeImportedSymbols>();
  gtirb::AuxDataContainer::registerAuxDataType<PeExportedSymbols>();
  gtirb::AuxDataContainer::registerAuxDataType<PeSafeExceptionHandlers>();
  gtirb::AuxDataContainer::registerAuxDataType<ExportEntries>();
  gtirb::AuxDataContainer::registerAuxDataType<ImportEntries>();
  gtirb::AuxDataContainer::registerAuxDataType<ElfSymbolInfo>();
  gtirb::AuxDataContainer::registerAuxDataType<SymbolicExpressionSizes>();
  gtirb::AuxDataContainer::registerAuxDataType<BinaryType>();
  gtirb::AuxDataContainer::registerAuxDataType<ArchInfo>();
  gtirb::AuxDataContainer::registerAuxDataType<PEResources>();
  gtirb::AuxDataContainer::registerAuxDataType<TypeTable>();
  gtirb::AuxDataContainer::registerAuxDataType<PrototypeTable>();
}

void registerPrettyPrinters() {
  registerPrinter({"elf", "raw"}, {"x86", "x64"}, {"intel"}, {"gas"},
                  std::make_shared<IntelPrettyPrinterFactory>());
  registerPrinter({"elf", "raw"}, {"x86", "x64"}, {"intel"}, {"icx"},
                  std::make_shared<IntelIcxPrettyPrinterFactory>());
  registerPrinter({"elf", "raw"}, {"x86", "x64"}, {"att"}, {"gas"},
                  std::make_shared<AttPrettyPrinterFactory>());
  setDefaultSyntax({"elf", "raw"}, {"x86", "x64"}, {"assembler"}, "att");
  setDefaultSyntax({"elf", "raw"}, {"x86", "x64"}, {"ui", "debug"}, "intel");
  setDefaultAssembler({"elf", "raw"}, {"x86", "x64"}, {"att", "intel"}, "gas");

  registerPrinter({"elf", "raw"}, {"arm"}, {"arm"}, {"gas"},
                  std::make_shared<ArmPrettyPrinterFactory>());
  setDefaultSyntax({"elf", "raw"}, {"arm"}, {"assembler", "ui", "debug"},
                   "arm");
  setDefaultAssembler({"elf", "raw"}, {"arm"}, {"arm"}, "gas");

  registerPrinter({"elf", "raw"}, {"arm64"}, {"arm64"}, {"gas"},
                  std::make_shared<Arm64PrettyPrinterFactory>());
  setDefaultSyntax({"elf", "raw"}, {"arm64"}, {"assembler", "ui", "debug"},
                   "arm64");
  setDefaultAssembler({"elf", "raw"}, {"arm64"}, {"arm64"}, "gas");

  registerPrinter({"elf", "raw"}, {"mips32"}, {"mips32"}, {"gas"},
                  std::make_shared<Mips32PrettyPrinterFactory>());
  setDefaultSyntax({"elf", "raw"}, {"mips32"}, {"assembler", "ui", "debug"},
                   "mips32");
  setDefaultAssembler({"elf", "raw"}, {"mips32"}, {"mips32"}, "gas");

  registerPrinter({"pe", "raw"}, {"x86", "x64"}, {"masm"}, {"gas"},
                  std::make_shared<MasmPrettyPrinterFactory>());
  registerPrinter({"pe", "raw"}, {"x86", "x64"}, {"masm"}, {"uasm"},
                  std::make_shared<UasmPrettyPrinterFactory>());
  setDefaultSyntax({"pe", "raw"}, {"x86", "x64"}, {"assembler", "ui", "debug"},
                   "masm");
  setDefaultAssembler({"pe", "raw"}, {"x86", "x64"}, {"masm"}, "gas");
}
} // namespace gtirb_pprint
