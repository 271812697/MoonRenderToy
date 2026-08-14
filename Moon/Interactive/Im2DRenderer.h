#pragma once 

namespace MOON {
	namespace Render2D{
	struct ImDrawList;
	class Im2DRender {
	public:
		~Im2DRender();
		static Im2DRender& instance();
		ImDrawList* getDrawList();
		void newFrame();
		void endFrame();
	private:
		Im2DRender();
		class Internal;
		Internal* mInternal=nullptr;
		
	};
}


}