#pragma once

#include <condition_variable>

#include <stddef.h>

namespace utils {

	class Condition : public std::condition_variable {
	public:
		using std::condition_variable::condition_variable;

		inline void notify_n(size_t n) noexcept {
			if (n == 1) {
				notify_one();
			}
			else if (n > 1) {
				notify_all();
			}
		}
	};

} // namespace utils

