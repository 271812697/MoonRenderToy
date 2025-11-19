#include "Core/ECS/Actor.h"
#include "Core/ECS/Components/CAmbientSphereLight.h"

Core::ECS::Components::CAmbientSphereLight::CAmbientSphereLight(ECS::Actor& p_owner) :
	CLight(p_owner)
{
	m_data.intensity = 0.03f;
	m_data.constant = 1.0f;

	m_data.type = Rendering::Settings::ELightType::AMBIENT_SPHERE;
}

std::string Core::ECS::Components::CAmbientSphereLight::GetName()
{
	return "Ambient Sphere Light";
}

float Core::ECS::Components::CAmbientSphereLight::GetRadius() const
{
	return m_data.quadratic;
}

void Core::ECS::Components::CAmbientSphereLight::SetRadius(float p_radius)
{
	m_data.constant = p_radius;
}

void Core::ECS::Components::CAmbientSphereLight::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	CLight::OnSerialize(p_doc, p_node);

	Serializer::SerializeFloat(p_doc, p_node, "radius", m_data.constant);
}

void Core::ECS::Components::CAmbientSphereLight::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	CLight::OnDeserialize(p_doc, p_node);

	Serializer::DeserializeFloat(p_doc, p_node, "radius", m_data.constant);
}

