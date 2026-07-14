#pragma once
#include "core/ECS/Actor.h"
namespace Core::SceneSystem
{
	class Scene;
}
namespace Part {
	class TopoShape;
}
namespace MOON { 
	class TopoActor :public ::Core::ECS::Actor {
	public:
		TopoActor(const std::string& p_name, const std::string& p_tag, bool p_playing,bool addToTree=true);
		void ClearModel();
		virtual ~TopoActor() override;
		virtual void OnSerialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot) override;
		virtual void OnDeserialize(tinyxml2::XMLDocument& p_doc, tinyxml2::XMLNode* p_actorsRoot) override;
		void RemoveFromScene();
		Part::TopoShape& GetTopoShape();
		void setTopoShape(Part::TopoShape shape);
	protected:
		Part::TopoShape* topoShape;
	private:
		
		Core::SceneSystem::Scene* m_scene=nullptr;
	};
}
