#include <filesystem>
#include <Rendering/Entities/Light.h>
#include <Core/Global/ServiceLocator.h>
#include "Context.h"
using namespace Core::Global;
using namespace Core::ResourceManagement;

Editor::Core::Context::Context(const std::string& p_projectPath, const std::string& p_projectName) :
	projectPath(p_projectPath),
	projectName(p_projectName),
	projectFilePath(p_projectPath + p_projectName + ".project"),
	engineAssetsPath(PROJECT_ENGINE_PATH),
	projectAssetsPath(engineAssetsPath + "/"),
	//engineAssetsPath( "\\"),
	//projectAssetsPath(p_projectPath + "Assets"),
	projectScriptsPath(p_projectPath + "Scripts\\"),
	editorAssetsPath(PROJECT_EDITOR_PATH),
	sceneManager(projectAssetsPath),
	projectSettings(projectFilePath)
{
	;

	if (!IsProjectSettingsIntegrityVerified())
	{
		ResetProjectSettings();
		projectSettings.Rewrite();
	}

	ModelManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	TextureManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	ShaderManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	MaterialManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);
	SoundManager::ProvideAssetPaths(projectAssetsPath, engineAssetsPath);


	/* Graphics context creation */
	driver = std::make_unique<Rendering::Context::Driver>(Rendering::Settings::DriverSettings{ false });
	renderer = std::make_unique<::Core::ECS::Renderer>(*driver);
	renderer->SetCapability(Rendering::Settings::ERenderingCapability::MULTISAMPLE, true);
	shapeDrawer = std::make_unique<Rendering::Core::ShapeDrawer>(*renderer);

	std::filesystem::create_directories(std::string(getenv("APPDATA")) + "\\erloadTech\\Editor\\");



	/* Audio */
	audioEngine = std::make_unique<Audio::Core::AudioEngine>(projectAssetsPath);
	audioPlayer = std::make_unique<Audio::Core::AudioPlayer>(*audioEngine);

	/* Editor resources */
	editorResources = std::make_unique<Editor::Core::EditorResources>(editorAssetsPath);

	/* Physics engine */
	physicsEngine = std::make_unique<Physics::Core::PhysicsEngine>(Physics::Settings::PhysicsSettings{ {0.0f, -9.81f, 0.0f } });

	/* Service Locator providing */
	ServiceLocator::Provide<Context>(*this);
	ServiceLocator::Provide<Physics::Core::PhysicsEngine>(*physicsEngine);
	ServiceLocator::Provide<ModelManager>(modelManager);

	ServiceLocator::Provide<TextureManager>(textureManager);
	ServiceLocator::Provide<ShaderManager>(shaderManager);
	ServiceLocator::Provide<MaterialManager>(materialManager);
	ServiceLocator::Provide<SoundManager>(soundManager);

	ServiceLocator::Provide<::Core::SceneSystem::SceneManager>(sceneManager);
	ServiceLocator::Provide<Audio::Core::AudioEngine>(*audioEngine);
	ServiceLocator::Provide<Audio::Core::AudioPlayer>(*audioPlayer);

	/* Scripting */
	scriptInterpreter = std::make_unique<::Core::Scripting::ScriptInterpreter>(projectScriptsPath);

	engineUBO = std::make_unique<Rendering::Buffers::UniformBuffer>
		(
			/* UBO Data Layout */
			sizeof(Maths::FMatrix4) +
			sizeof(Maths::FMatrix4) +
			sizeof(Maths::FMatrix4) +
			sizeof(Maths::FVector3) +
			sizeof(float) +
			sizeof(Maths::FMatrix4),
			0, 0, Rendering::Buffers::EAccessSpecifier::STREAM_DRAW
		);

	lightSSBO = std::make_unique<Rendering::Buffers::ShaderStorageBuffer>(Rendering::Buffers::EAccessSpecifier::STREAM_DRAW);
	simulatedLightSSBO = std::make_unique<Rendering::Buffers::ShaderStorageBuffer>(Rendering::Buffers::EAccessSpecifier::STREAM_DRAW); // Used in Asset View

	std::vector<Maths::FMatrix4> simulatedLights;

	Maths::FTransform simulatedLightTransform;
	simulatedLightTransform.SetLocalRotation(Maths::FQuaternion({ 45.f, 180.f, 10.f }));

	Rendering::Entities::Light simulatedDirectionalLight(simulatedLightTransform, Rendering::Entities::Light::Type::DIRECTIONAL);
	simulatedDirectionalLight.color = { 1.f, 1.f, 1.f };
	simulatedDirectionalLight.intensity = 1.f;

	Rendering::Entities::Light simulatedAmbientLight(simulatedLightTransform, Rendering::Entities::Light::Type::AMBIENT_SPHERE);
	simulatedAmbientLight.color = { 0.07f, 0.07f, 0.07f };
	simulatedAmbientLight.intensity = 1.f;
	simulatedAmbientLight.constant = 1000.0f;

	simulatedLights.push_back(simulatedDirectionalLight.GenerateMatrix());
	simulatedLights.push_back(simulatedAmbientLight.GenerateMatrix());

	simulatedLightSSBO->SendBlocks<Maths::FMatrix4>(simulatedLights.data(), simulatedLights.size() * sizeof(Maths::FMatrix4));

	ApplyProjectSettings();
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
	projectSettings.RemoveAll();
	projectSettings.Add<float>("gravity", -9.81f);
	projectSettings.Add<int>("x_resolution", 1280);
	projectSettings.Add<int>("y_resolution", 720);
	projectSettings.Add<bool>("fullscreen", false);
	projectSettings.Add<std::string>("executable_name", "Game");
	projectSettings.Add<std::string>("start_scene", "Scene.scene");
	projectSettings.Add<bool>("vsync", true);
	projectSettings.Add<bool>("multisampling", true);
	projectSettings.Add<int>("samples", 4);
	projectSettings.Add<int>("opengl_major", 4);
	projectSettings.Add<int>("opengl_minor", 3);
	projectSettings.Add<bool>("dev_build", true);
}

bool Editor::Core::Context::IsProjectSettingsIntegrityVerified()
{
	return
		projectSettings.IsKeyExisting("gravity") &&
		projectSettings.IsKeyExisting("x_resolution") &&
		projectSettings.IsKeyExisting("y_resolution") &&
		projectSettings.IsKeyExisting("fullscreen") &&
		projectSettings.IsKeyExisting("executable_name") &&
		projectSettings.IsKeyExisting("start_scene") &&
		projectSettings.IsKeyExisting("vsync") &&
		projectSettings.IsKeyExisting("multisampling") &&
		projectSettings.IsKeyExisting("samples") &&
		projectSettings.IsKeyExisting("opengl_major") &&
		projectSettings.IsKeyExisting("opengl_minor") &&
		projectSettings.IsKeyExisting("dev_build");
}

void Editor::Core::Context::ApplyProjectSettings()
{
	physicsEngine->SetGravity({ 0.0f, projectSettings.Get<float>("gravity"), 0.0f });
}
