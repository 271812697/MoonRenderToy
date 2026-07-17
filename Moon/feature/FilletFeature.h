#pragma once
#include "feature/Feature.h"
namespace MOON { 
	class FilletFeature :public Feature {
	public:
		FilletFeature(const std::string& p_name);
		virtual ~FilletFeature() override;
		virtual bool execute();
		float radius = 0.5;
		bool useAllEdges = false;
		float origin1[3];
		float origin2[3];
		float dir1[3];
		float dir2[3];
		float len=0;
	};
}
