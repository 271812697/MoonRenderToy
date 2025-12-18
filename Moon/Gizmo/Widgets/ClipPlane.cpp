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
			cirleDetectRadius = extent / 2;
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
		float cirleDetectRadius = 5.0f;
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
		bool ret = false;
		Eigen::Vector3f pos= m_internal->center;
		float radius=renderer->pixelsToWorldSize(m_internal->center,48);
		float worldHeight = renderer->pixelsToWorldSize(m_internal->center, 170);
		float dis=renderer->pixelsToWorldSize(m_internal->center, 30);

		renderer->drawOneMesh(
			m_internal->center,
			RotationMatrix(m_internal->xAxis,m_internal->yAxis,m_internal->zAxis),
			Eigen::Vector3f{ 0.1f,0.1f,0.1f },
			"GizmoAxis");
		
		Eigen::Vector3f scenter = m_internal->center + m_internal->yAxis* worldHeight;
		float sRadius = renderer->pixelsToWorldSize(scenter, 10);
        renderer->drawSphereFilled(scenter, sRadius);
		if (renderer->gizmoSphereRotateInCircleBehavior(renderer->makeId("planeEditz"),
			m_internal->center, sRadius, m_internal->zAxis,
			&scenter
		)) {
			ret = true;
			m_internal->yAxis = (scenter - m_internal->center).normalized();
			m_internal->xAxis = m_internal->yAxis.cross(m_internal->zAxis);
		}

		scenter = m_internal->center + m_internal->xAxis* worldHeight;
		sRadius = renderer->pixelsToWorldSize(scenter, 10);
		renderer->drawSphereFilled(scenter, sRadius);
		if (renderer->gizmoSphereRotateInCircleBehavior(renderer->makeId("planeEdity"),
			m_internal->center, sRadius, m_internal->yAxis,
			&scenter
		)) {
			ret = true;
			m_internal->xAxis = (scenter - m_internal->center).normalized();
			m_internal->zAxis = m_internal->xAxis.cross(m_internal->yAxis);
		}

		scenter = m_internal->center + m_internal->zAxis * worldHeight;
		sRadius=renderer->pixelsToWorldSize(scenter, 10);
		renderer->drawSphereFilled(scenter, sRadius);
		if (renderer->gizmoSphereRotateInCircleBehavior(renderer->makeId("planeEditx"),
			m_internal->center, sRadius, m_internal->xAxis,
			&scenter
		)) {
			ret = true;
			m_internal->zAxis = (scenter - m_internal->center).normalized();
			m_internal->yAxis = m_internal->zAxis.cross(m_internal->xAxis);
		}

		Eigen::Vector3f po = m_internal->center + m_internal->xAxis * radius + m_internal->zAxis * radius;
		renderer->drawPoint(po,10);
		ret|=renderer->gizmoPlaneTranslationBehavior(
			renderer->makeId("xz"), 
			po,
			m_internal->yAxis,0,dis, &m_internal->center);

		po = m_internal->center + m_internal->yAxis * radius + m_internal->zAxis * radius;
		renderer->drawPoint(po, 10);
		ret|=renderer->gizmoPlaneTranslationBehavior(
			renderer->makeId("yz"),
			po,
			m_internal->xAxis, 0,dis, &m_internal->center);

		po = m_internal->center + m_internal->xAxis * radius + m_internal->yAxis * radius;
		renderer->drawPoint(po, 10);
		renderer->gizmoPlaneTranslationBehavior(
			renderer->makeId("xy"),
			po,
			m_internal->zAxis, 0, dis, &m_internal->center);
		
		//bool Gizmo::gizmoAxisTranslationBehavior(unsigned int _id, const Eigen::Vector3f & _origin,
			//const Eigen::Vector3f & _axis, float _snap, float _worldHeight, float _worldSize, Eigen::Vector3f * _out_)

		renderer->gizmoAxisTranslationBehavior(renderer->makeId("xAxis"),m_internal->center,
			m_internal->xAxis,0,renderer->pixelsToWorldSize(m_internal->center,140), 
			renderer->pixelsToWorldSize(m_internal->center, 5),&m_internal->center);

		renderer->gizmoAxisTranslationBehavior(renderer->makeId("yAxis"), m_internal->center,
			m_internal->yAxis, 0, renderer->pixelsToWorldSize(m_internal->center, 140),
			renderer->pixelsToWorldSize(m_internal->center, 5), &m_internal->center);
		ret|=renderer->gizmoAxisTranslationBehavior(renderer->makeId("zAxis"), m_internal->center,
			m_internal->zAxis, 0, renderer->pixelsToWorldSize(m_internal->center, 140),
			renderer->pixelsToWorldSize(m_internal->center, 5), &m_internal->center);



		unsigned int planeOriginCircle = renderer->makeId("planeCircle");
		auto& cirleDetectRadius = m_internal->cirleDetectRadius;
		renderer->pushAlpha(0.2);
		renderer->pushColor({255,0,255,0});
		renderer->pushEnableSorting(true);
		renderer->drawCircleFilled(m_internal->center, m_internal->zAxis, cirleDetectRadius, 40);
		renderer->popEnableSorting();
		renderer->pushSize(3.0);;
	

		renderer->drawCircle(m_internal->center, m_internal->zAxis, cirleDetectRadius, 40);
		renderer->popSize();
		renderer->popColor();
		renderer->popAlpha();
		auto& normal = m_internal->zAxis;
		auto& planeOrigin = m_internal->center;
		
		Eigen::Vector3f boxMin = Eigen::Vector3f(std::min(m_internal->boxMin.x(), planeOrigin.x()), std::min(m_internal->boxMin.y(), planeOrigin.y()), std::min(m_internal->boxMin.z(), planeOrigin.z()));
		Eigen::Vector3f boxMax = Eigen::Vector3f(std::max(m_internal->boxMax.x(), planeOrigin.x()), std::max(m_internal->boxMax.y(), planeOrigin.y()), std::max(m_internal->boxMax.z(), planeOrigin.z()));

		renderer->pushSize(3.0);;
		renderer->drawAlignedBox(boxMin, boxMax);
		renderer->popSize();
		std::vector<Eigen::Vector3f> edges(std::move(clipBox(Plane(normal, planeOrigin), boxMin, boxMax)));
		for (int i = 0; i < edges.size() / 2; i++)
		{
			renderer->drawLine(edges[2 * i], edges[2 * i + 1], 4, {255,255,255,255});
		}

		Eigen::Vector3f up = abs(normal.y()) > 0.99 ? Eigen::Vector3f(1, 0, 0) : Eigen::Vector3f(0, 1, 0);
		Eigen::Vector3f xaxis = normal.cross(up).normalized();
		Eigen::Vector3f zaxis = xaxis.cross(normal).normalized();
		unsigned int pointId[4]{ renderer->makeId("p1"), renderer->makeId("p2"), renderer->makeId("p3"), renderer->makeId("p4") };
		Eigen::Vector3f pointPos[4] = { planeOrigin + xaxis * 1.0 * cirleDetectRadius,planeOrigin - xaxis * 1.0 * cirleDetectRadius, planeOrigin + zaxis * 1.0 * cirleDetectRadius,planeOrigin - zaxis * 1.0 * cirleDetectRadius };
		Eigen::Vector3f pointDir[4] = { xaxis, -xaxis, zaxis, -zaxis };
		for (int i = 0; i < 4; i++)
		{
			renderer->drawPoint(pointPos[i], 10, renderer->isHot(pointId[i]) ? Eigen::Vector4<uint8_t>{255,0,255,255} : Eigen::Vector4<uint8_t>{255,255,255,0});
			Eigen::Vector3f outFace = pointPos[i];
			float size = renderer->pixelsToWorldSize(pointPos[i], 10);
			if (renderer->gizmoSphereAxisTranslationBehavior(pointId[i], pointPos[i], size, pointDir[i], 0, &outFace))
			{
				cirleDetectRadius = (outFace - planeOrigin).norm();
			}
		}
		
		if (ret) {
			auto& feature=m_sceneView->GetRenderer().GetFeature<::Core::Rendering::EngineBufferRenderFeature>();
			
			feature.SetClipPlane(
				m_internal->zAxis.x(),
				m_internal->zAxis.y(),
				m_internal->zAxis.z(),
				-m_internal->zAxis.dot(m_internal->center)
				);
		}
	}

}