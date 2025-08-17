

#include <array>

#include <OvTools/Utils/PathParser.h>



#include <OvCore/Global/ServiceLocator.h>
#include <OvCore/ResourceManagement/ModelManager.h>
#include <OvCore/ResourceManagement/TextureManager.h>
#include <OvCore/ResourceManagement/ShaderManager.h>
#include <OvCore/ResourceManagement/MaterialManager.h>

#include "OvCore/Helpers/GUIDrawer.h"


const float OvCore::Helpers::GUIDrawer::_MIN_FLOAT = -999999999.f;
const float OvCore::Helpers::GUIDrawer::_MAX_FLOAT = +999999999.f;
OvRendering::Resources::Texture* OvCore::Helpers::GUIDrawer::__EMPTY_TEXTURE = nullptr;

void OvCore::Helpers::GUIDrawer::ProvideEmptyTexture(OvRendering::Resources::Texture& p_emptyTexture)
{
	__EMPTY_TEXTURE = &p_emptyTexture;
}







