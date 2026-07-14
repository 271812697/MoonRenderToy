#pragma once
#include <vector>
namespace MOON {
	class SketcherFeature;
	class SketcherObj;
	class SketcherObjManager {
	public:
		static SketcherObjManager& instance();
		~SketcherObjManager();
		SketcherFeature* GetCurrentActiveSketcherFeature();
		SketcherObj* GetCurrentActiveSketcherObj();
		void setCurrentActiveSketcherObj(SketcherObj* obj);
		std::vector<SketcherObj*> GetAllSketcherObjs();
		void Push();
		void Pop();
	private:
		class SketcherObjManagerInternal;
		SketcherObjManagerInternal* mInternal = nullptr;
		SketcherObjManager();
		SketcherObj* CreateSketcherObj();
	};
}