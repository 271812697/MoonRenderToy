/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Maths/FTransform.h>
#include <Maths/FVector2.h>

#include <Core/ECS/Actor.h>

namespace Editor::Core
{
	enum class EGizmoOperation
	{
		TRANSLATE,
		ROTATE,
		SCALE
	};

	/* Handle gizmo behaviours */
	class GizmoBehaviour
	{
	public:
		enum class EDirection
		{
			X,
			Y,
			Z
		};

		/**
		* Returns true if the snapping behaviour is enabled
		*/
		bool IsSnappedBehaviourEnabled() const;

		/**
		* Starts the gizmo picking behaviour for the given target in the given direction
		* @param p_actor
		* @param p_cameraPosition
		* @param p_operation
		* @param p_direction
		*/
		void StartPicking(::Core::ECS::Actor& p_target, const Maths::FVector3& p_cameraPosition, EGizmoOperation p_operation, EDirection p_direction);

		/**
		* Stops the gizmo picking behaviour
		*/
		void StopPicking();

		/**
		* Handle the current behaviour
		* @param p_viewMatrix
		* @param p_projectionMatrix
		* @param p_cameraPosition
		* @param p_viewSize
		*/
		void ApplyOperation(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector3& p_cameraPosition, const Maths::FVector2& p_viewSize);

		/**
		* Set the given mouse position as the current mouse position and update the previous mouse position
		* @param p_mousePosition
		*/
		void SetCurrentMouse(const Maths::FVector2& p_mousePosition);

		/**
		* Returns true if the gizmo is currently picked
		*/
		bool IsPicking() const;

		/**
		* Returns the current operation direction
		*/
		EDirection GetDirection() const;

		/**
		* Returns the mouse origin and direction in World Space
				* @param p_mousePos
		* @param p_viewMatrix
		* @param p_projectionMatrix
		* @param p_viewSize
		*/
		Maths::FVector3 GetMouseRay(const Maths::FVector2& p_mousePos, const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize);

	private:
		/**
		* Returns the global direction matching with the current m_direction
		*/
		Maths::FVector3 GetFakeDirection() const;

		/**
		* Returns the actual direction of the target matching with the current m_direction
		* @param p_relative (If true, the direction depends on hierarchy)
		*/
		Maths::FVector3 GetRealDirection(bool p_relative = false) const;

		/**
		* Returns the 3D vector of the arrow projected to the screen
		* @param p_viewMatrix
		* @param p_projectionMatrix
		* @param p_viewSize
		*/
		Maths::FVector2 GetScreenDirection(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize) const;

		Maths::FVector3 IntersectRayWithAxis(const Maths::FVector3& rayOrigin, const Maths::FVector3& rayDirection, const Maths::FVector3& axisOrigin, const Maths::FVector3& axisDirection) const;
		Maths::FVector3 RaycastToAxis(const Maths::FMatrix4& viewMatrix,
			const Maths::FMatrix4& projectionMatrix, const Maths::FVector2& viewSize,
			const Maths::FVector3& axisDirection, const Maths::FVector2& mousePosition) const;
		/**
		* Handle the translation behaviour
		* @param p_viewMatrix
		* @param p_projectionMatrix
		* @param p_cameraPosition
		* @param p_viewSize
		*/
		void ApplyTranslation(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector3& p_cameraPosition, const Maths::FVector2& p_viewSize);

		/**
		* Handle the rotation behaviour
		* @param p_viewMatrix
		* @param p_projectionMatrix
		* @param p_viewSize
		*/
		void ApplyRotation(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize) const;

		/**
		* Handle the scale behaviour
		* @param p_viewMatrix
		* @param p_projectionMatrix
		* @param p_viewSize
		*/
		void ApplyScale(const Maths::FMatrix4& p_viewMatrix, const Maths::FMatrix4& p_projectionMatrix, const Maths::FVector2& p_viewSize) const;

	private:
		bool m_firstMouse = true;
		bool m_firstPick = true;
		float m_distanceToActor = 0.0f;
		::Core::ECS::Actor* m_target = nullptr;
		EGizmoOperation m_currentOperation;
		EDirection m_direction;
		Maths::FTransform m_originalTransform;
		Maths::FVector3 m_initialOffset;
		Maths::FVector2 m_originMouse;
		Maths::FVector2 m_currentMouse;
		Maths::FVector2 m_screenDirection;
	};
}
