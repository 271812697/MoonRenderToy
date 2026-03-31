#include <ranges>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Core/Rendering/GbufferPass.h>
#include <Core/Rendering/ReflectionRenderPass.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Core/ResourceManagement/TextureManager.h>
#include <Core/Rendering/FramebufferUtil.h>
#include <Rendering/HAL/Renderbuffer.h>
#include <Rendering/HAL/Profiling.h>
#include <Rendering/Resources/Loaders/TextureLoader.h>


Core::Rendering::GbufferPass::GbufferPass(::Rendering::Core::CompositeRenderer& p_renderer) :
	::Rendering::Core::ARenderPass(p_renderer)
{
	gbufferMaterial.SetShader(GetShaderService[":Shaders\\gbuffer.ovfx"]);
	gbufferMaterial.SetBackfaceCulling(false);

	::Rendering::Settings::TextureDesc colorDesc{
			.width = 1,
			.height = 1,
			.minFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST,
			.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST,
			.horizontalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_BORDER,
			.verticalWrap = ::Rendering::Settings::ETextureWrapMode::CLAMP_TO_BORDER,
			.internalFormat = ::Rendering::Settings::EInternalFormat::RGBA32F,
			.useMipMaps = false,
			.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
				.format = ::Rendering::Settings::EFormat::RGBA,
				.type = ::Rendering::Settings::EPixelDataType::FLOAT
			}
	};
	::Rendering::Settings::TextureDesc depthDesc{
		.width = 1,
		.height = 1,
		.minFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST,
		.magFilter = ::Rendering::Settings::ETextureFilteringMode::NEAREST,
		.horizontalWrap = ::Rendering::Settings::ETextureWrapMode::REPEAT,
		.verticalWrap = ::Rendering::Settings::ETextureWrapMode::REPEAT,
		.internalFormat = ::Rendering::Settings::EInternalFormat::DEPTH_COMPONENT32,
		.useMipMaps = false,
		.mutableDesc = ::Rendering::Settings::MutableTextureDesc{
			.format = ::Rendering::Settings::EFormat::DEPTH_COMPONENT,
			.type = ::Rendering::Settings::EPixelDataType::FLOAT
		}
	};

	gbufferData.position = std::make_shared<::Rendering::HAL::Texture>(
		::Rendering::Settings::ETextureType::TEXTURE_2D, "position");
	gbufferData.position->Allocate(colorDesc);
	gbufferData.normal = std::make_shared<::Rendering::HAL::Texture>(
		::Rendering::Settings::ETextureType::TEXTURE_2D, "normal");
	gbufferData.normal->Allocate(colorDesc);
	auto depthTexture = std::make_shared<::Rendering::HAL::Texture>(
		::Rendering::Settings::ETextureType::TEXTURE_2D, "depth");
	depthTexture->Allocate(depthDesc);
	depthTexture->SetBorderColor(Maths::FVector4::One);
	gbuffer.Attach<::Rendering::HAL::Texture>(gbufferData.position, ::Rendering::Settings::EFramebufferAttachment::COLOR,0);
	gbuffer.Attach<::Rendering::HAL::Texture>(gbufferData.normal, ::Rendering::Settings::EFramebufferAttachment::COLOR,1);
	gbuffer.Attach<::Rendering::HAL::Texture>(depthTexture, ::Rendering::Settings::EFramebufferAttachment::DEPTH);
	//gbuffer.SetTargetDrawBuffer(0);
	gbuffer.SetDrawBuffers({0,1});
	if (!gbuffer.Validate()) {
		std::cout << "invalidate buffer" << std::endl;
	}

}


void Core::Rendering::GbufferPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	ZoneScoped;
	TracyGpuZone("SkyboxRenderPass");
	using namespace Core::Rendering;
	return;

	gbuffer.Bind();
	m_renderer.Clear(true, true, false, Maths::FVector4(0, 0, 0, 1.0));

	const auto& drawables = m_renderer.GetDescriptor<SceneRenderer::SceneFilteredDrawablesDescriptor>();

	for (auto drawable : drawables.opaques | std::views::values)
	{

		drawable.pass = "";
		drawable.stateMask.depthWriting = true;
		drawable.stateMask.colorWriting = true;
		drawable.stateMask.blendable = false;
		drawable.stateMask.frontfaceCulling = false;
		drawable.stateMask.backfaceCulling = false;
		drawable.stateMask.depthTest = true;
		drawable.material = gbufferMaterial;
		m_renderer.DrawEntity(p_pso, drawable);
	}
	gbuffer.Unbind();
	auto& mssaaframebuffer = m_renderer.GetFrameDescriptor().outputMsaaBuffer.value();
	mssaaframebuffer.Bind();
	const auto& content = gbuffer.GetAttachment<::Rendering::HAL::GLTexture>(::Rendering::Settings::EFramebufferAttachment::COLOR, 0);
	m_renderer.Present(content.value());
}

void Core::Rendering::GbufferPass::ResizeRenderer(int width, int height)
{
	gbuffer.Resize(width, height);
}

