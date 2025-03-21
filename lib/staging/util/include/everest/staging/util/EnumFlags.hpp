// SPDX-License-Identifier: Apache-2.0
// Copyright 2024 Pionix GmbH and Contributors to EVerest

#ifndef ENUMFLAGS_HPP
#define ENUMFLAGS_HPP

#include <atomic>
#include <type_traits>

namespace everest::staging::util {

template <typename T, typename B> class AtomicEnumFlags {
    static_assert(std::is_enum<T>(), "Not enum");
    static_assert(std::is_integral<B>(), "Not integer");
    static_assert((sizeof(B) * 8) >= static_cast<std::size_t>(T::last) + 1, "Underlying flag type too small");
    std::atomic<B> _value{0ULL};

public:
    constexpr std::size_t bit(const T& flag) const {
        return 1ULL << static_cast<std::underlying_type_t<T>>(flag);
    }

    constexpr void set(const T& flag, bool value) {
        if (value) {
            set(flag);
        } else {
            reset(flag);
        }
    }

    constexpr void set(const T& flag) {
        _value |= bit(flag);
    }

    constexpr void reset(const T& flag) {
        _value &= ~bit(flag);
    }

    constexpr void reset() {
        _value = 0ULL;
    }

    [[nodiscard]] constexpr bool all_reset() const {
        return _value == 0ULL;
    }

    constexpr bool is_set(const T& flag) const {
        return (_value & bit(flag)) != 0;
    }

    constexpr bool is_reset(const T& flag) const {
        return (_value & bit(flag)) == 0;
    }
};

} // namespace everest::staging::util
#endif
