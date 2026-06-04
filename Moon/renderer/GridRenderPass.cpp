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
	m_gridMaterial.SetBackfaceCulling(false);
	m_gridMaterial.SetDepthWriting(false);
	m_gridMaterial.SetDepthTest(true);

	
	MOON::DebugSettings::instance().addCallBack("showGrid","Default", [this](MOON::NodeBase* self) {
		bool value = self->getData<bool>();
		this->SetEnabled(value);
		});
}

void Editor::Rendering::GridRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	
	ZoneScoped;
	TracyGpuZone("GridRenderPass");
	
	assert(m_renderer.HasDescriptor<GridDescriptor>()&&"Cannot find GridDescriptor attached to this renderer");
	assert(m_renderer.HasFeature<::Rendering::Features::DebugShapeRenderFeature>()&& "Cannot find DebugShapeRenderFeature attached to this renderer");
	assert(m_renderer.HasFeature<Editor::Rendering::DebugModelRenderFeature>()&&"Cannot find DebugModelRenderFeature attached to this renderer");

	auto& gridDescriptor = m_renderer.GetDescriptor<GridDescriptor>();
	auto& debugShapeRenderer = m_renderer.GetFeature<::Rendering::Features::DebugShapeRenderFeature>();
	auto plane=Maths::FVector4(gridDescriptor.mirrorPlaneNormal,-gridDescriptor.mirrorPlaneNormal.Dot(gridDescriptor.mirrorPlaneCenter));
	m_renderer.GetFeature<::Core::Rendering::EngineBufferRenderFeature>().SetMirrorPlane(plane.x, plane.y, plane.z, plane.w);
	auto pso = m_renderer.CreatePipelineState();

	constexpr float gridSize = 500.0f;

	Maths::FMatrix4 model = Maths::FMatrix4::ComputeTransformFromYAxisAndOrigin(-gridDescriptor.mirrorPlaneNormal, gridDescriptor.mirrorPlaneCenter, { gridSize * 2.0f, 1.f, gridSize * 2.0f });
		//Maths::FMatrix4::Translation({ gridDescriptor.viewPosition.x, 0.0f, gridDescriptor.viewPosition.z }) *
		//Maths::FMatrix4::Scaling({ gridSize * 2.0f, 1.f, gridSize * 2.0f });
	model=Maths::FMatrix4::Transpose(model);
	m_gridMaterial.SetProperty("u_Color", gridDescriptor.gridColor);
	m_gridMaterial.SetProperty("u_PlaneTransform",model);

	

	constexpr float kLineWidth = 1.0f;

	debugShapeRenderer.DrawLine(pso, Maths::FVector3(-gridSize + gridDescriptor.viewPosition.x, 0.0f, 0.0f), Maths::FVector3(gridSize + gridDescriptor.viewPosition.x, 0.0f, 0.0f), Maths::FVector3::Right, kLineWidth);
	debugShapeRenderer.DrawLine(pso, Maths::FVector3(0.0f, -gridSize + gridDescriptor.viewPosition.y, 0.0f), Maths::FVector3(0.0f, gridSize + gridDescriptor.viewPosition.y, 0.0f), Maths::FVector3::Up, kLineWidth);
	debugShapeRenderer.DrawLine(pso, Maths::FVector3(0.0f, 0.0f, -gridSize + gridDescriptor.viewPosition.z), Maths::FVector3(0.0f, 0.0f, gridSize + gridDescriptor.viewPosition.z), Maths::FVector3::Forward, kLineWidth);


	auto& sceneDescriptor = m_renderer.GetDescriptor<::Core::Rendering::SceneRenderer::SceneDescriptor>();
	//auto& debugSceneDescriptor = m_renderer.GetDescriptor<DebugSceneRenderer::DebugSceneDescriptor>();
	auto& frameDescriptor = m_renderer.GetFrameDescriptor();const auto& filteredDrawables = m_renderer.GetDescriptor<::Core::Rendering::SceneRenderer::SceneFilteredDrawablesDescriptor>();
	auto drawMirroModels = [&](auto drawables) {
		for (auto& drawable : drawables)
		{
				const std::string pickingPassName = "MirroPlane_pass";
				::Rendering::Entities::Drawable finalDrawable = drawable;
				finalDrawable.pass = pickingPassName;
				m_renderer.DrawEntity(p_pso, finalDrawable);
		}
		};
	drawMirroModels(filteredDrawables.opaques | std::views::values);
	drawMirroModels(filteredDrawables.transparents | std::views::values);

	m_renderer.GetFeature<DebugModelRenderFeature>()
		.DrawModelWithSingleMaterial(pso, *::Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetModel("Plane"), m_gridMaterial, model);

}
