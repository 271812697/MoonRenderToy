#include <QMouseEvent>
#include "viewerwindow.h"
#include "glloader.h"
#include "test/TestInstance.h"
#include "test/CommandStream.h"
#include "Guizmo/RenderWindowInteractor.h"
#define __glad_h_
#include "renderer/Context.h"
#include "renderer/SceneView.h"


::Editor::Core::Context* editorContext = nullptr;
::Editor::Panels::SceneView* sceneView = nullptr;

namespace MOON {
	static float viewW;
	static float viewH;

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

		//设置可以捕获鼠标移动消息
		this->setMouseTracking(true);
		this->grabKeyboard();
		//反锯齿
		QSurfaceFormat format;
		format.setSamples(4);
		this->setFormat(format);
		setFocusPolicy(Qt::StrongFocus);  // 允许通过点击或Tab键获取焦点
		setFocus();                      // 主动获取焦点（可选）
	}

	ViewerWindow::~ViewerWindow()
	{
	
	}

	void ViewerWindow::initializeGL()
	{
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

	}

	void ViewerWindow::timerEvent(QTimerEvent* e)
	{
		this->update();
	}

	void ViewerWindow::paintGL()
	{

		sceneView->Update(0.016);
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

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
		std::cout << "tt" << std::endl;
	}
	void ViewerWindow::keyReleaseEvent(QKeyEvent* event)
	{
	}


}