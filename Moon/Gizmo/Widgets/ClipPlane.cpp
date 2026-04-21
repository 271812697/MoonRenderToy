#include "Gizmo/Widgets/ClipPlane.h"
#include "Gizmo/Gizmo.h"
#include "Gizmo/MathUtil/MathUtil.h"
#include "renderer/SceneView.h"
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include "Gizmo/Interactive/ExecuteCommand.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
#include "core/component/CTopoShape.h"
#include "TopoShape.h"
#include <TopoDS_Wire.hxx>
#include <BRepAdaptor_CompCurve.hxx>
#include <GCPnts_UniformAbscissa.hxx>
#include <gp_Pnt.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Compound.hxx>
#include <BRep_Builder.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <TopoDS.hxx>
#include <TopExp_Explorer.hxx>
#include <BRepLib_FindSurface.hxx>
namespace MOON {
	// 1. 从Wire列表构建截面面（无Compound，无编译错误）
	TopoDS_Face BuildSectionFace(const std::list<TopoDS_Wire>& wireList)
	{
		if (wireList.empty())
			return TopoDS_Face();

		// 取第一个wire创建基础面（平面切片专用）
		const TopoDS_Wire& firstWire = wireList.front();
		BRepBuilderAPI_MakeFace faceMaker(firstWire);

		if (!faceMaker.IsDone())
			return TopoDS_Face();

		// 添加其余wire（内孔/子轮廓）
		for (auto it = std::next(wireList.begin()); it != wireList.end(); ++it)
		{
			faceMaker.Add(*it);
		}

		return faceMaker.Face();
	}

	std::vector<Eigen::Vector3f> DiscretizeWire(const TopoDS_Wire& wire, double deflection = 0.01)
	{
		std::vector<Eigen::Vector3f> points;
		BRepAdaptor_CompCurve curve(wire, Standard_True);
		//if (!curve.IsValid())
			//return points;

		GCPnts_UniformAbscissa discretor(curve, deflection);
		if (!discretor.IsDone())
			return points;

		Standard_Integer nbPoints = discretor.NbPoints();
		if (nbPoints < 2)
			return points;

		// 先取第一个点
		gp_Pnt p_prev;
		curve.D0(discretor.Parameter(1), p_prev);

		for (Standard_Integer i = 2; i <= nbPoints; ++i)
		{
			gp_Pnt p_curr;
			curve.D0(discretor.Parameter(i), p_curr);

			// 每一段线：存 起点 + 终点
			points.push_back({ (float)p_prev.X(), (float)p_prev.Y(), (float)p_prev.Z() });
			points.push_back({ (float)p_curr.X(), (float)p_curr.Y(), (float)p_curr.Z() });

			p_prev = p_curr;
		}

		return points;
	}
	std::vector<Eigen::Vector3f> DiscretizeSectionFace(const std::list<TopoDS_Wire>& wires, double deflection = 0.1) {
	
		std::vector<Eigen::Vector3f> vertices;

		TopoDS_Face face = BuildSectionFace(wires);

		// 4. 对截面面进行三角网格化
		BRepMesh_IncrementalMesh mesh(face, deflection, Standard_True);
		if (!mesh.IsDone()) return vertices;

		// 5. 提取所有三角面顶点（三点一组 = 一个面）
		TopExp_Explorer explorer(face, TopAbs_FACE);
		for (; explorer.More(); explorer.Next())
		{
			TopoDS_Face f = TopoDS::Face(explorer.Current());
			TopLoc_Location loc;
			Handle(Poly_Triangulation) triangulation = BRep_Tool::Triangulation(f,loc);
			if (triangulation.IsNull()) continue;

			// 遍历所有三角形
			Standard_Integer nbTriangles = triangulation->NbTriangles();
			for (Standard_Integer i = 1; i <= nbTriangles; ++i)
			{
				const Poly_Triangle& tri = triangulation->Triangle(i);
				Standard_Integer n1 = tri(1);
				Standard_Integer n2 = tri(2);
				Standard_Integer n3 = tri(3);

				gp_Pnt p1 = triangulation->Node(n1);
				gp_Pnt p2 = triangulation->Node(n2);
				gp_Pnt p3 = triangulation->Node(n3);

				// 加入顶点：三点一组
				vertices.push_back({ (float)p1.X(), (float)p1.Y(), (float)p1.Z() });
				vertices.push_back({ (float)p2.X(), (float)p2.Y(), (float)p2.Z() });
				vertices.push_back({ (float)p3.X(), (float)p3.Y(), (float)p3.Z() });
			}
		}

		return vertices;
	}
	class ClipPlane::ClipPlaneInternal {
	public:
		ClipPlaneInternal(ClipPlane* clip):mSelf(clip) {
			clickObserver = mSelf->Interactor->AddObserver(ExecuteCommand::LeftButtonReleaseEvent, this, &ClipPlane::ClipPlaneInternal::onMouseLeftClick, 0.0f);		
			mSelf->Interactor;
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
		std::vector<Eigen::Vector3f>slicelines;
		std::vector<Eigen::Vector3f>sectionFace;
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
		mPreflag = mCurflag;
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
		if (renderer->isHot(renderer->makeId("planeEditz"))) {
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("YAxis"), { 1,1,0,1 });
		}
		else
		{
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("YAxis"), { 0,1,0,1 });
		}
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
		if (renderer->isHot(renderer->makeId("planeEdity"))) {
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("XAxis"), { 1,1,0,1 });
		}
		else
		{
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("XAxis"), { 1,0,0,1 });
		}
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
		if (renderer->isHot(renderer->makeId("planeEditx"))) {
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("ZAxis"), { 1,1,0,1 });
		}
		else
		{
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("ZAxis"), { 0,0,1,1 });
		}
		if (renderer->gizmoSphereRotateInCircleBehavior(renderer->makeId("planeEditx"),
			m_internal->center, sRadius, m_internal->xAxis,
			&scenter
		)) {
			ret = true;
			m_internal->zAxis = (scenter - m_internal->center).normalized();
			m_internal->yAxis = m_internal->zAxis.cross(m_internal->xAxis);
		}

		if (renderer->isHot(renderer->makeId("xz"))) {
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("YPlane"), { 1,1,0,0.7 });
		}
		else
		{
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("YPlane"), { 0,1,0,0.2 });
		}
		Eigen::Vector3f po = m_internal->center + m_internal->xAxis * radius + m_internal->zAxis * radius;
		renderer->drawPoint(po, 10);
		ret|=renderer->gizmoPlaneTranslationBehavior(
			renderer->makeId("xz"), 
			po,
			m_internal->yAxis,0,dis, &m_internal->center);

		po = m_internal->center + m_internal->yAxis * radius + m_internal->zAxis * radius;
		renderer->drawPoint(po, 10);
		if (renderer->isHot(renderer->makeId("yz"))) {
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("XPlane"), { 1,1,0,0.7 });
		}
		else
		{
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("XPlane"), { 1,0,0,0.2 });
		}
		ret|=renderer->gizmoPlaneTranslationBehavior(
			renderer->makeId("yz"),
			po,
			m_internal->xAxis, 0,dis, &m_internal->center);

		po = m_internal->center + m_internal->xAxis * radius + m_internal->yAxis * radius;
		renderer->drawPoint(po, 10);
		if (renderer->isHot(renderer->makeId("xy"))) {
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("ZPlane"), { 1,1,0,0.7 });
		}
		else
		{
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("ZPlane"), { 0,0,1,0.2 });
		}
		renderer->gizmoPlaneTranslationBehavior(
			renderer->makeId("xy"),
			po,
			m_internal->zAxis, 0, dis, &m_internal->center);
		
		//bool Gizmo::gizmoAxisTranslationBehavior(unsigned int _id, const Eigen::Vector3f & _origin,
			//const Eigen::Vector3f & _axis, float _snap, float _worldHeight, float _worldSize, Eigen::Vector3f * _out_)
		renderer->drawPoint(po, 10);
		if (renderer->isHot(renderer->makeId("xAxis"))) {
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("XArrow"), { 1,1,0,1 });
		}
		else
		{
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("XArrow"), { 1,0,0,1 });
		}
		renderer->gizmoAxisTranslationBehavior(renderer->makeId("xAxis"),m_internal->center,
			m_internal->xAxis,0,renderer->pixelsToWorldSize(m_internal->center,140), 
			renderer->pixelsToWorldSize(m_internal->center, 5),&m_internal->center);
		if (renderer->isHot(renderer->makeId("yAxis"))) {
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("YArrow"), { 1,1,0,1 });
		}
		else
		{
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("YArrow"), { 0,1,0,1 });
		}
		renderer->gizmoAxisTranslationBehavior(renderer->makeId("yAxis"), m_internal->center,
			m_internal->yAxis, 0, renderer->pixelsToWorldSize(m_internal->center, 140),
			renderer->pixelsToWorldSize(m_internal->center, 5), &m_internal->center);
		if (renderer->isHot(renderer->makeId("zAxis"))) {
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("ZArrow"), { 1,1,0,1 });
		}
		else
		{
			GizmoAxis().setBlockColor(GizmoAxis().getBlockId("ZArrow"), { 0,0,1,1 });
		}
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
		renderer->drawLineList(m_internal->slicelines,4, { 255,255,0,255 });
		renderer->pushAlpha(0.6);
		renderer->pushEnableSorting(true);
		renderer->drawTriangleList(m_internal->sectionFace,4,{ 255,255,215,255 });
		renderer->popAlpha();
		renderer->popEnableSorting();
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
		mCurflag = ret;
		if (mPreflag&&!mCurflag) {
				auto selectActor = m_sceneView->GetScene()->FindActorByTag("TopoShape");
				if (selectActor) {
					const auto& topoComp = selectActor->GetComponent<Core::ECS::Components::CTopoShape>();
					if (topoComp) {
						auto& topoShape= topoComp->GetTopoShape();
						double offset = m_internal->zAxis.dot(m_internal->center);
						Base::Vector3d dir{ m_internal->zAxis.x(), m_internal->zAxis.y() , m_internal->zAxis.z() };
						auto wires=topoShape.slice(dir,offset);
						m_internal->sectionFace=DiscretizeSectionFace(wires);
						
						m_internal->slicelines.clear();
						for (auto& w : wires) {
							auto tempLine=DiscretizeWire(w);
							m_internal->slicelines.insert(m_internal->slicelines.end(),
								tempLine.begin(),tempLine.end()
								);
						}
					}
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

	Maths::FVector4 ClipPlane::getClipPlane()
	{
		return Maths::FVector4(
			m_internal->zAxis.x(),
			m_internal->zAxis.y(),
			m_internal->zAxis.z(),
			m_internal->zAxis.dot(m_internal->center)
		);
	}

}