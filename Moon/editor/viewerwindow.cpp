#include <QMouseEvent>
#include "viewerwindow.h"
#include "nodedatas/SurfaceMeshData.h"
#include "glloader.h"
#include "renderer/grid_renderer.h"
#include "renderer/Guizmo.h"
#include "core/read_mesh.h"
#include "algorithm/mesh_triangulate.h"
#include "test/TestInstance.h"
#include "test/CommandStream.h"
#include "Guizmo/RenderWindowInteractor.h"
#include "Guizmo/ExecuteCommand.h"
#include "Guizmo/BoxWidget2.h"
#include "pathtrace/PathTrace.h"
#include "pathtrace/Renderer.h"
#include "pathtrace/Scene.h"
#include "pathtrace/Camera.h"
#define __glad_h_


#include "renderer/Context.h"
#include "renderer/SceneView.h"


::Editor::Core::Context* editorContext = nullptr;
::Editor::Panels::SceneView* sceneView = nullptr;

namespace MOON {
	static float viewW;
	static float viewH;
	Eigen::Matrix4f LookAt(const  Eigen::Vector3<float>& _from, const  Eigen::Vector3<float>& _to, const  Eigen::Vector3<float>& _up = Eigen::Vector3<float>(0.0f, 1.0f, 0.0f));
	static GridRenderer* grid_render;
	static Guizmo* guizmoRender;
	struct OpenGLProcAddressHelper {
		inline static QOpenGLContext* ctx;
		static void* getProcAddress(const char* name) {
			return (void*)ctx->getProcAddress(name);
		}
	};
	ViewerWindow* viewer_instance = nullptr;
	ViewerWindow::ViewerWindow(QWidget* parent) :
		QOpenGLWidget(parent)
	{

		if (viewer_instance == nullptr) {
			viewer_instance = this;
			//设置可以捕获鼠标移动消息
			this->setMouseTracking(true);
			this->grabKeyboard();
			//反锯齿
			QSurfaceFormat format;
			format.setSamples(4);
			this->setFormat(format);
			if (windowInteractor == nullptr) {
				windowInteractor = RenderWindowInteractor::New();
				windowInteractor->Enable();
				boxWidget = BoxWidget2::New();
				boxWidget->SetInteractor(windowInteractor);
				boxWidget->SetEnabled(1);
			}
		}
		this->installEventFilter(parent);
	}

	ViewerWindow::~ViewerWindow()
	{
		windowInteractor->Terminate();
	}

	void ViewerWindow::initializeGL()
	{
		// opengl funcs
		bool flag = initializeOpenGLFunctions();
		OpenGLProcAddressHelper::ctx = context();
		//CUSTOM_GL_API::CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);
		CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);
		TEST::TestInstance::Instance().getCommandStream()->test(8);
		TEST::TestInstance::Instance().getCommandStream()->queueCommand([]() {
			std::cout << "say hello" << std::endl;
			});

		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.append_mesh();
		viewer.init();
		//viewer.post_resize(800, 900);
		grid_render = new GridRenderer();
		guizmoRender = new Guizmo();
		//viewer.core().background_color = Eigen::Vector4f(1.0, 0.0, 0.0, 1.0);

		//开启计时器
		this->startTimer(16);


		PathTrace::GetSceneFiles();
		PathTrace::GetEnvMaps();
		PathTrace::LoadScene(PathTrace::sceneFiles[PathTrace::sampleSceneIdx]);
		if (!PathTrace::InitRenderer()) {
			std::cout << "error" << std::endl;
		}
		initFlag = true;
		editorContext = new ::Editor::Core::Context("", "");

		editorContext->sceneManager.LoadDefaultScene();
		sceneView = new ::Editor::Panels::SceneView("SceneView");

	}

	void ViewerWindow::timerEvent(QTimerEvent* e)
	{
		this->update();
	}



	void ViewerWindow::paintGL()
	{
		//TEST::TestInstance::Instance().flush();
		//TEST::TestInstance::Instance().execute();
		//PathTrace::Update();
		//PathTrace::GetRenderer()->Update(0.016);
		//PathTrace::GetRenderer()->Render();
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
		PathTrace::GetRenderer()->Present();
		//PathTrace::TraceScene();
		sceneView->Render();





	}

	bool ViewerWindow::event(QEvent* evt)
	{
		RenderWindowInteractor::Instance()->ReceiveEvent(evt);
		if (sceneView != nullptr)
			sceneView->ReceiveEvent(evt);
		return QOpenGLWidget::event(evt);
	}

	void ViewerWindow::leaveEvent(QEvent* event)
	{
	}

	void ViewerWindow::resizeEvent(QResizeEvent* event)
	{

		QOpenGLWidget::resizeEvent(event);
		viewer.post_resize(event->size().width() * 1.5, event->size().height() * 1.5);

		viewW = event->size().width();
		viewH = event->size().height();
		if (initFlag) {
			PathTrace::Resize(viewW, viewH);
		}

	}

	void ViewerWindow::mousePressEvent(QMouseEvent* e)
	{
		if (blockMouseMessage) {
			return;
		}
		QMouseEvent* e2 = static_cast<QMouseEvent*>(e);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		auto x = e2->x();
		auto y = e2->y();
#else
		auto x = e2->position().x();
		auto y = e2->position().y();
#endif
		Qt::MouseButton mb = e->button();
		if (mb == Qt::MouseButton::LeftButton) {
			viewer.mouse_down(Viewer::MouseButton::Left);

			PathTrace::CameraController::Instance().mouseLeftPress(x, y);
		}

		else if (mb == Qt::MouseButton::MiddleButton) {
			viewer.mouse_down(Viewer::MouseButton::Middle);
			PathTrace::CameraController::Instance().mouseMiddlePress(x, y);
		}

		else if (mb == Qt::MouseButton::RightButton)
		{
			PathTrace::CameraController::Instance().mouseRightPress(x, y);
			viewer.mouse_down(Viewer::MouseButton::Right);
		}

	}

	void ViewerWindow::mouseMoveEvent(QMouseEvent* event)
	{

		auto pos = event->localPos();
		//cursorX = pos.x() * 1.5;
		//cursorY = pos.y() * 1.5;
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		auto x = event->x();
		auto y = event->y();
#else
		auto x = e2->position().x();
		auto y = e2->position().y();
#endif
		PathTrace::CameraController::Instance().mouseMove(x, y);
		viewer.mouse_move(pos.x() * 1.5, pos.y() * 1.5);
	}

	void ViewerWindow::mouseReleaseEvent(QMouseEvent* event)
	{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		auto x = event->x();
		auto y = event->y();
#else
		auto x = e2->position().x();
		auto y = e2->position().y();
#endif
		if (blockMouseMessage) {
			return;
		}
		Qt::MouseButton mb = event->button();
		if (mb == Qt::MouseButton::LeftButton)
			viewer.mouse_up(Viewer::MouseButton::Left);
		else if (mb == Qt::MouseButton::MiddleButton) {
			viewer.mouse_up(Viewer::MouseButton::Middle);
			PathTrace::CameraController::Instance().mouseMiddleRelease(x, y);
		}

		else if (mb == Qt::MouseButton::RightButton)
		{
			viewer.mouse_up(Viewer::MouseButton::Right);
			PathTrace::CameraController::Instance().mouseRightRelease(x, y);
		}

	}

	void ViewerWindow::wheelEvent(QWheelEvent* event)
	{
		viewer.mouse_scroll(event->angleDelta().y());
		PathTrace::CameraController::Instance().wheelMouseWheel(event->angleDelta().y());
	}
	void ViewerWindow::keyPressEvent(QKeyEvent* event)
	{
		blockMouseMessage = true;
		if (event->key() == Qt::Key::Key_Space) {

		}
	}
	void ViewerWindow::keyReleaseEvent(QKeyEvent* event)
	{
		if (event->key() == Qt::Key::Key_Space) {
			blockMouseMessage = false;
		}
	}

	void ViewerWindow::viewnode(const std::shared_ptr<NodeData>& node) {
		auto mesh_data = std::dynamic_pointer_cast<SurfaceMeshData>(node);

		bool is_triangle = mesh_data->mesh()->is_triangle_mesh();
		SurfaceMesh* present_mesh = mesh_data->mesh().get();
		if (!is_triangle) {
			present_mesh = new SurfaceMesh(*mesh_data->mesh());
			triangulate(*present_mesh);
		}
		Eigen::MatrixXd SV;
		auto& pos = present_mesh->positions();
		SV.resize(pos.size(), 3);
		for (int i = 0; i < pos.size(); i++) {
			SV.row(i) << pos[i][0], pos[i][1], pos[i][2];
		}
		Eigen::MatrixXi SF;
		SF.resize(present_mesh->faces_size(), 3);
		const auto& faces = present_mesh->faces();
		int j = 0, x, y, z;
		for (auto f : faces) {
			auto it = present_mesh->vertices(f).begin();
			x = (*it++).idx();
			y = (*it++).idx();
			z = (*it++).idx();
			SF.row(j++) << x, y, z;
		}
		viewer.data(0).clear();
		viewer.data(0).set_mesh(SV, SF);
	}

}