#pragma once
#include <queue>
#include <Rendering/Entities/Camera.h>

#include "AView.h"

namespace Editor::Core
{
	/**
	* A simple camera controller used to navigate into views
	*/
	class CameraController
	{
	public:
		/**
		* Constructor
		* @param p_view
		* @param p_camera
		*/
		CameraController(
			Editor::Panels::AView& p_view,
			::Rendering::Entities::Camera& p_camera
		);

		/**
		* Handle mouse and keyboard inputs
		* @parma p_deltaTime
		*/
		void HandleInputs(float p_deltaTime);
		void HandleFirstMouse();

		/**
		* Asks the camera to move to the target actor
		* @param p_target
		*/
		void MoveToTarget(::Core::ECS::Actor& p_target);
		void MoveToPose(const Maths::FVector3& pos, const Maths::FQuaternion& quat);

		/**
		* Defines the speed of the camera
		* @param p_speed
		*/
		void SetSpeed(float p_speed);

		/**
		* Returns the camera speed
		*/
		float GetSpeed() const;

		/**
		* Defines the position of the camera
		* @param p_position
		*/
		void SetPosition(const Maths::FVector3& p_position);

		/**
		* Defines the rotation of the camera
		* @param p_rotation
		*/
		void SetRotation(const Maths::FQuaternion& p_rotation);

		/**
		* Returns the position of the camera
		*/
		const Maths::FVector3& GetPosition() const;

		/**
		* Returns the position of the camera
		*/
		const Maths::FQuaternion& GetRotation() const;

		/**
		* Returns true if the right mouse click is being pressed
		*/
		bool IsRightMousePressed() const;

		/**
		* Returns true if the camera controller is currently operating
		*/
		bool IsOperating() const;

		/**
		* Lock the target actor to the given actor.
		* @note Usefull to force orbital camera or camera focus to target a specific actor
		* @param p_actor
		*/
		void LockTargetActor(::Core::ECS::Actor& p_actor);

		/**
		* Removes any locked actor
		*/
		void UnlockTargetActor();

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