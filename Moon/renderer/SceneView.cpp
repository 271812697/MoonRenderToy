#include "DebugSceneRenderer.h"
#include "PickingRenderPass.h"

#include "Core/Global/ServiceLocator.h"
#include "Core/SceneSystem/SceneManager.h"
#include "SceneView.h"
#include <QMouseEvent>
namespace Editor::Panels
{
	Editor::Panels::SceneView::SceneView
	(
		const std::string& p_title
	) : AViewControllable(p_title),
		m_sceneManager(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().sceneManager)
	{
		m_renderer = std::make_unique<::Editor::Rendering::DebugSceneRenderer>(*::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().driver);

		m_camera.SetFar(5000.0f);

		m_fallbackMaterial.SetShader(::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().shaderManager[":Shaders\\Unlit.ovfx"]);
		m_fallbackMaterial.SetProperty("u_Diffuse", Maths::FVector4{ 1.f, 0.f, 1.f, 1.0f });
		m_fallbackMaterial.SetProperty("u_DiffuseMap", static_cast<::Rendering::Resources::Texture*>(nullptr));


		::Core::ECS::Actor::DestroyedEvent += [this](const ::Core::ECS::Actor& actor)
			{
				if (m_highlightedActor.has_value() && m_highlightedActor.value().GetID() == actor.GetID())
				{
					m_highlightedActor.reset();
				}
			};
	}

	void Editor::Panels::SceneView::Update(float p_deltaTime)
	{
		AViewControllable::Update(p_deltaTime);
	}

	void Editor::Panels::SceneView::InitFrame()
	{
		AViewControllable::InitFrame();

		Tools::Utils::OptRef<::Core::ECS::Actor> selectedActor;


		m_renderer->AddDescriptor<Rendering::DebugSceneRenderer::DebugSceneDescriptor>({
			m_currentOperation,
			m_highlightedActor,
			selectedActor,
			m_highlightedGizmoDirection
			});
	}

	::Core::SceneSystem::Scene* Editor::Panels::SceneView::GetScene()
	{
		return m_sceneManager.GetCurrentScene();
	}

	void SceneView::ReceiveEvent(QEvent* e)
	{

		if (e == nullptr)
			return;

		const QEvent::Type t = e->type();
		if (t == QEvent::Resize) {
			QResizeEvent* ev = static_cast<QResizeEvent*>(e);
			mWidth = ev->size().width();
			mHeight = ev->size().height();
		}
		m_cameraController.ReceiveEvent(e);


	}

	::Core::Rendering::SceneRenderer::SceneDescriptor Editor::Panels::SceneView::CreateSceneDescriptor()
	{
		auto descriptor = AViewControllable::CreateSceneDescriptor();
		descriptor.fallbackMaterial = m_fallbackMaterial;

		if (true)
		{
			auto& scene = *GetScene();

			if (auto mainCameraComponent = scene.FindMainCamera())
			{
				auto& sceneCamera = mainCameraComponent->GetCamera();
				m_camera.SetFrustumGeometryCulling(sceneCamera.HasFrustumGeometryCulling());
				m_camera.SetFrustumLightCulling(sceneCamera.HasFrustumLightCulling());
				descriptor.frustumerride = sceneCamera.GetFrustum();
			}
		}
		return descriptor;
	}

	void Editor::Panels::SceneView::DrawFrame()
	{
		Editor::Panels::AViewControllable::DrawFrame();
		HandleActorPicking();
	}

	bool IsResizing()
	{
		return false;
	}

	void Editor::Panels::SceneView::HandleActorPicking()
	{
	}
}