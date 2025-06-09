/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once

#include "GizmoBehaviour.h"
#include "AViewControllable.h"
#include "PickingRenderPass.h"


namespace OvEditor::Panels
{
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
		KEYR,
		KEYF,
		ALTA,
		RIGHT,
		LEFT,
		UP,
		DOWN,
		PageUp,
		PageDown
	};
	enum MouseButtonState {
		MOUSE_UP = 0,
		MOUSE_DOWN
	};
	enum MouseButton {
		MOUSE_BUTTON_LEFT = 0,
		MOUSE_BUTTON_RIGHT,
		MOUSE_BUTTON_MIDDLE
	};
	class InputState {
	public:
		InputState();
		KeyState GetKeyState(KeyBoard p_key) ;

		/**
		* Return the current state of the given mouse button
		* @param p_button
		*/
		MouseButtonState GetMouseButtonState(MouseButton p_button) ;

		/**
		* Return true if the given key has been pressed during the frame
		* @param p_key
		*/
		bool IsKeyPressed(KeyBoard p_key) ;

		/**
		* Return true if the given key has been released during the frame
		* @param p_key
		*/
		bool IsKeyReleased(KeyBoard p_key) ;

		/**
		* Return true if the given mouse button has been pressed during the frame
		* @param p_button
		*/
		bool IsMouseButtonPressed(MouseButton p_button) ;

		/**
		* Return true if the given mouse button has been released during the frame
		* @param p_button
		*/
		bool IsMouseButtonReleased(MouseButton p_button) ;

		/**
		* Return the current mouse position relative to the window
		*/
		std::pair<double, double> GetMousePosition();

		/**
		* Returns the scroll data for the current frame
		*/
		std::pair<double, double> GetMouseScroll() ;

		/**
		* Clear any event occured
		* @note Should be called at the end of every game tick
		*/
		void ClearEvents();
		void ReceiveEvent(QEvent* e);
	private:

		std::unordered_map<KeyBoard, KeyState> m_keyEvents;
		std::unordered_map<MouseButton, MouseButtonState> m_mouseButtonEvents;
		std::pair<double, double> m_scrollData;
		int mouseX=0, mouseY = 0;
	};

	class SceneView : public OvEditor::Panels::AViewControllable
	{
	public:
		/**
		* Constructor
		* @param p_title
		* @param p_opened
		* @param p_windowSettings
		*/
		SceneView(
			const std::string& p_title
		);

		/**
		* Update the scene view
		*/
		virtual void Update(float p_deltaTime) override;

		/**
		* Prepare the renderer for rendering
		*/
		virtual void InitFrame() override;

		/**
		* Returns the scene used by this view
		*/
		virtual OvCore::SceneSystem::Scene* GetScene();

		/**
		* Set the gizmo operation
		* @param p_operation
		*/
		void SetGizmoOperation(Core::EGizmoOperation p_operation);

		/**
		* Returns the current gizmo operation
		*/
		Core::EGizmoOperation GetGizmoOperation() const;
		void ReceiveEvent(QEvent* e);

		InputState& getInutState();
		void ClearEvents();
	protected:
		virtual OvCore::Rendering::SceneRenderer::SceneDescriptor CreateSceneDescriptor() override;

	private:
		virtual void DrawFrame() override;
		void HandleActorPicking();



	private:
		OvCore::SceneSystem::SceneManager& m_sceneManager;
		OvEditor::Core::GizmoBehaviour m_gizmoOperations;
		OvEditor::Core::EGizmoOperation m_currentOperation = OvEditor::Core::EGizmoOperation::TRANSLATE;
		OvCore::Resources::Material m_fallbackMaterial;

		OvTools::Utils::OptRef<OvCore::ECS::Actor> m_highlightedActor;
		std::optional<OvEditor::Core::GizmoBehaviour::EDirection> m_highlightedGizmoDirection;
		InputState input;
	};
}