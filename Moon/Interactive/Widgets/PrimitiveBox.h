#pragma once
#include "Interactive/Widgets/PrimitiveShape.h"

namespace MOON
{
	class PrimitiveBox : public PrimitiveShape
	{
	public:
		PrimitiveBox(const std::string& name);
		virtual ~PrimitiveBox()override;
		virtual void onUpdate()override;
		virtual void createTopoShape()override;
		

	private:
		Eigen::Vector3f translation;
		Eigen::Vector3f scale;
		Eigen::Matrix3f rot;
	};
}