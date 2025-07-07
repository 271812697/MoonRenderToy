#pragma once
#include <stdint.h>
namespace UI::Internal
{

	union TextureID
	{
		uint32_t id;
		void* raw;
	};
}