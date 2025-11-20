#include <functional>
#include <tracy/Tracy.hpp>
#include <Rendering/Core/ABaseRenderer.h>
#include <Rendering/HAL/TextureHandle.h>
#include <Rendering/Resources/Loaders/TextureLoader.h>
#include <Rendering/Resources/Loaders/ShaderLoader.h>
std::atomic_bool Rendering::Core::ABaseRenderer::s_isDrawing{ false };

namespace
{
	const Rendering::Entities::Camera kDefaultCamera(Maths::FTransform());

	constexpr auto kUnitQuadVertices = std::to_array<Rendering::Geometry::Vertex>({
		{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f} }, // Bottom-left
		{ { 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f} }, // Bottom-right
		{ { 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f} }, // Top-right
		{ {-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f} }  // Top-left
		});

	constexpr auto kUnitQuadIndices = std::to_array<uint32_t>({
		0, 1, 2, // First triangle
		0, 2, 3  // Second triangle
		});

	constexpr auto kWhitePixel = std::to_array<uint8_t>({ 255, 255, 255, 255 });
	constexpr auto kBlackPixel = std::to_array<uint8_t, 6 * 4>({ 0 });
}

Rendering::Core::ABaseRenderer::ABaseRenderer(Context::Driver& p_driver) :
	m_driver(p_driver),
	m_isDrawing(false),
	m_emptyTexture2D{ Settings::ETextureType::TEXTURE_2D },
	m_emptyTextureCube{ Settings::ETextureType::TEXTURE_CUBE },
	m_unitQuad(kUnitQuadVertices, kUnitQuadIndices)
{
	const auto kEmptyTextureDesc = Settings::TextureDesc{
		.width = 1,
		.height = 1,
		.minFilter = Settings::ETextureFilteringMode::NEAREST,
		.magFilter = Settings::ETextureFilteringMode::NEAREST,
		.horizontalWrap = Settings::ETextureWrapMode::REPEAT,
		.verticalWrap = Settings::ETextureWrapMode::REPEAT,
		.internalFormat = Settings::EInternalFormat::RGBA8,
		.useMipMaps = false
	};

	m_emptyTexture2D.Allocate(kEmptyTextureDesc);
	m_emptyTexture2D.Upload(kWhitePixel.data(), Settings::EFormat::RGBA, Settings::EPixelDataType::UNSIGNED_BYTE);

	m_emptyTextureCube.Allocate(kEmptyTextureDesc);
	m_emptyTextureCube.Upload(kBlackPixel.data(), Settings::EFormat::RGBA, Settings::EPixelDataType::UNSIGNED_BYTE);
	std::string v = R"(
#version 450 core

layout(location = 0) in vec2 geo_Pos;
layout(location = 1) in vec2 geo_TexCoords;

out vec2 TexCoords;

void main()
{
    TexCoords = geo_TexCoords;
    gl_Position = vec4(geo_Pos, 0.0, 1.0);
}
)";
	std::string f = R"(
#version 450 core

in vec2 TexCoords;
out vec4 FRAGMENT_COLOR;

uniform sampler2D _InputTexture;

void main()
{
    FRAGMENT_COLOR = texture(_InputTexture, TexCoords);
}

)";
	m_presentShader = Rendering::Resources::Loaders::ShaderLoader::CreateFromSource(v, f);
	m_presentMaterial.SetShader(m_presentShader);
}

void Rendering::Core::ABaseRenderer::BeginFrame(const Data::FrameDescriptor& p_frameDescriptor)
{
	ZoneScoped;

	
	m_frameDescriptor = p_frameDescriptor;

	if (p_frameDescriptor.outputMsaaBuffer)
	{
		p_frameDescriptor.outputMsaaBuffer.value().Bind();
	}

	m_basePipelineState = m_driver.CreatePipelineState();
	SetViewport(0, 0, p_frameDescriptor.renderWidth, p_frameDescriptor.renderHeight);

	Clear(
		p_frameDescriptor.camera->GetClearColorBuffer(),
		p_frameDescriptor.camera->GetClearDepthBuffer(),
		p_frameDescriptor.camera->GetClearStencilBuffer(),
		{ p_frameDescriptor.camera.value().GetClearColor(), 1.0f }
	);

	p_frameDescriptor.camera->CacheMatrices(p_frameDescriptor.renderWidth, p_frameDescriptor.renderHeight);

	m_isDrawing = true;
	s_isDrawing.store(true);
}

void Rendering::Core::ABaseRenderer::EndFrame()
{
	ZoneScoped;

	if (m_frameDescriptor.outputMsaaBuffer)
	{
		m_frameDescriptor.outputMsaaBuffer.value().Unbind();
	}

	m_isDrawing = false;
	s_isDrawing.store(false);
}

const Rendering::Data::FrameDescriptor& Rendering::Core::ABaseRenderer::GetFrameDescriptor() const
{
	return m_frameDescriptor;
}

Rendering::Data::PipelineState Rendering::Core::ABaseRenderer::CreatePipelineState() const
{
	return m_basePipelineState;
}

bool Rendering::Core::ABaseRenderer::IsDrawing() const
{
	return m_isDrawing;
}

void Rendering::Core::ABaseRenderer::SetViewport(uint32_t p_x, uint32_t p_y, uint32_t p_width, uint32_t p_height)
{
	m_driver.SetViewport(p_x, p_y, p_width, p_height);
}

void Rendering::Core::ABaseRenderer::Clear(
	bool p_colorBuffer,
	bool p_depthBuffer,
	bool p_stencilBuffer,
	const Maths::FVector4& p_color
)
{
	ZoneScoped;
	m_driver.Clear(p_colorBuffer, p_depthBuffer, p_stencilBuffer, p_color);
}

void Rendering::Core::ABaseRenderer::Blit(
	Rendering::Data::PipelineState p_pso,
	Rendering::HAL::Framebuffer& p_src,
	Rendering::HAL::Framebuffer& p_dst,
	Rendering::Data::Material& p_material,
	Rendering::Settings::EBlitFlags p_flags
)
{
	ZoneScoped;

	const auto [srcWidth, srcHeight] = p_src.GetSize();

	if (Rendering::Settings::IsFlagSet(Rendering::Settings::EBlitFlags::RESIZE_DST_TO_MATCH_SRC, p_flags))
	{
		p_dst.Resize(srcWidth, srcHeight);
	}

	if (Rendering::Settings::IsFlagSet(Rendering::Settings::EBlitFlags::FILL_INPUT_TEXTURE, p_flags))
	{
		const auto colorTex = p_src.GetAttachment<HAL::Texture>(Settings::EFramebufferAttachment::COLOR);
		
		p_material.SetProperty("_InputTexture", &colorTex.value());
	}

	Rendering::Entities::Drawable blit;
	blit.mesh = m_unitQuad;
	blit.material = p_material;

	if (Rendering::Settings::IsFlagSet(Rendering::Settings::EBlitFlags::USE_MATERIAL_STATE_MASK, p_flags))
	{
		blit.stateMask = p_material.GenerateStateMask();
	}
	else
	{
		blit.stateMask.depthWriting = false;
		blit.stateMask.colorWriting = true;
		blit.stateMask.blendable = false;
		blit.stateMask.frontfaceCulling = false;
		blit.stateMask.backfaceCulling = false;
		blit.stateMask.depthTest = false;
	}

	p_dst.Bind();

	if (Rendering::Settings::IsFlagSet(Rendering::Settings::EBlitFlags::UPDATE_VIEWPORT_SIZE, p_flags))
	{
		const auto [dstWidth, dstHeight] = p_dst.GetSize();
		SetViewport(0, 0, dstWidth, dstHeight);
	}

	DrawEntity(p_pso, blit);
	p_dst.Unbind();
}

void Rendering::Core::ABaseRenderer::Present(Rendering::HAL::Framebuffer& p_src)
{
	ZoneScoped;
	const auto colorTex = p_src.GetAttachment<HAL::Texture>(Settings::EFramebufferAttachment::COLOR);
	
	m_presentMaterial.SetProperty("_InputTexture", &colorTex.value());
	Rendering::Entities::Drawable blit;
	blit.mesh = m_unitQuad;
	blit.material = m_presentMaterial;
	blit.stateMask.depthWriting = false;
	blit.stateMask.colorWriting = true;
	blit.stateMask.blendable = false;

	blit.stateMask.frontfaceCulling = false;
	blit.stateMask.backfaceCulling = false;
	blit.stateMask.depthTest = false;
	auto pso = CreatePipelineState();

	auto material = blit.material;
	auto mesh = blit.mesh;

	const auto gpuInstances = material.value().GetGPUInstances();

	if (mesh && material && material->IsValid() && gpuInstances > 0)
	{
		pso.depthWriting = blit.stateMask.depthWriting;
		pso.colorWriting.mask = blit.stateMask.colorWriting ? 0xFF : 0x00;
		pso.blending = blit.stateMask.blendable;
		//p_pso.userInterface = blit.stateMask.userInterface;
		pso.culling = blit.stateMask.frontfaceCulling || blit.stateMask.backfaceCulling;
		pso.depthTest = blit.stateMask.depthTest;

		if (pso.culling)
		{
			if (blit.stateMask.backfaceCulling && blit.stateMask.frontfaceCulling)
			{
				pso.cullFace = Settings::ECullFace::FRONT_AND_BACK;
			}
			else
			{
				pso.cullFace =
					blit.stateMask.backfaceCulling ?
					Settings::ECullFace::BACK :
					Settings::ECullFace::FRONT;
			}
		}

		material->Bind(nullptr);
		m_driver.Draw(pso, mesh.value(), blit.primitiveMode, gpuInstances);
		material->Unbind();
	}
}
bool Rendering::Core::ABaseRenderer::IsDrawable(const Entities::Drawable& p_drawable) const
{
	return
		p_drawable.mesh &&
		p_drawable.material &&
		p_drawable.material->IsValid() &&
		p_drawable.material->SupportsProjectionMode(m_frameDescriptor.camera->GetProjectionMode()) &&
		p_drawable.material->GetGPUInstances() > 0;
}

void Rendering::Core::ABaseRenderer::DrawEntity(
	Rendering::Data::PipelineState p_pso,
	const Entities::Drawable& p_drawable
)
{
	ZoneScoped;
	p_pso.depthWriting = p_drawable.stateMask.depthWriting;
	p_pso.colorWriting.mask = p_drawable.stateMask.colorWriting ? 0xFF : 0x00;
	p_pso.blending = p_drawable.stateMask.blendable;
	p_pso.culling = p_drawable.stateMask.frontfaceCulling || p_drawable.stateMask.backfaceCulling;
	p_pso.depthTest = p_drawable.stateMask.depthTest;

	if (p_pso.culling)
	{
		if (p_drawable.stateMask.backfaceCulling && p_drawable.stateMask.frontfaceCulling)
		{
			p_pso.cullFace = Settings::ECullFace::FRONT_AND_BACK;
		}
		else
		{
			p_pso.cullFace =
				p_drawable.stateMask.backfaceCulling ?
				Settings::ECullFace::BACK :
				Settings::ECullFace::FRONT;
		}
	}

	p_drawable.material->Bind(
		&m_emptyTexture2D,
		&m_emptyTextureCube,
		p_drawable.pass,
		p_drawable.featureSetOverride.has_value() ?
		Tools::Utils::OptRef<const Data::FeatureSet>(p_drawable.featureSetOverride.value()) :
		std::nullopt
	);

	m_driver.Draw(
		p_pso,
		p_drawable.mesh.value(),
		p_drawable.primitiveMode,
		p_drawable.material->GetGPUInstances()
	);

	p_drawable.material->Unbind();
}
