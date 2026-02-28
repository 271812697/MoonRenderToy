#pragma once
#include <string>
namespace Tools::Utils
{
	/**
	* Provide some tools to get information about a given path
	*/
	class PathParser
	{
	public:
		enum class EFileType
		{
			UNKNOWN,
			MODEL,
			TEXTURE,
			SHADER,
			SHADER_PART,
			MATERIAL,
			SOUND,
			SCENE,
			SCRIPT,
			FONT
		};

		PathParser() = delete;
		static std::string MakeWindowsStyle(const std::string& p_path);
		static std::string MakeNonWindowsStyle(const std::string& p_path);
		static std::string GetContainingFolder(const std::string& p_path);
		static std::string GetElementName(const std::string& p_path);
		static std::string GetExtension(const std::string& p_path);
		static std::string FileTypeToString(EFileType p_fileType);
		static EFileType GetFileType(const std::string& p_path);
		static std::string GetExeDirectory();
	};
}