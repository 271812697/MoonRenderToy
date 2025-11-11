#include <tracy/Tracy.hpp>
#include <OvCore/Rendering/FramebufferUtil.h>
#include "AView.h"
#include <OvRendering/HAL/Profiling.h>
#include "OvRendering/HAL/Common/TFramebuffer.h"
#include "OvRendering/HAL/Common/TTexture.h"
#include "OvCore/Global/ServiceLocator.h"
#include "renderer/Context.h"
#include <QMouseEvent>
#include <qapplication.h>
#include <iostream>

OvEditor::Panels::AView::AView
(
	const std::string& p_title) :
	m_framebuffer(p_title), name(p_title)
{
	OvCore::Rendering::FramebufferUtil::SetupFramebuffer(
		m_framebuffer,
		static_cast<uint32_t>(1),
		static_cast<uint32_t>(1),
		true, true, false,false
	);
	OvCore::Rendering::FramebufferUtil::SetupFramebuffer(
		m_msaaframebuffer,
		static_cast<uint32_t>(1),
		static_cast<uint32_t>(1),
		true, true, false, true
	);
}

void OvEditor::Panels::AView::Update(float p_deltaTime)
{

}



void OvEditor::Panels::AView::InitFrame()
{
	m_renderer->AddDescriptor<OvCore::Rendering::SceneRenderer::SceneDescriptor>(
		CreateSceneDescriptor()
	);
}

void OvEditor::Panels::AView::Render()
{
	auto [winWidth, winHeight] = GetSafeSize();
	auto camera = GetCamera();
	auto scene = GetScene();

	if (winWidth > 0 && winHeight > 0 && camera && scene)
	{
		FrameMarkStart(name.c_str());

		m_framebuffer.Resize(winWidth, winHeight);
		m_msaaframebuffer.Resize(winWidth,winHeight);

		InitFrame();

		OvRendering::Data::FrameDescriptor frameDescriptor;
		frameDescriptor.renderWidth = winWidth;
		frameDescriptor.renderHeight = winHeight;
		frameDescriptor.camera = camera;
		frameDescriptor.outputMsaaBuffer = m_msaaframebuffer;
		frameDescriptor.presentBuffer = m_framebuffer;

		m_renderer->BeginFrame(frameDescriptor);
		DrawFrame();
		m_renderer->EndFrame();
		OvCore::Rendering::FramebufferUtil::CopyFramebufferColor(m_msaaframebuffer, 0, m_framebuffer, 0);
		FrameMarkEnd(name.c_str());
		OvCore::Global::ServiceLocator::Get<::OvEditor::Core::Context>().driver->OnFrameCompleted();
	}
}

void OvEditor::Panels::AView::Present()
{
	m_renderer->Present(m_framebuffer);
}

void OvEditor::Panels::AView::DrawFrame()
{
	m_renderer->DrawFrame();
}

std::pair<uint16_t, uint16_t> OvEditor::Panels::AView::GetSafeSize() const
{
	return { mWidth,mHeight };
}

const OvCore::Rendering::SceneRenderer& OvEditor::Panels::AView::GetRenderer() const
{
	return *m_renderer.get();
}

OvCore::Rendering::SceneRenderer::SceneDescriptor OvEditor::Panels::AView::CreateSceneDescriptor()
{
	auto scene = GetScene();

	OVASSERT(scene, "No scene assigned to this view!");

	return {
		*scene
	};
}

OvCore::ECS::Actor& OvEditor::Panels::AView::GetSelectedActor()
{
	return *mTargetActor;
}

void OvEditor::Panels::AView::SelectActor(::OvCore::ECS::Actor& actor)
{
	mTargetActor = &actor;
}

void OvEditor::Panels::AView::Resize(int width, int height)
{
	mWidth = width;
	mHeight = height;
}

void OvEditor::Panels::AView::UnselectActor()
{
	mTargetActor = nullptr;
}

bool OvEditor::Panels::AView::IsSelectActor()
{
	return mTargetActor != nullptr;
}

namespace OvEditor::Panels {

	InputState& OvEditor::Panels::AView::getInutState()
	{
		return input;
	}

	void AView::ClearEvents()
	{
		input.ClearEvents();
	}
}



