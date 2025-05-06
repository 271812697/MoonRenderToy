/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <queue>


#include <Rendering/Entities/Camera.h>


#include "AView.h"

class QEvent;
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
			::Editor::Panels::AView& p_view,
			::Rendering::Entities::Camera& p_camera
		);

		/**
		* Handle mouse and keyboard inputs
		* @parma p_deltaTime
		*/
		void HandleInputs(float p_deltaTime);

		/**
		* Asks the camera to move to the target actor
		* @param p_target
		*/
		void MoveToTarget(::Core::ECS::Actor& p_target);

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
		* Lock the target actor to the given actor.
		* @note Usefull to force orbital camera or camera focus to target a specific actor
		* @param p_actor
		*/
		void LockTargetActor(::Core::ECS::Actor& p_actor);

		/**
		* Removes any locked actor
		*/
		void UnlockTargetActor();
		void ReceiveEvent(QEvent* e);

	private:
		std::optional<std::reference_wrapper<::Core::ECS::Actor>> GetTargetActor() const;
		void HandleCameraPanning(const Maths::FVector2& p_mouseOffset, bool p_firstMouse);
		void HandleCameraOrbit(::Core::ECS::Actor& p_target, const Maths::FVector2& p_mouseOffset, bool p_firstMouse);
		void HandleCameraFPSMouse(const Maths::FVector2& p_mouseOffset, bool p_firstMouse);

		void HandleCameraZoom();
		void HandleCameraFPSKeyboard(float p_deltaTime, QEvent* e);
		void HandleMousePressed();
		void HandleMouseReleased();

	private:
		enum KeyState
		{
			Down = 0,
			Up
		};
		enum KeyBoard :uint8_t
		{
			KEYW = 0,
			KEYA,
			KEYS,
			KEYD,
			KEYQ,
			KEYE,
			KEYF,
			ALTA
		};
		std::unordered_map<KeyBoard, KeyState>mKeyState;


		::Editor::Panels::AView& m_view;
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