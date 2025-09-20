#include "Driver.h"
namespace MOON {
	Driver::Driver()
	{
	}
	Driver::~Driver()
	{
	}
	void Driver::execute(std::function<void(void)> const& fn) {
		fn();
	}
	void Driver::test(int val)
	{
	}
}

