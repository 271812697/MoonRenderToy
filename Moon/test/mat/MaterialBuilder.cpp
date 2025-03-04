
#include "MaterialBuilder.h"
#include "BufferInterfaceBlock.h"
#include "PushConstantDefinitions.h"
#include "Includes.h"
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

	bool MaterialBuilder::hasSamplerType(SamplerType samplerType) const noexcept {
		for (size_t i = 0, c = mParameterCount; i < c; i++) {
			auto const& param = mParameters[i];
			if (param.isSampler() && param.samplerType == samplerType) {
				return  true;
			}
		}
		return false;
	}



	void MaterialBuilder::prepareToBuild(MaterialInfo& info) noexcept
	{
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
