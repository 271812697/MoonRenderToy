#include "Core/ECS/Actor.h"

#include "Core/ECS/Components/CSpotLight.h"

Core::ECS::Components::CSpotLight::CSpotLight(ECS::Actor& p_owner) :
	CLight(p_owner)
{
	m_data.type = Rendering::Settings::ELightType::SPOT;
}

std::string Core::ECS::Components::CSpotLight::GetName()
{
	return "Spot Light";
}

float Core::ECS::Components::CSpotLight::GetConstant() const
{
	return m_data.constant;
}

float Core::ECS::Components::CSpotLight::GetLinear() const
{
	return m_data.linear;
}

float Core::ECS::Components::CSpotLight::GetQuadratic() const
{
	return m_data.quadratic;
}

float Core::ECS::Components::CSpotLight::GetCutoff() const
{
	return m_data.cutoff;
}

float Core::ECS::Components::CSpotLight::GetOuterCutoff() const
{
	return m_data.outerCutoff;
}

void Core::ECS::Components::CSpotLight::SetConstant(float p_constant)
{
	m_data.constant = p_constant;
}

void Core::ECS::Components::CSpotLight::SetLinear(float p_linear)
{
	m_data.linear = p_linear;
}

void Core::ECS::Components::CSpotLight::SetQuadratic(float p_quadratic)
{
	m_data.quadratic = p_quadratic;
}

void Core::ECS::Components::CSpotLight::SetCutoff(float p_cutoff)
{
	m_data.cutoff = p_cutoff;
}

void Core::ECS::Components::CSpotLight::SetOuterCutoff(float p_outerCutoff)
{
	m_data.outerCutoff = p_outerCutoff;
}

void Core::ECS::Components::CSpotLight::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	CLight::OnSerialize(p_doc, p_node);

	Serializer::SerializeFloat(p_doc, p_node, "constant", m_data.constant);
	Serializer::SerializeFloat(p_doc, p_node, "linear", m_data.linear);
	Serializer::SerializeFloat(p_doc, p_node, "quadratic", m_data.quadratic);
	Serializer::SerializeFloat(p_doc, p_node, "cutoff", m_data.cutoff);
	Serializer::SerializeFloat(p_doc, p_node, "outercutoff", m_data.outerCutoff);
}

void Core::ECS::Components::CSpotLight::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	using namespace Core::Helpers;

	CLight::OnDeserialize(p_doc, p_node);

	Serializer::DeserializeFloat(p_doc, p_node, "constant", m_data.constant);
	Serializer::DeserializeFloat(p_doc, p_node, "linear", m_data.linear);
	Serializer::DeserializeFloat(p_doc, p_node, "quadratic", m_data.quadratic);
	Serializer::DeserializeFloat(p_doc, p_node, "cutoff", m_data.cutoff);
	Serializer::DeserializeFloat(p_doc, p_node, "outercutoff", m_data.outerCutoff);
}
