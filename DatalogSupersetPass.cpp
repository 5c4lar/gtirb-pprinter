//
// Created by root on 5/21/24.
//

#include "DatalogSupersetPass.h"

void DatalogSupersetPass::transformImpl(AnalysisPassResult& Result, gtirb::Context& Context,
                                      gtirb::Module& Module)
{
    DatalogAnalysisPass::transformImpl(Result, Context, Module);
}
void DatalogSupersetPass::loadImpl(AnalysisPassResult& Result, const gtirb::Context& Context,
                                 const gtirb::Module& Module, AnalysisPass* PreviousPass)
{
    // Build GTIRB loader.
    CompositeLoader Loader("souffle_datalog_superset");

    Program = Loader.load(Module);
    if(!Program)
    {
        Result.Errors.push_back("Could not create souffle_datalog_superset program");
    }
}