/**
* @project: erload
* @author: erload Tech.
* @licence: MIT
*/

#pragma once

#include <Maths/FTransform.h> // TODO: Unused right now, might want to use it instead of model matrix
#include <Tools/Utils/OptRef.h>

#include "Rendering/Resources/Mesh.h"
#include "Rendering/Data/Material.h"
#include "Rendering/Data/Describable.h"

namespace Rendering::Entities
{
	/**
	* Drawable entity
	*/
	struct Drawable : public Data::Describable
	{
		Tools::Utils::OptRef<Rendering::Resources::IMesh> mesh;
		Tools::Utils::OptRef<Rendering::Data::Material> material;
		Rendering::Data::StateMask stateMask;
		Rendering::Settings::EPrimitiveMode primitiveMode = Rendering::Settings::EPrimitiveMode::TRIANGLES;
	};
}