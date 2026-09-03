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
		std::vector<Part::TopoShape> getBaseTopoFaceShapes();
		Part::TopoShape getBaseTopoEdgeShape();
		std::vector<Part::TopoShape> getBaseTopoEdgeShapes();
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
	class Feature3D :public Feature {
	public:
		Feature3D(const std::string& p_name, const std::string& tag)
			:Feature(p_name, tag)
		{

		}
	};
	class DatumFeature :public Feature
	{
	public:
		DatumFeature(const std::string& p_name, const std::string& tag)
			:Feature(p_name, tag)
		{

		}
	};
	class ProfileFeature :public Feature
	{
	public:
		ProfileFeature(const std::string& p_name, const std::string& tag)
			:Feature(p_name,tag)
		{

		}
	};
}
