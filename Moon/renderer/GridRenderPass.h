#pragma once
#include <Rendering/Entities/Camera.h>
#include <Rendering/Features/DebugShapeRenderFeature.h>

#include <Core/ECS/Actor.h>
#include <Core/SceneSystem/SceneManager.h>
#include <Core/ECS/Components/CModelRenderer.h>
#include <Core/Resources/Material.h>
#include <Core/ECS/Components/CAmbientBoxLight.h>
#include <Core/ECS/Components/CAmbientSphereLight.h>
#include <Core/Rendering/SceneRenderer.h>

#include "Context.h"

namespace Editor::Rendering
{
	/**
	* Draw a grid
	*/
	class GridRenderPass : public ::Rendering::Core::ARenderPass
	{
	public:
		struct GridDescriptor
		{
			Maths::FVector3 gridColor;
			Maths::FVector3 viewPosition;
		};

		/**
		* Constructor
		* @param p_renderer
		*/
		GridRenderPass(::Rendering::Core::CompositeRenderer& p_renderer);

	protected:
		virtual void Draw(::Rendering::Data::PipelineState p_pso) override;

	private:
		::Core::Resources::Material m_gridMaterial;
	};
}