#include "Interactive/Widgets/PadTaskWidget.h"
#include "Interactive/Im3DRenderer.h"
#include "Interactive/MathUtil/MathUtil.h"
#include "Core/Global/ServiceLocator.h"
#include "Rendering/Resources/Mesh.h"
#include "Rendering/Resources/Model.h"
#include "renderer/Context.h"
#include "Interactive/GizmoBehaviour.h"
#include "renderer/SceneView.h"

namespace MOON {
	struct TriangleFace
	{
		TriangleFace(std::vector<Eigen::Vector3f>f,
			Eigen::Vector4<uint8_t> c):faces(f),color(c) {

		}
		TriangleFace(std::vector<Eigen::Vector3f>f,
			Eigen::Vector4<uint8_t> c,const Eigen::Matrix4f& m) :model(m),faces(f), color(c) {

		}
		std::vector<Eigen::Vector3f>faces;
		Eigen::Vector4<uint8_t> color;
		Eigen::Matrix4f model=Eigen::Matrix4f::Identity();
	};
	struct VertexPoint
	{
		VertexPoint(const Eigen::Vector3f& p, float s, const Eigen::Vector4<uint8_t>& c):pos(p),size(s), color(c)
		{
		
		}
		Eigen::Vector3f pos;
		float size;
		Eigen::Vector4<uint8_t> color;
	};
	struct Edge {
		Edge(const std::vector<Eigen::Vector3f>&l,const Eigen::Vector4<uint8_t>& c):lines(l),color(c) {}
		Edge( const std::vector<Eigen::Vector3f>& l, const Eigen::Vector4<uint8_t>& c,const Eigen::Matrix4f& m ) :model(m), lines(l), color(c) {}
		std::vector<Eigen::Vector3f>lines;
		Eigen::Vector4<uint8_t> color;
		Eigen::Matrix4f model = Eigen::Matrix4f::Identity();
	};
	class  WidgetViewData {
	public:
		WidgetViewData() = default;
		~WidgetViewData() {

		}
		int addPoint(const Eigen::Vector3f& pos, float size, const Eigen::Vector4<uint8_t>& color) {
			points.emplace_back(pos,size,color);
			return points.size();
		}
		int addPoint(const std::vector<Eigen::Vector3f>& pos, float size, const Eigen::Vector4<uint8_t>& color) {
			for (int i = 0; i < pos.size(); i++) {
				points.emplace_back(pos[i], size, color);
			}
			return points.size();
		}
		int addEdge(const std::vector<Eigen::Vector3f>& l, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m = Eigen::Matrix4f::Identity()) {
			lines.emplace_back(l,c,m);
			return lines.size();
		}
		int addTriangleFace(const std::vector<Eigen::Vector3f>&f,const Eigen::Vector4<uint8_t>& c,const Eigen::Matrix4f& m= Eigen::Matrix4f::Identity()) {
			faces.emplace_back(f,c,m);
			return faces.size();
		}
		void setPoint(int id, const Eigen::Vector4<uint8_t>& color) {
			if (id < points.size()) {
				points[id].color= color;
			}
		}
		void setPoint(int id, float size) {
			if (id < points.size()) {
				points[id].size = size;
			}
		}
		void setPoint(int id, const Eigen::Vector3f& pos) {
			if (id < points.size()) {
				points[id].pos = pos;
			}
		}
		void setEdge(const std::string& name, const std::vector<Eigen::Vector3f>& l, const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m = Eigen::Matrix4f::Identity()) {
			auto it = edgesIdMap.find(name);
			if (it != edgesIdMap.end()) {
				int id = it->second;
				lines[id].lines= l;
				lines[id].color = c;
				lines[id].model = m;
				return;
			}
			int id = addEdge(l, c, m);
			edgesIdMap[name] = id - 1;
			edgesStrMap[id - 1] = name;
		}
		void setEdge(const std::string& name, const std::vector<Eigen::Vector3f>& l) {
			auto it = edgesIdMap.find(name);
			if (it != edgesIdMap.end()) {
				int id = it->second;
				lines[id].lines = l;
				return;
			}
		}
		void setEdge(const std::string& name, const Eigen::Vector4<uint8_t>& c) {
			auto it = edgesIdMap.find(name);
			if (it != edgesIdMap.end()) {
				int id = it->second;
				lines[id].color = c;
				return;
			}
		}
		void setEdge(const std::string& name, const Eigen::Matrix4f& m ) {
			auto it = edgesIdMap.find(name);
			if (it != edgesIdMap.end()) {
				int id = it->second;
				lines[id].model = m;
				return;
			}
		}
		void setTriangleFace(const std::string& name, const std::vector<Eigen::Vector3f>&f,const Eigen::Vector4<uint8_t>& c, const Eigen::Matrix4f& m = Eigen::Matrix4f::Identity()) {
			auto it = facesIdMap.find(name);
			if (it != facesIdMap.end()) {
				int id = it->second;
				faces[id].faces = f;
				faces[id].color = c;
				faces[id].model = m;
				return ;
			}
			int id=addTriangleFace(f,c,m);
			facesIdMap[name] = id - 1;
			facesStrMap[id - 1] = name;
		}
		void setTriangleFace(const std::string& name,const Eigen::Matrix4f& m) {
			auto it = facesIdMap.find(name);
			if (it != facesIdMap.end()) {
				int id = it->second;
				faces[id].model=m;
			}
		}
		void setTriangleFace(const std::string& name, const std::vector<Eigen::Vector3f>& f) {
			auto it = facesIdMap.find(name);
			if (it != facesIdMap.end()) {
				int id = it->second;
				faces[id].faces = f;
			}
		}
		void setTriangleFace(const std::string& name,  const Eigen::Vector4<uint8_t>& c) {
			auto it = facesIdMap.find(name);
			if (it != facesIdMap.end()) {
				int id = it->second;
				faces[id].color = c;
			}
		}
		std::vector<Edge>& getEdge() {
			return lines;
		}
		std::vector<VertexPoint>& getPoints() {
			return points;
		}
		std::vector<TriangleFace>& getFaces() {
			return faces;
		}
		bool getTriangleFace(const std::string& name, TriangleFace*& f) {
			auto it = facesIdMap.find(name);
			if (it != facesIdMap.end()) {
				f = &faces[it->second];
				return true;
			}
			return false;
		}
		bool getEdgeLine(const std::string& name, Edge*& e) {
			auto it = edgesIdMap.find(name);
			if (it!=edgesIdMap.end()) {
				e = &lines[it->second];
				return true;
			}
			return false;
		}
		void clearPoints() {
			points.clear();
		}
		void clearLines() {
			lines.clear();
			edgesIdMap.clear();
			edgesStrMap.clear();
		}
		void clearFaces() {
			faces.clear();
			facesIdMap.clear();
			facesStrMap.clear();
		}
		void clear() {
			lines.clear();
			points.clear();
			faces.clear();
			facesIdMap.clear();
			edgesIdMap.clear();
			facesStrMap.clear();
			edgesStrMap.clear();
		}
		std::string hitFace(const Ray& ray) {
			int res = -1;
			float minDist = 100000.0f;
			for (int i = 0; i < faces.size(); i++) {
				std::vector<Eigen::Vector3f>& tris = faces[i].faces;
				Eigen::Matrix4f& mat = faces[i].model;
				for (int j = 0; j < tris.size(); j += 3) {
					float tr;
					if (Intersect(ray, MatrixMulPoint(mat, tris[j]), MatrixMulPoint(mat, tris[j+1]), MatrixMulPoint(mat, tris[j+2]), tr)) {
						if (tr < minDist) {
							minDist = tr;
							res = i;
						}
					}
				}
			}
			if (res == -1) {
				return "";
			}
			return facesStrMap[res];
		}
		std::string hitEdge(const Eigen::Matrix4f& viewPortMat,const Eigen::Vector2f& pos) {
			int res = -1;
			float minDist = 100000.0f;
			float error = 3;
			for (int i = 0; i < lines.size(); i++) {
				std::vector<Eigen::Vector3f>& edges = lines[i].lines;
				Eigen::Matrix4f& mat = lines[i].model;
				std::vector<Eigen::Vector3f>screenPos(edges.size());
				for (int j = 0; j < screenPos.size(); j++) {
					screenPos[j] = MatrixMulPoint(viewPortMat, MatrixMulPoint(mat, edges[j]));
				}
				for (int j = 0; j < screenPos.size()-1; j ++) {

					float curDis=pointToSegmentDist(pos, screenPos[j].head<2>(), screenPos[j + 1].head<2>());
					if (curDis < error && curDis < minDist) {
						minDist = curDis;
						res = i;
					}
				}
			}
			if (res == -1) {
				return "";
			}
			return edgesStrMap[res];
		}
		int hitPoint(const Eigen::Matrix4f& viewPortMat, const Eigen::Vector2f& pos) {
			int res = -1;
			float minDist = 100000.0f;
			for (int i = 0; i <points.size(); i++) {
				Eigen::Vector3f p = points[i].pos;
				float size = points[i].size/2.0;
				Eigen::Vector2f screenPos= MatrixMulPoint(viewPortMat,p).head<2>();
				float curDis = (screenPos - pos).norm();
				if (curDis < size && curDis < minDist) {
					minDist = curDis;
					res = i;
				}
			}
			return res;
		}
	private:
		float pointToSegmentDist(const Eigen::Vector2f& p, const Eigen::Vector2f& s, const Eigen::Vector2f& e) {
			Eigen::Vector2f se = e - s;
			Eigen::Vector2f sp = p - s;
			float t = sp.dot(se) / se.dot(se);
			if (t < 0.0) {
				return sp.norm();
			}
			if (t > 1.0) {
				return (p - e).norm();
			}
			Eigen::Vector2f proj = s + t * se;
			return (p - proj).norm();
		};
	private:
		std::vector<Edge>lines;
		std::vector<VertexPoint>points;
		std::vector<TriangleFace>faces;
		std::unordered_map<std::string, int>facesIdMap;
		std::unordered_map<int, std::string>facesStrMap;
		std::unordered_map<std::string, int>edgesIdMap;
		std::unordered_map<int, std::string>edgesStrMap;
	};

	class PadTaskWidget::Internal {
	public:
		Internal(PadTaskWidget*s):self(s){}
		~Internal(){}
	private:
		friend PadTaskWidget;
		PadTaskWidget* self = nullptr;
		Eigen::Vector3f center = {0,0,0};
		Eigen::Vector3f normal = { 1,0,0 };
		Eigen::Vector3f xAxis = { 0,1,0 };
		Eigen::Vector3f yAxis = { 0,0,1 };
		Eigen::Vector3f rotDir = { 1,0,0 };
		float radius = 25;
		Eigen::Vector3f originCenter = {0,0,0};
		WidgetViewData viewData;
		GizmoAxisTranslate transLatePick;
		GizmoAxisRotate rotatePick;
		std::string curHitTarget = "";
	};
	PadTaskWidget::PadTaskWidget(const std::string& name):EventWidget(name), mInternal(new Internal(this)),mState(Stop)
	{
		std::vector<Eigen::Vector3f>f;
		Maths::FMatrix4 matrix =Maths::FMatrix4::Translation({ 0,0,0 }) * Maths::FMatrix4::Scaling({ 10,10,10 });
		auto model = GetService(Editor::Core::Context).editorResources->GetModel("Arrow_Translate");
		for (auto mesh : model->GetMeshes()) {
			int vcnt = mesh->GetVertexCount();
			int icnt = mesh->GetIndexCount();
			int num = icnt > 0 ? icnt : vcnt;
			for (int i = 0; i < num; i += 3) {
				auto v0 = Maths::FMatrix4::MulPoint(matrix, mesh->GetVertexPosition(i));
				auto v1 = Maths::FMatrix4::MulPoint(matrix, mesh->GetVertexPosition(i + 1));
				auto v2 = Maths::FMatrix4::MulPoint(matrix, mesh->GetVertexPosition(i + 2));
				f.push_back(Eigen::Vector3f(v0.x,v0.y,v0.z));
				f.push_back(Eigen::Vector3f(v1.x, v1.y, v1.z));
				f.push_back(Eigen::Vector3f(v2.x, v2.y, v2.z));
			}
		}
		mInternal->viewData.setTriangleFace("Arrow", f, {255,255,255,0});
		setActive(true);
	}
	PadTaskWidget::~PadTaskWidget()
	{
		delete mInternal;
	}
	void PadTaskWidget::onUpdate()
	{
		const auto& faces = mInternal->viewData.getFaces();
		const auto& points=mInternal->viewData.getPoints();
		mPreflag = mCurflag;
		for (int i = 0; i < points.size(); i++) {
			renderer->drawPoint(points[i].pos, points[i].size,points[i].color);
		}
		
		for (int i = 0; i < faces.size(); i++) {
			renderer->pushMatrix(faces[i].model);
			renderer->drawTriangleList(faces[i].faces, 1.0, faces[i].color);
			renderer->popMatrix();
		}
		
		int segCount = 30;
		std::vector<Eigen::Vector3f>seg(segCount+1);
		float step = 2*3.14159265358979323846f / static_cast<float>(segCount);
		for (int i = 0; i <= segCount; ++i)
		{
			float angle = step * i;
			Eigen::Vector3f d=cos(angle)* mInternal->normal + sin(angle) * mInternal->yAxis;
			seg[i]=d* mInternal->radius + mInternal->center;
		}
		if (mState == Rotate) {
			renderer->pushColor({255,0,255,255});
		}
		for (int i = 0; i < seg.size() - 1; i++) {
			renderer->drawLine(seg[i], seg[i + 1], 3);
		}
		if (mState == Rotate) {
			renderer->popColor();
		}
		//mCurflag=renderer->gizmoAxisTranslationBehavior(renderer->makeId("pad"),
		//	mInternal->center,mInternal->normal,0, 20.0, 10.0,&mInternal->center);
		//if (mPreflag && !mCurflag) {
		//	this->InvokeEvent(PadTaskEvent::LengthChange);
		//}
	}
	void PadTaskWidget::setUpOrigin(float x, float y, float z)
	{
		mInternal->center = Eigen::Vector3f(x, y, z);
		mInternal->originCenter = mInternal->center;
		TriangleFace* face;
		if (mInternal->viewData.getTriangleFace("Arrow", face)) {
			face->model.block(0, 3, 3, 1) = mInternal->center;
		}
	}
	void PadTaskWidget::setUpDir(float x, float y, float z)
	{
		mInternal->normal = Eigen::Vector3f(x,y,z);
		mInternal->rotDir = mInternal->normal;
		mInternal->viewData.clearPoints();
		mInternal->viewData.addPoint(mInternal->rotDir * mInternal->radius+ mInternal->center, 30, { 255,255,255,255 });
		TriangleFace* face;
		if (mInternal->viewData.getTriangleFace("Arrow",face)) {
			face->model.block(0, 0, 3, 3) = RotationMatrixZ(mInternal->normal);
		}
	}
	void PadTaskWidget::setUpXAxis(float x, float y, float z)
	{
		mInternal->xAxis = Eigen::Vector3f(x, y, z);
	}
	void PadTaskWidget::setUpYAxis(float x, float y, float z)
	{
		mInternal->yAxis = Eigen::Vector3f(x, y, z);
	}
	float PadTaskWidget::getLength()
	{
		return (mInternal->originCenter - mInternal->center).norm();
	}
	void PadTaskWidget::setLength(float len)
	{
		mInternal->center = len * mInternal->normal + mInternal->originCenter;
	}
	void PadTaskWidget::onLeftMousePressed()
	{
		if(mState==Hot) {
			if (mInternal->curHitTarget == "Arrow") {
				mState = AxisT;
				mInternal->viewData.setTriangleFace("Arrow", Eigen::Vector4<uint8_t>(255, 0, 255, 255));
				mInternal->transLatePick.startPick(mInternal->normal, mInternal->center);
			}
			if (mInternal->curHitTarget == "Point") {
				mState = Rotate;
				mInternal->viewData.setPoint(0, Eigen::Vector4<uint8_t>(255, 0, 255, 255));
				mInternal->rotatePick.startPick(mInternal->xAxis,mInternal->center,mInternal->rotDir, mInternal->viewData.getPoints()[0].pos);
			}
		}
	}
	void PadTaskWidget::onLeftMouseReleased()
	{
		if (mState == AxisT ) {
			mState = Hot;
			mInternal->viewData.setTriangleFace("Arrow", Eigen::Vector4<uint8_t>(255, 255, 255, 0));
		}
		if (mState == Rotate) {
			mState = Hot;
			mInternal->viewData.setPoint(0, Eigen::Vector4<uint8_t>(255, 255, 255, 255));
		}
	}
	void PadTaskWidget::onMouseMove()
	{	
		if (mState == Stop) {	
			auto ray=m_sceneView->GetMouseRay();
			Ray it(Eigen::Vector3f(ray.origin_.x, ray.origin_.y, ray.origin_.z), Eigen::Vector3f(ray.direction_.x, ray.direction_.y, ray.direction_.z));
			mInternal->curHitTarget =mInternal->viewData.hitFace(it);
			if (mInternal->curHitTarget == "Arrow") {
				mState = Hot;
				mInternal->viewData.setTriangleFace("Arrow", Eigen::Vector4<uint8_t>(255,0,0,255));
			}
			else {
				auto viewportMat=m_sceneView->GetCamera()->GetViewPortMatrix();
				Eigen::Matrix4f  mat(viewportMat.data);
				mat.transposeInPlace();
				int res=mInternal->viewData.hitPoint(mat,renderer->getFrameParam().viewPortCursor);
				if (res != -1) {
					mState = Hot;
					mInternal->curHitTarget = "Point";
					mInternal->viewData.setPoint(res, Eigen::Vector4<uint8_t>(255, 0, 0, 255));
				}
			}
		}
		else if (mState == Hot) {
			bool hitNone = false;
			auto ray = m_sceneView->GetMouseRay();
			Ray it(Eigen::Vector3f(ray.origin_.x, ray.origin_.y, ray.origin_.z), Eigen::Vector3f(ray.direction_.x, ray.direction_.y, ray.direction_.z));
			mInternal->curHitTarget = mInternal->viewData.hitFace(it);
			if (mInternal->curHitTarget == "") {
				mInternal->viewData.setTriangleFace("Arrow", Eigen::Vector4<uint8_t>(255, 255, 255, 0));
			}
			 {
				auto viewportMat = m_sceneView->GetCamera()->GetViewPortMatrix();
				Eigen::Matrix4f  mat(viewportMat.data);
				mat.transposeInPlace();
				int res = mInternal->viewData.hitPoint(mat, renderer->getFrameParam().viewPortCursor);
				if (res == -1) {
					mInternal->viewData.setPoint(0, Eigen::Vector4<uint8_t>(255, 255, 255, 255));
				}
				else
				{
					mInternal->curHitTarget = "Point";
				}
			}
			if (mInternal->curHitTarget=="") {
				mState = Stop;
			}
		}
		else if (mState==AxisT) {
			auto& param = renderer->getFrameParam();
			mInternal->transLatePick.apply(param.rayDirection, param.rayOrigin, mInternal->center);
			TriangleFace* face;
			if (mInternal->viewData.getTriangleFace("Arrow", face)) {
				face->model.block(0, 3, 3, 1) = mInternal->center;
			}
			Eigen::Vector3f pos = mInternal->radius * mInternal->rotDir + mInternal->center;
			mInternal->viewData.setPoint(0, pos);
		}
		else if (mState==Rotate) {
			auto& param = renderer->getFrameParam();
			mInternal->rotatePick.applyDir(param.rayDirection, param.rayOrigin, mInternal->rotDir);
			Eigen::Vector3f pos= mInternal->radius * mInternal->rotDir + mInternal->center;
			mInternal->viewData.setPoint(0,pos);
		}
	}
}