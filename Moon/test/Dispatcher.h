#pragma once
#include <stdint.h>

namespace MOON {

	class Driver;
	class CommandBase;

	class Dispatcher {
	public:
		using Execute = void (*)(Driver& driver, CommandBase* self, intptr_t* next);

	};

}