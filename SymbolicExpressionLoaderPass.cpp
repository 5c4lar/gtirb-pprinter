//
// Created by root on 4/15/24.
//

#include "SymbolicExpressionLoaderPass.h"
void SymbolicExpressionLoaderPass::loadImpl(AnalysisPassResult &Result, const gtirb::Context &Context,
                                         const gtirb::Module &Module, AnalysisPass *PreviousPass)
{
}
void SymbolicExpressionLoaderPass::analyzeImpl(AnalysisPassResult &Result, const gtirb::Module &Module)
{
}
void SymbolicExpressionLoaderPass::transformImpl(AnalysisPassResult &Result, gtirb::Context &Context,
                                              gtirb::Module &Module)
{
    auto RefSectionIndex = ReferenceModule.getAuxData<gtirb::schema::SectionIndex>();
    auto ModuleSectionIndex = Module.getAuxData<gtirb::schema::SectionIndex>();
    auto &RefSymbolicExpressionInfo = *ReferenceModule.getAuxData<gtirb::schema::SymbolicExpressionInfo>();
    gtirb::schema::SymbolicExpressionInfo::Type ModuleSymbolicExpressionInfo;

    for (auto &[Index, RefUUID]: *RefSectionIndex) {
        auto RefSection = gtirb::dyn_cast<gtirb::Section>(gtirb::Node::getByUUID(Context, RefUUID));
        if (ModuleSectionIndex->count(Index) == 0) {
            continue;
        }
        auto Section = gtirb::dyn_cast<gtirb::Section>(gtirb::Node::getByUUID(Context, (*ModuleSectionIndex)[Index]));
        for (auto SymbolicExpression: RefSection->symbolic_expressions()) {

            auto *RefBI = SymbolicExpression.getByteInterval();
            auto Offset = RefBI->getAddress().value() + SymbolicExpression.getOffset() - RefSection->getAddress().value();
            auto &BI = Section->findByteIntervalsOn(Section->getAddress().value() + Offset).front();
            BI.addSymbolicExpression(Offset, SymbolicExpression.getSymbolicExpression());
            ModuleSymbolicExpressionInfo[std::make_tuple(BI.getUUID(), Offset)] = RefSymbolicExpressionInfo[std::make_tuple(RefBI->getUUID(), SymbolicExpression.getOffset())];
        }
    }
    Module.addAuxData<gtirb::schema::SymbolicExpressionInfo>(std::move(ModuleSymbolicExpressionInfo));
}
