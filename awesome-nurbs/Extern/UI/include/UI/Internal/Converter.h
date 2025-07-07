
#pragma once

#include <Maths/FVector2.h>
#include <Maths/FVector4.h>

#include "imgui.h"
#include "UI/Types/Color.h"

namespace UI::Internal
{

	class Converter
	{
	public:

		Converter() = delete;


		static ImVec4 ToImVec4(const Types::Color& p_value);

		static Types::Color ToColor(const ImVec4& p_value);


		static ImVec2 ToImVec2(const Maths::FVector2& p_value);

		static Maths::FVector2 ToFVector2(const ImVec2& p_value);
	};
}