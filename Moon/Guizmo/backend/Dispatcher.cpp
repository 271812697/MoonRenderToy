#include "Driver.h"
#include "Dispatcher.h"
#include "CommandStream.h"
namespace MOON
{

	void Dispatcher::test(Driver& driver, CommandBase* base, intptr_t* next)
	{
		using Cmd = CommandType<decltype(&Driver::test)>::Command<&Driver::test>;
		Cmd::execute(&Driver::test, driver, base, next);
	}
}

