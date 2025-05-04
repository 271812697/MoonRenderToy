/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Core/Scripting/Common/TScript.h>

namespace Core::Scripting
{
	struct NullScriptContext {};
	using NullScript = TScript<EScriptingLanguage::NONE, NullScriptContext>;
}
