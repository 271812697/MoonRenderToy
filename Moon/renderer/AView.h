

#pragma once

#include <OvCore/Rendering/SceneRenderer.h>
#include <OvRendering/HAL/UniformBuffer.h>
#include <OvRendering/Entities/Camera.h>
#include <OvRendering/Core/CompositeRenderer.h>
#include <OvRendering/HAL/Framebuffer.h>

class QEvent;
namespace OvEditor {
	namespace Panels
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
		/**
		* Base class for any view
		*/
		class AView
		{
		public:
			/**
			* Constructor
			* @param p_title
			* @param p_opened
			* @param p_windowSettings
			*/
			AView(
				const std::string& p_title

			);

			/**
			* Update the view
			* @param p_deltaTime
			*/
			virtual void Update(float p_deltaTime);



			/**
			* Prepare the renderer for rendering
			*/
			virtual void InitFrame();

			/**
			* Render the view
			*/
			void Render();
			void Present();

			/**
			* Draw the frame (m_renderer->Draw() if not overriden)
			* @note You don't need to begin/end frame inside of this method, as this is called after begin, and after end
			*/
			virtual void DrawFrame();

			/**
			* Returns the camera used by this view
			*/
			virtual OvRendering::Entities::Camera* GetCamera() = 0;

			/**
			* Returns the scene used by this view
			*/
			virtual OvCore::SceneSystem::Scene* GetScene() = 0;

			/**
			* Returns the size of the panel ignoring its titlebar height
			*/
			std::pair<uint16_t, uint16_t> GetSafeSize() const;

			/**
			* Returns the renderer used by this view
			*/
			const OvCore::Rendering::SceneRenderer& GetRenderer() const;
			OvCore::ECS::Actor& GetSelectedActor();
			void SelectActor(OvCore::ECS::Actor& actor);
			void Resize(int width, int height);
			void UnselectActor();
			bool IsSelectActor();
			InputState& getInutState();
			void ClearEvents();
		protected:
			virtual OvCore::Rendering::SceneRenderer::SceneDescriptor CreateSceneDescriptor();

		protected:
			OvCore::ECS::Actor* mTargetActor = nullptr;;

			OvMaths::FVector3 m_gridColor = OvMaths::FVector3{ 0.176f, 0.176f, 0.176f };

			OvRendering::HAL::Framebuffer m_framebuffer;
			std::unique_ptr<OvCore::Rendering::SceneRenderer> m_renderer;
			int mWidth = 1;
			int mHeight = 1;
			std::string name = "View";
			InputState input;
		};
	}
}