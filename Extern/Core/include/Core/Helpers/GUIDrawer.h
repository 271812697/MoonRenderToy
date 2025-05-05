#pragma once
namespace Rendering::Resources
{
	class Texture;
}
namespace Core::Helpers
{
	class GUIDrawer
	{
	public:
		static void ProvideEmptyTexture(::Rendering::Resources::Texture& p_emptyTexture);
	private:
		static ::Rendering::Resources::Texture* __EMPTY_TEXTURE;
	};
}

