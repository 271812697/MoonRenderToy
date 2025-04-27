
#include"../../tools//tinyxml2.h"



#include "Core/SceneSystem/SceneManager.h"
#include "Core/ECS/Components/CDirectionalLight.h"
#include "Core/ECS/Components/CAmbientSphereLight.h"
#include "Core/ECS/Components/CCamera.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "Core/ResourceManagement/MaterialManager.h"

Core::SceneSystem::SceneManager::SceneManager(const std::string& p_sceneRootFolder) : m_sceneRootFolder(p_sceneRootFolder)
{
	LoadEmptyScene();
	//LoadEmptyLightedScene();
}

Core::SceneSystem::SceneManager::~SceneManager()
{
	UnloadCurrentScene();
}

void Core::SceneSystem::SceneManager::Update()
{
	if (m_delayedLoadCall)
	{
		m_delayedLoadCall();
		m_delayedLoadCall = 0;
	}
}

void Core::SceneSystem::SceneManager::LoadAndPlayDelayed(const std::string& p_path, bool p_absolute)
{
	m_delayedLoadCall = [this, p_path, p_absolute]
	{
		std::string previousSourcePath = GetCurrentSceneSourcePath();
		LoadScene(p_path, p_absolute);
		StoreCurrentSceneSourcePath(previousSourcePath);
		GetCurrentScene()->Play();
	};
}

void Core::SceneSystem::SceneManager::LoadEmptyScene()
{
	UnloadCurrentScene();

	m_currentScene = new Scene();

	SceneLoadEvent.Invoke();
}

void Core::SceneSystem::SceneManager::LoadEmptyLightedScene()
{
	UnloadCurrentScene();

	m_currentScene = new Scene();

	SceneLoadEvent.Invoke();

	auto& directionalLight = m_currentScene->CreateActor("Directional Light");
	directionalLight.AddComponent<ECS::Components::CDirectionalLight>().SetIntensity(0.75f);
	directionalLight.transform.SetLocalPosition({ 0.0f, 10.0f, 0.0f });
	directionalLight.transform.SetLocalRotation(Maths::FQuaternion({ 120.0f, -40.0f, 0.0f }));

	auto& ambientLight = m_currentScene->CreateActor("Ambient Light");
	ambientLight.AddComponent<ECS::Components::CAmbientSphereLight>().SetRadius(10000.0f);

	auto& camera = m_currentScene->CreateActor("Main Camera");
	camera.AddComponent<ECS::Components::CCamera>();
	camera.transform.SetLocalPosition({ 0.0f, 3.0f, 8.0f });
	camera.transform.SetLocalRotation(Maths::FQuaternion({ 20.0f, 180.0f, 0.0f }));

    auto& cube=m_currentScene->CreateActor("Cube");
    //m_context.editorResources->GetModel("Arrow_Translate");
   
    auto model=Core::Global::ServiceLocator::Get<Core::ResourceManagement::ModelManager>().GetResource(":Models/Cube.fbx"); 
    auto material= Core::Global::ServiceLocator::Get<Core::ResourceManagement::MaterialManager>().GetResource(":Materials\\Default.mat");
    cube.AddComponent<ECS::Components::CModelRenderer>().SetModel(model);
    if(material)
    cube.AddComponent<ECS::Components::CMaterialRenderer>().FillWithMaterial(*material);
}

bool Core::SceneSystem::SceneManager::LoadScene(const std::string& p_path, bool p_absolute)
{
	std::string completePath = (p_absolute ? "" : m_sceneRootFolder) + p_path;

	tinyxml2::XMLDocument doc;
	doc.LoadFile(completePath.c_str());

	if (LoadSceneFromMemory(doc))
	{
		StoreCurrentSceneSourcePath(completePath);
		return true;
	}

	return false;
}

bool Core::SceneSystem::SceneManager::LoadSceneFromMemory(tinyxml2::XMLDocument& p_doc)
{
	if (!p_doc.Error())
	{
		tinyxml2::XMLNode* root = p_doc.FirstChild();
		if (root)
		{
			tinyxml2::XMLNode* sceneNode = root->FirstChildElement("scene");
			if (sceneNode)
			{
				LoadEmptyScene();
				m_currentScene->OnDeserialize(p_doc, sceneNode);
				return true;
			}
		}
	}
	return false;
}

void Core::SceneSystem::SceneManager::UnloadCurrentScene()
{
	if (m_currentScene)
	{
		delete m_currentScene;
		m_currentScene = nullptr;
		SceneUnloadEvent.Invoke();
	}

	ForgetCurrentSceneSourcePath();
}

bool Core::SceneSystem::SceneManager::HasCurrentScene() const
{
	return m_currentScene;
}

Core::SceneSystem::Scene* Core::SceneSystem::SceneManager::GetCurrentScene()
{
	return m_currentScene;
}

std::string Core::SceneSystem::SceneManager::GetCurrentSceneSourcePath() const
{
	return m_currentSceneSourcePath;
}

bool Core::SceneSystem::SceneManager::IsCurrentSceneLoadedFromDisk() const
{
	return m_currentSceneLoadedFromPath;
}

void Core::SceneSystem::SceneManager::StoreCurrentSceneSourcePath(const std::string& p_path)
{
	m_currentSceneSourcePath = p_path;
	m_currentSceneLoadedFromPath = true;
	CurrentSceneSourcePathChangedEvent.Invoke(m_currentSceneSourcePath);
}

void Core::SceneSystem::SceneManager::ForgetCurrentSceneSourcePath()
{
	m_currentSceneSourcePath = "";
	m_currentSceneLoadedFromPath = false;
	CurrentSceneSourcePathChangedEvent.Invoke(m_currentSceneSourcePath);
}
