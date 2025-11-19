#include <array>

#include <Tools/Utils/PathParser.h>



#include <Core/Global/ServiceLocator.h>
#include <Core/ResourceManagement/ModelManager.h>
#include <Core/ResourceManagement/TextureManager.h>
#include <Core/ResourceManagement/ShaderManager.h>
#include <Core/ResourceManagement/MaterialManager.h>

#include "Core/Helpers/GUIDrawer.h"


const float Core::Helpers::GUIDrawer::_MIN_FLOAT = -999999999.f;
const float Core::Helpers::GUIDrawer::_MAX_FLOAT = +999999999.f;
Rendering::Resources::Texture* Core::Helpers::GUIDrawer::__EMPTY_TEXTURE = nullptr;

void Core::Helpers::GUIDrawer::ProvideEmptyTexture(Rendering::Resources::Texture& p_emptyTexture)
{
	__EMPTY_TEXTURE = &p_emptyTexture;
}







