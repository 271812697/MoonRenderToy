/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#include <Core/Scripting/Null/NullScript.h>

template<>
Core::Scripting::NullScript::TScript() {}

template<>
Core::Scripting::NullScript::~TScript() {}

template<>
bool Core::Scripting::NullScript::IsValid() const { return true; }
