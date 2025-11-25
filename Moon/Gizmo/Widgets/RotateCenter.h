#pragma once
#include "Gizmo/GizmoWidget.h"
namespace Editor {
	namespace Panels {
		class SceneView;
	}
}
namespace MOON
{
	class RotateCenter: public GizmoWidget
	{
	public:
		RotateCenter(const std::string& name, Editor::Panels::SceneView* view);
		virtual ~RotateCenter();
		virtual void onUpdate()override;
	private:
		Editor::Panels::SceneView* m_sceneView = nullptr;
	};
}