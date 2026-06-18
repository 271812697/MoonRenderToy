#pragma once
#include "Interactive/Widgets/PrimitiveShape.h"
#include <Eigen/Core>

namespace MOON
{
	class PrimitiveCylinder : public PrimitiveShape
	{
	public:
		PrimitiveCylinder(const std::string& name);
		virtual ~PrimitiveCylinder()override;
		virtual void onUpdate()override;
		virtual void createTopoShape()override;
		

	private:
		Eigen::Vector3f translation;
		Eigen::Vector3f normal;
		float height;
		float radiusTop;
		float radiusBottom;
		
	};
}