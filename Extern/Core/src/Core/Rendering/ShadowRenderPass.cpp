/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Core/Rendering/ShadowRenderPass.h>
#include <Core/ResourceManagement/ShaderManager.h>

#include <Rendering/Features/LightingRenderFeature.h>
#include <Rendering/HAL/Profiling.h>

constexpr uint8_t kMaxShadowMaps = 1;

Core::Rendering::ShadowRenderPass::ShadowRenderPass(::Rendering::Core::CompositeRenderer& p_renderer) :
	::Rendering::Core::ARenderPass(p_renderer)
{
	const auto shadowShader = OVSERVICE(Core::ResourceManagement::ShaderManager).GetResource(":Shaders\\Shadow.ovfx");
	ASSERT(shadowShader, "Cannot find the shadow shader");

	m_shadowMaterial.SetShader(shadowShader);
	m_shadowMaterial.SetFrontfaceCulling(false);
	m_shadowMaterial.SetBackfaceCulling(false);
}

void Core::Rendering::ShadowRenderPass::Draw(::Rendering::Data::PipelineState p_pso)
{
	TracyGpuZone("ShadowRenderPass");

	using namespace Core::Rendering;

	ASSERT(m_renderer.HasDescriptor<SceneRenderer::SceneDescriptor>(), "Cannot find SceneDescriptor attached to this renderer");
	ASSERT(m_renderer.HasFeature<Core::Rendering::EngineBufferRenderFeature>(), "Cannot find EngineBufferRenderFeature attached to this renderer");
	ASSERT(m_renderer.HasDescriptor<::Rendering::Features::LightingRenderFeature::LightingDescriptor>(), "Cannot find LightingDescriptor attached to this renderer");

	auto& engineBufferRenderFeature = m_renderer.GetFeature<Core::Rendering::EngineBufferRenderFeature>();
	auto& lightingDescriptor = m_renderer.GetDescriptor<::Rendering::Features::LightingRenderFeature::LightingDescriptor>();

	auto& sceneDescriptor = m_renderer.GetDescriptor<SceneRenderer::SceneDescriptor>();
	auto& frameDescriptor = m_renderer.GetFrameDescriptor();
	auto& scene = sceneDescriptor.scene;

	auto pso = m_renderer.CreatePipelineState();

	uint8_t lightIndex = 0;

	for (auto lightReference : lightingDescriptor.lights)
	{
		auto& light = lightReference.get();

		if (light.castShadows)
		{
			if (lightIndex < kMaxShadowMaps)
			{
				if (light.type == ::Rendering::Settings::ELightType::DIRECTIONAL)
				{
					light.UpdateShadowData(frameDescriptor.camera.value());
					const auto& lightSpaceMatrix = light.GetLightSpaceMatrix();
					const auto& shadowBuffer = light.GetShadowBuffer();
					m_shadowMaterial.SetProperty("_LightSpaceMatrix", lightSpaceMatrix);
					shadowBuffer.Bind();
					m_renderer.SetViewport(0, 0, light.shadowMapResolution, light.shadowMapResolution);
					m_renderer.Clear(true, true, true);
					DrawOpaques(pso, scene);
					shadowBuffer.Unbind();
				}
				else
				{
					// Other light types not supported!
				}
			}
		}
	}

	if (auto output = frameDescriptor.outputBuffer)
	{
		output.value().Bind();
	}

	m_renderer.SetViewport(0, 0, frameDescriptor.renderWidth, frameDescriptor.renderHeight);
}

void Core::Rendering::ShadowRenderPass::DrawOpaques(
	::Rendering::Data::PipelineState p_pso,
	Core::SceneSystem::Scene& p_scene
)
{
	for (auto modelRenderer : p_scene.GetFastAccessComponents().modelRenderers)
	{
		auto& actor = modelRenderer->owner;

		if (actor.IsActive())
		{
			if (auto model = modelRenderer->GetModel())
			{
				if (auto materialRenderer = modelRenderer->owner.GetComponent<Core::ECS::Components::CMaterialRenderer>())
				{
					const auto& materials = materialRenderer->GetMaterials();
					const auto& modelMatrix = actor.transform.GetWorldMatrix();

					for (auto mesh : model->GetMeshes())
					{
						if (auto material = materials.at(mesh->GetMaterialIndex()); material && material->IsValid() && material->IsShadowCaster())
						{
							::Rendering::Entities::Drawable drawable;
							drawable.mesh = *mesh;
							drawable.material = m_shadowMaterial;
							drawable.stateMask = m_shadowMaterial.GenerateStateMask();

							drawable.material.value().SetProperty("_ModelMatrix", modelMatrix);

							m_renderer.DrawEntity(p_pso, drawable);
						}

					}
				}
			}
		}
	}
}
