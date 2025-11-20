#pragma once
#include "Rendering/Core/CompositeRenderer.h"
#include "Rendering/Features/ARenderFeature.h"
#include "Rendering/Data/FrameInfo.h"

namespace Rendering::Features
{
	class FrameInfoRenderFeature : public Rendering::Features::ARenderFeature
	{
	public:
		/**
		* Constructor
		* @param p_renderer
		* @param p_executionPolicy
		*/
		FrameInfoRenderFeature(
			Rendering::Core::CompositeRenderer& p_renderer,
			Rendering::Features::EFeatureExecutionPolicy p_executionPolicy
		);

		/**
		* Destructor
		*/
		virtual ~FrameInfoRenderFeature();

		/**
		* Return a reference to the last frame info
		* @note Will throw an error if called during the rendering of a frame
		*/
		const Rendering::Data::FrameInfo& GetFrameInfo() const;

	protected:
		virtual void OnBeginFrame(const Data::FrameDescriptor& p_frameDescriptor) override;
		virtual void OnEndFrame() override;
		virtual void OnAfterDraw(const Rendering::Entities::Drawable& p_drawable) override;

	private:
		bool m_isFrameInfoDataValid;
		Rendering::Data::FrameInfo m_frameInfo;
		Tools::Eventing::ListenerID m_postDrawListener;
	};
}
