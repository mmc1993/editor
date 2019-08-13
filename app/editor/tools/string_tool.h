#pragma once

#include "../include.h"

namespace string_tool {
	
	inline std::string Join(const std::vector<std::string> & vec, const std::string & str)
	{
		std::string  ret;
		size_t count = 0;
		if (vec.empty()) { return std::move(ret); }
		for (auto & s : vec) { count += s.size(); }
		
		ret.reserve(count);
		auto it = vec.begin(); 
		ret.append(*it++); 
		while (it != vec.end()) 
		{
			ret.append(str);
			ret.append(*it++); 
		}
		return std::move(ret);
	}

	inline std::vector<std::string> Split(
		const std::string & str, 
		const std::string & sep)
	{
		std::vector<std::string> result;
		auto pos = (size_t)0;
		auto end = str.find(sep, pos);
		while (end != std::string::npos)
		{
			result.push_back(str.substr(pos, end - pos));
			pos = end + sep.size();
			end = str.find(sep, pos);
		}
		if (pos < str.size())
		{
			result.push_back(str.substr(pos, end - pos));
		}
		return std::move(result);
	}

	inline std::string Replace(const std::string & str, 
							   const std::string & sold, 
							   const std::string & snew, 
							   size_t count = ~0)
	{
		std::vector<std::string> vec;
		auto pos = (size_t)0;
		auto end = str.find(sold, pos);
		while (end != std::string::npos && --count != 0)
		{
			vec.push_back(str.substr(pos, end - pos));
			vec.push_back(snew);
			pos = end + sold.size();
			end = str.find(sold, pos);
		}
		if (pos < str.size())
		{
			vec.push_back(str.substr(pos, end - pos));
		}
		return Join(vec, "");
	}

    //  比较字符串是否一样
    //  同时忽略空白字符
    inline bool IsEqualSkipSpace(const std::string & str0, const char * str1)
    {
        static auto onFind = [](const std::string::value_type & ch) {
            return (int)ch > 32;
        };
        auto len = std::strlen(str1);
        auto it = std::find_if(str0.begin(), str0.end(), onFind);
        if (it == str0.end())
        {
            return len == 0;
        }
        auto pos = std::distance(str0.begin(), it);
        return str0.compare(pos, len, str1, len) == 0;
    }

    //  判断是否可打印字符串
    inline bool IsPrint(const std::string & str)
    {
        return !std::all_of(str.begin(), str.end(), [](const char ch)
            {
                return (int)ch <= 32;
            });
    }

    //  返回文件名 没后缀
    inline std::string QueryFileName(const std::string & fname)
    {
        assert(std::string::npos == fname.find_first_of('\\'));
        auto dot = fname.find_last_of('.');
        auto pos = fname.find_last_of('/');
        return fname.substr(pos + 1, dot - (pos + 1));
    }

    //  返回后缀名
    inline std::string QueryFileSuffix(const std::string & fname)
    {
        assert(std::string::npos == fname.find_first_of('\\'));
        auto pos = fname.find_last_of('.');
        return pos != std::string::npos
            ? fname.substr(pos)
            : std::string();
    }

    //  返回文件名 有后缀
    inline std::string QueryFileFullName(const std::string & fname)
    {
        assert(std::string::npos == fname.find_first_of('\\'));
        auto pos = fname.find_last_of('/');
        return fname.substr(pos + 1);
    }
}