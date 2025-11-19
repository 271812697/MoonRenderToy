#pragma once
#include <filesystem>

#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ResourceManagement/ModelManager.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Core/ResourceManagement/TextureManager.h>
#include <Core/SceneSystem/SceneManager.h>
#include "EditorResources.h"
#include <Rendering/HAL/UniformBuffer.h>
#include <Rendering/HAL/ShaderStorageBuffer.h>
#include <Tools/Filesystem/IniFile.h>

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
		* @param p_projectFolder (including the .project file)
		*/
		Context(const std::string& p_projectPath, const std::string& p_projectName);

		/**
		* Destructor
		*/
		virtual ~Context();

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

		std::unique_ptr<::Rendering::Context::Driver> driver;

		std::unique_ptr<Editor::Core::EditorResources> editorResources;



		::Core::SceneSystem::SceneManager sceneManager;

		::Core::ResourceManagement::ModelManager modelManager;
		::Core::ResourceManagement::TextureManager textureManager;
		::Core::ResourceManagement::ShaderManager shaderManager;
		::Core::ResourceManagement::MaterialManager materialManager;

		Tools::Filesystem::IniFile projectSettings;
	};
}
