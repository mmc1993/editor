#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cassert>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <functional>
#include <type_traits>
#include "sformat.h"

//  条件判断, 抛出异常
#define DEBUG_CHECK(exp, type, ...) if (!exp) { throw type(__VA_ARGS__); }

//  异常说明, 显示Json错误位置往后20个字符
#define DEBUG_ERROR(exp, string) SFormat("{0}: {1}", exp, std::string(string).substr(0, 20))

namespace mmc {
    class Json : public std::enable_shared_from_this<Json> {
    public:
        //  异常定义
        class Error : public std::exception {
        public:
            Error(const std::string & what) : std::exception(what.c_str()) {}
        };

    public:
        using Pointer = std::shared_ptr<Json>;

		struct Element {
			Element() {}
            Element(const Pointer & value) : mVal(value) {}
			Element(const Pointer & value, const std::string & key) : mKey(key), mVal(value) {}
            bool operator==(const Element & other) const { return mVal == other.mVal; }
			bool operator==(const Pointer & other) const { return mVal == other; }
			bool operator==(const std::string & key) const { return mKey == key; }
			std::string mKey;
			Pointer mVal;
		};

        enum Type {
            kNUMBER,
            kSTRING,
            kHASH,
            kLIST,
            kBOOL,
        };

    private:
        struct Parser {
            static const char * SkipSpace(const char * string)
            {
                for (; *string != '\0' && *string <= 32; ++string);
                return string;
            }

            static const char * ParseList(const char * string, std::vector<Element> & childs)
            {
                childs.clear();
                while (*string != ']')
                {
					auto value = New();
                    string = Parser::Parse(string, value);
                    string = SkipSpace(string);
                    *string == ',' && ++string;
                    string = SkipSpace(string);
                    childs.push_back(value);
                }
                return ++string;
            }

            static const char * ParseHash(const char * string, std::vector<Element> & childs)
            {
                childs.clear();
                while (*string != '}')
                {
                    DEBUG_CHECK((*string == '\"'), Error, DEBUG_ERROR("Parse Hash Error: ", string));
					std::string key; auto value = New();
                    string = SkipSpace(Parser::ParseString(string + 1, key));
                    DEBUG_CHECK((*string == ':'), Error, DEBUG_ERROR("Parse Hash Error: ", string));
                    string = SkipSpace(Parser::Parse(string + 1, value));
                    *string == ',' && ++string;
                    string = SkipSpace(string);
                    childs.emplace_back(value, key);
                }
                return ++string;
            }

            static const char * ParseString(const char * string, std::string & output)
            {
                output.clear();
                for (; *string != '\"'; ++string)
                {
                    DEBUG_CHECK(*string != '\0', Error, DEBUG_ERROR("Parse String Error", string));
                    output.append(1, *string);
                }
                return string + 1;
            }

            static const char * ParseNumber(const char * string, float & output)
            {
                char value[64] = { 0 };
                for (auto i = 0; 
					*string >= '0' &&
                    *string <= '9' ||
                    *string == '.' ||
                    *string == '-'; ++i, ++string)
                {
                    value[i] = *string;
                }
                output = std::strtof(value, nullptr);
                return string;
            }

            static const char * ParseFalse(const char * string, float & output)
            {
                output = 0; return string + 5;
            }

            static const char * ParseTrue(const char * string, float & output)
            {
                output = 1; return string + 4;
            }

            static const char * Parse(const char * string, Pointer & value)
            {
                string = Parser::SkipSpace(string);
                if (*string == '[')
                {
                    string = ParseList(SkipSpace(string + 1), value->_elems);
                    value->_type = mmc::Json::Type::kLIST;
                }
                else if (*string == '{')
                {
                    string = ParseHash(SkipSpace(string + 1), value->_elems);
                    value->_type = mmc::Json::Type::kHASH;
                }
                else if (*string == '\"')
                {
                    string = ParseString(string + 1, value->_string);
                    value->_type = mmc::Json::Type::kSTRING;
                }
                else if (*string >= '0' && *string <= '9' || *string == '-')
                {
                    string = ParseNumber(string, value->_number);
                    value->_type = mmc::Json::Type::kNUMBER;
                }
                else if (string[0] == 't' &&
                         string[1] == 'r' &&
                         string[2] == 'u' &&
                         string[3] == 'e')
                {
                    string = ParseTrue(string, value->_number);
                    value->_type = mmc::Json::Type::kBOOL;
                }
                else if (string[0] == 'f' &&
                         string[1] == 'a' &&
                         string[2] == 'l' &&
                         string[3] == 's' &&
                         string[4] == 'e')
                {
                    string = ParseFalse(string, value->_number);
					value->_type = mmc::Json::Type::kBOOL;
                }
                else
                {
                    DEBUG_CHECK(false, Error, DEBUG_ERROR("Parse Json Error", string));
                }
                return string;
            }
        };

	public:
		Json() : _type(Type::kNUMBER), _number(0) { }

        static Pointer New()
        {
            return std::make_shared<Json>();
        }

        static Pointer Hash()
        {
            auto ret = New();
            ret->_type = Type::kHASH;
            return ret;
        }

        static Pointer List()
        {
            auto ret = New();
            ret->_type = Type::kLIST;
            return ret;
        }

		template <class T>
		static Pointer FromValue(const T & val)
		{
			auto value = New();
			value->Set(val);
			return value;
		}

		static Pointer FromValue(const std::string & val)
		{
			return FromValue(val.c_str(), val.size());
		}

		static Pointer FromValue(const char * val, size_t len = 0)
		{
			auto value = New();
			value->Set(val, len);
			return value;
		}

        static Pointer FromFile(const std::string & fname)
        {
            std::string buffer;
            std::ifstream ifile(fname);
            std::noskipws(ifile);
            std::copy(
                std::istream_iterator<char>(ifile),
                std::istream_iterator<char>(),
                std::back_inserter(buffer));
            return FromBuffer(buffer);
        }

		static Pointer FromBuffer(const std::string & buffer)
		{
            auto pointer = New();
            try
            {
                Parser::Parse(buffer.c_str(), pointer);
            }
            catch (const Error & err)
            {
                ASSERT_LOG(&err, "");
                return nullptr;
            }
            return pointer;
		}

        Type                GetType()   { return _type; }
        size_t              GetCount()  { return _elems.size(); }
        bool                ToBool()    { return _number != 0; }
        float               ToNumber()  { return _number; }
        const std::string & ToString()  { return _string; }
        std::vector<Element> & GetElements() { return _elems; }

        std::string Print(size_t space = 0)
        {
            switch (GetType())
            {
            case kNUMBER:
                {
                    return std::to_string(_number);
                }
                break;
            case kSTRING:
                {
                    return "\"" + _string + "\"";
                }
                break;
            case kHASH:
                {
                    std::vector<std::string> strings;
                    std::string resule("{\n" + std::string(++space, '\t'));
                    for (const auto & ele : _elems)
                    {
                        strings.push_back(SFormat("\"{0}\": {1}",
                            ele.mKey, ele.mVal->Print(space)));
                    }
                    resule.append(tools::Join(strings, ",\n" + std::string(space, '\t')));
                    resule.append("\n"); resule.append(--space, '\t'); resule.append("}");
                    return std::move(resule);
                }
                break;
            case kLIST:
                {
                    std::vector<std::string> strings;
                    for (const auto & ele : _elems)
                    {
                        strings.push_back(ele.mVal->Print(space));
                    }
                    return "[" + tools::Join(strings, ", ") + "]";
                }
                break;
            case kBOOL:
                {
                    return ToBool() ? "true" : "false";
                }
                break;
            }
            return "null";
        }

        //  赋值函数
        template <class T>
        void Set(T && val)
        {
            using _Type = std::remove_const_t<std::remove_reference_t<std::remove_cv_t<T>>>;

            if constexpr (std::is_same_v<_Type, Pointer>)
            {
                _type = val->_type;
                _elems = val->_elems;
                _number = val->_number;
            }
            else if constexpr (std::is_arithmetic_v<_Type>)
            {
                _type = Type::kNUMBER; _elems.clear(); _number = (float)val;
            }
            else if constexpr (std::is_same_v<_Type, bool>)
            {
                _type = Type::kBOOL; _elems.clear(); _number = val ? 1 : 0;
            }
            else if constexpr (std::is_same_v<_Type, char *>)
            {
                _type = Type::kSTRING; _elems.clear(); _string.assign(val);
            }
            else if constexpr (std::is_same_v<_Type, std::string>)
            {
                _type = Type::kSTRING; _elems.clear(); _string = val;
            }
        }

		void Set(const char * val, size_t len = 0) 
		{ 
			_elems.clear();
			_string = 0 == len ? val
			: std::string(val, len);
			_type = Type::kSTRING;
		}

        template <class Key, class ...Keys>
        bool HasKey(const Key & key, Keys && ... keys)
        {
            if constexpr (std::is_arithmetic_v<Key>)
            {
                if (GetType() == kLIST && key < GetCount())
                {
                    return GetElements().at(key).mVal->HasKey(std::forward<Keys>(keys)...);
                }
            }
            else if (auto it = Find(key); it != _elems.end())
            {
                it->mVal->HasKey(std::forward<Keys>(keys)...);
            }
            return false;
        }

        template <class Key>
        bool HasKey(const Key & key)
        {
            if constexpr (std::is_arithmetic_v<Key>)
            {
                return GetType() == kLIST && key < GetCount();
            }
            else
            {
                return Find(key) != GetElements().end();
            }
        }

        template <class Key, class ...Keys>
        Pointer & At(const Key & key, Keys && ... keys)
        {
            return At(key)->At(std::forward<Keys>(keys)...);
        }

        template <class Key>
        Pointer & At(const Key & key)
        {
            return AtImpl(key);
        }

		template <class Key, class ...Keys>
		Pointer Insert(const Pointer & value, const Key & key, Keys && ... keys)
		{
			return At(key)->Insert(value, std::forward<Keys>(keys)...);
		}

		template <class Key>
		Pointer Insert(const Pointer & value, const Key & key)
		{
			return InsertImpl(value, key);
		}

    private:


        Pointer InsertImpl(const Pointer & value, const char * key)
        {
            assert(_type == Type::kHASH);
            return InsertImpl(value, std::string(key));
        }

        Pointer InsertImpl(const Pointer & value, const std::string & key)
        {
            assert(_type == Type::kHASH);
            if (Find(key) == _elems.end())
            {
                _elems.emplace_back(value, key);
            }
            return shared_from_this();
        }

        Pointer InsertImpl(const Pointer & value, size_t idx = (size_t)-1)
        {
            assert(_type == Type::kLIST);
            auto insert = idx >= _elems.size()
                ? _elems.insert(_elems.end(), value)
                : std::next(_elems.begin(), idx);
            _elems.insert(insert, value);
            return shared_from_this();
        }

        Pointer & AtImpl(size_t idx)
        {
            assert(_type == Type::kLIST);
            return _elems.at(idx).mVal;
        }

        Pointer & AtImpl(const char * key)
        {
            assert(_type == Type::kHASH);
            return AtImpl(std::string(key));
        }

        Pointer & AtImpl(const std::string & key)
        {
            auto it = Find(key);
            assert(it != _elems.end());
            return const_cast<Pointer &>(it->mVal);
        }

		std::vector<Element>::const_iterator Find(const std::string & key)
		{
			return std::find(_elems.begin(), _elems.end(), key);
		}

    private:
        std::vector<Element> _elems;
        std::string     _string;
        float           _number;
        Type            _type;
        friend struct Parser;
    };
}

namespace std {
    inline vector<mmc::Json::Element>::iterator begin(mmc::Json::Pointer & json)
    {
        return begin(json->GetElements());
    }

    inline vector<mmc::Json::Element>::iterator end(mmc::Json::Pointer & json)
    {   
        return end(json->GetElements());
    }

    inline vector<mmc::Json::Element>::iterator begin(const mmc::Json::Pointer & json)
    {
        return begin(json->GetElements());
    }

    inline vector<mmc::Json::Element>::iterator end(const mmc::Json::Pointer & json)
    {
        return end(json->GetElements());
    }

    inline std::string to_string(const mmc::Json & json)
    {
        return const_cast<mmc::Json &>(json).Print();
    }

    inline std::string to_string(const mmc::Json::Pointer & json)
    {
        return to_string(*json);
    }
}