#include "core/component/TopoShapeActor.h"

#include "editor/View/sceneview/viewerwidget.h"
#include "core/component/CTopoShape.h"
#include <Core/Global/ServiceLocator.h>
#include "Feature.h"
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
	Feature::Feature(const std::string& p_name) :TopoActor( p_name, "Feature", true, false),mInternal(new Internal(this))
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
		std::string idString = subValues[0].substr(5);
		auto comp = m_baseFeature->GetComponent<Core::ECS::Components::CTopoShape>();
		return comp->GetTopoFace(std::stoi(idString));
	}
	Part::TopoShape& Feature::getPreviewShape()
	{
		return mInternal->previewShape;
	}
	void Feature::addToTreeView()
	{
		GetViewerWidget.addActorToTreeView(this);
		auto comp =GetComponent<Core::ECS::Components::CTopoShape>();
		comp->discretizationShape();
	}
}