/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Rendering/Core/CompositeRenderer.h>

#include "Core/Rendering/EngineBufferRenderFeature.h"
#include "Core/Rendering/EngineDrawableDescriptor.h"

namespace
{
	constexpr size_t kUBOSize =
		sizeof(Maths::FMatrix4) +	// Model matrix
		sizeof(Maths::FMatrix4) +	// View matrix
		sizeof(Maths::FMatrix4) +	// Projection matrix
		sizeof(Maths::FVector3) +	// Camera position
		sizeof(float) +				// Elapsed time
		sizeof(Maths::FMatrix4);	// User matrix
}

Core::Rendering::EngineBufferRenderFeature::EngineBufferRenderFeature(::Rendering::Core::CompositeRenderer& p_renderer)
	: ARenderFeature(p_renderer)
{
	m_engineBuffer = std::make_unique<::Rendering::HAL::UniformBuffer>();

	m_engineBuffer->Allocate(kUBOSize, ::Rendering::Settings::EAccessSpecifier::STREAM_DRAW);

	m_startTime = std::chrono::high_resolution_clock::now();
}

void Core::Rendering::EngineBufferRenderFeature::OnBeginFrame(const ::Rendering::Data::FrameDescriptor& p_frameDescriptor)
{
	m_cachedFrameDescriptor = p_frameDescriptor;

	auto currentTime = std::chrono::high_resolution_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - m_startTime);

	struct
	{
		Maths::FMatrix4 viewMatrix;
		Maths::FMatrix4 projectionMatrix;
		Maths::FVector3 cameraPosition;
		float elapsedTime;
	} uboDataPage{
		.viewMatrix = Maths::FMatrix4::Transpose(p_frameDescriptor.camera->GetViewMatrix()),
		.projectionMatrix = Maths::FMatrix4::Transpose(p_frameDescriptor.camera->GetProjectionMatrix()),
		.cameraPosition = p_frameDescriptor.camera->GetPosition(),
		.elapsedTime = elapsedTime.count()
	};

	m_engineBuffer->Upload(&uboDataPage, ::Rendering::HAL::BufferMemoryRange{
		.offset = sizeof(Maths::FMatrix4), // Skip uploading the first matrix (Model matrix)
		.size = sizeof(uboDataPage)
		});

	m_engineBuffer->Bind(0);
}

void Core::Rendering::EngineBufferRenderFeature::OnEndFrame()
{
	m_engineBuffer->Unbind();
}

void Core::Rendering::EngineBufferRenderFeature::OnBeforeDraw(::Rendering::Data::PipelineState& p_pso, const ::Rendering::Entities::Drawable& p_drawable)
{
	Tools::Utils::OptRef<const EngineDrawableDescriptor> descriptor;

	if (p_drawable.TryGetDescriptor<EngineDrawableDescriptor>(descriptor))
	{
		const auto modelMatrix = Maths::FMatrix4::Transpose(descriptor->modelMatrix);

		// Upload model matrix (First matrix in the UBO)
		m_engineBuffer->Upload(&modelMatrix, ::Rendering::HAL::BufferMemoryRange{
			.offset = 0,
			.size = sizeof(modelMatrix)
			});

		// Upload user matrix (Last matrix in the UBO)
		m_engineBuffer->Upload(&descriptor->userMatrix, ::Rendering::HAL::BufferMemoryRange{
			.offset = kUBOSize - sizeof(modelMatrix),
			.size = sizeof(modelMatrix)
			});
	}
}
