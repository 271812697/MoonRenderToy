/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include "Core/ECS/Components/AComponent.h"

#include <Maths/FTransform.h>
#include <Maths/FVector3.h>
#include <Maths/FQuaternion.h>

#include "AComponent.h"

namespace Core::ECS { class Actor; }

namespace Core::ECS::Components
{
	/**
	* Represents the 3D transformations applied to an actor
	*/
	class CTransform : public AComponent
	{
	public:
		/**
		* Create a transform without setting a parent
		* @param p_localPosition
		* @param p_localRotation
		* @param p_localScale
		*/
		CTransform(ECS::Actor& p_owner, struct Maths::FVector3 p_localPosition = Maths::FVector3(0.0f, 0.0f, 0.0f), Maths::FQuaternion p_localRotation = Maths::FQuaternion::Identity, struct Maths::FVector3 p_localScale = Maths::FVector3(1.0f, 1.0f, 1.0f));

		/**
		* Returns the name of the component
		*/
		std::string GetName() override;

		/**
		* Defines a parent to the transform
		* @param p_parent
		*/
		void SetParent(CTransform& p_parent);

		/**
		* Set the parent to nullptr and recalculate world matrix
		* Returns true on success
		*/
		bool RemoveParent();

		/**
		* Check if the transform has a parent
		*/
		bool HasParent() const;
		void SetMatrix(float data[4][4]);
		void SetMatrix(float data[16]);
		/**
		* Set the position of the transform in the local space
		* @param p_newPosition
		*/
		void SetLocalPosition(struct Maths::FVector3 p_newPosition);

		/**
		* Set the rotation of the transform in the local space
		* @param p_newRotation
		*/
		void SetLocalRotation(Maths::FQuaternion p_newRotation);

		/**
		* Set the scale of the transform in the local space
		* @param p_newScale
		*/
		void SetLocalScale(struct Maths::FVector3 p_newScale);


		/**
		* Set the position of the transform in world space
		* @param p_newPosition
		*/
		void SetWorldPosition(struct Maths::FVector3 p_newPosition);

		/**
		* Set the rotation of the transform in world space
		* @param p_newRotation
		*/
		void SetWorldRotation(Maths::FQuaternion p_newRotation);

		/**
		* Set the scale of the transform in world space
		* @param p_newScale
		*/
		void SetWorldScale(struct Maths::FVector3 p_newScale);

		/**
		* Translate in the local space
		* @param p_translation
		*/
		void TranslateLocal(const struct Maths::FVector3& p_translation);

		/**
		* Rotate in the local space
		* @param p_rotation
		*/
		void RotateLocal(const Maths::FQuaternion& p_rotation);

		/**
		* Scale in the local space
		* @param p_scale
		*/
		void ScaleLocal(const struct Maths::FVector3& p_scale);

		/**
		* Return the position in local space
		*/
		const Maths::FVector3& GetLocalPosition() const;

		/**
		* Return the rotation in local space
		*/
		const Maths::FQuaternion& GetLocalRotation() const;

		/**
		* Return the scale in local space
		*/
		const Maths::FVector3& GetLocalScale() const;

		/**
		* Return the position in world space
		*/
		const Maths::FVector3& GetWorldPosition() const;

		/**
		* Return the rotation in world space
		*/
		const Maths::FQuaternion& GetWorldRotation() const;

		/**
		* Return the scale in world space
		*/
		const Maths::FVector3& GetWorldScale() const;

		/**
		* Return the local matrix
		*/
		const Maths::FMatrix4& GetLocalMatrix() const;

		/**
		* Return the world matrix
		*/
		const Maths::FMatrix4& GetWorldMatrix() const;

		/**
		* Return the FTransform attached to the CTransform
		*/
		Maths::FTransform& GetFTransform();

		/**
		* Return the transform world forward
		*/
		Maths::FVector3 GetWorldForward() const;

		/**
		* Return the transform world up
		*/
		Maths::FVector3 GetWorldUp() const;

		/**
		* Return the transform world right
		*/
		Maths::FVector3 GetWorldRight() const;

		/**
		* Return the transform local forward
		*/
		Maths::FVector3 GetLocalForward() const;

		/**
		* Return the transform local up
		*/
		Maths::FVector3 GetLocalUp() const;

		/**
		* Return the transform local right
		*/
		Maths::FVector3 GetLocalRight() const;

		/**
		* Serialize the component
		* @param p_doc
		* @param p_node
		*/
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;

		/**
		* Deserialize the component
		* @param p_doc
		* @param p_node
		*/
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node) override;

		/**
		* Defines how the component should be drawn in the inspector
		* @param p_root
		*/
		//virtual void OnInspector(UI::Internal::WidgetContainer& p_root) override;

	private:
		Maths::FTransform m_transform;
	};
}