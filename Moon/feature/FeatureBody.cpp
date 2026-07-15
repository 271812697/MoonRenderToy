#include "core/component/TopoShapeActor.h"

#include "editor/View/sceneview/viewerwidget.h"
#include "core/component/CTopoShape.h"
#include <Core/Global/ServiceLocator.h>
#include "feature/FeatureBody.h"
#include "SketcherFeature.h"
#include "Sketcher/SketcherObj.h"
#include "TopoShape.h"

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
	FeatureBody::FeatureBody(const std::string& p_name) :TopoActor( p_name, "Body", true, false), mInternal(new Internal(this))
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

	Feature* FeatureBody::getLastBaseFeature()
	{
		if (mInternal->featureList.size() > 0) {
			for (int i = mInternal->featureList.size() - 1; i >= 0; i--) {
				SketcherFeature* feature = dynamic_cast<SketcherFeature*>(mInternal->featureList[i]);
				if (!feature) {
					return mInternal->featureList[i];
				}
			}
		}
		return nullptr;
	}
}