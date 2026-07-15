#include "core/component/TopoShapeActor.h"

#include "editor/View/sceneview/viewerwidget.h"
#include "core/component/CTopoShape.h"
#include <Core/Global/ServiceLocator.h>
#include "Feature.h"
#include "SketcherFeature.h"
#include "Sketcher/SketcherObj.h"
#include "TopoShape.h"

namespace MOON {
	class Feature::Internal {
	public:
		Internal(Feature* f):self(f) {
		}
		~Internal() {
		}
	private:
		friend Feature;
		Feature* self = nullptr;
		Part::TopoShape previewShape;
	};
	Feature::Feature(const std::string& p_name,  const std::string& tag) :TopoActor( p_name, tag, true, false),mInternal(new Internal(this))
	{

	}
	Feature::~Feature()
	{
		delete mInternal;
	}
	bool Feature::execute()
	{
		return false;
	}
	Part::TopoShape& Feature::getBaseTopoShape()
	{
		return m_baseFeature->GetTopoShape();
	}
	Part::TopoShape Feature::getVerifyTopoFace()
	{
		SketcherFeature* sketch = dynamic_cast<SketcherFeature*>(m_baseFeature);
		if (sketch) {
		   return  sketch->getSketcherObj()->getDoneFaceShape();
		}
		std::string idString = subValues[0].substr(5);
		auto comp = m_baseFeature->GetComponent<Core::ECS::Components::CTopoShape>();
		return comp->GetTopoFace(std::stoi(idString));
	}
	Part::TopoShape& Feature::getPreviewShape()
	{
		return mInternal->previewShape;
	}
	void Feature::makeDone()
	{
		if (!hasInTree) {
			GetViewerWidget.addActorToTreeView(this);
			hasInTree = true;
		}
		auto comp =GetComponent<Core::ECS::Components::CTopoShape>();
		comp->discretizationShape();
	}
}