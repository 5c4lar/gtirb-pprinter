//
// Created by root on 5/14/24.
//

#ifndef GTIRB_MIXER_AUXDATASCHEMA_H
#define GTIRB_MIXER_AUXDATASCHEMA_H
#include <AuxDataSchema.h>
namespace auxdata {
    /// SymbolicExpressionInfo is a tuple of the form {UUID, offset, size, type, variant}.
    using SymbolicExpressionInfo = std::map<std::tuple<gtirb::UUID, uint64_t>, std::tuple<uint32_t, uint32_t>>;
} // namespace auxdata

namespace gtirb {
    namespace schema {
        // 5c4lar
        /// \brief Auxilary data recording symbolic expression types and variant
        struct SymbolicExpressionInfo
        {
            static constexpr const char* Name = "symbolicExpressionInfo";
            typedef auxdata::SymbolicExpressionInfo Type;
        };
    } // namespace schema
} // namespace gtirb
#endif // DDISASM_AUXDATASCHEMA_H
