/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Rendering/Features/ARenderFeature.h>

namespace Editor::Rendering
{
	/**
	* Provide utility methods to draw a model quickly using a single material for all its submeshes
	*/
	class DebugModelRenderFeature : public ::Rendering::Features::ARenderFeature
	{
	public:
		/**
		* Constructor
		* @param p_renderer
		*/
		DebugModelRenderFeature(::Rendering::Core::CompositeRenderer& p_renderer);

		/**
		* Utility function to draw a whole model with a single material,
		* instead of drawing sub-meshes with their individual materials
		* @param p_pso
		* @param p_model
		* @param p_material
		* @param p_modelMatrix
		*/
		virtual void DrawModelWithSingleMaterial(
			::Rendering::Data::PipelineState p_pso,
			::Rendering::Resources::Model& p_model,
			::Rendering::Data::Material& p_material,
			const Maths::FMatrix4& p_modelMatrix
		);
	};
}
