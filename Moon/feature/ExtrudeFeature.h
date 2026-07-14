#pragma once
#include "feature/Feature.h"
#include <gp_Vec.hxx>
namespace MOON { 
	class ExtrudeFeature :public Feature {
	public:
		ExtrudeFeature(const std::string& p_name);
		virtual ~ExtrudeFeature() override;
		virtual bool execute();
		
        float lengthForward =10 ;
        double angleForward = 0;
        double lengthRev = 10;
        double angleRev = 0;
        int dirType = 0;       // 0=正向,1=反向,2=双向,3=对称
		int extrudeType = 0;   //0 =length,1=through all,2=uptoface
		int addSubType = 0;//0=Add,1=sub
		gp_Vec finalDir;
		Part::TopoShape upToFace;
		Part::TopoShape supportShape;


	};
}
