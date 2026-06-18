#pragma once
#include "Interactive/EventWidget.h"

namespace MOON
{
	class SplitScreen: public EventWidget
	{
	public:
		SplitScreen(const std::string& name);
		virtual ~SplitScreen();
		virtual void onUpdate()override;
		void getLineEquation(float * out);

	private:
		class SplitScreenInternal;
		SplitScreenInternal* m_internal = nullptr;
	};
}