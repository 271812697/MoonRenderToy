#include "Interactive/Widgets/ClipPlane.h"
#include "Interactive/Im3DRenderer.h"
#include "Interactive/MathUtil/MathUtil.h"
#include "renderer/SceneView.h"
#include <Core/Rendering/EngineBufferRenderFeature.h>
#include "Interactive/Interactive/ExecuteCommand.h"
#include "Interactive/Interactive/RenderWindowInteractor.h"
#include "core/component/CTopoShape.h"
#include "TopoShape.h"
#include "core/component/TopoShapeActor.h"
#include "Interactive/GizmoBehaviour.h"
#include "Qtimgui/imgui/imgui.h"
#include "core/ViewTool.h"

#include <Core/ECS/Components/CMaterialRenderer.h>
#include "Core/Global/ServiceLocator.h"
#include <tracy/Tracy.hpp>
namespace MOON {

	struct PickMeshInfo {
		std::string blockName;
		ClipPlane::PickMeshId meshId;
		Maths::FVector4 blockColor;
	};
	static PickMeshInfo table[9] = {
		{"YArrow",ClipPlane::PickMeshId::YAxis,{ 0,1,0,1 }},
		{"XArrow",ClipPlane::PickMeshId::XAxis,{ 1,0,0,1 }},
		{"ZArrow",ClipPlane::PickMeshId::ZAxis,{ 0,0,1,1 }},
		{"XPlane",ClipPlane::PickMeshId::XNormalPlane,{ 1,0,0,1 }},
		{"YPlane",ClipPlane::PickMeshId::YNormalPlane,{ 0,1,0,1 }},
		{"ZPlane",ClipPlane::PickMeshId::ZNormalPlane,{ 0,0,1,1 }},
		{"XAxis",ClipPlane::PickMeshId::XNormalRotate ,{ 1,0,0,1 }},
		{"YAxis",ClipPlane::PickMeshId::YNormalRotate,{ 0,1,0,1 }},
		{"ZAxis",ClipPlane::PickMeshId::ZNormalRotate,{ 1,0,1,1 }}
	};
	static int meshInfoCnt = 9;
	Maths::FVector4 hotColor = { 0.1,1.0,1.0,1 };
	Maths::FVector4 activeColor = { 1,1,0,1 };

	class ClipPlane::ClipPlaneInternal {
	public:
		ClipPlaneInternal(ClipPlane* clip) :mSelf(clip) {
			clickObserver = mSelf->Interactor->AddObserver(ExecuteCommand::LeftButtonReleaseEvent, this, &ClipPlane::ClipPlaneInternal::onMouseLeftClick, 0.0f);

		}
		~ClipPlaneInternal() {
			delete clickObserver.command;
			//delete moveObserver.command;
		}
		void setUp() {
			sectionActor = new TopoActor("Section", "Geomerty", true);
			auto faceMat = sectionActor->GetChild("Face")->GetComponent<::Core::ECS::Components::CMaterialRenderer>()->GetMaterialAtIndex(0);
			faceMat->SetShader(Core::Global::ServiceLocator::Get<Editor::Core::Context>().shaderManager[":Shaders\\SectionFace.ovfx"]);
			auto edgeMat = sectionActor->GetChild("Edge")->GetComponent<::Core::ECS::Components::CMaterialRenderer>()->GetMaterialAtIndex(0);
			auto& renderer = GetSceneView.GetRenderer();;
			{
				faceMat->SetBackfaceCulling(false);
				faceMat->SetCastShadows(false);
				faceMat->SetReceiveShadows(false);
				faceMat->SetProperty("u_Albedo", Maths::FVector4(1, 1, 1, 1));
				faceMat->SetProperty("u_AlphaClippingThreshold", 0.0f);
				faceMat->SetProperty("u_Roughness", 0.25f);
				faceMat->SetProperty("u_Metallic", 0.75f);
				// Emission
				faceMat->SetProperty("u_EmissiveIntensity", 1.0f);
				faceMat->SetProperty("u_EmissiveColor", Maths::FVector3{ 0.0f, 0.0f, 0.0f });

				faceMat->TrySetProperty("_IrradianceCube", renderer.GetIrradianceCube());
				faceMat->TrySetProperty("_PrefilterCube", renderer.GetPrefilterCube());
				faceMat->TrySetProperty("_BRDFLut", renderer.GetBrdfTexture());
			}
			{
				edgeMat->SetLineWidth(4.0);
				edgeMat->SetProperty("color",Maths::FVector3(1,0,1));
				edgeMat->RemoveFeature("CLIP_PLANE");
			}
		}
		void onMouseLeftClick() {
			if (mSelf->m_sceneView->IsSelectActor()) {
				auto acptr = mSelf->m_sceneView->GetSelectedActor();
				while (!acptr->HasComponent("Model Renderer") && acptr->HasParent()) {
					acptr = acptr->GetParent();
				}

				if (acptr && acptr != ac) {
					ac = acptr;
					auto modelRenderer = ac->GetComponent<::Core::ECS::Components::CModelRenderer>();
					if (modelRenderer) {
						auto model = modelRenderer->GetModel();
						if (model) {
							auto& box = model->GetBoundingBox();
							auto transform = ac->GetComponent<::Core::ECS::Components::CTransform>();
							auto bbox = box.transform(transform->GetWorldMatrix());
							setupBox({ bbox.pmin.x,bbox.pmin.y,bbox.pmin.z }, { bbox.pmax.x,bbox.pmax.y,bbox.pmax.z });
						}
					}
				}
			}
		}
		void setupBox(const Eigen::Vector3f& min, const Eigen::Vector3f& max) {
			boxMin = min;
			boxMax = max;
			center = (min + max) * 0.5f;
			extent = (boxMax - boxMin).norm();
			cirleDetectRadius = extent / 2;
		}
	private:
		TopoActor* sectionActor = nullptr;
		friend class ClipPlane;
		ClipPlane* mSelf = nullptr;
		Core::ECS::Actor* ac = nullptr;
		Eigen::Vector3f center = { 0,0,0 };
		Eigen::Vector3f normal = { 0,1,0 };
		Eigen::Vector3f xAxis = { 1,0,0 };
		Eigen::Vector3f yAxis = { 0,1,0 };
		Eigen::Vector3f zAxis = { 0,0,1 };
		Eigen::Vector3f boxMin = { 0,0,0 };
		Eigen::Vector3f boxMax = { 0,0,0 };
		float cirleDetectRadius = 5.0f;
		float extent = 1.0f;
		ExecuteCommandPair clickObserver;
		ExecuteCommandPair moveObserver;
		std::vector<Eigen::Vector3f>slicelines;
		std::vector<Eigen::Vector3f>sectionFace;

		bool updateEngineUbo = false;
		GizmoAxisTranslate transLatePick;
		GizmoPlaneTranslate planeTPick;
		GizmoAxisRotate axisRPick;
	};

	ClipPlane::ClipPlane(const std::string& name) :EventWidget(name)
		, m_internal(new ClipPlaneInternal(this)), mState(Stop) {

	}
	ClipPlane::~ClipPlane()
	{
		delete m_internal;
	}
	void ClipPlane::onUpdate()
	{
		if (m_internal->sectionActor == nullptr) {
			m_internal->setUp();
		}
		if (mState == AxisR)
		{
			float angle = m_internal->axisRPick.getRotationAngle();
			float degree = -angle * 180 / 3.14159265358979323846f;
			std::string text = std::to_string(degree) + " degree\n";
			auto pos = renderer->getFrameParam().cursor;
			ImGui::GetForegroundDrawList()->AddText({ pos.x(),pos.y() - 20 }, IM_COL32(255, 255, 0, 255), text.c_str());
			auto seg = m_internal->axisRPick.getRotationArc();
			if (seg.size() > 0) {
				renderer->pushColor({ 255,255,255,0 });

				for (int i = 0; i < seg.size() - 1; i++) {
					renderer->drawLine(seg[i], seg[i + 1], 5);
				}
				renderer->popColor();
				renderer->pushColor({ 255,0,255,255 });
				renderer->drawLine(seg[0], m_internal->center, 4);
				renderer->drawLine(seg.back(), m_internal->center, 4);
				renderer->popColor();
			}
		}

		renderer->drawOneMesh(
			m_internal->center,
			RotationMatrix(m_internal->xAxis, m_internal->yAxis, m_internal->zAxis),
			Eigen::Vector3f{ 0.1f,0.1f,0.1f },
			"TransformAxis");

		auto& planeOrigin = m_internal->center;
		Eigen::Vector3f boxMin = Eigen::Vector3f(std::min(m_internal->boxMin.x(), planeOrigin.x()), std::min(m_internal->boxMin.y(), planeOrigin.y()), std::min(m_internal->boxMin.z(), planeOrigin.z()));
		Eigen::Vector3f boxMax = Eigen::Vector3f(std::max(m_internal->boxMax.x(), planeOrigin.x()), std::max(m_internal->boxMax.y(), planeOrigin.y()), std::max(m_internal->boxMax.z(), planeOrigin.z()));

		renderer->pushSize(3.0);;
		renderer->drawAlignedBox(boxMin, boxMax);
		renderer->popSize();


		if (m_internal->updateEngineUbo)
		{
			m_internal->updateEngineUbo = false;
			auto& feature = m_sceneView->GetRenderer().GetFeature<::Core::Rendering::EngineBufferRenderFeature>();
			feature.SetClipPlane(
				m_internal->zAxis.x(),
				m_internal->zAxis.y(),
				m_internal->zAxis.z(),
				-m_internal->zAxis.dot(m_internal->center)
			);
			
		
			
			Eigen::Vector3f lineNormal = (m_internal->xAxis - m_internal->yAxis).normalized();
			auto faceMat = m_internal->sectionActor->GetChild("Face")->GetComponent<::Core::ECS::Components::CMaterialRenderer>()->GetMaterialAtIndex(0);
			faceMat->SetProperty("lineNormal",Maths::FVector3(lineNormal.x(), lineNormal.y(), lineNormal.z()));
			faceMat->SetProperty("planeOrigin",Maths::FVector3(m_internal->center.x(), m_internal->center.y(), m_internal->center.z()));

		}
	}

	Maths::FVector4 ClipPlane::getClipPlane()
	{
		return Maths::FVector4(
			m_internal->zAxis.x(),
			m_internal->zAxis.y(),
			m_internal->zAxis.z(),
			m_internal->zAxis.dot(m_internal->center)
		);
	}

	void ClipPlane::onLeftMousePressed()
	{
		if (mState == Hot) {

			bool active = false;
			if (table[mPickMesh].meshId == PickMeshId::YAxis || table[mPickMesh].meshId == PickMeshId::XAxis || table[mPickMesh].meshId == PickMeshId::ZAxis) {
				mState = AxisT;
				Eigen::Vector3f axis = table[mPickMesh].meshId == PickMeshId::YAxis ?
					m_internal->yAxis : (table[mPickMesh].meshId == PickMeshId::XAxis ? m_internal->xAxis : m_internal->zAxis);
				m_internal->transLatePick.startPick(axis, m_internal->center);
				active = true;
			}
			else if (table[mPickMesh].meshId == PickMeshId::XNormalPlane || table[mPickMesh].meshId == PickMeshId::YNormalPlane || table[mPickMesh].meshId == PickMeshId::ZNormalPlane) {
				mState = PlaneT;
				Eigen::Vector3f normal = table[mPickMesh].meshId == PickMeshId::XNormalPlane ?
					m_internal->xAxis : (table[mPickMesh].meshId == PickMeshId::YNormalPlane ? m_internal->yAxis : m_internal->zAxis);
				float w = -m_internal->center.dot(normal);
				Eigen::Vector4f planeEqu = { normal.x(),normal.y(),normal.z(),w };
				m_internal->planeTPick.startPick(planeEqu, m_internal->center);
				active = true;
			}
			else if (table[mPickMesh].meshId == PickMeshId::XNormalRotate || table[mPickMesh].meshId == PickMeshId::YNormalRotate || mPickMesh == PickMeshId::ZNormalRotate) {
				mState = AxisR;
				Eigen::Vector3f normal = table[mPickMesh].meshId == PickMeshId::XNormalRotate ?
					m_internal->yAxis : (table[mPickMesh].meshId == PickMeshId::YNormalRotate ? m_internal->zAxis : m_internal->xAxis);
				Eigen::Vector3f refDir = table[mPickMesh].meshId == PickMeshId::XNormalRotate ?
					m_internal->xAxis : (table[mPickMesh].meshId == PickMeshId::YNormalRotate ? m_internal->yAxis : m_internal->zAxis);

				m_internal->axisRPick.startPick(normal, m_internal->center, refDir, m_internal->center + refDir);
				active = true;
			}
			if (active) {
				TransformAxis().setBlockColor(TransformAxis().getBlockId(table[mPickMesh].blockName), activeColor);
			}
		}
	}

	void ClipPlane::onLeftMouseReleased()
	{
		if (mState == AxisT || mState == PlaneT || mState == AxisR) {
			mState = Hot;
			TransformAxis().setBlockColor(TransformAxis().getBlockId(table[mPickMesh].blockName), hotColor);
			PickMeshId id = table[mPickMesh].meshId;
			bool updateFlag = (id != PickMeshId::XAxis) && (id != PickMeshId::YAxis) &&
				(id != PickMeshId::ZNormalPlane) && (id != PickMeshId::YNormalRotate);
			if (updateFlag) {
				updateSection();
			}
		}
	}

	void ClipPlane::onMouseMove()
	{
		if (mState == Stop) {
			bool selectFlag = false;
			mPickMesh = PickMeshId::None;
			for (int i = 0; i < meshInfoCnt; i++) {
				if (renderer->isSelectPolygon("TransformAxis", table[i].blockName)) {
					mPickMesh = i;
					selectFlag = true;
					break;
				}
			}
			if (selectFlag) {
				mState = Hot;
				TransformAxis().setBlockColor(TransformAxis().getBlockId(table[mPickMesh].blockName), hotColor);
			}
		}
		else if (mState == Hot) {
			bool selectFlag = false;
			for (int i = 0; i < meshInfoCnt; i++) {
				if (renderer->isSelectPolygon("TransformAxis", table[i].blockName)) {
					selectFlag = true;
					break;
				}
			}
			if (!selectFlag) {
				mState = Stop;
				TransformAxis().setBlockColor(TransformAxis().getBlockId(table[mPickMesh].blockName), table[mPickMesh].blockColor);
				mPickMesh = PickMeshId::None;
			}
		}
		else if (mState == AxisT) {
			auto& param = renderer->getFrameParam();
			m_internal->transLatePick.apply(param.rayDirection, param.rayOrigin, m_internal->center);
		}
		else if (mState == PlaneT) {
			auto& param = renderer->getFrameParam();
			m_internal->planeTPick.apply(param.rayDirection, param.rayOrigin, m_internal->center);
		}
		else if (mState == AxisR) {
			auto& param = renderer->getFrameParam();
			Eigen::Vector3f dir = table[mPickMesh].meshId == PickMeshId::XNormalRotate ?
				m_internal->yAxis : (table[mPickMesh].meshId == PickMeshId::YNormalRotate ? m_internal->zAxis : m_internal->xAxis);
			m_internal->axisRPick.applyDir(param.rayDirection, param.rayOrigin, dir);
			if (table[mPickMesh].meshId == PickMeshId::XNormalRotate) {
				m_internal->xAxis = dir;
				m_internal->zAxis = m_internal->xAxis.cross(m_internal->yAxis);
			}
			else if (table[mPickMesh].meshId == PickMeshId::YNormalRotate) {
				m_internal->yAxis = dir;
				m_internal->xAxis = m_internal->yAxis.cross(m_internal->zAxis);
			}
			else if (table[mPickMesh].meshId == PickMeshId::ZNormalRotate) {
				m_internal->zAxis = dir;
				m_internal->yAxis = m_internal->zAxis.cross(m_internal->xAxis);
			}
		}
		//if (mState == AxisT || mState == PlaneT || mState == AxisR) {
		//
		//	PickMeshId id = table[mPickMesh].meshId;
		//	bool updateFlag = (id != PickMeshId::XAxis) && (id != PickMeshId::YAxis) &&
		//		(id != PickMeshId::ZNormalPlane) && (id != PickMeshId::YNormalRotate);
		//	if (updateFlag) {
		//		updateSection();
		//	}
		//}
	}

	void ClipPlane::updateSection()
	{
		Core::ECS::Actor* selectActor = m_internal->ac;
		if (selectActor) {
			m_internal->updateEngineUbo = true;
		
			while (!selectActor->HasComponent("CTopoShape") && selectActor->HasParent())
			{
				selectActor = selectActor->GetParent();
			}
			if (selectActor)
			{
				Core::ECS::Components::CTopoShape* topoComp = selectActor->GetComponent<Core::ECS::Components::CTopoShape>();
				if (topoComp) {
					auto& topoShape = topoComp->GetTopoShape();
					double offset = m_internal->zAxis.dot(m_internal->center);
					Base::Vector3d dir{ m_internal->zAxis.x(), m_internal->zAxis.y() , m_internal->zAxis.z() };
					{
						ZoneScopedN("clip");
						auto wires = topoShape.slices(dir, std::vector<double>{ offset });
						Part::TopoShape shape(wires);
						if (!shape.isNull()) {
							try
							{
								ZoneScopedN("makeFace");
								shape = shape.makeElementFace("Part::FaceMakerCheese");
								m_internal->sectionActor->setTopoShape(shape);
							}
							catch (const Base::Exception& e)
							{
							}
						}
					}
				}
			}
		}
	}
}