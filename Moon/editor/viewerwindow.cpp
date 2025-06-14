#include "Qtimgui/imguiwidgets/QtImGui.h"
#include "Qtimgui/imgui/imgui.h"
#include "Qtimgui/implot/implotCustom.h"
#include "Qtimgui/implot/imguizmo.h"
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
#include "OvCore/Global/ServiceLocator.h"
#include "pathtrace/Scene.h"
#include "pathtrace/PathTrace.h"
#include "OvCore/ECS/Components/CMaterialRenderer.h"
#include "parsescene.h"


OvEditor::Core::Context* editorContext = nullptr;
OvEditor::Panels::SceneView* sceneView = nullptr;
static QtImGui::RenderRef imref = nullptr;
static ImPlotContext* ctx = nullptr;
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
		this->startTimer(0);

		editorContext = new OvEditor::Core::Context("", "");
		editorContext->sceneManager.LoadDefaultScene();
		sceneView = new OvEditor::Panels::SceneView("SceneView");
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
		sceneView->Update(0.01);
		if (mSwitchScene) {
			mSwitchScene = false;
			ParseScene::ParsePathTraceScene();
			sceneView->UnselectActor();
			OVSERVICE(TreeViewPanel).initModel();
		}
		sceneView->Render();
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
		sceneView->Present();

		showImGui();
		ImGui::Render();
		QtImGui::render(imref);
	}

	bool ViewerWindow::event(QEvent* evt)
	{

		//RenderWindowInteractor::Instance()->ReceiveEvent(evt);
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
	void ViewerWindow::showImGui()
	{
		static std::chrono::steady_clock::time_point pretime = std::chrono::steady_clock::now();
		static std::chrono::steady_clock::time_point curtime = std::chrono::steady_clock::now();
		curtime = std::chrono::steady_clock::now();
		std::chrono::duration<double>delta = curtime - pretime;
		pretime = curtime;
		static std::vector<float>x;
		static std::vector<float>y;
		static float history = 2.0f;
		static float t = 0.0f;
		float detalTime = delta.count();
		t += detalTime;
		float fps = 1 / detalTime;
		float ms = detalTime * 1000;
		float xm = fmod(t, history);
		if (!x.empty() && xm < x.back()) {
			x.clear();
			y.clear();
		}
		x.push_back(xm);
		y.push_back(fps);
		static ImPlotAxisFlags flags = ImPlotAxisFlags_None;
		if (ImPlot::BeginPlot("##Rolling", ImVec2(-1, 300))) {
			ImPlot::SetupAxes(nullptr, nullptr, flags, flags);
			ImPlot::SetupAxisLimits(ImAxis_X1, 0, history, ImGuiCond_Always);
			ImPlot::SetupAxisLimits(ImAxis_Y1, 60, 140);
			ImPlot::PlotLine("FPS", &x[0], &y[0], x.size(), 0, 0, sizeof(float));
			ImPlot::EndPlot();
		}
		static ImS8  data[10] = { 1,2,3,4,5,6,7,8,9,10 };
		if (ImPlot::BeginPlot("Bar Plot")) {
			ImPlot::PlotBars("Vertical", y.data(), y.size(), 0.5, 1);
			//ImPlot::PlotBars("Vertical", data, 10, 0.7, 1);
			//ImPlot::PlotBars("Horizontal", data, 10, 0.4, 1, ImPlotBarsFlags_Horizontal);
			for (int i = 0; i < y.size(); ++i)
				ImPlot::Annotation(i + 1, y[i], ImVec4(0, 0, 0, 0), ImVec2(0, -5), false, "%.1f FPS", y[i]);
			ImPlot::EndPlot();
		}
		ImGui::Text("%f ms,%f FPS", ms, fps);

		auto proj = sceneView->GetCamera()->GetProjectionMatrix();
		auto view = sceneView->GetCamera()->GetViewMatrix();
		//view = view.TransposeMartix();
		proj = proj.TransposeMartix();
		ImGuizmo::SetRect(50, viewH - 150, 100);
		if (ImGuizmo::DrawGizmo(view.data, proj.data, 0)) {
			auto& v = sceneView->GetCamera()->GetViewMatrix();
			sceneView->GetCamera()->SetRotation(OvMaths::FQuaternion(view));
			;

		}
		ImPlot::ShowDemoWindow();

	}
	void ViewerWindow::switchScene()
	{
		mSwitchScene = true;
	}
}