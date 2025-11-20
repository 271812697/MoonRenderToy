#pragma once
#include "Core/ECS/Components/AComponent.h"
#include <Maths/FTransform.h>
#include <Maths/FVector3.h>
#include <Maths/FQuaternion.h>
namespace Core::ECS { class Actor; }
namespace Core::ECS::Components
{
	class CTransform : public AComponent
	{
	public:
		CTransform(ECS::Actor& p_owner, struct Maths::FVector3 p_localPosition = Maths::FVector3(0.0f, 0.0f, 0.0f), Maths::FQuaternion p_localRotation = Maths::FQuaternion::Identity, struct Maths::FVector3 p_localScale = Maths::FVector3(1.0f, 1.0f, 1.0f));
		std::string GetName() override;
		void SetParent(CTransform& p_parent);
		bool RemoveParent();
		bool HasParent() const;
		void SetMatrix(float data[4][4]);
		void SetMatrix(float data[16]);
		void SetLocalPosition(struct Maths::FVector3 p_newPosition);
		void SetLocalRotation(Maths::FQuaternion p_newRotation);
		void SetLocalScale(struct Maths::FVector3 p_newScale);
		void SetWorldPosition(struct Maths::FVector3 p_newPosition);
		void SetWorldRotation(Maths::FQuaternion p_newRotation);
		void SetWorldScale(struct Maths::FVector3 p_newScale);
		void TranslateLocal(const struct Maths::FVector3& p_translation);
		void RotateLocal(const Maths::FQuaternion& p_rotation);
		void ScaleLocal(const struct Maths::FVector3& p_scale);
		const Maths::FVector3& GetLocalPosition() const;
		const Maths::FQuaternion& GetLocalRotation() const;
		const Maths::FVector3& GetLocalScale() const;
		const Maths::FVector3& GetWorldPosition() const;
		const Maths::FQuaternion& GetWorldRotation() const;
		const Maths::FVector3& GetWorldScale() const;
		const Maths::FMatrix4& GetLocalMatrix() const;
		const Maths::FMatrix4& GetWorldMatrix() const;
		Maths::FTransform& GetFTransform();
		Maths::FVector3 GetWorldForward() const;
		Maths::FVector3 GetWorldUp() const;
		Maths::FVector3 GetWorldRight() const;
		Maths::FVector3 GetLocalForward() const;
		Maths::FVector3 GetLocalUp() const;
		Maths::FVector3 GetLocalRight() const;
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;
	private:
		Maths::FTransform m_transform;
	};
}