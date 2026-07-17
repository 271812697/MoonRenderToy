#include "core/SelectionManager.h"
#include "renderer/SceneView.h"
#include "core/Global/ServiceLocator.h"
#include "core/component/CTopoShape.h"
#include <unordered_set>

namespace MOON {
	struct SelectIDHash
	{
		size_t operator()(const SelectID& id) const noexcept
		{
			return std::hash<size_t>{}(id.actorId);
		}
	};
	class SelectionManager::Internal 
	{
	public:
		Internal(SelectionManager* s):self(s){
			mSelectMode=OverrideSelect;
		}
		~Internal() {
		}
	private:
		friend SelectionManager;
		SelectionManager* self = nullptr;
		SelectID preSelect;

		std::unordered_set<SelectID, SelectIDHash>selectIDs;
		SelectMode mSelectMode;
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
									}
								}					
							}
						}
					}
				}
			}
		}
	}
	void SelectionManager::select(const std::vector<SelectID>& selectIdLists)
	{
		if (mInternal->mSelectMode == SelectMode::OverrideSelect&& selectIdLists.size()==0) {
			std::unordered_map<::Core::ECS::Components::CTopoShape*, int>selectFaceMap;
			std::unordered_map<::Core::ECS::Components::CTopoShape*, int>selectEdgeMap;
			for (auto& id : mInternal->selectIDs) {
				auto actor = GetMainScene->FindActorByID(id);
				if (actor) {
					if (actor->HasParent()) {
						auto parent = actor->GetParent();
						if (parent->HasParent()) {
							auto grandParent = parent->GetParent();
							if (grandParent->HasComponent("CTopoShape")) {
								auto topoComp = grandParent->GetComponent<::Core::ECS::Components::CTopoShape>();

								std::string idString = actor->GetName().substr(5);
								int childId = std::stoi(idString);
								if (parent->HasComponent("CBatchMeshTriangle")) {
									if (childId != -1) {
										selectFaceMap[topoComp]=1;
									}
								}
								else if (parent->HasComponent("CBatchMeshLine")) {
									if (childId != -1) {
										selectEdgeMap[topoComp]=1;
									}
								}
							}
						}
					}
				}
			}
			for (auto& it : selectFaceMap) {
				it.first->selectChildFaces({});
			}
			for (auto& it : selectEdgeMap) {
				it.first->clearSelectLines();
			}
			mInternal->selectIDs.clear();
			return;
		}
		int beforeSelectSize = mInternal->selectIDs.size();
		if (mInternal->mSelectMode==SelectMode::OverrideSelect) {
			mInternal->selectIDs.clear();
			mInternal->selectIDs.insert(selectIdLists.begin(), selectIdLists.end());
		}
		else if(mInternal->mSelectMode == SelectMode::AddSelect)
		{
			if (selectIdLists.size() > 0) {
				mInternal->selectIDs.insert( selectIdLists.begin(), selectIdLists.end());
			}
		}
		int curSelectSize= mInternal->selectIDs.size();
		if (curSelectSize != beforeSelectSize) {
			std::unordered_map<::Core::ECS::Components::CTopoShape*, std::vector<int>>selectFaceMap;
			std::unordered_map<::Core::ECS::Components::CTopoShape*, std::vector<int>>selectEdgeMap;
			for (auto& id:mInternal->selectIDs) {
				auto actor = GetMainScene->FindActorByID(id);
				if (actor) {
					if (actor->HasParent()) {
						auto parent = actor->GetParent();
						if (parent->HasParent()) {
							auto grandParent = parent->GetParent();
							if (grandParent->HasComponent("CTopoShape")) {
								auto topoComp = grandParent->GetComponent<::Core::ECS::Components::CTopoShape>();
								
								std::string idString = actor->GetName().substr(5);
								int childId = std::stoi(idString);
								if (parent->HasComponent("CBatchMeshTriangle")) {
									if (childId != -1) {
										selectFaceMap[topoComp].push_back(childId);
									}
								}
								else if (parent->HasComponent("CBatchMeshLine")) {
									if (childId != -1) {
										selectEdgeMap[topoComp].push_back(childId);
									}
								}
							}
						}
					}
				}
			}
			for(auto&it:selectFaceMap){
				it.first->selectChildFaces(it.second);
			}
			for (auto& it : selectEdgeMap) {
				it.first->selectChildLines(it.second);
			}
			InvokeEvent(SelectAny);
		}
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
		return std::vector<SelectID>{ mInternal->selectIDs.begin(), mInternal->selectIDs.end() };
	}
	void SelectionManager::setSelectMode(SelectMode mode)
	{
		mInternal->mSelectMode = mode;
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