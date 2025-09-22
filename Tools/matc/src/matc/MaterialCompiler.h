#pragma once

#include <string>
#include <unordered_map>

#include "Compiler.h"
#include "MaterialLexeme.h"

namespace filamat {
class MaterialBuilder;
}
class TestMaterialCompiler;

namespace matc {

class JsonishValue;
class MaterialCompiler final: public Compiler {
public:
    MaterialCompiler();

    bool run(const Config& config) override;

    bool checkParameters(const Config& config) override;

private:
    friend class ::TestMaterialCompiler;

    bool parseMaterial(const char* buffer, size_t size,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processMaterial(const MaterialLexeme&,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processSubshader(const MaterialLexeme&, filamat::MaterialBuilder& builder) const noexcept;
    bool processVertexShader(const MaterialLexeme&,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processGeomertyShader(const MaterialLexeme&,
        filamat::MaterialBuilder& builder) const noexcept;
    bool processFragmentShader(const MaterialLexeme&,
            filamat::MaterialBuilder& builder) const noexcept;
    bool processComputeShader(const MaterialLexeme&,
            filamat::MaterialBuilder& builder) const noexcept;
    bool ignoreLexeme(const MaterialLexeme&, filamat::MaterialBuilder& builder) const noexcept;


    bool compileRawShader(const char* glsl, size_t size, bool isDebug, Config::Output* output,
                const char* ext) const noexcept;

    bool processMaterialParameters(filamat::MaterialBuilder& builder, const Config& config) const;

    // Member function pointer type, this is used to implement a Command design
    // pattern.
    using MaterialConfigProcessor = bool (MaterialCompiler::*)
            (const MaterialLexeme&, filamat::MaterialBuilder& builder) const;
    // Map used to store Command pattern function pointers.
    std::unordered_map<std::string, MaterialConfigProcessor> mConfigProcessor;

    // The same, but for pure JSON syntax
    using MaterialConfigProcessorJSON = bool (MaterialCompiler::*)
            (const JsonishValue*, filamat::MaterialBuilder& builder) const;
    std::unordered_map<std::string, MaterialConfigProcessorJSON> mConfigProcessorJSON;
};

} 