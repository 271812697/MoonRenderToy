
#include "UI/Internal/Converter.h"

ImVec4 UI::Internal::Converter::ToImVec4(const Types::Color & p_value)
{
	return ImVec4(p_value.r, p_value.g, p_value.b, p_value.a);
}

UI::Types::Color UI::Internal::Converter::ToColor(const ImVec4 & p_value)
{
	return Types::Color(p_value.x, p_value.y, p_value.z, p_value.w);
}

ImVec2 UI::Internal::Converter::ToImVec2(const Maths::FVector2 & p_value)
{
	return ImVec2(p_value.x, p_value.y);
}

Maths::FVector2 UI::Internal::Converter::ToFVector2(const ImVec2 & p_value)
{
	return Maths::FVector2(p_value.x, p_value.y);
}
