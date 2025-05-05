#include "Core/Helpers/GUIDrawer.h"
#include "Rendering/Resources/Texture.h"
::Rendering::Resources::Texture* ::Core::Helpers::GUIDrawer::__EMPTY_TEXTURE = nullptr;

void ::Core::Helpers::GUIDrawer::ProvideEmptyTexture(::Rendering::Resources::Texture& p_emptyTexture)
{
	__EMPTY_TEXTURE = &p_emptyTexture;
}