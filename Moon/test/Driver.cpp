

#include "DriverBase.h"

#include "Driver.h"
#include "CommandStream.h"



#include <functional>
#include <mutex>
#include <utility>

#include <stddef.h>
#include <stdint.h>


#define UTILS_HAS_THREADING 0
namespace MOON {

	DriverBase::DriverBase() noexcept {

	}

	DriverBase::~DriverBase() noexcept {

	}

	Driver::~Driver() noexcept = default;

	void Driver::execute(std::function<void(void)> const& fn) {
		fn();
	}

} // namespace filament::backend
