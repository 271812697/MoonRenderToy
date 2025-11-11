#pragma once
#include <OvCore/Rendering/SceneRenderer.h>
#include <OvRendering/HAL/UniformBuffer.h>
#include <OvRendering/Entities/Camera.h>
#include <OvRendering/Core/CompositeRenderer.h>
#include <OvRendering/HAL/Framebuffer.h>
#include "renderer/InputState.h"
namespace OvEditor {
	namespace Panels
	{

		/**
		* Base class for any view
		*/
		class AView
		{
		public:

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

			OvRendering::HAL::Framebuffer m_msaaframebuffer;
			OvRendering::HAL::Framebuffer m_framebuffer;
			std::unique_ptr<OvCore::Rendering::SceneRenderer> m_renderer;
			int mWidth = 1;
			int mHeight = 1;
			std::string name = "View";
			InputState input;
		};
	}
}