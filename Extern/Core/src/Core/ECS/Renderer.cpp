

#include <Analytics/Profiling/ProfilerSpy.h>

#include <Rendering/Resources/Loaders/TextureLoader.h>
#include <Rendering/Data/Frustum.h>

#include "Core/ECS/Renderer.h"
#include "Core/ECS/Components/CModelRenderer.h"
#include "Core/ECS/Components/CMaterialRenderer.h"

Core::ECS::Renderer::Renderer(Rendering::Context::Driver& p_driver) :
	Rendering::Core::Renderer(p_driver),
	m_emptyTexture(Rendering::Resources::Loaders::TextureLoader::CreateColor
	(
		(255 << 24) | (255 << 16) | (255 << 8) | 255,
		Rendering::Settings::ETextureFilteringMode::NEAREST,
		Rendering::Settings::ETextureFilteringMode::NEAREST,
		false
	))
{
}

Core::ECS::Renderer::~Renderer()
{
	Rendering::Resources::Loaders::TextureLoader::Destroy(m_emptyTexture);
}

Core::ECS::Components::CCamera* Core::ECS::Renderer::FindMainCamera(const Core::SceneSystem::Scene& p_scene)
{
	for (Core::ECS::Components::CCamera* camera : p_scene.GetFastAccessComponents().cameras)
		if (camera->owner.IsActive())
			return camera;

	return nullptr;
}

std::vector<Maths::FMatrix4> Core::ECS::Renderer::FindLightMatrices(const Core::SceneSystem::Scene& p_scene)
{
	std::vector<Maths::FMatrix4> result;

	const auto& facs = p_scene.GetFastAccessComponents();

	for (auto light : facs.lights)
	{
		if (light->owner.IsActive())
		{
			result.push_back(light->GetData().GenerateMatrix());
		}
	}

	return result;
}



std::vector<Maths::FMatrix4> Core::ECS::Renderer::FindLightMatricesInFrustum(const Core::SceneSystem::Scene& p_scene, const Rendering::Data::Frustum& p_frustum)
{
	std::vector<Maths::FMatrix4> result;

	const auto& facs = p_scene.GetFastAccessComponents();

	for (auto light : facs.lights)
	{
		if (light->owner.IsActive())
		{
			const auto& lightData = light->GetData();
			const auto& position = lightData.GetTransform().GetWorldPosition();
			auto effectRange = lightData.GetEffectRange();

			// We always consider lights that have an +inf range (Not necessary to test if they are in frustum)
			if (std::isinf(effectRange) || p_frustum.SphereInFrustum(position.x, position.y, position.z, lightData.GetEffectRange()))
			{
				result.push_back(lightData.GenerateMatrix());
			}
		}
	}

	return result;
}

void Core::ECS::Renderer::RenderScene
(
	Core::SceneSystem::Scene& p_scene,
	const Maths::FVector3& p_cameraPosition,
	const Rendering::LowRenderer::Camera& p_camera,
	const Rendering::Data::Frustum* p_customFrustum,
	Core::Resources::Material* p_defaultMaterial
)
{
	OpaqueDrawables	opaqueMeshes;
	TransparentDrawables transparentMeshes;

	if (p_camera.HasFrustumGeometryCulling())
	{
		const auto& frustum = p_customFrustum ? *p_customFrustum : p_camera.GetFrustum();
		std::tie(opaqueMeshes, transparentMeshes) = FindAndSortFrustumCulledDrawables(p_scene, p_cameraPosition, frustum, p_defaultMaterial);
	}
	else
	{
		std::tie(opaqueMeshes, transparentMeshes) = FindAndSortDrawables(p_scene, p_cameraPosition, p_defaultMaterial);
	}

	for (const auto& [distance, drawable] : opaqueMeshes)
		DrawDrawable(drawable);

	for (const auto& [distance, drawable] : transparentMeshes)
		DrawDrawable(drawable);
}
void Core::ECS::Renderer::RenderScene
(
	Core::SceneSystem::Scene& p_scene,
	const Maths::FVector3& p_cameraPosition,
	Core::Resources::Material* p_defaultMaterial
)
{
	OpaqueDrawables	opaqueMeshes;
	TransparentDrawables transparentMeshes;
    std::tie(opaqueMeshes, transparentMeshes) = FindAndSortDrawables(p_scene, p_cameraPosition, p_defaultMaterial);
	for (const auto& [distance, drawable] : opaqueMeshes)
		DrawDrawable(drawable);

	for (const auto& [distance, drawable] : transparentMeshes)
		DrawDrawable(drawable);
}
void Core::ECS::Renderer::RenderSceneShadow
(
    Core::SceneSystem::Scene& p_scene,
    const Maths::FVector3& p_cameraPosition,
    const Rendering::LowRenderer::Camera& p_camera,
    const Rendering::Data::Frustum* p_customFrustum,
    Core::Resources::Material* p_defaultMaterial
)
{
    OpaqueDrawables	opaqueMeshes;
    TransparentDrawables transparentMeshes;

    if (p_camera.HasFrustumGeometryCulling())
    {
        const auto& frustum = p_customFrustum ? *p_customFrustum : p_camera.GetFrustum();
        std::tie(opaqueMeshes, transparentMeshes) = FindAndSortFrustumCulledDrawables(p_scene, p_cameraPosition, frustum, p_defaultMaterial);
    }
    else
    {
        std::tie(opaqueMeshes, transparentMeshes) = FindAndSortDrawables(p_scene, p_cameraPosition, p_defaultMaterial);
    }

    for (const auto& [distance, drawable] : opaqueMeshes)
        DrawDrawable(drawable);

    for (const auto& [distance, drawable] : transparentMeshes)
        DrawDrawable(drawable);
}

void FindAndSortDrawables
(
	Core::ECS::Renderer::OpaqueDrawables& p_opaques,
	Core::ECS::Renderer::TransparentDrawables& p_transparents,
	const Core::SceneSystem::Scene& p_scene,
	const Maths::FVector3& p_cameraPosition,
	Core::Resources::Material* p_defaultMaterial
)
{
	for (Core::ECS::Components::CModelRenderer* modelRenderer : p_scene.GetFastAccessComponents().modelRenderers)
	{
		if (modelRenderer->owner.IsActive())
		{
			if (auto model = modelRenderer->GetModel())
			{
				float distanceToActor = Maths::FVector3::Distance(modelRenderer->owner.transform.GetWorldPosition(), p_cameraPosition);

				if (auto materialRenderer = modelRenderer->owner.GetComponent<Core::ECS::Components::CMaterialRenderer>())
				{
					const auto& transform = modelRenderer->owner.transform.GetFTransform();

					const Core::ECS::Components::CMaterialRenderer::MaterialList& materials = materialRenderer->GetMaterials();

					for (auto mesh : model->GetMeshes())
					{
						Core::Resources::Material* material = nullptr;

						if (mesh->GetMaterialIndex() < MAX_MATERIAL_COUNT)
						{
							material = materials.at(mesh->GetMaterialIndex());
							if (!material || !material->GetShader())
								material = p_defaultMaterial;
						}

						if (material)
						{
							Core::ECS::Renderer::Drawable element = { transform.GetWorldMatrix(), mesh, material, materialRenderer->GetUserMatrix() };

							if (material->IsBlendable())
								p_transparents.emplace(distanceToActor, element);
							else
								p_opaques.emplace(distanceToActor, element);
						}
					}
				}
			}
		}
	}
}

std::pair<Core::ECS::Renderer::OpaqueDrawables, Core::ECS::Renderer::TransparentDrawables> Core::ECS::Renderer::FindAndSortFrustumCulledDrawables
(
	const Core::SceneSystem::Scene& p_scene,
	const Maths::FVector3& p_cameraPosition,
	const Rendering::Data::Frustum& p_frustum,
	Core::Resources::Material* p_defaultMaterial
)
{
	using namespace Core::ECS::Components;

	Core::ECS::Renderer::OpaqueDrawables opaqueDrawables;
	Core::ECS::Renderer::TransparentDrawables transparentDrawables;

	for (CModelRenderer* modelRenderer : p_scene.GetFastAccessComponents().modelRenderers)
	{
		auto& owner = modelRenderer->owner;

		if (owner.IsActive())
		{
			if (auto model = modelRenderer->GetModel())
			{
				if (auto materialRenderer = modelRenderer->owner.GetComponent<CMaterialRenderer>())
				{
					auto& transform = owner.transform.GetFTransform();

					Rendering::Settings::ECullingOptions cullingOptions = Rendering::Settings::ECullingOptions::NONE;

					if (modelRenderer->GetFrustumBehaviour() != CModelRenderer::EFrustumBehaviour::DISABLED)
					{
						cullingOptions |= Rendering::Settings::ECullingOptions::FRUSTUM_PER_MODEL;
					}

					if (modelRenderer->GetFrustumBehaviour() == CModelRenderer::EFrustumBehaviour::CULL_MESHES)
					{
						cullingOptions |= Rendering::Settings::ECullingOptions::FRUSTUM_PER_MESH;
					}

					const auto& modelBoundingSphere = modelRenderer->GetFrustumBehaviour() == CModelRenderer::EFrustumBehaviour::CULL_CUSTOM ? modelRenderer->GetCustomBoundingSphere() : model->GetBoundingSphere();

					std::vector<std::reference_wrapper<Rendering::Resources::Mesh>> meshes;

					{
						PROFILER_SPY("Frustum Culling");
						meshes = GetMeshesInFrustum(*model, modelBoundingSphere, transform, p_frustum, cullingOptions);
					}

					if (!meshes.empty())
					{
						float distanceToActor = Maths::FVector3::Distance(transform.GetWorldPosition(), p_cameraPosition);
						const Core::ECS::Components::CMaterialRenderer::MaterialList& materials = materialRenderer->GetMaterials();

						for (const auto& mesh : meshes)
						{
							Core::Resources::Material* material = nullptr;

							if (mesh.get().GetMaterialIndex() < MAX_MATERIAL_COUNT)
							{
								material = materials.at(mesh.get().GetMaterialIndex());
								if (!material || !material->GetShader())
									material = p_defaultMaterial;
							}

							if (material)
							{
								Core::ECS::Renderer::Drawable element = { transform.GetWorldMatrix(), &mesh.get(), material, materialRenderer->GetUserMatrix() };

								if (material->IsBlendable())
									transparentDrawables.emplace(distanceToActor, element);
								else
									opaqueDrawables.emplace(distanceToActor, element);
							}
						}
					}
				}
			}
		}
	}

	return { opaqueDrawables, transparentDrawables };
}

std::pair<Core::ECS::Renderer::OpaqueDrawables, Core::ECS::Renderer::TransparentDrawables> Core::ECS::Renderer::FindAndSortDrawables
(
	const Core::SceneSystem::Scene& p_scene,
	const Maths::FVector3& p_cameraPosition,
	Core::Resources::Material* p_defaultMaterial
)
{
	Core::ECS::Renderer::OpaqueDrawables opaqueDrawables;
	Core::ECS::Renderer::TransparentDrawables transparentDrawables;

	for (Core::ECS::Components::CModelRenderer* modelRenderer : p_scene.GetFastAccessComponents().modelRenderers)
	{
		if (modelRenderer->owner.IsActive())
		{
			if (auto model = modelRenderer->GetModel())
			{
				float distanceToActor = Maths::FVector3::Distance(modelRenderer->owner.transform.GetWorldPosition(), p_cameraPosition);

				if (auto materialRenderer = modelRenderer->owner.GetComponent<Core::ECS::Components::CMaterialRenderer>())
				{
					const auto& transform = modelRenderer->owner.transform.GetFTransform();

					const Core::ECS::Components::CMaterialRenderer::MaterialList& materials = materialRenderer->GetMaterials();

					for (auto mesh : model->GetMeshes())
					{
						Core::Resources::Material* material = nullptr;

						if (mesh->GetMaterialIndex() < MAX_MATERIAL_COUNT)
						{
							material = materials.at(mesh->GetMaterialIndex());
							if (!material || !material->GetShader())
								material = p_defaultMaterial;
						}

						if (material)
						{
							Core::ECS::Renderer::Drawable element = { transform.GetWorldMatrix(), mesh, material, materialRenderer->GetUserMatrix() };

							if (material->IsBlendable())
								transparentDrawables.emplace(distanceToActor, element);
							else
								opaqueDrawables.emplace(distanceToActor, element);
						}
					}
				}
			}
		}
	}

	return { opaqueDrawables, transparentDrawables };
}

void Core::ECS::Renderer::DrawDrawable(const Drawable& p_toDraw)
{
	m_userMatrixSender(std::get<3>(p_toDraw));
	DrawMesh(*std::get<1>(p_toDraw), *std::get<2>(p_toDraw), &std::get<0>(p_toDraw));
}

void Core::ECS::Renderer::DrawModelWithSingleMaterial(Rendering::Resources::Model& p_model, Core::Resources::Material& p_material, Maths::FMatrix4 const* p_modelMatrix, Core::Resources::Material* p_defaultMaterial)
{
	if (p_modelMatrix)
		m_modelMatrixSender(*p_modelMatrix);

	for (auto mesh : p_model.GetMeshes())
	{
		Core::Resources::Material* material = p_material.GetShader() ? &p_material : p_defaultMaterial;

		if (material)
			DrawMesh(*mesh, *material, nullptr);
	}
}

void Core::ECS::Renderer::DrawModelWithMaterials(Rendering::Resources::Model& p_model, std::vector<Core::Resources::Material*> p_materials, Maths::FMatrix4 const* p_modelMatrix, Core::Resources::Material* p_defaultMaterial)
{
	if (p_modelMatrix)
		m_modelMatrixSender(*p_modelMatrix);

	for (auto mesh : p_model.GetMeshes())
	{
		Core::Resources::Material* material = p_materials.size() > mesh->GetMaterialIndex() ? p_materials[mesh->GetMaterialIndex()] : p_defaultMaterial;
		if (material)
			DrawMesh(*mesh, *material, nullptr);
	}
}

void Core::ECS::Renderer::DrawMesh(Rendering::Resources::Mesh& p_mesh, Core::Resources::Material& p_material, Maths::FMatrix4 const* p_modelMatrix)
{
	using namespace Rendering::Settings;

	if (p_material.HasShader() && p_material.GetGPUInstances() > 0)
	{
		if (p_modelMatrix)
			m_modelMatrixSender(*p_modelMatrix);

		uint8_t stateMask = p_material.GenerateStateMask();
		ApplyStateMask(stateMask);
		
		/* Draw the mesh */
		p_material.Bind(m_emptyTexture);
		Draw(p_mesh, Rendering::Settings::EPrimitiveMode::TRIANGLES, p_material.GetGPUInstances());
		p_material.UnBind();
	}
}

void Core::ECS::Renderer::RegisterModelMatrixSender(std::function<void(Maths::FMatrix4)> p_modelMatrixSender)
{
	m_modelMatrixSender = p_modelMatrixSender;
}

void Core::ECS::Renderer::RegisterUserMatrixSender(std::function<void(Maths::FMatrix4)> p_userMatrixSender)
{
	m_userMatrixSender = p_userMatrixSender;
}
