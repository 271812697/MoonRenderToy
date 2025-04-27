

#include "Core/ECS/Actor.h"

#include "Core/ECS/Components/CDirectionalLight.h"

Core::ECS::Components::CDirectionalLight::CDirectionalLight(ECS::Actor & p_owner) :
	CLight(p_owner)
{
	m_data.type = static_cast<float>(Rendering::Entities::Light::Type::DIRECTIONAL);
}

std::string Core::ECS::Components::CDirectionalLight::GetName()
{
	return "Directional Light";
}

void Core::ECS::Components::CDirectionalLight::OnSerialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	CLight::OnSerialize(p_doc, p_node);
}

void Core::ECS::Components::CDirectionalLight::OnDeserialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	CLight::OnDeserialize(p_doc, p_node);
}


