#pragma once
#include <concepts>

namespace Rendering::Core
{
	class ARenderPass;
}

namespace Rendering::Types
{
	template<typename T>
	concept RenderPassType = std::derived_from<T, Rendering::Core::ARenderPass>;
}
