#include <ranges>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Core/Rendering/SkyBoxRenderPass .h>
#include <Core/Rendering/ReflectionRenderPass.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Rendering/HAL/Renderbuffer.h>
#include <Rendering/HAL/Profiling.h>

namespace
{
	unsigned int resolution = 1024;
	constexpr uint32_t kFaceCount = 6;
	const Maths::FVector3 kFaceRotations[kFaceCount] = {
		{ 0.0f, -90.0f, 180.0f },	// (Right)
		{ 0.0f, 90.0f, 180.0f },	// (Left)
		{ 90.0f, 0.0f, 180.0f },	// (Top)
		{ -90.0f, 0.0f, 180.0f },	// (Bottom)
		{ 0.0f, 0.0f, 180.0f },		// (Front)
		{ 0.0f, -180.0f, 180.0f }	// (Back)
	};
}

Core::Rendering::SkyboxRenderPass::SkyboxRenderPass(::Rendering::Core::CompositeRenderer& p_renderer) :
	::Rendering::Core::ARenderPass(p_renderer)
{
}

void Core::Rendering::SkyboxRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	ZoneScoped;
	TracyGpuZone("SkyboxRenderPass");
	using namespace Core::Rendering;
	if (!irradianceCube.get()) {
		irradianceCube= std::make_shared<::Rendering::HAL::Texture>(
			::Rendering::Settings::ETextureType::TEXTURE_CUBE,
			"irradianceCube"
		);
		irradianceCube->Allocate(
			::Rendering::Settings::TextureDesc{
				.width = resolution,
				.height = resolution,
				.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR_MIPMAP_LINEAR,
				.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
				.horizontalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_EDGE,
				.verticalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_EDGE,
				.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F,
				.useMipMaps = true
			}
		);
		prefilterCube = std::make_shared<::Rendering::HAL::Texture>(
			::Rendering::Settings::ETextureType::TEXTURE_CUBE,
			"prefilterCube"
		);
		prefilterCube->Allocate(
			::Rendering::Settings::TextureDesc{
				.width = resolution,
				.height = resolution,
				.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR_MIPMAP_LINEAR,
				.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
				.horizontalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_EDGE,
				.verticalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_EDGE,
				.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F,
				.useMipMaps = true
			}
		);
		{
			for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex)
			{
				irradianceBuffer.Attach<::Rendering::HAL::Texture>(
					irradianceCube,
					::Rendering::Settings::EFramebufferAttachment::COLOR,
					faceIndex, // Each color attachment is a face of the cubemap
					faceIndex // Each face of the cubemap is accessed by its layer index
				);
			}

			// Depth buffer
			const auto renderbuffer = std::make_shared<::Rendering::HAL::Renderbuffer>(false);
			const auto internalFormat = ::Rendering::Settings::EInternalFormat::DEPTH_COMPONENT;
			renderbuffer->Allocate(resolution, resolution, internalFormat);
			irradianceBuffer.Attach(renderbuffer, ::Rendering::Settings::EFramebufferAttachment::DEPTH);
			// Validation
			irradianceBuffer.Validate();
		}
		{
			for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex)
			{
				prefilterBuffer.Attach<::Rendering::HAL::Texture>(
					prefilterCube,
					::Rendering::Settings::EFramebufferAttachment::COLOR,
					faceIndex, // Each color attachment is a face of the cubemap
					faceIndex // Each face of the cubemap is accessed by its layer index
				);
			}

			// Depth buffer
			const auto renderbuffer = std::make_shared<::Rendering::HAL::Renderbuffer>(false);
			const auto internalFormat = ::Rendering::Settings::EInternalFormat::DEPTH_COMPONENT;
			renderbuffer->Allocate(resolution, resolution, internalFormat);
			prefilterBuffer.Attach(renderbuffer, ::Rendering::Settings::EFramebufferAttachment::DEPTH);
			// Validation
			prefilterBuffer.Validate();
		}
	}


}

