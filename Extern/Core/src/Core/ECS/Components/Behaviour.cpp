#include <Debug/Logger.h>
#include "Core/ECS/Actor.h"
#include "Core/ECS/Components/Behaviour.h"
#include "Core/Scripting/LuaBinder.h"

Tools::Eventing::Event<Core::ECS::Components::Behaviour*> Core::ECS::Components::Behaviour::CreatedEvent;
Tools::Eventing::Event<Core::ECS::Components::Behaviour*> Core::ECS::Components::Behaviour::DestroyedEvent;

Core::ECS::Components::Behaviour::Behaviour(ECS::Actor& p_owner, const std::string& p_name) :
	name(p_name), AComponent(p_owner)
{
	CreatedEvent.Invoke(this);
}

Core::ECS::Components::Behaviour::~Behaviour()
{
	DestroyedEvent.Invoke(this);
}

std::string Core::ECS::Components::Behaviour::GetName()
{
	return "Behaviour";
}

bool Core::ECS::Components::Behaviour::RegisterToLuaContext(sol::state& p_luaState, const std::string& p_scriptFolder)
{
	using namespace Core::Scripting;

	auto result = p_luaState.safe_script_file(p_scriptFolder + name + ".lua", &sol::script_pass_on_error);

	if (!result.valid())
	{
		sol::error err = result;
		OVLOG_ERROR(err.what());
		return false;
	}
	else
	{
		if (result.return_count() == 1 && result[0].is<sol::table>())
		{
			m_object = result[0];
			m_object["owner"] = &owner;
			return true;
		}
		else
		{
			OVLOG_ERROR("'" + name + ".lua' missing return expression");
			return false;
		}
	}
}

void Core::ECS::Components::Behaviour::UnregisterFromLuaContext()
{
	m_object = sol::nil;
}

sol::table& Core::ECS::Components::Behaviour::GetTable()
{
	return m_object;
}

void Core::ECS::Components::Behaviour::OnAwake()
{
	LuaCall("OnAwake");
}

void Core::ECS::Components::Behaviour::OnStart()
{
	LuaCall("OnStart");
}

void Core::ECS::Components::Behaviour::OnEnable()
{
	LuaCall("OnEnable");
}

void Core::ECS::Components::Behaviour::OnDisable()
{
	LuaCall("OnDisable");
}

void Core::ECS::Components::Behaviour::OnDestroy()
{
	LuaCall("OnEnd"); // Retro-compatibility
	LuaCall("OnDestroy");
}

void Core::ECS::Components::Behaviour::OnUpdate(float p_deltaTime)
{
	LuaCall("OnUpdate", p_deltaTime);
}

void Core::ECS::Components::Behaviour::OnFixedUpdate(float p_deltaTime)
{
	LuaCall("OnFixedUpdate", p_deltaTime);
}

void Core::ECS::Components::Behaviour::OnLateUpdate(float p_deltaTime)
{
	LuaCall("OnLateUpdate", p_deltaTime);
}

void Core::ECS::Components::Behaviour::OnCollisionEnter(Components::CPhysicalObject& p_otherObject)
{
	LuaCall("OnCollisionStart", p_otherObject); // Retro-compatibility
	LuaCall("OnCollisionEnter", p_otherObject);
}

void Core::ECS::Components::Behaviour::OnCollisionStay(Components::CPhysicalObject& p_otherObject)
{
	LuaCall("OnCollisionStay", p_otherObject);
}

void Core::ECS::Components::Behaviour::OnCollisionExit(Components::CPhysicalObject& p_otherObject)
{
	LuaCall("OnCollisionStop", p_otherObject); // Retro-compatibility
	LuaCall("OnCollisionExit", p_otherObject);
}

void Core::ECS::Components::Behaviour::OnTriggerEnter(Components::CPhysicalObject& p_otherObject)
{
	LuaCall("OnTriggerStart", p_otherObject); // Retro-compatibility
	LuaCall("OnTriggerEnter", p_otherObject);
}

void Core::ECS::Components::Behaviour::OnTriggerStay(Components::CPhysicalObject& p_otherObject)
{
	LuaCall("OnTriggerStay", p_otherObject);
}

void Core::ECS::Components::Behaviour::OnTriggerExit(Components::CPhysicalObject& p_otherObject)
{
	LuaCall("OnTriggerStop", p_otherObject); // Retro-compatibility
	LuaCall("OnTriggerExit", p_otherObject);
}

void Core::ECS::Components::Behaviour::OnSerialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
}

void Core::ECS::Components::Behaviour::OnDeserialize(tinyxml2::XMLDocument & p_doc, tinyxml2::XMLNode * p_node)
{
}


