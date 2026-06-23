#pragma once
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
		Part::TopoShape& getPreviewShape();
		Part::TopoShape& getGenerateShape();
	protected:
		struct PreviewOption {
			bool isTransparent = true;
			float r=1.0f, g=1.0f, b=1.0f, a = 0.4f;
			bool isBlend = true;
			bool useDomainColor = true;
		};
		PreviewOption mPreviewOption;
	private:
		class Internal;
		Internal* mInternal = nullptr;
	};
}