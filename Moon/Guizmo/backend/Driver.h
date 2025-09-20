#pragma once
#include <functional>
#include <stddef.h>
#include <stdint.h>
namespace MOON {
	class Dispatcher;
	class CommandStream;
	class Driver {
	public:
		Driver();
		virtual ~Driver();
		virtual void execute(std::function<void(void)> const& fn);
	public:
		void test(int val);
	private:
	};
}





