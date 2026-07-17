#include "core/ViewTool.h"
#include "core/SelectionManager.h"
#include "TopoShape.h"
#include "Core/Global/ServiceLocator.h"
#include "core/component/CTopoShape.h"
#include "core/component/TopoShapeActor.h"
#include "renderer/SceneView.h"
#include "feature/Feature.h"
namespace MOON {
	Core::ECS::Actor* ViewTool::getLastestActorSelected()
	{
		std::vector<SelectID> selectIds = SelectionManager::instance().getSelect();
		if (!selectIds.empty()) {
			auto& view = GetService(Editor::Panels::SceneView);
			auto scene = view.GetScene();
			auto actor = scene->FindActorByID(selectIds.back());
			return actor;
		}
		return nullptr;
	}
	bool ViewTool::getSelectedTopoShape(std::vector<Part::TopoShape>& topo)
	{
		Core::ECS::Actor* actor =getLastestActorSelected();
		if (!actor) {
			return false;
		}
		if (actor->HasParent()) {
			auto parent = actor->GetParent();
			if (parent->HasParent()) {
				auto grandParent = parent->GetParent();
				if (grandParent->HasComponent("CTopoShape")) {
					auto topoComp = grandParent->GetComponent<::Core::ECS::Components::CTopoShape>();
					std::string idString = actor->GetName().substr(5);
					std::string type = actor->GetName().substr(0, 4);
					int childId = std::stoi(idString);
					topo.push_back( topoComp->GetTopoShape());
					topo.push_back(type == "face"?topoComp->GetTopoFace(childId):topoComp->GetTopoEdge(childId));
					return true;
				}
			}
		}
		return false;
	}
	Feature* ViewTool::getSelectedFeature()
	{
		Core::ECS::Actor* actor = getLastestActorSelected();
		if (!actor) {
			return nullptr;
		}
		Feature* f = dynamic_cast<Feature*>(actor);
		return f;
	}
	bool ViewTool::getSelectedBasedFeature(Feature*&f,std::vector<std::string>&subValues)
	{
		//if use the method,we pretend that the selectids is from a Feature.
		Core::ECS::Actor* actor = getLastestActorSelected();
		if (!actor) {
			return false;
		}
		std::vector<SelectID> selectIds = SelectionManager::instance().getSelect();
		if (actor->HasParent()) {
			auto parent = actor->GetParent();
			if (parent->HasParent()) {
				auto grandParent = parent->GetParent();
				Feature* feature = dynamic_cast<Feature*>(grandParent);
				if (feature) {
					f = feature;
					subValues.clear();
					subValues.reserve(selectIds.size());
					auto& view = GetService(Editor::Panels::SceneView);
					auto scene = view.GetScene();
					for (int i = 0;i < selectIds.size();i++) {
						auto tempActor = scene->FindActorByID(selectIds[i]);
						if (tempActor) {
							subValues.emplace_back(tempActor->GetName());
						}
					}
					return true;
				}
			}
		}
		return false;
	}
	 Core::ECS::Actor* ViewTool::createTopoActor(const Part::TopoShape& topoShape, const char* name )
	{

		auto topoActor = new TopoActor( std::string(name), "TopoShape", false);
		const auto& topoComp = topoActor->GetComponent<Core::ECS::Components::CTopoShape>();
		Part::TopoShape& topo = topoComp->GetTopoShape();
		topo = topoShape;
		topoComp->discretizationShape();
		return topoActor;
	}
}