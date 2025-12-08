#pragma once
#include "Gizmo/GizmoWidget.h"

namespace MOON
{
	class SplitScreen: public GizmoWidget
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