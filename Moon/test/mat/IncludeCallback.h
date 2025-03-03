#pragma once

#include "test/utils/CString.h"//<utils/CString.h>

#include <functional>

namespace TEST {

	struct IncludeResult {
		const utils::CString includeName;
		utils::CString text;
		size_t lineNumberOffset = 0;
		utils::CString name;
	};


	using IncludeCallback = std::function<bool(
		const utils::CString& includedBy,
		IncludeResult& result)>;

}