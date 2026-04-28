#pragma once
#include "Gizmo/Widgets/PrimitiveShape.h"
#include <Eigen/Core>

namespace MOON
{
	class PrimitiveCone : public PrimitiveShape
	{
	public:
		PrimitiveCone(const std::string& name);
		virtual ~PrimitiveCone()override;
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