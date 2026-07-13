#pragma once
#include "core/component/TopoShapeActor.h"
namespace MOON { 
	class Feartue :public TopoActor {
	public:
		Feartue(const std::string& p_name);
		virtual ~Feartue() override;
	};
}
