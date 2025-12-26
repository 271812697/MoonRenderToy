#include <algorithm>
#include <Tools/Utils/PathParser.h>

std::string Tools::Utils::PathParser::MakeWindowsStyle(const std::string & p_path)
{
	std::string result;
	result.resize(p_path.size());

	for (size_t i = 0; i < p_path.size(); ++i)
		result[i] = p_path[i] == '/' ? '\\' : p_path[i];

	return result;
}

std::string Tools::Utils::PathParser::MakeNonWindowsStyle(const std::string & p_path)
{
	std::string result;
	result.resize(p_path.size());

	for (size_t i = 0; i < p_path.size(); ++i)
		result[i] = p_path[i] == '\\' ? '/' : p_path[i];

	return result;
}

std::string Tools::Utils::PathParser::GetContainingFolder(const std::string & p_path)
{
	std::string result;

	bool extraction = false;

	for (auto it = p_path.rbegin(); it != p_path.rend(); ++it)
	{
		if (extraction)
			result += *it;

		if (!extraction && it != p_path.rbegin() && (*it == '\\' || *it == '/'))
			extraction = true;
	}

	std::reverse(result.begin(), result.end());

	if (!result.empty() && result.back() != '\\')
		result += '\\';

	return result;
}

std::string Tools::Utils::PathParser::GetElementName(const std::string & p_path)
{
	std::string result;

	std::string path = p_path;
	if (!path.empty() && path.back() == '\\')
		path.pop_back();

	for (auto it = path.rbegin(); it != path.rend() && *it != '\\' && *it != '/'; ++it)
		result += *it;

	std::reverse(result.begin(), result.end());

	return result;
}

std::string Tools::Utils::PathParser::GetExtension(const std::string & p_path)
{
	std::string result;

	for (auto it = p_path.rbegin(); it != p_path.rend() && *it != '.'; ++it)
		result += *it;

	std::reverse(result.begin(), result.end());

	return result;
}

std::string Tools::Utils::PathParser::FileTypeToString(EFileType p_fileType)
{
	switch (p_fileType)
	{
	case Tools::Utils::PathParser::EFileType::MODEL:		return "Model";
	case Tools::Utils::PathParser::EFileType::TEXTURE:	return "Texture";
	case Tools::Utils::PathParser::EFileType::SHADER:		return "Shader";
	case Tools::Utils::PathParser::EFileType::SHADER_PART:return "Shader_Part";
	case Tools::Utils::PathParser::EFileType::MATERIAL:	return "Material";
	case Tools::Utils::PathParser::EFileType::SOUND:		return "Sound";
	case Tools::Utils::PathParser::EFileType::SCENE:		return "Scene";
	case Tools::Utils::PathParser::EFileType::SCRIPT:		return "Script";
	case Tools::Utils::PathParser::EFileType::FONT:		return "Font";
	}

	return "Unknown";
}

Tools::Utils::PathParser::EFileType Tools::Utils::PathParser::GetFileType(const std::string & p_path)
{

	return EFileType::UNKNOWN;
}
