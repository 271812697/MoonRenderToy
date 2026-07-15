#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
#include "feature/SketcherFeature.h"
#include "editor/UI/TreeViewPanel/treeViewpanel.h"
#include "Core/Global/ServiceLocator.h"
#include <memory>
#include <vector>
namespace MOON {
	class SketcherObjManager::SketcherObjManagerInternal {
	public:
		SketcherObjManagerInternal(SketcherObjManager* s):self(s) {

		}
		~SketcherObjManagerInternal() {

		}
	private:
		friend SketcherObjManager;
		SketcherObjManager* self = nullptr;
		SketcherObj* currentSketcherObj = nullptr;
		std::vector<SketcherFeature*>sketchers;
	};
	SketcherObjManager& SketcherObjManager::instance() {
		static SketcherObjManager sketcherManager;
		return sketcherManager;
	}
	SketcherObjManager::~SketcherObjManager()
	{
		delete mInternal;
	}
	SketcherFeature* SketcherObjManager::CreateSketcherFeature()
	{
		SketcherFeature* ptr = new SketcherFeature("sketcher");
		mInternal->sketchers.push_back(ptr);;
		return ptr;
	}
	SketcherFeature* SketcherObjManager::GetCurrentActiveSketcherFeature()
	{
		if(mInternal->sketchers.size()>0)
		return mInternal->sketchers.back();
		return nullptr;
	}
	SketcherObj* SketcherObjManager::GetCurrentActiveSketcherObj()
	{
		auto feature = GetCurrentActiveSketcherFeature();
		if (feature) {
			return feature->getSketcherObj();
		}
		return nullptr;
	}
	void SketcherObjManager::setCurrentActiveSketcherObj(SketcherObj* obj)
	{
		auto& sketchers = mInternal->sketchers;
		if (!obj) {
			if (sketchers.size()) {
				mInternal->currentSketcherObj = sketchers.back()->getSketcherObj();
			}
			else
			{
				mInternal->currentSketcherObj = nullptr;
			}
		}
		else
		{
			
			for (int i = 0;i < sketchers.size();i++) {
				if (sketchers[i]->getSketcherObj() == obj) {
					mInternal->currentSketcherObj = obj;
					break;
				}
			}
		}
		//for (int i = 0;i < sketchers.size();i++) {
		//	if (sketchers[i].get() != mInternal->currentSketcherObj) {
		//		sketchers[i]->setActive(false);
		//	}
		//	else
		//	{
		//		sketchers[i]->setActive(true);
		//	}
		//}
	}
	std::vector<SketcherObj*> SketcherObjManager::GetAllSketcherObjs()
	{
		std::vector<SketcherObj*>res;
		//for (int i = 0;i < mInternal->sketchers.size();i++) {
		//	res.push_back(mInternal->sketchers[i].get());
		//}
		return res;
	}

	SketcherObj* SketcherObjManager::CreateSketcherObj()
	{
		SketcherFeature* ptr = new SketcherFeature("sketcher");
		mInternal->sketchers.push_back(ptr);;
		return ptr->getSketcherObj();
	}
	SketcherObjManager::SketcherObjManager():mInternal(new SketcherObjManagerInternal(this))
	{
	}
}