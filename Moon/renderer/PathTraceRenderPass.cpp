#include <tracy/Tracy.hpp>
#include <Core/ECS/Components/CPostProcessStack.h>
#include "renderer/PathTraceRenderPass.h"
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/FramebufferUtil.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include "Rendering/Resources/Texture.h"

#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/HAL/Profiling.h>
namespace Editor::Rendering {
	PathTraceRenderPass::PathTraceRenderPass(::Rendering::Core::CompositeRenderer& p_renderer): ::Rendering::Core::ARenderPass(p_renderer)
	{
	}
	void PathTraceRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
	{
	}
	void PathTraceRenderPass::InitGPUDataBuffers() {
		glPixelStorei(GL_PACK_ALIGNMENT, 1);

		::Rendering::Settings::TextureDesc desc;
		desc.isTextureBuffer = true;
		desc.internalFormat = ::Rendering::Settings::EInternalFormat::RGB32F;
		//desc.buffetLen = edgeValue.size() * sizeof(uint8_t);
		//desc.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
		//		.data = edgeValue.data()
		//};
		BVHTex = new ::Rendering::Resources::Texture();;
		auto gltexture = new ::Rendering::HAL::GLTexture(::Rendering::Settings::ETextureType::TEXTURE_BUFFER);
		gltexture->Allocate(desc);
		BVHTex->SetTexture(std::unique_ptr<::Rendering::HAL::Texture>(gltexture));

	}
	void PathTraceRenderPass::InitShaders() {

	}
	void PathTraceRenderPass::InitFBOs() {

	}
}

