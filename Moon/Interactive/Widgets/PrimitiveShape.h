#pragma once
#include "Interactive/EventWidget.h"
#include <Eigen/Core>

namespace MOON
{
	class PrimitiveShape : public EventWidget
	{
	public:
		PrimitiveShape(const std::string& name);
		virtual ~PrimitiveShape()override;
		virtual void createTopoShape();
		virtual void onKeyPress(const std::string& key)override;
	};
}