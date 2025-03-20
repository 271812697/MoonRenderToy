#pragma once

#include <map>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>
#include <glad/glad.h>

#include "../asset/shader.h"
#include "../asset/texture.h"
#include  "component.h"
#include "core/base.h"

namespace component {

	using namespace glm;

	// allowed uniform types: int, uint, bool, float + 32-bit float vector/matrix
	template<typename T>
	struct is_glsl_t {
		using type = typename std::enable_if_t<
			std::is_same_v<T, int> ||
			std::is_same_v<T, bool> ||
			std::is_same_v<T, float> ||
			std::is_same_v<T, GLuint> ||
			std::is_same_v<T, vec2> || std::is_same_v<T, uvec2> || std::is_same_v<T, ivec2> ||
			std::is_same_v<T, vec3> || std::is_same_v<T, uvec3> || std::is_same_v<T, ivec3> ||
			std::is_same_v<T, vec4> || std::is_same_v<T, uvec4> || std::is_same_v<T, ivec4> ||
			std::is_same_v<T, mat2> ||
			std::is_same_v<T, mat3> ||
			std::is_same_v<T, mat4>
		>;
	};

	template<typename T>
	class Uniform {
	public:
		std::string name;
		GLuint location;
		GLuint owner_id;  // which shader owns this uniform
		GLuint size;  // if not an array, this is 1

	private:
		T value;
		const T* value_ptr;
		const std::vector<T>* array_ptr;
		void Upload(T val, GLuint index = 0) const;

	public:
		Uniform() = default;
		Uniform(GLuint owner_id, GLuint location, const char* name);

		void operator<<(const T& value);
		void operator<<=(const T* value_ptr);
		void operator<<=(const std::vector<T>* array_ptr);
		void Upload() const;
	};

	enum class pbr_u : uint16_t {
		albedo = 912U,
		roughness = 913U,
		ao = 914U,
		emission = 915U,
		uv_scale = 916U,
		alpha_mask = 928U,
		metalness = 917U,
		specular = 918U,
		anisotropy = 919U,
		aniso_dir = 920U,
		transmission = 921U,
		thickness = 922U,
		ior = 923U,
		transmittance = 924U,
		tr_distance = 925U,
		volume_type = 931U,
		sheen_color = 926U,
		subsurf_color = 927U,
		clearcoat = 929U,
		cc_roughness = 930U,
		shading_model = 999U
	};

	enum class pbr_t : uint8_t {
		irradiance_map = 17U,
		prefiltered_map = 18U,
		BRDF_LUT = 19U,
		albedo = 20U,
		normal = 21U,
		metallic = 22U,
		roughness = 23U,
		ao = 24U,
		emission = 25U,
		displace = 26U,
		opacity = 27U,
		lightmap = 28U,
		anisotropic = 29U
	};

	class Material : public Component {
	private:
		using uniform_variant = std::variant<
			Uniform<int>, Uniform<uint>, Uniform<bool>, Uniform<float>,
			Uniform<vec2>, Uniform<vec3>, Uniform<vec4>, Uniform<uvec2>, Uniform<uvec3>, Uniform<uvec4>,
			Uniform<mat2>, Uniform<mat3>, Uniform<mat4>, Uniform<ivec2>, Uniform<ivec3>, Uniform<ivec4>
		>;

		static_assert(std::variant_size_v<uniform_variant> == 16);

	private:
		using Shader = asset::Shader;
		using Texture = asset::Texture;

		asset_ref<Shader> shader;
		std::map<GLuint, uniform_variant> uniforms;
		std::map<GLuint, asset_ref<Texture>> textures;

	public:
		Material(const asset_ref<Shader>& shader_asset);
		Material(const asset_ref<Material>& material_asset);

		void Bind() const;
		void Unbind() const;

		void SetShader(asset_ref<Shader> shader_ref);
		void SetTexture(GLuint unit, asset_ref<Texture> texture_ref);
		void SetTexture(pbr_t attribute, asset_ref<Texture> texture_ref);

		template<typename T, typename = is_glsl_t<T>>
		void SetUniform(GLuint location, const T& value);

		template<typename T, typename = is_glsl_t<T>>
		void SetUniform(pbr_u attribute, const T& value);

		template<typename T, typename = is_glsl_t<T>>
		void BindUniform(GLuint location, const T* value_ptr);

		template<typename T, typename = is_glsl_t<T>>
		void BindUniform(pbr_u attribute, const T* value_ptr);

		template<typename T, typename = is_glsl_t<T>>
		void SetUniformArray(GLuint location, GLuint size, const std::vector<T>* array_ptr);
	};

}
