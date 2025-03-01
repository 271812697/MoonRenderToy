#include "DriverBase.h"

#include "Driver.h"
#include "CommandStream.h"



#include <functional>
#include <mutex>
#include <utility>

#include <stddef.h>
#include <stdint.h>
#include <iostream>

#define UTILS_HAS_THREADING 0
namespace MOON {

	DriverBase::DriverBase() noexcept {

	}

	DriverBase::~DriverBase() noexcept {

	}




	Driver::Driver()
	{
	}

	Driver::Driver(const Driver&&)
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
		std::cout << val << std::endl;
	}

} // namespace filament::backend
