/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#if defined(LUA_SCRIPTING)
#include <Core/Scripting/Lua/LuaScript.h>
#include <Core/Scripting/Lua/LuaScriptEngine.h>
#else
#include <Core/Scripting/Null/NullScript.h>
#include <Core/Scripting/Null/NullScriptEngine.h>
#endif

namespace Core::Scripting
{
#if defined(LUA_SCRIPTING)
	using Script = LuaScript;
	using ScriptEngine = LuaScriptEngine;
#else
	using Script = NullScript;
	using ScriptEngine = NullScriptEngine;
#endif
}
