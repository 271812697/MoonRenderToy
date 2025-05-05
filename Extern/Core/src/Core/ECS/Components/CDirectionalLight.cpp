/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/



#include "Core/ECS/Actor.h"

#include "Core/ECS/Components/CDirectionalLight.h"

Core::ECS::Components::CDirectionalLight::CDirectionalLight(ECS::Actor& p_owner) :
	CLight(p_owner)
{
	m_data.type = ::Rendering::Settings::ELightType::DIRECTIONAL;
}

std::string Core::ECS::Components::CDirectionalLight::GetName()
{
	return "Directional Light";
}

void Core::ECS::Components::CDirectionalLight::SetCastShadows(bool p_enabled)
{
	m_data.castShadows = p_enabled;
}

bool Core::ECS::Components::CDirectionalLight::GetCastShadows() const
{
	return m_data.castShadows;
}

void Core::ECS::Components::CDirectionalLight::SetShadowAreaSize(float p_shadowAreaSize)
{
	m_data.shadowAreaSize = p_shadowAreaSize;
}

float Core::ECS::Components::CDirectionalLight::GetShadowAreaSize() const
{
	return m_data.shadowAreaSize;
}

void Core::ECS::Components::CDirectionalLight::SetShadowFollowCamera(bool p_enabled)
{
	m_data.shadowFollowCamera = p_enabled;
}

bool Core::ECS::Components::CDirectionalLight::GetShadowFollowCamera() const
{
	return m_data.shadowFollowCamera;
}

void Core::ECS::Components::CDirectionalLight::SetShadowMapResolution(uint32_t p_resolution)
{
	m_data.shadowMapResolution = p_resolution;
}

uint32_t Core::ECS::Components::CDirectionalLight::GetShadowMapResolution() const
{
	return m_data.shadowMapResolution;
}

void Core::ECS::Components::CDirectionalLight::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	CLight::OnSerialize(p_doc, p_node);
	Core::Helpers::Serializer::SerializeBoolean(p_doc, p_node, "cast_shadows", m_data.castShadows);
	Core::Helpers::Serializer::SerializeFloat(p_doc, p_node, "shadow_area_size", m_data.shadowAreaSize);
	Core::Helpers::Serializer::SerializeBoolean(p_doc, p_node, "shadow_follow_camera", m_data.shadowFollowCamera);
	Core::Helpers::Serializer::SerializeInt(p_doc, p_node, "shadow_map_resolution", m_data.shadowMapResolution);
}

void Core::ECS::Components::CDirectionalLight::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	CLight::OnDeserialize(p_doc, p_node);
	m_data.castShadows = Core::Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "cast_shadows");
	m_data.shadowAreaSize = Core::Helpers::Serializer::DeserializeFloat(p_doc, p_node, "shadow_area_size");
	m_data.shadowFollowCamera = Core::Helpers::Serializer::DeserializeBoolean(p_doc, p_node, "shadow_follow_camera");
	m_data.shadowMapResolution = Core::Helpers::Serializer::DeserializeInt(p_doc, p_node, "shadow_map_resolution");
}
