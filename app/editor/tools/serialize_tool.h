#pragma once

//  POD
template <class T, typename std::enable_if_t<std::is_trivially_copyable_v<T>, int> N = 0>
void Serialize(std::ostream & os, const T & val)
{
    os.write((const char *)&val, sizeof(T));
}

//  ÈÝÆ÷
template <class T, typename std::enable_if_t<
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().begin())> &&
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().end())> &&
    std::is_trivially_copyable_v<typename T::value_type>, int> N = 0>
    void Serialize(std::ostream & os, const T & val)
{
    unsigned int size = val.size();
    os.write((const char *)&size, sizeof(size));
    os.write((const char *)val.data(), size * sizeof(typename T::value_type));
}

template <class T, typename std::enable_if_t<
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().begin())> &&
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().end())> &&
    !std::is_trivially_copyable_v<typename T::value_type>, int> N = 0>
void Serialize(std::ostream & os, const T & val)
{
    unsigned int size = val.size();
    os.write((const char *)&size, sizeof(size));
    for (auto & v : val) { Serialize(os, v); }
}

//  POD
template <class T, typename std::enable_if_t<std::is_trivially_copyable_v<T>, int> N = 0>
void Deserialize(std::istream & is, T & val)
{
    is.read((char *)&val, sizeof(T));
}

//  ÈÝÆ÷
template <class T, typename std::enable_if_t<
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().begin())> &&
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().end())> &&
    std::is_trivially_copyable_v<typename T::value_type>, int> N = 0>
    void Deserialize(std::istream & is, T & val)
{
    unsigned int size = 0;
    is.read((char *)&size, sizeof(size));
    val.resize(size);
    is.read((char *)val.data(), size * sizeof(typename T::value_type));
}

template <class T, typename std::enable_if_t<
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().begin())> &&
    std::is_same_v<typename T::iterator, decltype(std::declval<T>().end())> &&
    !std::is_trivially_copyable_v<typename T::value_type>, int> N = 0>
void Deserialize(std::istream & is, T & val)
{
    unsigned int size = 0;
    is.read((char *)&size, sizeof(size));
    val.resize(size);
    for (auto & v : val) { Deserialize(is, v); }
}