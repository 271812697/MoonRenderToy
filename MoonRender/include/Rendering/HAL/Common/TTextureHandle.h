#pragma once
#include <optional>

#include <Rendering/Settings/EGraphicsBackend.h>
#include <Rendering/Settings/ETextureType.h>

namespace Rendering::HAL
{
	/**
	* Represents a texture handle, acts as a view to the texture.
	*/
	template<Settings::EGraphicsBackend Backend, class Context>
	class TTextureHandle
	{
	public:
		/**
		* Binds the texture to the given slot.
		* @param p_slot Optional slot to bind the texture to.
		*/
		void Bind(std::optional<uint32_t> p_slot = std::nullopt) const;

		/**
		* Unbinds the texture.
		*/
		void Unbind() const;

		/**
		* Returns the ID associated with the texture.
		* @return The texture ID.
		*/
		uint32_t GetID() const;

		/**
		* Returns the texture type
		*/
		Settings::ETextureType GetType() const;

	protected:
		TTextureHandle(Settings::ETextureType p_type);
		TTextureHandle(Settings::ETextureType p_type, uint32_t p_id);

	protected:
		Context m_context;
	};
}
