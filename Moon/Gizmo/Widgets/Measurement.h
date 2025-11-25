#pragma once
#include "Gizmo/GizmoWidget.h"
namespace Editor {
	namespace Panels {
		class SceneView;
	}
}
namespace MOON
{
	class Measurement: public GizmoWidget
	{
	public:
		Measurement(const std::string& name, Editor::Panels::SceneView* view);
		virtual ~Measurement();
		virtual void onUpdate()override;
	private:
		Editor::Panels::SceneView* m_sceneView = nullptr;
	};
}