#include <tracy/Tracy.hpp>
#include <Core/Rendering/FramebufferUtil.h>
#include "AView.h"
#include <Rendering/HAL/Profiling.h>
#include "Rendering/HAL/Common/TFramebuffer.h"
#include "Rendering/HAL/Common/TTexture.h"
#include "Core/Global/ServiceLocator.h"
#include "renderer/Context.h"
#include <QMouseEvent>
#include <qapplication.h>
#include <iostream>

Editor::Panels::AView::AView
(
	const std::string& p_title) :
	m_framebuffer(p_title), name(p_title)
{
	::Core::Rendering::FramebufferUtil::SetupFramebuffer(
		m_framebuffer,
		static_cast<uint32_t>(1),
		static_cast<uint32_t>(1),
		true, true, false,false
	);
	::Core::Rendering::FramebufferUtil::SetupFramebuffer(
		m_msaaframebuffer,
		static_cast<uint32_t>(1),
		static_cast<uint32_t>(1),
		true, true, false, true
	);
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
	auto camera = GetCamera();
	auto scene = GetScene();

	if (winWidth > 1 && winHeight > 1 && camera && scene)
	{
		FrameMarkStart(name.c_str());

		m_framebuffer.Resize(winWidth, winHeight);
		m_msaaframebuffer.Resize(winWidth,winHeight);

		InitFrame();

		Rendering::Data::FrameDescriptor frameDescriptor;
		frameDescriptor.renderWidth = winWidth;
		frameDescriptor.renderHeight = winHeight;
		frameDescriptor.camera = camera;
		frameDescriptor.outputMsaaBuffer = m_msaaframebuffer;
		frameDescriptor.presentBuffer = m_framebuffer;

		m_renderer->BeginFrame(frameDescriptor);
		DrawFrame();
		m_renderer->EndFrame();
		::Core::Rendering::FramebufferUtil::CopyFramebufferColor(m_msaaframebuffer, 0, m_framebuffer, 0);
		FrameMarkEnd(name.c_str());
		::Core::Global::ServiceLocator::Get<Editor::Core::Context>().driver->OnFrameCompleted();
	}
}

void Editor::Panels::AView::Present()
{
	m_renderer->Present(m_framebuffer);
}

void Editor::Panels::AView::DrawFrame()
{
	m_renderer->DrawFrame();
}

std::pair<uint16_t, uint16_t> Editor::Panels::AView::GetSafeSize() const
{
	return { mWidth,mHeight };
}

 Core::Rendering::SceneRenderer& Editor::Panels::AView::GetRenderer() 
{
	return *m_renderer.get();
}

Core::Rendering::SceneRenderer::SceneDescriptor Editor::Panels::AView::CreateSceneDescriptor()
{
	auto scene = GetScene();
	assert(scene&&"No scene assigned to this view!");
	return {
		*scene
	};
}

Core::ECS::Actor& Editor::Panels::AView::GetSelectedActor()
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
	m_renderer->Resize(width, height);
}

void Editor::Panels::AView::UnselectActor()
{
	mTargetActor = nullptr;
}

bool Editor::Panels::AView::IsSelectActor()
{
	return mTargetActor != nullptr;
}

namespace Editor::Panels {

	InputState& Editor::Panels::AView::getInutState()
	{
		return input;
	}

	void AView::ClearEvents()
	{
		input.ClearEvents();
	}
	Maths::FVector3 AView::GetRoaterCenter()
	{
		return m_roaterCenter;
	}
	void AView::SetRotaterCenter(const Maths::FVector3& center)
	{
		m_roaterCenter = center;
	}
}



