/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Core/Rendering/FramebufferUtil.h>

#include "DebugModelRenderFeature.h"
#include "DebugSceneRenderer.h"
#include "PickingRenderPass.h"
#include "Core/Global/ServiceLocator.h"
#include <Rendering/HAL/Profiling.h>
#include "PickingRenderPass.h"

Editor::Rendering::PickingRenderPass::PickingRenderPass(::Rendering::Core::CompositeRenderer& p_renderer) :
	::Rendering::Core::ARenderPass(p_renderer),
	m_actorPickingFramebuffer("ActorPicking")
{
	::Core::Rendering::FramebufferUtil::SetupFramebuffer(
		m_actorPickingFramebuffer, 1, 1, true, false, false
	);

	/* Light Material */
	m_lightMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetShader("Billboard"));
	m_lightMaterial.SetDepthTest(true);

	/* Gizmo Pickable Material */
	m_gizmoPickingMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetShader("Gizmo"));
	m_gizmoPickingMaterial.SetGPUInstances(3);
	m_gizmoPickingMaterial.SetProperty("u_IsBall", false);
	m_gizmoPickingMaterial.SetProperty("u_IsPickable", true);

	/* Picking Material */
	m_actorPickingMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().shaderManager[":Shaders\\Unlit.ovfx"]);
	m_actorPickingMaterial.SetProperty("u_Diffuse", Maths::FVector4{ 1.f, 1.f, 1.f, 1.0f });
	m_actorPickingMaterial.SetProperty("u_DiffuseMap", static_cast<::Rendering::Resources::Texture*>(nullptr));
	m_actorPickingMaterial.SetFrontfaceCulling(false);
	m_actorPickingMaterial.SetBackfaceCulling(false);
}

Editor::Rendering::PickingRenderPass::~PickingRenderPass() noexcept
{
}


Editor::Rendering::PickingRenderPass::PickingResult Editor::Rendering::PickingRenderPass::ReadbackPickingResult(
	const ::Core::SceneSystem::Scene& p_scene,
	uint32_t p_x,
	uint32_t p_y
)
{
	uint8_t pixel[3];

	m_actorPickingFramebuffer.ReadPixels(
		p_x, p_y, 1, 1,
		::Rendering::Settings::EPixelDataFormat::RGB,
		::Rendering::Settings::EPixelDataType::UNSIGNED_BYTE,
		pixel
	);

	uint32_t actorID = (0 << 24) | (pixel[2] << 16) | (pixel[1] << 8) | (pixel[0] << 0);
	auto actorUnderMouse = p_scene.FindActorByID(actorID);

	if (actorUnderMouse)
	{
		return Tools::Utils::OptRef(*actorUnderMouse);
	}
	else if (
		pixel[0] == 255 &&
		pixel[1] == 255 &&
		pixel[2] >= 252 &&
		pixel[2] <= 254
		)
	{
		return static_cast<Editor::Core::GizmoBehaviour::EDirection>(pixel[2] - 252);
	}

	return std::nullopt;
}

void Editor::Rendering::PickingRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	// TODO: Make sure we only renderer when the view is hovered and not being resized

	TracyGpuZone("PickingRenderPass");

	using namespace ::Core::Rendering;

	assert(m_renderer.HasDescriptor<SceneRenderer::SceneDescriptor>(), "Cannot find SceneDescriptor attached to this renderer");
	assert(m_renderer.HasDescriptor<DebugSceneRenderer::DebugSceneDescriptor>(), "Cannot find DebugSceneDescriptor attached to this renderer");

	auto& sceneDescriptor = m_renderer.GetDescriptor<SceneRenderer::SceneDescriptor>();
	auto& debugSceneDescriptor = m_renderer.GetDescriptor<DebugSceneRenderer::DebugSceneDescriptor>();
	auto& frameDescriptor = m_renderer.GetFrameDescriptor();
	auto& scene = sceneDescriptor.scene;

	m_actorPickingFramebuffer.Resize(frameDescriptor.renderWidth, frameDescriptor.renderHeight);

	m_actorPickingFramebuffer.Bind();

	auto pso = m_renderer.CreatePipelineState();

	m_renderer.Clear(true, true, true);

	DrawPickableModels(pso, scene);
	DrawPickableCameras(pso, scene);
	DrawPickableLights(pso, scene);

	if (debugSceneDescriptor.selectedActor)
	{
		auto& selectedActor = debugSceneDescriptor.selectedActor.value();

		DrawPickableGizmo(
			pso,
			selectedActor.transform.GetWorldPosition(),
			selectedActor.transform.GetWorldRotation(),
			debugSceneDescriptor.gizmoOperation
		);
	}

	m_actorPickingFramebuffer.Unbind();

	if (auto output = frameDescriptor.outputBuffer)
	{
		output.value().Bind();
	}
}

void PreparePickingMaterial(Core::ECS::Actor& p_actor, Core::Resources::Material& p_material)
{
	uint32_t actorID = static_cast<uint32_t>(p_actor.GetID());

	auto bytes = reinterpret_cast<uint8_t*>(&actorID);
	auto color = Maths::FVector4{ bytes[0] / 255.0f, bytes[1] / 255.0f, bytes[2] / 255.0f, 1.0f };

	p_material.SetProperty("u_Diffuse", color);
}

void Editor::Rendering::PickingRenderPass::DrawPickableModels(
	::Rendering::Data::PipelineState p_pso,
	::Core::SceneSystem::Scene& p_scene
)
{
	for (auto modelRenderer : p_scene.GetFastAccessComponents().modelRenderers)
	{
		auto& actor = modelRenderer->owner;

		if (actor.IsActive())
		{
			if (auto model = modelRenderer->GetModel())
			{
				if (auto materialRenderer = modelRenderer->owner.GetComponent<::Core::ECS::Components::CMaterialRenderer>())
				{
					const auto& materials = materialRenderer->GetMaterials();
					const auto& modelMatrix = actor.transform.GetWorldMatrix();

					PreparePickingMaterial(actor, m_actorPickingMaterial);

					for (auto mesh : model->GetMeshes())
					{
						auto stateMask = m_actorPickingMaterial.GenerateStateMask();

						// erride the state mask to use the material state mask (if this one is valid)
						if (auto material = materials.at(mesh->GetMaterialIndex()); material && material->IsValid())
						{
							stateMask = material->GenerateStateMask();
						}

						::Rendering::Entities::Drawable drawable;
						drawable.mesh = *mesh;
						drawable.material = m_actorPickingMaterial;
						drawable.stateMask = stateMask;

						drawable.AddDescriptor<::Core::Rendering::EngineDrawableDescriptor>({
							modelMatrix
							});

						m_renderer.DrawEntity(p_pso, drawable);
					}
				}
			}
		}
	}
}

void Editor::Rendering::PickingRenderPass::DrawPickableCameras(
	::Rendering::Data::PipelineState p_pso,
	::Core::SceneSystem::Scene& p_scene
)
{
	for (auto camera : p_scene.GetFastAccessComponents().cameras)
	{
		auto& actor = camera->owner;

		if (actor.IsActive())
		{
			PreparePickingMaterial(actor, m_actorPickingMaterial);
			auto& cameraModel = *::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetModel("Camera");
			auto translation = Maths::FMatrix4::Translation(actor.transform.GetWorldPosition());
			auto rotation = Maths::FQuaternion::ToMatrix4(actor.transform.GetWorldRotation());
			auto modelMatrix = translation * rotation;

			m_renderer.GetFeature<DebugModelRenderFeature>()
				.DrawModelWithSingleMaterial(p_pso, cameraModel, m_actorPickingMaterial, modelMatrix);
		}
	}
}

void Editor::Rendering::PickingRenderPass::DrawPickableLights(
	::Rendering::Data::PipelineState p_pso,
	::Core::SceneSystem::Scene& p_scene
)
{
	if (1.0f > 0.001f)
	{
		m_renderer.Clear(false, true, false);

		m_lightMaterial.SetProperty("u_Scale", 1.0f * 0.1f);

		for (auto light : p_scene.GetFastAccessComponents().lights)
		{
			auto& actor = light->owner;

			if (actor.IsActive())
			{
				PreparePickingMaterial(actor, m_lightMaterial);
				auto& lightModel = *::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetModel("Vertical_Plane");
				auto modelMatrix = Maths::FMatrix4::Translation(actor.transform.GetWorldPosition());

				m_renderer.GetFeature<DebugModelRenderFeature>()
					.DrawModelWithSingleMaterial(p_pso, lightModel, m_lightMaterial, modelMatrix);
			}
		}
	}
}

void Editor::Rendering::PickingRenderPass::DrawPickableGizmo(
	::Rendering::Data::PipelineState p_pso,
	const Maths::FVector3& p_position,
	const Maths::FQuaternion& p_rotation,
	Editor::Core::EGizmoOperation p_operation
)
{
	auto modelMatrix =
		Maths::FMatrix4::Translation(p_position) *
		Maths::FQuaternion::ToMatrix4(Maths::FQuaternion::Normalize(p_rotation));

	auto arrowModel = ::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetModel("Arrow_Picking");

	m_renderer.GetFeature<DebugModelRenderFeature>()
		.DrawModelWithSingleMaterial(p_pso, *arrowModel, m_gizmoPickingMaterial, modelMatrix);
}
