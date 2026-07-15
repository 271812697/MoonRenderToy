#pragma once
#include "core/component/TopoShapeActor.h"
namespace MOON { 
	class Feature;
	class FeatureBody :public TopoActor {
	public:
		FeatureBody(const std::string& p_name);
		static FeatureBody& instance();
		virtual ~FeatureBody() override;
		void addFeature(Feature* feature);
		Feature* getLastBaseFeature();
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}
