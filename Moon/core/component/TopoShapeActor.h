#pragma once
#include "core/ECS/Actor.h"
namespace Core::SceneSystem
{
	class Scene;
}
namespace Core::ECS { 
	class TopoActor :public Actor {
	public:
		TopoActor(Core::SceneSystem::Scene* scene, const std::string& p_name, const std::string& p_tag, bool p_playing);
		virtual ~TopoActor() override;
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot) override;
	};
}
