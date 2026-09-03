#pragma once
#include "feature/Feature.h"
#include <Maths/FVector3.h>
namespace MOON
{
	// A datum line is a parametric reference line in 3D space. Unlike boolean
	// features it does not modify a base shape: it only carries its own edge
	// geometry so it can be used later as a reference while modeling.
	class DatumLineFeature : public DatumFeature
	{
	public:
		DatumLineFeature(const std::string& p_name);
		virtual ~DatumLineFeature() override;
		virtual bool execute() override;

		Maths::FVector3 origin{ 0.0f, 0.0f, 0.0f };
		Maths::FVector3 direction{ 0.0f, 0.0f, 1.0f };
		float length = 10.0f;
	};
}
