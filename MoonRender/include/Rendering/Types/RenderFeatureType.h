#pragma once
#include <concepts>

namespace Rendering::Features
{
	class ARenderFeature;
}

namespace Rendering::Types
{
	template<typename T>
	concept RenderFeatureType = std::derived_from<T, Rendering::Features::ARenderFeature>;
}
