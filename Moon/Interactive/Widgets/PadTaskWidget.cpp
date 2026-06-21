#include "Interactive/Widgets/PadTaskWidget.h"
#include "Interactive/Im3DRenderer.h"
#include "Interactive/MathUtil/MathUtil.h"
#include "Core/Global/ServiceLocator.h"
#include "Rendering/Resources/Mesh.h"
#include "Rendering/Resources/Model.h"
#include "renderer/Context.h"
#include "Interactive/GizmoBehaviour.h"
#include "renderer/SceneView.h"
#include "Interactive/ViewData.h"
namespace MOON {
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
	float PadTaskWidget::getAngle()
	{
		float crossSign = std::acos(mInternal->normal.dot(mInternal->rotDir));
		int degree = crossSign * 180 / 3.14159265358979323846f;
		if (mInternal->rotDir.dot(mInternal->yAxis) < 0) {
			return -degree;
		}
		return degree;
	}
	void PadTaskWidget::setLength(float len)
	{
		mInternal->center = len * mInternal->normal + mInternal->originCenter;
		TriangleFace* face;
		if (mInternal->viewData.getTriangleFace("Arrow", face)) {
			face->model.block(0, 3, 3, 1) = mInternal->center;
		}
		mInternal->viewData.clearPoints();
		mInternal->viewData.addPoint(mInternal->rotDir * mInternal->radius + mInternal->center, 30, { 255,255,255,255 });
	}
	void PadTaskWidget::setAngle(float degree)
	{
		float angle=degree / 180.0f * 3.14159265358979323846f;
		mInternal->rotDir= cos(angle) * mInternal->normal + sin(angle) * mInternal->yAxis;
		mInternal->viewData.clearPoints();
		mInternal->viewData.addPoint(mInternal->rotDir * mInternal->radius + mInternal->center, 30, { 255,255,255,255 });
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
			if (!mImInvoke) {
				this->InvokeEvent(PadTaskEvent::LengthChange);
			}
		}
		if (mState == Rotate) {
			mState = Hot;
			mInternal->viewData.setPoint(0, Eigen::Vector4<uint8_t>(255, 255, 255, 255));
			if (!mImInvoke) {
				this->InvokeEvent(PadTaskEvent::AngleChange);
			}
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
			if (mImInvoke) {
				this->InvokeEvent(PadTaskEvent::LengthChange);
			}
		}
		else if (mState==Rotate) {
			auto& param = renderer->getFrameParam();
			mInternal->rotatePick.applyDir(param.rayDirection, param.rayOrigin, mInternal->rotDir);
			Eigen::Vector3f pos= mInternal->radius * mInternal->rotDir + mInternal->center;
			mInternal->viewData.setPoint(0,pos);
			if (mImInvoke) {
				this->InvokeEvent(PadTaskEvent::AngleChange);
			}
		}
	}
}