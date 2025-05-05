/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Debug/Logger.h>
#include <Debug/Assertion.h>

#include <Core/ECS/Actor.h>
#include <Core/Scripting/Lua/LuaScript.h>

template<>
Core::Scripting::LuaScriptBase::TScript() = default;

template<>
Core::Scripting::LuaScriptBase::~TScript() = default;

template<>
bool Core::Scripting::LuaScriptBase::IsValid() const
{
	return m_context.table.valid();
}

Core::Scripting::LuaScript::LuaScript(sol::table table)
{
	m_context.table = table;
}

void Core::Scripting::LuaScript::SetOwner(Core::ECS::Actor& p_owner)
{
	m_context.table["owner"] = &p_owner;
}
