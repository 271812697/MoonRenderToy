/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once

#include "OvCore/Helpers/GUIDrawer.h"

namespace OvCore::Helpers
{

	template<typename T>
	inline std::string GUIDrawer::GetFormat()
	{
		if constexpr (std::is_same<T, double>::value) return "%.5f";
		else if constexpr (std::is_same<T, float>::value) return "%.3f";
		else return "%d";
	}

}