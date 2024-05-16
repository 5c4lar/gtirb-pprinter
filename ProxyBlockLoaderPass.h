//
// Created by root on 4/15/24.
//

#ifndef DDISASM_ProxyBlockLoaderPass_H
#define DDISASM_ProxyBlockLoaderPass_H

#include <gtirb/gtirb.hpp>

#include "../../AuxDataSchema.h"
#include <passes/AnalysisPass.h>

/**
Compute strongly connected components and store them in a AuxData table SccMap called "SCCs"
*/
class ProxyBlockLoaderPass : public AnalysisPass
{
public:
    explicit ProxyBlockLoaderPass(gtirb::Module& referenceModule) : ReferenceModule(referenceModule)
    {}
    virtual std::string getName() const override
    {
        return "ProxyBlock loader";
    }

    virtual bool hasTransform(void) override
    {
        return true;
    }

protected:
    virtual void loadImpl(AnalysisPassResult& Result, const gtirb::Context& Context,
                          const gtirb::Module& Module,
                          AnalysisPass* PreviousPass) override;
    virtual void analyzeImpl(AnalysisPassResult& Result, const gtirb::Module& Module) override;
    virtual void transformImpl(AnalysisPassResult& Result, gtirb::Context& Context,
                               gtirb::Module& Module) override;

private:
    gtirb::Module& ReferenceModule;
};

#endif // DDISASM_ProxyBlockLoaderPass_H
