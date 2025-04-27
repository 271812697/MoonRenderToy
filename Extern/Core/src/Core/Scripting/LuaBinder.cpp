

#include "Core/Scripting/LuaBinder.h"

#include "Core/Scripting/LuaMathsBinder.h"
#include "Core/Scripting/LuaActorBinder.h"
#include "Core/Scripting/LuaComponentBinder.h"
#include "Core/Scripting/LuaGlobalsBinder.h"

void Core::Scripting::LuaBinder::CallBinders(sol::state& p_luaState)
{
	auto& L = p_luaState;

	LuaMathsBinder::BindMaths(L);
	LuaActorBinder::BindActor(L);
	LuaComponentBinder::BindComponent(L);
	LuaGlobalsBinder::BindGlobals(L);
}