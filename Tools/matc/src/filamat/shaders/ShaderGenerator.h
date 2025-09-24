#pragma once
#include "MaterialInfo.h"
#include "filaflat/MaterialEnums.h"
#include <filamat/MaterialBuilder.h>
#include "filaflat/backend/DriverEnums.h"
#include <sstream>
#include <string>

#include <stdint.h>
#include <stddef.h>

namespace filamat
{

class CodeGenerator;

class ShaderGenerator
{
public:
    ShaderGenerator(
        std::string const& materialCode,
        size_t lineOffset,
        std::string const& materialVertexCode,
        size_t vertexLineOffset) noexcept;
    ShaderGenerator() = default;
    void setUpGeomertyCode(std::string const& materialCode, size_t const lineOffset);
    void setUpVertexCode(std::string const& code, size_t const lineOffset);
    void setUpFragmentCode(std::string const& code, size_t const lineOffset);

    std::string createSurfaceVertexProgram(
        MaterialInfo const& material, const std::string& featureDefine = "") const noexcept;
    std::string createSurfaceGeomertyProgram(
        MaterialInfo const& material, const std::string& featureDefine = "") const noexcept;

    std::string createSurfaceFragmentProgram(
        MaterialInfo const& material, const std::string& featureDefine = "") const noexcept;




private:


    static void appendShader(std::ostringstream& ss,
        const std::string& shader, size_t lineOffset) noexcept;

    std::string mMaterialFragmentCode; // fragment or compute code
    std::string mMaterialVertexCode;
    std::string mMaterialGeomertyCode;
    size_t mMaterialLineOffset;
    size_t mMaterialVertexLineOffset;
    size_t mMaterialGeomertyLineOffset;
    bool mIsMaterialVertexShaderEmpty;
};

} // namespace filamat