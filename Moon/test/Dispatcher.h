#pragma once
#include <stdint.h>

namespace TEST {

	class Driver;
	class CommandBase;

	class Dispatcher {
	public:
		using Execute = void (*)(Driver& driver, CommandBase* self, intptr_t* next);
		static void test(Driver& driver, CommandBase* base, intptr_t* next);

	};

}