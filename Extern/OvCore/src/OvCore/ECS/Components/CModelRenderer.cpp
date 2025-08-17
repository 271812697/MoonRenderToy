



#include "OvCore/Global/ServiceLocator.h"
#include "OvCore/ResourceManagement/TextureManager.h"
#include "OvCore/ResourceManagement/ModelManager.h"
#include "OvCore/ResourceManagement/ShaderManager.h"
#include "OvCore/ECS/Components/CModelRenderer.h"
#include "OvCore/ECS/Components/CMaterialRenderer.h"
#include "OvCore/ECS/Actor.h"

OvCore::ECS::Components::CModelRenderer::CModelRenderer(ECS::Actor& p_owner) : AComponent(p_owner)
{

}

std::string OvCore::ECS::Components::CModelRenderer::GetName()
{
	return "Model Renderer";
}

void OvCore::ECS::Components::CModelRenderer::SetModel(OvRendering::Resources::Model* p_model)
{
	m_model = p_model;
	if (auto materialRenderer = owner.GetComponent<CMaterialRenderer>())
		materialRenderer->UpdateMaterialList();

}

OvRendering::Resources::Model* OvCore::ECS::Components::CModelRenderer::GetModel() const
{
	return m_model;
}

void OvCore::ECS::Components::CModelRenderer::SetFrustumBehaviour(EFrustumBehaviour p_boundingMode)
{
	m_frustumBehaviour = p_boundingMode;
}

OvCore::ECS::Components::CModelRenderer::EFrustumBehaviour OvCore::ECS::Components::CModelRenderer::GetFrustumBehaviour() const
{
	return m_frustumBehaviour;
}

const OvRendering::Geometry::BoundingSphere& OvCore::ECS::Components::CModelRenderer::GetCustomBoundingSphere() const
{
	return m_customBoundingSphere;
}

void OvCore::ECS::Components::CModelRenderer::SetCustomBoundingSphere(const OvRendering::Geometry::BoundingSphere& p_boundingSphere)
{
	m_customBoundingSphere = p_boundingSphere;
}

void OvCore::ECS::Components::CModelRenderer::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	OvCore::Helpers::Serializer::SerializeModel(p_doc, p_node, "model", m_model);
	OvCore::Helpers::Serializer::SerializeInt(p_doc, p_node, "frustum_behaviour", reinterpret_cast<int&>(m_frustumBehaviour));
	OvCore::Helpers::Serializer::SerializeVec3(p_doc, p_node, "custom_bounding_sphere_position", m_customBoundingSphere.position);
	OvCore::Helpers::Serializer::SerializeFloat(p_doc, p_node, "custom_bounding_sphere_radius", m_customBoundingSphere.radius);
}

void OvCore::ECS::Components::CModelRenderer::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	OvCore::Helpers::Serializer::DeserializeModel(p_doc, p_node, "model", m_model);
	OvCore::Helpers::Serializer::DeserializeInt(p_doc, p_node, "frustum_behaviour", reinterpret_cast<int&>(m_frustumBehaviour));
	OvCore::Helpers::Serializer::DeserializeVec3(p_doc, p_node, "custom_bounding_sphere_position", m_customBoundingSphere.position);
	OvCore::Helpers::Serializer::DeserializeFloat(p_doc, p_node, "custom_bounding_sphere_radius", m_customBoundingSphere.radius);
}
