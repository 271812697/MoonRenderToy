#include "Interactive/Widgets/AxisTranslationWidget.h"
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
	class AxisTranslationWidget::Internal {
	public:
		Internal(AxisTranslationWidget*s):self(s){}
		~Internal(){}
	private:
		friend AxisTranslationWidget;
		AxisTranslationWidget* self = nullptr;
		Eigen::Vector3f center = {0,0,0};
		Eigen::Vector3f normal = { 1,0,0 };
		
		Eigen::Vector3f originCenter = {0,0,0};
		WidgetViewData viewData;
		GizmoAxisTranslate transLatePick;
		
		std::string curHitTarget = "";
	};
	AxisTranslationWidget::AxisTranslationWidget(const std::string& name):EventWidget(name), mInternal(new Internal(this)),mState(Stop)
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
	AxisTranslationWidget::~AxisTranslationWidget()
	{
		delete mInternal;
	}
	void AxisTranslationWidget::onUpdate()
	{
		const auto& faces = mInternal->viewData.getFaces();
		for (int i = 0; i < faces.size(); i++) {
			renderer->pushMatrix(faces[i].model);
			renderer->drawTriangleList(faces[i].faces, 1.0, faces[i].color);
			renderer->popMatrix();
		}
	}
	void AxisTranslationWidget::setUpOrigin(float x, float y, float z)
	{
		mInternal->center = Eigen::Vector3f(x, y, z);
		mInternal->originCenter = mInternal->center;
		TriangleFace* face;
		if (mInternal->viewData.getTriangleFace("Arrow", face)) {
			face->model.block(0, 3, 3, 1) = mInternal->center;
		}
	}
	void AxisTranslationWidget::setUpDir(float x, float y, float z)
	{
		mInternal->normal = Eigen::Vector3f(x,y,z).normalized();
		TriangleFace* face;
		if (mInternal->viewData.getTriangleFace("Arrow",face)) {
			face->model.block(0, 0, 3, 3) = RotationMatrixZ(mInternal->normal);
		}
	}

	void AxisTranslationWidget::setUpScale(float s)
	{
		mInternal->viewData.setTriangleFaceScale("Arrow",s);
	}


	float AxisTranslationWidget::getLength()
	{
		return (mInternal->originCenter - mInternal->center).norm();
	}

	void AxisTranslationWidget::setLength(float len)
	{
		mInternal->center = len * mInternal->normal + mInternal->originCenter;
		TriangleFace* face;
		if (mInternal->viewData.getTriangleFace("Arrow", face)) {
			face->model.block(0, 3, 3, 1) = mInternal->center;
		}
	}
	void AxisTranslationWidget::onLeftMousePressed()
	{
		if(mState==Hot) {
			if (mInternal->curHitTarget == "Arrow") {
				mState = AxisT;
				mInternal->viewData.setTriangleFace("Arrow", Eigen::Vector4<uint8_t>(255, 0, 255, 255));
				mInternal->transLatePick.startPick(mInternal->normal, mInternal->center);
			}
		}
	}
	void AxisTranslationWidget::onLeftMouseReleased()
	{
		if (mState == AxisT ) {
			mState = Hot;
			mInternal->viewData.setTriangleFace("Arrow", Eigen::Vector4<uint8_t>(255, 255, 255, 0));
			if (!mImInvoke) {
				this->InvokeEvent(AxisTranslationEvent::LengthChange);
			}
		}
	}
	void AxisTranslationWidget::onMouseMove()
	{	
		if (mState == Stop) {	
			auto ray=m_sceneView->GetMouseRay();
			Ray it(Eigen::Vector3f(ray.origin_.x, ray.origin_.y, ray.origin_.z), Eigen::Vector3f(ray.direction_.x, ray.direction_.y, ray.direction_.z));
			mInternal->curHitTarget =mInternal->viewData.hitFace(it);
			if (mInternal->curHitTarget == "Arrow") {
				mState = Hot;
				mInternal->viewData.setTriangleFace("Arrow", Eigen::Vector4<uint8_t>(255,0,0,255));
			}
		}
		else if (mState == Hot) {
			
			auto ray = m_sceneView->GetMouseRay();
			Ray it(Eigen::Vector3f(ray.origin_.x, ray.origin_.y, ray.origin_.z), Eigen::Vector3f(ray.direction_.x, ray.direction_.y, ray.direction_.z));
			mInternal->curHitTarget = mInternal->viewData.hitFace(it);
			if (mInternal->curHitTarget == "") {
				mState = Stop;
				mInternal->viewData.setTriangleFace("Arrow", Eigen::Vector4<uint8_t>(255, 255, 255, 0));
			}
		}
		else if (mState==AxisT) {
			auto& param = renderer->getFrameParam();
			mInternal->transLatePick.apply(param.rayDirection, param.rayOrigin, mInternal->center);
			TriangleFace* face;
			if (mInternal->viewData.getTriangleFace("Arrow", face)) {
				face->model.block(0, 3, 3, 1) = mInternal->center;
			}
			if (mImInvoke) {
				this->InvokeEvent(AxisTranslationEvent::LengthChange);
			}
		}
	}
}