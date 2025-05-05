/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/



#include "Core/ECS/Actor.h"

#include "Core/ECS/Components/CPointLight.h"

Core::ECS::Components::CPointLight::CPointLight(ECS::Actor& p_owner) :
	CLight(p_owner)
{
	m_data.type = ::Rendering::Settings::ELightType::POINT;
}

std::string Core::ECS::Components::CPointLight::GetName()
{
	return "Point Light";
}

float Core::ECS::Components::CPointLight::GetConstant() const
{
	return m_data.constant;
}

float Core::ECS::Components::CPointLight::GetLinear() const
{
	return m_data.linear;
}

float Core::ECS::Components::CPointLight::GetQuadratic() const
{
	return m_data.quadratic;
}

void Core::ECS::Components::CPointLight::SetConstant(float p_constant)
{
	m_data.constant = p_constant;
}

void Core::ECS::Components::CPointLight::SetLinear(float p_linear)
{
	m_data.linear = p_linear;
}

void Core::ECS::Components::CPointLight::SetQuadratic(float p_quadratic)
{
	m_data.quadratic = p_quadratic;
}

void Core::ECS::Components::CPointLight::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	CLight::OnSerialize(p_doc, p_node);

	Serializer::SerializeFloat(p_doc, p_node, "constant", m_data.constant);
	Serializer::SerializeFloat(p_doc, p_node, "linear", m_data.linear);
	Serializer::SerializeFloat(p_doc, p_node, "quadratic", m_data.quadratic);
}

void Core::ECS::Components::CPointLight::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	CLight::OnDeserialize(p_doc, p_node);

	Serializer::DeserializeFloat(p_doc, p_node, "constant", m_data.constant);
	Serializer::DeserializeFloat(p_doc, p_node, "linear", m_data.linear);
	Serializer::DeserializeFloat(p_doc, p_node, "quadratic", m_data.quadratic);
}

