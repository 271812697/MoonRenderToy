/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Core/ECS/Components/CCamera.h>
#include <Core/ECS/Components/CDirectionalLight.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CPhysicalBox.h>
#include <Core/ECS/Components/CPhysicalCapsule.h>
#include <Core/ECS/Components/CPhysicalSphere.h>
#include <Core/ECS/Components/CPointLight.h>
#include <Core/ECS/Components/CSpotLight.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>

#include <Debug/Assertion.h>


#include "EditorResources.h"
#include "AView.h"

#include "DebugModelRenderFeature.h"
#include "DebugSceneRenderer.h"
#include "GizmoRenderFeature.h"
#include "GridRenderPass.h"
#include "OutlineRenderFeature.h"
#include "PickingRenderPass.h"
//#include <Editor/Settings/EditorSettings.h>
#include "core/Global/ServiceLocator.h"
#include <Rendering/Features/DebugShapeRenderFeature.h>
#include <Rendering/Features/FrameInfoRenderFeature.h>
#include <Rendering/HAL/Profiling.h>
#include <Rendering/Resources/Texture.h>

using namespace Maths;
using namespace Rendering::Resources;
using namespace Core::Resources;

const Maths::FVector3 kDebugBoundsColor = { 1.0f, 0.0f, 0.0f };
const Maths::FVector3 kLightVolumeColor = { 1.0f, 1.0f, 0.0f };
const Maths::FVector3 kColliderColor = { 0.0f, 1.0f, 0.0f };
const Maths::FVector3 kFrustumColor = { 1.0f, 1.0f, 1.0f };

const Maths::FVector4 kHoveredOutlineColor{ 1.0f, 1.0f, 0.0f, 1.0f };
const Maths::FVector4 kSelectedOutlineColor{ 1.0f, 0.7f, 0.0f, 1.0f };

constexpr float kHoveredOutlineWidth = 2.5f;
constexpr float kSelectedOutlineWidth = 5.0f;

Maths::FMatrix4 CalculateCameraModelMatrix(Core::ECS::Actor& p_actor)
{
	auto translation = FMatrix4::Translation(p_actor.transform.GetWorldPosition());
	auto rotation = FQuaternion::ToMatrix4(p_actor.transform.GetWorldRotation());
	return translation * rotation;
}

std::optional<std::string> GetLightTypeTextureName(Rendering::Settings::ELightType type)
{
	using namespace Rendering::Settings;

	switch (type)
	{
	case ELightType::POINT: return "Point_Light";
	case ELightType::SPOT: return "Spot_Light";
	case ELightType::DIRECTIONAL: return "Directional_Light";
	case ELightType::AMBIENT_BOX: return "Ambient_Box_Light";
	case ELightType::AMBIENT_SPHERE: return "Ambient_Sphere_Light";
	}

	return std::nullopt;
}

class DebugCamerasRenderPass : public Rendering::Core::ARenderPass
{
public:
	DebugCamerasRenderPass(Rendering::Core::CompositeRenderer& p_renderer) : Rendering::Core::ARenderPass(p_renderer)
	{
		m_cameraMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().shaderManager[":Shaders\\Lambert.ovfx"]);
		m_cameraMaterial.SetProperty("u_Diffuse", FVector4{ 0.0f, 0.3f, 0.7f, 1.0f });
		m_cameraMaterial.SetProperty("u_DiffuseMap", static_cast<Rendering::Resources::Texture*>(nullptr));
	}

protected:
	virtual void Draw(Rendering::Data::PipelineState p_pso) override
	{
		TracyGpuZone("DebugCamerasRenderPass");

		auto& sceneDescriptor = m_renderer.GetDescriptor<Core::Rendering::SceneRenderer::SceneDescriptor>();

		for (auto camera : sceneDescriptor.scene.GetFastAccessComponents().cameras)
		{
			auto& actor = camera->owner;

			if (actor.IsActive())
			{
				auto& model = *::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetModel("Camera");
				auto modelMatrix = CalculateCameraModelMatrix(actor);

				m_renderer.GetFeature<Editor::Rendering::DebugModelRenderFeature>()
					.DrawModelWithSingleMaterial(p_pso, model, m_cameraMaterial, modelMatrix);
			}
		}
	}

private:
	Core::Resources::Material m_cameraMaterial;
};

class DebugLightsRenderPass : public Rendering::Core::ARenderPass
{
public:
	DebugLightsRenderPass(Rendering::Core::CompositeRenderer& p_renderer) : Rendering::Core::ARenderPass(p_renderer)
	{
		m_lightMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetShader("Billboard"));
		m_lightMaterial.SetProperty("u_Diffuse", FVector4{ 1.f, 1.f, 0.5f, 0.5f });
		m_lightMaterial.SetBackfaceCulling(false);
		m_lightMaterial.SetBlendable(true);
		m_lightMaterial.SetDepthTest(false);
	}

protected:
	virtual void Draw(Rendering::Data::PipelineState p_pso) override
	{
		TracyGpuZone("DebugLightsRenderPass");

		auto& sceneDescriptor = m_renderer.GetDescriptor<Core::Rendering::SceneRenderer::SceneDescriptor>();

		m_lightMaterial.SetProperty("u_Scale", 1.0f * 0.1f);

		for (auto light : sceneDescriptor.scene.GetFastAccessComponents().lights)
		{
			auto& actor = light->owner;

			if (actor.IsActive())
			{
				auto& model = *::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetModel("Vertical_Plane");
				auto modelMatrix = Maths::FMatrix4::Translation(actor.transform.GetWorldPosition());

				auto lightTypeTextureName = GetLightTypeTextureName(light->GetData().type);

				::Rendering::Resources::Texture* lightTexture =
					lightTypeTextureName ?
					::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetTexture(lightTypeTextureName.value()) :
					nullptr;

				const auto& lightColor = light->GetColor();
				m_lightMaterial.SetProperty("u_DiffuseMap", lightTexture);
				m_lightMaterial.SetProperty("u_Diffuse", Maths::FVector4(lightColor.x, lightColor.y, lightColor.z, 0.75f));

				m_renderer.GetFeature<Editor::Rendering::DebugModelRenderFeature>()
					.DrawModelWithSingleMaterial(p_pso, model, m_lightMaterial, modelMatrix);
			}
		}
	}

private:
	Core::Resources::Material m_lightMaterial;
};

class DebugActorRenderPass : public Rendering::Core::ARenderPass
{
public:
	DebugActorRenderPass(Rendering::Core::CompositeRenderer& p_renderer) : Rendering::Core::ARenderPass(p_renderer),
		m_debugShapeFeature(m_renderer.GetFeature<Rendering::Features::DebugShapeRenderFeature>())
	{

	}

protected:
	Rendering::Features::DebugShapeRenderFeature& m_debugShapeFeature;

	virtual void Draw(Rendering::Data::PipelineState p_pso) override
	{
		TracyGpuZone("DebugActorRenderPass");

		auto& debugSceneDescriptor = m_renderer.GetDescriptor<Editor::Rendering::DebugSceneRenderer::DebugSceneDescriptor>();

		if (debugSceneDescriptor.selectedActor)
		{
			auto& selectedActor = debugSceneDescriptor.selectedActor.value();
			const bool isActorHovered = debugSceneDescriptor.highlightedActor && debugSceneDescriptor.highlightedActor->GetID() == selectedActor.GetID();

			DrawActorDebugElements(selectedActor);
			m_renderer.GetFeature<Editor::Rendering::OutlineRenderFeature>().DrawOutline(
				selectedActor,
				isActorHovered ?
				kHoveredOutlineColor :
				kSelectedOutlineColor,
				kSelectedOutlineWidth
			);
			m_renderer.Clear(false, true, false, Maths::FVector3::Zero);
			m_renderer.GetFeature<Editor::Rendering::GizmoRenderFeature>().DrawGizmo(
				selectedActor.transform.GetWorldPosition(),
				selectedActor.transform.GetWorldRotation(),
				debugSceneDescriptor.gizmoOperation,
				false,
				debugSceneDescriptor.highlightedGizmoDirection
			);
		}

		if (debugSceneDescriptor.highlightedActor)
		{
			auto& highlightedActor = debugSceneDescriptor.highlightedActor.value();

			// Render the outline only if the actor is not already selected (as its outline render should have been handled already).
			if (!debugSceneDescriptor.selectedActor || highlightedActor.GetID() != debugSceneDescriptor.selectedActor->GetID())
			{
				m_renderer.GetFeature<Editor::Rendering::OutlineRenderFeature>().DrawOutline(highlightedActor, kHoveredOutlineColor, kHoveredOutlineWidth);
			}
		}
	}

	void DrawActorDebugElements(Core::ECS::Actor& p_actor)
	{
		if (p_actor.IsActive())
		{
			/* Render static mesh outline and bounding spheres */
			if (true)
			{
				auto modelRenderer = p_actor.GetComponent<Core::ECS::Components::CModelRenderer>();

				if (modelRenderer && modelRenderer->GetModel())
				{
					DrawBoundingSpheres(*modelRenderer);
				}
			}

			/* Render camera component outline */
			if (auto cameraComponent = p_actor.GetComponent<Core::ECS::Components::CCamera>(); cameraComponent)
			{
				auto model = CalculateCameraModelMatrix(p_actor);
				DrawCameraFrustum(*cameraComponent);
			}

			/* Render the actor collider */
			if (p_actor.GetComponent<Core::ECS::Components::CPhysicalObject>())
			{
				DrawActorCollider(p_actor);
			}

			/* Render the actor ambient light */
			if (auto ambientBoxComp = p_actor.GetComponent<Core::ECS::Components::CAmbientBoxLight>())
			{
				DrawAmbientBoxVolume(*ambientBoxComp);
			}

			if (auto ambientSphereComp = p_actor.GetComponent<Core::ECS::Components::CAmbientSphereLight>())
			{
				DrawAmbientSphereVolume(*ambientSphereComp);
			}

			if (true)
			{
				if (auto light = p_actor.GetComponent<Core::ECS::Components::CLight>())
				{
					DrawLightBounds(*light);
				}
			}

			for (auto& child : p_actor.GetChildren())
			{
				DrawActorDebugElements(*child);
			}
		}
	}

	void DrawFrustumLines(
		const Maths::FVector3& pos,
		const Maths::FVector3& forward,
		float near,
		const float far,
		const Maths::FVector3& a,
		const Maths::FVector3& b,
		const Maths::FVector3& c,
		const Maths::FVector3& d,
		const Maths::FVector3& e,
		const Maths::FVector3& f,
		const Maths::FVector3& g,
		const Maths::FVector3& h
	)
	{
		auto pso = m_renderer.CreatePipelineState();

		// Convenient lambda to draw a frustum line
		auto draw = [&](const FVector3& p_start, const FVector3& p_end, const float planeDistance)
			{
				auto offset = pos + forward * planeDistance;
				auto start = offset + p_start;
				auto end = offset + p_end;
				m_debugShapeFeature.DrawLine(pso, start, end, kFrustumColor);
			};

		// Draw near plane
		draw(a, b, near);
		draw(b, d, near);
		draw(d, c, near);
		draw(c, a, near);

		// Draw far plane
		draw(e, f, far);
		draw(f, h, far);
		draw(h, g, far);
		draw(g, e, far);

		// Draw lines between near and far planes
		draw(a + forward * near, e + forward * far, 0);
		draw(b + forward * near, f + forward * far, 0);
		draw(c + forward * near, g + forward * far, 0);
		draw(d + forward * near, h + forward * far, 0);
	}

	void DrawCameraPerspectiveFrustum(std::pair<uint16_t, uint16_t>& p_size, Core::ECS::Components::CCamera& p_camera)
	{
		const auto& owner = p_camera.owner;
		auto& camera = p_camera.GetCamera();

		const auto& cameraPos = owner.transform.GetWorldPosition();
		const auto& cameraRotation = owner.transform.GetWorldRotation();
		const auto& cameraForward = owner.transform.GetWorldForward();

		camera.CacheMatrices(p_size.first, p_size.second); // TODO: We shouldn't cache matrices mid air, we could use another function to get the matrices/calculate
		const auto proj = FMatrix4::Transpose(camera.GetProjectionMatrix());
		const auto near = camera.GetNear();
		const auto far = camera.GetFar();

		const auto nLeft = near * (proj.data[2] - 1.0f) / proj.data[0];
		const auto nRight = near * (1.0f + proj.data[2]) / proj.data[0];
		const auto nTop = near * (1.0f + proj.data[6]) / proj.data[5];
		const auto nBottom = near * (proj.data[6] - 1.0f) / proj.data[5];

		const auto fLeft = far * (proj.data[2] - 1.0f) / proj.data[0];
		const auto fRight = far * (1.0f + proj.data[2]) / proj.data[0];
		const auto fTop = far * (1.0f + proj.data[6]) / proj.data[5];
		const auto fBottom = far * (proj.data[6] - 1.0f) / proj.data[5];

		auto a = cameraRotation * FVector3{ nLeft, nTop, 0 };
		auto b = cameraRotation * FVector3{ nRight, nTop, 0 };
		auto c = cameraRotation * FVector3{ nLeft, nBottom, 0 };
		auto d = cameraRotation * FVector3{ nRight, nBottom, 0 };
		auto e = cameraRotation * FVector3{ fLeft, fTop, 0 };
		auto f = cameraRotation * FVector3{ fRight, fTop, 0 };
		auto g = cameraRotation * FVector3{ fLeft, fBottom, 0 };
		auto h = cameraRotation * FVector3{ fRight, fBottom, 0 };

		DrawFrustumLines(cameraPos, cameraForward, near, far, a, b, c, d, e, f, g, h);
	}

	void DrawCameraOrthographicFrustum(std::pair<uint16_t, uint16_t>& p_size, Core::ECS::Components::CCamera& p_camera)
	{
		auto& owner = p_camera.owner;
		auto& camera = p_camera.GetCamera();
		const auto ratio = p_size.first / static_cast<float>(p_size.second);

		const auto& cameraPos = owner.transform.GetWorldPosition();
		const auto& cameraRotation = owner.transform.GetWorldRotation();
		const auto& cameraForward = owner.transform.GetWorldForward();

		const auto near = camera.GetNear();
		const auto far = camera.GetFar();
		const auto size = p_camera.GetSize();

		const auto right = ratio * size;
		const auto left = -right;
		const auto top = size;
		const auto bottom = -top;

		const auto a = cameraRotation * FVector3{ left, top, 0 };
		const auto b = cameraRotation * FVector3{ right, top, 0 };
		const auto c = cameraRotation * FVector3{ left, bottom, 0 };
		const auto d = cameraRotation * FVector3{ right, bottom, 0 };

		DrawFrustumLines(cameraPos, cameraForward, near, far, a, b, c, d, a, b, c, d);
	}

	void DrawCameraFrustum(Core::ECS::Components::CCamera& p_camera)
	{

		std::pair<uint16_t, uint16_t>gameViewSize = { 16, 9 };
		switch (p_camera.GetProjectionMode())
		{
		case Rendering::Settings::EProjectionMode::ORTHOGRAPHIC:
			DrawCameraOrthographicFrustum(gameViewSize, p_camera);
			break;

		case Rendering::Settings::EProjectionMode::PERSPECTIVE:
			DrawCameraPerspectiveFrustum(gameViewSize, p_camera);
			break;
		}
	}

	void DrawActorCollider(Core::ECS::Actor& p_actor)
	{
		using namespace Core::ECS::Components;
		using namespace Physics::Entities;

		auto pso = m_renderer.CreatePipelineState();
		pso.depthTest = false;

		/* Draw the box collider if any */
		if (auto boxColliderComponent = p_actor.GetComponent<Core::ECS::Components::CPhysicalBox>(); boxColliderComponent)
		{
			m_debugShapeFeature.DrawBox(
				pso,
				p_actor.transform.GetWorldPosition(),
				p_actor.transform.GetWorldRotation(),
				boxColliderComponent->GetSize() * p_actor.transform.GetWorldScale(),
				Maths::FVector3{ 0.f, 1.f, 0.f },
				1.0f
			);
		}

		/* Draw the sphere collider if any */
		if (auto sphereColliderComponent = p_actor.GetComponent<Core::ECS::Components::CPhysicalSphere>(); sphereColliderComponent)
		{
			FVector3 actorScale = p_actor.transform.GetWorldScale();
			float radius = sphereColliderComponent->GetRadius() * std::max(std::max(std::max(actorScale.x, actorScale.y), actorScale.z), 0.0f);

			m_debugShapeFeature.DrawSphere(
				pso,
				p_actor.transform.GetWorldPosition(),
				p_actor.transform.GetWorldRotation(),
				radius,
				Maths::FVector3{ 0.f, 1.f, 0.f },
				1.0f
			);
		}

		/* Draw the capsule collider if any */
		if (auto capsuleColliderComponent = p_actor.GetComponent<Core::ECS::Components::CPhysicalCapsule>(); capsuleColliderComponent)
		{
			FVector3 actorScale = p_actor.transform.GetWorldScale();
			float radius = abs(capsuleColliderComponent->GetRadius() * std::max(std::max(actorScale.x, actorScale.z), 0.f));
			float height = abs(capsuleColliderComponent->GetHeight() * actorScale.y);

			m_debugShapeFeature.DrawCapsule(
				pso,
				p_actor.transform.GetWorldPosition(),
				p_actor.transform.GetWorldRotation(),
				radius,
				height,
				Maths::FVector3{ 0.f, 1.f, 0.f },
				1.0f
			);
		}
	}

	void DrawLightBounds(Core::ECS::Components::CLight& p_light)
	{
		auto pso = m_renderer.CreatePipelineState();
		pso.depthTest = false;

		auto& data = p_light.GetData();

		m_debugShapeFeature.DrawSphere(
			pso,
			data.transform->GetWorldPosition(),
			data.transform->GetWorldRotation(),
			data.GetEffectRange(),
			kDebugBoundsColor,
			1.0f
		);
	}

	void DrawAmbientBoxVolume(Core::ECS::Components::CAmbientBoxLight& p_ambientBoxLight)
	{
		auto pso = m_renderer.CreatePipelineState();
		pso.depthTest = false;

		auto& data = p_ambientBoxLight.GetData();

		m_debugShapeFeature.DrawBox(
			pso,
			p_ambientBoxLight.owner.transform.GetWorldPosition(),
			data.transform->GetWorldRotation(),
			{ data.constant, data.linear, data.quadratic },
			data.GetEffectRange(),
			1.0f
		);
	}

	void DrawAmbientSphereVolume(Core::ECS::Components::CAmbientSphereLight& p_ambientSphereLight)
	{
		auto pso = m_renderer.CreatePipelineState();
		pso.depthTest = false;

		auto& data = p_ambientSphereLight.GetData();

		m_debugShapeFeature.DrawSphere(
			pso,
			p_ambientSphereLight.owner.transform.GetWorldPosition(),
			p_ambientSphereLight.owner.transform.GetWorldRotation(),
			data.constant,
			kLightVolumeColor,
			1.0f
		);
	}

	void DrawBoundingSpheres(Core::ECS::Components::CModelRenderer& p_modelRenderer)
	{
		using namespace Core::ECS::Components;
		using namespace Physics::Entities;

		auto pso = m_renderer.CreatePipelineState();
		pso.depthTest = false;

		/* Draw the sphere collider if any */
		if (auto model = p_modelRenderer.GetModel())
		{
			auto& actor = p_modelRenderer.owner;

			Maths::FVector3 actorScale = actor.transform.GetWorldScale();
			Maths::FQuaternion actorRotation = actor.transform.GetWorldRotation();
			Maths::FVector3 actorPosition = actor.transform.GetWorldPosition();

			const auto& modelBoundingsphere =
				p_modelRenderer.GetFrustumBehaviour() == Core::ECS::Components::CModelRenderer::EFrustumBehaviour::CULL_CUSTOM ?
				p_modelRenderer.GetCustomBoundingSphere() :
				model->GetBoundingSphere();

			float radiusScale = std::max(std::max(std::max(actorScale.x, actorScale.y), actorScale.z), 0.0f);
			float scaledRadius = modelBoundingsphere.radius * radiusScale;
			auto sphereOffset = Maths::FQuaternion::RotatePoint(modelBoundingsphere.position, actorRotation) * radiusScale;

			m_debugShapeFeature.DrawSphere(
				pso,
				actorPosition + sphereOffset,
				actorRotation,
				scaledRadius,
				kDebugBoundsColor,
				1.0f
			);

			if (p_modelRenderer.GetFrustumBehaviour() == Core::ECS::Components::CModelRenderer::EFrustumBehaviour::CULL_MESHES)
			{
				const auto& meshes = model->GetMeshes();

				if (meshes.size() > 1) // One mesh would result into the same bounding sphere for mesh and model
				{
					for (auto mesh : meshes)
					{
						auto& meshBoundingSphere = mesh->GetBoundingSphere();
						float scaledRadius = meshBoundingSphere.radius * radiusScale;
						auto sphereOffset = Maths::FQuaternion::RotatePoint(meshBoundingSphere.position, actorRotation) * radiusScale;

						m_debugShapeFeature.DrawSphere(
							pso,
							actorPosition + sphereOffset,
							actorRotation,
							scaledRadius,
							kDebugBoundsColor,
							1.0f
						);
					}
				}
			}
		}
	}
};

Editor::Rendering::DebugSceneRenderer::DebugSceneRenderer(::Rendering::Context::Driver& p_driver) :
	::Core::Rendering::SceneRenderer(p_driver, true /* enable stencil write, required by the grid */)
{
	AddFeature<::Rendering::Features::FrameInfoRenderFeature>();
	AddFeature<::Rendering::Features::DebugShapeRenderFeature>();
	AddFeature<::Editor::Rendering::DebugModelRenderFeature>();
	AddFeature<::Editor::Rendering::OutlineRenderFeature>();
	AddFeature<::Editor::Rendering::GizmoRenderFeature>();

	AddPass<GridRenderPass>("Grid", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<DebugCamerasRenderPass>("Debug Cameras", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<DebugLightsRenderPass>("Debug Lights", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<DebugActorRenderPass>("Debug Actor", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<PickingRenderPass>("Picking", ::Rendering::Settings::ERenderPassOrder::Debug);
}
