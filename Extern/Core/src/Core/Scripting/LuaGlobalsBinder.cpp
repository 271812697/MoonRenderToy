

#include "Core/Scripting/LuaGlobalsBinder.h"

#include <Debug/Logger.h>
#include"../../tools/Random.h"

#include "Core/ECS/Actor.h"
#include "Core/ECS/PhysicsWrapper.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/SceneSystem/SceneManager.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "Core/ResourceManagement/ShaderManager.h"
#include "Core/ResourceManagement/TextureManager.h"
#include "Core/ResourceManagement/MaterialManager.h"
#include "Core/ResourceManagement/SoundManager.h"

#include <Physics/Entities/PhysicalObject.h>



void Core::Scripting::LuaGlobalsBinder::BindGlobals(sol::state & p_luaState)
{


}
