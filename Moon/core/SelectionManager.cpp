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
	};
	SelectionManager& SelectionManager::instance()
	{
		static SelectionManager manager;
		return manager;
	}
	void SelectionManager::setPreselect(SelectID id)
	{
		clearPreselect();
		mInternal->preSelect = id;
		if (mInternal->preSelect.isValid()) {
			auto actor = GetMainScene->FindActorByID(mInternal->preSelect);
			if (actor->HasParent()) {
				auto parent = actor->GetParent();
				if (parent->HasComponent("CTopoShape")) {
					auto topoComp = parent->GetComponent<::Core::ECS::Components::CTopoShape>();
					int childId=parent->GetChildId(actor);
					if (childId != -1) {
						topoComp->hoverChild(childId);
					}
				}
			}
		}
	}
	void SelectionManager::addSelect(const std::vector<SelectID>& selectIdLists)
	{
	}
	void SelectionManager::setSelect(const std::vector<SelectID>& selectIdLists)
	{
	}
	void SelectionManager::clearSelect()
	{
	}
	void SelectionManager::clearPreselect()
	{
		if (mInternal->preSelect.isValid()) {
			auto actor=GetMainScene->FindActorByID(mInternal->preSelect);
			if (actor->HasParent()) {
				auto parent=actor->GetParent();
				if (parent->HasComponent("CTopoShape")) {
					auto topoComp = parent->GetComponent<::Core::ECS::Components::CTopoShape>();
					topoComp->clearHover();
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
		return std::vector<SelectID>();
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