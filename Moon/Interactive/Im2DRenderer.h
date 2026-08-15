#pragma once 

namespace MOON {
	struct ImDrawList;
	namespace Render2D{
	
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