#pragma once
#include "Gizmo/Widgets/PrimitiveShape.h"
#include <Eigen/Core>

namespace MOON
{
	class PrimitiveSphere : public PrimitiveShape
	{
	public:
		PrimitiveSphere(const std::string& name);
		virtual ~PrimitiveSphere()override;
		virtual void onUpdate()override;
		virtual void createTopoShape()override;
		

	private:
		Eigen::Vector3f translation;
		float radius;
		
	};
}