#pragma once
namespace MOON {
	class SketcherObj;
	class SketcherObjManager {
	public:
		static SketcherObjManager& instance();
		~SketcherObjManager();
		SketcherObj* GetCurrentActiveSketcherObj();
		void Push();
		void Pop();
	private:
		class SketcherObjManagerInternal;
		SketcherObjManagerInternal* mInternal = nullptr;
		SketcherObjManager();
		SketcherObj* CreateSketcherObj();
	};
}