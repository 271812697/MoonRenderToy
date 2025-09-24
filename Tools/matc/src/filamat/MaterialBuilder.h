#pragma once
#include <filaflat/MaterialEnums.h>
#include "IncludeCallback.h"
#include "Package.h"

#include <filaflat/backend/DriverEnums.h>
#include <mutex>
#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <unordered_map>
#include <stddef.h>
#include <stdint.h>

namespace filament {
class BufferInterfaceBlock;
}

namespace filamat {

struct MaterialInfo;
struct Variant;
class ChunkContainer;

class  MaterialBuilderBase {
public:


    enum class ShaderChain : uint8_t
    {
        NONE = 0x0u,
        VERTEXSHADER=0x1u,
        GEOMERTYSHADER=0x02u,
        FRAGMENTSHADER = 0x04u,
        COMPUTESHADER =0x08u,
        COMMONUSE = VERTEXSHADER | FRAGMENTSHADER,
        COMMONSGEOMERTY = VERTEXSHADER | FRAGMENTSHADER | GEOMERTYSHADER
    };
    /*
     * Generally we generate GLSL that will be converted to SPIRV, optimized and then
     * transpiled to the backend's language such as MSL, ESSL300, GLSL410 or SPIRV, in this
     * case the generated GLSL uses ESSL310 or GLSL450 and has Vulkan semantics and
     * TargetLanguage::SPIRV must be used.
     *
     * However, in some cases (e.g. when no optimization is asked) we generate the *final* GLSL
     * directly, this GLSL must be ESSL300 or GLSL410 and cannot use any Vulkan syntax, for this
     * situation we use TargetLanguage::GLSL. In this case TargetApi is guaranteed to be OPENGL.
     *
     * Note that TargetLanguage::GLSL is not the common case, as it is generally not used in
     * release builds.
     *
     * Also note that glslang performs semantics analysis on whichever GLSL ends up being generated.
     */
    enum class TargetLanguage {
        GLSL,           // GLSL with OpenGL 4.1 / OpenGL ES 3.0 semantics
        SPIRV           // GLSL with Vulkan semantics
    };

    enum class Optimization {
        NONE,
        PREPROCESSOR,
        SIZE,
        PERFORMANCE
    };

    /**
     * Initialize MaterialBuilder.
     *
     * init must be called first before building any materials.
     */
    static void init();

    /**
     * Release internal MaterialBuilder resources.
     *
     * Call shutdown when finished building materials to release all internal resources. After
     * calling shutdown, another call to MaterialBuilder::init must precede another material build.
     */
    static void shutdown();

protected:


};



/**
 * MaterialBuilder builds Filament materials from shader code.
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * #include <filamat/MaterialBuilder.h>
 * using namespace filamat;
 *
 * // Must be called before any materials can be built.
 * MaterialBuilder::init();

 * MaterialBuilder builder;
 * builder
 *     .name("My material")
 *     .material("void material (inout MaterialInputs material) {"
 *               "  prepareMaterial(material);"
 *               "  material.baseColor.rgb = float3(1.0, 0.0, 0.0);"
 *               "}")
 *     .shading(MaterialBuilder::Shading::LIT)
 *     .targetApi(MaterialBuilder::TargetApi::ALL)
 *     .platform(MaterialBuilder::Platform::ALL);

 * Package package = builder.build();
 * if (package.isValid()) {
 *     // success!
 * }

 * // Call when finished building all materials to release internal
 * // MaterialBuilder resources.
 * MaterialBuilder::shutdown();
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 *
 * @see filament::Material
 */
class  MaterialBuilder : public MaterialBuilderBase {
public:
    MaterialBuilder();
    ~MaterialBuilder();

    MaterialBuilder(const MaterialBuilder& rhs) = delete;
    MaterialBuilder& operator=(const MaterialBuilder& rhs) = delete;

    MaterialBuilder(MaterialBuilder&& rhs) noexcept = default;
    MaterialBuilder& operator=(MaterialBuilder&& rhs) noexcept = default;

    static constexpr size_t MATERIAL_VARIABLES_COUNT = 5;
    enum class Variable : uint8_t {
        CUSTOM0,
        CUSTOM1,
        CUSTOM2,
        CUSTOM3,
        CUSTOM4, // CUSTOM4 is only available if the vertex attribute `color` is not required.
        // when adding more variables, make sure to update MATERIAL_VARIABLES_COUNT
    };

    using MaterialDomain = filament::MaterialDomain;
    using RefractionMode = filament::RefractionMode;
    using RefractionType = filament::RefractionType;
    using ReflectionMode = filament::ReflectionMode;


    using ShaderQuality = filament::ShaderQuality;
    using BlendingMode = filament::BlendingMode;
    using BlendFunction = filament::backend::BlendFunction;
    using Shading = filament::Shading;
    using Interpolation = filament::Interpolation;

    using TransparencyMode = filament::TransparencyMode;
    using SpecularAmbientOcclusion = filament::SpecularAmbientOcclusion;

    using AttributeType = filament::backend::UniformType;
    using UniformType = filament::backend::UniformType;
    using ConstantType = filament::backend::ConstantType;
    using SamplerType = filament::backend::SamplerType;
    using SubpassType = filament::backend::SubpassType;
    using SamplerFormat = filament::backend::SamplerFormat;
    using ParameterPrecision = filament::backend::Precision;
    using Precision = filament::backend::Precision;
    using CullingMode = filament::backend::CullingMode;
    
    
    using ShaderStage = filament::backend::ShaderStage;
    

    enum class VariableQualifier : uint8_t {
        OUT
    };

    enum class OutputTarget : uint8_t {
        COLOR,
        DEPTH
    };

    enum class OutputType : uint8_t {
        FLOAT,
        FLOAT2,
        FLOAT3,
        FLOAT4
    };

    struct PreprocessorDefine {
        std::string name;
        std::string value;

        PreprocessorDefine(std::string  name, std::string  value) :
                name(std::move(name)), value(std::move(value)) {}
    };
    using PreprocessorDefineList = std::vector<PreprocessorDefine>;


    MaterialBuilder& noSamplerValidation(bool enabled) noexcept;


    //! Set the name of this material.
    MaterialBuilder& name(const char* name) noexcept;

    //! Set the file name of this material file. Used in error reporting.
    MaterialBuilder& fileName(const char* name) noexcept;

    //! Set the shading model.
    MaterialBuilder& shading(Shading shading) noexcept;

    //! Set the interpolation mode.
    MaterialBuilder& interpolation(Interpolation interpolation) noexcept;

    //! Add a parameter (i.e., a uniform) to this material.
    MaterialBuilder& parameter(const char* name, UniformType type,
            ParameterPrecision precision = ParameterPrecision::DEFAULT) noexcept;

    //! Add a parameter array to this material.
    MaterialBuilder& parameter(const char* name, size_t size, UniformType type,
            ParameterPrecision precision = ParameterPrecision::DEFAULT);

    //! Add a constant parameter to this material.
    template<typename T>
    using is_supported_constant_parameter_t = typename std::enable_if<
            std::is_same<int32_t, T>::value ||
            std::is_same<float, T>::value ||
            std::is_same<bool, T>::value>::type;
    template<typename T, typename = is_supported_constant_parameter_t<T>>
    MaterialBuilder& constant(const char *name, ConstantType type, T defaultValue = 0);

    /**
     * Add a sampler parameter to this material.
     *
     * When SamplerType::SAMPLER_EXTERNAL is specified, format and precision are ignored.
     */
    MaterialBuilder& parameter(const char* name, SamplerType samplerType, SamplerFormat format = SamplerFormat::FLOAT,
                               ParameterPrecision precision = ParameterPrecision::DEFAULT, bool unfilterable = false,
                               bool multisample = false, const char* transformName = "",
                              
                               bool preLoad = false, filament::backend::ShaderStageFlags stage = filament::backend::ShaderStageFlags::ALL_SHADER_STAGE_FLAGS);

    MaterialBuilder& buffer(filament::BufferInterfaceBlock bib);

    //! Custom variables (all float4).
    MaterialBuilder& variable(Variable v, const char* name) noexcept;

    MaterialBuilder& variable(Variable v, const char* name,
            ParameterPrecision precision) noexcept;


    //! Specify the domain that this material will operate in.
    MaterialBuilder& materialDomain(MaterialBuilder::MaterialDomain materialDomain) noexcept;

    /**
     * Set the code content of this material.
     *
     * Surface Domain
     * --------------
     *
     * Materials in the SURFACE domain must declare a function:
     * ~~~~~
     * void material(inout MaterialInputs material) {
     *     prepareMaterial(material);
     *     material.baseColor.rgb = float3(1.0, 0.0, 0.0);
     * }
     * ~~~~~
     * this function *must* call `prepareMaterial(material)` before it returns.
     *
     * Post-process Domain
     * -------------------
     *
     * Materials in the POST_PROCESS domain must declare a function:
     * ~~~~~
     * void postProcess(inout PostProcessInputs postProcess) {
     *     postProcess.color = float4(1.0);
     * }
     * ~~~~~
     *
     * @param code The source code of the material.
     * @param line The line number offset of the material, where 0 is the first line. Used for error
     *             reporting
     */
    MaterialBuilder& material(const char* code, size_t line = 0) noexcept;

    /**
     * Set the callback used for resolving include directives.
     * The default is no callback, which disallows all includes.
     */
    MaterialBuilder& includeCallback(IncludeCallback callback) noexcept;

    /**
     * Set the vertex code content of this material.
     *
     * Surface Domain
     * --------------
     *
     * Materials in the SURFACE domain must declare a function:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * void materialVertex(inout MaterialVertexInputs material) {
     *
     * }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     *
     * Post-process Domain
     * -------------------
     *
     * Materials in the POST_PROCESS domain must declare a function:
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
     * void postProcessVertex(inout PostProcessVertexInputs postProcess) {
     *
     * }
     * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

     * @param code The source code of the material.
     * @param line The line number offset of the material, where 0 is the first line. Used for error
     *             reporting
     */
    MaterialBuilder& materialVertex(const char* code, size_t line = 0) noexcept;
    MaterialBuilder& materialGeomerty(const char* code, size_t line = 0) noexcept;

    MaterialBuilder& quality(ShaderQuality quality) noexcept;

  

    /**
     * Set the blending mode for this material. When set to MASKED, alpha to coverage is turned on.
     * You can override this behavior using alphaToCoverage(false).
     */
    MaterialBuilder& blending(BlendingMode blending) noexcept;

    /**
     * Set the blend function  for this material. blending must be et to CUSTOM.
     */
    MaterialBuilder& customBlendFunctions(
            BlendFunction srcRGB,
            BlendFunction srcA,
            BlendFunction dstRGB,
            BlendFunction dstA) noexcept;

    /**
     * Set the blending mode of the post-lighting color for this material.
     * Only OPAQUE, TRANSPARENT and ADD are supported, the default is TRANSPARENT.
     * This setting requires the material properties "postLightingColor" and
     * "postLightingMixFactor" to be set.
     */
    MaterialBuilder& postLightingBlending(BlendingMode blending) noexcept;

    /**
     * How triangles are culled by default (doesn't affect points or lines, BACK by default).
     * Material instances can override this.
     */
    MaterialBuilder& culling(CullingMode culling) noexcept;

    //! Enable / disable color-buffer write (enabled by default, material instances can override).
    MaterialBuilder& colorWrite(bool enable) noexcept;

    //! Enable / disable depth-buffer write (enabled by default for opaque, disabled for others, material instances can override).
    MaterialBuilder& depthWrite(bool enable) noexcept;

    //! Enable / disable depth based culling (enabled by default, material instances can override).
    MaterialBuilder& depthCulling(bool enable) noexcept;

    //! Enable / disable instanced primitives (disabled by default).
    MaterialBuilder& instanced(bool enable) noexcept;

    /**
     * Double-sided materials don't cull faces, equivalent to culling(CullingMode::NONE).
     * doubleSided() overrides culling() if called.
     * When called with "false", this enables the capability for a run-time toggle.
     */
    MaterialBuilder& doubleSided(bool doubleSided) noexcept;

    /**
     * Any fragment with an alpha below this threshold is clipped (MASKED blending mode only).
     * The mask threshold can also be controlled by using the float material parameter called
     * `_maskThreshold`, or by calling
     * @ref filament::MaterialInstance::setMaskThreshold "MaterialInstance::setMaskThreshold".
     */
    MaterialBuilder& maskThreshold(float threshold) noexcept;

    /**
     * Enables or disables alpha-to-coverage. When enabled, the coverage of a fragment is based
     * on its alpha value. This parameter is only useful when MSAA is in use. Alpha to coverage
     * is enabled automatically when the blend mode is set to MASKED; this behavior can be
     * overridden by calling alphaToCoverage(false).
     */
    MaterialBuilder& alphaToCoverage(bool enable) noexcept;

    //! The material output is multiplied by the shadowing factor (UNLIT model only).
    MaterialBuilder& shadowMultiplier(bool shadowMultiplier) noexcept;

    //! This material casts transparent shadows. The blending mode must be TRANSPARENT or FADE.
    MaterialBuilder& transparentShadow(bool transparentShadow) noexcept;

    /**
     * Reduces specular aliasing for materials that have low roughness. Turning this feature on also
     * helps preserve the shapes of specular highlights as an object moves away from the camera.
     * When turned on, two float material parameters are added to control the effect:
     * `_specularAAScreenSpaceVariance` and `_specularAAThreshold`. You can also use
     * @ref filament::MaterialInstance::setSpecularAntiAliasingVariance
     * "MaterialInstance::setSpecularAntiAliasingVariance" and
     * @ref filament::MaterialInstance::setSpecularAntiAliasingThreshold
     * "setSpecularAntiAliasingThreshold"
     *
     * Disabled by default.
     */
    MaterialBuilder& specularAntiAliasing(bool specularAntiAliasing) noexcept;

    /**
     * Sets the screen-space variance of the filter kernel used when applying specular
     * anti-aliasing. The default value is set to 0.15. The specified value should be between 0 and
     * 1 and will be clamped if necessary.
     */
    MaterialBuilder& specularAntiAliasingVariance(float screenSpaceVariance) noexcept;

    /**
     * Sets the clamping threshold used to suppress estimation errors when applying specular
     * anti-aliasing. The default value is set to 0.2. The specified value should be between 0 and 1
     * and will be clamped if necessary.
     */
    MaterialBuilder& specularAntiAliasingThreshold(float threshold) noexcept;

    /**
     * Enables or disables the index of refraction (IoR) change caused by the clear coat layer when
     * present. When the IoR changes, the base color is darkened. Disabling this feature preserves
     * the base color as initially specified.
     *
     * Enabled by default.
     */
    MaterialBuilder& clearCoatIorChange(bool clearCoatIorChange) noexcept;

    //! Enable / disable flipping of the Y coordinate of UV attributes, enabled by default.
    MaterialBuilder& flipUV(bool flipUV) noexcept;

    //! Enable / disable multi-bounce ambient occlusion, disabled by default on mobile.
    MaterialBuilder& multiBounceAmbientOcclusion(bool multiBounceAO) noexcept;

    //! Set the specular ambient occlusion technique. Disabled by default on mobile.
    MaterialBuilder& specularAmbientOcclusion(SpecularAmbientOcclusion specularAO) noexcept;

    //! Specify the refraction
    MaterialBuilder& refractionMode(RefractionMode refraction) noexcept;

    //! Specify the refraction type
    MaterialBuilder& refractionType(RefractionType refractionType) noexcept;

    //! Specifies how reflections should be rendered (default is DEFAULT).
    MaterialBuilder& reflectionMode(ReflectionMode mode) noexcept;

    //! Specifies how transparent objects should be rendered (default is DEFAULT).
    MaterialBuilder& transparencyMode(TransparencyMode mode) noexcept;






    /**
     * Build the material. If you are using the Filament engine with this library, you should use
     * the job system provided by Engine.
     */
    Package build();
    std::string getPass();
    void setPass(const std::string& pass);

public:

    // The methods and types below are for internal use
    /// @cond never



    struct Parameter {
        Parameter() noexcept: parameterType(INVALID) {}

        // Sampler
        Parameter(const char* paramName, SamplerType t, SamplerFormat f, ParameterPrecision p, bool unfilterable,
            bool ms, const char* tn, bool preLoad, filament::backend::ShaderStageFlags flag)
            : name(paramName)
            , size(1)
            , precision(p)
            , samplerType(t)
            , format(f)
            , unfilterable(unfilterable)
            , multisample(ms)
            , transformName(tn)
         
            , isPreLoaded(preLoad)
            , parameterType(SAMPLER), stageFlag(flag)
        {
        }
        Parameter(const char* paramName, SamplerType t, SamplerFormat f, ParameterPrecision p,
            bool unfilterable, bool ms, const char* tn, filament::backend::ShaderStageFlags flag)
            : name(paramName),
              size(1),
              precision(p),
              samplerType(t),
              format(f),
              unfilterable(unfilterable),
              multisample(ms),
              transformName(tn),
            
              isPreLoaded(false),
              parameterType(SAMPLER),stageFlag(flag) {}

        // Uniform
        Parameter(const char* paramName, UniformType t, size_t typeSize, ParameterPrecision p)
            : name(paramName),
              size(typeSize),
              uniformType(t),
              precision(p), format{SamplerFormat::INT}
            , isPreLoaded(false)
            ,
              unfilterable(false),
              multisample(false),
              parameterType(UNIFORM) {}

        // Subpass
        Parameter(const char* paramName, SubpassType t, SamplerFormat f, ParameterPrecision p)
            : name(paramName),
              size(1),
              precision(p),
              subpassType(t),
              format(f),
              unfilterable(false),
              multisample(false),
              isPreLoaded(false),
              parameterType(SUBPASS) {}

        std::string name;
        size_t size;
        UniformType uniformType;
        ParameterPrecision precision;
        SamplerType samplerType;
        SubpassType subpassType;
        SamplerFormat format;
        //this is useful for sampler2D
        filament::backend::ShaderStageFlags stageFlag;
        bool unfilterable;
        bool multisample;
        bool isPreLoaded;
        std::string transformName;
     
        enum {
            INVALID,
            UNIFORM,
            SAMPLER,
            SUBPASS
        } parameterType;

        bool isSampler() const { return parameterType == SAMPLER; }
        bool isUniform() const { return parameterType == UNIFORM; }
        bool isSubpass() const { return parameterType == SUBPASS; }
    };

 
    static constexpr size_t MATERIAL_PROPERTIES_COUNT = filament::MATERIAL_PROPERTIES_COUNT;
    using Property = filament::Property;

    using PropertyList = bool[MATERIAL_PROPERTIES_COUNT];


    static constexpr size_t MAX_COLOR_OUTPUT = 8;
    static constexpr size_t MAX_DEPTH_OUTPUT = 1;




    // Returns true if any of the parameter samplers matches the specified type.
    bool hasSamplerType(SamplerType samplerType) const noexcept;

    static constexpr size_t MAX_PARAMETERS_COUNT = 48;
    static constexpr size_t MAX_SUBPASS_COUNT = 1;
    static constexpr size_t MAX_BUFFERS_COUNT = 4;
    using ParameterList = Parameter[MAX_PARAMETERS_COUNT];
    using SubpassList = Parameter[MAX_SUBPASS_COUNT];
    using BufferList = std::vector<std::unique_ptr<filament::BufferInterfaceBlock>>;

    // returns the number of parameters declared in this material
    uint8_t getParameterCount() const noexcept { return mParameterCount; }

    // returns a list of at least getParameterCount() parameters
    const ParameterList& getParameters() const noexcept { return mParameters; }

    // returns the number of parameters declared in this material
    uint8_t getSubpassCount() const noexcept { return mSubpassCount; }

    // returns a list of at least getParameterCount() parameters
    const SubpassList& getSubPasses() const noexcept { return mSubpasses; }

    class ShaderCode
    {
    public:
        ShaderCode(const std::string& str, filament::backend::ShaderStage stage)
            : mCode(str)
            , mStage(stage)
            , mLineOffset(0)
            , mIncludesResolved(false)
        {
            
        }
        ShaderCode() = default;
        void setLineOffset(size_t offset) noexcept
        {
            mLineOffset = offset;
        }
        void setUnresolved(const std::string& code) noexcept
        {
            mIncludesResolved = false;
            mCode = code;
        }
        void setStage(const filament::backend::ShaderStage& stage)
        {
            mStage = stage;
        }
        // Resolve all the #include directives, returns true if successful.
        bool resolveIncludes(IncludeCallback callback, const std::string& fileName) noexcept;

        const std::string& getResolved() const noexcept
        {
            assert(mIncludesResolved);
            return mCode;
        }

        size_t getLineOffset() const noexcept
        {
            return mLineOffset;
        }
        filament::backend::ShaderStage getStage()const
        {
            return mStage;
        }

    private:
        filament::backend::ShaderStage mStage;
        std::string mCode;
        size_t mLineOffset = 0;
        bool mIncludesResolved = false;
    };
    class PassInfo
    {
    public:

        PassInfo(const std::string& tag, const std::unordered_map<std::string, std::vector<std::string>>& features, const std::vector<ShaderCode>& codes)
            : mTag(tag)
            , mShaderFeatures(features)
            , mCodes(codes)
        {
        }
    private:
        friend MaterialBuilder;
        std::string mTag;
        std::unordered_map<std::string, std::vector<std::string>> mShaderFeatures;
        std::vector<ShaderCode> mCodes;
    };

    void addPassInfo(const std::string& tag, const std::unordered_map<std::string, std::vector<std::string>>&features, const std::vector<ShaderCode>& codes);
private:
 

    void prepareToBuild(MaterialInfo& info) noexcept;

   

    void writeCommonChunks(ChunkContainer& container, MaterialInfo& info) const noexcept;
    void writeSurfaceChunks(ChunkContainer& container) const noexcept;

    bool generateShaders( std::vector<PassInfo>& passes,
        ChunkContainer& container, const MaterialInfo& info) const noexcept;

    std::string mMaterialName;
    std::string mFileName;


    std::vector<PassInfo> mPassCodes;



    ShaderCode mMaterialFragmentCode;
    ShaderCode mMaterialGeomertyCode;
    ShaderCode mMaterialVertexCode;


    IncludeCallback mIncludeCallback = nullptr;

    PropertyList mProperties;
    ParameterList mParameters;

    SubpassList mSubpasses;

    BufferList mBuffers;

    ShaderQuality mShaderQuality = ShaderQuality::DEFAULT;
   
    BlendingMode mBlendingMode = BlendingMode::OPAQUE;
    BlendingMode mPostLightingBlendingMode = BlendingMode::TRANSPARENT;
    std::array<BlendFunction, 4> mCustomBlendFunctions = {};
    CullingMode mCullingMode = CullingMode::BACK;
    Shading mShading = Shading::LIT;
    MaterialDomain mMaterialDomain = MaterialDomain::SURFACE;
    RefractionMode mRefractionMode = RefractionMode::NONE;
    RefractionType mRefractionType = RefractionType::SOLID;
    ReflectionMode mReflectionMode = ReflectionMode::DEFAULT;
    Interpolation mInterpolation = Interpolation::SMOOTH;
   
    TransparencyMode mTransparencyMode = TransparencyMode::DEFAULT;
    ShaderChain mShaderChain = ShaderChain::NONE;
  
    uint8_t mStereoscopicEyeCount = 2;

  

    float mMaskThreshold = 0.4f;
    float mSpecularAntiAliasingVariance = 0.15f;
    float mSpecularAntiAliasingThreshold = 0.2f;

    

    bool mShadowMultiplier = false;
    bool mTransparentShadow = false;

    uint8_t mParameterCount = 0;
    uint8_t mSubpassCount = 0;

    bool mDoubleSided = false;
    bool mDoubleSidedCapability = false;
    bool mColorWrite = true;
    bool mDepthTest = true;
    bool mInstanced = false;
    bool mDepthWrite = true;
    bool mDepthWriteSet = false;
    bool mAlphaToCoverage = false;
    bool mAlphaToCoverageSet = false;

    bool mSpecularAntiAliasing = false;
    bool mClearCoatIorChange = true;

    bool mFlipUV = true;

    bool mMultiBounceAO = false;
    bool mMultiBounceAOSet = false;

    SpecularAmbientOcclusion mSpecularAO = SpecularAmbientOcclusion::NONE;
    bool mSpecularAOSet = false;

    bool mCustomSurfaceShading = false;
    bool mHasGeomertyShading = false;

    bool mEnableFramebufferFetch = false;

    bool mVertexDomainDeviceJittered = false;

    bool mUseLegacyMorphing = false;

    PreprocessorDefineList mDefines;

  
    bool mNoSamplerValidation = false;

    std::string mPass;
};

} // namespace filamat


