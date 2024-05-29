//
// Created by root on 4/15/24.
//

#include "SymbolLoaderPass.h"
void SymbolLoaderPass::loadImpl(AnalysisPassResult &Result, const gtirb::Context &Context,
                                         const gtirb::Module &Module, AnalysisPass *PreviousPass)
{
}
void SymbolLoaderPass::analyzeImpl(AnalysisPassResult &Result, const gtirb::Module &Module)
{
}
void SymbolLoaderPass::transformImpl(AnalysisPassResult &Result, gtirb::Context &Context,
                                              gtirb::Module &Module)
{
    auto RefSectionIndex = ReferenceModule.getAuxData<gtirb::schema::SectionIndex>();
    auto ModuleSectionIndex = Module.getAuxData<gtirb::schema::SectionIndex>();
    auto SectionMap = std::map<gtirb::Section*, gtirb::Section*>();

    for (auto &[Index, RefUUID]: *RefSectionIndex)
    {
        auto RefSection = gtirb::dyn_cast<gtirb::Section>(gtirb::Node::getByUUID(Context, RefUUID));
        if(ModuleSectionIndex->count(Index) == 0)
        {
            continue;
        }
        auto Section = gtirb::dyn_cast<gtirb::Section>(
            gtirb::Node::getByUUID(Context, (*ModuleSectionIndex)[Index]));
        SectionMap[RefSection] = Section;
    }
    std::vector<gtirb::Symbol *> absoluteSymbols;
    for (auto &symbol : Module.symbols()) {
        absoluteSymbols.emplace_back(&symbol);
    }
    for (auto *symbol : absoluteSymbols) {
        Module.removeSymbol(symbol);
    }
    std::vector<gtirb::Symbol *> originSymbols;
    for (auto &symbol : ReferenceModule.symbols()) {
        originSymbols.emplace_back(&symbol);
    }
    for (auto *symbol : originSymbols) {
        if (symbol->hasReferent()) {
            auto *datablock = symbol->getReferent<gtirb::DataBlock>();
            auto *codeblock = symbol->getReferent<gtirb::CodeBlock>();
            auto *proxyblock = symbol->getReferent<gtirb::ProxyBlock>();
            if (datablock) {
                if (datablock->getByteInterval()->getSection()->getModule()->getUUID() == ReferenceModule.getUUID()) {
                    continue;
                }
            } else if (codeblock) {
                if (codeblock->getByteInterval()->getSection()->getModule()->getUUID() == ReferenceModule.getUUID()) {
                    continue;
                }
            }
        }
        Module.addSymbol(symbol);
    }
}
