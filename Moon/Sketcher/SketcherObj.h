#pragma once
namespace MOON {
	class SketcherObj {
	public:
		SketcherObj();
		~SketcherObj();
		void setPlane(int p);
		int getPlane();
		void makeDone();
	private:
		int mPlane = 0;
	};

}