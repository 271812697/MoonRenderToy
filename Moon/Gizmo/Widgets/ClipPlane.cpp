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
		if (renderer->gizmoSpherePlaneTranslationBehavior(renderer->makeId("clip"), m_internal->center, radius, m_internal->normal, 0.0, &pos)) {
			//m_internal->center = pos;
		}
		//RotationMatrixX(m_internal->normal);
		//renderer->gizmoOperateNormalBehavior(renderer->makeId("clipNormal"), m_internal->center, m_internal->center + m_internal->normal * radius * 2.0f, radius/2, &m_internal->normal);
		//drawOneMesh(Eigen::Vector3f & translation, Eigen::Matrix3f & rotation, Eigen::Vector3f & scale, const std::string & mesh, bool longterm = false);
		renderer->drawOneMesh(
			m_internal->center,
			RotationMatrixX(m_internal->normal),
			Eigen::Vector3f{ 0.1f,0.1f,0.1f },
			"GizmoAxis");
		//renderer->drawPlaneGrid(
		//	m_internal->center,
		//	m_internal->normal,
		//	1.0,
		//	m_internal->extent);
		//(unsigned int _id, const Eigen::Vector3f& _origin, const Eigen::Vector3f& _axis, float _snap, float _worldRadius, float _worldSize, float* _out_)
		static float angle = 1.0f;
		renderer->gizmoAxislAngleBehavior(renderer->makeId("planeEdite"), 
			m_internal->center, m_internal->normal,0,7,1,&angle);
		renderer->gizmoAxislAngleDraw(
			renderer->makeId("planeEdite"), m_internal->center, m_internal->normal, 7,
			angle, { 255,255,255,255 }, 1.0);
		bool ret=renderer->planeEdit(renderer->makeId("planeEdit"), m_internal->center, m_internal->normal);
		//renderer->planeEdit();
		if (ret) {
			auto& feature=m_sceneView->GetRenderer().GetFeature<::Core::Rendering::EngineBufferRenderFeature>();
			
			feature.SetClipPlane(
				m_internal->normal.x(),
				m_internal->normal.y(),
				m_internal->normal.z(),
				-m_internal->normal.dot(m_internal->center)
				);
		}
	}

}