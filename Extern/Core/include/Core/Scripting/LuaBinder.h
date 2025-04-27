

#pragma once

#include <sol.hpp>

namespace Core::Scripting
{
	/**
	* Handle lua binding
	*/
	class LuaBinder
	{
	public:
		/**
		* Bind engine features to lua
		*/
		static void CallBinders(sol::state& p_luaState);
	};
}