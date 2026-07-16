#pragma once
#include "core/component/TopoShapeActor.h"
namespace MOON { 
	class Feature :public TopoActor {
	public:
		Feature(const std::string& p_name,const std::string& tag);
		virtual ~Feature() override;
		virtual bool execute();
		void setBaseFeature(Feature* f) {
			m_baseFeature = f;
		}
		void setSubValues(const std::vector<std::string>& values) {
			subValues = values;
		}
		Feature* getBaseFeature() { return m_baseFeature; }
		Part::TopoShape getBaseTopoShape();
		Part::TopoShape getBaseTopoFaceShape();
	    Part::TopoShape& getPreviewShape();
		void makeDone();
	protected:
		Feature* m_baseFeature = nullptr;
		std::vector<std::string> subValues;
	private:
		bool hasInTree = false;
		class Internal;
		Internal* mInternal = nullptr;

	};
}
