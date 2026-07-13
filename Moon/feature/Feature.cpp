#include "core/component/TopoShapeActor.h"
#include "renderer/SceneView.h"
#include <Core/ResourceManagement/MaterialManager.h>
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include "Core/ECS/Components/CBatchMeshTriangle.h"
#include "Core/ECS/Components/CBatchMeshLine.h"
#include "Core/ResourceManagement/ModelManager.h"
#include "editor/View/sceneview/viewerwidget.h"
#include "core/component/CTopoShape.h"
#include <Core/Global/ServiceLocator.h>
#include <Core/SceneSystem/Scene.h>
#include "Feature.h"
#include "TopoShape.h"

namespace MOON {
	Feature::Feature(const std::string& p_name) :TopoActor( p_name, "Feature", true, true)
	{

	}
	Feature::~Feature()
	{
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
}