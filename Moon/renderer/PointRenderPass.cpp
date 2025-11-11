#include <ranges>
#include <OvCore/ECS/Components/CMaterialRenderer.h>
#include <OvCore/Rendering/EngineDrawableDescriptor.h>

#include "OvCore/Global/ServiceLocator.h"
#include "PointRenderPass.h"
#include "renderer/DebugSceneRenderer.h"
#include <glad/glad.h>
static OvRendering::Resources::Model* sphere = nullptr;

OvEditor::Rendering::PointRenderPass::PointRenderPass(OvRendering::Core::CompositeRenderer& p_renderer)
	:OvRendering::Core::ARenderPass(p_renderer)
{
	m_PointMaterial.SetShader(OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().shaderManager[":Shaders\\Point.ovfx"]);
	m_PointMaterial.SetDepthTest(true);
	sphere=OvCore::Global::ServiceLocator::Get<OvEditor::Core::Context>().modelManager[":Models/Sphere.fbx"];

}

void OvEditor::Rendering::PointRenderPass::Draw(OvRendering::Data::PipelineState p_pso)
{

	auto& debugSceneDescriptor = m_renderer.GetDescriptor<OvEditor::Rendering::DebugSceneRenderer::DebugSceneDescriptor>();
	//m_renderer.Clear(false, true, false);
	if (debugSceneDescriptor.selectedActor)
	{
		auto& selectedActor = debugSceneDescriptor.selectedActor.value();
		if (auto selectModel = selectedActor.GetComponent<OvCore::ECS::Components::CModelRenderer>(); selectModel) {
			auto engineDrawableDescriptor = OvCore::Rendering::EngineDrawableDescriptor{
			selectedActor.transform.GetWorldMatrix(),
	        OvMaths::FMatrix4::Identity
			};
			//OvMaths::FMatrix4::Scale(selectedActor.transform.GetWorldMatrix(),{0.1,0.1,0.1});
			//selectMode.
			
			auto selectMesh=selectModel->GetModel()->GetMeshes()[0];
			auto& sphereVao=sphere->GetMeshes()[0]->getVertexArray();
			auto& instanceVBO = selectMesh->getVertexBuffer();
			
			sphereVao.Bind();
			instanceVBO.Bind();
			// 设置实例化属性指针（3D偏移量）
			glEnableVertexAttribArray(5);
			glVertexAttribPointer(5, 3, GL_FLOAT, GL_FALSE, 14 * sizeof(float), (void*)0);
			glVertexAttribDivisor(5, 1);  // 每个实例更新一次
			sphereVao.Unbind();
			instanceVBO.Unbind();
			m_PointMaterial.SetGPUInstances(selectMesh->GetVertexCount());
			auto stateMask = m_PointMaterial.GenerateStateMask();
			OvRendering::Entities::Drawable element;
			element.mesh = *sphere->GetMeshes()[0];
			element.material = m_PointMaterial;
			element.stateMask = stateMask;
			element.AddDescriptor(engineDrawableDescriptor);
			m_renderer.DrawEntity(p_pso, element);
		}
	}
}
