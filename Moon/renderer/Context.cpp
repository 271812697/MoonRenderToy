#include <filesystem>
#include <Rendering/Entities/Light.h>
#include <Core/Global/ServiceLocator.h>
#include <Tools/Utils/SystemCalls.h>
#include <Debug/Assertion.h>
#include <Core/Scripting/ScriptEngine.h>

#include "Context.h"

using namespace Core::Global;
using namespace Core::ResourceManagement;

constexpr std::array<std::pair<int, int>, 13> kResolutions
{
	std::make_pair(640, 360), // nHD
	std::make_pair(854, 480), // FWVGA
	std::make_pair(960, 540), // qHD
	std::make_pair(1024, 576), // WSVGA
	std::make_pair(1280, 720), // HD
	std::make_pair(1366, 768), // FWXGA
	std::make_pair(1600, 900), // HD+
	std::make_pair(1920, 1080), // Full HD
	std::make_pair(2560, 1440), // QHD
	std::make_pair(3200, 1800), // QHD+
	std::make_pair(3840, 2160), // 4K UHD
	std::make_pair(5120, 2880), // 5K
	std::make_pair(7680, 4320), // 8K UHD
};

std::array<int, 4> FindBestFitWindowSizeAndPosition(std::array<int, 4> p_workAreaSize)
{
	// Extract work area dimensions
	int workAreaX = p_workAreaSize[0];
	int workAreaY = p_workAreaSize[1];
	int workAreaWidth = p_workAreaSize[2];
	int workAreaHeight = p_workAreaSize[3];

	// Iterate over available resolutions
	for (auto it = kResolutions.rbegin(); it != kResolutions.rend(); ++it)
	{
		int width = it->first;
		int height = it->second;

		// Check if resolution fits within work area
		if (width <= workAreaWidth && height <= workAreaHeight)
		{
			// Center the resolution within the work area
			int posX = workAreaX + workAreaWidth / 2 - width / 2;
			int posY = workAreaY + workAreaHeight / 2 - height / 2;

			return { posX, posY, width, height };
		}
	}

	assert(false, "No resolution found to fit the work area");
	return {};
}

Editor::Core::Context::Context(const std::string& p_projectPath, const std::string& p_projectName) :
	projectPath(p_projectPath),
	projectName(p_projectName),
	projectFilePath(p_projectPath + p_projectName + ".ovproject"),
	engineAssetsPath(PROJECT_ENGINE_PATH),
	projectAssetsPath(p_projectPath + "Assets\\"),
	projectScriptsPath(p_projectPath + "Scripts\\"),
	editorAssetsPath(PROJECT_EDITOR_PATH),
	sceneManager(projectAssetsPath),
	projectSettings(projectFilePath)
{

	ModelManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	TextureManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	ShaderManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	MaterialManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	SoundManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);




	/* Graphics context creation */
	driver = std::make_unique<Rendering::Context::Driver>(Rendering::Settings::DriverSettings{ true });
	//textureRegistry = std::make_unique<Editor::Utils::TextureRegistry>();

	std::filesystem::create_directories(Tools::Utils::SystemCalls::GetPathToAppdata() + "\\erloadTech\\Editor\\");




	/* Audio */
	//audioEngine = std::make_unique<Audio::Core::AudioEngine>();

	/* Editor resources */
	editorResources = std::make_unique<Editor::Core::EditorResources>(editorAssetsPath);

	/* Physics engine */
	physicsEngine = std::make_unique<Physics::Core::PhysicsEngine>(Physics::Settings::PhysicsSettings{ {0.0f, -9.81f, 0.0f } });

	/* Scripting */
	scriptEngine = std::make_unique<::Core::Scripting::ScriptEngine>();
	scriptEngine->SetScriptRootFolder(projectScriptsPath);

	/* Service Locator providing */
	ServiceLocator::Provide<Physics::Core::PhysicsEngine>(*physicsEngine);
	ServiceLocator::Provide<ModelManager>(modelManager);
	ServiceLocator::Provide<TextureManager>(textureManager);
	ServiceLocator::Provide<ShaderManager>(shaderManager);
	ServiceLocator::Provide<MaterialManager>(materialManager);
	ServiceLocator::Provide<SoundManager>(soundManager);

	ServiceLocator::Provide<::Core::SceneSystem::SceneManager>(sceneManager);
	ServiceLocator::Provide<Audio::Core::AudioEngine>(*audioEngine);
	ServiceLocator::Provide<::Core::Scripting::ScriptEngine>(*scriptEngine);
	ServiceLocator::Provide<::Editor::Core::Context>(*this);

}

Editor::Core::Context::~Context()
{
	modelManager.UnloadResources();
	textureManager.UnloadResources();
	shaderManager.UnloadResources();
	materialManager.UnloadResources();
	soundManager.UnloadResources();
}

void Editor::Core::Context::ResetProjectSettings()
{

}

bool Editor::Core::Context::IsProjectSettingsIntegrityVerified()
{

	return false;
}

void Editor::Core::Context::ApplyProjectSettings()
{

}
