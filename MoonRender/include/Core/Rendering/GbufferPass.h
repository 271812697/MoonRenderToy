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
		std::shared_ptr<::Rendering::HAL::Texture> occlusion;
	};
	class GbufferPass : public ::Rendering::Core::ARenderPass
	{
	public:
		GbufferPass(::Rendering::Core::CompositeRenderer& p_renderer);
	protected:
		virtual void Draw(::Rendering::Data::PipelineState p_pso) override;
		virtual void ResizeRenderer(int width, int height) override;
	private:

	private:
		std::shared_ptr<::Rendering::HAL::Texture> noiseTexture;
		::Core::Resources::Material gbufferMaterial;
		::Core::Resources::Material ssaoMaterial;
		GbufferTextureData gbufferData;
		::Rendering::HAL::Framebuffer gbuffer;
		::Rendering::HAL::Framebuffer ssaobuffer;
	};
}
