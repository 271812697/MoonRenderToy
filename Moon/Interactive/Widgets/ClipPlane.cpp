#include "Interactive/Widgets/ClipPlane.h"
#include "Interactive/Im3DRenderer.h"
#include "Interactive/MathUtil/MathUtil.h"
#include "renderer/SceneView.h"
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include "Interactive/Interactive/ExecuteCommand.h"
#include "Interactive/Interactive/RenderWindowInteractor.h"
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
#include "Qtimgui/imgui/imgui.h"
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

	std::vector<Eigen::Vector3f> DiscretizeWire(const TopoDS_Wire& wire, double deflection = 0.1)
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
	struct PickMeshInfo {
		std::string blockName;
		ClipPlane::PickMeshId meshId;
		Maths::FVector4 blockColor;
	};
	static PickMeshInfo table[9] = {
		{"YArrow",ClipPlane::PickMeshId::YAxis,{ 0,1,0,1 }},
		{"XArrow",ClipPlane::PickMeshId::XAxis,{ 1,0,0,1 }},
		{"ZArrow",ClipPlane::PickMeshId::ZAxis,{ 0,0,1,1 }},
		{"XPlane",ClipPlane::PickMeshId::XNormalPlane,{ 1,0,0,1 }},
		{"YPlane",ClipPlane::PickMeshId::YNormalPlane,{ 0,1,0,1 }},
		{"ZPlane",ClipPlane::PickMeshId::ZNormalPlane,{ 0,0,1,1 }},
		{"XAxis",ClipPlane::PickMeshId::XNormalRotate ,{ 1,0,0,1 }},
		{"YAxis",ClipPlane::PickMeshId::YNormalRotate,{ 0,1,0,1 }},
		{"ZAxis",ClipPlane::PickMeshId::ZNormalRotate,{ 1,0,1,1 }}
	};
	static int meshInfoCnt = 9;
	Maths::FVector4 hotColor = { 0.1,1.0,1.0,1 };
	Maths::FVector4 activeColor = { 1,1,0,1 };
	class GizmoAxisRotate
	{
	public:
		static constexpr float PLANE_DOT_EPS = 0.001f;
		static constexpr float RAY_T_MIN = 0.001f;
		static constexpr float PROJ_LEN_EPS = 1e-6f;
		static constexpr float ANGLE_EPS = 1e-6f;
		static constexpr int   DEFAULT_SEG = 32;

		GizmoAxisRotate() = default;
		GizmoAxisRotate(const Eigen::Vector3f& Axis, const Eigen::Vector3f& center)
			: m_axis(Axis.normalized()), m_origin(center)
		{
		}

		void startPick(const Eigen::Vector3f& Axis, const Eigen::Vector3f& center,
			const Eigen::Vector3f& srcDir, const Eigen::Vector3f& srcPos)
		{
			m_axis = Axis.normalized();
			m_origin = center;
			m_refDir = srcDir;
			m_refPos = srcPos;

			m_totalAngle = 0.f;
			m_firstPick = true;
			m_mouseStart.setZero();
			m_lastProj.setZero();
		}

		bool applyDir(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& outDir)
		{
			Eigen::Vector3f currProj;
			Eigen::Vector3f currHit;
			if (!computePlaneProj(ray, eye, currProj, currHit))
				return false;

			if (m_firstPick)
			{
				m_mouseStart = currProj;
				m_lastProj = currProj;
				
				m_firstPick = false;
				m_refPos = currHit;
				outDir = m_refDir;
				m_totalAngle = 0.f;
				return true;
			}

			// 计算单帧增量（相邻两帧夹角，永远不会超过半圈）
			float delta = computeAngle(m_lastProj, currProj);
			m_totalAngle += delta;
			m_totalAngle = fmod(m_totalAngle, 3.14159265358979323846f *2);

			float snapAngle = m_totalAngle;
			//degree snap
			if (m_enableSnap) {
				int degree=m_totalAngle * 180 / 3.14159265358979323846f;
				snapAngle = degree * 3.14159265358979323846f / 180.0f;
			}


			m_lastProj = currProj;

			Eigen::AngleAxisf rot(snapAngle, m_axis);
			outDir = rot * m_refDir;
			return true;
		}

		bool applyPos(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& outPos)
		{
			Eigen::Vector3f currProj;
			Eigen::Vector3f currHit;
			if (!computePlaneProj(ray, eye, currProj,currHit))
				return false;

			if (m_firstPick)
			{
				m_mouseStart = currProj;
				m_lastProj = currProj;
				m_firstPick = false;
				m_refPos = currHit;
				outPos = m_refPos;
				m_totalAngle = 0.f;
				return true;
			}

			float delta = computeAngle(m_lastProj, currProj);
			m_totalAngle += delta;
			m_lastProj = currProj;

			float snapAngle = m_totalAngle;
			//degree snap
			if (m_enableSnap) {
				int degree = m_totalAngle * 180 / 3.14159265358979323846f;
				snapAngle = degree * 3.14159265358979323846f / 180.0f;
			}

			Eigen::AngleAxisf rot(snapAngle, m_axis);
			Eigen::Vector3f offset = m_refPos - m_origin;
			outPos = m_origin + rot * offset;
			return true;
		}

		std::vector<Eigen::Vector3f> getRotationArc(int segCount = DEFAULT_SEG) const
		{
			std::vector<Eigen::Vector3f> arcPoints;
			if (segCount < 2 || std::fabs(m_totalAngle) < ANGLE_EPS)
				return arcPoints;

			float snapAngle = m_totalAngle;
			//degree snap
			if (m_enableSnap) {
				int degree = m_totalAngle * 180 / 3.14159265358979323846f;
				snapAngle = degree * 3.14159265358979323846f / 180.0f;
			}

			Eigen::Vector3f baseOffset = m_refPos - m_origin;
			float step = snapAngle / static_cast<float>(segCount);
			for (int i = 0; i <= segCount; ++i)
			{
				float ang = step * static_cast<float>(i);
				Eigen::AngleAxisf rotStep(ang, m_axis);
				arcPoints.push_back(m_origin + rotStep * baseOffset);
			}
			return arcPoints;
		}

		float getRotationAngle() const {
			//degree snap
			if (m_enableSnap) {
				int degree = m_totalAngle * 180 / 3.14159265358979323846f;
				return  degree * 3.14159265358979323846f / 180.0f;
			}
			return m_totalAngle; 
		}
		void enableSnap(bool flag) {
			m_enableSnap = flag;
		}
	private:
		bool computePlaneProj(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& outProj,Eigen::Vector3f& hitPos) const
		{
			const Eigen::Vector3f n = m_axis;
			const Eigen::Vector3f planePt = m_origin;

			float denom = ray.dot(n);
			if (std::abs(denom) <= PLANE_DOT_EPS)
				return false;

			float t = (planePt - eye).dot(n) / denom;
			if (t <= RAY_T_MIN)
				return false;

			Eigen::Vector3f hit = eye + ray * t;
			Eigen::Vector3f hitRel = hit - planePt;

			outProj = hitRel - n * (hitRel.dot(n));
			float projLen = outProj.norm();
			if (projLen < PROJ_LEN_EPS)
				return false;
			outProj /= projLen;
			hitPos = hit;
			
			return true;
		}

		float computeAngle(const Eigen::Vector3f& a, const Eigen::Vector3f& b) const
		{
			float dot = a.dot(b);
			float crossSign = m_axis.dot(a.cross(b));
			return std::atan2(crossSign, dot);
		}

	private:
		Eigen::Vector3f m_axis = Eigen::Vector3f::UnitX();
		Eigen::Vector3f m_origin = Eigen::Vector3f::Zero();

		Eigen::Vector3f m_refDir;
		Eigen::Vector3f m_refPos;
		Eigen::Vector3f m_mouseStart;
		Eigen::Vector3f m_lastProj; // 新增：缓存上一帧鼠标投影

		float m_totalAngle = 0.f;   // 累积总角，范围无限制
		bool m_firstPick = true;
		bool m_enableSnap = true;
	};
	class GizmoPlaneTranslate
	{
	public:
		GizmoPlaneTranslate() = default;
		GizmoPlaneTranslate(const Eigen::Vector4f& planeEq, const Eigen::Vector3f& pos)
			: plane(planeEq), origin(pos) {
		}

		void startPick(const Eigen::Vector4f& planeEq, const Eigen::Vector3f& pos)
		{
			plane = planeEq;
			origin = pos;
			firstPick = true;
		}

		void apply(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye, Eigen::Vector3f& pos)
		{
			Eigen::Vector3f n(plane.x(), plane.y(), plane.z());
			float d = plane.w();

			float denom = ray.dot(n);
			if (std::abs(denom) <= 0.001f)
				return;

			float t = -(eye.dot(n) + d) / denom;
			if (t <= 0.001f)
				return;

			Eigen::Vector3f hit = eye + ray * t;
			if (firstPick)
			{
				mOffset = origin - hit;
				firstPick = false;
			}
			pos = hit + mOffset;
		}

	private:
		Eigen::Vector4f plane = Eigen::Vector4f(0, 0, 1, 0);
		Eigen::Vector3f origin = Eigen::Vector3f::Zero();
		Eigen::Vector3f mOffset = Eigen::Vector3f::Zero();
		bool firstPick = true;
	};
	class GizmoAxisTranslate {
	public:
		GizmoAxisTranslate() = default;
		GizmoAxisTranslate(const Eigen::Vector3f&Axis, const Eigen::Vector3f& pos):axis(Axis),origin(pos) {}
		void apply(const Eigen::Vector3f& ray, const Eigen::Vector3f& eye,Eigen::Vector3f&pos) {
			 Eigen::Vector3f planeTangent=axis.cross(pos-eye);
			 Eigen::Vector3f planeNormal = axis.cross(planeTangent);
			 Eigen::Vector3f planePoint = origin;
			 float denom = ray.dot(planeNormal);
			 if (std::abs(denom) <= 0.001f)
				 return;
			 float t = (planePoint - eye).dot(planeNormal) / denom;
			 if (t <= 0.001f)
				 return;
			 Eigen::Vector3f point = eye + ray * t;

			 if (firstPick)
			 {
				 mInitialOffset = origin - point;
				 firstPick = false;
			 }
			 Eigen::Vector3f translationVector = point - planePoint + mInitialOffset;
			 pos = planePoint + axis*translationVector.dot(axis);
		}
		void startPick(const Eigen::Vector3f& Axis, const Eigen::Vector3f& pos) {
			axis = Axis;
			origin = pos;
			firstPick = true;
		}
	private:
		Eigen::Vector3f axis = { 1,0,0 };
		Eigen::Vector3f origin = { 0,0,0 };
		Eigen::Vector3f mInitialOffset = { 0,0,0 };
		bool firstPick = true;
	};
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
				auto acptr=mSelf->m_sceneView->GetSelectedActor();
				while (!acptr->HasComponent("Model Renderer")&&acptr->HasParent()) {
					acptr = acptr->GetParent();
				}

				if (acptr&&acptr!=ac) {
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

		bool updateEngineUbo = false;
		GizmoAxisTranslate transLatePick;
		GizmoPlaneTranslate planeTPick;
		GizmoAxisRotate axisRPick;
	};
	
	ClipPlane::ClipPlane(const std::string& name) :EventWidget(name)
	, m_internal(new ClipPlaneInternal(this)),mState(Stop){
	
	}
	ClipPlane::~ClipPlane()
	{
		delete m_internal;
	}
	void ClipPlane::onUpdate()
	{
		if (mState == AxisR)
		{
			float angle=m_internal->axisRPick.getRotationAngle();
			float degree = -angle * 180 / 3.14159265358979323846f;
			std::string text = std::to_string(degree) + " degree\n";
			auto pos=renderer->getFrameParam().cursor;
			ImGui::GetForegroundDrawList()->AddText({ pos.x(),pos.y()-20 }, IM_COL32(255, 255, 0, 255), text.c_str());
			auto seg = m_internal->axisRPick.getRotationArc();
			if (seg.size() > 0) {
				renderer->pushColor({ 255,255,255,0 });
			
				for (int i = 0; i < seg.size()-1; i++) {
					renderer->drawLine(seg[i],seg[i+1],5);
				}
				renderer->popColor();
				renderer->pushColor({255,0,255,255});
				renderer->drawLine(seg[0], m_internal->center, 4);
				renderer->drawLine(seg.back(), m_internal->center, 4);
				renderer->popColor();
			}
		}
		bool ret = true;
		Eigen::Vector3f pos= m_internal->center;
		float radius=renderer->pixelsToWorldSize(m_internal->center,48);
		float worldHeight = renderer->pixelsToWorldSize(m_internal->center, 170);
		float dis=renderer->pixelsToWorldSize(m_internal->center, 30);
		renderer->drawOneMesh(
			m_internal->center,
			RotationMatrix(m_internal->xAxis,m_internal->yAxis,m_internal->zAxis),
			Eigen::Vector3f{ 0.1f,0.1f,0.1f },
			"TransformAxis");
		unsigned int planeOriginCircle = renderer->makeId("planeCircle");
		auto& cirleDetectRadius = m_internal->cirleDetectRadius;

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

		if (m_internal->updateEngineUbo)
		{
			m_internal->updateEngineUbo = false;
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

	void ClipPlane::onLeftMousePressed()
	{
		if (mState == Hot) {
			
			bool active = false;
			if (table[mPickMesh].meshId == PickMeshId::YAxis|| table[mPickMesh].meshId == PickMeshId::XAxis|| table[mPickMesh].meshId == PickMeshId::ZAxis) {
				mState = AxisT;
				Eigen::Vector3f axis = table[mPickMesh].meshId == PickMeshId::YAxis ?
					m_internal->yAxis : (table[mPickMesh].meshId == PickMeshId::XAxis ? m_internal->xAxis : m_internal->zAxis);
				m_internal->transLatePick.startPick(axis,m_internal->center);
				active = true;
			}
			else if (table[mPickMesh].meshId == PickMeshId::XNormalPlane || table[mPickMesh].meshId == PickMeshId::YNormalPlane || table[mPickMesh].meshId == PickMeshId::ZNormalPlane) {
				mState = PlaneT;
				Eigen::Vector3f normal= table[mPickMesh].meshId == PickMeshId::XNormalPlane ?
					m_internal->xAxis : (table[mPickMesh].meshId == PickMeshId::YNormalPlane ? m_internal->yAxis : m_internal->zAxis);
				float w=-m_internal->center.dot(normal);
				Eigen::Vector4f planeEqu = { normal.x(),normal.y(),normal.z(),w };
				m_internal->planeTPick.startPick(planeEqu,m_internal->center);
				active = true;
			}
			else if (table[mPickMesh].meshId == PickMeshId::XNormalRotate || table[mPickMesh].meshId == PickMeshId::YNormalRotate || mPickMesh == PickMeshId::ZNormalRotate) {
				mState = AxisR;
				Eigen::Vector3f normal = table[mPickMesh].meshId == PickMeshId::XNormalRotate?
					m_internal->yAxis : (table[mPickMesh].meshId == PickMeshId::YNormalRotate? m_internal->zAxis : m_internal->xAxis);
				Eigen::Vector3f refDir = table[mPickMesh].meshId == PickMeshId::XNormalRotate ?
					m_internal->xAxis : (table[mPickMesh].meshId == PickMeshId::YNormalRotate ? m_internal->yAxis : m_internal->zAxis);
				
				m_internal->axisRPick.startPick(normal,m_internal->center,refDir, m_internal->center+refDir);
				active = true;
			}
			if (active) {
				TransformAxis().setBlockColor(TransformAxis().getBlockId(table[mPickMesh].blockName), activeColor);
			}
		}
	}

	void ClipPlane::onLeftMouseReleased()
	{
		if (mState == AxisT||mState==PlaneT||mState==AxisR) {
			mState = Hot;
			TransformAxis().setBlockColor(TransformAxis().getBlockId(table[mPickMesh].blockName), hotColor);
			PickMeshId id=table[mPickMesh].meshId;
			bool updateFlag = (id != PickMeshId::XAxis)&& (id != PickMeshId::YAxis)&&
				(id != PickMeshId::ZNormalPlane)&&(id != PickMeshId::YNormalRotate);
			if (updateFlag) {
				updateSection();			
			}
		}
	}

	void ClipPlane::onMouseMove()
	{
		if (mState == Stop) {
			bool selectFlag = false;
			mPickMesh = PickMeshId::None;
			for (int i = 0;i < meshInfoCnt;i++) {
				if (renderer->isSelectPolygon("TransformAxis", table[i].blockName)) {
					mPickMesh = i;
					selectFlag = true;
					break;
				}
			}
			if (selectFlag) {
				mState = Hot;
				TransformAxis().setBlockColor(TransformAxis().getBlockId(table[mPickMesh].blockName), hotColor);
			}
		}
		else if (mState == Hot) {
			bool selectFlag = false;
			for (int i = 0;i < meshInfoCnt;i++) {
				if (renderer->isSelectPolygon("TransformAxis", table[i].blockName)) {
					selectFlag = true;
					break;
				}
			}
			if (!selectFlag) {
				mState = Stop;
				TransformAxis().setBlockColor(TransformAxis().getBlockId(table[mPickMesh].blockName), table[mPickMesh].blockColor);
			    mPickMesh = PickMeshId::None;
			}
		}
		else if (mState == AxisT) {
			auto&param=renderer->getFrameParam();
			m_internal->transLatePick.apply(param.rayDirection,param.rayOrigin,m_internal->center);
		}
		else if (mState==PlaneT) {
			auto& param = renderer->getFrameParam();
			m_internal->planeTPick.apply(param.rayDirection, param.rayOrigin, m_internal->center);
		}
		else if (mState==AxisR) {
			auto& param = renderer->getFrameParam();
			Eigen::Vector3f dir = table[mPickMesh].meshId == PickMeshId::XNormalRotate ?
				m_internal->yAxis : (table[mPickMesh].meshId == PickMeshId::YNormalRotate ? m_internal->zAxis : m_internal->xAxis);
			m_internal->axisRPick.applyDir(param.rayDirection, param.rayOrigin,dir);
			if (table[mPickMesh].meshId == PickMeshId::XNormalRotate) {
				m_internal->xAxis = dir;
				m_internal->zAxis = m_internal->xAxis.cross(m_internal->yAxis);
			}
			else if (table[mPickMesh].meshId == PickMeshId::YNormalRotate) {
				m_internal->yAxis = dir;
				m_internal->xAxis = m_internal->yAxis.cross(m_internal->zAxis);
			}
			else if (table[mPickMesh].meshId == PickMeshId::ZNormalRotate) {
				m_internal->zAxis = dir;
				m_internal->yAxis = m_internal->zAxis.cross(m_internal->xAxis);
			}
		}
	}

	void ClipPlane::updateSection()
	{
		m_internal->updateEngineUbo = true;
		Core::ECS::Actor* selectActor = nullptr;
		if (m_sceneView->IsSelectActor()) {
			selectActor = m_sceneView->GetSelectedActor();
			while (!selectActor->HasComponent("CTopoShape") && selectActor->HasParent())
			{
				selectActor = selectActor->GetParent();
			}
		}
		else
		{
			selectActor = m_sceneView->GetScene()->FindActorByTag("TopoShape");
		}

		if (selectActor)
		{
			Core::ECS::Components::CTopoShape* topoComp = selectActor->GetComponent<Core::ECS::Components::CTopoShape>();

			if (topoComp) {
				auto& topoShape = topoComp->GetTopoShape();
				double offset = m_internal->zAxis.dot(m_internal->center);
				Base::Vector3d dir{ m_internal->zAxis.x(), m_internal->zAxis.y() , m_internal->zAxis.z() };
				auto wires = topoShape.slice(dir, offset);
				m_internal->sectionFace = DiscretizeSectionFace(wires);
				m_internal->slicelines.clear();
				for (auto& w : wires) {
					auto tempLine = DiscretizeWire(w);
					m_internal->slicelines.insert(m_internal->slicelines.end(),
						tempLine.begin(), tempLine.end()
					);
				}
			}
		}
	}
}