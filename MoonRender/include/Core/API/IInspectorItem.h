#pragma once
#include "Core/API/ISerializable.h"
#include "Core/Helpers/GUIDrawer.h"

namespace Core::API
{
	/**
	* IInspectorItem is an interface to any class that is serializable and drawable on inspector
	*/
	class IInspectorItem : public ISerializable
	{
	public:
		/**
		* Called when the inspector needs to create the UI elements for the given IInspectorItem
		* @param p_root
		*/

	};
}