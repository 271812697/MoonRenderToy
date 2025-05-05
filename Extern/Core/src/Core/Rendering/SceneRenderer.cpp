/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <tracy/Tracy.hpp>

#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Core/Rendering/PostProcessRenderPass.h>
#include <Core/Rendering/SceneRenderer.h>
#include <Core/Rendering/ShadowRenderFeature.h>
#include <Core/Rendering/ShadowRenderPass.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Rendering/Data/Frustum.h>
#include <Rendering/Features/LightingRenderFeature.h>
#include <Rendering/HAL/Profiling.h>
#include <Rendering/Resources/Loaders/ShaderLoader.h>

struct SceneRenderPassDescriptor
{
	Core::Rendering::SceneRenderer::AllDrawables drawables;
};

class SceneRenderPass : public ::Rendering::Core::ARenderPass
{
public:
	SceneRenderPass(::Rendering::Core::CompositeRenderer& p_renderer, bool stencilWrite = false) :
		::Rendering::Core::ARenderPass(p_renderer),
		m_stencilWrite(stencilWrite)
	{}

protected:
	void PrepareStencilBuffer(::Rendering::Data::PipelineState& p_pso)
	{
		p_pso.stencilTest = true;
		p_pso.stencilWriteMask = 0xFF;
		p_pso.stencilFuncRef = 1;
		p_pso.stencilFuncMask = 0xFF;
		p_pso.stencilOpFail = ::Rendering::Settings::EOperation::REPLACE;
		p_pso.depthOpFail = ::Rendering::Settings::EOperation::REPLACE;
		p_pso.bothOpFail = ::Rendering::Settings::EOperation::REPLACE;
		p_pso.colorWriting.mask = 0x00;
	}

private:
	bool m_stencilWrite;
};

class OpaqueRenderPass : public SceneRenderPass
{
public:
	OpaqueRenderPass(::Rendering::Core::CompositeRenderer& p_renderer, bool p_stencilWrite = false) :
		SceneRenderPass(p_renderer, p_stencilWrite)
	{}

protected:
	virtual void Draw(::Rendering::Data::PipelineState p_pso) override
	{
		ZoneScoped;
		TracyGpuZone("OpaqueRenderPass");

		PrepareStencilBuffer(p_pso);

		auto& sceneContent = m_renderer.GetDescriptor<SceneRenderPassDescriptor>();

		for (const auto& [distance, drawable] : sceneContent.drawables.opaques)
		{
			m_renderer.DrawEntity(p_pso, drawable);
		}
	}
};

class TransparentRenderPass : public SceneRenderPass
{
public:
	TransparentRenderPass(::Rendering::Core::CompositeRenderer& p_renderer, bool p_stencilWrite = false) :
		SceneRenderPass(p_renderer, p_stencilWrite) {}

protected:
	virtual void Draw(::Rendering::Data::PipelineState p_pso) override
	{
		ZoneScoped;
		TracyGpuZone("TransparentRenderPass");

		PrepareStencilBuffer(p_pso);

		auto& sceneContent = m_renderer.GetDescriptor<SceneRenderPassDescriptor>();

		for (const auto& [distance, drawable] : sceneContent.drawables.transparents)
		{
			m_renderer.DrawEntity(p_pso, drawable);
		}
	}
};

class UIRenderPass : public SceneRenderPass
{
public:
	UIRenderPass(::Rendering::Core::CompositeRenderer& p_renderer, bool p_stencilWrite = false) :
		SceneRenderPass(p_renderer, p_stencilWrite) {}

protected:
	virtual void Draw(::Rendering::Data::PipelineState p_pso) override
	{
		ZoneScoped;
		TracyGpuZone("UIRenderPass");

		PrepareStencilBuffer(p_pso);

		auto& sceneContent = m_renderer.GetDescriptor<SceneRenderPassDescriptor>();

		for (const auto& [distance, drawable] : sceneContent.drawables.ui)
		{
			m_renderer.DrawEntity(p_pso, drawable);
		}
	}
};


Core::Rendering::SceneRenderer::SceneRenderer(::Rendering::Context::Driver& p_driver, bool p_stencilWrite)
	: ::Rendering::Core::CompositeRenderer(p_driver)
{
	AddFeature<EngineBufferRenderFeature>();
	AddFeature<::Rendering::Features::LightingRenderFeature>();
	AddFeature<ShadowRenderFeature>();

	AddPass<ShadowRenderPass>("Shadows", ::Rendering::Settings::ERenderPassOrder::Shadows);
	AddPass<OpaqueRenderPass>("Opaques", ::Rendering::Settings::ERenderPassOrder::Opaque, p_stencilWrite);
	AddPass<TransparentRenderPass>("Transparents", ::Rendering::Settings::ERenderPassOrder::Transparent, p_stencilWrite);
	AddPass<PostProcessRenderPass>("Post-Process", ::Rendering::Settings::ERenderPassOrder::PostProcessing);
	AddPass<UIRenderPass>("UI", ::Rendering::Settings::ERenderPassOrder::UI);
}

::Rendering::Features::LightingRenderFeature::LightSet FindActiveLights(const Core::SceneSystem::Scene& p_scene)
{
	::Rendering::Features::LightingRenderFeature::LightSet lights;

	const auto& facs = p_scene.GetFastAccessComponents();

	for (auto light : facs.lights)
	{
		if (light->owner.IsActive())
		{
			lights.push_back(std::ref(light->GetData()));
		}
	}

	return lights;
}

void Core::Rendering::SceneRenderer::BeginFrame(const ::Rendering::Data::FrameDescriptor& p_frameDescriptor)
{
	ZoneScoped;

	ASSERT(HasDescriptor<SceneDescriptor>(), "Cannot find SceneDescriptor attached to this renderer");

	auto& sceneDescriptor = GetDescriptor<SceneDescriptor>();

	const bool frustumLightCulling = p_frameDescriptor.camera.value().HasFrustumLightCulling();

	AddDescriptor<::Rendering::Features::LightingRenderFeature::LightingDescriptor>({
		FindActiveLights(sceneDescriptor.scene),
		frustumLightCulling ? sceneDescriptor.frustumerride : std::nullopt
		});

	::Rendering::Core::CompositeRenderer::BeginFrame(p_frameDescriptor);

	AddDescriptor<SceneRenderPassDescriptor>({
		ParseScene()
		});
}

void Core::Rendering::SceneRenderer::DrawModelWithSingleMaterial(::Rendering::Data::PipelineState p_pso, ::Rendering::Resources::Model& p_model, ::Rendering::Data::Material& p_material, const Maths::FMatrix4& p_modelMatrix)
{
	auto stateMask = p_material.GenerateStateMask();
	auto userMatrix = Maths::FMatrix4::Identity;

	auto engineDrawableDescriptor = EngineDrawableDescriptor{
		p_modelMatrix,
		userMatrix
	};

	for (auto mesh : p_model.GetMeshes())
	{
		::Rendering::Entities::Drawable element;
		element.mesh = *mesh;
		element.material = p_material;
		element.stateMask = stateMask;
		element.AddDescriptor(engineDrawableDescriptor);

		DrawEntity(p_pso, element);
	}
}

Core::Rendering::SceneRenderer::AllDrawables Core::Rendering::SceneRenderer::ParseScene()
{
	ZoneScoped;

	using namespace Core::ECS::Components;

	OpaqueDrawables opaques;
	TransparentDrawables transparents;
	UIDrawables ui;

	auto& camera = m_frameDescriptor.camera.value();

	auto& sceneDescriptor = GetDescriptor<SceneDescriptor>();
	auto& scene = sceneDescriptor.scene;
	auto overrideMaterial = sceneDescriptor.overrideMaterial;
	auto fallbackMaterial = sceneDescriptor.fallbackMaterial;

	Tools::Utils::OptRef<const ::Rendering::Data::Frustum> frustum;

	if (camera.HasFrustumGeometryCulling())
	{
		auto& frustumerride = sceneDescriptor.frustumerride;
		frustum = frustumerride ? frustumerride : camera.GetFrustum();
	}

	for (CModelRenderer* modelRenderer : scene.GetFastAccessComponents().modelRenderers)
	{
		auto& owner = modelRenderer->owner;

		if (owner.IsActive())
		{
			if (auto model = modelRenderer->GetModel())
			{
				if (auto materialRenderer = modelRenderer->owner.GetComponent<CMaterialRenderer>())
				{
					auto& transform = owner.transform.GetFTransform();

					auto cullingOptions = ::Rendering::Settings::ECullingOptions::NONE;

					if (modelRenderer->GetFrustumBehaviour() != CModelRenderer::EFrustumBehaviour::DISABLED)
					{
						cullingOptions |= ::Rendering::Settings::ECullingOptions::FRUSTUM_PER_MODEL;
					}

					if (modelRenderer->GetFrustumBehaviour() == CModelRenderer::EFrustumBehaviour::CULL_MESHES)
					{
						cullingOptions |= ::Rendering::Settings::ECullingOptions::FRUSTUM_PER_MESH;
					}

					const auto& modelBoundingSphere = modelRenderer->GetFrustumBehaviour() == CModelRenderer::EFrustumBehaviour::CULL_CUSTOM ? modelRenderer->GetCustomBoundingSphere() : model->GetBoundingSphere();

					std::vector<::Rendering::Resources::Mesh*> meshes;

					if (frustum)
					{
						ZoneScopedN("Frustum Culling");
						meshes = frustum.value().GetMeshesInFrustum(*model, modelBoundingSphere, transform, cullingOptions);
					}
					else
					{
						meshes = model->GetMeshes();
					}

					if (!meshes.empty())
					{
						float distanceToActor = Maths::FVector3::Distance(transform.GetWorldPosition(), camera.GetPosition());
						const Core::ECS::Components::CMaterialRenderer::MaterialList& materials = materialRenderer->GetMaterials();

						for (const auto& mesh : meshes)
						{
							Tools::Utils::OptRef<::Rendering::Data::Material> material;

							if (mesh->GetMaterialIndex() < kMaxMaterialCount)
							{
								if (overrideMaterial && overrideMaterial->IsValid())
								{
									material = overrideMaterial.value();
								}
								else
								{
									material = materials.at(mesh->GetMaterialIndex());
								}

								const bool isMaterialValid = material && material->IsValid();
								const bool hasValidFallbackMaterial = fallbackMaterial && fallbackMaterial->IsValid();

								if (!isMaterialValid && hasValidFallbackMaterial)
								{
									material = fallbackMaterial;
								}
							}

							if (material && material->IsValid())
							{
								::Rendering::Entities::Drawable drawable;
								drawable.mesh = *mesh;
								drawable.material = material;
								drawable.stateMask = material->GenerateStateMask();

								drawable.AddDescriptor<EngineDrawableDescriptor>({
									transform.GetWorldMatrix(),
									materialRenderer->GetUserMatrix()
									});

								if (material->IsUserInterface())
								{
									ui.emplace(distanceToActor, drawable);
								}
								else
								{
									if (material->IsBlendable())
									{
										transparents.emplace(distanceToActor, drawable);
									}
									else
									{
										opaques.emplace(distanceToActor, drawable);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	return { opaques, transparents, ui };
}
