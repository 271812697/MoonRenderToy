/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <map>
#include <chrono>

#include <Rendering/Features/ARenderFeature.h>
#include <Rendering/HAL/UniformBuffer.h>
#include <Rendering/Entities/Camera.h>

namespace Core::Rendering
{
	/**
	* Render feature handling engine buffer (UBO) updates
	*/
	class EngineBufferRenderFeature : public ::Rendering::Features::ARenderFeature
	{
	public:
		/**
		* Constructor
		* @param p_renderer
		*/
		EngineBufferRenderFeature(::Rendering::Core::CompositeRenderer& p_renderer);

	protected:
		virtual void OnBeginFrame(const ::Rendering::Data::FrameDescriptor& p_frameDescriptor) override;
		virtual void OnEndFrame() override;
		virtual void OnBeforeDraw(::Rendering::Data::PipelineState& p_pso, const ::Rendering::Entities::Drawable& p_drawable) override;

	protected:
		std::chrono::high_resolution_clock::time_point m_startTime;
		std::unique_ptr<::Rendering::HAL::UniformBuffer> m_engineBuffer;
		::Rendering::Data::FrameDescriptor m_cachedFrameDescriptor;
	};
}
