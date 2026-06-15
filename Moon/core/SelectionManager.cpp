#include "core/SelectionManager.h"
#include "renderer/SceneView.h"
#include "core/Global/ServiceLocator.h"
#include "core/component/CTopoShape.h"

namespace MOON {
	class SelectionManager::Internal 
	{
	public:
		Internal(SelectionManager* s):self(s){
		
		}
		~Internal() {
		}
	private:
		friend SelectionManager;
		SelectionManager* self = nullptr;
		SelectID preSelect;
		std::vector<SelectID>selectIDs;
	};
	SelectionManager& SelectionManager::instance()
	{
		static SelectionManager manager;
		return manager;
	}
	void SelectionManager::setPreselect(SelectID id)
	{
		if (mInternal->preSelect != id) {
			clearPreselect();
			mInternal->preSelect = id;
			if (mInternal->preSelect.isValid()) {
				auto actor = GetMainScene->FindActorByID(mInternal->preSelect);
				if (actor) {
					if (actor->HasParent()) {
						auto parent = actor->GetParent();
						if (parent->HasParent()) {
							auto grandParent = parent->GetParent();
							if (grandParent->HasComponent("CTopoShape")) {
								auto topoComp = grandParent->GetComponent<::Core::ECS::Components::CTopoShape>();
								//int childId=parent->GetChildId(actor);
								std::string idString=actor->GetName().substr(5);
								int childId = std::stoi(idString);
								if (parent->HasComponent("CBatchMeshTriangle")) {
									if (childId != -1) {
										topoComp->hoverChild(childId);
									}
								}
								else if (parent->HasComponent("CBatchMeshLine")) {
									if (childId != -1) {
										topoComp->hoverChildLine(childId);
										//topoComp->hoverChild(childId);
									}
								}					
							}
						}
					}
				}
			}
		}
	}
	void SelectionManager::addSelect(const std::vector<SelectID>& selectIdLists)
	{
		mInternal->selectIDs.insert(mInternal->selectIDs.end(), selectIdLists.begin(), selectIdLists.end());
	}
	void SelectionManager::setSelect(const std::vector<SelectID>& selectIdLists)
	{
		mInternal->selectIDs = selectIdLists;
	}
	void SelectionManager::clearSelect()
	{
		mInternal->selectIDs.clear();
	}
	void SelectionManager::clearPreselect()
	{
		if (mInternal->preSelect.isValid()) {
			auto actor=GetMainScene->FindActorByID(mInternal->preSelect);
			if (actor) {
				if (actor->HasParent()) {
					auto parent=actor->GetParent();
					if (parent->HasParent()) {
						auto grandParent = parent->GetParent();
						if (grandParent->HasComponent("CTopoShape")) {
							auto topoComp = grandParent->GetComponent<::Core::ECS::Components::CTopoShape>();
							if (parent->HasComponent("CBatchMeshTriangle")) {
								topoComp->clearHover();
							}
							else if (parent->HasComponent("CBatchMeshLine")) {
								topoComp->clearHoverLine();
							}
						}
					}
				}
			}
			mInternal->preSelect.reset();
		}
	}
	SelectID SelectionManager::getPreselect()
	{
		return mInternal->preSelect;
	}
	std::vector<SelectID> SelectionManager::getSelect()
	{
		return mInternal->selectIDs;
	}
	SelectionManager::~SelectionManager()
	{
		if (mInternal) {
			delete mInternal;
		}
	}
	SelectionManager::SelectionManager():mInternal(new Internal(this))
	{
	}
}