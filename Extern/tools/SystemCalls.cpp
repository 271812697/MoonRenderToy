#include "PathParser.h"
#include"SystemCallls.h"

#include <Windows.h>

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
	size_t origsize = p_url.size() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t* wcstring = (wchar_t*)malloc(sizeof(wchar_t)*(p_url.length()-1));
	mbstowcs_s(&convertedChars,wcstring,origsize,p_url.c_str(),_TRUNCATE);
	//ShellExecute(0, 0, wcstring, 0, 0, SW_SHOW);
}
