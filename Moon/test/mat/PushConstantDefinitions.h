#pragma once

//#include <private/filament/EngineEnums.h>
#include "PushConstantInfo.h"//<private/filament/>

#include "test/utils/FixedCapacityVector.h"//<>

#include <tuple>

namespace TEST {

	constexpr char PUSH_CONSTANT_STRUCT_VAR_NAME[] = "pushConstants";

	utils::FixedCapacityVector<MaterialPushConstant> const PUSH_CONSTANTS = {
		{
			"morphingBufferOffset",
			ConstantType::INT,
			ShaderStage::VERTEX,
		},
	};

	// Make sure that the indices defined in filabridge match the actual array indices defined here.
	//static_assert(static_cast<uint8_t>(PushConstantIds::MORPHING_BUFFER_OFFSET) == 0u);

}