#pragma once

#include "test/DriverEnums.h"//<backend/DriverEnums.h>

#include "test/utils/CString.h"//<utils/CString.h>

namespace TEST {

	struct MaterialPushConstant {
		using ShaderStage = ShaderStage;
		using ConstantType = ConstantType;

		utils::CString name;
		ConstantType type;
		ShaderStage stage;

		MaterialPushConstant() = default;
		MaterialPushConstant(const char* name, ConstantType type, ShaderStage stage)
			: name(name),
			type(type),
			stage(stage) {}
	};

}
