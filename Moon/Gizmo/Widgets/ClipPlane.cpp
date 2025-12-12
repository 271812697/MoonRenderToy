#include "Gizmo/Widgets/ClipPlane.h"
#include "Gizmo/Gizmo.h"
#include "Gizmo/MathUtil/MathUtil.h"
#include "renderer/SceneView.h"
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"

namespace MOON {
	class ClipPlane::ClipPlaneInternal {
	public:
		ClipPlaneInternal(ClipPlane* clip):mSelf(clip) {
		
			clickObserver = mSelf->Interactor->AddObserver(ExecuteCommand::LeftButtonReleaseEvent, this, &ClipPlane::ClipPlaneInternal::onMouseLeftClick, 0.0f);
			
		}
		~ClipPlaneInternal() {
			delete clickObserver.command;
			//delete moveObserver.command;
		}
		
		void onMouseLeftClick() {
			if (mSelf->m_sceneView->IsSelectActor()) {
				auto acptr=&mSelf->m_sceneView->GetSelectedActor();
				if (acptr!=ac) {
					ac = acptr;
					auto modelRenderer = ac->GetComponent<::Core::ECS::Components::CModelRenderer>();
					if (modelRenderer) {
						auto model = modelRenderer->GetModel();
						if (model) {
							auto& box=model->GetBoundingBox();
							auto transform = ac->GetComponent<::Core::ECS::Components::CTransform>();
							auto bbox=box.transform(transform->GetWorldMatrix());
							setupBox({ bbox.pmin.x,bbox.pmin.y,bbox.pmin.z }, { bbox.pmax.x,bbox.pmax.y,bbox.pmax.z });						
						}
					}
				}
			}
		}
		void setupBox(const Eigen::Vector3f& min, const Eigen::Vector3f& max) {
			boxMin = min;
			boxMax = max;
			center = (min + max) * 0.5f;
			extent = (boxMax - boxMin).norm();
		}
	private:
		friend class ClipPlane;
		ClipPlane* mSelf = nullptr;
		Core::ECS::Actor* ac = nullptr;
		Eigen::Vector3f center = {0,0,0};
		Eigen::Vector3f normal = { 0,1,0 };
		Eigen::Vector3f xAxis = {1,0,0};
		Eigen::Vector3f yAxis = { 0,1,0 };
		Eigen::Vector3f zAxis = { 0,0,1 };
		Eigen::Vector3f boxMin = {0,0,0};
		Eigen::Vector3f boxMax = {0,0,0};
		float extent = 1.0f;
		ExecuteCommandPair clickObserver;
		ExecuteCommandPair moveObserver;
	};

	ClipPlane::ClipPlane(const std::string& name) :GizmoWidget(name)
	, m_internal(new ClipPlaneInternal(this)){
	
	}
	ClipPlane::~ClipPlane()
	{
		delete m_internal;
	}
	void ClipPlane::onUpdate()
	{
		Eigen::Vector3f pos= m_internal->center;
		float radius=renderer->pixelsToWorldSize(m_internal->center,10);
		float worldHeight = renderer->pixelsToWorldSize(m_internal->center, 170);

		//RotationMatrixX(m_internal->normal);
		//renderer->gizmoOperateNormalBehavior(renderer->makeId("clipNormal"), m_internal->center, m_internal->center + m_internal->normal * radius * 2.0f, radius/2, &m_internal->normal);
		//drawOneMesh(Eigen::Vector3f & translation, Eigen::Matrix3f & rotation, Eigen::Vector3f & scale, const std::string & mesh, bool longterm = false);
		renderer->drawOneMesh(
			m_internal->center,
			RotationMatrix(m_internal->xAxis,m_internal->yAxis,m_internal->zAxis),
			Eigen::Vector3f{ 0.1f,0.1f,0.1f },
			"GizmoAxis");
		
		//gizmoSphereRotateInCircleBehavior(unsigned int _id, const Eigen::Vector3f & _origin, float _radius, const Eigen::Vector3f & axis, Eigen::Vector3f * _out_)
		Eigen::Vector3f scenter = m_internal->center + m_internal->yAxis* worldHeight;
		renderer->drawSphereFilled(scenter,0.2);
		if (renderer->gizmoSphereRotateInCircleBehavior(renderer->makeId("planeEditz"),
			m_internal->center, 0.2, m_internal->zAxis,
			&scenter
		)) {
			m_internal->yAxis = (scenter - m_internal->center).normalized();
			m_internal->xAxis = m_internal->yAxis.cross(m_internal->zAxis);
		}

		scenter = m_internal->center + m_internal->xAxis* worldHeight;
		renderer->drawSphereFilled(scenter, 0.2);
		if (renderer->gizmoSphereRotateInCircleBehavior(renderer->makeId("planeEdity"),
			m_internal->center, 0.2, m_internal->yAxis,
			&scenter
		)) {
			m_internal->xAxis = (scenter - m_internal->center).normalized();
			m_internal->zAxis = m_internal->xAxis.cross(m_internal->yAxis);
		}

		scenter = m_internal->center + m_internal->zAxis * worldHeight;
		renderer->drawSphereFilled(scenter, 0.2);
		if (renderer->gizmoSphereRotateInCircleBehavior(renderer->makeId("planeEditx"),
			m_internal->center, 0.2, m_internal->xAxis,
			&scenter
		)) {
			m_internal->zAxis = (scenter - m_internal->center).normalized();
			m_internal->yAxis = m_internal->zAxis.cross(m_internal->xAxis);
		}


		
		bool ret=renderer->planeEdit(renderer->makeId("planeEdit"), m_internal->center, m_internal->zAxis);
		//renderer->planeEdit();
		//if (ret) {
		//	auto& feature=m_sceneView->GetRenderer().GetFeature<::Core::Rendering::EngineBufferRenderFeature>();
		//	
		//	feature.SetClipPlane(
		//		m_internal->zAxis.x(),
		//		m_internal->zAxis.y(),
		//		m_internal->zAxis.z(),
		//		-m_internal->zAxis.dot(m_internal->center)
		//		);
		//}
	}

}