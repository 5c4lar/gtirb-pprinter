//
// Created by root on 4/15/24.
//

#include "ProxyBlockLoaderPass.h"
void ProxyBlockLoaderPass::loadImpl(AnalysisPassResult &Result, const gtirb::Context &Context,
                                         const gtirb::Module &Module, AnalysisPass *PreviousPass)
{
}
void ProxyBlockLoaderPass::analyzeImpl(AnalysisPassResult &Result, const gtirb::Module &Module)
{
}
void ProxyBlockLoaderPass::transformImpl(AnalysisPassResult &Result, gtirb::Context &Context,
                                              gtirb::Module &Module)
{
    std::vector<gtirb::ProxyBlock*> proxyBlocks;
    for (auto &block : ReferenceModule.proxy_blocks()) {
        proxyBlocks.push_back(&block);
    }
    for (auto *block : proxyBlocks) {
        Module.addProxyBlock(block);
    }
}
