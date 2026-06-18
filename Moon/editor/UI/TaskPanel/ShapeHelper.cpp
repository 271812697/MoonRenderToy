#include "editor/UI/TaskPanel/ShapeHelper.h"
#include "TopoShape.h"
#include "core/component/TopoShapeActor.h"
#include "core/component/CTopoShape.h"
#include "Core/Global/ServiceLocator.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "renderer/SceneView.h"
#include <Core/ECS/Components/CMaterialRenderer.h>
namespace MOON {
	class ShapeHelper::Internal
	{
	public:
		Internal(ShapeHelper* s):self(s) {

		}
		~Internal() {

		}
	private:
		friend ShapeHelper;
		ShapeHelper* self = nullptr;
		Part::TopoShape m_previewShape;
		// 预览用的Actor
		Core::ECS::TopoActor* m_previewActor = nullptr;
	};
	ShapeHelper::ShapeHelper():mInternal(new Internal(this))
	{
	}
	ShapeHelper::~ShapeHelper()
	{
		delete mInternal;
	}
	bool ShapeHelper::generatePreviewShape()
	{
		return false;
	}
	void ShapeHelper::previewShape()
	{
		if (generatePreviewShape()) {

			Part::TopoShape shape(mInternal->m_previewShape);

			try
			{
				mInternal->m_previewShape = shape.makeElementRefine();
			}
			catch (Standard_Failure& err)
			{
				CORE_ERROR("Refine failed:{}",err.GetMessageString());
			}

			if (mInternal->m_previewActor == nullptr) {
				auto& view = GetService(Editor::Panels::SceneView);
				auto scene = view.GetScene();
				auto preActor = scene->FindActorByName("TopoShapePrismPreview");
				if (preActor) {
					scene->RemoveActor(preActor);
				}
				mInternal->m_previewActor = new Core::ECS::TopoActor(scene, "TopoShapePrismPreview", "TopoShape", true);
			}
			const auto& topoComp = mInternal->m_previewActor->GetComponent<Core::ECS::Components::CTopoShape>();
			Part::TopoShape& topo = topoComp->GetTopoShape();
			topo.setShape(mInternal->m_previewShape);
			topoComp->discretizationShape();
			auto MatRender = mInternal->m_previewActor->GetChild("Face")->GetComponent<Core::ECS::Components::CMaterialRenderer>();
			Core::Resources::Material* tempMat = MatRender->GetMaterialAtIndex(0);
			tempMat->SetProperty("u_Albedo", Maths::FVector4(1, 1, 1, 0.4));
			tempMat->SetBlendable(true);
			tempMat->SetDepthWriting(true);
		}
	}
	void ShapeHelper::generateFinalShape()
	{
		if (mInternal->m_previewShape.isNull()) {
			generatePreviewShape();
		}
		auto& view = GetService(Editor::Panels::SceneView);
		auto scene = view.GetScene();
		if (!mInternal->m_previewShape.isNull()) {
			auto topoActor = new Core::ECS::TopoActor(scene, "TopoShapePrism", "TopoShape", false);
			const auto& topoComp = topoActor->GetComponent<Core::ECS::Components::CTopoShape>();
			Part::TopoShape& topo = topoComp->GetTopoShape();
			topo.setShape(mInternal->m_previewShape);
			//topo.setShape(topo.MakeBottle(200,250,100));
			topoComp->discretizationShape();
		}
		auto preActor = scene->FindActorByName("TopoShapePrismPreview");
		if (preActor) {
			scene->RemoveActor(preActor);
			delete preActor;
		}
	}
	Part::TopoShape& ShapeHelper::getPreviewShape()
	{
		return mInternal->m_previewShape;
	}
}