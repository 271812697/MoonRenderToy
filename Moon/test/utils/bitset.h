#pragma once
#include "algorithm.h"
#include <assert.h>
#include <stddef.h>
#include <stdint.h>


#include <algorithm> // for std::fill
#include <iterator>
#include <type_traits>



namespace utils {


	template<typename T, size_t N = 1,
		typename = typename std::enable_if<std::is_integral<T>::value&&
		std::is_unsigned<T>::value>::type>
	class  bitset {
		T storage[N];

	public:
		static constexpr T BITS_PER_WORD = sizeof(T) * 8;
		static constexpr T BIT_COUNT = BITS_PER_WORD * N;
		static constexpr T WORLD_COUNT = N;
		using container_type = T;

		bitset() noexcept {
			std::fill(std::begin(storage), std::end(storage), 0);
		}

		template<typename U, typename = typename std::enable_if_t<N == 1, U>>
		explicit bitset(U value) noexcept {
			storage[0] = value;
		}

		T getBitsAt(size_t n) const noexcept {
			assert(n < N);
			return storage[n];
		}

		T& getBitsAt(size_t n) noexcept {
			assert(n < N);
			return storage[n];
		}

		T getValue() const noexcept {
			static_assert(N == 1, "bitfield must only have one storage word");
			return storage[0];
		}

		void setValue(T value) noexcept {
			static_assert(N == 1, "bitfield must only have one storage word");
			storage[0] = value;
		}

		template<typename F>
		void forEachSetBit(F exec) const noexcept {
			for (size_t i = 0; i < N; i++) {
				T v = storage[i];
				while (v) {
					T k = utils::ctz(v);
					v &= ~(T(1) << k);
					exec(size_t(k + BITS_PER_WORD * i));
				}
			}
		}

		size_t size() const noexcept { return N * BITS_PER_WORD; }

		bool empty() const noexcept { return none(); }

		bool test(size_t bit) const noexcept { return operator[](bit); }

		void set(size_t b) noexcept {
			assert(b / BITS_PER_WORD < N);
			storage[b / BITS_PER_WORD] |= T(1) << (b % BITS_PER_WORD);
		}

		void set(size_t b, bool value) noexcept {
			assert(b / BITS_PER_WORD < N);
			storage[b / BITS_PER_WORD] &= ~(T(1) << (b % BITS_PER_WORD));
			storage[b / BITS_PER_WORD] |= T(value) << (b % BITS_PER_WORD);
		}

		void unset(size_t b) noexcept {
			assert(b / BITS_PER_WORD < N);
			storage[b / BITS_PER_WORD] &= ~(T(1) << (b % BITS_PER_WORD));
		}

		void flip(size_t b) noexcept {
			assert(b / BITS_PER_WORD < N);
			storage[b / BITS_PER_WORD] ^= T(1) << (b % BITS_PER_WORD);
		}

		void reset() noexcept {
			std::fill(std::begin(storage), std::end(storage), 0);
		}

		void clear() noexcept {
			reset();
		}

		bool operator[](size_t b) const noexcept {
			assert(b / BITS_PER_WORD < N);
			return bool(storage[b / BITS_PER_WORD] & (T(1) << (b % BITS_PER_WORD)));
		}

		size_t count() const noexcept {

			{
				T r = utils::popcount(storage[0]);
				for (size_t i = 1; i < N; ++i) {
					r += utils::popcount(storage[i]);
				}
				return r;
			}
		}

		bool any() const noexcept {

			{
				T r = storage[0];
				for (size_t i = 1; i < N; ++i) {
					r |= storage[i];
				}
				return bool(r);
			}
		}

		bool none() const noexcept {
			return !any();
		}

		bool all() const noexcept {

			{
				T r = storage[0];
				for (size_t i = 1; i < N; ++i) {
					r &= storage[i];
				}
				return T(~r) == T(0);
			}
		}

		bool operator!=(const bitset& b) const noexcept {

			{
				T r = storage[0] ^ b.storage[0];
				for (size_t i = 1; i < N; ++i) {
					r |= storage[i] ^ b.storage[i];
				}
				return bool(r);
			}
		}

		bool operator==(const bitset& b) const noexcept {
			return !operator!=(b);
		}

		bitset& operator&=(const bitset& b) noexcept {

			{
				for (size_t i = 0; i < N; ++i) {
					storage[i] &= b.storage[i];
				}
			}
			return *this;
		}

		bitset& operator|=(const bitset& b) noexcept {

			{
				for (size_t i = 0; i < N; ++i) {
					storage[i] |= b.storage[i];
				}
			}
			return *this;
		}

		bitset& operator^=(const bitset& b) noexcept {

			{
				for (size_t i = 0; i < N; ++i) {
					storage[i] ^= b.storage[i];
				}
			}
			return *this;
		}

		bitset operator~() const noexcept {
			bitset r;

			{
				for (size_t i = 0; i < N; ++i) {
					r.storage[i] = ~storage[i];
				}
			}
			return r;
		}

	private:
		friend bool operator<(bitset const& lhs, bitset const& rhs) noexcept {
			return std::lexicographical_compare(
				std::begin(lhs.storage), std::end(lhs.storage),
				std::begin(rhs.storage), std::end(rhs.storage)
			);
		}

		friend bitset operator&(const bitset& lhs, const bitset& rhs) noexcept {
			return bitset(lhs) &= rhs;
		}

		friend bitset operator|(const bitset& lhs, const bitset& rhs) noexcept {
			return bitset(lhs) |= rhs;
		}

		friend bitset operator^(const bitset& lhs, const bitset& rhs) noexcept {
			return bitset(lhs) ^= rhs;
		}
	};

	using bitset8 = bitset<uint8_t>;
	using bitset32 = bitset<uint32_t>;
	using bitset64 = bitset<uint64_t>;
	using bitset128 = bitset<uint64_t, 2>;
	using bitset256 = bitset<uint64_t, 4>;

	static_assert(sizeof(bitset8) == 1, "bitset8 isn't 8 bits!");
	static_assert(sizeof(bitset32) == 4, "bitset32 isn't 32 bits!");
	static_assert(sizeof(bitset64) == 8, "bitset64 isn't 64 bits!");
	static_assert(sizeof(bitset128) == 16, "bitset128 isn't 128 bits!");
	static_assert(sizeof(bitset256) == 32, "bitset256 isn't 256 bits!");

}