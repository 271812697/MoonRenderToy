
#include "MaterialBuilder.h"
#include <filamat/Enums.h>
#include <filamat/IncludeCallback.h>
#include <filamat/Package.h>

#include "Includes.h"
#include "shaders/MaterialInfo.h"
#include "shaders/ShaderGenerator.h"
#include "eiff/BlobDictionary.h"
#include "eiff/ChunkContainer.h"

#include "eiff/DictionaryTextChunk.h"
#include "eiff/LineDictionary.h"
#include "eiff/ShaderPassInfoChunk.h"
#include "eiff/MaterialInterfaceBlockChunk.h"
#include "eiff/MaterialTextChunk.h"
#include "eiff/ShaderEntry.h"
#include "filaflat//BufferInterfaceBlock.h"
#include "filaflat/DescriptorSets.h"
#include "filaflat/SamplerInterfaceBlock.h"
#include "filaflat/MaterialChunkType.h"
#include "filaflat/MaterialEnums.h"
#include "filaflat/backend/DriverEnums.h"
#include <algorithm>
#include <atomic>
#include <fstream>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <xhash>

namespace filamat {
	using namespace filament;
	MaterialBuilder::MaterialBuilder() : mMaterialName("Unnamed") {
		std::fill_n(mProperties, MATERIAL_PROPERTIES_COUNT, false);

	}

	MaterialBuilder::~MaterialBuilder() = default;
	void MaterialBuilderBase::init() {
	}

	void MaterialBuilderBase::shutdown() {
	}

	MaterialBuilder& MaterialBuilder::name(const char* name) noexcept {
		mMaterialName = std::string(name);
		return *this;
	}

	MaterialBuilder& MaterialBuilder::fileName(const char* name) noexcept {
		mFileName = std::string(name);
		return *this;
	}

	MaterialBuilder& MaterialBuilder::material(const char* code, size_t const line) noexcept {
		mMaterialFragmentCode.setUnresolved(std::string(code));
		mMaterialFragmentCode.setLineOffset(line);
        mShaderChain = static_cast<ShaderChain>(
            (static_cast<uint8_t>(mShaderChain) | static_cast<uint8_t>(ShaderChain::FRAGMENTSHADER)));
		return *this;
	}

	MaterialBuilder& MaterialBuilder::includeCallback(IncludeCallback callback) noexcept {
		mIncludeCallback = std::move(callback);
		return *this;
	}

	MaterialBuilder& MaterialBuilder::materialVertex(const char* code, size_t const line) noexcept {
		mMaterialVertexCode.setUnresolved(std::string(code));
		mMaterialVertexCode.setLineOffset(line);
        mShaderChain = static_cast<ShaderChain>(
            (static_cast<uint8_t>(mShaderChain) | static_cast<uint8_t>(ShaderChain::VERTEXSHADER)));
		return *this;
	}
    MaterialBuilder& MaterialBuilder::materialGeomerty(const char* code, size_t const line) noexcept
    {
        mHasGeomertyShading = true;
        mMaterialGeomertyCode.setUnresolved(std::string(code));
        mMaterialGeomertyCode.setLineOffset(line);
        mShaderChain = static_cast<ShaderChain>(
            (static_cast<uint8_t>(mShaderChain) | static_cast<uint8_t>(ShaderChain::GEOMERTYSHADER)));
        return *this;
    }
	MaterialBuilder& MaterialBuilder::shading(Shading const shading) noexcept {
		mShading = shading;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::interpolation(Interpolation const interpolation) noexcept {
		mInterpolation = interpolation;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::variable(Variable v, const char* name) noexcept {

		return *this;
	}

	MaterialBuilder& MaterialBuilder::variable(Variable v,
		const char* name, ParameterPrecision const precision) noexcept {

		return *this;
	}

	MaterialBuilder& MaterialBuilder::parameter(const char* name, size_t size, UniformType type,
		ParameterPrecision precision) {
        assert(mParameterCount < MAX_PARAMETERS_COUNT && "Too many parameters");
		mParameters[mParameterCount++] = { name, type, size, precision };
		return *this;
	}

	MaterialBuilder& MaterialBuilder::parameter(const char* name, UniformType const type,
		ParameterPrecision const precision) noexcept {
		return parameter(name, 1, type, precision);
	}

	MaterialBuilder& MaterialBuilder::parameter(const char* name, SamplerType samplerType,
		SamplerFormat format, ParameterPrecision precision, bool unfilterable, bool multisample, const char* transformName,
        bool preLoad, filament::backend::ShaderStageFlags stage)
    {
		

		assert(mParameterCount < MAX_PARAMETERS_COUNT&&"Too many parameters");
       
		mParameters[mParameterCount++] = { name, samplerType, format, precision, unfilterable,
			multisample, transformName ,preLoad,stage};
		return *this;
	}



	MaterialBuilder& MaterialBuilder::buffer(BufferInterfaceBlock bib) {
		assert(mBuffers.size() < MAX_BUFFERS_COUNT&&"Too many buffers");
		mBuffers.emplace_back(std::make_unique<BufferInterfaceBlock>(std::move(bib)));
		return *this;
	}

	MaterialBuilder& MaterialBuilder::materialDomain(
		MaterialDomain const materialDomain) noexcept {
		mMaterialDomain = materialDomain;
		if (mMaterialDomain == MaterialDomain::COMPUTE) {

		}
		return *this;
	}

	MaterialBuilder& MaterialBuilder::refractionMode(RefractionMode const refraction) noexcept {
		mRefractionMode = refraction;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::refractionType(RefractionType const refractionType) noexcept {
		mRefractionType = refractionType;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::quality(ShaderQuality const quality) noexcept {
		mShaderQuality = quality;
		return *this;
	}



	MaterialBuilder& MaterialBuilder::blending(BlendingMode const blending) noexcept {
		mBlendingMode = blending;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::customBlendFunctions(
		BlendFunction const srcRGB, BlendFunction const srcA,
		BlendFunction const dstRGB, BlendFunction const dstA) noexcept {
		mCustomBlendFunctions[0] = srcRGB;
		mCustomBlendFunctions[1] = srcA;
		mCustomBlendFunctions[2] = dstRGB;
		mCustomBlendFunctions[3] = dstA;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::postLightingBlending(BlendingMode const blending) noexcept {
		mPostLightingBlendingMode = blending;
		return *this;
	}


	MaterialBuilder& MaterialBuilder::culling(CullingMode const culling) noexcept {
		mCullingMode = culling;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::colorWrite(bool const enable) noexcept {
		mColorWrite = enable;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::depthWrite(bool const enable) noexcept {
		mDepthWrite = enable;
		mDepthWriteSet = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::depthCulling(bool const enable) noexcept {
		mDepthTest = enable;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::instanced(bool const enable) noexcept {
		mInstanced = enable;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::doubleSided(bool const doubleSided) noexcept {
		mDoubleSided = doubleSided;
		mDoubleSidedCapability = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::maskThreshold(float const threshold) noexcept {
		mMaskThreshold = threshold;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::alphaToCoverage(bool const enable) noexcept {
		mAlphaToCoverage = enable;
		mAlphaToCoverageSet = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::shadowMultiplier(bool const shadowMultiplier) noexcept {
		mShadowMultiplier = shadowMultiplier;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::transparentShadow(bool const transparentShadow) noexcept {
		mTransparentShadow = transparentShadow;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::specularAntiAliasing(bool const specularAntiAliasing) noexcept {
		mSpecularAntiAliasing = specularAntiAliasing;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::specularAntiAliasingVariance(float const screenSpaceVariance) noexcept {
		mSpecularAntiAliasingVariance = screenSpaceVariance;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::specularAntiAliasingThreshold(float const threshold) noexcept {
		mSpecularAntiAliasingThreshold = threshold;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::clearCoatIorChange(bool const clearCoatIorChange) noexcept {
		mClearCoatIorChange = clearCoatIorChange;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::flipUV(bool const flipUV) noexcept {
		mFlipUV = flipUV;
		return *this;
	}


	MaterialBuilder& MaterialBuilder::multiBounceAmbientOcclusion(bool const multiBounceAO) noexcept {
		mMultiBounceAO = multiBounceAO;
		mMultiBounceAOSet = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::specularAmbientOcclusion(SpecularAmbientOcclusion const specularAO) noexcept {
		mSpecularAO = specularAO;
		mSpecularAOSet = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::transparencyMode(TransparencyMode const mode) noexcept {
		mTransparencyMode = mode;
		return *this;
	}




	MaterialBuilder& MaterialBuilder::reflectionMode(ReflectionMode const mode) noexcept {
		mReflectionMode = mode;
		return *this;
	}







	bool MaterialBuilder::hasSamplerType(SamplerType const samplerType) const noexcept {
		for (size_t i = 0, c = mParameterCount; i < c; i++) {
			auto const& param = mParameters[i];
			if (param.isSampler() && param.samplerType == samplerType) {
				return  true;
			}
		}
		return false;
	}

	void MaterialBuilder::prepareToBuild(MaterialInfo& info) noexcept {
		// Build the per-material sampler block and uniform block.
		SamplerInterfaceBlock::Builder sbb;
		BufferInterfaceBlock::Builder ibb;
		// sampler bindings start at 1, 0 is the ubo

		uint16_t binding = 1;
		for (size_t i = 0, c = mParameterCount; i < c; i++) {
			auto const& param = mParameters[i];
			assert(!param.isSubpass());
			
			if (param.isSampler()) {
				sbb.add({ param.name.data(), param.name.size() }, binding, param.samplerType,
					param.format, param.precision, param.unfilterable, param.multisample,
					param.isPreLoaded,param.stageFlag);
				binding++;
			}
			else if (param.isUniform()) {
				ibb.add({ {{ param.name.data(), param.name.size() },
						  uint32_t(param.size == 1u ? 0u : param.size), param.uniformType,
						  param.precision} });
			}
		}
        for( size_t i = 0, c = mParameterCount; i < c; i++ )
        {
            auto const& param = mParameters[i];

            if( param.isSampler() )
            {
                std::string hasTex = "HAS_" + param.name;
                ibb.add({{{hasTex.data(), hasTex.size()},
                    uint32_t(0),
                    UniformType::BOOL,
                    ParameterPrecision::DEFAULT}});
            }
        }

		for (auto const& buffer : mBuffers) {
			info.buffers.emplace_back(buffer.get());
		}

		info.sib = sbb.name("MaterialParams").build();
		info.uib = ibb.name("MaterialParams").build();
		info.hasDoubleSidedCapability = mDoubleSidedCapability;
		info.hasExternalSamplers = hasSamplerType(SamplerType::SAMPLER_EXTERNAL);
		info.has3dSamplers = hasSamplerType(SamplerType::SAMPLER_3D);
		info.specularAntiAliasing = mSpecularAntiAliasing;
		info.clearCoatIorChange = mClearCoatIorChange;
		info.flipUV = mFlipUV;
		
		info.blendingMode = mBlendingMode;
		info.postLightingBlendingMode = mPostLightingBlendingMode;
		info.shading = mShading;
		info.hasShadowMultiplier = mShadowMultiplier;
		info.hasTransparentShadow = mTransparentShadow;
		info.multiBounceAO = mMultiBounceAO;
		info.multiBounceAOSet = mMultiBounceAOSet;
		info.specularAO = mSpecularAO;
		info.specularAOSet = mSpecularAOSet;
		info.refractionMode = mRefractionMode;
		info.refractionType = mRefractionType;
		info.reflectionMode = mReflectionMode;
		info.quality = mShaderQuality;
		info.hasCustomSurfaceShading = mCustomSurfaceShading;
		info.useLegacyMorphing = mUseLegacyMorphing;
		info.instanced = mInstanced;
		info.vertexDomainDeviceJittered = mVertexDomainDeviceJittered;
		
		
		
		info.stereoscopicEyeCount = mStereoscopicEyeCount;

		// This is determined via static analysis of the glsl after prepareToBuild().
		info.userMaterialHasCustomDepth = false;
        info.passInfo.clear();
        for( int i = 0; i < mPassCodes.size(); i++ )
        {
            info.passInfo.addPass(mPassCodes[i].mTag, mPassCodes[i].mShaderFeatures);
		}
		
	}

	bool MaterialBuilder::ShaderCode::resolveIncludes(IncludeCallback callback,
		const std::string& fileName) noexcept {
		if (!mCode.empty()) {
			ResolveOptions const options{
					 false,
					 true,
					 true
			};
			IncludeResult source {
               fileName, 
			   mCode,  
			   getLineOffset(), 
			   std::string("")
			};
			if (!filamat::resolveIncludes(source, std::move(callback), options)) {
				return false;
			}
			mCode = source.text;
		}

		mIncludesResolved = true;
		return true;
	}

    bool MaterialBuilder::generateShaders( std::vector<PassInfo>& passes,
        ChunkContainer& container, const MaterialInfo& info) const noexcept
    {
        std::vector<TextEntry> glslEntries;
        LineDictionary textDictionary;

        for(  auto& pass : passes )
        {
            int passIndex = &pass - &passes[0];
			ShaderGenerator sg;
			for(int i=0;i<pass.mCodes.size();i++)
			{
                if( pass.mCodes[i].getStage() == filament::backend::ShaderStage::VERTEX )
                {
                    sg.setUpVertexCode(pass.mCodes[i].getResolved(), pass.mCodes[i].getLineOffset());
				}
                if( pass.mCodes[i].getStage() == filament::backend::ShaderStage::FRAGMENT )
                {
                    sg.setUpFragmentCode(pass.mCodes[i].getResolved(), pass.mCodes[i].getLineOffset());
                }
                if( pass.mCodes[i].getStage() == filament::backend::ShaderStage::GEOMERTY )
                {
                    sg.setUpGeomertyCode(pass.mCodes[i].getResolved(), pass.mCodes[i].getLineOffset());
                }      
			}

            TextEntry glslEntry{};
		    glslEntry.passTag = passIndex;
			for( int k = 0; k < pass.mCodes.size(); k++ )
			{ 
				
				glslEntry.stage = pass.mCodes[k].getStage();
                
				std::string featureStage = "";
				if( pass.mCodes[k].getStage() == filament::backend::ShaderStage::VERTEX )
				{
					featureStage = "vertex";
				}
				if( pass.mCodes[k].getStage() == filament::backend::ShaderStage::FRAGMENT )
				{
					featureStage =  "fragment";
				}
				if( pass.mCodes[k].getStage() == filament::backend::ShaderStage::GEOMERTY )
				{
					featureStage =  "geomerty";
				}
				std::vector<std::string>& features =pass.mShaderFeatures[featureStage];
				int shaderCount = (1 << features.size());
				for( int i = 0; i < shaderCount; i++ )
				{
					std::string defineStr = "";
					glslEntry.variant = i;
					for( int j = 0; j < features.size(); j++ )
					{
						if( i & (1 << j) )
						{
							defineStr += "#define " + features[j] + "\n";
						}
					}
					if( pass.mCodes[k].getStage() == filament::backend::ShaderStage::VERTEX )
					{
						glslEntry.shader = sg.createSurfaceVertexProgram(info, defineStr);
					}
					if( pass.mCodes[k].getStage() == filament::backend::ShaderStage::FRAGMENT )
					{
						glslEntry.shader = sg.createSurfaceFragmentProgram(info, defineStr);
					}
					if( pass.mCodes[k].getStage() == filament::backend::ShaderStage::GEOMERTY )
					{
						glslEntry.shader = sg.createSurfaceGeomertyProgram(info, defineStr);
					}

					glslEntries.push_back(glslEntry);
				}

			}
        }
        

        auto compare = [](const auto& a, const auto& b)
        {
            static_assert(sizeof(decltype(a.variant)) == 1);
            static_assert(sizeof(decltype(b.variant)) == 1);
            const uint32_t akey = (uint32_t(a.passTag) << 16) | (uint32_t(a.variant) << 8) | uint32_t(a.stage);
            const uint32_t bkey = (uint32_t(b.passTag) << 16) | (uint32_t(b.variant) << 8) | uint32_t(b.stage);
            return akey < bkey;
        };
        std::sort(glslEntries.begin(), glslEntries.end(), compare);
        // Generate the dictionaries.
        for( const auto& s : glslEntries )
        {
            textDictionary.addText(s.shader);
        }
        // Emit dictionary chunk (TextDictionaryReader and DictionaryTextChunk)
        const auto& dictionaryChunk = container.push<DictionaryTextChunk>(std::move(textDictionary), DictionaryText);
        // Emit GLSL chunk (MaterialTextChunk).
        if( !glslEntries.empty() )
        {
            container.push<MaterialTextChunk>(std::move(glslEntries), dictionaryChunk.getDictionary(), MaterialGlsl);
        }
        return true;
    }


    void MaterialBuilder::setPass(const std::string& pass)
    {
        mPass=pass;
	}
    std::string MaterialBuilder::getPass(){
        return mPass;
	}
	Package MaterialBuilder::build() {

        for( auto& it: mPassCodes )
        {
            for( auto& itcode : it.mCodes )
            {
                if( !itcode.resolveIncludes(mIncludeCallback, mFileName) )
                {
                    return Package::invalidPackage();
                }
			}
		}
		// prepareToBuild must be called first, to populate mCodeGenPermutations.
		MaterialInfo info{};
		prepareToBuild(info);

		// Create chunk tree.
		ChunkContainer container;
		writeCommonChunks(container, info);
		if (mMaterialDomain == MaterialDomain::SURFACE) {
			writeSurfaceChunks(container);
		}
		
		info.useLegacyMorphing = mUseLegacyMorphing;       
        bool const success = generateShaders( mPassCodes, container,info);
		if (!success) {
            return Package::invalidPackage();
		}
		// Flatten all chunks in the container into a Package.
		Package package(container.getSize());
		Flattener f{ package.getData() };
		container.flatten(f);
		return package;
	}

	using namespace backend;
    static const char* ShaderStageFlagsto_string(ShaderStageFlags const stageFlags) noexcept
    {
		switch (stageFlags) {
		case ShaderStageFlags::NONE:                    return "{ }";
		case ShaderStageFlags::VERTEX:                  return "{ vertex }";
		case ShaderStageFlags::FRAGMENT:                return "{ fragment }";
		case ShaderStageFlags::COMPUTE:                 return "{ compute }";
		case ShaderStageFlags::ALL_SHADER_STAGE_FLAGS:  return "{ vertex | fragment | COMPUTE }";
		}
		return nullptr;
	}


    void MaterialBuilder::addPassInfo(const std::string& tag, const std::unordered_map<std::string, std::vector<std::string>>& features, const std::vector<ShaderCode>& codes)
    {
        mPassCodes.push_back({tag, features, codes});
    }




	void MaterialBuilder::writeCommonChunks(ChunkContainer& container, MaterialInfo& info) const noexcept {
		container.emplace<uint32_t>(MaterialVersion, MATERIAL_VERSION);
		container.emplace<const char*>(MaterialName, mMaterialName.c_str());
        container.emplace<const char*>(MaterialPass,mPass.c_str());

		container.emplace<uint8_t>(ChunkType::MaterialDomain, static_cast<uint8_t>(mMaterialDomain));
		
		using namespace filament;

		container.push<ShaderPassInfoChunk>(info.passInfo);
		// User parameters (UBO)
		container.push<MaterialUniformInterfaceBlockChunk>(info.uib);
		// User texture parameters
		container.push<MaterialSamplerInterfaceBlockChunk>(info.sib);
		// Descriptor layout and descriptor name/binding mapping
		container.push<MaterialDescriptorBindingsChuck>(info.sib);
		container.push<MaterialDescriptorSetLayoutChunk>(info.sib);

		if (mMaterialDomain != MaterialDomain::COMPUTE) {


			container.emplace<bool>(MaterialDoubleSidedSet, mDoubleSidedCapability);
			container.emplace<bool>(MaterialDoubleSided, mDoubleSided);
			container.emplace<uint8_t>(MaterialBlendingMode,
				static_cast<uint8_t>(mBlendingMode));

			if (mBlendingMode == BlendingMode::CUSTOM) {
				uint32_t const blendFunctions =
					(uint32_t(mCustomBlendFunctions[0]) << 24) |
					(uint32_t(mCustomBlendFunctions[1]) << 16) |
					(uint32_t(mCustomBlendFunctions[2]) << 8) |
					(uint32_t(mCustomBlendFunctions[3]) << 0);
				container.emplace< uint32_t >(MaterialBlendFunction, blendFunctions);
			}

			container.emplace<uint8_t>(MaterialTransparencyMode,
				static_cast<uint8_t>(mTransparencyMode));
			container.emplace<uint8_t>(MaterialReflectionMode,
				static_cast<uint8_t>(mReflectionMode));
			container.emplace<bool>(MaterialColorWrite, mColorWrite);
			container.emplace<bool>(MaterialDepthWriteSet, mDepthWriteSet);
			container.emplace<bool>(MaterialDepthWrite, mDepthWrite);
			container.emplace<bool>(MaterialDepthTest, mDepthTest);
			container.emplace<bool>(MaterialInstanced, mInstanced);
			container.emplace<bool>(MaterialAlphaToCoverageSet, mAlphaToCoverageSet);
			container.emplace<bool>(MaterialAlphaToCoverage, mAlphaToCoverage);
			container.emplace<uint8_t>(MaterialCullingMode,
				static_cast<uint8_t>(mCullingMode));

			uint64_t properties = 0;

			for (size_t i = 0; i < MATERIAL_PROPERTIES_COUNT; i++) {
				if (mProperties[i]) {
					properties |= uint64_t(1u) << i;
				}
			}
			container.emplace<uint64_t>(MaterialProperties, properties);
			container.emplace<uint8_t>(MaterialStereoscopicType, static_cast<uint8_t>(1));
		}
		// create a unique material id

		std::hash<std::string> const hasher;
        size_t const materialId =MATERIAL_VERSION;
		container.emplace<uint64_t>(MaterialCacheId, materialId);
	}

	void MaterialBuilder::writeSurfaceChunks(ChunkContainer& container) const noexcept {
		if (mBlendingMode == BlendingMode::MASKED) {
			container.emplace<float>(MaterialMaskThreshold, mMaskThreshold);
		}
		container.emplace<uint8_t>(MaterialShading, static_cast<uint8_t>(mShading));
		if (mShading == Shading::UNLIT) {
			container.emplace<bool>(MaterialShadowMultiplier, mShadowMultiplier);
		}
		container.emplace<uint8_t>(MaterialRefraction, static_cast<uint8_t>(mRefractionMode));
		container.emplace<uint8_t>(MaterialRefractionType,
			static_cast<uint8_t>(mRefractionType));
		container.emplace<bool>(MaterialClearCoatIorChange, mClearCoatIorChange);

		container.emplace<bool>(MaterialSpecularAntiAliasing, mSpecularAntiAliasing);
		container.emplace<float>(MaterialSpecularAntiAliasingVariance,
			mSpecularAntiAliasingVariance);
		container.emplace<float>(MaterialSpecularAntiAliasingThreshold,
			mSpecularAntiAliasingThreshold);
		
		container.emplace<uint8_t>(MaterialInterpolation,
			static_cast<uint8_t>(mInterpolation));
	}

	MaterialBuilder& MaterialBuilder::noSamplerValidation(bool const enabled) noexcept {
		mNoSamplerValidation = enabled;
		return *this;
	}



} // namespace filamat
