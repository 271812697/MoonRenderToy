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
		class AView
		{
		public:

			AView(
				const std::string& p_title
			);

			virtual void Update(float p_deltaTime);
			virtual void InitFrame();
			void Render();
			void Present();
			virtual void DrawFrame();
			virtual ::Rendering::Entities::Camera* GetCamera() = 0;
			virtual ::Core::SceneSystem::Scene* GetScene() = 0;
			std::pair<uint16_t, uint16_t> GetSafeSize() const;
		    ::Core::Rendering::SceneRenderer& GetRenderer() ;
			::Core::ECS::Actor& GetSelectedActor();
			void SelectActor(::Core::ECS::Actor& actor);
			void Resize(int width, int height);
			void UnselectActor();
			bool IsSelectActor();
			InputState& getInutState();
			void ClearEvents();
			Maths::FVector3 GetRoaterCenter();
			void SetRotaterCenter(const Maths::FVector3&center);
		protected:
			virtual ::Core::Rendering::SceneRenderer::SceneDescriptor CreateSceneDescriptor();

		protected:
			::Core::ECS::Actor* mTargetActor = nullptr;;
			Maths::FVector3 m_roaterCenter = {0,0,0};
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