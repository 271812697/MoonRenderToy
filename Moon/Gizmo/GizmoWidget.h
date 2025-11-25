#pragma once
#include <string>

namespace MOON
{
	class Gizmo;
	class GizmoWidget
	{
	public:
		GizmoWidget(const std::string& name);
		virtual ~GizmoWidget();
		const std::string& getName() const { return mName; }
		bool isEnabled() const { return mActive; }
		void setEnabled(bool flag) { mActive = flag; onSetEnable(flag); }
		void update();
		virtual void onUpdate();
		virtual void onSetEnable(bool flag);
	protected:
		std::string mName;	
		bool mActive = true;
		bool mPreflag = false;
		bool mCurflag = false;
		Gizmo* renderer= nullptr;
	};
}