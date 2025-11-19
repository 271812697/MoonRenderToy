#include "Core/ECS/Actor.h"

#include "Core/ECS/Components/CLight.h"

Core::ECS::Components::CLight::CLight(ECS::Actor& p_owner) :
	AComponent(p_owner),
	m_data{ p_owner.transform.GetFTransform() }
{
}

Rendering::Entities::Light& Core::ECS::Components::CLight::GetData()
{
	return m_data;
}

const Maths::FVector3& Core::ECS::Components::CLight::GetColor() const
{
	return m_data.color;
}

float Core::ECS::Components::CLight::GetIntensity() const
{
	return m_data.intensity;
}

void Core::ECS::Components::CLight::SetColor(const Maths::FVector3& p_color)
{
	m_data.color = p_color;
}

void Core::ECS::Components::CLight::SetIntensity(float p_intensity)
{
	m_data.intensity = p_intensity;
}

void Core::ECS::Components::CLight::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	Serializer::SerializeVec3(p_doc, p_node, "color", m_data.color);
	Serializer::SerializeFloat(p_doc, p_node, "intensity", m_data.intensity);
}

void Core::ECS::Components::CLight::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	Serializer::DeserializeVec3(p_doc, p_node, "color", m_data.color);
	Serializer::DeserializeFloat(p_doc, p_node, "intensity", m_data.intensity);
}

