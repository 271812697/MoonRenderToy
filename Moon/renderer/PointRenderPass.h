#pragma once
#include <OvCore/ECS/Actor.h>

#include <OvCore/Resources/Material.h>
#include <OvCore/Rendering/SceneRenderer.h>
#include <OvCore/SceneSystem/SceneManager.h>
#include <OvRendering/HAL/VertexBuffer.h>
#include "Context.h"



namespace OvEditor::Rendering
{

	class PointRenderPass : public OvRendering::Core::ARenderPass
	{
	public:


		/**
		* Constructor
		* @param p_renderer
		*/
		PointRenderPass(OvRendering::Core::CompositeRenderer& p_renderer);


	private:
		virtual void Draw(OvRendering::Data::PipelineState p_pso) override;
	private:
	    OvRendering::HAL::VertexBuffer vbo;
		OvCore::Resources::Material m_PointMaterial;
	};
}
