#pragma once
#include "Gizmo/Interactive/AbstractWidget.h"
#include <string>
namespace Editor {
	namespace Panels {
		class SceneView;
	}
}
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

		virtual void onLeftMousePressed();
		virtual void onLeftMouseReleased();
		virtual void onRightMousePressed();
		virtual void onRightMouseReleased();
		virtual void onMouseMove();
		virtual void onKeyPress(const std::string& key);
		virtual void onKeyRelease(const std::string& key);
		void SetEnabled(int) override;
		static void LeftMousePressed(AbstractWidget*);
		static void LeftMouseReleased(AbstractWidget*);
		static void RightMouseReleased(AbstractWidget*);
		static void RightMousePressed(AbstractWidget*);
		static void MouseMove(AbstractWidget*);
	protected:
		CallbackCommand* KeyEventCallbackCommand;
		static void ProcessKeyEvents(GizmoObject*, unsigned long, void*, void*);
		std::string mName;	
		//mActive 
		bool mActive = true;
		bool mVisible = true;
		bool mPreflag = false;
		bool mCurflag = false;
		Gizmo* renderer= nullptr;
		Editor::Panels::SceneView* m_sceneView = nullptr;
	};
}