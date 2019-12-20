#pragma once

#include <iterator>
#include <algorithm>

#define SAFE_DELETE(ptr)        { delete ptr; ptr = nullptr; }

namespace std {
    template <class Iter, class Val>
    bool all_ofv(Iter first, Iter last, const Val & val)
    {
        return all_of(first, last, [&](const auto & ele) { return ele == val; });
    }

    template <class Iter, class Val>
    bool any_ofv(Iter first, Iter last, const Val & val)
    {
        return any_of(first, last, [&](const auto & ele) { return ele == val; });
    }

    template <class Iter, class Val>
    bool none_ofv(Iter first, Iter last, const Val & val)
    {
        return none_of(first, last, [&](const auto & ele) { return ele == val; });
    }

    template <class T>
    size_t length(const T & arr)
    {
        return std::distance(std::begin(arr), std::end(arr));
    }
}