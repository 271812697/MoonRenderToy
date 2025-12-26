#include "Core/ECS/Actor.h"
#include "Core/ECS/Components/CAmbientBoxLight.h"

Core::ECS::Components::CAmbientBoxLight::CAmbientBoxLight(ECS::Actor& p_owner) :
	CLight(p_owner)
{
	m_data.type = Rendering::Settings::ELightType::AMBIENT_BOX;

	m_data.intensity = 0.03f;
	m_data.constant = 1.0f;
	m_data.linear = 1.0f;
	m_data.quadratic = 1.0f;
}

std::string Core::ECS::Components::CAmbientBoxLight::GetName()
{
	return "Ambient Box Light";
}

Maths::FVector3 Core::ECS::Components::CAmbientBoxLight::GetSize() const
{
	return { m_data.constant, m_data.linear, m_data.quadratic };
}

void Core::ECS::Components::CAmbientBoxLight::SetSize(const Maths::FVector3& p_size)
{
	m_data.constant = p_size.x;
	m_data.linear = p_size.y;
	m_data.quadratic = p_size.z;
}

void Core::ECS::Components::CAmbientBoxLight::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	CLight::OnSerialize(p_doc, p_node);

	Serializer::SerializeVec3(p_doc, p_node, "size", { m_data.constant, m_data.linear, m_data.quadratic });
}

void Core::ECS::Components::CAmbientBoxLight::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	CLight::OnDeserialize(p_doc, p_node);

	Maths::FVector3 size = Serializer::DeserializeVec3(p_doc, p_node, "size");
	m_data.constant = size.x;
	m_data.linear = size.y;
	m_data.quadratic = size.z;
}

