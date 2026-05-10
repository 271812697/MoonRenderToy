#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"
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
		std::vector<std::shared_ptr<SketcherObj>>sketchers;
	};
	SketcherObjManager& SketcherObjManager::instance() {
		static SketcherObjManager sketcherManager;
		return sketcherManager;
	}
	SketcherObjManager::~SketcherObjManager()
	{
		delete mInternal;
	}
	SketcherObj* SketcherObjManager::GetCurrentActiveSketcherObj()
	{
		if (mInternal->sketchers.size()) {
			return mInternal->sketchers.back().get();
		}
		return nullptr;
	}
	void SketcherObjManager::Push()
	{
		CreateSketcherObj();
	}
	void SketcherObjManager::Pop()
	{
		mInternal->sketchers.pop_back();
	}
	SketcherObj* SketcherObjManager::CreateSketcherObj()
	{
		std::shared_ptr<SketcherObj>ptr = std::make_shared<SketcherObj>();
		mInternal->sketchers.push_back(ptr);
		return ptr.get();
	}
	SketcherObjManager::SketcherObjManager():mInternal(new SketcherObjManagerInternal(this))
	{
	}
}