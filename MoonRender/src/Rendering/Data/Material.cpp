#include <format>
#include <ranges>
#include <tracy/Tracy.hpp>
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

		if (auto value = as.operator() < bool > ()) return *value;
		if (auto value = as.operator() < int > ()) return *value;
		if (auto value = as.operator() < float > ()) return *value;
		if (auto value = as.operator() < FVector2 > ()) return *value;
		if (auto value = as.operator() < FVector3 > ()) return *value;
		if (auto value = as.operator() < FVector4 > ()) return *value;
		if (auto value = as.operator() < FMatrix3 > ()) return *value;
		if (auto value = as.operator() < FMatrix4 > ()) return *value;
		if (auto value = as.operator() < HAL::TextureHandle* > ()) return *value;
		if (auto value = as.operator() < Resources::Texture* > ()) return *value;

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
		m_properties.clear();
		UpdateProperties();
	}
	else
	{
		m_properties.clear();
	}
}

Tools::Utils::OptRef<Rendering::HAL::ShaderProgram> Rendering::Data::Material::GetVariant(
	std::optional<const std::string_view> p_pass,
	Tools::Utils::OptRef<const Data::FeatureSet> p_override
) const
{
	if (m_shader)
	{
		return m_shader->GetVariant(
			p_pass,
			p_override.value_or(m_features)
		);
	}

	return std::nullopt;
}

void Rendering::Data::Material::UpdateProperties()
{
	// Collect all uniform names currently used by the shader
	std::unordered_set<std::string> usedUniforms;

	auto variants_view = m_shader->GetVariants()
		| std::views::values
		| std::views::join
		| std::views::values;

	for (const auto& variant : variants_view)
	{
		for (const auto& [name, uniformInfo] : variant->GetUniforms())
		{
			usedUniforms.insert(name);

			if (!m_properties.contains(name))
			{
				m_properties.emplace(name, MaterialProperty{
					.value = UniformToPropertyValue(uniformInfo.defaultValue),
					.singleUse = false
					});
			}
		}
	}

	std::erase_if(m_properties, [&usedUniforms](const auto& property) {
		return !usedUniforms.contains(property.first);
		});
}
// Note: this function is critical for performance, as it may be called many times during a frame.
// Avoid using any heavy operations or allocations inside this function.
void Rendering::Data::Material::Bind(
	HAL::Texture* p_emptyTexture,
	HAL::Texture* p_emptyTextureCube,
	std::optional<const std::string_view> p_pass,
	Tools::Utils::OptRef<const Data::FeatureSet> p_featureSetOverride
)
{
	ZoneScoped;

	using namespace Maths;
	using enum Rendering::Settings::EUniformType;
	auto& program = m_shader->GetVariant(
		p_pass,
		p_featureSetOverride.value_or(m_features)
	);

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

		// Iterating over the properties to set them in the shader.
		// This could have been cleaner with a visitor, but the performance impact
		// is not worth it. This is a critical path in the rendering pipeline.

		if (uniformType == BOOL)
		{
			program.SetUniform<int>(name, static_cast<int>(std::get<bool>(value)));
		}
		else if (uniformType == INT)
		{
			program.SetUniform<int>(name, std::get<int>(value));
		}
		else if (uniformType == FLOAT)
		{
			program.SetUniform<float>(name, std::get<float>(value));
		}
		else if (uniformType == FLOAT_VEC2)
		{
			program.SetUniform<FVector2>(name, std::get<FVector2>(value));
		}
		else if (uniformType == FLOAT_VEC3)
		{
			program.SetUniform<FVector3>(name, std::get<FVector3>(value));
		}
		else if (uniformType == FLOAT_VEC4)
		{
			program.SetUniform<FVector4>(name, std::get<FVector4>(value));
		}
		else if (uniformType == FLOAT_MAT3)
		{
			program.SetUniform<FMatrix3>(name, std::get<FMatrix3>(value));
		}
		else if (uniformType == FLOAT_MAT4)
		{
			program.SetUniform<FMatrix4>(name, std::get<FMatrix4>(value));
		}
		else if (
			uniformType == SAMPLER_2D || uniformType == SAMPLER_CUBE
			|| uniformType ==SAMPLER_BUFFER ||uniformType==UINTSAMPLER_BUFFER 
			|| uniformType == SAMPLER_2DARRAY || uniformType == INTSAMPLER_BUFFER
			)
		{
			HAL::TextureHandle* handle = nullptr;
			if (auto textureHandle = std::get_if<HAL::TextureHandle*>(&value))
			{
				handle = *textureHandle;
			}
			else if (auto texture = std::get_if<Resources::Texture*>(&value))
			{
				if (*texture != nullptr)
				{
					handle = &(*texture)->GetTexture();
				}
			}
			BindTexture(program, name, handle, uniformType == SAMPLER_2D ? p_emptyTexture : p_emptyTextureCube, textureSlot);
		}

		if (prop.singleUse)
		{
			value = UniformToPropertyValue(uniformData->defaultValue);
		}
	}
}

void Rendering::Data::Material::Unbind() const
{
	m_shader->GetVariant().Unbind();
}

bool Rendering::Data::Material::HasProperty(const std::string& p_name) const
{
	return m_properties.contains(p_name);
}

void Rendering::Data::Material::SetProperty(const std::string p_name, const MaterialPropertyType& p_value, bool p_singleUse)
{
	const auto property =
		m_properties[p_name] = MaterialProperty{
			p_value,
			p_singleUse
	};
}

bool Rendering::Data::Material::TrySetProperty(const std::string& p_name, const MaterialPropertyType& p_value, bool p_singleUse)
{
	if (HasProperty(p_name))
	{
		SetProperty(p_name, p_value, p_singleUse);
		return true;
	}
	return false;
}

Tools::Utils::OptRef<const Rendering::Data::MaterialProperty> Rendering::Data::Material::GetProperty(const std::string p_key) const
{
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

void Rendering::Data::Material::SetOrthographicSupport(bool p_supportOrthographic)
{
	m_supportOrthographic = p_supportOrthographic;
}

void Rendering::Data::Material::SetPerspectiveSupport(bool p_supportPerspective)
{
	m_supportPerspective = p_supportPerspective;
}

void Rendering::Data::Material::SetDrawOrder(int p_order)
{
	m_drawOrder = p_order;
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

void Rendering::Data::Material::SetCapturedByReflectionProbes(bool p_capturedByReflectionProbes)
{
	m_capturedByReflectionProbes = p_capturedByReflectionProbes;
}

void Rendering::Data::Material::SetReceiveReflections(bool p_receiveReflections)
{
	m_receiveReflections = p_receiveReflections;
}

void Rendering::Data::Material::SetGPUInstances(int p_instances)
{
	m_gpuInstances = p_instances;
}

int Rendering::Data::Material::GetDrawOrder() const
{
	return m_drawOrder;
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

bool Rendering::Data::Material::IsCapturedByReflectionProbes() const
{
	return m_capturedByReflectionProbes;
}

bool Rendering::Data::Material::IsReflectionReceiver() const
{
	return m_receiveReflections;
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
	stateMask.frontfaceCulling = m_frontfaceCulling;
	stateMask.backfaceCulling = m_backfaceCulling;
	stateMask.lineWidth = lineWitdh;
	return stateMask;
}

Rendering::Data::Material::PropertyMap& Rendering::Data::Material::GetProperties()
{
	return m_properties;
}

Rendering::Data::FeatureSet& Rendering::Data::Material::GetFeatures()
{
	return m_features;
}

void Rendering::Data::Material::SetFeatures(const Data::FeatureSet& p_features)
{
	m_features = p_features;
}

void Rendering::Data::Material::AddFeature(const std::string& p_feature)
{
	m_features.insert(p_feature);
}

void Rendering::Data::Material::RemoveFeature(const std::string& p_feature)
{
	m_features.erase(p_feature);
}
void Rendering::Data::Material::EnableFeature(const std::string& p_feature,bool flag)
{
	if (flag) {
		m_features.insert(p_feature);
	}
	else
	{
		m_features.erase(p_feature);
	}
}

bool Rendering::Data::Material::HasFeature(const std::string& p_feature) const
{
	return m_features.contains(p_feature);
}

bool Rendering::Data::Material::SupportsFeature(const std::string& p_feature) const
{
	return m_shader->GetFeatures().contains(p_feature);
}

bool Rendering::Data::Material::HasPass(const std::string& p_pass) const
{
	return m_shader->GetPasses().contains(p_pass);
}

bool Rendering::Data::Material::SupportsOrthographic() const
{
	return m_supportOrthographic;
}

bool Rendering::Data::Material::SupportsPerspective() const
{
	return m_supportPerspective;
}

bool Rendering::Data::Material::SupportsProjectionMode(Rendering::Settings::EProjectionMode p_projectionMode) const
{
	using enum Rendering::Settings::EProjectionMode;
	switch (p_projectionMode)
	{
	case ORTHOGRAPHIC: return SupportsOrthographic();
	case PERSPECTIVE: return SupportsPerspective();
	}
	return true;
}

void Rendering::Data::Material::SetLineWidth(float p_width)
{
	lineWitdh = p_width;
}
