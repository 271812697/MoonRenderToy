#pragma once
#include <cstdint>
#include "surfacemesh/mat_vec.h"

namespace MOON {

	using Scalar = float;

	using Point = Vector<Scalar, 3>;

	using Normal = Vector<Scalar, 3>;

	using Color = Vector<Scalar, 3>;

	using TexCoord = Vector<Scalar, 2>;
	using IndexType = std::uint_least32_t;
#define MAX_INDEX UINT_LEAST32_MAX

}