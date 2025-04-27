

#include <Debug/Logger.h>

#include "Core/Scripting/LuaBinder.h"
#include "Core/Scripting/ScriptInterpreter.h"

Core::Scripting::ScriptInterpreter::ScriptInterpreter(const std::string& p_scriptRootFolder) :
	m_scriptRootFolder(p_scriptRootFolder)
{
	CreateLuaContextAndBindGlobals();

	/* Listen to behaviours */
	Core::ECS::Components::Behaviour::CreatedEvent	+= std::bind(&ScriptInterpreter::Consider, this, std::placeholders::_1);
	Core::ECS::Components::Behaviour::DestroyedEvent	+= std::bind(&ScriptInterpreter::Unconsider, this, std::placeholders::_1);
}

Core::Scripting::ScriptInterpreter::~ScriptInterpreter()
{
	DestroyLuaContext();
}

void Core::Scripting::ScriptInterpreter::CreateLuaContextAndBindGlobals()
{
	if (!m_luaState)
	{
		m_luaState = std::make_unique<sol::state>();
		m_luaState->open_libraries(sol::lib::base, sol::lib::math);
		Core::Scripting::LuaBinder::CallBinders(*m_luaState);
		m_isOk = true;

		std::for_each(m_behaviours.begin(), m_behaviours.end(), [this](ECS::Components::Behaviour* behaviour)
		{
			if (!behaviour->RegisterToLuaContext(*m_luaState, m_scriptRootFolder))
				m_isOk = false;
		});

		if (!m_isOk)
			OVLOG_ERROR("Script interpreter failed to register scripts. Check your lua scripts");
	}
}

void Core::Scripting::ScriptInterpreter::DestroyLuaContext()
{
	if (m_luaState)
	{
		std::for_each(m_behaviours.begin(), m_behaviours.end(), [this](ECS::Components::Behaviour* behaviour)
		{
			behaviour->UnregisterFromLuaContext();
		});

		m_luaState.reset();
		m_isOk = false;
	}
}

void Core::Scripting::ScriptInterpreter::Consider(Core::ECS::Components::Behaviour* p_toConsider)
{
	if (m_luaState)
	{
		m_behaviours.push_back(p_toConsider);

		if (!p_toConsider->RegisterToLuaContext(*m_luaState, m_scriptRootFolder))
			m_isOk = false;
	}
}

void Core::Scripting::ScriptInterpreter::Unconsider(Core::ECS::Components::Behaviour* p_toUnconsider)
{
	if (m_luaState)
		p_toUnconsider->UnregisterFromLuaContext();

	m_behaviours.erase(std::remove_if(m_behaviours.begin(), m_behaviours.end(), [p_toUnconsider](ECS::Components::Behaviour* behaviour)
	{
		return p_toUnconsider == behaviour;
	}));

	RefreshAll(); // Unconsidering a script is impossible with Lua, we have to reparse every behaviours
}

void Core::Scripting::ScriptInterpreter::RefreshAll()
{
	DestroyLuaContext();
	CreateLuaContextAndBindGlobals();
}

bool Core::Scripting::ScriptInterpreter::IsOk() const
{
	return m_isOk;
}
