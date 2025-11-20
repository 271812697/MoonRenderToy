#include <Core/ECS/Components/CCamera.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CPointLight.h>
#include <Core/ECS/Components/CDirectionalLight.h>
#include <Core/ECS/Components/CSpotLight.h>

#include <Rendering/Features/DebugShapeRenderFeature.h>
#include "DebugModelRenderFeature.h"
#include "DebugSceneRenderer.h"
#include "EditorResources.h"
//#include "AView.h"
#include "GizmoBehaviour.h"
#include "Core/Global/ServiceLocator.h"
#include "GizmoRenderFeature.h"

Editor::Rendering::GizmoRenderFeature::GizmoRenderFeature(
	::Rendering::Core::CompositeRenderer& p_renderer,
	::Rendering::Features::EFeatureExecutionPolicy p_executionPolicy
) :
	::Rendering::Features::ARenderFeature(p_renderer, p_executionPolicy)
{
	/* Gizmo Arrow Material */
	m_gizmoArrowMaterial.SetShader(::Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetShader("Gizmo"));
	m_gizmoArrowMaterial.SetGPUInstances(3);
	m_gizmoArrowMaterial.SetProperty("u_IsBall", false);
	m_gizmoArrowMaterial.SetProperty("u_IsPickable", false);

	/* Gizmo Ball Material */
	m_gizmoBallMaterial.SetShader(::Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetShader("Gizmo"));
	m_gizmoBallMaterial.SetProperty("u_IsBall", true);
	m_gizmoBallMaterial.SetProperty("u_IsPickable", false);
}

std::string GetArrowModelName(Editor::Core::EGizmoOperation p_operation)
{
	using namespace Editor::Core;

	switch (p_operation)
	{
	default:
	case EGizmoOperation::TRANSLATE: return "Arrow_Translate";
	case EGizmoOperation::ROTATE: return "Arrow_Rotate";
	case EGizmoOperation::SCALE: return "Arrow_Scale";
	}
}

int GetAxisIndexFromDirection(std::optional<Editor::Core::GizmoBehaviour::EDirection> p_direction)
{
	return p_direction ? static_cast<int>(p_direction.value()) : -1;
}

void Editor::Rendering::GizmoRenderFeature::DrawGizmo(
	const Maths::FVector3& p_position,
	const Maths::FQuaternion& p_rotation,
	Editor::Core::EGizmoOperation p_operation,
	bool p_pickable,
	std::optional<Editor::Core::GizmoBehaviour::EDirection> p_highlightedDirection
)
{
	auto pso = m_renderer.CreatePipelineState();

	auto modelMatrix =
		Maths::FMatrix4::Translation(p_position) *
		Maths::FQuaternion::ToMatrix4(Maths::FQuaternion::Normalize(p_rotation));

	if (auto sphereModel = ::Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetModel("Sphere"))
	{
		auto sphereModelMatrix = modelMatrix * Maths::FMatrix4::Scaling({ 0.1f, 0.1f, 0.1f });

		m_renderer.GetFeature<DebugModelRenderFeature>()
			.DrawModelWithSingleMaterial(
				pso,
				*sphereModel,
				m_gizmoBallMaterial,
				sphereModelMatrix
			);
	}

	auto arrowModelName = GetArrowModelName(p_operation);

	if (auto arrowModel = ::Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetModel(arrowModelName))
	{
		const auto axisIndex = GetAxisIndexFromDirection(p_highlightedDirection);
		m_gizmoArrowMaterial.SetProperty("u_HighlightedAxis", axisIndex);
		
		m_renderer.GetFeature<DebugModelRenderFeature>()
			.DrawModelWithSingleMaterial(
				pso,
				*arrowModel,
				m_gizmoArrowMaterial,
				modelMatrix
			);
	}
}
