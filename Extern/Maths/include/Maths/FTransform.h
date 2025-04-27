
#pragma once

#include "Maths/Internal/TransformNotifier.h"
#include "Maths/FQuaternion.h"
#include "Maths/FMatrix4.h"
#include "Maths/FVector3.h"

namespace Maths
{

	class FTransform
	{
	public:

		FTransform(FVector3 p_localPosition = FVector3(0.0f, 0.0f, 0.0f), FQuaternion p_localRotation = FQuaternion::Identity, FVector3 p_localScale = FVector3(1.0f, 1.0f, 1.0f));


		~FTransform();

		void NotificationHandler(Internal::TransformNotifier::ENotification p_notification);

		void SetParent(FTransform& p_parent);

		bool RemoveParent();

		bool HasParent() const;

		void GenerateMatrices(FVector3 p_position, FQuaternion p_rotation, FVector3 p_scale);

		void UpdateWorldMatrix();

		void SetLocalPosition(FVector3 p_newPosition);


		void SetLocalRotation(FQuaternion p_newRotation);

		void SetLocalScale(FVector3 p_newScale);

		void SetWorldPosition(FVector3 p_newPosition);

		void SetWorldRotation(FQuaternion p_newRotation);

		void SetWorldScale(FVector3 p_newScale);

		void TranslateLocal(const FVector3& p_translation);

		void RotateLocal(const FQuaternion& p_rotation);

		void ScaleLocal(const FVector3& p_scale);

		const FVector3& GetLocalPosition() const;

		const FQuaternion& GetLocalRotation() const;

		const FVector3& GetLocalScale() const;

		const FVector3& GetWorldPosition() const;

		const FQuaternion& GetWorldRotation() const;

		const FVector3& GetWorldScale() const;

		const FMatrix4& GetLocalMatrix() const;

		const FMatrix4& GetWorldMatrix() const;

		FVector3 GetWorldForward() const;

		FVector3 GetWorldUp() const;

		FVector3 GetWorldRight() const;

		FVector3 GetLocalForward() const;

		FVector3 GetLocalUp() const;

		FVector3 GetLocalRight() const;

	public:
		Internal::TransformNotifier Notifier;
		Internal::TransformNotifier::NotificationHandlerID m_notificationHandlerID;
		FTransform*	m_parent;

	private:
		void PreDecomposeWorldMatrix();
		void PreDecomposeLocalMatrix();

		FVector3 m_localPosition;
		FQuaternion m_localRotation;
		FVector3 m_localScale;
		FVector3 m_worldPosition;
		FQuaternion m_worldRotation;
		FVector3 m_worldScale;

		FMatrix4 m_localMatrix;
		FMatrix4 m_worldMatrix;

		
	};
}