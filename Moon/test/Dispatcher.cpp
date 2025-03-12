#pragma once
#include "Driver.h"
#include "Dispatcher.h"
#include "CommandStream.h"

namespace TEST {
	void Dispatcher::test(Driver& driver, CommandBase* base, intptr_t* next)
	{
		using Cmd = CommandType<decltype(&Driver::test)>::Command<&Driver::test>;
		Cmd::execute(&Driver::test, driver, base, next);
	}
	void Dispatcher::createProgram(Driver& driver, CommandBase* base, intptr_t* next)
	{
		using Cmd = CommandType<decltype(&Driver::createProgramR)>::Command<&Driver::createProgramR>;
		Cmd::execute(&Driver::createProgramR, driver, base, next);
	}
}