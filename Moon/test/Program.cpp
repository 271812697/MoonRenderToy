#include "Program.h"
#include "DriverEnums.h"
#include <utility>
#include <stddef.h>
#include <stdint.h>

namespace TEST {

	//using namespace utils;

	// We want these in the .cpp file, so they're not inlined (not worth it)
	Program::Program() noexcept {  // NOLINT(modernize-use-equals-default)
	}

	Program::Program(Program&& rhs) noexcept = default;

	Program::~Program() noexcept = default;

	Program& Program::priorityQueue(CompilerPriorityQueue priorityQueue) noexcept {
		mPriorityQueue = priorityQueue;
		return *this;
	}



	Program& Program::shader(ShaderStage shader, void const* data, size_t size) {
		ShaderBlob blob(size);
		std::copy_n((const uint8_t*)data, size, blob.data());
		mShadersSource[size_t(shader)] = std::move(blob);
		return *this;
	}

	Program& Program::shaderLanguage(ShaderLanguage shaderLanguage) {
		mShaderLanguage = shaderLanguage;
		return *this;
	}

	Program& Program::descriptorBindings(descriptor_set_t set,
		DescriptorBindingsInfo descriptorBindings) noexcept {
		mDescriptorBindings[set] = std::move(descriptorBindings);
		return *this;
	}

	Program& Program::uniforms(uint32_t index, utils::CString name, UniformInfo uniforms) noexcept {
		mBindingUniformsInfo.reserve(mBindingUniformsInfo.capacity() + 1);
		mBindingUniformsInfo.emplace_back(index, std::move(name), std::move(uniforms));
		return *this;
	}

	Program& Program::attributes(AttributesInfo attributes) noexcept {
		mAttributes = std::move(attributes);
		return *this;
	}

	Program& Program::name(utils::CString& name)
	{
		mName = name;
		return *this;
	}

	Program& Program::specializationConstants(SpecializationConstantsInfo specConstants) noexcept {
		mSpecializationConstants = std::move(specConstants);
		return *this;
	}

	Program& Program::pushConstants(ShaderStage stage,
		utils::FixedCapacityVector<PushConstant> constants) noexcept {
		mPushConstants[static_cast<uint8_t>(stage)] = std::move(constants);
		return *this;
	}

	Program& Program::cacheId(uint64_t cacheId) noexcept {
		mCacheId = cacheId;
		return *this;
	}

	Program& Program::multiview(bool multiview) noexcept {
		mMultiview = multiview;
		return *this;
	}



} // namespace filament::backend
