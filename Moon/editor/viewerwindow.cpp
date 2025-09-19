#include <QMouseEvent>
#include "viewerwindow.h"
#include "glloader.h"
#include "Guizmo/RenderWindowInteractor.h"
#define __glad_h_
#include "core/callbackManager.h"
#include "renderer/Context.h"
#include "renderer/SceneView.h"
#include "editor/UI/TreeViewPanel/treeViewpanel.h"
#include "OvCore/Global/ServiceLocator.h"
#include "pathtrace/Scene.h"
#include "pathtrace/PathTrace.h"
#include "OvCore/ECS/Components/CMaterialRenderer.h"
#include "parsescene.h"

OvEditor::Core::Context* editorContext = nullptr;
OvEditor::Panels::SceneView* sceneView = nullptr;
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
		//开启计时器
		this->startTimer(0);
		editorContext = new OvEditor::Core::Context("", "");
		editorContext->sceneManager.LoadDefaultScene();
		sceneView = new OvEditor::Panels::SceneView("SceneView");
		ParseScene::ParsePathTraceScene();
		
	}

	void ViewerWindow::timerEvent(QTimerEvent* e)
	{
		this->update();
	}

	void ViewerWindow::paintGL()
	{
		CallBackManager::instance().exectue();
		sceneView->Update(0.01);
		if (mSwitchScene) {
			mSwitchScene = false;
			ParseScene::ParsePathTraceScene();
			sceneView->UnselectActor();
		}
		sceneView->Render();
		glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());
		sceneView->Present();
	}

	bool ViewerWindow::event(QEvent* evt)
	{
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