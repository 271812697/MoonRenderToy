#include "debugOpenGlWidget.h"
#include "Qtimgui/imguiwidgets/QtImGui.h"
#include "Qtimgui/imgui/imgui.h"
#include "Qtimgui/implot/implotCustom.h"
#include "Qtimgui/implot/imguizmo.h"
namespace MOON {
	static QtImGui::RenderRef imref = nullptr;
	static ImPlotContext* ctx = nullptr;
	DebugOpenGLWidget::DebugOpenGLWidget(QWidget* parent) {
		this->setFocusPolicy(Qt::StrongFocus);
		this->setMouseTracking(true);
		QSurfaceFormat format;
		format.setSamples(4);
		this->setFormat(format);
	}
	DebugOpenGLWidget::~DebugOpenGLWidget() {

	}
	void DebugOpenGLWidget::initializeGL() {

		QOpenGLWidget::initializeGL();
		// opengl funcs
		bool flag = initializeOpenGLFunctions();

		//¿ªÆô¼ÆÊ±Æ÷
		this->startTimer(0);
		imref = QtImGui::initialize(this, false);
		ctx = ImPlot::CreateContext();
	}
	void DebugOpenGLWidget::timerEvent(QTimerEvent* e) {
		this->update();
	}
	void DebugOpenGLWidget::paintGL() {
		QtImGui::newFrame(imref);
		ImPlot::SetCurrentContext(ctx);
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());



		showImGui();
		ImGui::Render();
		QtImGui::render(imref);
	}
	bool DebugOpenGLWidget::event(QEvent* evt) {
		return QOpenGLWidget::event(evt);
	}
	void DebugOpenGLWidget::leaveEvent(QEvent* event) {

	}

	void DebugOpenGLWidget::resizeEvent(QResizeEvent* event) {
		QOpenGLWidget::resizeEvent(event);
	}
	void DebugOpenGLWidget::mousePressEvent(QMouseEvent* event) {

	}

	void DebugOpenGLWidget::mouseMoveEvent(QMouseEvent* event) {

	}

	void DebugOpenGLWidget::mouseReleaseEvent(QMouseEvent* event) {

	}

	void DebugOpenGLWidget::wheelEvent(QWheelEvent* event) {

	}
	void DebugOpenGLWidget::keyPressEvent(QKeyEvent* event) {

	}
	void DebugOpenGLWidget::keyReleaseEvent(QKeyEvent* event) {

	}
	void DebugOpenGLWidget::showImGui() {
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
		ImPlot::ShowDemoWindow();
	}
}



