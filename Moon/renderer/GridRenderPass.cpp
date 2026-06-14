#include "EditorResources.h"
#include "DebugModelRenderFeature.h"
#include "GridRenderPass.h"
#include "Core/Global/ServiceLocator.h"
#include "Settings/DebugSetting.h"
#include "Core/Rendering/SceneRenderer.h"
#include "Core/Rendering/EngineBufferRenderFeature.h"
#include <Rendering/Features/DebugShapeRenderFeature.h>
#include <Rendering/HAL/Profiling.h>



Editor::Rendering::GridRenderPass::GridRenderPass(::Rendering::Core::CompositeRenderer& p_renderer) :

	::Rendering::Core::ARenderPass(p_renderer)
{
	/* Grid Material */
	m_gridMaterial.SetShader(::Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetShader("Grid"));
	m_gridMaterial.SetBlendable(true);
	m_gridMaterial.SetBackfaceCulling(true);
	m_gridMaterial.SetDepthWriting(false);
	m_gridMaterial.SetDepthTest(true);


	::Rendering::Settings::TextureDesc colorDesc{
		.width = 1,
		.height = 1,
		.minFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
		.magFilter = ::Rendering::Settings::ETextureFilteringMode::LINEAR,
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

	auto renderTexture = std::make_shared<::Rendering::HAL::Texture>(
		::Rendering::Settings::ETextureType::TEXTURE_2D, "peelcolor");
	renderTexture->Allocate(colorDesc);
	auto depthTexture = std::make_shared<::Rendering::HAL::Texture>(
		::Rendering::Settings::ETextureType::TEXTURE_2D, "peeldepth");
	depthTexture->Allocate(depthDesc);
	depthTexture->SetBorderColor(Maths::FVector4::One);
	m_mirroFbo.Attach<::Rendering::HAL::Texture>(renderTexture, ::Rendering::Settings::EFramebufferAttachment::COLOR);
	m_mirroFbo.Attach<::Rendering::HAL::Texture>(depthTexture, ::Rendering::Settings::EFramebufferAttachment::DEPTH);
}

void Editor::Rendering::GridRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	
	ZoneScoped;
	TracyGpuZone("GridRenderPass");
	
	assert(m_renderer.HasDescriptor<GridDescriptor>()&&"Cannot find GridDescriptor attached to this renderer");
	assert(m_renderer.HasFeature<::Rendering::Features::DebugShapeRenderFeature>()&& "Cannot find DebugShapeRenderFeature attached to this renderer");
	assert(m_renderer.HasFeature<Editor::Rendering::DebugModelRenderFeature>()&&"Cannot find DebugModelRenderFeature attached to this renderer");

	auto& frameDesc = m_renderer.GetFrameDescriptor();
	auto& gridDescriptor = m_renderer.GetDescriptor<GridDescriptor>();
	auto& debugShapeRenderer = m_renderer.GetFeature<::Rendering::Features::DebugShapeRenderFeature>();
	auto plane=Maths::FVector4(gridDescriptor.mirrorPlaneNormal,-gridDescriptor.mirrorPlaneNormal.Dot(gridDescriptor.mirrorPlaneCenter));
	auto& engineBufferRenderFeature =m_renderer.GetFeature<::Core::Rendering::EngineBufferRenderFeature>();
	Maths::FMatrix4 mirrorMat=Maths::FMatrix4::MirrorPlane(plane.x, plane.y, plane.z, plane.w);
	Maths::FMatrix4 viewMat= frameDesc.camera->GetViewMatrix();
	viewMat =  viewMat*mirrorMat ;
	engineBufferRenderFeature.SetViewMatrix(viewMat);
	engineBufferRenderFeature.SetViewPos(mirrorMat.MulPoint(frameDesc.camera->GetPosition()));
	engineBufferRenderFeature.SetMirrorPlane(mirrorMat);

	/*
	Step 1. 
	Render the models to the mirror framebuffer,from the mirror camera
	*/
	const auto& filteredDrawables = m_renderer.GetDescriptor<::Core::Rendering::SceneRenderer::SceneFilteredDrawablesDescriptor>();
	auto drawMirroModels = [&](auto drawables,bool transparent) {
		for (auto& drawable : drawables)
		{
			auto drawableCopy = drawable;
			if (transparent) {
				drawableCopy.pass = "";
				
			}
			m_renderer.DrawEntity(p_pso, drawableCopy);
		}
		};
	m_mirroFbo.Bind();
	m_renderer.Clear(true, true, false, Maths::FVector4(0.7, 0.7, 0.7 ,0.2));
	drawMirroModels(filteredDrawables.opaques | std::views::values,false);
	drawMirroModels(filteredDrawables.lines | std::views::values, false);
	drawMirroModels(filteredDrawables.transparents | std::views::values,true);
	m_mirroFbo.Unbind();
	/*
	Step 2.
	Blend the mirror framebuffer to the main framebuffer with grid shader, from the main camera
	*/
	engineBufferRenderFeature.SetCamera(frameDesc.camera.value());
	auto& msaaframebuffer = frameDesc.outputMsaaBuffer.value();
	msaaframebuffer.Bind();
	auto pso = m_renderer.CreatePipelineState();
	constexpr float gridSize = 500.0f;
	Maths::FMatrix4 model = Maths::FMatrix4::ComputeTransformFromYAxisAndOrigin(-gridDescriptor.mirrorPlaneNormal, gridDescriptor.mirrorPlaneCenter, { gridSize * 2.0f, 1.f, gridSize * 2.0f });
	//Maths::FMatrix4::Translation({ gridDescriptor.viewPosition.x, 0.0f, gridDescriptor.viewPosition.z }) *
	//Maths::FMatrix4::Scaling({ gridSize * 2.0f, 1.f, gridSize * 2.0f });

	model = Maths::FMatrix4::Transpose(model);
	const auto& color = m_mirroFbo.GetAttachment<::Rendering::HAL::GLTexture>(::Rendering::Settings::EFramebufferAttachment::COLOR, 0);
	m_gridMaterial.SetProperty("u_Color", gridDescriptor.gridColor);
	m_gridMaterial.SetProperty("u_PlaneTransform", model);
	m_gridMaterial.SetProperty("u_MirrorTex", &color.value());
	m_renderer.GetFeature<DebugModelRenderFeature>()
		.DrawModelWithSingleMaterial(pso, *::Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetModel("Plane"), m_gridMaterial, model);

}

void Editor::Rendering::GridRenderPass::ResizeRenderer(int width, int height)
{
	m_mirroFbo.Resize(width,height);
}
