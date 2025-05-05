/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/


#include <Debug/Logger.h>

#include <Core/ECS/Actor.h>
#include <Core/ECS/Components/Behaviour.h>
#include <Core/Global/ServiceLocator.h>
#include <Core/Scripting/ScriptEngine.h>

Core::ECS::Components::Behaviour::Behaviour(ECS::Actor& p_owner, const std::string& p_name) :
	name(p_name), AComponent(p_owner)
{
	OVSERVICE(Scripting::ScriptEngine).AddBehaviour(*this);
}

Core::ECS::Components::Behaviour::~Behaviour()
{
	OVSERVICE(Scripting::ScriptEngine).RemoveBehaviour(*this);
}

std::string Core::ECS::Components::Behaviour::GetName()
{
	return "Behaviour";
}

void Core::ECS::Components::Behaviour::SetScript(std::unique_ptr<Scripting::Script>&& p_scriptContext)
{
	m_script = std::move(p_scriptContext);
}

Tools::Utils::OptRef<Core::Scripting::Script> Core::ECS::Components::Behaviour::GetScript()
{
	if (m_script)
	{
		return { *m_script };
	}

	return std::nullopt;
}

void Core::ECS::Components::Behaviour::RemoveScript()
{
	m_script.reset();
}

void Core::ECS::Components::Behaviour::OnAwake()
{
	OVSERVICE(Scripting::ScriptEngine).OnAwake(*this);
}

void Core::ECS::Components::Behaviour::OnStart()
{
	OVSERVICE(Scripting::ScriptEngine).OnStart(*this);
}

void Core::ECS::Components::Behaviour::OnEnable()
{
	OVSERVICE(Scripting::ScriptEngine).OnEnable(*this);
}

void Core::ECS::Components::Behaviour::OnDisable()
{
	OVSERVICE(Scripting::ScriptEngine).OnDisable(*this);
}

void Core::ECS::Components::Behaviour::OnDestroy()
{
	OVSERVICE(Scripting::ScriptEngine).OnDestroy(*this);
}

void Core::ECS::Components::Behaviour::OnUpdate(float p_deltaTime)
{
	OVSERVICE(Scripting::ScriptEngine).OnUpdate(*this, p_deltaTime);
}

void Core::ECS::Components::Behaviour::OnFixedUpdate(float p_deltaTime)
{
	OVSERVICE(Scripting::ScriptEngine).OnFixedUpdate(*this, p_deltaTime);
}

void Core::ECS::Components::Behaviour::OnLateUpdate(float p_deltaTime)
{
	OVSERVICE(Scripting::ScriptEngine).OnLateUpdate(*this, p_deltaTime);
}

void Core::ECS::Components::Behaviour::OnCollisionEnter(Components::CPhysicalObject& p_otherObject)
{
	OVSERVICE(Scripting::ScriptEngine).OnCollisionEnter(*this, p_otherObject);
}

void Core::ECS::Components::Behaviour::OnCollisionStay(Components::CPhysicalObject& p_otherObject)
{
	OVSERVICE(Scripting::ScriptEngine).OnCollisionStay(*this, p_otherObject);
}

void Core::ECS::Components::Behaviour::OnCollisionExit(Components::CPhysicalObject& p_otherObject)
{
	OVSERVICE(Scripting::ScriptEngine).OnCollisionExit(*this, p_otherObject);
}

void Core::ECS::Components::Behaviour::OnTriggerEnter(Components::CPhysicalObject& p_otherObject)
{
	OVSERVICE(Scripting::ScriptEngine).OnTriggerEnter(*this, p_otherObject);
}

void Core::ECS::Components::Behaviour::OnTriggerStay(Components::CPhysicalObject& p_otherObject)
{
	OVSERVICE(Scripting::ScriptEngine).OnTriggerStay(*this, p_otherObject);
}

void Core::ECS::Components::Behaviour::OnTriggerExit(Components::CPhysicalObject& p_otherObject)
{
	OVSERVICE(Scripting::ScriptEngine).OnTriggerExit(*this, p_otherObject);
}

void Core::ECS::Components::Behaviour::OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
}

void Core::ECS::Components::Behaviour::OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_node)
{
}

