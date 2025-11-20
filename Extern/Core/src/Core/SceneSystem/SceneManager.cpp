#include <tinyxml2.h>
#include <filesystem>

#include "Core/SceneSystem/SceneManager.h"
#include "Core/ECS/Components/CDirectionalLight.h"
#include "Core/ECS/Components/CAmbientSphereLight.h"
#include "Core/ECS/Components/CCamera.h"

Core::SceneSystem::SceneManager::SceneManager(const std::string& p_sceneRootFolder) : m_sceneRootFolder(p_sceneRootFolder)
{
	LoadEmptyScene();
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
	m_currentScene.reset(new Scene());
	SceneLoadEvent.Invoke();
}

void Core::SceneSystem::SceneManager::LoadDefaultScene()
{
	UnloadCurrentScene();
	m_currentScene.reset(new Scene());
	m_currentScene->AddDefaultCamera();
	m_currentScene->AddDefaultLights();

	//m_currentScene->AddDefaultReflections();
	//m_currentScene->AddDefaultAtmosphere();

	m_currentScene->AddDefaultSkysphere();
	m_currentScene->AddDefaultPostProcessStack();
	SceneLoadEvent.Invoke();
}

bool Core::SceneSystem::SceneManager::LoadScene(const std::string& p_path, bool p_absolute)
{
	std::filesystem::path path =
		p_absolute ?
		std::filesystem::current_path() :
		std::filesystem::path{ m_sceneRootFolder };

	path /= p_path;

	tinyxml2::XMLDocument doc;
	doc.LoadFile(path.string().c_str());

	if (LoadSceneFromMemory(doc))
	{
		StoreCurrentSceneSourcePath(path.string());
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
		m_currentScene.reset();
		SceneUnloadEvent.Invoke();
	}

	ForgetCurrentSceneSourcePath();
}

bool Core::SceneSystem::SceneManager::HasCurrentScene() const
{
	return m_currentScene != nullptr;
}

Core::SceneSystem::Scene* Core::SceneSystem::SceneManager::GetCurrentScene() const
{
	return m_currentScene.get();
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
