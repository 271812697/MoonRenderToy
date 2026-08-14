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

#include "Interactive/Im3DRenderer.h"
#include "Interactive/Interactive/RenderWindowInteractor.h"
#include "core/log.h"
#include "Core/Rendering/GbufferPass.h"
#include "Qtimgui/imgui/imgui.h"
#include "Interactive/Im2DRenderer.h"
#include "Settings/DebugSetting.h"
#include "core/SelectionManager.h"
#include "renderer/GizmoRenderPass.h"

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
			
			
			QObject::connect(&tree, &TreeViewPanel::setSelectActor, mSelf, &onActorSelected);
			QObject::connect(&tree, &TreeViewPanel::itemHovered, mSelf, &onActorHovered);

			QObject::connect(&tree, &TreeViewPanel::itemLeave, mSelf, &onActorHoverLeaved);
			mEditorContext = new Editor::Core::Context("", "");
			mEditorContext->sceneManager.LoadDefaultScene();
			
			mSceneView = new Editor::Panels::SceneView("SceneView");
			GetService(RenderPassSettingWidget).Refresh();
			parser->ParseFile(mReadFilePath.toStdString());
			
			ImRenderer::instance().init();

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
			ImRenderer::instance().newImgui();
			Render2D::Im2DRender::instance().newFrame();
			if (mSceneView->GetRenderer().GetPass<Editor::Rendering::GizmoRenderPass>("ImRenderer").IsEnabled()) {
				ImRenderer::instance().newFrame(mSceneView);
			}
			mSceneView->Update(0.01);
			if (mDoReadFile) {
				mDoReadFile = false;
				parser->ParseFile(mReadFilePath.toStdString());
				mSceneView->UnselectActor();
				
			}
			else if (mRefreshTreeView) {
				mRefreshTreeView = false;
				
			}
				
			if (mAddActors.size() > 0|| mRemoveActors.size() > 0||mModifyActors.size()>0) {
				std::vector<TreeViewPanel::Operation> operations;
				operations.push_back(TreeViewPanel::Operation( TreeViewPanel::OperationType::Add,mAddActors ));
				operations.push_back(TreeViewPanel::Operation(TreeViewPanel::OperationType::Remove, mRemoveActors));
				operations.push_back(TreeViewPanel::Operation(TreeViewPanel::OperationType::Update, mModifyActors));
				GetTreeView.updateActorInTree(operations);
				mAddActors.clear();
				mRemoveActors.clear();
				mModifyActors.clear();
			}
			mSceneView->Render();
			
			mSelf->glBindFramebuffer(GL_FRAMEBUFFER, mSelf->defaultFramebufferObject());
			mSceneView->Present();
			debugImgui();
			Render2D::Im2DRender::instance().endFrame();
			ImRenderer::instance().endImgui();
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
		void onReadFile(const QString& path)
		{
			mReadFilePath = path;
			mDoReadFile = true;
		}
	private:
		friend ViewerWidget;
		ViewerWidget* mSelf = nullptr;
		Editor::Core::Context* mEditorContext = nullptr;
		Editor::Panels::SceneView* mSceneView = nullptr;
		std::vector<Core::ECS::Actor*>mAddActors;
		std::vector<Core::ECS::Actor*>mRemoveActors;
		std::vector<Core::ECS::Actor*>mModifyActors;
		ParseScene* parser = nullptr;
		int mViewWidth;
		int mViewHeight;
		bool mInitFlag = false;
		
		bool mRefreshTreeView = false;
		QString mReadFilePath = "";
		bool mDoReadFile = false;

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

	void ViewerWidget::addActorToTreeView(Core::ECS::Actor* actor)
	{
		mInternal->mAddActors.push_back(actor);
	}

	void ViewerWidget::removeActorFromTreeView(Core::ECS::Actor* actor)
	{
		mInternal->mRemoveActors.push_back(actor);
	}

	void ViewerWidget::modifyActorInTreeView(Core::ECS::Actor* actor)
	{
		mInternal->mModifyActors.push_back(actor);
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

	void ViewerWidget::onReadFile(const QString& path)
	{
		mInternal->onReadFile(path);
	}
	void ViewerWidget::refreshTreeView()
	{
		mInternal->mRefreshTreeView = true;
		
	}
	void ViewerWidget::onActorSelected(::Core::ECS::Actor* actor) {
		if (actor != nullptr) {
			
			mInternal->mSceneView->SelectActor(*actor);
			GetSelection.select({ actor->GetID() });
		}
	}
}