

#pragma once

#include <sol.hpp>

namespace Core::Scripting
{
	/**
	* Handle lua maths binding
	*/
	class LuaMathsBinder
	{
	public:
		/**
		* Bind engine maths-related features to lua
		*/
		static void BindMaths(sol::state& p_luaState);
	};
}