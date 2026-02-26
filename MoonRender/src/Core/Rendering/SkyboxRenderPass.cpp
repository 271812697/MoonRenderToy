#include <ranges>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Core/Rendering/SkyBoxRenderPass .h>
#include <Core/Rendering/ReflectionRenderPass.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Core/ResourceManagement/TextureManager.h>
#include <Core/Rendering/FramebufferUtil.h>
#include <Rendering/HAL/Renderbuffer.h>
#include <Rendering/HAL/Profiling.h>
#include <Rendering/Resources/Loaders/TextureLoader.h>

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
	m_skyboxMaterial.SetShader(::Core::Global::ServiceLocator::Get<Core::ResourceManagement::ShaderManager>()[":Shaders\\SkyboxConvert.ovfx"]);
	m_skyboxMaterial.SetBackfaceCulling(false);
	m_skyboxIrrandianceMaterial.SetShader(GetShaderService[":Shaders\\SkyboxIrrandiance.ovfx"]);
	m_skyboxIrrandianceMaterial.SetBackfaceCulling(false);
	m_skyboxPrefilterMaterial.SetShader(GetShaderService[":Shaders\\SkyboxPrefilter.ovfx"]);
	m_skyboxPrefilterMaterial.SetBackfaceCulling(false);
	m_brdfMaterial.SetShader(GetShaderService[":Shaders\\BrdfLut.ovfx"]);
}

void Core::Rendering::SkyboxRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	ZoneScoped;
	TracyGpuZone("SkyboxRenderPass");
	using namespace Core::Rendering;
	if (!irradianceCube.get()) {
		auto fBColorDesc = ::Rendering::Settings::TextureDesc{
		.width = resolution, 
		.height = resolution,
		.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
		.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
		.horizontalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_EDGE,
		.verticalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_EDGE,
		.internalFormat = ::Rendering::Settings::EInternalFormat::RG32F,
		.useMipMaps = false,
		.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.format = ::Rendering::Settings::EFormat::RG,
			.type = ::Rendering::Settings::EPixelDataType::FLOAT
		}
		};
		::Core::Rendering::FramebufferUtil::SetupFramebuffer(brdfBuffer, fBColorDesc, false, false, false);

		skyTexture=GetService(Core::ResourceManagement::TextureManager).GetResource(":Textures/PureSky.hdr");
		
		skyBoxCube = std::make_shared<::Rendering::HAL::Texture>(
			::Rendering::Settings::ETextureType::TEXTURE_CUBE,
			"skyBoxCube"
		);
		skyBoxCube->Allocate(
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
		
		irradianceCube = std::make_shared<::Rendering::HAL::Texture>(
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
		prefilterCube->GenerateMipmaps();
		{
			for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex)
			{
				skyBoxBuffer.Attach<::Rendering::HAL::Texture>(
					skyBoxCube,
					::Rendering::Settings::EFramebufferAttachment::COLOR,
					faceIndex, // Each color attachment is a face of the cubemap
					faceIndex // Each face of the cubemap is accessed by its layer index
				);
			}
			// Depth buffer
			const auto renderbuffer = std::make_shared<::Rendering::HAL::Renderbuffer>(false);
			const auto internalFormat = ::Rendering::Settings::EInternalFormat::DEPTH_COMPONENT;
			renderbuffer->Allocate(resolution, resolution, internalFormat);
			skyBoxBuffer.Attach(renderbuffer, ::Rendering::Settings::EFramebufferAttachment::DEPTH);
			// Validation
			skyBoxBuffer.Validate();
		}
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

		//convert 
		{
			auto& engineBufferRenderFeature = m_renderer.GetFeature<Core::Rendering::EngineBufferRenderFeature>();
			auto& frameDescriptor = m_renderer.GetFrameDescriptor();
			m_skyboxMaterial.AddFeature("SKY_Convert");

			::Rendering::Entities::Camera skyCamera;
			skyCamera.SetPosition({ 0.0f, 0.0f, 0.0f });
			skyCamera.SetFov(90.0f);
			const auto [width, height] = skyBoxBuffer.GetSize();
			skyBoxBuffer.Bind();
			m_renderer.SetViewport(0, 0, width, height);
			for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex)
			{
				skyCamera.SetRotation(Maths::FQuaternion{ kFaceRotations[faceIndex] });
				skyCamera.CacheMatrices(width, height);
				engineBufferRenderFeature.SetCamera(skyCamera);
				skyBoxBuffer.SetTargetDrawBuffer(faceIndex);
				m_renderer.Clear(true, true, true);
				// Draw skybox
				::Rendering::Entities::Drawable skyboxDrawable;
				skyboxDrawable.mesh = m_renderer.m_unitCube;
				skyboxDrawable.material = m_skyboxMaterial;
				auto stateMask = m_skyboxMaterial.GenerateStateMask();
				skyboxDrawable.stateMask = stateMask;
				skyboxDrawable.material.value().SetProperty("u_SkyBoxTexture", skyTexture);
				m_renderer.DrawEntity(p_pso, skyboxDrawable);
			}
			skyBoxBuffer.Unbind();
			m_renderer.SetViewport(0, 0, frameDescriptor.renderWidth, frameDescriptor.renderHeight);
			//it is important to do this at this point because we need the mipmaps for the prefilter
			skyBoxCube->GenerateMipmaps();
		}
		//convolution
		{
			auto& engineBufferRenderFeature = m_renderer.GetFeature<Core::Rendering::EngineBufferRenderFeature>();
			auto& frameDescriptor = m_renderer.GetFrameDescriptor();
			

			::Rendering::Entities::Camera skyCamera;
			skyCamera.SetPosition({ 0.0f, 0.0f, 0.0f });
			skyCamera.SetFov(90.0f);
			const auto [width, height] = irradianceBuffer.GetSize();
			irradianceBuffer.Bind();
			m_renderer.SetViewport(0, 0, width, height);
			for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex)
			{
				skyCamera.SetRotation(Maths::FQuaternion{ kFaceRotations[faceIndex] });
				skyCamera.CacheMatrices(width, height);
				engineBufferRenderFeature.SetCamera(skyCamera);
				irradianceBuffer.SetTargetDrawBuffer(faceIndex);
				m_renderer.Clear(true, true, true);
				// Draw skybox
				::Rendering::Entities::Drawable skyboxDrawable;
				skyboxDrawable.mesh = m_renderer.m_unitCube;
				skyboxDrawable.material = m_skyboxIrrandianceMaterial;
				auto stateMask = m_skyboxIrrandianceMaterial.GenerateStateMask();
				skyboxDrawable.stateMask = stateMask;
				skyboxDrawable.material.value().SetProperty("SkyboxCube", skyBoxCube.get());
				m_renderer.DrawEntity(p_pso, skyboxDrawable);
			}
			irradianceBuffer.Unbind();
			m_renderer.SetViewport(0, 0, frameDescriptor.renderWidth, frameDescriptor.renderHeight);
		}
		//prefilter
		{
			auto& engineBufferRenderFeature = m_renderer.GetFeature<Core::Rendering::EngineBufferRenderFeature>();
			auto& frameDescriptor = m_renderer.GetFrameDescriptor();

			::Rendering::Entities::Camera skyCamera;
			skyCamera.SetPosition({ 0.0f, 0.0f, 0.0f });
			skyCamera.SetFov(90.0f);
			const auto [width, height] = prefilterBuffer.GetSize();
			const uint32_t maxMipLevels = 11;
			prefilterBuffer.Bind();
			for (uint32_t mip = 0; mip < maxMipLevels; ++mip)
			{
				uint32_t mipWidth = static_cast<uint32_t>(width * std::pow(0.5f, mip));
				uint32_t mipHeight = static_cast<uint32_t>(height * std::pow(0.5f, mip));
				m_renderer.SetViewport(0, 0, mipWidth, mipHeight);
				float roughness = (float)mip / (float)(maxMipLevels - 1);
				for (uint32_t faceIndex = 0; faceIndex < 6; ++faceIndex)
				{
					skyCamera.SetRotation(Maths::FQuaternion{ kFaceRotations[faceIndex] });
					skyCamera.CacheMatrices(mipWidth, mipHeight);
					engineBufferRenderFeature.SetCamera(skyCamera);

					prefilterBuffer.AttachLevel<::Rendering::HAL::Texture>(
						prefilterCube,
						::Rendering::Settings::EFramebufferAttachment::COLOR,
						faceIndex, // Each color attachment is a face of the cubemap
						mip,
						faceIndex // Each face of the cubemap is accessed by its layer index
					);
					prefilterBuffer.SetTargetDrawBuffer(faceIndex);
					prefilterBuffer.Validate();
					m_renderer.Clear(true, true, true);
					// Draw skybox
					::Rendering::Entities::Drawable skyboxDrawable;
					skyboxDrawable.mesh = m_renderer.m_unitCube;
					skyboxDrawable.material = m_skyboxPrefilterMaterial;
					auto stateMask = m_skyboxPrefilterMaterial.GenerateStateMask();
					skyboxDrawable.stateMask = stateMask;
					skyboxDrawable.material.value().SetProperty("SkyboxCube", skyBoxCube.get());
					skyboxDrawable.material.value().SetProperty("roughness", roughness);
					m_renderer.DrawEntity(p_pso, skyboxDrawable);
				}
			}
			prefilterBuffer.Unbind();
			m_renderer.SetViewport(0, 0, frameDescriptor.renderWidth, frameDescriptor.renderHeight);
		}
		//brdf lut
		{
			auto& frameDescriptor = m_renderer.GetFrameDescriptor();

			::Rendering::Entities::Drawable blit;
			blit.mesh = m_renderer.m_unitQuad;
			blit.material = m_brdfMaterial;
			blit.stateMask.depthWriting = false;
			blit.stateMask.colorWriting = true;
			blit.stateMask.blendable = false;
			blit.stateMask.frontfaceCulling = false;
			blit.stateMask.backfaceCulling = false;
			blit.stateMask.depthTest = false;
			const auto [width, height] = brdfBuffer.GetSize();
			brdfBuffer.Bind();
			m_renderer.SetViewport(0, 0, width, height);
			m_renderer.DrawEntity(p_pso, blit);
			brdfBuffer.Unbind();
			m_renderer.SetViewport(0, 0, frameDescriptor.renderWidth, frameDescriptor.renderHeight);
		}
	}


	{
		//visible skybox
		auto& engineBufferRenderFeature = m_renderer.GetFeature<Core::Rendering::EngineBufferRenderFeature>();
		auto& frameDescriptor = m_renderer.GetFrameDescriptor();
		m_skyboxMaterial.RemoveFeature("SKY_Convert");
		engineBufferRenderFeature.SetCamera(frameDescriptor.camera.value());

		if (auto output = frameDescriptor.outputMsaaBuffer)
		{
			output.value().Bind();
		}
		// Draw skybox
		::Rendering::Entities::Drawable skyboxDrawable;
		skyboxDrawable.mesh = m_renderer.m_unitCube;
		skyboxDrawable.material = m_skyboxMaterial;
		auto stateMask = m_skyboxMaterial.GenerateStateMask();
		skyboxDrawable.stateMask = stateMask;
		skyboxDrawable.material.value().SetProperty("SkyboxCube", prefilterCube.get());
		m_renderer.DrawEntity(p_pso, skyboxDrawable);
	}

}

