#pragma once

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

    template <class Iter, class Val>
    bool exist(Iter first, Iter last, const Val & val)
    {
        return std::find(first, last, val) != last;
    }

    //  Ìø¹ý×ó±ß×Ö·û
    inline std::string lstrip(const std::string & src, char ch)
    {
        auto pos = src.find_first_not_of(ch);
        return pos != std::string::npos
            ? src.substr(pos)
            : std::string();
    }

    template <class T>
    size_t length(const T & arr)
    {
        return std::distance(std::begin(arr), std::end(arr));
    }

    inline std::string to_string(void * val)
    {
        return std::to_string((size_t)val);
    }

    template <class T, class ...ARGS>
    std::shared_ptr<T> create_ptr(ARGS && ...args)
    {
        return std::make_shared<T>(std::forward<ARGS>(args)...);
    }
}