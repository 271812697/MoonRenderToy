#pragma once
#include <functional>
#include <Rendering/Resources/Shader.h>

namespace Rendering::Resources::Loaders
{

	class ShaderLoader
	{
	public:
		struct LoggingSettings
		{
			bool summary : 1;
			bool linkingErrors : 1;
			bool linkingSuccess : 1;
			bool compilationErrors : 1;
			bool compilationSuccess : 1;
		};

		using FilePathParserCallback = std::function<std::string(const std::string&)>;

		ShaderLoader() = delete;

		static LoggingSettings GetLoggingSettings();

		static void SetLoggingSettings(LoggingSettings p_settings);

		static Shader* Create(const std::string& p_filePath, FilePathParserCallback p_pathParser = nullptr);
		static Shader* CreateFromSource(const std::string& p_vertexShader, const std::string& p_fragmentShader, const std::string& p_geomertyShader = "");
		static Shader* CreateFromSource(const std::string& p_sourceShader, FilePathParserCallback p_pathParser=nullptr);
		static void	Recompile(Shader& p_shader, const std::string& p_filePath, FilePathParserCallback p_pathParser = nullptr);
		static bool Destroy(Shader*& p_shader);
	};
}
