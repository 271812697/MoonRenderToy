
#include "feature/FeatureBody.h"
#include "SketcherFeature.h"
#include "feature/FeatureBaseProfile.h"

namespace MOON {
	class FeatureBody::Internal {
	public:
		Internal(FeatureBody* f):self(f) {}
		~Internal() {}
	private:
		friend FeatureBody;
		FeatureBody* self = nullptr;
		std::vector<Feature*>featureList;
	};
	FeatureBody::FeatureBody(const std::string& p_name) :mInternal(new Internal(this))
	{

	}
	FeatureBody::~FeatureBody()
	{
		delete mInternal;
	}
	FeatureBody& FeatureBody::instance() {
		static FeatureBody* body = nullptr;
		if (body == nullptr) {
			body = new FeatureBody("Body");
		}
		return *body;
	}
	void FeatureBody::addFeature(Feature* feature)
	{
		if (feature) {
			mInternal->featureList.push_back(feature);
		}
	}

	bool FeatureBody::removeFeature(Feature* feature)
	{
		if (feature) {
			int k = 0;
			for (int i = 0;i < mInternal->featureList.size();i++) {
				if (mInternal->featureList[i] != feature) {
					mInternal->featureList[k++] = mInternal->featureList[i];
				}
			}
			mInternal->featureList.resize(k);
			for (int i = 0;i < mInternal->featureList.size();i++) {
				if (mInternal->featureList[i]->getBaseFeature() == feature) {
					mInternal->featureList[i]->setBaseFeature(nullptr);
				}
			}
		}
		return false;
	}

	void FeatureBody::populateFeature(Feature* feature)
	{
		if (feature) {
			std::vector<Feature*>stack;
			stack.push_back(feature);
			while (!stack.empty()) {
				Feature* curFeature = stack.back(); stack.pop_back();
				for (int i = 0; i < mInternal->featureList.size(); i++) {
					if (mInternal->featureList[i]->getBaseFeature()== curFeature) {
						mInternal->featureList[i]->execute();
						mInternal->featureList[i]->makeDone();
					}
					else {
						FeatureBaseProfile*  profile=dynamic_cast<FeatureBaseProfile*>(mInternal->featureList[i]);
						if (profile) {
							if (profile->getProfile() == curFeature) {
								mInternal->featureList[i]->execute();
								mInternal->featureList[i]->makeDone();
							}
						}
					}
				}
			}
		}
	}

	Feature* FeatureBody::getLastBaseFeature(Feature* target)
	{
		if (mInternal->featureList.size() > 0) {
			for (int i = mInternal->featureList.size()-1;i>=0;i--) {
				if (mInternal->featureList[i] == target) {
					for (int k = i - 1; k >= 0; k--) {
						Feature3D* feature = dynamic_cast<Feature3D*>(mInternal->featureList[k]);
						if (feature) {
							return mInternal->featureList[k];
						}
					}
				}
			}
		}
		return nullptr;
	}
	bool FeatureBody::setBaseFeatureFor(Feature* feature)
	{
		if (feature) {
			Feature* baseFeature=getLastBaseFeature(feature);
			if (baseFeature) {
				feature->setBaseFeature(baseFeature);
				return true;
			}
		}
		return false;
	}
}