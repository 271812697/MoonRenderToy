/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <tracy/Tracy.hpp>

#include <Debug/Logger.h>
#include <Debug/Assertion.h>
#include <Core/Scripting/ScriptEngine.h>
#include <Core/ECS/Components/Behaviour.h>
#include <Core/ECS/Actor.h>

void BindLuaActor(sol::state& p_state);
void BindLuaComponents(sol::state& p_state);
void BindLuaGlobal(sol::state& p_state);
void BindLuaMath(sol::state& p_state);
void BindLuaProfiler(sol::state& p_state);

constexpr auto luaBindings = std::array{
	BindLuaActor,
	BindLuaComponents,
	BindLuaGlobal,
	BindLuaMath,
	BindLuaProfiler
};

template<typename... Args>
void ExecuteLuaFunction(Core::ECS::Components::Behaviour& p_behaviour, const std::string& p_functionName, Args&& ...p_args)
{
	auto context = p_behaviour.GetScript();

	ASSERT(context.has_value(), "The given context is null");
	ASSERT(context->IsValid(), "The given context is invalid");

	const sol::table& table = static_cast<Core::Scripting::LuaScript&>(context.value()).GetContext().table;

	if (table[p_functionName].valid())
	{
		sol::protected_function pfr = table[p_functionName];
		auto pfrResult = pfr.call(table, std::forward<Args>(p_args)...);
		if (!pfrResult.valid())
		{
			sol::error err = pfrResult;
			OVLOG_ERROR(err.what());
		}
	}
}

sol::table LoadScript(sol::state& p_luaState, const std::string& p_scriptName)
{
	using namespace Core::Scripting;

	const auto result = p_luaState.safe_script_file(p_scriptName, &sol::script_pass_on_error);

	if (!result.valid())
	{
		sol::error err = result;
		OVLOG_ERROR(err.what());
		return {};
	}
	else
	{
		if (result.return_count() == 1 && result[0].is<sol::table>())
		{
			return result[0];
		}
		else
		{
			OVLOG_ERROR("'" + p_scriptName + "' missing return expression");
			return {};
		}
	}
}

bool RegisterBehaviour(sol::state& p_luaState, Core::ECS::Components::Behaviour& p_behaviour, const std::string& p_scriptName)
{
	auto table = LoadScript(p_luaState, p_scriptName);

	p_behaviour.SetScript(std::make_unique<Core::Scripting::LuaScript>(table));

	// Update the script context to add the owner reference
	if (auto context = p_behaviour.GetScript(); context.has_value() && context->IsValid())
	{
		auto& luaScript = static_cast<Core::Scripting::LuaScript&>(context.value());
		luaScript.SetOwner(p_behaviour.owner);
		return true;
	}

	return false;
}

Core::Scripting::LuaScriptEngine::LuaScriptEngine()
{
	CreateContext();
}

Core::Scripting::LuaScriptEngine::~LuaScriptEngine()
{
	DestroyContext();
}

void Core::Scripting::LuaScriptEngine::CreateContext()
{
	ASSERT(m_context.luaState == nullptr, "A Lua context already exists!");

	m_context.luaState = std::make_unique<sol::state>();
	m_context.luaState->open_libraries(sol::lib::base, sol::lib::math);

	for (auto& callback : luaBindings)
	{
		callback(*m_context.luaState);
	}

	m_context.errorCount = 0;

	std::for_each(m_context.behaviours.begin(), m_context.behaviours.end(),
		[this](std::reference_wrapper<Core::ECS::Components::Behaviour> behaviour)
		{
			if (!RegisterBehaviour(*m_context.luaState, behaviour.get(), m_context.scriptRootFolder + behaviour.get().name + GetDefaultExtension()))
			{
				++m_context.errorCount;
			}
		}
	);

	if (m_context.errorCount > 0)
	{
		const std::string message = std::to_string(m_context.errorCount) + " script(s) failed to register";
		OVLOG_ERROR(message);
	}
}

void Core::Scripting::LuaScriptEngine::DestroyContext()
{
	ASSERT(m_context.luaState != nullptr, "No valid Lua context");

	std::for_each(m_context.behaviours.begin(), m_context.behaviours.end(),
		[this](std::reference_wrapper<Core::ECS::Components::Behaviour> behaviour)
		{
			behaviour.get().RemoveScript();
		}
	);

	m_context.luaState.reset();
}

template<>
Core::Scripting::LuaScriptEngineBase::TScriptEngine() {}

template<>
Core::Scripting::LuaScriptEngineBase::~TScriptEngine() {}

template<>
void Core::Scripting::LuaScriptEngineBase::SetScriptRootFolder(const std::string& p_scriptRootFolder)
{
	m_context.scriptRootFolder = p_scriptRootFolder;
}

template<>
std::vector<std::string> Core::Scripting::LuaScriptEngineBase::GetValidExtensions()
{
	return { GetDefaultExtension() };
}

template<>
std::string Core::Scripting::LuaScriptEngineBase::GetDefaultScriptContent(const std::string& p_name)
{
	return "local " + p_name + " =\n{\n}\n\nfunction " + p_name + ":OnStart()\nend\n\nfunction " + p_name + ":OnUpdate(deltaTime)\nend\n\nreturn " + p_name;
}

template<>
std::string Core::Scripting::LuaScriptEngineBase::GetDefaultExtension()
{
	return ".lua";
}

template<>
void Core::Scripting::LuaScriptEngineBase::AddBehaviour(Core::ECS::Components::Behaviour& p_toAdd)
{
	ASSERT(m_context.luaState != nullptr, "No valid Lua context");

	m_context.behaviours.push_back(std::ref(p_toAdd));

	if (!RegisterBehaviour(*m_context.luaState, p_toAdd, m_context.scriptRootFolder + p_toAdd.name + GetDefaultExtension()))
	{
		++m_context.errorCount;
	}
}

template<>
void Core::Scripting::LuaScriptEngineBase::RemoveBehaviour(Core::ECS::Components::Behaviour& p_toRemove)
{
	if (m_context.luaState)
	{
		p_toRemove.RemoveScript();
	}

	m_context.behaviours.erase(
		std::remove_if(m_context.behaviours.begin(), m_context.behaviours.end(),
			[&p_toRemove](std::reference_wrapper< Core::ECS::Components::Behaviour> behaviour) {
				return &p_toRemove == &behaviour.get();
			}
		)
	);

	// Unconsidering a script is impossible with Lua, we have to reparse every behaviours
	// @note this might be costly, we should look into it more seriously.
	Reload(); 
}

template<>
void Core::Scripting::LuaScriptEngineBase::Reload()
{
	static_cast<LuaScriptEngine&>(*this).DestroyContext();
	static_cast<LuaScriptEngine&>(*this).CreateContext();
}

template<>
bool Core::Scripting::LuaScriptEngineBase::IsOk() const
{
	return m_context.luaState && m_context.errorCount == 0;
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnAwake(Core::ECS::Components::Behaviour& p_target)
{
	ExecuteLuaFunction(p_target, "OnAwake");
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnStart(Core::ECS::Components::Behaviour& p_target)
{
	ExecuteLuaFunction(p_target, "OnStart");
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnEnable(Core::ECS::Components::Behaviour& p_target)
{
	ExecuteLuaFunction(p_target, "OnEnable");
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnDisable(Core::ECS::Components::Behaviour& p_target)
{
	ExecuteLuaFunction(p_target, "OnDisable");
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnDestroy(Core::ECS::Components::Behaviour& p_target)
{
	ExecuteLuaFunction(p_target, "OnDestroy");
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnUpdate(Core::ECS::Components::Behaviour& p_target, float p_deltaTime)
{
	ExecuteLuaFunction(p_target, "OnUpdate", p_deltaTime);
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnFixedUpdate(Core::ECS::Components::Behaviour& p_target, float p_deltaTime)
{
	ExecuteLuaFunction(p_target, "OnFixedUpdate", p_deltaTime);
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnLateUpdate(Core::ECS::Components::Behaviour& p_target, float p_deltaTime)
{
	ExecuteLuaFunction(p_target, "OnLateUpdate", p_deltaTime);
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnCollisionEnter(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
	ExecuteLuaFunction(p_target, "OnCollisionEnter", p_otherObject);
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnCollisionStay(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
	ExecuteLuaFunction(p_target, "OnCollisionStay", p_otherObject);
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnCollisionExit(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
	ExecuteLuaFunction(p_target, "OnCollisionExit", p_otherObject);
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnTriggerEnter(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
	ExecuteLuaFunction(p_target, "OnTriggerEnter", p_otherObject);
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnTriggerStay(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
	ExecuteLuaFunction(p_target, "OnTriggerStay", p_otherObject);
}

template<>
void Core::Scripting::LuaScriptEngineBase::OnTriggerExit(Core::ECS::Components::Behaviour& p_target, Core::ECS::Components::CPhysicalObject& p_otherObject)
{
	ExecuteLuaFunction(p_target, "OnTriggerExit", p_otherObject);
}
