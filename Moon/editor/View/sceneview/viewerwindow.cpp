#include <QMouseEvent>
#include "viewerwindow.h"
#include "glloader.h"
#define __glad_h_
#include "core/callbackManager.h"
#include "renderer/Context.h"
#include "renderer/SceneView.h"
#include "editor/UI/TreeViewPanel/treeViewpanel.h"
#include "OvCore/Global/ServiceLocator.h"
#include "pathtrace/Scene.h"
#include "pathtrace/PathTrace.h"
#include "OvCore/ECS/Components/CMaterialRenderer.h"
#include "editor/parsescene.h"

namespace MOON {
	struct OpenGLProcAddressHelper {
		inline static QOpenGLContext* ctx;
		static void* getProcAddress(const char* name) {
			return (void*)ctx->getProcAddress(name);
		}
	};
	class ViewerWindow::ViewerWindowInternal {
	public:
		ViewerWindowInternal(ViewerWindow* view) :mSelf(view) {
		}
		void initializeGL() {
			mEditorContext = new OvEditor::Core::Context("", "");
			mEditorContext->sceneManager.LoadDefaultScene();
			mSceneView = new OvEditor::Panels::SceneView("SceneView");
			ParseScene::ParsePathTraceScene();
		}
		~ViewerWindowInternal() {
			delete mEditorContext;
			delete mSceneView;
		}
		void paintGL() {
			mSceneView->Update(0.01);
			if (mSwitchScene) {
				mSwitchScene = false;
				ParseScene::ParsePathTraceScene();
				mSceneView->UnselectActor();
			}
			mSceneView->Render();
			mSelf->glBindFramebuffer(GL_FRAMEBUFFER, mSelf->defaultFramebufferObject());
			mSceneView->Present();
		}
		bool event(QEvent* evt)
		{
			if (mSceneView != nullptr)
				mSceneView->ReceiveEvent(evt);
			return true;
		}
		void resizeEvent(QResizeEvent* event)
		{
			mViewWidth = event->size().width();
			mViewHeight = event->size().height();
			if (mSceneView != nullptr)
				mSceneView->Resize(mViewWidth, mViewHeight);
		}
		void switchScene()
		{
			mSwitchScene = true;
		}
	private:
		ViewerWindow* mSelf = nullptr;
		OvEditor::Core::Context* mEditorContext = nullptr;
		OvEditor::Panels::SceneView* mSceneView = nullptr;
		int mViewWidth;
		int mViewHeight;
		bool mInitFlag = false;
		bool mSwitchScene = false;

	};
	ViewerWindow::ViewerWindow(QWidget* parent) :
		QOpenGLWidget(parent), mInternal(new ViewerWindowInternal(this))
	{
		//设置可以捕获鼠标移动消息
		// default to strong focus
		this->setFocusPolicy(Qt::StrongFocus);
		//this->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
		this->setMouseTracking(true);
		QSurfaceFormat format;
		format.setSamples(4);
		this->setFormat(format);
		COPROVITE(ViewerWindow, *this);
	}

	ViewerWindow::~ViewerWindow()
	{
		delete mInternal;
	}

	void ViewerWindow::initializeGL()
	{
		QOpenGLWidget::initializeGL();
		// opengl funcs
		bool flag = initializeOpenGLFunctions();
		OpenGLProcAddressHelper::ctx = context();
		GlLoader::CustomLoadGL(OpenGLProcAddressHelper::getProcAddress);
		//开启计时器
		this->startTimer(0);
		mInternal->initializeGL();
	}

	void ViewerWindow::timerEvent(QTimerEvent* e)
	{
		this->update();
	}

	void ViewerWindow::paintGL()
	{
		CallBackManager::instance().exectue();
		mInternal->paintGL();
	}

	bool ViewerWindow::event(QEvent* evt)
	{
		mInternal->event(evt);
		return QOpenGLWidget::event(evt);
	}

	void ViewerWindow::leaveEvent(QEvent* event)
	{
	}

	void ViewerWindow::resizeEvent(QResizeEvent* event)
	{
		QOpenGLWidget::resizeEvent(event);
		mInternal->resizeEvent(event);
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
		mInternal->switchScene();
	}
}