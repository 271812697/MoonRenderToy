
#pragma once
#include "../util/ext.h"
#include "../util/math.h"

namespace component {

    class Component {
      protected:
        uint64_t uuid;  // universal unique instance id (in case of collision, go buy a lottery)
        bool enabled;

      public:
        Component() : uuid(utils::math::RandomGenerator<uint64_t>()), enabled(true) {}
        virtual ~Component() {}

        Component(const Component&) = default;
        Component& operator=(const Component&) = default;
        Component(Component&& other) noexcept = default;
        Component& operator=(Component&& other) noexcept = default;

        uint64_t UUID()          const { return uuid; }
        explicit operator bool() const { return enabled; }

        void Enable()  { enabled = true; }
        void Disable() { enabled = false; }
    };

    enum class ETag : uint16_t {  // allow up to 16 tags
        Untagged   = 1 << 0,
        Static     = 1 << 1,
        MainCamera = 1 << 2,
        WorldPlane = 1 << 3,
        Skybox     = 1 << 4,
        Water      = 1 << 5,
        Particle   = 1 << 6
    };

    // DEFINE_ENUM_FLAG_OPERATORS(ETag)  // C-style built-in solution for bitfields, don't use

    inline constexpr ETag operator|(ETag a, ETag b) {
        return static_cast<ETag>(utils::to_integral(a) | utils::to_integral(b));
    }

    inline constexpr ETag operator&(ETag a, ETag b) {
        return static_cast<ETag>(utils::to_integral(a) & utils::to_integral(b));
    }

    inline constexpr ETag operator^(ETag a, ETag b) {
        return static_cast<ETag>(utils::to_integral(a) ^ utils::to_integral(b));
    }

    inline constexpr ETag operator~(ETag a) {
        return static_cast<ETag>(~utils::to_integral(a));
    }

    inline ETag& operator|=(ETag& lhs, ETag rhs) { return lhs = lhs | rhs; }
    inline ETag& operator&=(ETag& lhs, ETag rhs) { return lhs = lhs & rhs; }
    inline ETag& operator^=(ETag& lhs, ETag rhs) { return lhs = lhs ^ rhs; }

    class Tag : public Component {
      private:
        ETag tag;

      public:
        explicit Tag(ETag tag) : Component(), tag(tag) {}

        void Add(ETag t) { tag |= t; }
        void Del(ETag t) { tag &= ~t; }

        constexpr bool Contains(ETag t) const {
            return utils::to_integral(tag & t) > 0;
        }
    };

}