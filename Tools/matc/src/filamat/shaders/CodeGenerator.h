#pragma once
#include "MaterialInfo.h"
#include "filamat/MaterialBuilder.h"

#include "filaflat/MaterialEnums.h"
#include "filaflat/SamplerInterfaceBlock.h"
#include "filaflat/BufferInterfaceBlock.h"
#include "filaflat/backend/DriverEnums.h"
#include <sstream>
#include <exception>
#include <string>


namespace filamat {

class  CodeGenerator {
   
    using ShaderStage = filament::backend::ShaderStage;

    using TargetLanguage = MaterialBuilder::TargetLanguage;
    using ShaderQuality = MaterialBuilder::ShaderQuality;
public:
    CodeGenerator() noexcept
   {
    }

 
    // insert a separator (can be a new line)
    static std::ostringstream& generateSeparator(std::ostringstream& out);

    // generate prolog for the given shader
    std::ostringstream& generateCommonProlog(
        std::ostringstream& out, ShaderStage stage, const std::string& featureDefineStr = "") const;

    static std::ostringstream& generateCommonEpilog(std::ostringstream& out);

    static std::ostringstream& generateSurfaceTypes(std::ostringstream& out, ShaderStage stage);

    // generate common functions for the given shader
    static std::ostringstream& generateSurfaceCommon(std::ostringstream& out, ShaderStage stage);
    static std::ostringstream& generatePostProcessCommon(std::ostringstream& out, ShaderStage stage);
    static std::ostringstream& generateSurfaceMaterial(std::ostringstream& out, ShaderStage stage);

    static std::ostringstream& generateSurfaceFog(std::ostringstream& out, ShaderStage stage);

    // generate the shader's main()
    static std::ostringstream& generateSurfaceMain(std::ostringstream& out, ShaderStage stage);
    static std::ostringstream& generatePostProcessMain(std::ostringstream& out, ShaderStage stage);

    // generate the shader's code for the screen-space reflections
    static std::ostringstream& generateSurfaceReflections(std::ostringstream& out, ShaderStage stage);

    // generate declarations for non-custom "in" variables
    std::ostringstream& generateSurfaceShaderInputs(std::ostringstream& out, ShaderStage stage) const;
    static std::ostringstream& generatePostProcessInputs(std::ostringstream& out, ShaderStage stage);



    // generate no-op shader for depth prepass
    static std::ostringstream& generateSurfaceDepthMain(std::ostringstream& out, ShaderStage stage);

    // generate samplers
    std::ostringstream& generateCommonSamplers(std::ostringstream& out,
        filament::SamplerInterfaceBlock::SamplerInfoList const& list, ShaderStage stage) const;

    std::ostringstream& generateCommonSamplers(std::ostringstream& out,
            const filament::SamplerInterfaceBlock& sib,ShaderStage stage) const {
        return generateCommonSamplers(out,  sib.getSamplerInfoList(),stage);
    }



    // generate uniforms
    std::ostringstream& generateUniforms(std::ostringstream& out, ShaderStage stage,
            const filament::BufferInterfaceBlock& uib) const;

    // generate buffers
    std::ostringstream& generateBuffers(std::ostringstream& out,
            MaterialInfo::BufferContainer const& buffers) const;

    // generate an interface block
    std::ostringstream& generateBufferInterfaceBlock(std::ostringstream& out, ShaderStage stage,
            filament::backend::descriptor_binding_t binding,
            const filament::BufferInterfaceBlock& uib) const;

    // generate material properties getters
    static std::ostringstream& generateMaterialProperty(std::ostringstream& out,
            MaterialBuilder::Property property, bool isSet);

    std::ostringstream& generateQualityDefine(std::ostringstream& out, ShaderQuality quality) const;

    static std::ostringstream& generateDefine(std::ostringstream& out, const char* name, bool value);
    static std::ostringstream& generateDefine(std::ostringstream& out, const char* name, uint32_t value);
    static std::ostringstream& generateDefine(std::ostringstream& out, const char* name, const char* string);
    static std::ostringstream& generateIndexedDefine(std::ostringstream& out, const char* name,
            uint32_t index, uint32_t value);



    static std::ostringstream& generatePostProcessGetters(std::ostringstream& out, ShaderStage stage);
    static std::ostringstream& generateSurfaceGetters(std::ostringstream& out, ShaderStage stage);
    static std::ostringstream& generateSurfaceParameters(std::ostringstream& out, ShaderStage stage);


    // These constants must match the equivalent in MetalState.h.
    // These values represent the starting index for uniform, ssbo, and sampler group [[buffer(n)]]
    // bindings. See the chart at the top of MetalState.h.
    static constexpr uint32_t METAL_PUSH_CONSTANT_BUFFER_INDEX = 20u;
    static constexpr uint32_t METAL_DESCRIPTOR_SET_BINDING_START = 21u;
    static constexpr uint32_t METAL_DYNAMIC_OFFSET_BINDING = 25u;

    uint32_t getUniqueSamplerBindingPoint() const noexcept {
        return mUniqueSamplerBindingPoint++;
    }

    uint32_t getUniqueUboBindingPoint() const noexcept {
        return mUniqueUboBindingPoint++;
    }

    uint32_t getUniqueSsboBindingPoint() const noexcept {
        return mUniqueSsboBindingPoint++;
    }

private:
    filament::backend::Precision getDefaultPrecision(ShaderStage stage) const;
    filament::backend::Precision getDefaultUniformPrecision() const;

    std::ostringstream& generateInterfaceFields(std::ostringstream& out,
            std::vector<filament::BufferInterfaceBlock::FieldInfo> const& infos,
            filament::backend::Precision defaultPrecision) const;

    std::ostringstream& generateUboAsPlainUniforms(std::ostringstream& out, ShaderStage stage,
            const filament::BufferInterfaceBlock& uib) const;

    static const char* getUniformPrecisionQualifier(filament::backend::UniformType type,
            filament::backend::Precision precision,
            filament::backend::Precision uniformPrecision,
            filament::backend::Precision defaultPrecision) noexcept;

    // return type name of sampler  (e.g.: "sampler2D")
    char const* getSamplerTypeName(filament::backend::SamplerType type,
            filament::backend::SamplerFormat format, bool multisample) const noexcept;

    // return name of the material property (e.g.: "ROUGHNESS")
    static char const* getConstantName(MaterialBuilder::Property property) noexcept;

    static char const* getPrecisionQualifier(filament::backend::Precision precision) noexcept;

    // return type (e.g.: "vec3", "vec4", "float")
    static char const* getTypeName(UniformType type) noexcept;

    // return type name of uniform Field (e.g.: "vec3", "vec4", "float")
    static char const* getUniformTypeName(filament::BufferInterfaceBlock::FieldInfo const& info) noexcept;

    // return type name of output  (e.g.: "vec3", "vec4", "float")
    static char const* getOutputTypeName(MaterialBuilder::OutputType type) noexcept;

    // return qualifier for the specified interpolation mode
    static char const* getInterpolationQualifier(filament::Interpolation interpolation) noexcept;

    static bool hasPrecision(filament::BufferInterfaceBlock::Type type) noexcept;


    TargetLanguage mTargetLanguage;
  
    mutable uint32_t mUniqueSamplerBindingPoint = 0;
    mutable uint32_t mUniqueUboBindingPoint = 0;
    mutable uint32_t mUniqueSsboBindingPoint = 0;
};

} 