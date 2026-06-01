#pragma once
#include <string>
#include <array>
#include <memory>
#include "Rendering/Settings/DriverSettings.h"
#include "Rendering/Settings/ERenderingCapability.h"
#include "Rendering/Settings/EPrimitiveMode.h"
#include "Rendering/Settings/ERasterizationMode.h"
#include "Rendering/Settings/EComparaisonAlgorithm.h"
#include "Rendering/Settings/EOperation.h"
#include "Rendering/Settings/ECullFace.h"
#include "Rendering/Settings/ECullingOptions.h"
#include "Rendering/Settings/EPixelDataFormat.h"
#include "Rendering/Settings/EPixelDataType.h"
#include "Rendering/Data/PipelineState.h"
#include "Rendering/Resources/IMesh.h"
#include <Maths/FVector4.h>
#include <Tools/Utils/OptRef.h>
namespace Rendering::Context
{

	class Driver final
	{
	public:
		Driver(const Settings::DriverSettings& p_driverSettings);

		~Driver();

		void OnFrameCompleted();
		void SetViewport(
			uint32_t p_x,
			uint32_t p_y,
			uint32_t p_width,
			uint32_t p_height
		);
		void Clear(
			bool p_colorBuffer,
			bool p_depthBuffer,
			bool p_stencilBuffer,
			const Maths::FVector4& p_color = Maths::FVector4::Zero
		);
		void Draw(
			Rendering::Data::PipelineState p_pso,
			const Resources::IMesh& p_mesh,
			Settings::EPrimitiveMode p_primitiveMode = Settings::EPrimitiveMode::TRIANGLES,
			uint32_t p_instances = 1
		);
		void Draw(
			Rendering::Data::PipelineState p_pso,
			const Resources::IMesh& p_mesh,
			int index,
			Settings::EPrimitiveMode p_primitiveMode = Settings::EPrimitiveMode::TRIANGLES,
			uint32_t p_instances = 1
		);
		Data::PipelineState CreatePipelineState() const;
		std::string_view GetVendor() const;
		std::string_view GetHardware() const;
		std::string_view GetVersion() const;
		std::string_view GetShadingLanguageVersion() const;

	public:
		void SetPipelineState(Data::PipelineState p_state);
		void ResetPipelineState();

	private:
		std::string m_vendor;
		std::string m_hardware;
		std::string m_version;
		std::string m_shadingLanguageVersion;
		Data::PipelineState m_defaultPipelineState;
		Data::PipelineState m_pipelineState;
	};
}