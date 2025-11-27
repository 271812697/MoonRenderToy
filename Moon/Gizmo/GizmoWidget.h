#pragma once
#include "Gizmo/Interactive/AbstractWidget.h"
#include <string>

namespace MOON
{
	class Gizmo;
	class GizmoWidget : public AbstractWidget
	{
	public:
		GizmoWidget(const std::string& name);
		virtual ~GizmoWidget();
		const std::string& getName() const { return mName; }
		bool isActived() const { return mActive; }
		void setActive(bool flag);
		void setVisible(bool flag);
		void update();
		virtual void onUpdate();
		virtual void onSetActive(bool flag);
	protected:
		std::string mName;	
		//mActive 
		bool mActive = true;
		bool mVisible = true;
		bool mPreflag = false;
		bool mCurflag = false;
		Gizmo* renderer= nullptr;
		
	};
}