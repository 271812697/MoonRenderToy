/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <atomic>

#include "Rendering/Core/IRenderer.h"
#include "Rendering/Data/FrameInfo.h"
#include "Rendering/Resources/IMesh.h"
#include "Rendering/Resources/Texture.h"
#include "Rendering/Entities/Drawable.h"
#include "Rendering/Settings/EBlitFlags.h"
#include "Rendering/Context/Driver.h"

namespace Rendering::Core
{
	/**
	* A simple base renderer that doesn't handle any object binding, but provide a strong base for other renderers
	* to implement their own logic.
	*/
	class ABaseRenderer : public IRenderer
	{
	public:
		/**
		* Constructor of the base renderer
		* @param p_driver
		*/
		ABaseRenderer(Context::Driver& p_driver);

		/**
		* Destructor of the base renderer
		*/
		virtual ~ABaseRenderer();

		/**
		* Begin Frame
		* @param p_frameDescriptor
		*/
		virtual void BeginFrame(const Data::FrameDescriptor& p_frameDescriptor);

		/**
		* End Frame
		*/
		virtual void EndFrame();

		/**
		* Returns a reference to the current frame descriptor
		* @note Cannot be called outside of a frame drawing operation
		*/
		const Data::FrameDescriptor& GetFrameDescriptor() const;

		/**
		* Create a pipeline state object.
		* The settings are set with default settings provided by the renderer
		*/
		Data::PipelineState CreatePipelineState() const;

		/**
		* Returns true if the renderer is currently drawing a frame
		*/
		bool IsDrawing() const;

		/**
		* Set the viewport
		* @param p_x
		* @param p_y
		* @param p_width
		* @param p_height
		*/
		void SetViewport(uint32_t p_x, uint32_t p_y, uint32_t p_width, uint32_t p_height);

		/**
		* Clear the screen
		* @param p_colorBuffer
		* @param p_depthBuffer
		* @param p_stencilBuffer
		* @param p_color
		*/
		virtual void Clear(
			bool p_colorBuffer,
			bool p_depthBuffer,
			bool p_stencilBuffer,
			const Maths::FVector4& p_color = Maths::FVector4::Zero
		);

		/**
		* Draw a fullscreen quad with the given material
		* @param p_pso
		* @param p_src
		* @param p_dst
		* @param p_material
		* @param p_flags
		*/
		virtual void Blit(
			Rendering::Data::PipelineState p_pso,
			Rendering::HAL::Framebuffer& p_src,
			Rendering::HAL::Framebuffer& p_dst,
			Rendering::Data::Material& p_material,
			Rendering::Settings::EBlitFlags p_flags = Rendering::Settings::EBlitFlags::DEFAULT
		);
		void Present(Rendering::HAL::Framebuffer& p_src);

		/**
		* Draw a drawable entity
		* @param p_pso
		* @param p_drawable
		*/
		virtual void DrawEntity(
			Rendering::Data::PipelineState p_pso,
			const Entities::Drawable& p_drawable
		);

	protected:
		Data::FrameDescriptor m_frameDescriptor;
		Context::Driver& m_driver;
		::Rendering::Resources::Texture* m_emptyTexture;
		std::unique_ptr<Rendering::Resources::IMesh> m_unitQuad;

		Rendering::Data::PipelineState m_basePipelineState;
		bool m_isDrawing;
		::Rendering::Resources::Shader* m_presentShader;
		::Rendering::Data::Material m_presentMaterial;

	private:
		static std::atomic_bool s_isDrawing;
	};
}
