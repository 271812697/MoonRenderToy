#include <tracy/Tracy.hpp>

#include <Core/Rendering/EngineBufferRenderFeature.h>
#include <Core/Rendering/EngineDrawableDescriptor.h>
#include <Rendering/Core/CompositeRenderer.h>

namespace
{
	struct CameraInfo
	{
		Maths::FMatrix4    ubo_View;
		Maths::FMatrix4    ubo_Projection;
		Maths::FVector3    ubo_ViewPos;
		int					ubo_CameraType; //0 orth,1 pers
	};
	struct ViewInfo
	{
		float   ubo_Time;
		int		ubo_screenWidth;
		int		ubo_screenHeigh;
		float	ubo_pad;
	};
	struct ClipPlane
	{
		Maths::FVector4 plane;
		static unsigned long long offset;
	};
	unsigned long long ClipPlane::offset = sizeof(Maths::FMatrix4)+ sizeof(CameraInfo) + sizeof(ViewInfo);
	struct EngineUBO{
		Maths::FMatrix4    ubo_Model;
		CameraInfo ubo_CameraInfo;
		ViewInfo ubo_CustomInfo;
		ClipPlane ubo_plane;
		Maths::FMatrix4    ubo_UserMatrix;

	};
	constexpr size_t kUBOSize = sizeof(EngineUBO);

}

Core::Rendering::EngineBufferRenderFeature::EngineBufferRenderFeature(
	::Rendering::Core::CompositeRenderer& p_renderer,
	::Rendering::Features::EFeatureExecutionPolicy p_executionPolicy
) : 
	ARenderFeature(p_renderer, p_executionPolicy)
{
	m_engineBuffer = std::make_unique<::Rendering::HAL::UniformBuffer>();
	m_engineBuffer->Allocate(kUBOSize, ::Rendering::Settings::EAccessSpecifier::STREAM_DRAW);
	m_startTime = std::chrono::high_resolution_clock::now();
}

void Core::Rendering::EngineBufferRenderFeature::SetCamera(const ::Rendering::Entities::Camera& p_camera)
{
     CameraInfo uboDataPage{
		.ubo_View= Maths::FMatrix4::Transpose(p_camera.GetViewMatrix()),
		.ubo_Projection = Maths::FMatrix4::Transpose(p_camera.GetProjectionMatrix()),
		.ubo_ViewPos = p_camera.GetPosition(),
		.ubo_CameraType = p_camera.GetProjectionMode() == ::Rendering::Settings::EProjectionMode::ORTHOGRAPHIC ? 0 : 1
	};

	m_engineBuffer->Upload(&uboDataPage, ::Rendering::HAL::BufferMemoryRange{
		.offset = sizeof(Maths::FMatrix4), // Skip uploading the first matrix (Model matrix)
		.size = sizeof(uboDataPage)
	});
}

void Core::Rendering::EngineBufferRenderFeature::SetClipPlane(float x, float y, float z, float w)
{
	
	Maths::FVector4 plane = {x,y,z,w};
	m_engineBuffer->Upload(&plane, ::Rendering::HAL::BufferMemoryRange{
	.offset = ClipPlane::offset, // Skip uploading the first matrix (Model matrix)
	.size = sizeof(ClipPlane)
		});
}

void Core::Rendering::EngineBufferRenderFeature::OnBeginFrame(const ::Rendering::Data::FrameDescriptor& p_frameDescriptor)
{
	assert(p_frameDescriptor.camera.has_value()&&"Camera is not set in the frame descriptor");

	auto currentTime = std::chrono::high_resolution_clock::now();
	auto elapsedTime = std::chrono::duration_cast<std::chrono::duration<float>>(currentTime - m_startTime);

	struct
	{
		CameraInfo cameraInfo;
		ViewInfo viewInfo;
	} uboDataPage;
	
	uboDataPage.cameraInfo.ubo_View = Maths::FMatrix4::Transpose(p_frameDescriptor.camera->GetViewMatrix());
	uboDataPage.cameraInfo.ubo_Projection = Maths::FMatrix4::Transpose(p_frameDescriptor.camera->GetProjectionMatrix());
	uboDataPage.cameraInfo.ubo_ViewPos = p_frameDescriptor.camera->GetPosition();
	uboDataPage.cameraInfo.ubo_CameraType = p_frameDescriptor.camera->GetProjectionMode() == ::Rendering::Settings::EProjectionMode::ORTHOGRAPHIC ? 0 : 1;
	uboDataPage.viewInfo.ubo_Time = elapsedTime.count();
	uboDataPage.viewInfo.ubo_screenWidth = p_frameDescriptor.renderWidth;
	uboDataPage.viewInfo.ubo_screenHeigh = p_frameDescriptor.renderHeight;

	
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
	ZoneScoped;

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
