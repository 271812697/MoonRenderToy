#include <QMouseEvent>
#include "viewerwidget.h"
#include "glloader.h"
#define  __glad_h_
#include "core/callbackManager.h"
#include "renderer/Context.h"
#include "renderer/SceneView.h"
#include "Core/Global/ServiceLocator.h"
#include "Core/ECS/Components/CMaterialRenderer.h"
#include "editor/parsescene.h"
#include "editor/UI/TreeViewPanel/treeViewpanel.h"
#include "editor/UI/SettingPanel/PassSettingWidget.h"
#include "Gizmo/Gizmo.h"
#include "Gizmo/Interactive/RenderWindowInteractor.h"
#include "core/log.h"
#include "Core/Rendering/GbufferPass.h"
#include "Qtimgui/imgui/imgui.h"
#include "Settings/DebugSetting.h"
#include "core/SelectionManager.h"

namespace MOON {
	struct OpenGLProcAddressHelper {
		inline static QOpenGLContext* ctx;
		static void* getProcAddress(const char* name) {
			return (void*)ctx->getProcAddress(name);
		}
	};
	class ViewerWidget::ViewerWindowInternal {
	public:
		ViewerWindowInternal(ViewerWidget* view) :mSelf(view) {

		}
		void initializeGL() {
			auto& tree = GetService(TreeViewPanel);
			
			QObject::connect(mSelf, &ViewerWidget::sceneChange, &tree, &TreeViewPanel::updateTreeViewSceneRoot
				, Qt::ConnectionType::QueuedConnection);
			QObject::connect(&tree, &TreeViewPanel::setSelectActor, mSelf, &onActorSelected);
			QObject::connect(&tree, &TreeViewPanel::itemHovered, mSelf, &onActorHovered);

			QObject::connect(&tree, &TreeViewPanel::itemLeave, mSelf, &onActorHoverLeaved);
			mEditorContext = new Editor::Core::Context("", "");
			mEditorContext->sceneManager.LoadDefaultScene();
			mSceneView = new Editor::Panels::SceneView("SceneView");
			GetService(RenderPassSettingWidget).Refresh();
			parser->ParsePathTraceScene(mScenePath.toStdString());
			emit mSelf->sceneChange();
			Gizmo::instance().init();

		}
		~ViewerWindowInternal() {
			delete mEditorContext;
			delete mSceneView;
		}
		void debugImgui() {
			bool value=MOON::DebugSettings::instance().getNode("DebugImgui")->getData<bool>();
			if (value) {
				ImVec2 a = { 0,1 }, b = { 1,0 };
				//ImVec2 size = ImVec2(mViewWidth / 2, mViewHeight/2);
				ImVec2 size = ImVec2(mViewWidth, mViewHeight);
				auto& gbufferData = mSceneView->GetRenderer().GetPass<::Core::Rendering::GbufferPass>("Gbuffer").GetGbufferData();
				ImGui::Image(gbufferData.position->GetID(), size, a, b);
				ImGui::Image(gbufferData.normal->GetID(), size, a, b);
				ImGui::Image(gbufferData.occlusion->GetID(), size, a, b);
				ImGui::Image(gbufferData.occlusionBlur->GetID(), size, a, b);
			}
		}

		void paintGL() {
			Gizmo::instance().newImgui();
			Gizmo::instance().newFrame(mSceneView);
			mSceneView->Update(0.01);
			if (mSwitchScene) {
				mSwitchScene = false;
				parser->ParsePathTraceScene(mScenePath.toStdString());
				mSceneView->UnselectActor();
				emit mSelf->sceneChange();
			}
			else if (mUpdateTreeView) {
				mUpdateTreeView = false;
				emit mSelf->sceneChange();
			}
			
			mSceneView->Render();
			mSelf->glBindFramebuffer(GL_FRAMEBUFFER, mSelf->defaultFramebufferObject());
			mSceneView->Present();
			debugImgui();
			Gizmo::instance().endFrame();
			Gizmo::instance().endImgui();
			mSceneView->getInutState().ClearEvents();
		}
		bool event(QEvent* evt)
		{
			if (mSceneView != nullptr)
				mSceneView->ReceiveEvent(evt);
			RenderWindowInteractor::Instance()->ReceiveEvent(evt);
			return true;
		}
		void resizeEvent(QResizeEvent* event)
		{
			mViewWidth = event->size().width();
			mViewHeight = event->size().height();
			if (mSceneView != nullptr)
				mSceneView->Resize(mViewWidth, mViewHeight);
			RenderWindowInteractor::Instance()->UpdateSize(mViewWidth,mViewHeight);
		}
		void onSwitchScene(const QString& path)
		{
			mScenePath = path;
			mSwitchScene = true;
		}
	private:
		friend ViewerWidget;
		ViewerWidget* mSelf = nullptr;
		Editor::Core::Context* mEditorContext = nullptr;
		Editor::Panels::SceneView* mSceneView = nullptr;
		ParseScene* parser = nullptr;
		int mViewWidth;
		int mViewHeight;
		bool mInitFlag = false;
		bool mSwitchScene = false;
		bool mUpdateTreeView = false;
		QString mScenePath = "";

	};
	ViewerWidget::ViewerWidget(QWidget* parent) :
		QOpenGLWidget(parent), mInternal(new ViewerWindowInternal(this))
	{
		//设置可以捕获鼠标移动消息
		// default to strong focus
		this->setFocusPolicy(Qt::StrongFocus);
		//this->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
		this->setMouseTracking(true);
		QSurfaceFormat format;
		format.setSamples(1);
		this->setFormat(format);
		RegService(ViewerWidget, *this);
	}

	ViewerWidget::~ViewerWidget()
	{
		delete mInternal;
	}

	void ViewerWidget::initializeGL()
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

	void ViewerWidget::timerEvent(QTimerEvent* e)
	{
		this->update();
	}

	void ViewerWidget::paintGL()
	{
		CallBackManager::instance().exectue();
		mInternal->paintGL();
	}

	bool ViewerWidget::event(QEvent* evt)
	{
		mInternal->event(evt);
		return QOpenGLWidget::event(evt);
	}

	void ViewerWidget::leaveEvent(QEvent* event)
	{
	}

	void ViewerWidget::resizeEvent(QResizeEvent* event)
	{
		QOpenGLWidget::resizeEvent(event);
		mInternal->resizeEvent(event);
	}

	void ViewerWidget::mousePressEvent(QMouseEvent* e)
	{
	}

	void ViewerWidget::mouseMoveEvent(QMouseEvent* event)
	{
	}

	void ViewerWidget::mouseReleaseEvent(QMouseEvent* event)
	{
	}

	void ViewerWidget::wheelEvent(QWheelEvent* event)
	{
	}
	void ViewerWidget::keyPressEvent(QKeyEvent* event)
	{
	}
	void ViewerWidget::keyReleaseEvent(QKeyEvent* event)
	{
	}

	::Editor::Panels::AView* ViewerWidget::getView()
	{
		return mInternal->mSceneView;
	}

	void ViewerWidget::onActorHovered(Core::ECS::Actor* actor)
	{
		if (actor != nullptr) {
			GetSelection.setPreselect(actor->GetID());
		}
	}

	void ViewerWidget::onActorHoverLeaved(Core::ECS::Actor* actor)
	{
		if (actor != nullptr) {
			GetSelection.clearPreselect();
		}
	}

	void ViewerWidget::onSceneChange(const QString& path)
	{
		mInternal->onSwitchScene(path);
	}
	void ViewerWidget::updateTreeView()
	{
		mInternal->mUpdateTreeView = true;
		
	}
	void ViewerWidget::onActorSelected(::Core::ECS::Actor* actor) {
		if (actor != nullptr) {
			
			mInternal->mSceneView->SelectActor(*actor);
		}
	}
}