/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <tracy/Tracy.hpp>

#include <Core/Rendering/FramebufferUtil.h>

#include "AView.h"

#include <Rendering/HAL/Profiling.h>
#include "Rendering/HAL/Common/TFramebuffer.h"
#include "Rendering/HAL/Common/TTexture.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/Context.h"

Editor::Panels::AView::AView
(
	const std::string& p_title
) :
	m_framebuffer(p_title)
{
	::Core::Rendering::FramebufferUtil::SetupFramebuffer(
		m_framebuffer,
		static_cast<uint32_t>(1),
		static_cast<uint32_t>(1),
		true, true, false
	);


}

Editor::Panels::AView::~AView()
{
}

void Editor::Panels::AView::Update(float p_deltaTime)
{
}



void Editor::Panels::AView::InitFrame()
{
	m_renderer->AddDescriptor<::Core::Rendering::SceneRenderer::SceneDescriptor>(
		CreateSceneDescriptor()
	);
}

void Editor::Panels::AView::Render()
{
	auto [winWidth, winHeight] = GetSafeSize();
	//m_image->size = Maths::FVector2(static_cast<float>(winWidth), static_cast<float>(winHeight));

	auto camera = GetCamera();
	auto scene = GetScene();

	if (winWidth > 0 && winHeight > 0 && camera && scene)
	{
		FrameMarkStart(name.c_str());

		m_framebuffer.Resize(winWidth, winHeight);

		InitFrame();

		Rendering::Data::FrameDescriptor frameDescriptor;
		frameDescriptor.renderWidth = winWidth;
		frameDescriptor.renderHeight = winHeight;
		frameDescriptor.camera = camera;
		//frameDescriptor.outputBuffer = m_framebuffer;

		m_renderer->BeginFrame(frameDescriptor);
		DrawFrame();
		m_renderer->EndFrame();
		FrameMarkEnd(name.c_str());
		::Core::Global::ServiceLocator::Get<::Editor::Core::Context>().driver->OnFrameCompleted();
	}
}

void Editor::Panels::AView::DrawFrame()
{
	m_renderer->DrawFrame();
}

std::pair<uint16_t, uint16_t> Editor::Panels::AView::GetSafeSize() const
{
	return { mWidth,mHeight };
}

const Core::Rendering::SceneRenderer& Editor::Panels::AView::GetRenderer() const
{
	return *m_renderer.get();
}

Core::Rendering::SceneRenderer::SceneDescriptor Editor::Panels::AView::CreateSceneDescriptor()
{
	auto scene = GetScene();
	assert(scene, "No scene assigned to this view!");
	return {
		*scene
	};
}

::Core::ECS::Actor& Editor::Panels::AView::GetSelectedActor()
{
	return *mTargetActor;
}

void Editor::Panels::AView::SelectActor(::Core::ECS::Actor& actor)
{
	mTargetActor = &actor;
}

void Editor::Panels::AView::Resize(int width, int height)
{
	mWidth = width;
	mHeight = height;
}

void Editor::Panels::AView::UnselectActor()
{
	mTargetActor = nullptr;
}

bool Editor::Panels::AView::IsSelectActor()
{
	return mTargetActor != nullptr;
}
