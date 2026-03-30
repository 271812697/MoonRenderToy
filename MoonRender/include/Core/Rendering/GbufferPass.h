#pragma once
#include <Rendering/Core/ARenderPass.h>
#include <Rendering/HAL/Framebuffer.h>
#include <Core/Resources/Material.h>
namespace Core::Rendering
{
	struct GbufferTextureData {
		std::shared_ptr<::Rendering::HAL::Texture> position;
		std::shared_ptr<::Rendering::HAL::Texture> normal;
		std::shared_ptr<::Rendering::HAL::Texture> albedo;
	};
	class GbufferPass : public ::Rendering::Core::ARenderPass
	{
	public:
		GbufferPass(::Rendering::Core::CompositeRenderer& p_renderer);


	protected:
		virtual void Draw(::Rendering::Data::PipelineState p_pso) override;
		virtual void ResizeRenderer(int width, int height) override;
	private:
		::Core::Resources::Material gbufferMaterial;
		GbufferTextureData gbufferData;
		::Rendering::HAL::Framebuffer gbuffer;
		
	};
}
