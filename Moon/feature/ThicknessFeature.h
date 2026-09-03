#pragma once
#include "feature/Feature.h"
namespace MOON { 
	class ThicknessFeature :public Feature3D {
	public:
		ThicknessFeature(const std::string& p_name);
		virtual ~ThicknessFeature() override;
		virtual bool execute();
		
		float thickNessValue = 0.5;
		int mode = 0;
		int joinType = 0;
		bool reverse = false;
		bool intersection = false;
		float dir[3];
		float midPoint[3];
		float scale=1.0;
	};
}
