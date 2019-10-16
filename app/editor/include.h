#pragma once

#include "base.h"
#include "global.h"

template <class T>
using UniquePtr = std::unique_ptr<T>;

template <class T>
using SharePtr = std::shared_ptr<T>;

template <class T>
using WeakPtr = std::weak_ptr<T>;

template <class T, class U>
SharePtr<T> CastPtr(const U & ptr)
{
#ifdef ENABLE_DEBUG
    return std::dynamic_pointer_cast<T>(ptr);
#else
    return std::reinterpret_pointer_cast<T>(ptr);
#endif
}
