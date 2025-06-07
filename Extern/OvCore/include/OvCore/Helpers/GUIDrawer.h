/**
* @project: Overload
* @author: Overload Tech.
* @licence: MIT
*/

#pragma once

#include <OvMaths/FVector2.h>
#include <OvMaths/FVector3.h>
#include <OvMaths/FVector4.h>
#include <OvMaths/FQuaternion.h>



namespace OvCore::Resources
{
	class Material;
}

namespace OvAudio::Resources
{
	class Sound;
}

namespace OvRendering::Resources
{
	class Model;
	class Shader;
	class Texture;
}

namespace OvCore::Helpers
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
		static void ProvideEmptyTexture(OvRendering::Resources::Texture& p_emptyTexture);


		template <typename T>
		static std::string GetFormat();

	private:
		static OvRendering::Resources::Texture* __EMPTY_TEXTURE;
	};
}

#include "OvCore/Helpers/GUIDrawer.inl"