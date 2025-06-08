/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#include <tinyxml2.h>

#include <OvCore/ECS/Actor.h>
#include <OvCore/ECS/Components/CMaterialRenderer.h>
#include <OvCore/ECS/Components/CModelRenderer.h>
#include <OvCore/Global/ServiceLocator.h>
#include <OvCore/ResourceManagement/MaterialManager.h>

#include <OvTools/Utils/PathParser.h>


OvCore::ECS::Components::CMaterialRenderer::CMaterialRenderer(ECS::Actor& p_owner) : AComponent(p_owner)
{
	m_materials.fill(nullptr);


	UpdateMaterialList();
}

std::string OvCore::ECS::Components::CMaterialRenderer::GetName()
{
	return "Material Renderer";
}

void OvCore::ECS::Components::CMaterialRenderer::FillWithMaterial(OvCore::Resources::Material& p_material)
{
	for (uint8_t i = 0; i < m_materials.size(); ++i)
		m_materials[i] = &p_material;
}

void OvCore::ECS::Components::CMaterialRenderer::SetMaterialAtIndex(uint8_t p_index, OvCore::Resources::Material& p_material)
{
	m_materials[p_index] = &p_material;
}

OvCore::Resources::Material* OvCore::ECS::Components::CMaterialRenderer::GetMaterialAtIndex(uint8_t p_index)
{
	return m_materials.at(p_index);
}

void OvCore::ECS::Components::CMaterialRenderer::RemoveMaterialAtIndex(uint8_t p_index)
{
	if (p_index < m_materials.size())
	{
		m_materials[p_index] = nullptr;;
	}
}

void OvCore::ECS::Components::CMaterialRenderer::RemoveMaterialByInstance(OvCore::Resources::Material& p_instance)
{
	for (uint8_t i = 0; i < m_materials.size(); ++i)
		if (m_materials[i] == &p_instance)
			m_materials[i] = nullptr;
}

void OvCore::ECS::Components::CMaterialRenderer::RemoveAllMaterials()
{
	for (uint8_t i = 0; i < m_materials.size(); ++i)
		m_materials[i] = nullptr;
}

const OvMaths::FMatrix4& OvCore::ECS::Components::CMaterialRenderer::GetUserMatrix() const
{
	return m_userMatrix;
}

const OvCore::ECS::Components::CMaterialRenderer::MaterialList& OvCore::ECS::Components::CMaterialRenderer::GetMaterials() const
{
	return m_materials;
}

void OvCore::ECS::Components::CMaterialRenderer::SetVisibilityFlags(OvCore::Rendering::EVisibilityFlags p_flags)
{
	m_visibilityFlags = p_flags;
}

OvCore::Rendering::EVisibilityFlags OvCore::ECS::Components::CMaterialRenderer::GetVisibilityFlags() const
{
	return m_visibilityFlags;
}

bool OvCore::ECS::Components::CMaterialRenderer::HasVisibilityFlags(OvCore::Rendering::EVisibilityFlags p_flags) const
{
	return OvCore::Rendering::SatisfiesVisibility(m_visibilityFlags, p_flags);
}

void OvCore::ECS::Components::CMaterialRenderer::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	tinyxml2::XMLNode* materialsNode = p_doc.NewElement("materials");
	p_node->InsertEndChild(materialsNode);

	auto modelRenderer = owner.GetComponent<CModelRenderer>();
	uint8_t elementsToSerialize = modelRenderer->GetModel() ? (uint8_t)std::min(modelRenderer->GetModel()->GetMaterialNames().size(), (size_t)kMaxMaterialCount) : 0;

	for (uint8_t i = 0; i < elementsToSerialize; ++i)
	{
		OvCore::Helpers::Serializer::SerializeMaterial(p_doc, materialsNode, "material", m_materials[i]);
	}

	OvCore::Helpers::Serializer::SerializeUint32(p_doc, p_node, "visibility_flags", reinterpret_cast<uint32_t&>(m_visibilityFlags));
}

void OvCore::ECS::Components::CMaterialRenderer::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	tinyxml2::XMLNode* materialsRoot = p_node->FirstChildElement("materials");
	if (materialsRoot)
	{
		tinyxml2::XMLElement* currentMaterial = materialsRoot->FirstChildElement("material");

		uint8_t materialIndex = 0;

		while (currentMaterial)
		{
			if (auto material = Global::ServiceLocator::Get<ResourceManagement::MaterialManager>()[currentMaterial->GetText()])
				m_materials[materialIndex] = material;

			currentMaterial = currentMaterial->NextSiblingElement("material");
			++materialIndex;
		}
	}

	UpdateMaterialList();

	OvCore::Helpers::Serializer::DeserializeUint32(p_doc, p_node, "visibility_flags", reinterpret_cast<uint32_t&>(m_visibilityFlags));
}




void OvCore::ECS::Components::CMaterialRenderer::UpdateMaterialList()
{
	if (auto modelRenderer = owner.GetComponent<CModelRenderer>(); modelRenderer && modelRenderer->GetModel())
	{
		uint8_t materialIndex = 0;

		for (const std::string& materialName : modelRenderer->GetModel()->GetMaterialNames())
		{
			m_materialNames[materialIndex++] = materialName;
		}

		for (uint8_t i = materialIndex; i < kMaxMaterialCount; ++i)
			m_materialNames[i] = "";
	}


}

void OvCore::ECS::Components::CMaterialRenderer::SetUserMatrixElement(uint32_t p_row, uint32_t p_column, float p_value)
{
	if (p_row < 4 && p_column < 4)
		m_userMatrix.data[4 * p_row + p_column] = p_value;
}

float OvCore::ECS::Components::CMaterialRenderer::GetUserMatrixElement(uint32_t p_row, uint32_t p_column) const
{
	if (p_row < 4 && p_column < 4)
		return m_userMatrix.data[4 * p_row + p_column];
	else
		return 0.0f;
}
