
#include "FeatureBaseProfile.h"
#include "SketcherFeature.h"
#include "Sketcher/SketcherObj.h"
namespace MOON {
	
	FeatureBaseProfile::FeatureBaseProfile(const std::string& p_name, const std::string& tag) :Feature3D(p_name,tag)
	{

	}
	FeatureBaseProfile::~FeatureBaseProfile()
	{
		
	}
	bool FeatureBaseProfile::execute()
	{
		return false;
	}
	Part::TopoShape FeatureBaseProfile::getProfileFace()
	{
		if (mProfile) {
			return mProfile->getSketcherObj()->getDoneFaceShape();
		}
		return getBaseTopoFaceShape();
	}
}