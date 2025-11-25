#include "Gizmo/Widgets/Measurement.h"
#include "Gizmo/Gizmo.h"
#include "renderer/SceneView.h"
namespace MOON {
	struct Measure
	{
		Eigen::Vector3f start;
		Eigen::Vector3f end;
	};
	std::vector<Measure> g_measures;
	bool selectingStart = false;
	Measurement::Measurement(const std::string& name, Editor::Panels::SceneView* view) :GizmoWidget(name), m_sceneView(view)
	{
	}
	Measurement::~Measurement()
	{
	}
	void Measurement::onUpdate()
	{
		 {
			Maths::FVector3 out;
			if (m_sceneView->MouseHit(out)) {
				Eigen::Vector3f pos = { out.x,out.y,out.z };
				if (renderer->wasKeyPressed(MOON::MouseLeft)) {
					if (!selectingStart) {
						Measure m;
						m.start = pos;
						m.end = pos;
						g_measures.push_back(m);
						selectingStart = true;
					}
					else {
						g_measures.back().end = pos;
						selectingStart = false;
					}
				}
				else
				{
					if (selectingStart) {
						g_measures.back().end = pos;
					}
				}
			}
		}
		for (const auto& m : g_measures) {
			renderer->drawLine(m.start, m.end, 2.0,Eigen::Vector4<uint8_t>{ 255, 255, 0, 255 });
			float distance = (m.start - m.end).norm();
			//Eigen::Vector3f mid = (m.start + m.end) * 0.5f;
			//renderer->drawText3D(mid, std::to_string(distance), Eigen::Vector4<uint8_t>{ 255, 255, 255, 255 }, 16);
		}
		//auto rc = m_sceneView->GetRoaterCenter();
		//Eigen::Vector3f center = { rc.x,rc.y,rc.z };
		//renderer->translation(renderer->makeId("rotaterCenter"), center);
	}
}