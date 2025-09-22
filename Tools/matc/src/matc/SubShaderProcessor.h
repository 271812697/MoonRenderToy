#pragma once
#include <unordered_map>
#include <string>
#include "JsonishParser.h"

#include <filamat/MaterialBuilder.h>

namespace matc {
class MaterialLexeme;

class SubShaderProcessor {

public:
    SubShaderProcessor();
    ~SubShaderProcessor() = default;
    bool process(filamat::MaterialBuilder& builder, const MaterialLexeme& lexeme);
    bool processPassJson(filamat::MaterialBuilder& builder, const JsonishObject& jsonObject)const;
    bool processComment(const MaterialLexeme& lexeme, filamat::MaterialBuilder& builder) const;
    bool processPass(const MaterialLexeme&lexeme, filamat::MaterialBuilder& builder) const;
private:

    using Callback = bool (*)(filamat::MaterialBuilder& builder, const JsonishValue& value);

    struct ParameterInfo {
        Callback callback;
        JsonishValue::Type rootAssert;
    };

    std::unordered_map<std::string, ParameterInfo> mParameters;

        // Member function pointer type, this is used to implement a Command design
    // pattern.
    using SubShaderConfigProcessor = bool (SubShaderProcessor::*)(
        const MaterialLexeme&, filamat::MaterialBuilder& builder) const;
    // Map used to store Command pattern function pointers.
    std::unordered_map<std::string, SubShaderConfigProcessor> mConfigProcessor;
};

} // namespace matc
