#include "Qtimgui/imguiwidgets/QtImGui.h"
#include "Qtimgui/imgui/imgui.h"
#include "Qtimgui/implot/implotCustom.h"
#include <QMouseEvent>
#include "viewerwindow.h"
#include "glloader.h"
#include "test/TestInstance.h"
#include "test/CommandStream.h"
#include "Guizmo/RenderWindowInteractor.h"
#define __glad_h_
#include "renderer/Context.h"
#include "renderer/SceneView.h"
#include "treeViewpanel.h"
#include "Core/Global/ServiceLocator.h"
#include "pathtrace/Scene.h"
#include "pathtrace/PathTrace.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "parsescene.h"


::Editor::Core::Context* editorContext = nullptr;
::Editor::Panels::SceneView* sceneView = nullptr;
QtImGui::RenderRef imref = nullptr;
ImPlotContext* ctx = nullptr;
namespace MOON {
	static float viewW;
	static float viewH;

	struct OpenGLProcAddressHelper {
		inline static QOpenGLContext* ctx;
		static void* getProcAddress(const char* name) {
			return (void*)ctx->getProcAddress(name);
		}
	};
	ViewerWindow::ViewerWindow(QWidget* parent) :
		QOpenGLWidget(parent)
	{
		//设置可以捕获鼠标移动消息
		// default to strong focus
		this->setFocusPolicy(Qt::StrongFocus);
		//this->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
		this->setMouseTracking(true);
		//this->grabKeyboard();
		//反锯齿
		QSurfaceFormat format;
		format.setSamples(4);
		this->setFormat(format);
		COPROVITE(ViewerWindow, *this);
	}

	ViewerWindow::~ViewerWindow()
	{
		delete editorContext;

	}

	void ViewerWindow::initializeGL()
	{

		QOpenGLWidget::initializeGL();
		// opengl funcs
		bool flag = initializeOpenGLFunctions();
		OpenGLProcAddressHelper::ctx = context();
		CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);
		TEST::TestInstance::Instance().getCommandStream()->test(8);
		TEST::TestInstance::Instance().getCommandStream()->queueCommand([]() {
			std::cout << "say hello" << std::endl;
			});
		//开启计时器
		this->startTimer(16);

		editorContext = new ::Editor::Core::Context("", "");
		editorContext->sceneManager.LoadDefaultScene();
		sceneView = new ::Editor::Panels::SceneView("SceneView");
		ParseScene::ParsePathTraceScene();

		OVSERVICE(TreeViewPanel).initModel();
		imref = QtImGui::initialize(this, false);
		ctx = ImPlot::CreateContext();

	}

	void ViewerWindow::timerEvent(QTimerEvent* e)
	{
		this->update();
	}

	void ViewerWindow::paintGL()
	{
		QtImGui::newFrame(imref);
		ImPlot::SetCurrentContext(ctx);
		sceneView->Update(0.016);
		if (mSwitchScene) {
			mSwitchScene = false;
			ParseScene::ParsePathTraceScene();
			sceneView->UnselectActor();
			OVSERVICE(TreeViewPanel).initModel();
		}
		sceneView->Render();
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
		sceneView->Present();
		//ImGui::Text("Hello world!");


		bool show_implot_demo_window = true;
		//ImPlot::ShowDemoWindow(&show_implot_demo_window);
		static float scale_min = 1.0f;
		static float scale_max = 16.3f;
		static float val = 3.0f;
		ImGui::SliderFloat("HeadVal", &val, scale_min, scale_max);
		static ImPlotColormap map = ImPlotColormap_Cool;
		ImPlot::PushColormap(map);
		ImPlotCustom::ColormapScale("HeadMap", val, scale_min, scale_max, ImVec2(10, 150), ImVec2(90, 225));
		ImPlot::PopColormap();


		ImGui::Render();
		QtImGui::render(imref);

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
		viewW = event->size().width();
		viewH = event->size().height();
		if (sceneView != nullptr)
			sceneView->Resize(viewW, viewH);
		QOpenGLWidget::resizeEvent(event);
	}

	void ViewerWindow::mousePressEvent(QMouseEvent* e)
	{
	}

	void ViewerWindow::mouseMoveEvent(QMouseEvent* event)
	{
	}

	void ViewerWindow::mouseReleaseEvent(QMouseEvent* event)
	{
	}

	void ViewerWindow::wheelEvent(QWheelEvent* event)
	{
	}
	void ViewerWindow::keyPressEvent(QKeyEvent* event)
	{
	}
	void ViewerWindow::keyReleaseEvent(QKeyEvent* event)
	{
	}
	void ViewerWindow::switchScene()
	{
		mSwitchScene = true;
	}
}