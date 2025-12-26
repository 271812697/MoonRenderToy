#include "Core/ECS/Components/CTransform.h"

Core::ECS::Components::CTransform::CTransform(ECS::Actor& p_owner, Maths::FVector3 p_localPosition, Maths::FQuaternion p_localRotation, Maths::FVector3 p_localScale) :
	AComponent(p_owner)
{
	m_transform.GenerateMatricesLocal(p_localPosition, p_localRotation, p_localScale);
}

std::string Core::ECS::Components::CTransform::GetName()
{
	return "Transform";
}

void Core::ECS::Components::CTransform::SetParent(CTransform& p_parent)
{
	m_transform.SetParent(p_parent.GetFTransform());
}

bool Core::ECS::Components::CTransform::RemoveParent()
{
	return m_transform.RemoveParent();
}

bool Core::ECS::Components::CTransform::HasParent() const
{
	return m_transform.HasParent();
}
void Core::ECS::Components::CTransform::SetMatrix(float data[4][4])
{
	Maths::FMatrix4 mat;
	int k = 0;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			mat.data[k++] = data[j][i];
		}
	}
	m_transform.SetLocalMatrix(mat);
}

void Core::ECS::Components::CTransform::SetMatrix(float data[16])
{
	Maths::FMatrix4 mat;
	for (int i = 0; i < 16; i++) {
		mat.data[i] = data[i];
	}
	m_transform.SetLocalMatrix(mat);
}
void Core::ECS::Components::CTransform::SetLocalPosition(Maths::FVector3 p_newPosition)
{
	m_transform.SetLocalPosition(p_newPosition);
}

void Core::ECS::Components::CTransform::SetLocalRotation(Maths::FQuaternion p_newRotation)
{
	m_transform.SetLocalRotation(p_newRotation);
}

void Core::ECS::Components::CTransform::SetLocalScale(Maths::FVector3 p_newScale)
{
	m_transform.SetLocalScale(p_newScale);
}

void Core::ECS::Components::CTransform::SetWorldPosition(Maths::FVector3 p_newPosition)
{
	m_transform.SetWorldPosition(p_newPosition);
}

void Core::ECS::Components::CTransform::SetWorldRotation(Maths::FQuaternion p_newRotation)
{
	m_transform.SetWorldRotation(p_newRotation);
}

void Core::ECS::Components::CTransform::SetWorldScale(Maths::FVector3 p_newScale)
{
	m_transform.SetWorldScale(p_newScale);
}

void Core::ECS::Components::CTransform::TranslateLocal(const Maths::FVector3& p_translation)
{
	m_transform.TranslateLocal(p_translation);
}

void Core::ECS::Components::CTransform::RotateLocal(const Maths::FQuaternion& p_rotation)
{
	m_transform.RotateLocal(p_rotation);
}

void Core::ECS::Components::CTransform::ScaleLocal(const Maths::FVector3& p_scale)
{
	m_transform.ScaleLocal(p_scale);
}

const Maths::FVector3& Core::ECS::Components::CTransform::GetLocalPosition() const
{
	return m_transform.GetLocalPosition();
}

const Maths::FQuaternion& Core::ECS::Components::CTransform::GetLocalRotation() const
{
	return m_transform.GetLocalRotation();
}

const Maths::FVector3& Core::ECS::Components::CTransform::GetLocalScale() const
{
	return m_transform.GetLocalScale();
}

const Maths::FVector3& Core::ECS::Components::CTransform::GetWorldPosition() const
{
	return m_transform.GetWorldPosition();
}

const Maths::FQuaternion& Core::ECS::Components::CTransform::GetWorldRotation() const
{
	return m_transform.GetWorldRotation();
}

const Maths::FVector3& Core::ECS::Components::CTransform::GetWorldScale() const
{
	return m_transform.GetWorldScale();
}

const Maths::FMatrix4& Core::ECS::Components::CTransform::GetLocalMatrix() const
{
	return m_transform.GetLocalMatrix();
}

const Maths::FMatrix4& Core::ECS::Components::CTransform::GetWorldMatrix() const
{
	return m_transform.GetWorldMatrix();
}

Maths::FTransform& Core::ECS::Components::CTransform::GetFTransform()
{
	return m_transform;
}

Maths::FVector3 Core::ECS::Components::CTransform::GetWorldForward() const
{
	return m_transform.GetWorldForward();
}

Maths::FVector3 Core::ECS::Components::CTransform::GetWorldUp() const
{
	return m_transform.GetWorldUp();
}

Maths::FVector3 Core::ECS::Components::CTransform::GetWorldRight() const
{
	return m_transform.GetWorldRight();
}

Maths::FVector3 Core::ECS::Components::CTransform::GetLocalForward() const
{
	return m_transform.GetLocalForward();
}

Maths::FVector3 Core::ECS::Components::CTransform::GetLocalUp() const
{
	return m_transform.GetLocalUp();
}

Maths::FVector3 Core::ECS::Components::CTransform::GetLocalRight() const
{
	return m_transform.GetLocalRight();
}

void Core::ECS::Components::CTransform::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	Core::Helpers::Serializer::SerializeVec3(p_doc, p_node, "position", GetLocalPosition());
	Core::Helpers::Serializer::SerializeQuat(p_doc, p_node, "rotation", GetLocalRotation());
	Core::Helpers::Serializer::SerializeVec3(p_doc, p_node, "scale", GetLocalScale());
}

void Core::ECS::Components::CTransform::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
	m_transform.GenerateMatricesLocal
	(
		Core::Helpers::Serializer::DeserializeVec3(p_doc, p_node, "position"),
		Core::Helpers::Serializer::DeserializeQuat(p_doc, p_node, "rotation"),
		Core::Helpers::Serializer::DeserializeVec3(p_doc, p_node, "scale")
	);
}

