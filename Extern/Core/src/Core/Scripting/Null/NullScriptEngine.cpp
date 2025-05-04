/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Core/Scripting/Null/NullScriptEngine.h>

template<>
Core::Scripting::NullScriptEngineBase::TScriptEngine() {}

template<>
Core::Scripting::NullScriptEngineBase::~TScriptEngine() {}

template<>
void Core::Scripting::NullScriptEngineBase::SetScriptRootFolder(const std::string& p_scriptRootFolder) {}

template<>
std::vector<std::string> Core::Scripting::NullScriptEngineBase::GetValidExtensions()
{
	return { GetDefaultExtension() };
}

template<>
std::string Core::Scripting::NullScriptEngineBase::GetDefaultScriptContent(const std::string& p_name)
{
	return "class " + p_name + " {\n}";
}

template<>
std::string Core::Scripting::NullScriptEngineBase::GetDefaultExtension()
{
	return ".ovscript";
}

template<>
void Core::Scripting::NullScriptEngineBase::AddBehaviour(Core::ECS::Components::Behaviour& p_toAdd)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::RemoveBehaviour(Core::ECS::Components::Behaviour& p_toRemove)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::Reload()
{
}

template<>
bool Core::Scripting::NullScriptEngineBase::IsOk() const
{
	return true;
}

template<>
void Core::Scripting::NullScriptEngineBase::OnAwake(Core::ECS::Components::Behaviour& p_target)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnStart(Core::ECS::Components::Behaviour& p_target)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnEnable(Core::ECS::Components::Behaviour& p_target)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnDisable(Core::ECS::Components::Behaviour& p_target)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnDestroy(Core::ECS::Components::Behaviour& p_target)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnUpdate(Core::ECS::Components::Behaviour& p_target, float p_deltaTime)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnFixedUpdate(Core::ECS::Components::Behaviour& p_target, float p_deltaTime)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnLateUpdate(Core::ECS::Components::Behaviour& p_target, float p_deltaTime)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnCollisionEnter(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnCollisionStay(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnCollisionExit(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnTriggerEnter(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnTriggerStay(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
}

template<>
void Core::Scripting::NullScriptEngineBase::OnTriggerExit(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
}
