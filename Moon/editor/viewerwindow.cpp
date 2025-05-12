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
	ViewerWindow::ViewerWindow(QWidget* parent) :
		QOpenGLWidget(parent)
	{
		//设置可以捕获鼠标移动消息
		// default to strong focus
		this->setFocusPolicy(Qt::StrongFocus);
		this->setUpdateBehavior(QOpenGLWidget::NoPartialUpdate);
		this->setMouseTracking(true);
		//this->grabKeyboard();
		//反锯齿
		QSurfaceFormat format;
		format.setSamples(4);
		this->setFormat(format);
	}

	ViewerWindow::~ViewerWindow()
	{

	}
	void ParsePathTraceScene(PathTrace::Scene* sce, ::Core::SceneSystem::Scene* scene) {
		if (sce == nullptr || scene == nullptr) {
			return;
		}
		static ::Core::Resources::Material m_cameraMaterial;
		m_cameraMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().shaderManager[":Shaders\\Lambert.ovfx"]);
		m_cameraMaterial.SetProperty("u_Diffuse", Maths::FVector4{ 0.0f, 0.3f, 0.7f, 1.0f });
		m_cameraMaterial.SetProperty("u_DiffuseMap", static_cast<Rendering::Resources::Texture*>(nullptr));
		auto& mesh = sce->meshes;
		auto& instance = sce->meshInstances;
		auto& material = sce->materials;
		for (auto& mi : instance) {
			auto& actor = scene->CreateActor();
			actor.SetName(mi.name);
			auto m = ::Core::Global::ServiceLocator::Get<::Core::ResourceManagement::ModelManager>().GetResource("#" + mesh[mi.meshID]->name);
			actor.AddComponent<::Core::ECS::Components::CModelRenderer>().SetModel(m);

			auto& materilaRener = actor.AddComponent<::Core::ECS::Components::CMaterialRenderer>();
			materilaRener.SetMaterialAtIndex(0, m_cameraMaterial);
			materilaRener.UpdateMaterialList();
		}



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
		ParsePathTraceScene(PathTrace::GetScene(), editorContext->sceneManager.GetCurrentScene());
		sceneView = new ::Editor::Panels::SceneView("SceneView");
		OVSERVICE(TreeViewPanel).initModel();

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
}