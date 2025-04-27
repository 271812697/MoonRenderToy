

#include "Core/Global/ServiceLocator.h"
#include "Core/ResourceManagement/TextureManager.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "Core/ResourceManagement/ShaderManager.h"
#include "Core/ECS/Components/CModelRenderer.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "Core/ECS/Actor.h"

Core::ECS::Components::CModelRenderer::CModelRenderer(ECS::Actor& p_owner) : AComponent(p_owner)
{

}

std::string Core::ECS::Components::CModelRenderer::GetName()
{
	return "Model Renderer";
}

void Core::ECS::Components::CModelRenderer::SetModel(Rendering::Resources::Model* p_model)
{
	m_model = p_model;
	if (auto materialRenderer = owner.GetComponent<CMaterialRenderer>())
		materialRenderer->UpdateMaterialList();
}

Rendering::Resources::Model * Core::ECS::Components::CModelRenderer::GetModel() const
{
	return m_model;
}

void Core::ECS::Components::CModelRenderer::SetFrustumBehaviour(EFrustumBehaviour p_boundingMode)
{
	m_frustumBehaviour = p_boundingMode;
}

Core::ECS::Components::CModelRenderer::EFrustumBehaviour Core::ECS::Components::CModelRenderer::GetFrustumBehaviour() const
{
	return m_frustumBehaviour;
}

const Rendering::Geometry::BoundingSphere& Core::ECS::Components::CModelRenderer::GetCustomBoundingSphere() const
{
	return m_customBoundingSphere;
}

void Core::ECS::Components::CModelRenderer::SetCustomBoundingSphere(const Rendering::Geometry::BoundingSphere& p_boundingSphere)
{
	m_customBoundingSphere = p_boundingSphere;
}

void Core::ECS::Components::CModelRenderer::OnSerialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
	Core::Helpers::Serializer::SerializeModel(p_doc, p_node, "model", m_model);
	Core::Helpers::Serializer::SerializeInt(p_doc, p_node, "frustum_behaviour", reinterpret_cast<int&>(m_frustumBehaviour));
	Core::Helpers::Serializer::SerializeVec3(p_doc, p_node, "custom_bounding_sphere_position", m_customBoundingSphere.position);
	Core::Helpers::Serializer::SerializeFloat(p_doc, p_node, "custom_bounding_sphere_radius", m_customBoundingSphere.radius);
}

void Core::ECS::Components::CModelRenderer::OnDeserialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode* p_node)
{
	Core::Helpers::Serializer::DeserializeModel(p_doc, p_node, "model", m_model);
	Core::Helpers::Serializer::DeserializeInt(p_doc, p_node, "frustum_behaviour", reinterpret_cast<int&>(m_frustumBehaviour));
	Core::Helpers::Serializer::DeserializeVec3(p_doc, p_node, "custom_bounding_sphere_position", m_customBoundingSphere.position);
	Core::Helpers::Serializer::DeserializeFloat(p_doc, p_node, "custom_bounding_sphere_radius", m_customBoundingSphere.radius);
}

