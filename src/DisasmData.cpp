#include "DisasmData.h"
#include <boost/algorithm/string/trim.hpp>
#include <boost/archive/polymorphic_text_iarchive.hpp>
#include <boost/archive/polymorphic_text_oarchive.hpp>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <gsl/gsl>
#include <gtirb/gtirb.hpp>
#include <iostream>

void DisasmData::parseDirectory(std::string x) {
  boost::trim(x);

  boost::filesystem::path irPath(x + "/gtirb");
  this->loadIRFromFile(irPath.string());

  // These things aren't stored in gtirb yet.
  this->parseDecodedInstruction(x + "/instruction.facts");
  this->parseOpRegdirect(x + "/op_regdirect.facts");
  this->parseOpImmediate(x + "/op_immediate.facts");
  this->parseOpIndirect(x + "/op_indirect.facts");

  this->parseRemainingEA(x + "/phase2-remaining_ea.csv");
  this->parseMainFunction(x + "/main_function.csv");
  this->parseStartFunction(x + "/start_function.csv");
  this->parseFunctionEntry(x + "/function_entry2.csv");
  this->parseAmbiguousSymbol(x + "/ambiguous_symbol.csv");

  this->parseStackOperand(x + "/stack_operand.csv");
  this->parsePreferredDataAccess(x + "/preferred_data_access.csv");
  this->parseDataAccessPattern(x + "/data_access_pattern.csv");

  this->parseDiscardedBlock(x + "/discarded_block.csv");
  this->parseDirectJump(x + "/direct_jump.csv");
  this->parsePCRelativeJump(x + "/pc_relative_jump.csv");
  this->parsePCRelativeCall(x + "/pc_relative_call.csv");
  this->parseBlockOverlap(x + "/block_still_overlap.csv");
  this->parseDefUsed(x + "/def_used.csv");

  this->parsePairedDataAccess(x + "/paired_data_access.csv");
  this->parseValueReg(x + "/value_reg.csv");
  this->parseIncompleteCFG(x + "/incomplete_cfg.csv");
  this->parseNoReturn(x + "/no_return.csv");
  this->parseInFunction(x + "/in_function.csv");
  this->parseBSSData(x + "/bss_data.csv");
}

void DisasmData::loadIRFromFile(std::string path) {
  std::ifstream in(path);
  this->ir.load(in);
  this->functionEAs = boost::get<std::vector<gtirb::EA>>(*this->ir.getTable("functionEAs"));
}

void DisasmData::saveIRToFile(std::string path) {
  std::ofstream out(path);
  this->ir.save(out);
}

void DisasmData::parseDecodedInstruction(const std::string& x) {
  Table fromFile{8};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    DecodedInstruction inst(ff);
    this->instruction.emplace(inst.EA, std::move(inst));
  }

  std::cerr << " # Number of instruction: " << this->instruction.size() << std::endl;
}

void DisasmData::parseOpRegdirect(const std::string& x) {
  Table fromFile{2};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    this->op_regdirect.push_back(OpRegdirect(ff));
  }

  std::cerr << " # Number of op_regdirect: " << this->op_regdirect.size() << std::endl;
}

void DisasmData::parseOpImmediate(const std::string& x) {
  Table fromFile{2};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    OpImmediate op(ff);
    this->op_immediate.emplace(op.N, std::move(op));
  }

  std::cerr << " # Number of op_immediate: " << this->op_immediate.size() << std::endl;
}

void DisasmData::parseOpIndirect(const std::string& x) {
  Table fromFile{7};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    OpIndirect op(ff);
    this->op_indirect.emplace(op.N, std::move(op));
  }

  std::cerr << " # Number of op_indirect: " << this->op_indirect.size() << std::endl;
}

void DisasmData::parseRemainingEA(const std::string& x) {
  Table fromFile{1};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    this->remaining_ea.push_back(boost::lexical_cast<uint64_t>(ff[0]));
  }

  std::cerr << " # Number of remaining_ea: " << this->remaining_ea.size() << std::endl;
}

void DisasmData::parseMainFunction(const std::string& x) {
  Table fromFile{1};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    this->main_function.push_back(boost::lexical_cast<uint64_t>(ff[0]));
  }

  std::cerr << " # Number of main_function: " << this->main_function.size() << std::endl;
}

void DisasmData::parseStartFunction(const std::string& x) {
  Table fromFile{1};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    this->start_function.push_back(boost::lexical_cast<uint64_t>(ff[0]));
  }

  std::cerr << " # Number of start_function: " << this->start_function.size() << std::endl;
}

void DisasmData::parseFunctionEntry(const std::string& x) {
  Table fromFile{1};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    this->function_entry.push_back(boost::lexical_cast<uint64_t>(ff[0]));
  }

  std::sort(std::begin(this->function_entry), std::end(this->function_entry));

  std::cerr << " # Number of function_entry: " << this->function_entry.size() << std::endl;
}

void DisasmData::parseAmbiguousSymbol(const std::string& x) {
  Table fromFile{1};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    this->ambiguous_symbol.push_back(ff[0]);
  }

  std::cerr << " # Number of ambiguous_symbol: " << this->ambiguous_symbol.size() << std::endl;
}

void DisasmData::parseBSSData(const std::string& x) {
  Table fromFile{1};
  fromFile.parseFile(x);

  for (const auto& ff : fromFile) {
    this->bss_data.push_back(boost::lexical_cast<uint64_t>(ff[0]));
  }

  std::sort(std::begin(this->bss_data), std::end(this->bss_data));

  std::cerr << " # Number of bss_data: " << this->bss_data.size() << std::endl;
}

void DisasmData::parseStackOperand(const std::string& x) {
  this->stack_operand.parseFile(x);
  std::cerr << " # Number of stack_operand: " << this->stack_operand.size() << std::endl;
}

void DisasmData::parsePreferredDataAccess(const std::string& x) {
  this->preferred_data_access.parseFile(x);
  std::cerr << " # Number of preferred_data_access: " << this->preferred_data_access.size()
            << std::endl;
}

void DisasmData::parseDataAccessPattern(const std::string& x) {
  this->data_access_pattern.parseFile(x);
  std::cerr << " # Number of data_access_pattern: " << this->data_access_pattern.size()
            << std::endl;
}

void DisasmData::parseDiscardedBlock(const std::string& x) {
  this->discarded_block.parseFile(x);
  std::cerr << " # Number of discarded_block: " << this->discarded_block.size() << std::endl;
}

void DisasmData::parseDirectJump(const std::string& x) {
  this->direct_jump.parseFile(x);
  std::cerr << " # Number of direct_jump: " << this->direct_jump.size() << std::endl;
}

void DisasmData::parsePCRelativeJump(const std::string& x) {
  this->pc_relative_jump.parseFile(x);
  std::cerr << " # Number of pc_relative_jump: " << this->pc_relative_jump.size() << std::endl;
}

void DisasmData::parsePCRelativeCall(const std::string& x) {
  this->pc_relative_call.parseFile(x);
  std::cerr << " # Number of pc_relative_call: " << this->pc_relative_call.size() << std::endl;
}

void DisasmData::parseBlockOverlap(const std::string& x) {
  this->block_overlap.parseFile(x);
  std::cerr << " # Number of block_overlap: " << this->block_overlap.size() << std::endl;
}

void DisasmData::parseDefUsed(const std::string& x) {
  this->def_used.parseFile(x);
  std::cerr << " # Number of def_used: " << this->def_used.size() << std::endl;
}

void DisasmData::parsePairedDataAccess(const std::string& x) {
  this->paired_data_access.parseFile(x);
  std::cerr << " # Number of paired_data_access: " << this->paired_data_access.size() << std::endl;
}

void DisasmData::parseValueReg(const std::string& x) {
  this->value_reg.parseFile(x);
  std::cerr << " # Number of value_reg: " << this->value_reg.size() << std::endl;
}

void DisasmData::parseIncompleteCFG(const std::string& x) {
  this->incomplete_cfg.parseFile(x);
  std::cerr << " # Number of incomplete_cfg: " << this->incomplete_cfg.size() << std::endl;
}

void DisasmData::parseNoReturn(const std::string& x) {
  this->no_return.parseFile(x);
  std::cerr << " # Number of no_return: " << this->no_return.size() << std::endl;
}

void DisasmData::parseInFunction(const std::string& x) {
  this->in_function.parseFile(x);
  std::cerr << " # Number of in_function: " << this->in_function.size() << std::endl;
}

const std::vector<gtirb::Section>& DisasmData::getSections() const {
  return this->ir.getMainModule().getSections();
}

std::map<gtirb::EA, DecodedInstruction>* DisasmData::getDecodedInstruction() {
  return &this->instruction;
}

std::vector<OpRegdirect>* DisasmData::getOPRegdirect() { return &this->op_regdirect; }

std::map<uint64_t, OpImmediate>* DisasmData::getOPImmediate() { return &this->op_immediate; }

std::map<uint64_t, OpIndirect>* DisasmData::getOPIndirect() { return &this->op_indirect; }

std::vector<uint64_t>* DisasmData::getRemainingEA() { return &this->remaining_ea; }

std::vector<uint64_t>* DisasmData::getMainFunction() { return &this->main_function; }

std::vector<uint64_t>* DisasmData::getStartFunction() { return &this->start_function; }

std::vector<uint64_t>* DisasmData::getFunctionEntry() { return &this->function_entry; }

std::vector<std::string>* DisasmData::getAmbiguousSymbol() { return &this->ambiguous_symbol; }

std::vector<uint64_t>* DisasmData::getBSSData() { return &this->bss_data; }

std::vector<gtirb::table::InnerMapType>& DisasmData::getDataSections() {
  return boost::get<std::vector<gtirb::table::InnerMapType>>(*this->ir.getTable("dataSections"));
}

Table* DisasmData::getStackOperand() { return &this->stack_operand; }

Table* DisasmData::getPreferredDataAccess() { return &this->preferred_data_access; }

Table* DisasmData::getDataAccessPattern() { return &this->data_access_pattern; }

Table* DisasmData::getDiscardedBlock() { return &this->discarded_block; }

Table* DisasmData::getDirectJump() { return &this->direct_jump; }

Table* DisasmData::getPCRelativeJump() { return &this->pc_relative_jump; }

Table* DisasmData::getPCRelativeCall() { return &this->pc_relative_call; }

Table* DisasmData::getBlockOverlap() { return &this->block_overlap; }

Table* DisasmData::getDefUsed() { return &this->def_used; }

Table* DisasmData::getPairedDataAccess() { return &this->paired_data_access; }

Table* DisasmData::getValueReg() { return &this->value_reg; }

Table* DisasmData::getIncompleteCFG() { return &this->incomplete_cfg; }

Table* DisasmData::getNoReturn() { return &this->no_return; }

Table* DisasmData::getInFunction() { return &this->in_function; }

std::string DisasmData::getSectionName(uint64_t x) const {
  const auto& sections = this->getSections();
  const auto& match = find_if(sections.begin(), sections.end(),
                              [x](const auto& s) { return s.getStartingAddress() == x; });

  if (match != sections.end()) {
    return match->getName();
  }

  return std::string{};
}

bool DisasmData::isFunction(const gtirb::Symbol& sym) const {
  return std::binary_search(this->functionEAs.begin(), this->functionEAs.end(), sym.getEA());
}

// function_complete_name
std::string DisasmData::getFunctionName(gtirb::EA x) const {
  for (auto& s : gtirb::findSymbols(this->getSymbols(), x)) {
    if (isFunction(*s)) {
      std::stringstream name;
      name << s->getName();

      if (this->getIsAmbiguousSymbol(s->getName()) == true) {
        name << "_" << std::hex << x;
      }

      return name.str();
    }
  }

  if (x == this->main_function[0]) {
    return "main";
  } else if (x == this->start_function[0]) {
    return "_start";
  }

  // or is this a funciton entry?
  for (auto f : this->function_entry) {
    if (x == f) {
      std::stringstream ss;
      ss << "unknown_function_" << std::hex << x;
      return ss.str();
    }
  }

  return std::string{};
}

std::string DisasmData::getGlobalSymbolReference(uint64_t ea) const {
  auto end = getSymbols().rend();
  for (auto it = std::reverse_iterator<gtirb::SymbolSet::const_iterator>(
           getSymbols().upper_bound(gtirb::EA(ea))); //
       it != end && it->second.getEA() <= ea; it++) {
    const auto& sym = it->second;
    gtirb::Data* data = sym.getDataReferent();

    /// \todo This will need looked at again to cover the logic
    if (data && gtirb::utilities::containsEA(*data, gtirb::EA(ea))) {
      uint64_t displacement = ea - sym.getEA().get();

      // in a function with non-zero displacement we do not use the relative addressing
      if (displacement > 0 && isFunction(sym)) {
        return std::string{};
      }
      if (sym.getStorageKind() != gtirb::Symbol::StorageKind::Local) {
        // %do not print labels for symbols that have to be relocated
        const auto name = DisasmData::CleanSymbolNameSuffix(sym.getName());

        if (DisasmData::GetIsReservedSymbol(name) == false) {
          if (displacement > 0) {
            return DisasmData::AvoidRegNameConflicts(name) + "+" + std::to_string(displacement);
          } else {
            return DisasmData::AvoidRegNameConflicts(name);
          }
        }
      }
    }
  }

  // check the relocation table
  for (const auto& r : this->ir.getMainModule().getRelocations()) {
    if (r.ea == ea) {
      if (r.type == std::string{"R_X86_64_GLOB_DAT"})
        return DisasmData::AvoidRegNameConflicts(r.name) + "@GOTPCREL";
      else
        return DisasmData::AvoidRegNameConflicts(r.name);
    }
  }
  return std::string{};
}

std::string DisasmData::getGlobalSymbolName(uint64_t ea) const {
  for (const auto sym : findSymbols(getSymbols(), gtirb::EA(ea))) {
    if (sym->getEA().get() == ea) {
      if ((sym->getStorageKind() != gtirb::Symbol::StorageKind::Local)) {
        // %do not print labels for symbols that have to be relocated
        const auto name = DisasmData::CleanSymbolNameSuffix(sym->getName());

        // if it is not relocated...
        if (this->getRelocation(name) == nullptr) {
          if (DisasmData::GetIsReservedSymbol(name) == false) {
            return std::string{DisasmData::AvoidRegNameConflicts(name)};
          }
        }
      }
    }
  }

  return std::string{};
}

const gtirb::Relocation* const DisasmData::getRelocation(const std::string& x) const {
  auto& relocations = this->ir.getMainModule().getRelocations();
  const auto found = std::find_if(std::begin(relocations), std::end(relocations),
                                  [x](const auto& element) { return element.name == x; });

  if (found != std::end(relocations)) {
    return &(*found);
  }

  return nullptr;
}

const gtirb::SymbolSet& DisasmData::getSymbols() const {
  return this->ir.getMainModule().getSymbols();
}

const gtirb::Section* const DisasmData::getSection(const std::string& x) const {
  const auto found = std::find_if(getSections().begin(), getSections().end(),
                                  [x](const auto& element) { return element.getName() == x; });

  if (found != getSections().end()) {
    return &(*found);
  }

  return nullptr;
}

const DecodedInstruction* const DisasmData::getDecodedInstruction(uint64_t ea) const {
  const auto inst = this->instruction.find(gtirb::EA(ea));

  if (inst != this->instruction.end()) {
    return &(inst->second);
  }

  return nullptr;
}

const OpIndirect* const DisasmData::getOpIndirect(uint64_t x) const {
  const auto found = this->op_indirect.find(x);

  if (found != std::end(this->op_indirect)) {
    return &found->second;
  }

  return nullptr;
}

const OpRegdirect* const DisasmData::getOpRegdirect(uint64_t x) const {
  const auto found = std::find_if(std::begin(this->op_regdirect), std::end(this->op_regdirect),
                                  [x](const auto& element) { return element.N == x; });

  if (found != std::end(this->op_regdirect)) {
    return &(*found);
  }

  return nullptr;
}

uint64_t DisasmData::getOpRegdirectCode(std::string x) const {
  const auto found = std::find_if(std::begin(this->op_regdirect), std::end(this->op_regdirect),
                                  [x](const auto& element) { return element.Register == x; });

  if (found != std::end(this->op_regdirect)) {
    return found->N;
  }

  return 0;
}

const OpImmediate* const DisasmData::getOpImmediate(uint64_t x) const {
  const auto found = this->op_immediate.find(x);

  if (found != std::end(this->op_immediate)) {
    return &found->second;
  }

  return nullptr;
}

bool DisasmData::getIsAmbiguousSymbol(const std::string& name) const {
  const auto found =
      std::find(std::begin(this->ambiguous_symbol), std::end(this->ambiguous_symbol), name);
  return found != std::end(this->ambiguous_symbol);
}

void DisasmData::AdjustPadding(std::vector<gtirb::Block>& blocks) {
  for (auto i = std::begin(blocks); i != std::end(blocks); ++i) {
    auto next = i;
    ++next;
    if (next != std::end(blocks)) {
      const auto gap = next->getStartingAddress() - i->getEndingAddress();

      // If we have overlap, erase the next element in the list.
      if (i->getEndingAddress() > next->getStartingAddress()) {
        blocks.erase(next);
      } else if (gap > 0) {
        // insert a block with no instructions.
        // This should be interpreted as nop's.
        blocks.insert(next, gtirb::Block{i->getEndingAddress(), next->getStartingAddress()});
      }
    }
  }
}

std::string DisasmData::CleanSymbolNameSuffix(std::string x) {
  return x.substr(0, x.find_first_of('@'));
}

std::string DisasmData::AdaptOpcode(const std::string& x) {
  const std::map<std::string, std::string> adapt{{"movsd2", "movsd"}, {"imul2", "imul"},
                                                 {"imul3", "imul"},   {"imul1", "imul"},
                                                 {"cmpsd3", "cmpsd"}, {"out_i", "out"}};

  const auto found = adapt.find(x);
  if (found != std::end(adapt)) {
    return found->second;
  }

  return x;
}

std::string DisasmData::AdaptRegister(const std::string& x) {
  const std::map<std::string, std::string> adapt{
      {"R8L", "R8B"},   {"R9L", "R9B"},   {"R10L", "R10B"}, {"R11L", "R11B"}, {"R12L", "R12B"},
      {"R13L", "R13B"}, {"R14L", "R14B"}, {"R15L", "R15B"}, {"R12L", "R12B"}, {"R13L", "R13B"},
      {"ST0", "ST(0)"}, {"ST1", "ST(1)"}, {"ST2", "ST(2)"}, {"ST3", "ST(3)"}, {"ST4", "ST(4)"},
      {"ST5", "ST(5)"}, {"ST6", "ST(6)"}, {"ST7", "ST(7)"}};

  const auto found = adapt.find(x);
  if (found != std::end(adapt)) {
    return found->second;
  }

  return x;
}

std::string DisasmData::GetSizeName(uint64_t x) {
  return DisasmData::GetSizeName(std::to_string(x));
}

std::string DisasmData::GetSizeName(const std::string& x) {
  const std::map<std::string, std::string> adapt{
      {"128", ""},         {"0", ""},          {"80", "TBYTE PTR"}, {"64", "QWORD PTR"},
      {"32", "DWORD PTR"}, {"16", "WORD PTR"}, {"8", "BYTE PTR"}};

  const auto found = adapt.find(x);
  if (found != std::end(adapt)) {
    return found->second;
  }

  assert("Unknown Size");

  return x;
}

std::string DisasmData::GetSizeSuffix(const OpIndirect& x) {
  return DisasmData::GetSizeSuffix(x.Size);
}

std::string DisasmData::GetSizeSuffix(uint64_t x) {
  return DisasmData::GetSizeSuffix(std::to_string(x));
}

std::string DisasmData::GetSizeSuffix(const std::string& x) {
  const std::map<std::string, std::string> adapt{{"128", ""}, {"0", ""},   {"80", "t"}, {"64", "q"},
                                                 {"32", "d"}, {"16", "w"}, {"8", "b"}};

  const auto found = adapt.find(x);
  if (found != std::end(adapt)) {
    return found->second;
  }

  assert("Unknown Size");

  return x;
}

bool DisasmData::GetIsReservedSymbol(const std::string& x) {
  if (x.length() > 2) {
    return ((x[0] == '_') && (x[1] == '_'));
  }

  return false;
}

std::string DisasmData::AvoidRegNameConflicts(const std::string& x) {
  const std::vector<std::string> adapt{"FS", "MOD", "DIV", "NOT", "mod", "div", "not", "and", "or"};

  const auto found = std::find(std::begin(adapt), std::end(adapt), x);
  if (found != std::end(adapt)) {
    return x + "_renamed";
  }

  return x;
}

// Name, Alignment.
const std::array<std::pair<std::string, int>, 7> DataSectionDescriptors{{
    {".got", 8},         //
    {".got.plt", 8},     //
    {".data.rel.ro", 8}, //
    {".init_array", 8},  //
    {".fini_array", 8},  //
    {".rodata", 16},     //
    {".data", 16}        //
}};

const std::pair<std::string, int>* getDataSectionDescriptor(const std::string& name) {
  const auto foundDataSection =
      std::find_if(std::begin(DataSectionDescriptors), std::end(DataSectionDescriptors),
                   [name](const auto& dsd) { return dsd.first == name; });
  if (foundDataSection != std::end(DataSectionDescriptors))
    return foundDataSection;
  else
    return nullptr;
}
