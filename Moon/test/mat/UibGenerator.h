#pragma once
#include "test/DriverEnums.h"//<backend/DriverEnums.h>
#include "BitmaskEnum.h"
#include <type_traits>
#include <stddef.h>
#include <stdint.h>

namespace TEST {

	class BufferInterfaceBlock;

	class UibGenerator {
	public:
		// tag to represent a generated ubo.
		enum class Ubo : uint8_t {
			FrameUniforms,              // uniforms updated per view
			ObjectUniforms,             // uniforms updated per renderable
			BonesUniforms,              // bones data, per renderable
			MorphingUniforms,           // morphing uniform/sampler updated per render primitive
			LightsUniforms,             // lights data array
			ShadowUniforms,             // punctual shadow data
			FroxelRecordUniforms,       // froxel records
			FroxelsUniforms,            // froxels
			MaterialParams,             // material instance ubo
			// Update utils::Enum::count<>() below when adding values here
			// These are limited by CONFIG_BINDING_COUNT (currently 10)
			// When adding an UBO here, make sure to also update
			//      MaterialBuilder::writeCommonChunks() if needed
		};

		struct Binding {
			descriptor_set_t set;
			descriptor_binding_t binding;
		};

		// return the BufferInterfaceBlock for the given UBO tag
		static BufferInterfaceBlock const& get(Ubo ubo) noexcept;

		// return the {set, binding } for the given UBO tag
		static Binding getBinding(Ubo ubo) noexcept;

		// deprecate these...
		static BufferInterfaceBlock const& getPerViewUib() noexcept;
		static BufferInterfaceBlock const& getPerRenderableUib() noexcept;
		static BufferInterfaceBlock const& getLightsUib() noexcept;
		static BufferInterfaceBlock const& getShadowUib() noexcept;
		static BufferInterfaceBlock const& getPerRenderableBonesUib() noexcept;
		static BufferInterfaceBlock const& getPerRenderableMorphingUib() noexcept;
		static BufferInterfaceBlock const& getFroxelRecordUib() noexcept;
		static BufferInterfaceBlock const& getFroxelsUib() noexcept;
	};

} // namespace filament

template<>
struct utils::EnableIntegerOperators<TEST::UibGenerator::Ubo> : public std::true_type {};

template<>
inline constexpr size_t utils::Enum::count<TEST::UibGenerator::Ubo>() { return 9; }

