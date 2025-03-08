#pragma once

#include <unordered_map>
#include <string>
#include <variant>

#include "JsonishLexeme.h"
#include "JsonishParser.h"

#include "MaterialBuilder.h"
namespace TEST {
	class MaterialBuilder;
	class ParametersProcessor {

	public:
		ParametersProcessor();
		~ParametersProcessor() = default;
		bool process(MaterialBuilder& builder, const JsonishObject& jsonObject);
		bool process(MaterialBuilder& builder, const std::string& key, const std::string& value);

	private:

		using Callback = bool (*)(MaterialBuilder& builder, const JsonishValue& value);

		struct ParameterInfo {
			Callback callback;
			JsonishValue::Type rootAssert;
		};

		std::unordered_map<std::string, ParameterInfo> mParameters;
	};

}
