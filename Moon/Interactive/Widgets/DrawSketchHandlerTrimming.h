#pragma once

#include "Interactive/Widgets/DrawSketchHandler.h"
namespace MOON
{

	class DrawSketchHandlerTrimming: public DrawSketchHandler
	{
	public:
		DrawSketchHandlerTrimming(const std::string& name);
		virtual ~DrawSketchHandlerTrimming();
		virtual void onUpdate()override;
		virtual void onSetActive(bool flag)override;
		virtual void onMouseMove()override;
		virtual void onLeftMousePressed()override;
	private:
		void updateTrimData();
		class Internal;
		Internal* m_internal = nullptr;
	};
}