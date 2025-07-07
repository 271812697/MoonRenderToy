#pragma once
namespace UI::Plugins
{
	class IPlugin
	{
	public:
		virtual void Execute() = 0;
		void* userData = nullptr;
	};
}