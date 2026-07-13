#pragma once
#include "core/component/TopoShapeActor.h"
namespace MOON { 
	class Feature :public TopoActor {
	public:
		Feature(const std::string& p_name);
		virtual ~Feature() override;
		virtual bool execute();
		void setBaseFeature(Feature* f) {
			m_baseFeature = f;
		}
		void setSubValues(const std::vector<std::string>& values) {
			subValues = values;
		}
		Feature* getBaseFeature() { return m_baseFeature; }
		Part::TopoShape& getBaseTopoShape();
		Part::TopoShape getVerifyTopoFace();
	private:
		Feature* m_baseFeature = nullptr;
		std::vector<std::string> subValues;

	};
}
