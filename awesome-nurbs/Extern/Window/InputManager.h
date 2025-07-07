
#pragma once

#include <unordered_map>

#include "Window.h"
#include "EKey.h"
#include "EKeyState.h"
#include "EMouseButton.h"
#include "EMouseButtonState.h"

namespace Windowing::Inputs
{
	/**
	* Handles inputs (Mouse and keyboard)
	*/
	class InputManager
	{
	public:
		/**
		* Create the window
		* @param p_windowSettings
		*/
		InputManager(Window& p_window);

		/**
		* Destroy the input manager by removing listeners on the window
		*/
		~InputManager();
		
		void SetDeltaTime(float dt) {
			deltaTime = dt;
		}
		float GetDeltaTime() {
			return deltaTime;
		}

		/**
		* Return the current state of the given key
		* @param p_key
		*/
		EKeyState GetKeyState(EKey p_key) const;

		/**
		* Return the current state of the given mouse button
		* @param p_button
		*/
		EMouseButtonState GetMouseButtonState(EMouseButton p_button) const;

		/**
		* Return true if the given key has been pressed during the frame
		* @param p_key
		*/
		bool IsKeyPressed(EKey p_key) const;

		/**
		* Return true if the given key has been released during the frame
		* @param p_key
		*/
		bool IsKeyReleased(EKey p_key) const;

		/**
		* Return true if the given mouse button has been pressed during the frame
		* @param p_button
		*/
		bool IsMouseButtonPressed(EMouseButton p_button) const;

		/**
		* Return true if the given mouse button has been released during the frame
		* @param p_button
		*/
		bool IsMouseButtonReleased(EMouseButton p_button) const;

		/**
		* Return the current mouse position relative to the window
		*/
		std::pair<double, double> GetMousePosition() const;

		std::pair<int, int>GetCursoreOffset();

		/**
		* Clear any event occured
		* @note Should be called at the end of every game tick
		*/
		void ClearEvents();
        float GetMouseScrollOffset()const;
		static InputManager* GetInputManager();
	private:
		void OnCursorMove(int p_x,int p_y);
		void OnKeyPressed(int p_key);
		void OnKeyReleased(int p_key);
		void OnMouseButtonPressed(int p_button);
		void OnMouseButtonReleased(int p_button);
        void OnMouseScroll(double delta);

	private:
		Window& m_window;
	    float deltaTime = 0;

		Tools::Eventing::ListenerID m_keyPressedListener;
		Tools::Eventing::ListenerID m_keyReleasedListener;
		Tools::Eventing::ListenerID m_mouseButtonPressedListener;
		Tools::Eventing::ListenerID m_mouseButtonReleasedListener;
        float wheel = 0.0f;
		int horizon_X = 0;
		int vertical_Y = 0;
		std::unordered_map<EKey, EKeyState>					m_keyEvents;
		std::unordered_map<EMouseButton, EMouseButtonState>	m_mouseButtonEvents;
	};
}