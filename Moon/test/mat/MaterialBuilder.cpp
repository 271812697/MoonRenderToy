#include "MaterialBuilder.h"
#include "DescriptorSets.h"
#include "BufferInterfaceBlock.h"
#include "PushConstantDefinitions.h"
#include "Includes.h"
#include "SamplerInterfaceBlock.h"
#include "test/mat/MaterialInfo.h"
#include "ShaderGenerator.h"
#include "MaterialVariants.h"
#include <algorithm>
#include <atomic>
#include <tuple>
#include <utility>
#include <vector>
#include <iostream>
#include <fstream>

#include <stdint.h>
#include <stddef.h>

namespace TEST {


	using namespace utils;
	// Note: the VertexAttribute enum value must match the index in the array
	const MaterialBuilder::AttributeDatabase MaterialBuilder::sAttributeDatabase = { {
			{ "position",      AttributeType::FLOAT4, VertexAttribute::POSITION     },
			{ "tangents",      AttributeType::FLOAT4, VertexAttribute::TANGENTS     },
			{ "color",         AttributeType::FLOAT4, VertexAttribute::COLOR        },
			{ "uv0",           AttributeType::FLOAT2, VertexAttribute::UV0          },
			{ "uv1",           AttributeType::FLOAT2, VertexAttribute::UV1          },
			{ "bone_indices",  AttributeType::UINT4,  VertexAttribute::BONE_INDICES },
			{ "bone_weights",  AttributeType::FLOAT4, VertexAttribute::BONE_WEIGHTS },
			{ },
			{ "custom0",       AttributeType::FLOAT4, VertexAttribute::CUSTOM0      },
			{ "custom1",       AttributeType::FLOAT4, VertexAttribute::CUSTOM1      },
			{ "custom2",       AttributeType::FLOAT4, VertexAttribute::CUSTOM2      },
			{ "custom3",       AttributeType::FLOAT4, VertexAttribute::CUSTOM3      },
			{ "custom4",       AttributeType::FLOAT4, VertexAttribute::CUSTOM4      },
			{ "custom5",       AttributeType::FLOAT4, VertexAttribute::CUSTOM5      },
			{ "custom6",       AttributeType::FLOAT4, VertexAttribute::CUSTOM6      },
			{ "custom7",       AttributeType::FLOAT4, VertexAttribute::CUSTOM7      },
	} };

	MaterialBuilder::MaterialBuilder() : mMaterialName("Unnamed") {
		std::fill_n(mProperties, MATERIAL_PROPERTIES_COUNT, false);


		initPushConstants();
	}

	MaterialBuilder::~MaterialBuilder() = default;

	MaterialBuilder& MaterialBuilder::noSamplerValidation(bool enabled) noexcept {
		mNoSamplerValidation = enabled;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::includeEssl1(bool enabled) noexcept {
		mIncludeEssl1 = enabled;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::name(const char* name) noexcept
	{
		mMaterialName = CString(name);
		return *this;
	}

	MaterialBuilder& MaterialBuilder::fileName(const char* name) noexcept
	{
		mFileName = CString(name);
		return *this;
	}

	MaterialBuilder& MaterialBuilder::shading(Shading shading) noexcept
	{
		// TODO: 在此处插入 return 语句
		return *this;
	}
	MaterialBuilder& MaterialBuilder::interpolation(Interpolation interpolation) noexcept {
		mInterpolation = interpolation;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::variable(Variable v, const char* name) noexcept {
		switch (v) {
		case Variable::CUSTOM0:
		case Variable::CUSTOM1:
		case Variable::CUSTOM2:
		case Variable::CUSTOM3:
			assert(size_t(v) < MATERIAL_VARIABLES_COUNT);
			mVariables[size_t(v)] = { CString(name), Precision::DEFAULT, false };
			break;
		}
		return *this;
	}

	MaterialBuilder& MaterialBuilder::variable(Variable v,
		const char* name, ParameterPrecision precision) noexcept {
		switch (v) {
		case Variable::CUSTOM0:
		case Variable::CUSTOM1:
		case Variable::CUSTOM2:
		case Variable::CUSTOM3:
			assert(size_t(v) < MATERIAL_VARIABLES_COUNT);
			mVariables[size_t(v)] = { CString(name), precision, true };
			break;
		}
		return *this;
	}

	MaterialBuilder& MaterialBuilder::parameter(const char* name, size_t size, UniformType type,
		ParameterPrecision precision) noexcept {
		assert(mParameterCount < MAX_PARAMETERS_COUNT);// << "Too many parameters";
		mParameters[mParameterCount++] = { name, type, size, precision };
		return *this;
	}

	MaterialBuilder& MaterialBuilder::parameter(const char* name, UniformType type,
		ParameterPrecision precision) noexcept {
		return parameter(name, 1, type, precision);
	}
	MaterialBuilder& MaterialBuilder::parameter(const char* name, SamplerType samplerType,
		SamplerFormat format, ParameterPrecision precision, bool multisample) noexcept {
		assert(!multisample ||
			(format != SamplerFormat::SHADOW &&
				(samplerType == SamplerType::SAMPLER_2D ||
					samplerType == SamplerType::SAMPLER_2D_ARRAY)));

		assert(mParameterCount < MAX_PARAMETERS_COUNT);// << "Too many parameters";
		mParameters[mParameterCount++] = { name, samplerType, format, precision, multisample };
		return *this;
	}
	template<typename T, typename>
	MaterialBuilder& MaterialBuilder::constant(const char* name, ConstantType type, T defaultValue) {
		auto result = std::find_if(mConstants.begin(), mConstants.end(), [name](const Constant& c) {
			return c.name == utils::CString(name);
			});
		assert(result == mConstants.end());

		Constant constant;
		constant.name = CString(name);
		constant.type = type;

		auto toString = [](ConstantType t) {
			switch (t) {
			case ConstantType::INT: return "INT";
			case ConstantType::FLOAT: return "FLOAT";
			case ConstantType::BOOL: return "BOOL";
			}
			};

		if constexpr (std::is_same_v<T, int32_t>) {
			assert(type == ConstantType::INT);
			constant.defaultValue.i = defaultValue;
		}
		else if constexpr (std::is_same_v<T, float>) {
			assert(type == ConstantType::FLOAT);
			constant.defaultValue.f = defaultValue;
		}
		else if constexpr (std::is_same_v<T, bool>) {
			assert(type == ConstantType::BOOL);
			constant.defaultValue.b = defaultValue;
		}
		else {
			assert(false);
		}

		mConstants.push_back(constant);
		return *this;
	}
	template MaterialBuilder& MaterialBuilder::constant<int32_t>(
		const char* name, ConstantType type, int32_t defaultValue);
	template MaterialBuilder& MaterialBuilder::constant<float>(
		const char* name, ConstantType type, float defaultValue);
	template MaterialBuilder& MaterialBuilder::constant<bool>(
		const char* name, ConstantType type, bool defaultValue);
	MaterialBuilder& MaterialBuilder::buffer(BufferInterfaceBlock bib) noexcept {
		assert(mBuffers.size() < MAX_BUFFERS_COUNT);// << "Too many buffers";
		mBuffers.emplace_back(std::make_unique<BufferInterfaceBlock>(std::move(bib)));
		return *this;
	}

	MaterialBuilder& MaterialBuilder::require(VertexAttribute attribute) noexcept {
		mRequiredAttributes.set(attribute);
		return *this;
	}



	MaterialBuilder& MaterialBuilder::materialDomain(
		MaterialBuilder::MaterialDomain materialDomain) noexcept {
		mMaterialDomain = materialDomain;
		if (mMaterialDomain == MaterialDomain::COMPUTE) {
			// compute implies feature level 2
			if (mFeatureLevel < FeatureLevel::FEATURE_LEVEL_2) {
				mFeatureLevel = FeatureLevel::FEATURE_LEVEL_2;
			}
		}
		return *this;
	}

	MaterialBuilder& MaterialBuilder::refractionMode(RefractionMode refraction) noexcept {
		mRefractionMode = refraction;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::refractionType(RefractionType refractionType) noexcept {
		mRefractionType = refractionType;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::material(const char* code, size_t line) noexcept
	{
		mMaterialFragmentCode.setUnresolved(CString(code));
		mMaterialFragmentCode.setLineOffset(line);
		return *this;
	}

	MaterialBuilder& MaterialBuilder::includeCallback(IncludeCallback callback) noexcept
	{
		mIncludeCallback = std::move(callback);
		return *this;
	}


	MaterialBuilder& MaterialBuilder::materialVertex(const char* code, size_t line) noexcept
	{
		mMaterialVertexCode.setUnresolved(CString(code));
		mMaterialVertexCode.setLineOffset(line);
		return *this;
	}

	MaterialBuilder& MaterialBuilder::quality(ShaderQuality quality) noexcept {
		mShaderQuality = quality;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::featureLevel(FeatureLevel featureLevel) noexcept {
		mFeatureLevel = featureLevel;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::blending(BlendingMode blending) noexcept {
		mBlendingMode = blending;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::customBlendFunctions(
		BlendFunction srcRGB, BlendFunction srcA,
		BlendFunction dstRGB, BlendFunction dstA) noexcept {
		mCustomBlendFunctions[0] = srcRGB;
		mCustomBlendFunctions[1] = srcA;
		mCustomBlendFunctions[2] = dstRGB;
		mCustomBlendFunctions[3] = dstA;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::postLightingBlending(BlendingMode blending) noexcept {
		mPostLightingBlendingMode = blending;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::vertexDomain(VertexDomain domain) noexcept {
		mVertexDomain = domain;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::culling(CullingMode culling) noexcept {
		mCullingMode = culling;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::colorWrite(bool enable) noexcept {
		mColorWrite = enable;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::depthWrite(bool enable) noexcept {
		mDepthWrite = enable;
		mDepthWriteSet = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::depthCulling(bool enable) noexcept {
		mDepthTest = enable;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::instanced(bool enable) noexcept {
		mInstanced = enable;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::doubleSided(bool doubleSided) noexcept {
		mDoubleSided = doubleSided;
		mDoubleSidedCapability = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::maskThreshold(float threshold) noexcept {
		mMaskThreshold = threshold;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::alphaToCoverage(bool enable) noexcept {
		mAlphaToCoverage = enable;
		mAlphaToCoverageSet = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::shadowMultiplier(bool shadowMultiplier) noexcept {
		mShadowMultiplier = shadowMultiplier;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::transparentShadow(bool transparentShadow) noexcept {
		mTransparentShadow = transparentShadow;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::specularAntiAliasing(bool specularAntiAliasing) noexcept {
		mSpecularAntiAliasing = specularAntiAliasing;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::specularAntiAliasingVariance(float screenSpaceVariance) noexcept {
		mSpecularAntiAliasingVariance = screenSpaceVariance;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::specularAntiAliasingThreshold(float threshold) noexcept {
		mSpecularAntiAliasingThreshold = threshold;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::clearCoatIorChange(bool clearCoatIorChange) noexcept {
		mClearCoatIorChange = clearCoatIorChange;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::flipUV(bool flipUV) noexcept {
		mFlipUV = flipUV;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::customSurfaceShading(bool customSurfaceShading) noexcept {
		mCustomSurfaceShading = customSurfaceShading;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::multiBounceAmbientOcclusion(bool multiBounceAO) noexcept {
		mMultiBounceAO = multiBounceAO;
		mMultiBounceAOSet = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::specularAmbientOcclusion(SpecularAmbientOcclusion specularAO) noexcept {
		mSpecularAO = specularAO;
		mSpecularAOSet = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::transparencyMode(TransparencyMode mode) noexcept {
		mTransparencyMode = mode;
		return *this;
	}



	MaterialBuilder& MaterialBuilder::stereoscopicEyeCount(uint8_t eyeCount) noexcept {
		mStereoscopicEyeCount = eyeCount;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::reflectionMode(ReflectionMode mode) noexcept {
		mReflectionMode = mode;
		return *this;
	}







	MaterialBuilder& MaterialBuilder::shaderDefine(const char* name, const char* value) noexcept {
		mDefines.emplace_back(name, value);
		return *this;
	}

	MaterialBuilder& MaterialBuilder::output(VariableQualifier qualifier, OutputTarget target, Precision precision, OutputType type, const char* name, int location) noexcept
	{
		assert(target != OutputTarget::DEPTH || type == OutputType::FLOAT);
		assert(target != OutputTarget::DEPTH || qualifier == VariableQualifier::OUT);

		assert(location >= -1);

		// A location value of -1 signals using the default location. We'll simply take the previous
		// output's location and add 1.
		if (location == -1) {
			location = mOutputs.empty() ? 0 : mOutputs.back().location + 1;
		}

		// Unconditionally add this output, then we'll check if we've maxed on on any particular target.
		mOutputs.emplace_back(name, qualifier, target, precision, type, location);

		uint8_t colorOutputCount = 0;
		uint8_t depthOutputCount = 0;
		for (const auto& output : mOutputs) {
			if (output.target == OutputTarget::COLOR) {
				colorOutputCount++;
			}
			if (output.target == OutputTarget::DEPTH) {
				depthOutputCount++;
			}
		}

		assert(colorOutputCount <= MAX_COLOR_OUTPUT);
		assert(depthOutputCount <= MAX_DEPTH_OUTPUT);

		assert(mOutputs.size() <= MAX_COLOR_OUTPUT + MAX_DEPTH_OUTPUT);

		return *this;
	}

	MaterialBuilder& MaterialBuilder::enableFramebufferFetch() noexcept
	{
		// This API is temporary, it is used to enable EXT_framebuffer_fetch for GLSL shaders,
		  // this is used sparingly by filament's post-processing stage.
		mEnableFramebufferFetch = true;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::vertexDomainDeviceJittered(bool enabled) noexcept
	{
		mVertexDomainDeviceJittered = enabled;
		return *this;
	}

	MaterialBuilder& MaterialBuilder::useLegacyMorphing() noexcept
	{
		mUseLegacyMorphing = true;
		return *this;
	}

	Program MaterialBuilder::build()
	{
		if (mMaterialDomain == MaterialDomain::POST_PROCESS) {
			mShading = Shading::UNLIT;
		}
		// Add a default color output.
		if (mMaterialDomain == MaterialDomain::POST_PROCESS && mOutputs.empty()) {
			output(VariableQualifier::OUT, OutputTarget::COLOR, Precision::DEFAULT, OutputType::FLOAT4, "color");
		}
		// Resolve all the #include directives within user code.
		if (!mMaterialFragmentCode.resolveIncludes(mIncludeCallback, mFileName) ||
			!mMaterialVertexCode.resolveIncludes(mIncludeCallback, mFileName)) {

		}
		ShaderGenerator sg(mProperties, mVariables, mOutputs, mDefines, mConstants, mPushConstants,
			mMaterialFragmentCode.getResolved(), mMaterialFragmentCode.getLineOffset(),
			mMaterialVertexCode.getResolved(), mMaterialVertexCode.getLineOffset(),
			mMaterialDomain);
		MaterialInfo info{};
		prepareToBuild(info);
		utils::FixedCapacityVector<uint8_t> vsBuilder;
		utils::FixedCapacityVector<uint8_t> fsBuilder;
		utils::FixedCapacityVector<uint8_t> csBuilder;
		for (const auto& stage : { ShaderStage::VERTEX ,ShaderStage::FRAGMENT }) {
			std::string shader;
			if (stage == ShaderStage::VERTEX) {
				shader = sg.createVertexProgram(info, Variant(103), mInterpolation, mVertexDomain);


				vsBuilder.reserve(shader.size());
				vsBuilder.resize(shader.size());
				memcpy(&vsBuilder[0], shader.c_str(), shader.size());
				vsBuilder.back() = 0;
			}
			else if (stage == ShaderStage::FRAGMENT) {
				shader = sg.createFragmentProgram(info, Variant(103), mInterpolation, mVariantFilter);

				fsBuilder.reserve(shader.size());
				fsBuilder.resize(shader.size());
				memcpy(&fsBuilder[0], shader.data(), shader.size());
				fsBuilder.back() = 0;
			}
			else if (stage == ShaderStage::COMPUTE) {
				shader = sg.createComputeProgram(info);

				csBuilder.reserve(shader.size());
				csBuilder.resize(shader.size());
				memcpy(&csBuilder[0], shader.data(), shader.size());
				csBuilder.back() = 0;
			}

		}
		utils::FixedCapacityVector<Program::PushConstant> vertexPushConstant;
		utils::FixedCapacityVector<Program::PushConstant> fragPushConstant;
		vertexPushConstant.reserve(mPushConstants.size());
		fragPushConstant.reserve(mPushConstants.size());
		for (auto& push : mPushConstants) {
			if (push.stage == ShaderStage::VERTEX) {
				vertexPushConstant.push_back({ push.name,push.type });
			}
			else if (push.stage == ShaderStage::FRAGMENT) {
				fragPushConstant.push_back({ push.name,push.type });
			}

		}
		//Per_Material
		Program::DescriptorBindingsInfo bindInfo;
		bindInfo.reserve(1 + info.sib.getSamplerInfoList().size());
		bindInfo.push_back({ TEST::getDescriptorName(DescriptorSetBindingPoints::PER_MATERIAL, 0),DescriptorType::UNIFORM_BUFFER,0 });
		for (auto& sampleItem : info.sib.getSamplerInfoList()) {
			bindInfo.push_back({ sampleItem.uniformName,
				sampleItem.type == SamplerType::SAMPLER_EXTERNAL ? DescriptorType::SAMPLER_EXTERNAL : DescriptorType::SAMPLER,sampleItem.binding });
		}
		Program program;
		program.shader(ShaderStage::VERTEX, vsBuilder.data(), vsBuilder.size())
			.shader(ShaderStage::FRAGMENT, fsBuilder.data(), fsBuilder.size()).
			shaderLanguage(ShaderLanguage::ESSL3).
			pushConstants(ShaderStage::VERTEX, vertexPushConstant)
			.pushConstants(ShaderStage::FRAGMENT, fragPushConstant).
			descriptorBindings(0, bindInfo);
		return program;

	}

	bool MaterialBuilder::hasSamplerType(SamplerType samplerType) const noexcept {
		for (size_t i = 0, c = mParameterCount; i < c; i++) {
			auto const& param = mParameters[i];
			if (param.isSampler() && param.samplerType == samplerType) {
				return  true;
			}
		}
		return false;
	}
	void MaterialBuilder::prepareToBuild(MaterialInfo& info)
	{
		// Build the per-material sampler block and uniform block.
		SamplerInterfaceBlock::Builder sbb;
		BufferInterfaceBlock::Builder ibb;
		// sampler bindings start at 1, 0 is the ubo
		for (size_t i = 0, binding = 1, c = mParameterCount; i < c; i++) {
			auto const& param = mParameters[i];
			assert(!param.isSubpass());
			if (param.isSampler()) {
				sbb.add({ param.name.data(), param.name.size() },
					binding++, param.samplerType, param.format, param.precision, param.multisample);
			}
			else if (param.isUniform()) {
				ibb.add({ {{ param.name.data(), param.name.size() },
						  uint32_t(param.size == 1u ? 0u : param.size), param.uniformType,
						  param.precision } });
			}
		}

		for (size_t i = 0, c = mSubpassCount; i < c; i++) {
			auto const& param = mSubpasses[i];
			assert(param.isSubpass());
			// For now, we only support a single subpass for attachment 0.
			// Subpasses belong to the "MaterialParams" block.
			const uint8_t attachmentIndex = 0;
			const uint8_t binding = 0;
			//info.subpass = { CString("MaterialParams"), param.name, param.subpassType,
							 //param.format, param.precision, attachmentIndex, binding };
		}

		for (auto const& buffer : mBuffers) {
			info.buffers.emplace_back(buffer.get());
		}

		if (mSpecularAntiAliasing) {
			ibb.add({
					{ "_specularAntiAliasingVariance",  0, UniformType::FLOAT },
					{ "_specularAntiAliasingThreshold", 0, UniformType::FLOAT },
				});
		}

		if (mBlendingMode == BlendingMode::MASKED) {
			ibb.add({ { "_maskThreshold", 0, UniformType::FLOAT, Precision::DEFAULT} });
		}

		if (mDoubleSidedCapability) {
			ibb.add({ { "_doubleSided", 0, UniformType::BOOL, Precision::DEFAULT } });
		}

		mRequiredAttributes.set(VertexAttribute::POSITION);
		if (mShading != Shading::UNLIT || mShadowMultiplier) {
			//mRequiredAttributes.set(VertexAttribute::TANGENTS);
		}

		info.sib = sbb.name("MaterialParams").build();
		info.uib = ibb.name("MaterialParams").build();

		info.isLit = isLit();
		info.hasDoubleSidedCapability = mDoubleSidedCapability;
		info.hasExternalSamplers = hasSamplerType(SamplerType::SAMPLER_EXTERNAL);
		info.has3dSamplers = hasSamplerType(SamplerType::SAMPLER_3D);
		info.specularAntiAliasing = mSpecularAntiAliasing;
		info.clearCoatIorChange = mClearCoatIorChange;
		info.flipUV = mFlipUV;
		info.requiredAttributes = mRequiredAttributes;
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
		//info.featureLevel = mFeatureLevel;
		//info.groupSize = mGroupSize;
		//info.stereoscopicType = mStereoscopicType;
		info.stereoscopicEyeCount = mStereoscopicEyeCount;

		// This is determined via static analysis of the glsl after prepareToBuild().
		info.userMaterialHasCustomDepth = false;
	}

	void MaterialBuilder::initPushConstants() noexcept {
		mPushConstants.reserve(PUSH_CONSTANTS.size());
		mPushConstants.resize(PUSH_CONSTANTS.size());
		std::transform(PUSH_CONSTANTS.cbegin(), PUSH_CONSTANTS.cend(), mPushConstants.begin(),
			[](MaterialPushConstant const& inConstant) -> PushConstant {
				return {
					inConstant.name,
					 inConstant.type,
					 inConstant.stage,
				};
			});
	}

	bool MaterialBuilder::checkLiteRequirements() noexcept
	{
		return false;
	}

	bool MaterialBuilder::checkMaterialLevelFeatures(MaterialInfo const& info) const noexcept
	{
		return false;
	}

	void MaterialBuilder::writeCommonChunks(ChunkContainer& container, MaterialInfo& info) const noexcept
	{
	}

	void MaterialBuilder::writeSurfaceChunks(ChunkContainer& container) const noexcept
	{
	}

	bool MaterialBuilder::hasCustomVaryings() const noexcept {
		for (const auto& variable : mVariables) {
			if (!variable.name.empty()) {
				return true;
			}
		}
		return false;
	}
	bool MaterialBuilder::needsStandardDepthProgram() const noexcept {
		const bool hasEmptyVertexCode = mMaterialVertexCode.getResolved().empty();
		return !hasEmptyVertexCode ||
			hasCustomVaryings() ||
			mBlendingMode == BlendingMode::MASKED ||
			(mTransparentShadow &&
				(mBlendingMode == BlendingMode::TRANSPARENT ||
					mBlendingMode == BlendingMode::FADE));
	}




	bool MaterialBuilder::ShaderCode::resolveIncludes(IncludeCallback callback,
		const CString& fileName) noexcept {
		if (!mCode.empty()) {
			ResolveOptions options{
					 true,
					true,
					true
			};
			IncludeResult source{
					 fileName,
					 mCode,
					 getLineOffset(),
					 CString("")
			};

			if (!TEST::resolveIncludes(source, std::move(callback), options)) {
				return false;
			}
			mCode = source.text;
		}

		mIncludesResolved = true;
		return true;
	}
} // namespace filamat
