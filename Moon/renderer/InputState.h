#pragma once
#include "renderer/InpuKey.h"
#include <unordered_map>
class QEvent;
namespace OvEditor {
	namespace Panels
	{
		class InputState {
		public:
			InputState();
			KeyState GetKeyState(KeyBoard p_key);

			/**
			* Return the current state of the given mouse button
			* @param p_button
			*/
			MouseButtonState GetMouseButtonState(MouseButton p_button);

			/**
			* Return true if the given key has been pressed during the frame
			* @param p_key
			*/
			bool IsKeyPressed(KeyBoard p_key);

			/**
			* Return true if the given key has been released during the frame
			* @param p_key
			*/
			bool IsKeyReleased(KeyBoard p_key);

			/**
			* Return true if the given mouse button has been pressed during the frame
			* @param p_button
			*/
			bool IsMouseButtonPressed(MouseButton p_button);

			/**
			* Return true if the given mouse button has been released during the frame
			* @param p_button
			*/
			bool IsMouseButtonReleased(MouseButton p_button);

			/**
			* Return the current mouse position relative to the window
			*/
			std::pair<double, double> GetMousePosition();

			/**
			* Returns the scroll data for the current frame
			*/
			std::pair<double, double> GetMouseScroll();

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
			int mouseX = 0, mouseY = 0;
		};
	}
}