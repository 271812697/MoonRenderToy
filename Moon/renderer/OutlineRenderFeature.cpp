#include "DebugModelRenderFeature.h"
#include "OutlineRenderFeature.h"
#include <Core/ECS/Components/CMaterialRenderer.h>
#include "Core/Global/ServiceLocator.h"
#include <Rendering/Utils/Conversions.h>

constexpr uint32_t kStencilMask = 0xFF;
constexpr int32_t kStencilReference = 1;

Editor::Rendering::OutlineRenderFeature::OutlineRenderFeature(::Rendering::Core::CompositeRenderer& p_renderer) :
	::Rendering::Features::ARenderFeature(p_renderer)
{

	/* Stencil Fill Material */
	m_stencilFillMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().shaderManager[":Shaders\\Unlit.ovfx"]);
	m_stencilFillMaterial.SetBackfaceCulling(true);
	m_stencilFillMaterial.SetDepthTest(false);
	m_stencilFillMaterial.SetColorWriting(false);
	m_stencilFillMaterial.SetProperty("u_DiffuseMap", static_cast<::Rendering::Resources::Texture*>(nullptr));

	/* Outline Material */
	m_outlineMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().shaderManager[":Shaders\\Unlit.ovfx"]);
	m_outlineMaterial.SetProperty("u_DiffuseMap", static_cast<::Rendering::Resources::Texture*>(nullptr));
	m_outlineMaterial.SetDepthTest(false);
}

void Editor::Rendering::OutlineRenderFeature::DrawOutline(
	::Core::ECS::Actor& p_actor,
	const Maths::FVector4& p_color,
	float p_thickness
)
{
	DrawStencilPass(p_actor);
	DrawOutlinePass(p_actor, p_color, p_thickness);
}

void Editor::Rendering::OutlineRenderFeature::DrawStencilPass(::Core::ECS::Actor& p_actor)
{
	auto pso = m_renderer.CreatePipelineState();

	pso.stencilTest = true;
	pso.stencilWriteMask = kStencilMask;
	pso.stencilFuncRef = kStencilReference;
	pso.stencilFuncMask = kStencilMask;
	pso.stencilOpFail = ::Rendering::Settings::EOperation::REPLACE;
	pso.depthOpFail = ::Rendering::Settings::EOperation::REPLACE;
	pso.bothOpFail = ::Rendering::Settings::EOperation::REPLACE;
	pso.colorWriting.mask = 0x00;

	DrawActorToStencil(pso, p_actor);
}

void Editor::Rendering::OutlineRenderFeature::DrawOutlinePass(::Core::ECS::Actor& p_actor, const Maths::FVector4& p_color, float p_thickness)
{
	auto pso = m_renderer.CreatePipelineState();

	pso.stencilTest = true;
	pso.stencilOpFail = ::Rendering::Settings::EOperation::KEEP;
	pso.depthOpFail = ::Rendering::Settings::EOperation::KEEP;
	pso.bothOpFail = ::Rendering::Settings::EOperation::REPLACE;
	pso.stencilFuncOp = ::Rendering::Settings::EComparaisonAlgorithm::NOTEQUAL;
	pso.stencilFuncRef = kStencilReference;
	pso.stencilFuncMask = kStencilMask;
	pso.rasterizationMode = ::Rendering::Settings::ERasterizationMode::LINE;
	pso.lineWidthPow2 = ::Rendering::Utils::Conversions::FloatToPow2(p_thickness);

	// Prepare the outline material
	m_outlineMaterial.SetProperty("u_Diffuse", p_color);

	DrawActorOutline(pso, p_actor);
}

void Editor::Rendering::OutlineRenderFeature::DrawActorToStencil(::Rendering::Data::PipelineState p_pso, ::Core::ECS::Actor& p_actor)
{
	if (p_actor.IsActive())
	{
		/* Render static mesh outline and bounding spheres */
		if (auto modelRenderer = p_actor.GetComponent<::Core::ECS::Components::CModelRenderer>(); modelRenderer && modelRenderer->GetModel())
		{
			DrawModelToStencil(p_pso, p_actor.transform.GetWorldMatrix(), *modelRenderer->GetModel());
		}

		/* Render camera component outline */
		if (auto cameraComponent = p_actor.GetComponent<::Core::ECS::Components::CCamera>(); cameraComponent)
		{
			auto translation = Maths::FMatrix4::Translation(p_actor.transform.GetWorldPosition());
			auto rotation = Maths::FQuaternion::ToMatrix4(p_actor.transform.GetWorldRotation());
			auto model = translation * rotation;
			DrawModelToStencil(p_pso, model, *::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetModel("Camera"));
		}

		for (auto& child : p_actor.GetChildren())
		{
			DrawActorToStencil(p_pso, *child);
		}
	}
}

void Editor::Rendering::OutlineRenderFeature::DrawActorOutline(::Rendering::Data::PipelineState p_pso, ::Core::ECS::Actor& p_actor)
{
	if (p_actor.IsActive())
	{
		if (auto modelRenderer = p_actor.GetComponent<::Core::ECS::Components::CModelRenderer>(); modelRenderer && modelRenderer->GetModel())
		{
			DrawModelOutline(p_pso, p_actor.transform.GetWorldMatrix(), *modelRenderer->GetModel());
		}

		if (auto cameraComponent = p_actor.GetComponent<::Core::ECS::Components::CCamera>(); cameraComponent)
		{
			auto translation = Maths::FMatrix4::Translation(p_actor.transform.GetWorldPosition());
			auto rotation = Maths::FQuaternion::ToMatrix4(p_actor.transform.GetWorldRotation());
			auto model = translation * rotation;
			DrawModelOutline(p_pso, model, *::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetModel("Camera"));
		}

		for (auto& child : p_actor.GetChildren())
		{
			DrawActorOutline(p_pso, *child);
		}
	}
}

void Editor::Rendering::OutlineRenderFeature::DrawModelToStencil(
	::Rendering::Data::PipelineState p_pso,
	const Maths::FMatrix4& p_worldMatrix,
	::Rendering::Resources::Model& p_model
)
{
	m_renderer.GetFeature<DebugModelRenderFeature>()
		.DrawModelWithSingleMaterial(p_pso, p_model, m_stencilFillMaterial, p_worldMatrix);
}

void Editor::Rendering::OutlineRenderFeature::DrawModelOutline(
	::Rendering::Data::PipelineState p_pso,
	const Maths::FMatrix4& p_worldMatrix,
	::Rendering::Resources::Model& p_model
)
{
	m_renderer.GetFeature<DebugModelRenderFeature>()
		.DrawModelWithSingleMaterial(p_pso, p_model, m_outlineMaterial, p_worldMatrix);
}
