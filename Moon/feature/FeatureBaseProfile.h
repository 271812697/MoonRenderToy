#pragma once
#include "feature/Feature.h"
#include "TopoShape.h"
namespace MOON { 
	class SketcherFeature;
	class FeatureBaseProfile :public Feature {
	public:
		FeatureBaseProfile(const std::string& p_name,const std::string& tag);
		virtual ~FeatureBaseProfile() override;
		virtual bool execute()override;
		Part::TopoShape getProfileFace();
		void setProfile(SketcherFeature* f) {
			mProfile = f;
		}
	
	protected:
		SketcherFeature* mProfile = nullptr;
	};
}
