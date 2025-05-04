/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <vector>
#include <memory>

#include <Core/Scripting/Common/TScriptEngine.h>

#include <sol/sol.hpp>

namespace Core::ECS::Components
{
	class Behaviour;
}

namespace Core::Scripting
{
	/**
	* Lua script engine context
	*/
	struct LuaScriptEngineContext
	{
		std::unique_ptr<sol::state> luaState;
		std::string scriptRootFolder;
		std::vector<std::reference_wrapper<Core::ECS::Components::Behaviour>> behaviours;
		uint32_t errorCount;
	};

	using LuaScriptEngineBase = TScriptEngine<EScriptingLanguage::LUA, LuaScriptEngineContext>;

	/**
	* Lua script engine implementation
	*/
	class LuaScriptEngine : public LuaScriptEngineBase
	{
	public:
		/**
		* Constructor of the Lua script engine
		*/
		LuaScriptEngine();

		/**
		* Destructor of the Lua script engine
		*/
		~LuaScriptEngine();

		/**
		* Create the Lua state
		*/
		void CreateContext();

		/**
		* Destroy the lua state
		*/
		void DestroyContext();
	};
}
