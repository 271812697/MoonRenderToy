/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Debug/Logger.h>
#include <Tools/Utils/Random.h>

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

#include <sol/sol.hpp>

void BindLuaGlobal(sol::state& p_luaState)
{
	//using namespace Windowing;
	//using namespace Windowing::Inputs;
	using namespace Maths;
	using namespace Core::ECS;
	using namespace Core::SceneSystem;
	using namespace Core::ResourceManagement;

	p_luaState.new_usertype<Scene>("Scene",
		"FindActorByName", &Scene::FindActorByName,
		"FindActorByTag", &Scene::FindActorByTag,
		"FindActorsByName", &Scene::FindActorsByName,
		"FindActorsByTag", &Scene::FindActorsByTag,
		"CreateActor", sol::overload(
			sol::resolve<Actor & (void)>(&Scene::CreateActor),
			sol::resolve<Actor & (const std::string&, const std::string&)>(&Scene::CreateActor))
	);



	p_luaState.create_named_table("Debug",
		"Log", [](const std::string& p_message) { OVLOG(p_message); },
		"LogInfo", [](const std::string& p_message) { OVLOG_INFO(p_message); },
		"LogWarning", [](const std::string& p_message) { OVLOG_WARNING(p_message); },
		"LogError", [](const std::string& p_message) { OVLOG_ERROR(p_message); }
	);



	p_luaState.create_named_table("Scenes",
		"GetCurrentScene", []() -> Scene& { return *OVSERVICE(SceneManager).GetCurrentScene(); },
		"Load", [](const std::string& p_path) { OVSERVICE(SceneManager).LoadAndPlayDelayed(p_path); }
	);

	p_luaState.create_named_table("Resources",
		"GetModel", [](const std::string& p_resPath) { return OVSERVICE(ModelManager).GetResource(p_resPath); },
		"GetShader", [](const std::string& p_resPath) { return OVSERVICE(ShaderManager).GetResource(p_resPath); },
		"GetTexture", [](const std::string& p_resPath) { return OVSERVICE(TextureManager).GetResource(p_resPath); },
		"GetMaterial", [](const std::string& p_resPath) { return OVSERVICE(MaterialManager).GetResource(p_resPath); },
		"GetSound", [](const std::string& p_resPath) { return OVSERVICE(SoundManager).GetResource(p_resPath); }
	);

	p_luaState.create_named_table("Math",
		"RandomInt", [](int p_min, int p_max) { return Tools::Utils::Random::Generate(p_min, p_max); },
		"RandomFloat", [](float p_min, float p_max) { return Tools::Utils::Random::Generate(p_min, p_max); },
		"CheckPercentage", [](float p_percentage) { return Tools::Utils::Random::CheckPercentage(p_percentage); },
		"Lerp", [](float a, float b, float t) -> float { return a + t * (b - a); }
	);

	p_luaState.new_usertype<PhysicsWrapper::RaycastHit>("RaycastHit",
		"FirstResultObject", &PhysicsWrapper::RaycastHit::FirstResultObject,
		"ResultObjects", &PhysicsWrapper::RaycastHit::ResultObjects
	);

	p_luaState.create_named_table("Physics",
		"Raycast", [](const Maths::FVector3& p_origin, const Maths::FVector3& p_direction, float p_distance) { return PhysicsWrapper::Raycast(p_origin, p_direction, p_distance); }
	);
}
