#include <Core/ECS/Components/CCamera.h>
#include <Core/ECS/Components/CDirectionalLight.h>
#include <Core/ECS/Components/CMaterialRenderer.h>

#include <Core/ECS/Components/CPointLight.h>
#include <Core/ECS/Components/CSpotLight.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Core/Rendering/ReflectionRenderFeature.h>

#include "EditorResources.h"
#include "AView.h"

#include "DebugModelRenderFeature.h"
#include "DebugSceneRenderer.h"
#include "PointRenderPass.h"
#include "GizmoRenderPass.h"
#include "GizmoRenderFeature.h"
#include "GridRenderPass.h"
#include "OutlineRenderFeature.h"
#include "PickingRenderPass.h"

#include "Core/Global/ServiceLocator.h"
#include "renderer/SceneView.h"
#include <Rendering/Features/DebugShapeRenderFeature.h>
#include <Rendering/Features/FrameInfoRenderFeature.h>
#include <Rendering/Features/LightingRenderFeature.h>
#include <Rendering/HAL/Profiling.h>

using namespace Maths;
using namespace Rendering::Resources;
using namespace Core::Resources;

namespace
{
	const Maths::FVector3 kDebugBoundsColor = { 1.0f, 0.0f, 0.0f };
	const Maths::FVector3 kLightVolumeColor = { 1.0f, 1.0f, 0.0f };
	const Maths::FVector3 kColliderColor = { 0.0f, 1.0f, 0.0f };
	const Maths::FVector3 kFrustumColor = { 1.0f, 1.0f, 1.0f };

	const Maths::FVector4 kHeredOutlineColor{ 1.0f, 1.0f, 0.0f, 1.0f };
	const Maths::FVector4 kSelectedOutlineColor{ 1.0f, 0.7f, 0.0f, 1.0f };

	constexpr float kHeredOutlineWidth = 2.5f;
	constexpr float kSelectedOutlineWidth = 5.0f;

	Maths::FMatrix4 CalculateUnscaledModelMatrix(Core::ECS::Actor& p_actor)
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

	Maths::FMatrix4 CreateDebugDirectionalLight()
	{
		Rendering::Entities::Light directionalLight;
		directionalLight.intensity = 2.0f;
		directionalLight.type = Rendering::Settings::ELightType::DIRECTIONAL;

		directionalLight.transform->SetLocalPosition({ 0.0f, 10.0f, 0.0f });
		directionalLight.transform->SetLocalRotation(Maths::FQuaternion({ 120.0f, -40.0f, 0.0f }));
		return directionalLight.GenerateMatrix();
	}

	Maths::FMatrix4 CreateDebugAmbientLight()
	{
		Rendering::Entities::Light light;
		light.intensity = 0.01f;
		light.constant = 10000.0f;
		light.type = Rendering::Settings::ELightType::AMBIENT_SPHERE;
		return light.GenerateMatrix();
	}

	std::unique_ptr<Rendering::HAL::ShaderStorageBuffer> CreateDebugLightBuffer()
	{
		auto lightBuffer = std::make_unique<Rendering::HAL::ShaderStorageBuffer>();
		Maths::FMatrix4 lightMatrices[2] = {
			CreateDebugDirectionalLight(),
			CreateDebugAmbientLight() };

		lightBuffer->Allocate(sizeof(Maths::FMatrix4) * 2, Rendering::Settings::EAccessSpecifier::STATIC_READ);
		lightBuffer->Upload(&lightMatrices[0]);

		return lightBuffer;
	}
}

class DebugCamerasRenderPass : public Rendering::Core::ARenderPass
{
public:
	DebugCamerasRenderPass(Rendering::Core::CompositeRenderer& p_renderer) : Rendering::Core::ARenderPass(p_renderer)
	{
		m_fakeLightsBuffer = CreateDebugLightBuffer();

		m_cameraMaterial.SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\Standard.ovfx"]);
		m_cameraMaterial.SetProperty("u_Albedo", FVector4{ 0.0f, 0.447f, 1.0f, 1.0f });
		m_cameraMaterial.SetProperty("u_Metallic", 0.0f);
		m_cameraMaterial.SetProperty("u_Roughness", 0.25f);
		m_cameraMaterial.SetProperty("u_BuiltInGammaCorrection", true);
		m_cameraMaterial.SetProperty("u_BuiltInToneMapping", true);
	}

protected:
	virtual void Draw(Rendering::Data::PipelineState p_pso) override
	{
		ZoneScoped;
		TracyGpuZone("DebugCamerasRenderPass");

		using namespace Rendering::Features;

		const auto lightingRenderFeature = Tools::Utils::OptRef<LightingRenderFeature>{
			m_renderer.HasFeature<LightingRenderFeature>() ?
			m_renderer.GetFeature<LightingRenderFeature>() :
			Tools::Utils::OptRef<LightingRenderFeature>{std::nullopt}
		};

		// erride the light buffer with fake lights
		m_fakeLightsBuffer->Bind(
			lightingRenderFeature ?
			lightingRenderFeature->GetBufferBindingPoint() :
			0
		);

		auto& sceneDescriptor = m_renderer.GetDescriptor<Core::Rendering::SceneRenderer::SceneDescriptor>();

		for (auto camera : sceneDescriptor.scene.GetFastAccessComponents().cameras)
		{
			auto& actor = camera->owner;

			if (actor.IsActive())
			{
				auto& model = *::Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetModel("Camera");
				auto modelMatrix = CalculateUnscaledModelMatrix(actor);

				m_renderer.GetFeature<Editor::Rendering::DebugModelRenderFeature>()
					.DrawModelWithSingleMaterial(p_pso, model, m_cameraMaterial, modelMatrix);
			}
		}

		if (lightingRenderFeature)
		{
			// Bind back the original light buffer
			lightingRenderFeature->Bind();
		}
	}

private:
	::Core::Resources::Material m_cameraMaterial;
	std::unique_ptr<::Rendering::HAL::ShaderStorageBuffer> m_fakeLightsBuffer;
};

class DebugReflectionProbesRenderPass : public Rendering::Core::ARenderPass
{
public:
	DebugReflectionProbesRenderPass(Rendering::Core::CompositeRenderer& p_renderer) : Rendering::Core::ARenderPass(p_renderer)
	{
		m_fakeLightsBuffer = CreateDebugLightBuffer();

		m_reflectiveMaterial.SetDepthTest(false);
		m_reflectiveMaterial.SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\Standard.ovfx"]);
		m_reflectiveMaterial.SetProperty("u_Albedo", FVector4{ 1.0, 1.0f, 1.0f, 1.0f });
		m_reflectiveMaterial.SetProperty("u_Metallic", 1.0f);
		m_reflectiveMaterial.SetProperty("u_Roughness", 0.0f);
		m_reflectiveMaterial.SetProperty("u_BuiltInGammaCorrection", true);
		m_reflectiveMaterial.SetProperty("u_BuiltInToneMapping", true);
	}

protected:
	virtual void Draw(Rendering::Data::PipelineState p_pso) override
	{
		ZoneScoped;
		TracyGpuZone("DebugReflectionProbesRenderPass");

		using namespace Rendering::Features;

		const auto lightingRenderFeature = Tools::Utils::OptRef<LightingRenderFeature>{
			m_renderer.HasFeature<LightingRenderFeature>() ?
			m_renderer.GetFeature<LightingRenderFeature>() :
			Tools::Utils::OptRef<LightingRenderFeature>{std::nullopt}
		};

		// erride the light buffer with fake lights
		m_fakeLightsBuffer->Bind(
			lightingRenderFeature ?
			lightingRenderFeature->GetBufferBindingPoint() :
			0
		);

		auto& sceneDescriptor = m_renderer.GetDescriptor<Core::Rendering::SceneRenderer::SceneDescriptor>();
		auto& reflectionRenderFeature = m_renderer.GetFeature<Core::Rendering::ReflectionRenderFeature>();

		for (auto reflectionProbe : sceneDescriptor.scene.GetFastAccessComponents().reflectionProbes)
		{
			auto& actor = reflectionProbe->owner;

			if (actor.IsActive())
			{
				auto& model = *::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().editorResources->GetModel("Sphere");
				auto modelMatrix =
					Maths::FMatrix4::Scale(
						Maths::FMatrix4::Translate(
							CalculateUnscaledModelMatrix(actor),
							reflectionProbe->GetCapturePosition()
						),
						{ 0.5f, 0.5f, 0.5f }
				);

				reflectionRenderFeature.PrepareProbe(*reflectionProbe);
				reflectionRenderFeature.SendProbeData(m_reflectiveMaterial, *reflectionProbe);
				reflectionRenderFeature.BindProbe(*reflectionProbe);

				m_renderer.GetFeature<Editor::Rendering::DebugModelRenderFeature>()
					.DrawModelWithSingleMaterial(p_pso, model, m_reflectiveMaterial, modelMatrix);
			}
		}

		if (lightingRenderFeature)
		{
			// Bind back the original light buffer
			lightingRenderFeature->Bind();
		}
	}

private:
	Core::Resources::Material m_reflectiveMaterial;
	std::unique_ptr<Rendering::HAL::ShaderStorageBuffer> m_fakeLightsBuffer;
};

class DebugLightsRenderPass : public Rendering::Core::ARenderPass
{
public:
	DebugLightsRenderPass(Rendering::Core::CompositeRenderer& p_renderer) : Rendering::Core::ARenderPass(p_renderer)
	{
		m_lightMaterial.SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetShader("Billboard"));
		m_lightMaterial.SetProperty("u_Diffuse", FVector4{ 1.f, 1.f, 0.5f, 0.5f });
		m_lightMaterial.SetBackfaceCulling(false);
		m_lightMaterial.SetBlendable(true);
		m_lightMaterial.SetDepthTest(false);
	}

protected:
	virtual void Draw(Rendering::Data::PipelineState p_pso) override
	{
		ZoneScoped;
		TracyGpuZone("DebugLightsRenderPass");

		auto& sceneDescriptor = m_renderer.GetDescriptor<Core::Rendering::SceneRenderer::SceneDescriptor>();

		m_lightMaterial.SetProperty("u_Scale", 0.035f);

		for (auto light : sceneDescriptor.scene.GetFastAccessComponents().lights)
		{
			auto& actor = light->owner;

			if (actor.IsActive())
			{
				auto& model = *Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetModel("Vertical_Plane");
				auto modelMatrix = Maths::FMatrix4::Translation(actor.transform.GetWorldPosition());

				auto lightTypeTextureName = GetLightTypeTextureName(light->GetData().type);

				auto lightTexture =
					lightTypeTextureName ?
					Core::Global::ServiceLocator::Get<Editor::Core::Context>().editorResources->GetTexture(lightTypeTextureName.value()) :
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
		ZoneScoped;
		TracyGpuZone("DebugActorRenderPass");
		
		// Clear stencil buffer for outline rendering
		m_renderer.Clear(false, false, true);

		auto& debugSceneDescriptor = m_renderer.GetDescriptor<Editor::Rendering::DebugSceneRenderer::DebugSceneDescriptor>();

		if (debugSceneDescriptor.selectedActor)
		{

			auto& selectedActor = debugSceneDescriptor.selectedActor.value();
			const bool isActorHered = debugSceneDescriptor.highlightedActor && debugSceneDescriptor.highlightedActor->GetID() == selectedActor.GetID();

			DrawActorDebugElements(selectedActor);
			m_renderer.GetFeature<Editor::Rendering::OutlineRenderFeature>().DrawOutline(
				selectedActor,
				isActorHered ?
				kHeredOutlineColor :
				kSelectedOutlineColor,
				kSelectedOutlineWidth
			);
			//m_renderer.Clear(false, true, false, Maths::FVector3::Zero);
			//m_renderer.GetFeature<Editor::Rendering::GizmoRenderFeature>().DrawGizmo(
			//	selectedActor.transform.GetWorldPosition(),
			//	selectedActor.transform.GetWorldRotation(),
			//	debugSceneDescriptor.gizmoOperation,
			//	false,
			//	debugSceneDescriptor.highlightedGizmoDirection
			//);
		}

		if (debugSceneDescriptor.highlightedActor)
		{
			auto& highlightedActor = debugSceneDescriptor.highlightedActor.value();

			// Render the outline only if the actor is not already selected (as its outline render should have been handled already).
			if (!debugSceneDescriptor.selectedActor || highlightedActor.GetID() != debugSceneDescriptor.selectedActor->GetID())
			{
				m_renderer.GetFeature<Editor::Rendering::OutlineRenderFeature>().DrawOutline(highlightedActor, kHeredOutlineColor, kHeredOutlineWidth);
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

			/* Render camera component frustum */
			if (auto cameraComponent = p_actor.GetComponent<Core::ECS::Components::CCamera>(); cameraComponent)
			{
				DrawCameraFrustum(*cameraComponent);
			}

			/* Render camera component frustum */
			if (auto reflectionProbeComponent = p_actor.GetComponent<Core::ECS::Components::CReflectionProbe>(); reflectionProbeComponent)
			{
				if (reflectionProbeComponent->GetInfluencePolicy() == Core::ECS::Components::CReflectionProbe::EInfluencePolicy::LOCAL)
				{
					DrawReflectionProbeInfluenceVolume(*reflectionProbeComponent);
				}
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
		auto draw = [&](const FVector3& p_start, const FVector3& p_end, const float planeDistance) {
			auto offset = pos + forward * planeDistance;
			auto start = offset + p_start;
			auto end = offset + p_end;
			m_debugShapeFeature.DrawLine(pso, start, end, kFrustumColor, 1.0f, false);
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

		if (gameViewSize.first == 0 || gameViewSize.second == 0)
		{
			gameViewSize = { 16, 9 };
		}

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

	void DrawReflectionProbeInfluenceVolume(Core::ECS::Components::CReflectionProbe& p_reflectionProbe)
	{
		auto pso = m_renderer.CreatePipelineState();
		pso.depthTest = false;
		const auto& size = p_reflectionProbe.GetInfluenceSize();
		const auto position = p_reflectionProbe.owner.transform.GetWorldPosition();
		m_debugShapeFeature.DrawBox(
			pso,
			position,
			p_reflectionProbe.owner.transform.GetWorldRotation(),
			size,
			kDebugBoundsColor,
			1.0f,
			false
		);
	}

	void DrawActorCollider(Core::ECS::Actor& p_actor)
	{

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
			data.CalculateEffectRange(),
			kDebugBoundsColor,
			1.0f,
			false
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
			data.CalculateEffectRange(),
			1.0f,
			false
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
			1.0f,
			false
		);
	}

	void DrawBoundingSpheres(Core::ECS::Components::CModelRenderer& p_modelRenderer)
	{
		using namespace Core::ECS::Components;
		auto pso = m_renderer.CreatePipelineState();
		const auto frustumBehaviour = p_modelRenderer.GetFrustumBehaviour();
		// Draw the mesh, model, or custom bounding sphere
		if (auto model = p_modelRenderer.GetModel())
		{
			auto& actor = p_modelRenderer.owner;

			const auto& actorScale = actor.transform.GetWorldScale();
			const auto& actorRotation = actor.transform.GetWorldRotation();
			const auto& actorPosition = actor.transform.GetWorldPosition();

			const float radiusScale = std::max(std::max(std::max(actorScale.x, actorScale.y), actorScale.z), 0.0f);

			auto drawBounds = [&](const Rendering::Geometry::BoundingSphere& p_bounds) {
				const float scaledRadius = p_bounds.radius * radiusScale;
				const auto sphereOffset = Maths::FQuaternion::RotatePoint(
					p_bounds.position,
					actorRotation
				) * radiusScale;

				m_debugShapeFeature.DrawSphere(
					pso,
					actorPosition + sphereOffset,
					actorRotation,
					scaledRadius,
					kDebugBoundsColor,
					1.0f,
					false
				);
				};

			if (frustumBehaviour == Core::ECS::Components::CModelRenderer::EFrustumBehaviour::MESH_BOUNDS)
			{
				for (auto mesh : model->GetMeshes())
				{
					drawBounds(mesh->GetBoundingSphere());
				}
			}
			else
			{
				drawBounds(
					frustumBehaviour == Core::ECS::Components::CModelRenderer::EFrustumBehaviour::CUSTOM_BOUNDS ?
					p_modelRenderer.GetCustomBoundingSphere() :
					model->GetBoundingSphere()
				);
			}
		}
	}
};

Editor::Rendering::DebugSceneRenderer::DebugSceneRenderer(::Rendering::Context::Driver& p_driver) :
	::Core::Rendering::SceneRenderer(p_driver, true /* enable stencil write, required by the grid */)
{
	AddFeature<::Rendering::Features::FrameInfoRenderFeature, ::Rendering::Features::EFeatureExecutionPolicy::ALWAYS>();
	AddFeature<::Rendering::Features::DebugShapeRenderFeature, ::Rendering::Features::EFeatureExecutionPolicy::FRAME_EVENTS_ONLY>();
	AddFeature<Editor::Rendering::DebugModelRenderFeature, ::Rendering::Features::EFeatureExecutionPolicy::NEVER>();
	AddFeature<OutlineRenderFeature, ::Rendering::Features::EFeatureExecutionPolicy::NEVER>();
	AddFeature<GizmoRenderFeature, ::Rendering::Features::EFeatureExecutionPolicy::NEVER>();

	AddPass<GridRenderPass>("Grid", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<DebugCamerasRenderPass>("Debug Cameras", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<DebugReflectionProbesRenderPass>("Debug Reflection Probes", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<DebugLightsRenderPass>("Debug Lights", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<DebugActorRenderPass>("Debug Actor", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<PickingRenderPass>("Picking", ::Rendering::Settings::ERenderPassOrder::Debug);
	AddPass<PointRenderPass>("PointDraw", ::Rendering::Settings::ERenderPassOrder::Opaque).SetEnabled(false);
	AddPass<GizmoRenderPass>("Gizmo", ::Rendering::Settings::ERenderPassOrder::Last);
}
