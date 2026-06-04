#pragma once
#include <vector>
namespace MOON {
	class SketcherObj;
	class SketcherObjManager {
	public:
		static SketcherObjManager& instance();
		~SketcherObjManager();
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