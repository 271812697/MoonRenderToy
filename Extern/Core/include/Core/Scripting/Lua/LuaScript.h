/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <memory>

#include <Core/Scripting/Common/TScript.h>

#include <sol/sol.hpp>

namespace Core::ECS
{
	class Actor;
}

namespace Core::Scripting
{
	/**
	* Lua script context
	*/
	struct LuaScriptContext
	{
		sol::table table;
	};

	using LuaScriptBase = TScript<EScriptingLanguage::LUA, LuaScriptContext>;

	/**
	* Lua script implementation
	*/
	class LuaScript : public LuaScriptBase
	{
	public:
		/**
		* Constructor of the Lua script
		* @param p_table
		*/
		LuaScript(sol::table p_table);

		/**
		* Sets the owner of the script
		* @param p_owner
		*/
		void SetOwner(Core::ECS::Actor& p_owner);
	};
}
