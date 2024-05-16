//
// Created by root on 4/15/24.
//

#include "FunctionLoaderPass.h"
void FunctionLoaderPass::loadImpl(AnalysisPassResult &Result, const gtirb::Context &Context,
                                         const gtirb::Module &Module, AnalysisPass *PreviousPass)
{
}
void FunctionLoaderPass::analyzeImpl(AnalysisPassResult &Result, const gtirb::Module &Module)
{
}
void FunctionLoaderPass::transformImpl(AnalysisPassResult &Result, gtirb::Context &Context,
                                              gtirb::Module &Module)
{
    gtirb::schema::FunctionNames::Type functionNames;
    gtirb::schema::FunctionEntries::Type functionEntries;
    gtirb::schema::FunctionBlocks::Type functionBlocks;
    auto *originFunctionNames =
        ReferenceModule.getAuxData<gtirb::schema::FunctionNames>();
    functionNames.insert(originFunctionNames->begin(),
                         originFunctionNames->end());
    auto *originFunctionEntries =
        ReferenceModule.getAuxData<gtirb::schema::FunctionEntries>();
    functionEntries.insert(originFunctionEntries->begin(),
                           originFunctionEntries->end());
    auto *originFunctionBlocks =
        ReferenceModule.getAuxData<gtirb::schema::FunctionBlocks>();
    functionBlocks.insert(originFunctionBlocks->begin(),
                          originFunctionBlocks->end());
    Module.addAuxData<gtirb::schema::FunctionNames>(std::move(functionNames));
    Module.addAuxData<gtirb::schema::FunctionEntries>(
        std::move(functionEntries));
    Module.addAuxData<gtirb::schema::FunctionBlocks>(std::move(functionBlocks));
}
