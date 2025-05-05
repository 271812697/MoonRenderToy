/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <tracy/Tracy.hpp>

#include <Debug/Assertion.h>
#include <Debug/Logger.h>

#include <Rendering/Data/Material.h>
#include <Rendering/HAL/UniformBuffer.h>
#include <Rendering/HAL/TextureHandle.h>
#include <Rendering/Resources/Texture.h>

#include <Tools/Utils/OptRef.h>

namespace
{
	Rendering::Data::MaterialPropertyType UniformToPropertyValue(const std::any& p_uniformValue)
	{
		using namespace Maths;
		using namespace Rendering;

		auto as = [&]<typename T>() -> std::optional<T> {
			return
				p_uniformValue.type() == typeid(T) ?
				std::optional<T>{std::any_cast<T>(p_uniformValue)} :
				std::nullopt;
		};

		if (auto value = as.operator()<int>()) return *value;
		if (auto value = as.operator()<float>()) return *value;
		if (auto value = as.operator()<FVector2>()) return *value;
		if (auto value = as.operator()<FVector3>()) return *value;
		if (auto value = as.operator()<FVector4>()) return *value;
		if (auto value = as.operator()<FMatrix4>()) return *value;
		if (auto value = as.operator()<HAL::TextureHandle*>()) return *value;
		if (auto value = as.operator()<Resources::Texture*>()) return *value;

		return std::monostate{};
	}

	void BindTexture(
		Rendering::HAL::ShaderProgram& p_shader,
		const std::string& p_uniformName,
		Rendering::HAL::TextureHandle* p_texture,
		Rendering::HAL::TextureHandle* p_fallback,
		int& p_textureSlot
	)
	{
		if (auto target = p_texture ? p_texture : p_fallback)
		{
			target->Bind(p_textureSlot);
			p_shader.SetUniform<int>(p_uniformName, p_textureSlot++);
		}
	}
}

Rendering::Data::Material::Material(Rendering::Resources::Shader* p_shader)
{
	SetShader(p_shader);
}

void Rendering::Data::Material::SetShader(Rendering::Resources::Shader* p_shader)
{
	m_shader = p_shader;

	if (m_shader)
	{
		FillUniform();
	}
	else
	{
		m_properties.clear();
	}
}

void Rendering::Data::Material::FillUniform()
{
	m_properties.clear();

	for (const auto& uniform : m_shader->GetProgram().GetUniforms())
	{
		m_properties.emplace(uniform.name, MaterialProperty{
			.value = UniformToPropertyValue(uniform.defaultValue),
			.singleUse = false
		});
	}
}

void Rendering::Data::Material::Bind(Rendering::HAL::Texture* p_emptyTexture)
{
	ZoneScoped;

	using namespace Maths;
	using enum Rendering::Settings::EUniformType;

	ASSERT(IsValid(), "Attempting to bind an invalid material.");

	auto& program = m_shader->GetProgram();
	program.Bind();

	int textureSlot = 0;

	for (auto& [name, prop] : m_properties)
	{
		const auto uniformData = program.GetUniformInfo(name);

		// Skip this property if the current program isn't using its associated uniform
		if (!uniformData)
		{
			continue;
		}

		auto& value = prop.value;
		auto uniformType = uniformData->type;

		// Visitor to handle each variant type
		auto visitor = [&](auto&& arg) {
			using PropertyType = std::decay_t<decltype(arg)>;

			if constexpr (std::same_as<PropertyType, bool>)
			{
				if (uniformType == BOOL)
				{
					program.SetUniform<int>(name, arg);
				}
			}
			else if constexpr (std::same_as<PropertyType, int>)
			{
				if (uniformType == INT)
				{
					program.SetUniform<int>(name, arg);
				}
			}
			else if constexpr (std::same_as<PropertyType, float>)
			{
				if (uniformType == FLOAT)
				{
					program.SetUniform<float>(name, arg);
				}
			}
			else if constexpr (std::same_as<PropertyType, FVector2>)
			{
				if (uniformType == FLOAT_VEC2)
				{
					program.SetUniform<FVector2>(name, arg);
				}
			}
			else if constexpr (std::same_as<PropertyType, FVector3>)
			{
				if (uniformType == FLOAT_VEC3)
				{
					program.SetUniform<FVector3>(name, arg);
				}
			}
			else if constexpr (std::same_as<PropertyType, FVector4>)
			{
				if (uniformType == FLOAT_VEC4)
				{
					program.SetUniform<FVector4>(name, arg);
				}
			}
			else if constexpr (std::same_as<PropertyType, FMatrix4>)
			{
				if (uniformType == FLOAT_MAT4)
				{
					program.SetUniform<FMatrix4>(name, arg);
				}
			}
			else if constexpr (std::same_as<PropertyType, HAL::TextureHandle*>)
			{
				if (uniformType == SAMPLER_2D)
				{
					BindTexture(program, name, arg, p_emptyTexture, textureSlot);
				}
			}
			else if constexpr (std::same_as<PropertyType, Resources::Texture*>)
			{
				if (uniformType == SAMPLER_2D)
				{
					BindTexture(program, name, arg ? &arg->GetTexture() : nullptr, p_emptyTexture, textureSlot);
				}
			}
		};

		std::visit(visitor, value);

		if (prop.singleUse)
		{
			value = UniformToPropertyValue(uniformData->defaultValue);
		}
	}
}

void Rendering::Data::Material::Unbind() const
{
	ASSERT(IsValid(), "Attempting to unbind an invalid material.");
	m_shader->GetProgram().Unbind();
}

void Rendering::Data::Material::SetProperty(const std::string p_name, const MaterialPropertyType& p_value, bool p_singleUse)
{
	ASSERT(IsValid(), "Attempting to SetProperty on an invalid material.");

	if (m_properties.find(p_name) != m_properties.end())
	{
		const auto property = MaterialProperty{
			p_value,
			p_singleUse
		};

		m_properties[p_name] = property;
	}
	else
	{
		OVLOG_ERROR("Material Set failed: Uniform not found");
	}
}

Tools::Utils::OptRef<const Rendering::Data::MaterialProperty> Rendering::Data::Material::GetProperty(const std::string p_key) const
{
	ASSERT(IsValid(), "Attempting to GetProperty on an invalid material.");

	if (m_properties.find(p_key) != m_properties.end())
	{
		return m_properties.at(p_key);
	}

	return std::nullopt;
}

Rendering::Resources::Shader*& Rendering::Data::Material::GetShader()
{
	return m_shader;
}

bool Rendering::Data::Material::HasShader() const
{
	return m_shader;
}

bool Rendering::Data::Material::IsValid() const
{
	return HasShader();
}

void Rendering::Data::Material::SetBlendable(bool p_transparent)
{
	m_blendable = p_transparent;
}

void Rendering::Data::Material::SetUserInterface(bool p_userInterface)
{
	m_userInterface = p_userInterface;
}


void Rendering::Data::Material::SetBackfaceCulling(bool p_backfaceCulling)
{
	m_backfaceCulling = p_backfaceCulling;
}

void Rendering::Data::Material::SetFrontfaceCulling(bool p_frontfaceCulling)
{
	m_frontfaceCulling = p_frontfaceCulling;
}

void Rendering::Data::Material::SetDepthTest(bool p_depthTest)
{
	m_depthTest = p_depthTest;
}

void Rendering::Data::Material::SetDepthWriting(bool p_depthWriting)
{
	m_depthWriting = p_depthWriting;
}

void Rendering::Data::Material::SetColorWriting(bool p_colorWriting)
{
	m_colorWriting = p_colorWriting;
}

void Rendering::Data::Material::SetCastShadows(bool p_castShadows)
{
	m_castShadows = p_castShadows;
}

void Rendering::Data::Material::SetReceiveShadows(bool p_receiveShadows)
{
	m_receiveShadows = p_receiveShadows;
}

void Rendering::Data::Material::SetGPUInstances(int p_instances)
{
	m_gpuInstances = p_instances;
}

bool Rendering::Data::Material::IsBlendable() const
{
	return m_blendable;
}

bool Rendering::Data::Material::IsUserInterface() const
{
	return m_userInterface;
}

bool Rendering::Data::Material::HasBackfaceCulling() const
{
	return m_backfaceCulling;
}

bool Rendering::Data::Material::HasFrontfaceCulling() const
{
	return m_frontfaceCulling;
}

bool Rendering::Data::Material::HasDepthTest() const
{
	return m_depthTest;
}

bool Rendering::Data::Material::HasDepthWriting() const
{
	return m_depthWriting;
}

bool Rendering::Data::Material::HasColorWriting() const
{
	return m_colorWriting;
}

bool Rendering::Data::Material::IsShadowCaster() const
{
	return m_castShadows;
}

bool Rendering::Data::Material::IsShadowReceiver() const
{
	return m_receiveShadows;
}

int Rendering::Data::Material::GetGPUInstances() const
{
	return m_gpuInstances;
}

const Rendering::Data::StateMask Rendering::Data::Material::GenerateStateMask() const
{
	StateMask stateMask;
	stateMask.depthWriting = m_depthWriting;
	stateMask.colorWriting = m_colorWriting;
	stateMask.blendable = m_blendable;
	stateMask.depthTest = m_depthTest;
	stateMask.userInterface = m_userInterface;
	stateMask.frontfaceCulling = m_frontfaceCulling;
	stateMask.backfaceCulling = m_backfaceCulling;
	return stateMask;
}

Rendering::Data::Material::PropertyMap& Rendering::Data::Material::GetProperties()
{
	return m_properties;
}
