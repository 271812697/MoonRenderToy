#include "Interactive/Widgets/SketchPlane.h"
#include "Interactive/Im3DRenderer.h"
#include "Interactive/MathUtil/MathUtil.h"
#include "renderer/SceneView.h"
#include "Interactive/Interactive/ExecuteCommand.h"
#include "Interactive/Interactive/RenderWindowInteractor.h"
#include "Sketcher/SketcherObjManager.h"
#include "Sketcher/SketcherObj.h"

namespace MOON {
	class SketchPlane::SketchPlaneInternal {
	public:
		SketchPlaneInternal(SketchPlane* clip):mSelf(clip) {
			mSelf->Interactor;
		}
		~SketchPlaneInternal() {
		}

		bool onMouseLeftClick() {
			if (mSelectPlane != mPreSelectPlane) {
				mSelectPlane = mPreSelectPlane;
				if (mSelectPlane != NO_Plane) {
				
					if (mSelectPlane == XY_Plane) {
						plane = SketcherPlane2D();
					}
					if (mSelectPlane == XZ_Plane) {
						plane.xAxis = Base::Vector3d{ 1,0,0 };
						plane.yAxis = Base::Vector3d{ 0,0,-1 };
						plane.normal = Base::Vector3d{ 0,1,0 };
						
					}
					if (mSelectPlane == YZ_Plane) {
						plane.xAxis = Base::Vector3d{ 0,0,-1 };
						plane.yAxis = Base::Vector3d{ 0,1,0 };
						plane.normal = Base::Vector3d{ 1,0,0 };
						
					}
					mSelf->InvokeEvent(SketchPlaneEvent::SelectPlane);
					mSelf->setActive(false);
					mSelectPlane = NO_Plane;
					return true;
				}
			}
			return false;
		}
	private:
		friend class SketchPlane;
		SketchPlane* mSelf = nullptr;
		Plane mSelectPlane{ NO_Plane };
		Plane mPreSelectPlane{ NO_Plane };
		SketcherPlane2D plane;
	};

	SketchPlane::SketchPlane(const std::string& name) :EventWidget(name)
	, m_internal(new SketchPlaneInternal(this)){
		setActive(true);
	}
	SketchPlane::~SketchPlane()
	{
		delete m_internal;
	}
	void SketchPlane::onUpdate()
	{
		
		m_internal->mPreSelectPlane = Plane::NO_Plane;
	
		renderer->drawOneMesh(
			{0,0,0},
			Eigen::Matrix3f::Identity(),
			Eigen::Vector3f{ 0.2f,0.2f,0.2f },
			"GizmoSketchPlane");

		if (renderer->isSelectPolygon("GizmoSketchPlane", "YPlane")|| renderer->isSelectPolygon("GizmoSketchPlane", "YArrow")) {
		    m_internal->mPreSelectPlane = Plane::XZ_Plane;
		}
		if (renderer->isSelectPolygon("GizmoSketchPlane", "XPlane") || renderer->isSelectPolygon("GizmoSketchPlane", "XArrow")) {
			m_internal->mPreSelectPlane = Plane::YZ_Plane;
		}
		if (renderer->isSelectPolygon("GizmoSketchPlane", "ZPlane") || renderer->isSelectPolygon("GizmoSketchPlane", "ZArrow")) {			
			m_internal->mPreSelectPlane = Plane::XY_Plane;
		}


		if (m_internal->mPreSelectPlane ==Plane::XY_Plane) {
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("ZPlane"), { 1,1,0,0.7 });
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("ZArrow"), { 1,1,0,1.0 });
		}
		else
		{
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("ZPlane"), { 1,1,1,1 });
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("ZArrow"), { 0,0,1,1.0 });
		}
		if (m_internal->mPreSelectPlane == Plane::YZ_Plane) {	
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("XPlane"), { 1,1,0,0.7 });
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("XArrow"), { 1,1,0,1.0 });
		}
		else
		{		
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("XPlane"), { 1,1,1,1 });
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("XArrow"), { 1,0,0,1.0 });
		}
		if (m_internal->mPreSelectPlane == Plane::XZ_Plane) {
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("YPlane"), { 1,1,0,0.7 });
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("YArrow"), { 1,1,0,1.0 });
		}
		else
		{
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("YPlane"), { 1,1,1,1 });
			GizmoSketchPlane().setBlockColor(GizmoSketchPlane().getBlockId("YArrow"), { 0,1,0,1 });
		}
	}

	void SketchPlane::onLeftMousePressed()
	{
		m_internal->onMouseLeftClick();
	}
	void SketchPlane::onMouseMove()
	{
		//m_internal->onMouseLeftClick();
	}
	SketcherPlane2D SketchPlane::getSelectPlane()
	{
		return m_internal->plane;
	}
}