#pragma once
#include <Rendering/Core/ARenderPass.h>
#include <Rendering/HAL/Framebuffer.h>
#include <Core/Rendering/PostProcess/AEffect.h>
namespace Core::Rendering
{
	class SkyboxRenderPass : public ::Rendering::Core::ARenderPass
	{
	public:
		SkyboxRenderPass(::Rendering::Core::CompositeRenderer& p_renderer);

	protected:
		virtual void Draw(::Rendering::Data::PipelineState p_pso) override;
		
	private:
		::Core::Resources::Material m_skyboxMaterial;
		::Core::Resources::Material m_skyboxIrrandianceMaterial;
		::Rendering::HAL::Framebuffer skyBoxBuffer;
		::Rendering::HAL::Framebuffer irradianceBuffer;
		::Rendering::HAL::Framebuffer prefilterBuffer;
		::Rendering::Resources::Texture*skyTexture=nullptr;
		std::shared_ptr<::Rendering::HAL::Texture> skyBoxCube;
		std::shared_ptr<::Rendering::HAL::Texture> irradianceCube;
		std::shared_ptr<::Rendering::HAL::Texture> prefilterCube;
		
	};
}
