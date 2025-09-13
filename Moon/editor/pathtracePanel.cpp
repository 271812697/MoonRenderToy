#include <QMouseEvent>
#include "pathtracePanel.h"
#include "glloader.h"
#include "pathtrace/PathTrace.h"
#include "pathtrace/Renderer.h"
#include "pathtrace/Scene.h"
#include "pathtrace/Camera.h"

namespace MOON {
	static float viewW;
	static float viewH;

	struct OpenGLProcAddressHelper {
		inline static QOpenGLContext* ctx;
		static void* getProcAddress(const char* name) {
			return (void*)ctx->getProcAddress(name);
		}
	};
	PathTracePanel::PathTracePanel(QWidget* parent) :
		QOpenGLWidget(parent)
	{

		//设置可以捕获鼠标移动消息
		this->setMouseTracking(true);
		//this->grabKeyboard();
		//反锯齿
		QSurfaceFormat format;
		format.setSamples(4);
		this->setFormat(format);
		setFocusPolicy(Qt::StrongFocus);  // 允许通过点击或Tab键获取焦点
		setFocus();                      // 主动获取焦点（可选）
	}

	PathTracePanel::~PathTracePanel()
	{
		PathTrace::Ret();
	}

	void PathTracePanel::initializeGL()
	{
		// opengl funcs
		bool flag = initializeOpenGLFunctions();
		OpenGLProcAddressHelper::ctx = context();
		//CUSTOM_GL_API::CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);
		CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);

		//开启计时器
		this->startTimer(0);
		PathTrace::GetSceneFiles();
		PathTrace::GetEnvMaps();
		PathTrace::LoadScene(PathTrace::sceneFiles[PathTrace::sampleSceneIdx]);
		if (!PathTrace::InitRenderer()) {
			std::cout << "error" << std::endl;
		}
		initFlag = true;


	}

	void  PathTracePanel::timerEvent(QTimerEvent* e)
	{
		this->update();
	}

	void  PathTracePanel::paintGL()
	{

		PathTrace::Update();
		PathTrace::GetRenderer()->Update(0.016);
		PathTrace::GetRenderer()->Render();

		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
		PathTrace::GetRenderer()->Present();

	}

	void  PathTracePanel::leaveEvent(QEvent* event)
	{
	}

	void  PathTracePanel::resizeEvent(QResizeEvent* event)
	{

		QOpenGLWidget::resizeEvent(event);
		viewW = event->size().width();
		viewH = event->size().height();
		if (initFlag && viewH > 0 && viewH > 0) {
			PathTrace::Resize(viewW, viewH);
		}

	}

	void  PathTracePanel::mousePressEvent(QMouseEvent* e)
	{

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

			PathTrace::CameraController::Instance().mouseLeftPress(x, y);
		}

		else if (mb == Qt::MouseButton::MiddleButton) {
			PathTrace::CameraController::Instance().mouseMiddlePress(x, y);
		}

		else if (mb == Qt::MouseButton::RightButton)
		{
			PathTrace::CameraController::Instance().mouseRightPress(x, y);

		}

	}

	void  PathTracePanel::mouseMoveEvent(QMouseEvent* event)
	{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
		auto x = event->x();
		auto y = event->y();
#else
		auto x = e2->position().x();
		auto y = e2->position().y();
#endif
		PathTrace::CameraController::Instance().mouseMove(x, y);

	}

	void  PathTracePanel::mouseReleaseEvent(QMouseEvent* e)
	{
		Qt::MouseButton mb = e->button();
		if (mb == Qt::MouseButton::LeftButton) {

			//PathTrace::CameraController::Instance().mousele;
		}

		else if (mb == Qt::MouseButton::MiddleButton) {
			PathTrace::CameraController::Instance().mouseMiddleRelease(0, 0);
		}

		else if (mb == Qt::MouseButton::RightButton)
		{
			PathTrace::CameraController::Instance().mouseRightRelease(0, 0);

		}

	}

	void  PathTracePanel::wheelEvent(QWheelEvent* event)
	{
		PathTrace::CameraController::Instance().wheelMouseWheel(event->angleDelta().y());
	}
	void  PathTracePanel::keyPressEvent(QKeyEvent* event)
	{

	}
	void  PathTracePanel::keyReleaseEvent(QKeyEvent* event)
	{

	}
}