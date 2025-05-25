/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <functional>

#include <tracy/Tracy.hpp>

#include <Rendering/Core/ABaseRenderer.h>
#include <Rendering/HAL/TextureHandle.h>
#include <Rendering/Resources/Loaders/TextureLoader.h>
#include <Rendering/Resources/Loaders/ShaderLoader.h>

std::atomic_bool Rendering::Core::ABaseRenderer::s_isDrawing{ false };

const Rendering::Entities::Camera kDefaultCamera(Maths::FTransform());

std::unique_ptr<Rendering::Resources::Mesh> CreateUnitQuad()
{
	const std::vector<Rendering::Geometry::Vertex> vertices = {
		{ {-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f} }, // Bottom-left
		{ { 1.0f, -1.0f, 0.0f}, {1.0f, 0.0f} }, // Bottom-right
		{ { 1.0f,  1.0f, 0.0f}, {1.0f, 1.0f} }, // Top-right
		{ {-1.0f,  1.0f, 0.0f}, {0.0f, 1.0f} }  // Top-left
	};

	const std::vector<uint32_t> indices = {
		0, 1, 2, // First triangle
		0, 2, 3  // Second triangle
	};

	return std::make_unique<Rendering::Resources::Mesh>(vertices, indices, 0);
}

Rendering::Core::ABaseRenderer::ABaseRenderer(Context::Driver& p_driver) :
	m_driver(p_driver),
	m_isDrawing(false),
	m_emptyTexture(Rendering::Resources::Loaders::TextureLoader::CreatePixel(255, 255, 255, 255)),
	m_unitQuad(CreateUnitQuad())
{

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

Rendering::Core::ABaseRenderer::~ABaseRenderer()
{
	Rendering::Resources::Loaders::TextureLoader::Destroy(m_emptyTexture);
	Rendering::Resources::Loaders::ShaderLoader::Destroy(m_presentShader);
}

void Rendering::Core::ABaseRenderer::BeginFrame(const Data::FrameDescriptor& p_frameDescriptor)
{
	ZoneScoped;

	ASSERT(!s_isDrawing, "Cannot call BeginFrame() when previous frame hasn't finished.");
	ASSERT(p_frameDescriptor.IsValid(), "Invalid FrameDescriptor!");

	m_frameDescriptor = p_frameDescriptor;

	if (p_frameDescriptor.outputBuffer)
	{
		p_frameDescriptor.outputBuffer.value().Bind();
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

	ASSERT(s_isDrawing, "Cannot call EndFrame() before calling BeginFrame()");

	if (m_frameDescriptor.outputBuffer)
	{
		m_frameDescriptor.outputBuffer.value().Unbind();
	}

	m_isDrawing = false;
	s_isDrawing.store(false);
}

const Rendering::Data::FrameDescriptor& Rendering::Core::ABaseRenderer::GetFrameDescriptor() const
{
	ASSERT(m_isDrawing, "Cannot call GetFrameDescriptor() outside of a frame");
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

	ASSERT(m_unitQuad != nullptr, "Invalid unit quad mesh, cannot blit!");

	auto [srcWidth, srcHeight] = p_src.GetSize();

	if (Rendering::Settings::IsFlagSet(Rendering::Settings::EBlitFlags::RESIZE_DST_TO_MATCH_SRC, p_flags))
	{
		p_dst.Resize(srcWidth, srcHeight);
	}

	if (Rendering::Settings::IsFlagSet(Rendering::Settings::EBlitFlags::FILL_INPUT_TEXTURE, p_flags))
	{
		const auto colorTex = p_src.GetAttachment<HAL::Texture>(Settings::EFramebufferAttachment::COLOR);
		ASSERT(colorTex.has_value(), "Invalid color attachment");
		p_material.SetProperty("_InputTexture", &colorTex.value());
	}

	Rendering::Entities::Drawable blit;
	blit.mesh = *m_unitQuad;
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
		blit.stateMask.userInterface = false;
		blit.stateMask.frontfaceCulling = false;
		blit.stateMask.backfaceCulling = false;
		blit.stateMask.depthTest = false;
	}

	p_dst.Bind();

	if (Rendering::Settings::IsFlagSet(Rendering::Settings::EBlitFlags::UPDATE_VIEWPORT_SIZE, p_flags))
	{
		auto [dstWidth, dstHeight] = p_dst.GetSize();
		SetViewport(0, 0, dstWidth, dstHeight);
	}

	DrawEntity(p_pso, blit);
	p_dst.Unbind();
}

void Rendering::Core::ABaseRenderer::Present(Rendering::HAL::Framebuffer& p_src)
{
	ZoneScoped;

	ASSERT(m_unitQuad != nullptr, "Invalid unit quad mesh, cannot blit!");
	const auto colorTex = p_src.GetAttachment<HAL::Texture>(Settings::EFramebufferAttachment::COLOR);
	ASSERT(colorTex.has_value(), "Invalid color attachment");
	m_presentMaterial.SetProperty("_InputTexture", &colorTex.value());
	Rendering::Entities::Drawable blit;
	blit.mesh = *m_unitQuad;
	blit.material = m_presentMaterial;
	blit.stateMask.depthWriting = false;
	blit.stateMask.colorWriting = true;
	blit.stateMask.blendable = false;
	blit.stateMask.userInterface = false;
	blit.stateMask.frontfaceCulling = false;
	blit.stateMask.backfaceCulling = false;
	blit.stateMask.depthTest = false;
	auto pso = CreatePipelineState();
	DrawEntity(pso, blit);
}

void Rendering::Core::ABaseRenderer::DrawEntity(
	Rendering::Data::PipelineState p_pso,
	const Entities::Drawable& p_drawable
)
{
	ZoneScoped;

	auto material = p_drawable.material;
	auto mesh = p_drawable.mesh;

	const auto gpuInstances = material.value().GetGPUInstances();

	if (mesh && material && material->IsValid() && gpuInstances > 0)
	{
		p_pso.depthWriting = p_drawable.stateMask.depthWriting;
		p_pso.colorWriting.mask = p_drawable.stateMask.colorWriting ? 0xFF : 0x00;
		p_pso.blending = p_drawable.stateMask.blendable;
		p_pso.userInterface = p_drawable.stateMask.userInterface;
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

		material->Bind(&m_emptyTexture->GetTexture());
		m_driver.Draw(p_pso, mesh.value(), p_drawable.primitiveMode, gpuInstances);
		material->Unbind();
	}
}
