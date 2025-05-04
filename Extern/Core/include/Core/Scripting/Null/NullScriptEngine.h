/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Core/Scripting/Common/TScriptEngine.h>

namespace Core::Scripting
{
	struct NullScriptEngineContext {};
	using NullScriptEngineBase = TScriptEngine<EScriptingLanguage::NONE, NullScriptEngineContext>;
	using NullScriptEngine = NullScriptEngineBase;
}
