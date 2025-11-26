#include <filesystem>
#include <Core/Global/ServiceLocator.h>
#include "Context.h"
#include <Rendering/Entities/Light.h>
#include <Tools/Utils/SystemCalls.h>
#include <assert.h>

using namespace Core::Global;
using namespace ::Core::ResourceManagement;


Editor::Core::Context::Context(const std::string& p_projectPath, const std::string& p_projectName) :
	projectPath(p_projectPath),
	projectName(p_projectName),
	projectFilePath(p_projectPath + p_projectName + ".ovproject"),
	engineAssetsPath(std::string(PROJECT_ENGINE_PATH) + std::string("\\")),
	projectAssetsPath(p_projectPath + "Assets\\"),
	projectScriptsPath(p_projectPath + "Scripts\\"),
	editorAssetsPath(PROJECT_EDITOR_PATH + std::string("\\")),
	sceneManager(projectAssetsPath),
	projectSettings(projectFilePath)
{

	ModelManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	TextureManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	ShaderManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	MaterialManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);




	/* Graphics context creation */
	driver = std::make_unique<::Rendering::Context::Driver>(::Rendering::Settings::DriverSettings{ true });



	/* Editor resources */
	editorResources = std::make_unique<Editor::Core::EditorResources>(editorAssetsPath);


	/* Service Locator providing */

	ServiceLocator::Provide<ModelManager>(modelManager);
	ServiceLocator::Provide<TextureManager>(textureManager);
	ServiceLocator::Provide<ShaderManager>(shaderManager);
	ServiceLocator::Provide<MaterialManager>(materialManager);

	ServiceLocator::Provide<::Core::SceneSystem::SceneManager>(sceneManager);


	ServiceLocator::Provide<Editor::Core::Context>(*this);
}

Editor::Core::Context::~Context()
{
	modelManager.UnloadResources();
	textureManager.UnloadResources();
	shaderManager.UnloadResources();
	materialManager.UnloadResources();

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
