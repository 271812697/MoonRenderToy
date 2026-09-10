#include <Core/Rendering/HzbBuildPass.h>

#include <algorithm>
#include <chrono>

#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/FramebufferUtil.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Rendering/Core/CompositeRenderer.h>
#include <Rendering/HAL/Profiling.h>
#include <Rendering/HAL/Texture.h>
#include <Rendering/Settings/EBlitFlags.h>
#include <Rendering/Settings/TextureDesc.h>

namespace
{
	using namespace ::Rendering::Settings;

	/** R32F single channel target used for every pyramid level. */
	TextureDesc MakeGridTextureDesc(uint32_t p_width, uint32_t p_height)
	{
		TextureDesc desc{};
		desc.width = std::max(1u, p_width);
		desc.height = std::max(1u, p_height);
		desc.minFilter = ETextureFilteringMode::NEAREST;
		desc.magFilter = ETextureFilteringMode::NEAREST;
		desc.horizontalWrap = ETextureWrapMode::CLAMP_TO_EDGE;
		desc.verticalWrap = ETextureWrapMode::CLAMP_TO_EDGE;
		desc.internalFormat = EInternalFormat::R32F;
		desc.useMipMaps = false;
		desc.mutableDesc = MutableTextureDesc{
			.format = EFormat::RED,
			.type = EPixelDataType::FLOAT
		};
		return desc;
	}

	/** Depth only source: the resolved (single sampled) opaque scene depth. */
	TextureDesc MakeDepthTextureDesc(uint32_t p_width, uint32_t p_height)
	{
		TextureDesc desc{};
		desc.width = std::max(1u, p_width);
		desc.height = std::max(1u, p_height);
		desc.minFilter = ETextureFilteringMode::NEAREST;
		desc.magFilter = ETextureFilteringMode::NEAREST;
		desc.horizontalWrap = ETextureWrapMode::CLAMP_TO_EDGE;
		desc.verticalWrap = ETextureWrapMode::CLAMP_TO_EDGE;
		desc.internalFormat = EInternalFormat::DEPTH_COMPONENT32F;
		desc.useMipMaps = false;
		desc.mutableDesc = MutableTextureDesc{
			.format = EFormat::DEPTH_COMPONENT,
			.type = EPixelDataType::FLOAT
		};
		return desc;
	}
}

Core::Rendering::HzbBuildPass::HzbBuildPass(::Rendering::Core::CompositeRenderer& p_renderer)
	: ::Rendering::Core::ARenderPass(p_renderer)
{
	m_reduceMaterial.SetShader(GetShaderService[":Shaders\\PostProcess\\HzbReduce.ovfx"]);
	m_reduceMaterial.SetDepthTest(false);
	m_reduceMaterial.SetDepthWriting(false);
	m_reduceMaterial.SetColorWriting(true);
	m_reduceMaterial.SetBlendable(false);
	m_reduceMaterial.SetBackfaceCulling(false);
	m_reduceMaterial.SetFrontfaceCulling(false);

	SetupTargets(1, 1);
}

void Core::Rendering::HzbBuildPass::ResizeRenderer(int p_width, int p_height)
{
	// Targets are rebuilt lazily in Draw() when the frame size changes.
	m_targetsReady = false;
}

void Core::Rendering::HzbBuildPass::ReleaseLevels()
{
	m_levels.clear();
	m_levelResolutions.clear();
	m_levelTextures.clear();
	m_gridWidth = 0;
	m_gridHeight = 0;
	m_targetsReady = false;
}

void Core::Rendering::HzbBuildPass::SetupTargets(uint32_t p_width, uint32_t p_height)
{
	p_width = std::max(1u, p_width);
	p_height = std::max(1u, p_height);

	m_resolveColor = std::make_shared<::Rendering::HAL::Texture>(
		ETextureType::TEXTURE_2D,
		"Hzb/ResolveColor"
	);
	m_resolveColor->Allocate(MakeGridTextureDesc(p_width, p_height));

	m_resolveDepth = std::make_shared<::Rendering::HAL::Texture>(
		ETextureType::TEXTURE_2D,
		"Hzb/ResolveDepth"
	);
	m_resolveDepth->Allocate(MakeDepthTextureDesc(p_width, p_height));

	m_resolveFbo.Attach<::Rendering::HAL::Texture>(
		m_resolveColor,
		EFramebufferAttachment::COLOR
	);
	m_resolveFbo.Attach<::Rendering::HAL::Texture>(
		m_resolveDepth,
		EFramebufferAttachment::DEPTH
	);
	m_resolveFbo.Validate();

	m_levels.clear();
	m_levelResolutions.clear();
	m_levelTextures.clear();

	// Half resolution per level until the grid is small enough to read back
	// cheaply. Each level stores the max depth of the 2x2 block below it.
	uint32_t levelWidth = p_width;
	uint32_t levelHeight = p_height;
	constexpr int kMaxLevelCount = 16;

	for (int i = 0; i < kMaxLevelCount; ++i)
	{
		if (levelWidth <= m_maxGridSize && levelHeight <= m_maxGridSize)
		{
			break;
		}

		levelWidth = std::max(1u, levelWidth / 2);
		levelHeight = std::max(1u, levelHeight / 2);

		auto texture = std::make_shared<::Rendering::HAL::Texture>(
			ETextureType::TEXTURE_2D,
			std::format("Hzb/Level{}", i)
		);
		texture->Allocate(MakeGridTextureDesc(levelWidth, levelHeight));

		auto framebuffer = std::make_unique<::Rendering::HAL::Framebuffer>(
			std::format("HzbLevel{}", i)
		);
		framebuffer->Attach<::Rendering::HAL::Texture>(
			texture,
			EFramebufferAttachment::COLOR
		);
		framebuffer->Validate();
		m_levels.push_back(std::move(framebuffer));

		m_levelTextures.push_back(texture);
		m_levelResolutions.emplace_back(
			static_cast<float>(levelWidth),
			static_cast<float>(levelHeight)
		);
	}

	m_gridWidth = m_levels.empty() ? 0 : static_cast<uint32_t>(levelWidth);
	m_gridHeight = m_levels.empty() ? 0 : static_cast<uint32_t>(levelHeight);
	m_width = p_width;
	m_height = p_height;
	m_targetsReady = true;
}

void Core::Rendering::HzbBuildPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	ZoneScoped;
	TracyGpuZone("HzbBuildPass");

	const auto& frameDescriptor = m_renderer.GetFrameDescriptor();
	if (!frameDescriptor.outputMsaaBuffer.has_value() || m_culler == nullptr)
	{
		return;
	}

	auto& msaaBuffer = frameDescriptor.outputMsaaBuffer.value();
	const auto [frameWidth, frameHeight] = msaaBuffer.GetSize();
	if (frameWidth == 0 || frameHeight == 0)
	{
		return;
	}

	if (!m_targetsReady || frameWidth != m_width || frameHeight != m_height)
	{
		SetupTargets(frameWidth, frameHeight);
	}

	if (m_levels.empty())
	{
		return;
	}

	const auto startTime = std::chrono::high_resolution_clock::now();

	// 1) Resolve the MSAA depth into a sampleable depth texture. This is the
	//    opaque (+ section cap) depth: transparents are depth peeled into their
	//    own layer framebuffers and never occlude anything.
	::Core::Rendering::FramebufferUtil::CopyFramebufferDepth(msaaBuffer, m_resolveFbo);

	// 2) First reduce: depth texture -> level 0.
	{
		const auto [levelWidth, levelHeight] = m_levels[0]->GetSize();
		m_reduceMaterial.SetProperty(
			"_InputResolution",
			Maths::FVector2{
				static_cast<float>(m_width),
				static_cast<float>(m_height)
			},
			true
		);

		m_levels[0]->Bind();
		m_renderer.SetViewport(0, 0, levelWidth, levelHeight);
		m_renderer.Present(*m_resolveDepth, m_reduceMaterial);
		m_levels[0]->Unbind();
	}

	// 3) Remaining levels: R32F color -> half resolution R32F color.
	for (size_t i = 1; i < m_levels.size(); ++i)
	{
		m_reduceMaterial.SetProperty("_InputResolution", m_levelResolutions[i - 1], true);
		m_renderer.Blit(
			p_pso,
			*m_levels[i - 1],
			*m_levels[i],
			m_reduceMaterial,
			::Rendering::Settings::EBlitFlags::FILL_INPUT_TEXTURE
				| ::Rendering::Settings::EBlitFlags::UPDATE_VIEWPORT_SIZE
		);
	}

	// 4) Read the smallest level back to the CPU. The grid is tiny (<= 64x64),
	//    so this stays cheap; it is consumed by the next frame's culler, which
	//    keeps the one frame latency but avoids resting on a fresh GPU sync.
	{
		// Default (reverse-Z) far value: 0 means "nothing drawn", which never
		// causes a cull even if a readback were to come back untouched.
		std::vector<float> depths(static_cast<size_t>(m_gridWidth) * m_gridHeight, 0.0f);
		m_levels.back()->ReadPixels(
			0,
			0,
			static_cast<uint32_t>(m_gridWidth),
			static_cast<uint32_t>(m_gridHeight),
			::Rendering::Settings::EPixelDataFormat::RED,
			::Rendering::Settings::EPixelDataType::FLOAT,
			depths.data()
		);
		m_culler->SetGrid(m_gridWidth, m_gridHeight, std::move(depths));
	}

	// Restore the frame's default target for the following passes.
	msaaBuffer.Bind();

	m_lastBuildTimeMs = std::chrono::duration<float, std::milli>(
		std::chrono::high_resolution_clock::now() - startTime
	).count();
}
