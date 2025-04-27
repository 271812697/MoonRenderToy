
#pragma once

#include <sol.hpp>

namespace Core::Scripting
{
	/**
	* Handle actor binding
	*/
	class LuaActorBinder
	{
	public:
		/**
		* Bind engine actor-related features to lua
		*/
		static void BindActor(sol::state& p_luaState);
	};
}