#include "Tools/Utils/PathParser.h"
#include "Tools/Utils/SystemCalls.h"

#include <Windows.h>
#include <ShlObj.h>
#include <memory>
#include <assert.h>

void Tools::Utils::SystemCalls::ShowInExplorer(const std::string & p_path)
{
	ShellExecuteA(NULL, "open", Tools::Utils::PathParser::MakeWindowsStyle(p_path).c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void Tools::Utils::SystemCalls::OpenFile(const std::string & p_file, const std::string & p_workingDir)
{
	ShellExecuteA(NULL, "open", Tools::Utils::PathParser::MakeWindowsStyle(p_file).c_str(), NULL,
		p_workingDir.empty() ? NULL : Tools::Utils::PathParser::MakeWindowsStyle(p_workingDir).c_str(),
		SW_SHOWNORMAL);
}

void Tools::Utils::SystemCalls::EditFile(const std::string & p_file)
{
	ShellExecuteW(NULL, NULL, std::wstring(p_file.begin(), p_file.end()).c_str(), NULL, NULL, SW_NORMAL);
}

void Tools::Utils::SystemCalls::OpenURL(const std::string& p_url)
{
	ShellExecute(0, 0, p_url.c_str(), 0, 0, SW_SHOW);
}

std::string Tools::Utils::SystemCalls::GetPathToAppdata()
{
	// Retrieve app-data path
	PWSTR rawPath = nullptr;
	const HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, nullptr, &rawPath);
	std::unique_ptr<wchar_t, decltype(&CoTaskMemFree)> path(rawPath, CoTaskMemFree);
	assert(SUCCEEDED(hr) && "Failed to get AppData path");
	
	// Convert app-data path from wide char to UTF-8 string
	const int size_needed = WideCharToMultiByte(CP_UTF8, 0, path.get(), -1, nullptr, 0, nullptr, nullptr);
	assert(size_needed > 0 && "failed to convert from wide char to UTF-8");
	std::string appDataPath(size_needed - 1, 0);
	WideCharToMultiByte(CP_UTF8, 0, path.get(), -1, &appDataPath[0], size_needed, nullptr, nullptr);
	return appDataPath;
}