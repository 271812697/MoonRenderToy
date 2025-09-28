#include <QMouseEvent>
#include "viewerwindow.h"
#include "glloader.h"
#define __glad_h_
#include "core/callbackManager.h"
#include "renderer/Context.h"
#include "renderer/SceneView.h"

#include "OvCore/Global/ServiceLocator.h"
#include "pathtrace/Scene.h"
#include "pathtrace/PathTrace.h"
#include "OvCore/ECS/Components/CMaterialRenderer.h"
#include "editor/parsescene.h"
#include "editor/UI/TreeViewPanel/treeViewpanel.h"
#include "Guizmo/Guizmo.h"

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
			auto& tree = OVSERVICE(TreeViewPanel);
			QObject::connect(mSelf, &ViewerWindow::sceneChange, &tree, &TreeViewPanel::updateTreeViewSceneRoot
				, Qt::ConnectionType::QueuedConnection);
			QObject::connect(&tree,&TreeViewPanel::setSelectActor,mSelf,&onActorSelected);
			Guizmo::instance().init();
			mScenePath=QString::fromStdString(PathTraceRender::instance().GetSceneFilePath());
			mEditorContext = new OvEditor::Core::Context("", "");
			mEditorContext->sceneManager.LoadDefaultScene();
			mSceneView = new OvEditor::Panels::SceneView("SceneView");
			parser->ParsePathTraceScene(mScenePath.toStdString());
			emit mSelf->sceneChange();

		}
		~ViewerWindowInternal() {
			delete mEditorContext;
			delete mSceneView;
		}
		void paintGL() {
			mSceneView->Update(0.01);
			Guizmo::instance().newFrame(mSceneView);
			if (mSwitchScene) {
				mSwitchScene = false;
				parser->ParsePathTraceScene(mScenePath.toStdString());
				mSceneView->UnselectActor();
				emit mSelf->sceneChange();
			}

			mSceneView->Render();
			mSelf->glBindFramebuffer(GL_FRAMEBUFFER, mSelf->defaultFramebufferObject());
			mSceneView->Present();
			Guizmo::instance().endFrame();
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
		void onSwitchScene(const QString& path)
		{
			mScenePath = path;
			mSwitchScene = true;
		}
	private:
		friend ViewerWindow;
		ViewerWindow* mSelf = nullptr;
		OvEditor::Core::Context* mEditorContext = nullptr;
		OvEditor::Panels::SceneView* mSceneView = nullptr;
		ParseScene* parser = nullptr;
		int mViewWidth;
		int mViewHeight;
		bool mInitFlag = false;
		bool mSwitchScene = false;
		QString mScenePath = "";

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

	void ViewerWindow::onSceneChange(const QString& path)
	{
		mInternal->onSwitchScene(path);
	}
	void ViewerWindow::onActorSelected(OvCore::ECS::Actor* actor) {
		if (actor != nullptr) {
            mInternal->mSceneView->SelectActor(*actor);
		}
	}
}