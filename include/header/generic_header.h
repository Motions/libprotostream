#pragma once

#include <cstddef>

#include <algorithm>
#include <tuple>

namespace protostream {
namespace detail {

namespace overlap_check {

template <class Field1, class Field2>
struct check_pair {
    static_assert(!(Field1::offset <= Field2::offset &&
                    Field1::offset + Field1::size > Field2::offset),
                  "fields overlap");
};

template <class Field>
struct check_pair<Field, Field> {};

template <class Field, class... Fields>
struct check_one_to_many : check_pair<Field, Fields>... {};

template <class... Fields>
struct check_all_pairs : check_one_to_many<Fields, Fields...>... {};
}

template <class Derived, class... Fields>
class generic_header {
    static constexpr overlap_check::check_all_pairs<Fields...> overlap_checker{};

public:
    static constexpr std::size_t size = std::max({(Fields::offset + Fields::size)...});

    template <class... Args>
    void read_self(Args&&... args) {
        /* Call read_self for every field */
        int _[] = {(std::get<Fields>(fields).read_self(args...), 0)...};
        (void)_;
    }

    template <class... Args>
    static Derived read(Args&&... args) {
        Derived result;
        result.read_self(std::forward<Args>(args)...);
        return result;
    }

    template <class... Args>
    void write(Args&&... args) const {
        /* Call write for every field */
        int _[] = {(std::get<Fields>(fields).write(args...), 0)...};
        (void)_;
    }

    template <class Field>
    const auto& get() const {
        return std::get<Field>(fields).value;
    }

    template <class Field>
    auto& get() {
        return std::get<Field>(fields).value;
    }

    constexpr bool operator==(const generic_header& that) const {
        return fields == that.fields;
    }

    constexpr bool operator!=(const generic_header& that) const {
        return fields != that.fields;
    }

private:
    std::tuple<Fields...> fields;
};
}
}
