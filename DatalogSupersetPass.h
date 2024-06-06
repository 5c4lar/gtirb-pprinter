//
// Created by root on 5/21/24.
//

#ifndef DDISASM_DATALOGSUPERSETPASS_H
#define DDISASM_DATALOGSUPERSETPASS_H

#include <gtirb/gtirb.hpp>
#include <utility>

#include <gtirb-decoder/CompositeLoader.h>
#include <passes/DatalogAnalysisPass.h>

class DatalogSupersetPass : public DatalogAnalysisPass
{
    // Loader factory registration.
    using Target = std::tuple<gtirb::FileFormat, gtirb::ISA, gtirb::ByteOrder>;
    using Factory = std::function<CompositeLoader()>;
public:
    static std::map<Target, Factory>& loaders();
    static void registerLoader(Target T, Factory F)
    {
        loaders()[T] = std::move(F);
    }
    static void registerDatalogLoaders();
    virtual std::string getName() const override
    {
        return "datalog superset loader";
    }

    virtual std::string getSourceFilename() const override
    {
        return "src/tools/gtirb-mixer/datalog/datalog_superset.dl";
    }

    virtual bool hasLoad(void) override
    {
        return true;
    }

    virtual bool hasTransform(void) override
    {
        return true;
    }

protected:
    void loadImpl(AnalysisPassResult& Result, const gtirb::Context& Context, const gtirb::Module &Module, AnalysisPass* PreviousPass=nullptr) override;
    void transformImpl(AnalysisPassResult& Result, gtirb::Context& Context, gtirb::Module &Module) override;
};

#endif // DDISASM_DATALOGSUPERSETPASS_H
