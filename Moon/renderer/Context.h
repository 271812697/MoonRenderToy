
#pragma once

#include"tools/IniFile.h"

#include <Rendering/Buffers/UniformBuffer.h>
#include <Rendering/Buffers/ShaderStorageBuffer.h>
#include <Rendering/Core/ShapeDrawer.h>

#include <Physics/Core/PhysicsEngine.h>





#include <Core/ECS/Renderer.h>
#include <Core/ResourceManagement/ModelManager.h>
#include <Core/ResourceManagement/TextureManager.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ResourceManagement/SoundManager.h>
#include <Core/SceneSystem/SceneManager.h>
#include <Core/Scripting/ScriptInterpreter.h>


#include <Audio/Core/AudioEngine.h>
#include <Audio/Core/AudioPlayer.h>

#include "EditorResources.h"

namespace Editor::Core
{
	/**
	* The Context handle the engine features setup
	*/
	class Context
	{
	public:
		/**
		* Constructor
		* @param p_projectPath
		* @param p_projectName
		*/
		Context(const std::string& p_projectPath, const std::string& p_projectName);

		/**
		* Destructor
		*/
		~Context();

		/**
		* Reset project settings ini file
		*/
		void ResetProjectSettings();

		/**
		* Verify that project settings are complete (No missing key).
		* Returns true if the integrity is verified
		*/
		bool IsProjectSettingsIntegrityVerified();

		/**
		* Apply project settings to the ini file
		*/
		void ApplyProjectSettings();

	public:
		const std::string projectPath;
		const std::string projectName;
		const std::string projectFilePath;
		const std::string engineAssetsPath;
		const std::string projectAssetsPath;
		const std::string projectScriptsPath;
		const std::string editorAssetsPath;


		std::unique_ptr<Rendering::Context::Driver>			    driver;
		std::unique_ptr<::Core::ECS::Renderer>					renderer;
		std::unique_ptr<Rendering::Core::ShapeDrawer>			shapeDrawer;

		std::unique_ptr<Physics::Core::PhysicsEngine>			physicsEngine;
		std::unique_ptr<Audio::Core::AudioEngine>				audioEngine;
		std::unique_ptr<Audio::Core::AudioPlayer>				audioPlayer;
		std::unique_ptr<::Editor::Core::EditorResources>		editorResources;

		std::unique_ptr<::Core::Scripting::ScriptInterpreter>	scriptInterpreter;
		std::unique_ptr<Rendering::Buffers::UniformBuffer>	    engineUBO;

		std::unique_ptr<Rendering::Buffers::ShaderStorageBuffer>	lightSSBO;
		std::unique_ptr<Rendering::Buffers::ShaderStorageBuffer>	simulatedLightSSBO;
		
		::Core::SceneSystem::SceneManager sceneManager;

		::Core::ResourceManagement::ModelManager	modelManager;
		::Core::ResourceManagement::TextureManager	textureManager;
		::Core::ResourceManagement::ShaderManager	shaderManager;
		::Core::ResourceManagement::MaterialManager	materialManager;
		::Core::ResourceManagement::SoundManager	soundManager;


		Tools::Filesystem::IniFile projectSettings;
	};
}
