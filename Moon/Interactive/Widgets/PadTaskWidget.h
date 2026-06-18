#pragma once
#include "Interactive/EventWidget.h"
namespace MOON
{
	enum PadTaskEvent{
		LengthChange,
		None
	};
	class PadTaskWidget: public EventWidget
	{
	public:
		PadTaskWidget(const std::string& name);
		virtual ~PadTaskWidget();
		virtual void onUpdate()override;
		void setUpOrigin(float x,float y,float z );
		void setUpDir(float x, float y, float z);
		float getLength();
		void setLength(float len);
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}