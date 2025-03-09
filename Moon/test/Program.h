#pragma once

#include "utils/CString.h"
#include "utils/FixedCapacityVector.h"
//#include <utils/Invocable.h>
//#include <utils/ostream.h>

#include "DriverEnums.h"//<backend/>
#include <array>
#include <unordered_map>
#include <tuple>
#include <utility>

#include <variant>

#include <stddef.h>
#include <stdint.h>

namespace TEST {

	class Program {
	public:

		static constexpr size_t SHADER_TYPE_COUNT = 3;
		static constexpr size_t UNIFORM_BINDING_COUNT = CONFIG_UNIFORM_BINDING_COUNT;
		static constexpr size_t SAMPLER_BINDING_COUNT = CONFIG_SAMPLER_BINDING_COUNT;

		struct Descriptor {
			utils::CString name;
			DescriptorType type;
			descriptor_binding_t binding;
		};

		struct SpecializationConstant {
			using Type = std::variant<int32_t, float, bool>;
			uint32_t id;    // id set in glsl
			Type value;     // value and type
		};

		struct Uniform { // For ES2 support
			utils::CString name;    // full qualified name of the uniform field
			uint16_t offset;        // offset in 'uint32_t' into the uniform buffer
			uint8_t size;           // >1 for arrays
			UniformType type;       // uniform type
		};

		using DescriptorBindingsInfo = utils::FixedCapacityVector<Descriptor>;
		using DescriptorSetInfo = std::array<DescriptorBindingsInfo, MAX_DESCRIPTOR_SET_COUNT>;
		using SpecializationConstantsInfo = utils::FixedCapacityVector<SpecializationConstant>;
		using ShaderBlob = utils::FixedCapacityVector<uint8_t>;
		using ShaderSource = std::array<ShaderBlob, SHADER_TYPE_COUNT>;

		using AttributesInfo = utils::FixedCapacityVector<std::pair<utils::CString, uint8_t>>;
		using UniformInfo = utils::FixedCapacityVector<Uniform>;
		using BindingUniformsInfo = utils::FixedCapacityVector<
			std::tuple<uint8_t, utils::CString, Program::UniformInfo>>;

		Program() noexcept;

		Program(const Program& rhs) = delete;
		Program& operator=(const Program& rhs) = delete;

		Program(Program&& rhs) noexcept;
		Program& operator=(Program&& rhs) noexcept = delete;

		~Program() noexcept;

		Program& priorityQueue(CompilerPriorityQueue priorityQueue) noexcept;

		// sets the material name and variant for diagnostic purposes only
	   // Program& diagnostics(utils::CString const& name,
		  //      utils::Invocable<utils::io::ostream&(utils::io::ostream& out)>&& logger);

		// Sets one of the program's shader (e.g. vertex, fragment)
		// string-based shaders are null terminated, consequently the size parameter must include the
		// null terminating character.
		Program& shader(ShaderStage shader, void const* data, size_t size);

		// Sets the language of the shader sources provided with shader() (defaults to ESSL3)
		Program& shaderLanguage(ShaderLanguage shaderLanguage);

		// Descriptor binding (set, binding, type -> shader name) info
		Program& descriptorBindings(descriptor_set_t set,
			DescriptorBindingsInfo descriptorBindings) noexcept;

		Program& specializationConstants(SpecializationConstantsInfo specConstants) noexcept;

		struct PushConstant {
			utils::CString name;
			ConstantType type;
		};

		Program& pushConstants(ShaderStage stage,
			utils::FixedCapacityVector<PushConstant> constants) noexcept;

		Program& cacheId(uint64_t cacheId) noexcept;

		Program& multiview(bool multiview) noexcept;

		// For ES2 support only...
		Program& uniforms(uint32_t index, utils::CString name, UniformInfo uniforms) noexcept;
		Program& attributes(AttributesInfo attributes) noexcept;

		//
		// Getters for program construction...
		//

		ShaderSource const& getShadersSource() const noexcept { return mShadersSource; }
		ShaderSource& getShadersSource() noexcept { return mShadersSource; }

		utils::CString const& getName() const noexcept { return mName; }
		utils::CString& getName() noexcept { return mName; }

		auto const& getShaderLanguage() const { return mShaderLanguage; }

		uint64_t getCacheId() const noexcept { return mCacheId; }

		bool isMultiview() const noexcept { return mMultiview; }

		CompilerPriorityQueue getPriorityQueue() const noexcept { return mPriorityQueue; }

		SpecializationConstantsInfo const& getSpecializationConstants() const noexcept {
			return mSpecializationConstants;
		}

		SpecializationConstantsInfo& getSpecializationConstants() noexcept {
			return mSpecializationConstants;
		}

		DescriptorSetInfo& getDescriptorBindings() noexcept {
			return mDescriptorBindings;
		}

		utils::FixedCapacityVector<PushConstant> const& getPushConstants(
			ShaderStage stage) const noexcept {
			return mPushConstants[static_cast<uint8_t>(stage)];
		}

		utils::FixedCapacityVector<PushConstant>& getPushConstants(ShaderStage stage) noexcept {
			return mPushConstants[static_cast<uint8_t>(stage)];
		}

		auto const& getBindingUniformInfo() const { return mBindingUniformsInfo; }
		auto& getBindingUniformInfo() { return mBindingUniformsInfo; }

		auto const& getAttributes() const { return mAttributes; }
		auto& getAttributes() { return mAttributes; }

	private:
		//friend utils::io::ostream& operator<<(utils::io::ostream& out, const Program& builder);

		ShaderSource mShadersSource;
		ShaderLanguage mShaderLanguage = ShaderLanguage::ESSL3;
		utils::CString mName;
		uint64_t mCacheId{};
		CompilerPriorityQueue mPriorityQueue = CompilerPriorityQueue::HIGH;
		//utils::Invocable<utils::io::ostream&(utils::io::ostream& out)> mLogger;
		SpecializationConstantsInfo mSpecializationConstants;
		std::array<utils::FixedCapacityVector<PushConstant>, SHADER_TYPE_COUNT> mPushConstants;
		DescriptorSetInfo mDescriptorBindings;

		// For ES2 support only
		AttributesInfo mAttributes;
		BindingUniformsInfo mBindingUniformsInfo;
		bool mMultiview = false;
	};

}