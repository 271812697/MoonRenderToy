#pragma once
#include <gp_Dir.hxx>
namespace Part {
	class TopoShape;
}
namespace MOON {
	class ShapeHelper {
	public:
		ShapeHelper();
		virtual ~ShapeHelper();
		virtual bool generateShape();
		void previewShape();
		void generateFinalShape();
		void clearPreviewShape();
		Part::TopoShape& getPreviewShape();
		Part::TopoShape& getGenerateShape();
	protected:
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