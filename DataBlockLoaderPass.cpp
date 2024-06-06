//
// Created by root on 4/15/24.
//

#include "DataBlockLoaderPass.h"
void DataBlockLoaderPass::loadImpl(AnalysisPassResult &Result, const gtirb::Context &Context,
                                         const gtirb::Module &Module, AnalysisPass *PreviousPass)
{
}
void DataBlockLoaderPass::analyzeImpl(AnalysisPassResult &Result, const gtirb::Module &Module)
{
}
void DataBlockLoaderPass::transformImpl(AnalysisPassResult &Result, gtirb::Context &Context,
                                              gtirb::Module &Module)
{
    auto RefSectionIndex = ReferenceModule.getAuxData<gtirb::schema::SectionIndex>();
    auto ModuleSectionIndex = Module.getAuxData<gtirb::schema::SectionIndex>();

    for (auto &[Index, RefUUID]: *RefSectionIndex)
    {
        auto RefSection = gtirb::dyn_cast<gtirb::Section>(gtirb::Node::getByUUID(Context, RefUUID));
        if(ModuleSectionIndex->count(Index) == 0)
        {
            continue;
        }
        auto Section = gtirb::dyn_cast<gtirb::Section>(
            gtirb::Node::getByUUID(Context, (*ModuleSectionIndex)[Index]));
        std::vector<gtirb::DataBlock *> DataBlocks;
        for(auto &Block : RefSection->data_blocks())
        {
            DataBlocks.push_back(&Block);
        }
        for(auto *Block : DataBlocks)
        {
            auto Offset = Block->getAddress().value() - RefSection->getAddress().value();
            auto Addr = Section->getAddress().value();
            auto Result = Section->findByteIntervalsOn(Addr + Offset);
            if (!Result.empty()) {
                auto &ByteInterval = Result.front();
                ByteInterval.addBlock(Offset, Block);
            }
        }
    }
}
