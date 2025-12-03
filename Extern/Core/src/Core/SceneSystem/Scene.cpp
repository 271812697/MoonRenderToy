#include <algorithm>
#include <string>
#include <tinyxml2.h>
#include <tracy/Tracy.hpp>
#include <Core/ECS/Components/CAmbientSphereLight.h>
#include <Core/ECS/Components/CDirectionalLight.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ResourceManagement/ModelManager.h>
#include <Core/SceneSystem/Scene.h>
#include <Core/SceneSystem/BvhService.h>


Core::SceneSystem::Scene::Scene()
{
	bvhService = new BvhService();
}

Core::SceneSystem::Scene::~Scene()
{
	std::for_each(m_actors.begin(), m_actors.end(), [](Core::ECS::Actor* element)
	{ 
		delete element;
	});

	m_actors.clear();
	delete bvhService;
}

void Core::SceneSystem::Scene::AddDefaultCamera()
{
	auto& camera = CreateActor("Main Camera");
	camera.AddComponent<ECS::Components::CCamera>();
	camera.transform.SetLocalPosition({ 0.0f, 3.0f, 8.0f });
	camera.transform.SetLocalRotation(Maths::FQuaternion({ 20.0f, 180.0f, 0.0f }));
}

void Core::SceneSystem::Scene::AddDefaultLights()
{
	auto& directionalLight = CreateActor("Directional Light");
	directionalLight.AddComponent<ECS::Components::CDirectionalLight>().SetIntensity(1.0f);
	directionalLight.transform.SetLocalPosition({ 0.0f, 10.0f, 0.0f });
	directionalLight.transform.SetLocalRotation(Maths::FQuaternion({ 120.0f, -40.0f, 0.0f }));
}

void Core::SceneSystem::Scene::AddDefaultReflections()
{
	auto& reflectionProbe = CreateActor("Reflection Probe");
	reflectionProbe.AddComponent<ECS::Components::CReflectionProbe>();
	reflectionProbe.transform.SetLocalPosition({ 0.0f, 3.0f, 0.0f });
}

void Core::SceneSystem::Scene::AddDefaultPostProcessStack()
{
	auto& postProcessStack = CreateActor("Post Process Stack");
	postProcessStack.AddComponent<ECS::Components::CPostProcessStack>();
}

void Core::SceneSystem::Scene::AddDefaultSkysphere()
{
	auto& skysphere = CreateActor("Skysphere");
	auto& materialRenderer = skysphere.AddComponent<ECS::Components::CMaterialRenderer>();
	auto& modelRenderer = skysphere.AddComponent<ECS::Components::CModelRenderer>();
	modelRenderer.SetFrustumBehaviour(ECS::Components::CModelRenderer::EFrustumBehaviour::DISABLED);

	auto skysphereMaterial = Global::ServiceLocator::Get<ResourceManagement::MaterialManager>().GetResource(":Materials\\Skysphere.ovmat");
	auto sphereModel = Global::ServiceLocator::Get<ResourceManagement::ModelManager>().GetResource(":Models\\Sphere.fbx");
	
	if (skysphereMaterial)
	{
		materialRenderer.SetMaterialAtIndex(0, *skysphereMaterial);
	}

	if (sphereModel)
	{
		modelRenderer.SetModel(sphereModel);
	}
}

void Core::SceneSystem::Scene::AddDefaultAtmosphere()
{
	auto& atmosphere = CreateActor("Atmoshpere");
	auto& materialRenderer = atmosphere.AddComponent<ECS::Components::CMaterialRenderer>();
	auto& modelRenderer = atmosphere.AddComponent<ECS::Components::CModelRenderer>();
	modelRenderer.SetFrustumBehaviour(ECS::Components::CModelRenderer::EFrustumBehaviour::DISABLED);

	auto atmosphereMaterial = Global::ServiceLocator::Get<ResourceManagement::MaterialManager>().GetResource(":Materials\\Atmosphere.ovmat");
	auto sphereModel = Global::ServiceLocator::Get<ResourceManagement::ModelManager>().GetResource(":Models\\Sphere.fbx");

	if (atmosphereMaterial)
	{
		materialRenderer.SetMaterialAtIndex(0, *atmosphereMaterial);
	}

	if (sphereModel)
	{
		modelRenderer.SetModel(sphereModel);
	}
}

void Core::SceneSystem::Scene::Play()
{
	m_isPlaying = true;

	/* Wake up actors to allow them to react to OnEnable, OnDisable and OnDestroy, */
	std::for_each(m_actors.begin(), m_actors.end(), [](ECS::Actor * p_element) { p_element->SetSleeping(false); });

	std::for_each(m_actors.begin(), m_actors.end(), [](ECS::Actor * p_element) { if (p_element->IsActive()) p_element->OnAwake(); });
	std::for_each(m_actors.begin(), m_actors.end(), [](ECS::Actor * p_element) { if (p_element->IsActive()) p_element->OnEnable(); });
	std::for_each(m_actors.begin(), m_actors.end(), [](ECS::Actor * p_element) { if (p_element->IsActive()) p_element->OnStart(); });
}

bool Core::SceneSystem::Scene::IsPlaying() const
{
	return m_isPlaying;
}

void Core::SceneSystem::Scene::Update(float p_deltaTime)
{
	ZoneScoped;
	auto actors = m_actors;
	std::for_each(actors.begin(), actors.end(), std::bind(std::mem_fn(&ECS::Actor::OnUpdate), std::placeholders::_1, p_deltaTime));
}

void Core::SceneSystem::Scene::FixedUpdate(float p_deltaTime)
{
	ZoneScoped;
	auto actors = m_actors;
	std::for_each(actors.begin(), actors.end(), std::bind(std::mem_fn(&ECS::Actor::OnFixedUpdate), std::placeholders::_1, p_deltaTime));
}

void Core::SceneSystem::Scene::LateUpdate(float p_deltaTime)
{
	ZoneScoped;
	auto actors = m_actors;
	std::for_each(actors.begin(), actors.end(), std::bind(std::mem_fn(&ECS::Actor::OnLateUpdate), std::placeholders::_1, p_deltaTime));
}

Core::ECS::Actor& Core::SceneSystem::Scene::CreateActor()
{
	return CreateActor("New Actor");
}

Core::ECS::Actor& Core::SceneSystem::Scene::CreateActor(const std::string& p_name, const std::string& p_tag)
{
	m_actors.push_back(new Core::ECS::Actor(m_availableID++, p_name, p_tag, m_isPlaying));
	ECS::Actor& instance = *m_actors.back();
	instance.ComponentAddedEvent	+= std::bind(&Scene::OnComponentAdded, this, std::placeholders::_1);
	instance.ComponentRemovedEvent	+= std::bind(&Scene::OnComponentRemoved, this, std::placeholders::_1);
	if (m_isPlaying)
	{
		instance.SetSleeping(false);
		if (instance.IsActive())
		{
			instance.OnAwake();
			instance.OnEnable();
			instance.OnStart();
		}
	}
	return instance;
}

bool Core::SceneSystem::Scene::DestroyActor(ECS::Actor& p_target)
{
	auto found = std::find_if(m_actors.begin(), m_actors.end(), [&p_target](Core::ECS::Actor* element)
	{
		return element == &p_target;
	});

	if (found != m_actors.end())
	{
		delete *found;
		m_actors.erase(found);
		return true;
	}
	else
	{
		return false;
	}
}

void Core::SceneSystem::Scene::CollectGarbages()
{
	m_actors.erase(std::remove_if(m_actors.begin(), m_actors.end(), [this](ECS::Actor* element)
	{ 
		bool isGarbage = !element->IsAlive();
		if (isGarbage)
		{
			delete element;
		}
		return isGarbage;
	}), m_actors.end());
}

Core::ECS::Actor* Core::SceneSystem::Scene::FindActorByName(const std::string& p_name) const
{
	auto result = std::find_if(m_actors.begin(), m_actors.end(), [p_name](Core::ECS::Actor* element)
	{ 
		return element->GetName() == p_name;
	});

	if (result != m_actors.end())
		return *result;
	else
		return nullptr;
}

Core::ECS::Actor* Core::SceneSystem::Scene::FindActorByTag(const std::string & p_tag) const
{
	auto result = std::find_if(m_actors.begin(), m_actors.end(), [p_tag](Core::ECS::Actor* element)
	{
		return element->GetTag() == p_tag;
	});

	if (result != m_actors.end())
		return *result;
	else
		return nullptr;
}

Core::ECS::Actor* Core::SceneSystem::Scene::FindActorByID(int64_t p_id) const
{
	auto result = std::find_if(m_actors.begin(), m_actors.end(), [p_id](Core::ECS::Actor* element)
	{
		return element->GetID() == p_id;
	});

	if (result != m_actors.end())
		return *result;
	else
		return nullptr;
}

std::vector<std::reference_wrapper<Core::ECS::Actor>> Core::SceneSystem::Scene::FindActorsByName(const std::string & p_name) const
{
	std::vector<std::reference_wrapper<Core::ECS::Actor>> actors;

	for (auto actor : m_actors)
	{
		if (actor->GetName() == p_name)
			actors.push_back(std::ref(*actor));
	}

	return actors;
}

std::vector<std::reference_wrapper<Core::ECS::Actor>> Core::SceneSystem::Scene::FindActorsByTag(const std::string & p_tag) const
{
	std::vector<std::reference_wrapper<Core::ECS::Actor>> actors;

	for (auto actor : m_actors)
	{
		if (actor->GetTag() == p_tag)
			actors.push_back(std::ref(*actor));
	}

	return actors;
}

Core::ECS::Components::CCamera* Core::SceneSystem::Scene::FindMainCamera() const
{
	for (Core::ECS::Components::CCamera* camera : m_fastAccessComponents.cameras)
	{
		if (camera->owner.IsActive())
		{
			return camera;
		}
	}

	return nullptr;
}

void Core::SceneSystem::Scene::OnComponentAdded(ECS::Components::AComponent& p_compononent)
{
	if (auto result = dynamic_cast<ECS::Components::CModelRenderer*>(&p_compononent))
		m_fastAccessComponents.modelRenderers.push_back(result);

	if (auto result = dynamic_cast<ECS::Components::CCamera*>(&p_compononent))
		m_fastAccessComponents.cameras.push_back(result);

	if (auto result = dynamic_cast<ECS::Components::CLight*>(&p_compononent))
		m_fastAccessComponents.lights.push_back(result);

	if (auto result = dynamic_cast<ECS::Components::CPostProcessStack*>(&p_compononent))
		m_fastAccessComponents.postProcessStacks.push_back(result);

	if (auto result = dynamic_cast<ECS::Components::CReflectionProbe*>(&p_compononent))
		m_fastAccessComponents.reflectionProbes.push_back(result);
}

void Core::SceneSystem::Scene::OnComponentRemoved(ECS::Components::AComponent& p_compononent)
{
	if (auto result = dynamic_cast<ECS::Components::CModelRenderer*>(&p_compononent))
		m_fastAccessComponents.modelRenderers.erase(std::remove(m_fastAccessComponents.modelRenderers.begin(), m_fastAccessComponents.modelRenderers.end(), result), m_fastAccessComponents.modelRenderers.end());

	if (auto result = dynamic_cast<ECS::Components::CCamera*>(&p_compononent))
		m_fastAccessComponents.cameras.erase(std::remove(m_fastAccessComponents.cameras.begin(), m_fastAccessComponents.cameras.end(), result), m_fastAccessComponents.cameras.end());

	if (auto result = dynamic_cast<ECS::Components::CLight*>(&p_compononent))
		m_fastAccessComponents.lights.erase(std::remove(m_fastAccessComponents.lights.begin(), m_fastAccessComponents.lights.end(), result), m_fastAccessComponents.lights.end());

	if (auto result = dynamic_cast<ECS::Components::CPostProcessStack*>(&p_compononent))
		m_fastAccessComponents.postProcessStacks.erase(std::remove(m_fastAccessComponents.postProcessStacks.begin(), m_fastAccessComponents.postProcessStacks.end(), result), m_fastAccessComponents.postProcessStacks.end());

	if (auto result = dynamic_cast<ECS::Components::CReflectionProbe*>(&p_compononent))
		m_fastAccessComponents.reflectionProbes.erase(std::remove(m_fastAccessComponents.reflectionProbes.begin(), m_fastAccessComponents.reflectionProbes.end(), result), m_fastAccessComponents.reflectionProbes.end());
}

std::vector<Core::ECS::Actor*>& Core::SceneSystem::Scene::GetActors()
{
	return m_actors;
}

const Core::SceneSystem::Scene::FastAccessComponents& Core::SceneSystem::Scene::GetFastAccessComponents() const
{
	return m_fastAccessComponents;
}

void Core::SceneSystem::Scene::OnSerialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_root)
{
	tinyxml2::XMLNode* sceneNode = p_doc.NewElement("scene");
	p_root->InsertEndChild(sceneNode);

	tinyxml2::XMLNode* actorsNode = p_doc.NewElement("actors");
	sceneNode->InsertEndChild(actorsNode);

	for (auto& actor : m_actors)
	{
		actor->OnSerialize(p_doc, actorsNode);
	}
}

void Core::SceneSystem::Scene::OnDeserialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_root)
{
	tinyxml2::XMLNode* actorsRoot = p_root->FirstChildElement("actors");

	if (actorsRoot)
	{
		tinyxml2::XMLElement* currentActor = actorsRoot->FirstChildElement("actor");

		int64_t maxID = 1;

		while (currentActor)
		{
			auto& actor = CreateActor();
			actor.OnDeserialize(p_doc, currentActor);
			maxID = std::max(actor.GetID() + 1, maxID);
			currentActor = currentActor->NextSiblingElement("actor");
		}

		m_availableID = maxID;

		/* We recreate the hierarchy of the scene by attaching children to their parents */
		for (auto actor : m_actors)
		{
			if (actor->GetParentID() > 0)
			{
				if (auto found = FindActorByID(actor->GetParentID()); found)
					actor->SetParent(*found);
			}
		}
	}
}

void Core::SceneSystem::Scene::BuildSceneBvh()
{
	ZoneScoped;
	bvhService->Clear();
	int i = 0;
	std::vector<::Rendering::Geometry::bbox> bounds;
	std::vector<MeshInstance>meshInstances;
	std::vector<Core::Resources::Material*>matrialLists;
	std::vector<::Rendering::Resources::Mesh*>sceneMeshes;
	for (auto modelRenderer : m_fastAccessComponents.modelRenderers)
	{
		if (modelRenderer->owner.IsActive())
		{
			
			auto model=modelRenderer->GetModel();
			auto mat=modelRenderer->owner.GetComponent<Core::ECS::Components::CMaterialRenderer>();
			if (model&&mat) {
				auto matrix = modelRenderer->owner.transform.GetWorldMatrix();	
				for (auto& m: model->GetMeshes()) {
					
					auto meshInstaceBox=m->GetBoundingBox().transform(matrix);
					if (meshInstaceBox.isValid()) {
						bounds.push_back(meshInstaceBox);

						//figure out  the matId
						int materialId = -1;
						auto meshMat=mat->GetMaterialAtIndex(m->GetMaterialIndex()[0]);
						for (int j = 0;j < matrialLists.size();j++) {
							if (matrialLists[j] == meshMat) {
								materialId = j;
								break;
							}
						}
						if (materialId == -1) {
							materialId = static_cast<int>(matrialLists.size());
							matrialLists.push_back(meshMat);
							bvhService->AddMaterial(meshMat);
						}
						
						//figure out the meshId in the sceneMeshes if not find,push back the mesh to sceneMeshes
						int meshId = -1;
						for (int i = 0;i < sceneMeshes.size();i++) {
							if (sceneMeshes[i] == m) {
								meshId = i;
								break;
							}
						}
						if (meshId == -1) {
							meshId = static_cast<int>(sceneMeshes.size());
							sceneMeshes.push_back(m);
						}


						//push back the mesh instance
						MeshInstance meshInstance(modelRenderer->owner.GetName(), meshId, matrix, materialId);
						meshInstances.push_back(meshInstance);
					}
				}
			}
		}
	}
	// Build BVH
	bvhService->m_sceneBvh->Build(bounds.data(), static_cast<int>(bounds.size()));
	bvhService->Process(bvhService->m_sceneBvh,sceneMeshes, meshInstances);
}

bool Core::SceneSystem::Scene::RayHit(const::Rendering::Geometry::Ray& ray,Maths::FVector3& outPoint)
{
	float triDist = 1e6;
	bool hit = false;
	Maths::FVector3 hitNormal;
	Maths::FVector3 bary;
	std::vector<::Rendering::Geometry::Bvh::Node*>stack;
	stack.push_back(bvhService->m_sceneBvh->m_root);
	while (!stack.empty()) {
		auto cur = stack.back();stack.pop_back();
		if (!cur)continue;
		float tempDist = 1e6;
		if (ray.HitDistance(cur->bounds, tempDist)) 
		{
			if (cur->type == ::Rendering::Geometry::Bvh::kInternal) {
				stack.push_back(cur->lc);
				stack.push_back(cur->rc);
			}
			else if (cur->type == ::Rendering::Geometry::Bvh::kLeaf) {
				for (int i = cur->startidx;i < cur->startidx + cur->numprims;i++) {
					int index= bvhService->m_sceneBvh->m_packed_indices[i];
					int meshId=bvhService->meshInstances[index].meshID;
					auto matrix = bvhService->meshInstances[index].transform;
					auto invMatrix = Maths::FMatrix4::Inverse(matrix);
					auto localRay = ray.Transformed(invMatrix);
					auto& mesh = bvhService->meshes[meshId];
					auto meshBvh = mesh->GetBvh();		
					std::vector<::Rendering::Geometry::Bvh::Node*>meshBvhStack;
					meshBvhStack.push_back(meshBvh->m_root);
					while (!meshBvhStack.empty()) {
						auto meshBvhCur = meshBvhStack.back(); meshBvhStack.pop_back();
						if (!meshBvhCur)continue;
						float meshTempDist = 1e6;
						if (localRay.HitDistance(meshBvhCur->bounds, meshTempDist))
						{
							if (meshBvhCur->type == ::Rendering::Geometry::Bvh::kInternal) {
								meshBvhStack.push_back(meshBvhCur->lc);
								meshBvhStack.push_back(meshBvhCur->rc);
							}
							else if (meshBvhCur->type == ::Rendering::Geometry::Bvh::kLeaf) {
								for (int j = meshBvhCur->startidx;j < meshBvhCur->startidx + meshBvhCur->numprims;j++) {
									int triIndex = meshBvh->m_packed_indices[j];
									Maths::FVector3 v0 = mesh->GetVertexPosition(triIndex * 3);
									Maths::FVector3 v1 = mesh->GetVertexPosition(triIndex * 3 + 1);
									Maths::FVector3 v2 = mesh->GetVertexPosition(triIndex * 3 + 2);
									float currentTriDist = 1e6;
									Maths::FVector3 currentHitNormal;
									Maths::FVector3 currentBary;
									if (localRay.HitDistance(v0, v1, v2, currentTriDist, &currentHitNormal, &currentBary)) {
										if (currentTriDist < triDist) {
											triDist = currentTriDist;
											hitNormal = currentHitNormal;
											bary = currentBary;
											outPoint = v0 * bary[0] + v1 * bary[1] + v2 * bary[2];
											outPoint = Maths::FMatrix4::MulPoint(matrix,outPoint);
											hit = true;
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
	return hit;
}

::Rendering::Geometry::Bvh* Core::SceneSystem::Scene::GetBvh()
{
	return bvhService->m_sceneBvh;
}
