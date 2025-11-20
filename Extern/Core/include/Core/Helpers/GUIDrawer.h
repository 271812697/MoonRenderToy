#pragma once
#include <Maths/FVector2.h>
#include <Maths/FVector3.h>
#include <Maths/FVector4.h>
#include <Maths/FQuaternion.h>

namespace Core::Resources
{
	class Material;
}

namespace Audio::Resources
{
	class Sound;
}

namespace Rendering::Resources
{
	class Model;
	class Shader;
	class Texture;
}

namespace Core::Helpers
{
	/**
	* Provide some helpers to draw UI elements
	*/
	class GUIDrawer
	{
	public:


		static const float _MIN_FLOAT;
		static const float _MAX_FLOAT;

		/**
		* Defines the texture to use when there is no texture in a texture resource field
		* @param p_emptyTexture
		*/
		static void ProvideEmptyTexture(::Rendering::Resources::Texture& p_emptyTexture);


		template <typename T>
		static std::string GetFormat();

	private:
		static ::Rendering::Resources::Texture* __EMPTY_TEXTURE;
	};
}

#include "Core/Helpers/GUIDrawer.inl"