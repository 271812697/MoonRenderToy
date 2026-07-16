#pragma once
#include <string>
namespace MOON { 
	class Feature;
	class FeatureBody  {
	public:
		FeatureBody(const std::string& p_name);
		static FeatureBody& instance();
		virtual ~FeatureBody() ;
		void addFeature(Feature* feature);
		bool removeFeature(Feature* feature);
		void populateFeature(Feature* feature);
		Feature* getLastBaseFeature(Feature* feature);
		bool setBaseFeatureFor(Feature* feature);
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}
