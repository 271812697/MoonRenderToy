#include "editor/UI/TaskPanel/ShapeHelper.h"
#include "TopoShape.h"
#include "core/component/TopoShapeActor.h"
#include "core/component/CTopoShape.h"
#include "Core/Global/ServiceLocator.h"
#include "core/ViewTool.h"
#include "core/log.h"
#include "renderer/SceneView.h"
#include <Core/ECS/Components/CMaterialRenderer.h>
#include <tracy/Tracy.hpp>
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
		// 预览用的Actor
		Part::TopoShape m_previewShape;
		Part::TopoShape m_generateShape;
		Core::ECS::TopoActor* m_previewActor = nullptr;
		std::string name="GenerateShape";
	};
	ShapeHelper::ShapeHelper():mInternal(new Internal(this))
	{
	}
	ShapeHelper::~ShapeHelper()
	{
		delete mInternal;
	}
	bool ShapeHelper::generateShape()
	{
		return false;
	}
	void ShapeHelper::previewShape()
	{
		ZoneScoped;
		if (generateShape()) {

			Part::TopoShape shape(mInternal->m_previewShape);
			try
			{
				mInternal->m_previewShape = shape.makeElementRefine();
			}
			catch (Standard_Failure& err)
			{
				CORE_ERROR("Refine generateShape failed:{}",err.GetMessageString());
			}


			if (mInternal->m_previewActor == nullptr) {
				auto& view = GetService(Editor::Panels::SceneView);
				auto scene = view.GetScene();
				auto preActor = scene->FindActorByName("TopoShapePreview");
				if (preActor) {
					scene->RemoveActor(preActor);
				}
				mInternal->m_previewActor = new Core::ECS::TopoActor(scene, "TopoShapePreview", "TopoShape", true);
			}
			const auto& topoComp = mInternal->m_previewActor->GetComponent<Core::ECS::Components::CTopoShape>();
			Part::TopoShape& topo = topoComp->GetTopoShape();
			topo.setShape(mInternal->m_previewShape);
			topoComp->discretizationShape();
			auto MatRender = mInternal->m_previewActor->GetChild("Face")->GetComponent<Core::ECS::Components::CMaterialRenderer>();
			Core::Resources::Material* tempMat = MatRender->GetMaterialAtIndex(0);
			/*
					struct PreviewOption {
			bool isTransparent = true;
			float r=1.0f, g=1.0f, b=1.0f, a = 0.4f;
			bool isBlend = true;
			bool domainColor = true;
		};
			*/
			tempMat->SetProperty("u_Albedo", Maths::FVector4(mPreviewOption.r, mPreviewOption.g, mPreviewOption.b, mPreviewOption.a));
			if (mPreviewOption.isTransparent) {
				tempMat->SetTransparent(true);
				tempMat->SetDepthWriting(true);
			}
			else {
				if (mPreviewOption.isBlend) {
					tempMat->SetBlendable(true);
					tempMat->SetDepthTest(false);
					tempMat->SetDepthWriting(false);
					tempMat->SetDrawOrder(10000);
				}
			}
			if (!mPreviewOption.useDomainColor) {
				tempMat->AddFeature("DISABLE_DOMAIN_COLOR");
			}
			tempMat->SetBackfaceCulling(false);
			tempMat->SetFrontfaceCulling(false);
			//tempMat->SetDepthWriting(true);

			/*
			tempMat->SetBlendable(true);
			tempMat->SetDepthTest(false);
			tempMat->SetDepthWriting(false);
			tempMat->SetDrawOrder(10000);
			*/
		}
	}
	void ShapeHelper::generateFinalShape()
	{
		if (mInternal->m_generateShape.isNull()) {
			generateShape();
		}
		auto& view = GetService(Editor::Panels::SceneView);
		auto scene = view.GetScene();
		if (!mInternal->m_generateShape.isNull()) {
			Part::TopoShape shape(mInternal->m_generateShape);
			try
			{
				mInternal->m_generateShape = shape.makeElementRefine();
			}
			catch (Standard_Failure& err)
			{
				CORE_ERROR("Refine generateShape failed:{}", err.GetMessageString());
			}
			for (auto& ac:scene->FindActorsByTag("TopoShape")) {
				ac.get().SetActive(false);
			}
			auto topoActor = new Core::ECS::TopoActor(scene, mInternal->name, "TopoShape", false);
			const auto& topoComp = topoActor->GetComponent<Core::ECS::Components::CTopoShape>();
			Part::TopoShape& topo = topoComp->GetTopoShape();
			topo.setShape(mInternal->m_generateShape);
			topoComp->discretizationShape();
		}
		auto preActor = scene->FindActorByName("TopoShapePreview");
		if (preActor) {
			scene->RemoveActor(preActor);
			delete preActor;
		}
	}
	void ShapeHelper::clearPreviewShape()
	{
		auto& view = GetService(Editor::Panels::SceneView);
		auto scene = view.GetScene();
		auto preActor = scene->FindActorByName("TopoShapePreview");
		if (preActor) {
			scene->RemoveActor(preActor);
			delete preActor;
		}
	}
	Part::TopoShape& ShapeHelper::getPreviewShape()
	{
		return mInternal->m_previewShape;
	}
	Part::TopoShape& ShapeHelper::getGenerateShape()
	{
		return mInternal->m_generateShape;
	}
	void ShapeHelper::setGenerateShapeName(const char* name)
	{
		mInternal->name = name;
	}
}