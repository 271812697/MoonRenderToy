#pragma once
#include <Core/Rendering/SceneRenderer.h>
#include <Rendering/HAL/UniformBuffer.h>
#include <Rendering/Entities/Camera.h>
#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/HAL/Framebuffer.h>
#include "renderer/InputState.h"
namespace Editor {
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
			* Draw the frame (m_renderer->Draw() if not erriden)
			* @note You don't need to begin/end frame inside of this method, as this is called after begin, and after end
			*/
			virtual void DrawFrame();

			/**
			* Returns the camera used by this view
			*/
			virtual ::Rendering::Entities::Camera* GetCamera() = 0;

			/**
			* Returns the scene used by this view
			*/
			virtual ::Core::SceneSystem::Scene* GetScene() = 0;

			/**
			* Returns the size of the panel ignoring its titlebar height
			*/
			std::pair<uint16_t, uint16_t> GetSafeSize() const;

			/**
			* Returns the renderer used by this view
			*/
			const ::Core::Rendering::SceneRenderer& GetRenderer() const;
			::Core::ECS::Actor& GetSelectedActor();
			void SelectActor(::Core::ECS::Actor& actor);
			void Resize(int width, int height);
			void UnselectActor();
			bool IsSelectActor();
			InputState& getInutState();
			void ClearEvents();
		protected:
			virtual ::Core::Rendering::SceneRenderer::SceneDescriptor CreateSceneDescriptor();

		protected:
			::Core::ECS::Actor* mTargetActor = nullptr;;

			Maths::FVector3 m_gridColor = Maths::FVector3{ 0.176f, 0.176f, 0.176f };

			::Rendering::HAL::Framebuffer m_msaaframebuffer;
			::Rendering::HAL::Framebuffer m_framebuffer;
			std::unique_ptr<::Core::Rendering::SceneRenderer> m_renderer;
			int mWidth = 1;
			int mHeight = 1;
			std::string name = "View";
			InputState input;
		};
	}
}