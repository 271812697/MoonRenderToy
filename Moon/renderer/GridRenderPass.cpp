#include "EditorResources.h"
#include "DebugModelRenderFeature.h"
#include "GridRenderPass.h"
#include "Core/Global/ServiceLocator.h"
#include "Settings/DebugSetting.h"
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

	auto node = MOON::DebugSettings::instance().getNode("showGrid");
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

	auto pso = m_renderer.CreatePipelineState();

	constexpr float gridSize = 5000.0f;

	Maths::FMatrix4 model =
		Maths::FMatrix4::Translation({ gridDescriptor.viewPosition.x, 0.0f, gridDescriptor.viewPosition.z }) *
		Maths::FMatrix4::Scaling({ gridSize * 2.0f, 1.f, gridSize * 2.0f });

	m_gridMaterial.SetProperty("u_Color", gridDescriptor.gridColor);

	m_renderer.GetFeature<DebugModelRenderFeature>()
		.DrawModelWithSingleMaterial(pso, *::Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetModel("Plane"), m_gridMaterial, model);

	constexpr float kLineWidth = 1.0f;

	debugShapeRenderer.DrawLine(pso, Maths::FVector3(-gridSize + gridDescriptor.viewPosition.x, 0.0f, 0.0f), Maths::FVector3(gridSize + gridDescriptor.viewPosition.x, 0.0f, 0.0f), Maths::FVector3::Right, kLineWidth);
	debugShapeRenderer.DrawLine(pso, Maths::FVector3(0.0f, -gridSize + gridDescriptor.viewPosition.y, 0.0f), Maths::FVector3(0.0f, gridSize + gridDescriptor.viewPosition.y, 0.0f), Maths::FVector3::Up, kLineWidth);
	debugShapeRenderer.DrawLine(pso, Maths::FVector3(0.0f, 0.0f, -gridSize + gridDescriptor.viewPosition.z), Maths::FVector3(0.0f, 0.0f, gridSize + gridDescriptor.viewPosition.z), Maths::FVector3::Forward, kLineWidth);
}
