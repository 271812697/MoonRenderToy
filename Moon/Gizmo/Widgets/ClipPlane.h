#pragma once
#include "Gizmo/GizmoWidget.h"
namespace Editor {
	namespace Panels {
		class SceneView;
	}
}
namespace MOON
{
	class ClipPlane: public GizmoWidget
	{
	public:
		ClipPlane(const std::string& name, Editor::Panels::SceneView* view);
		virtual ~ClipPlane();
		virtual void onUpdate()override;

	private:
		class ClipPlaneInternal;
		ClipPlaneInternal* m_internal = nullptr;
		Editor::Panels::SceneView* m_sceneView = nullptr;
	};
}