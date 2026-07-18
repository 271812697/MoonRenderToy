#pragma once
#include <gp_Dir.hxx>
#include <string>
namespace Part {
	class TopoShape;
}
namespace MOON {
	class Feature;
	class ShapeHelper {
	public:
		ShapeHelper(Feature* feature);
		virtual ~ShapeHelper();
		
		void previewShape();
		void generateFinalShape();
		void clearPreviewShape();
		void setFeature(Feature* feature);
		Feature* getFeature();
		void setFeatureSubValues(const std::vector<std::string>& subValues);
		//void 
		Part::TopoShape& getPreviewShape();
		
	protected:
		void onSelectAny();
		virtual void onSelectEdge(const std::vector<Part::TopoShape>& edge);
		virtual void onSelectFace(const std::vector<Part::TopoShape>& face);
		void setGenerateShapeName(const char* name);
		struct PreviewOption {
			bool isTransparent = true;
			float r=1.0f, g=1.0f, b=1.0f, a = 0.4f;
			bool isBlend = true;
			bool useDomainColor = true;
		};
		PreviewOption mPreviewOption;
		/// Find a valid face to extrude up to
		static void getUpToFace(
			Part::TopoShape& upToFace,
			const Part::TopoShape& support,
			const Part::TopoShape& sketchshape,
			const std::string& method,
			gp_Dir& dir
		);
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}