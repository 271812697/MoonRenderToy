/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Core/Rendering/SceneRenderer.h>
#include <Rendering/HAL/UniformBuffer.h>
#include <Rendering/Entities/Camera.h>
#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/HAL/Framebuffer.h>


namespace Editor::Panels
{
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
		~AView();
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

		/**
		* Draw the frame (m_renderer->Draw() if not overriden)
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

	protected:
		virtual ::Core::Rendering::SceneRenderer::SceneDescriptor CreateSceneDescriptor();

	protected:


		Maths::FVector3 m_gridColor = Maths::FVector3{ 0.176f, 0.176f, 0.176f };

		::Rendering::HAL::Framebuffer m_framebuffer;
		std::unique_ptr<::Core::Rendering::SceneRenderer> m_renderer;
	};
}