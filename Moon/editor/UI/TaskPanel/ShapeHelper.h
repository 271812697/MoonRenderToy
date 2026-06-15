#pragma once
namespace Part {
	class TopoShape;
}
namespace MOON {
	class ShapeHelper {
	public:
		ShapeHelper();
		virtual ~ShapeHelper();
		virtual bool generatePreviewShape();
		void previewShape();
		void generateFinalShape();
		Part::TopoShape& getPreviewShape();
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}