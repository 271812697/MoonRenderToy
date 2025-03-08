

#include "sstream.h"
#include "ostream.h"

#include "ostream_.h"

namespace TEST {

	TEST::ostream& sstream::flush() noexcept {
		// no-op.
		return *this;
	}

	const char* sstream::c_str() const noexcept {
		char const* buffer = getBuffer().get();
		return buffer ? buffer : "";
	}

	size_t sstream::length() const noexcept {
		return getBuffer().length();
	}

} // namespace utils::io
