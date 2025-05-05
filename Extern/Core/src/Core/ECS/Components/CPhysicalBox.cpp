/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Physics/Entities/PhysicalBox.h>

#include "Core/ECS/Components/CPhysicalBox.h"
#include "Core/ECS/Actor.h"

using namespace Physics::Entities;

Core::ECS::Components::CPhysicalBox::CPhysicalBox(ECS::Actor& p_owner) :
	CPhysicalObject(p_owner)
{
	m_physicalObject = std::make_unique<Physics::Entities::PhysicalBox>(p_owner.transform.GetFTransform());

	m_physicalObject->SetUserData<std::reference_wrapper<CPhysicalObject>>(*this);

	BindListener();
	Init();
}

std::string Core::ECS::Components::CPhysicalBox::GetName()
{
	return "Physical Box";
}

void Core::ECS::Components::CPhysicalBox::SetSize(const Maths::FVector3& p_size)
{
	GetPhysicalObjectAs<PhysicalBox>().SetSize(p_size);
}

Maths::FVector3 Core::ECS::Components::CPhysicalBox::GetSize() const
{
	return GetPhysicalObjectAs<PhysicalBox>().GetSize();
}

void Core::ECS::Components::CPhysicalBox::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	CPhysicalObject::OnSerialize(p_doc, p_node);

	Helpers::Serializer::SerializeVec3(p_doc, p_node, "size", GetSize());
}

void Core::ECS::Components::CPhysicalBox::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	CPhysicalObject::OnDeserialize(p_doc, p_node);

	SetSize(Helpers::Serializer::DeserializeVec3(p_doc, p_node, "size"));
}

