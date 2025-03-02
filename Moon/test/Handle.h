#pragma once
#include <type_traits> // FIXME: STL headers are not allowed in public headers
#include <utility>

#include <stdint.h>
#include <assert.h>

namespace TEST {



	struct HwVertexBufferInfo;
	struct HwVertexBuffer;
	class HandleBase {
	public:
		using HandleId = uint32_t;
		static constexpr const HandleId nullid = HandleId{ UINT32_MAX };

		constexpr HandleBase() noexcept : object(nullid) {}

		// whether this Handle is initialized
		explicit operator bool() const noexcept { return object != nullid; }

		// clear the handle, this doesn't free associated resources
		void clear() noexcept { object = nullid; }

		// get this handle's handleId
		HandleId getId() const noexcept { return object; }

		// initialize a handle, for internal use only.
		explicit HandleBase(HandleId id) noexcept : object(id) {
			assert(object != nullid); // usually means an uninitialized handle is used
		}

	protected:
		HandleBase(HandleBase const& rhs) noexcept = default;
		HandleBase& operator=(HandleBase const& rhs) noexcept = default;

		HandleBase(HandleBase&& rhs) noexcept
			: object(rhs.object) {
			rhs.object = nullid;
		}

		HandleBase& operator=(HandleBase&& rhs) noexcept {
			if (this != &rhs) {
				object = rhs.object;
				rhs.object = nullid;
			}
			return *this;
		}

	private:
		HandleId object;
	};

	/**
	 * Type-safe handle to backend resources
	 * @tparam T Type of the resource
	 */
	template<typename T>
	struct Handle : public HandleBase {

		Handle() noexcept = default;

		Handle(Handle const& rhs) noexcept = default;
		Handle(Handle&& rhs) noexcept = default;

		// Explicitly redefine copy/move assignment operators rather than just using default here.
		// Because it doesn't make a call to the parent's method automatically during the std::move
		// function call(https://en.cppreference.com/w/cpp/algorithm/move) in certain compilers like
		// NDK 25.1.8937393 and below (see b/371980551)
		Handle& operator=(Handle const& rhs) noexcept {
			HandleBase::operator=(rhs);
			return *this;
		}
		Handle& operator=(Handle&& rhs) noexcept {
			HandleBase::operator=(std::move(rhs));
			return *this;
		}

		explicit Handle(HandleId id) noexcept : HandleBase(id) { }

		// compare handles of the same type
		bool operator==(const Handle& rhs) const noexcept { return getId() == rhs.getId(); }
		bool operator!=(const Handle& rhs) const noexcept { return getId() != rhs.getId(); }
		bool operator<(const Handle& rhs) const noexcept { return getId() < rhs.getId(); }
		bool operator<=(const Handle& rhs) const noexcept { return getId() <= rhs.getId(); }
		bool operator>(const Handle& rhs) const noexcept { return getId() > rhs.getId(); }
		bool operator>=(const Handle& rhs) const noexcept { return getId() >= rhs.getId(); }

		// type-safe Handle cast
		template<typename B, typename = std::enable_if_t<std::is_base_of<T, B>::value> >
		Handle(Handle<B> const& base) noexcept : HandleBase(base) { } // NOLINT(hicpp-explicit-conversions,google-explicit-constructor)


	};

	using VertexBufferHandle = Handle<HwVertexBuffer>;
	using VertexBufferInfoHandle = Handle<HwVertexBufferInfo>;

}
