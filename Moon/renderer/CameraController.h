#pragma once
#include <queue>
#include <Rendering/Entities/Camera.h>

#include "AView.h"

namespace Editor::Core
{
	class CameraController
	{
	public:
		CameraController(
			Editor::Panels::AView& p_view,
			::Rendering::Entities::Camera& p_camera
		);

		void HandleInputs(float p_deltaTime);
		void HandleFirstMouse();
		void MoveToTarget(::Core::ECS::Actor& p_target);
		void MoveToPose(const Maths::FVector3& pos, const Maths::FQuaternion& quat);
		void SetSpeed(float p_speed);
		float GetSpeed() const;
		void SetPosition(const Maths::FVector3& p_position);
		void SetRotation(const Maths::FQuaternion& p_rotation);
		const Maths::FVector3& GetPosition() const;
		const Maths::FQuaternion& GetRotation() const;
		bool IsRightMousePressed() const;
		bool IsOperating() const;
		void LockTargetActor(::Core::ECS::Actor& p_actor);
		void UnlockTargetActor();
		void EnableRotate(bool flag);

	private:
		std::optional<std::reference_wrapper<::Core::ECS::Actor>> GetTargetActor() const;
		void HandleCameraPanning(const Maths::FVector2& p_mouseOffset, bool p_firstMouse);
		void HandleCameraOrbit(const Maths::FVector3& center, const Maths::FVector2& p_mouseOffset, bool p_firstMouse);
		void HandleCameraFPSMouse(const Maths::FVector2& p_mouseOffset, bool p_firstMouse);

		void HandleCameraZoom();
		void HandleCameraFPSKeyboard(float p_deltaTime);
		void HandleMousePressed();
		void HandleMouseReleased();

	private:
		Editor::Panels::AView& m_view;
		::Rendering::Entities::Camera& m_camera;
		std::queue<std::tuple<Maths::FVector3, Maths::FQuaternion>> m_cameraDestinations;
		bool m_leftMousePressed = false;
		bool m_middleMousePressed = false;
		bool m_rightMousePressed = false;
		bool m_enableRotate = true;

		Maths::FVector3 m_targetSpeed;
		Maths::FVector3 m_currentMovementSpeed;

		Maths::FTransform* m_orbitTarget = nullptr;
		Maths::FVector3 m_orbitStartOffset;
		bool m_firstMouse = true;
		double m_lastMousePosX = 0.0;
		double m_lastMousePosY = 0.0;
		Maths::FVector3 m_ypr;
		float m_mouseSensitivity = 0.12f;
		float m_cameraDragSpeed = 0.03f;
		float m_cameraOrbitSpeed = 0.5f;
		float m_cameraMoveSpeed = 15.0f;
		float m_focusDistance = 15.0f;
		float m_focusLerpCoefficient = 8.0f;

		std::optional<std::reference_wrapper<::Core::ECS::Actor>> m_lockedActor = std::nullopt;
	};
}