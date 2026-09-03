#pragma once
#include "feature/Feature.h"
namespace MOON { 

	class SketcherObj;
	class SketcherFeature :public ProfileFeature {
	public:
		SketcherFeature(const std::string& p_name);
		SketcherObj* getSketcherObj();
		virtual ~SketcherFeature() override;
		virtual bool execute();
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}
