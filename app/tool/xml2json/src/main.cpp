#include "include.h"

#include "tools/std_tool.h"
#include "tools/file_tool.h"
#include "tools/debug_tool.h"
#include "tools/string_tool.h"

#include "third/mmcjson.h"
#include "third/sformat.h"
#include "third/tinyxml/tinyxml.h"

constexpr auto JSON_PROPERTY = "__Property";
constexpr auto JSON_CHILDREN = "__Children";

// ---
//  控件对齐
// ---
enum class UIAlignEnum {
    kDEFAULT,               //  绝对坐标
    kCLING_T = 1 << 1,      //  靠上
    kCLING_B = 1 << 2,      //  靠下
    kCLING_L = 1 << 3,      //  靠左
    kCLING_R = 1 << 4,      //  靠右
    kCENTER_H = 1 << 5,     //  水平居中
    kCENTER_V = 1 << 6,     //  垂直居中
    kSTRETCH_H = 1 << 7,    //  水平拉伸
    kSTRETCH_V = 1 << 8,    //  垂直拉伸
    LENGTH,
};

// ---
//  文本对齐
// ---
enum class UITextAlignEnum {
    kCLING_T = 1 << 1,      //  靠上
    kCLING_B = 1 << 2,      //  靠下
    kCLING_L = 1 << 3,      //  靠左
    kCLING_R = 1 << 4,      //  靠右
    kCENTER_H = 1 << 5,     //  水平居中
    kCENTER_V = 1 << 6,     //  垂直居中
};

//  值字符串转数字
static const std::map<std::string, std::map<std::string, int>> VALUE_STRING_2_NUMBER {
    std::make_pair("Align", std::map<std::string, int>
        {
            std::make_pair("Default",               (int)UIAlignEnum::kDEFAULT),
            std::make_pair("Stretch",               (int)UIAlignEnum::kSTRETCH_H | (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Center",                (int)UIAlignEnum::kCENTER_H | (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Left Top",              (int)UIAlignEnum::kCLING_L | (int)UIAlignEnum::kCLING_T),
            std::make_pair("Left Bottom",           (int)UIAlignEnum::kCLING_L | (int)UIAlignEnum::kCLING_B),
            std::make_pair("Left VStretch",         (int)UIAlignEnum::kCLING_L | (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Left VCenter",          (int)UIAlignEnum::kCLING_L | (int)UIAlignEnum::kCENTER_V),
            std::make_pair("Right Top",             (int)UIAlignEnum::kCLING_R | (int)UIAlignEnum::kCLING_T),
            std::make_pair("Right Bottom",          (int)UIAlignEnum::kCLING_R | (int)UIAlignEnum::kCLING_B),
            std::make_pair("Right VStretch",        (int)UIAlignEnum::kCLING_R | (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Right VCenter",         (int)UIAlignEnum::kCLING_R | (int)UIAlignEnum::kCENTER_V),
            std::make_pair("HStretch Top",          (int)UIAlignEnum::kSTRETCH_H | (int)UIAlignEnum::kCLING_T),
            std::make_pair("HStretch Bottom",       (int)UIAlignEnum::kSTRETCH_H | (int)UIAlignEnum::kCLING_B),
            std::make_pair("HStretch VStretch",     (int)UIAlignEnum::kSTRETCH_H | (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("HStretch VCenter",      (int)UIAlignEnum::kSTRETCH_H | (int)UIAlignEnum::kCENTER_V),
            std::make_pair("HCenter Top",           (int)UIAlignEnum::kCENTER_H | (int)UIAlignEnum::kCLING_T),
            std::make_pair("HCenter Bottom",        (int)UIAlignEnum::kCENTER_H | (int)UIAlignEnum::kCLING_B),
            std::make_pair("HCenter VStretch",      (int)UIAlignEnum::kCENTER_H | (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("HCenter VCenter",       (int)UIAlignEnum::kCENTER_H | (int)UIAlignEnum::kCENTER_V),
            std::make_pair("Left Right",            (int)UIAlignEnum::kCLING_L | (int)UIAlignEnum::kCLING_R),
            std::make_pair("Top Bottom",            (int)UIAlignEnum::kCLING_T | (int)UIAlignEnum::kCLING_B),
            std::make_pair("Top Left Right",        (int)UIAlignEnum::kCLING_T | (int)UIAlignEnum::kCLING_L | (int)UIAlignEnum::kCLING_R),
            std::make_pair("Top Bottom Left",       (int)UIAlignEnum::kCLING_T | (int)UIAlignEnum::kCLING_B | (int)UIAlignEnum::kCLING_L),
            std::make_pair("Top Bottom Right",      (int)UIAlignEnum::kCLING_T | (int)UIAlignEnum::kCLING_B | (int)UIAlignEnum::kCLING_R),
            std::make_pair("Top Bottom Right Left", (int)UIAlignEnum::kCLING_T | (int)UIAlignEnum::kCLING_B | (int)UIAlignEnum::kCLING_L | (int)UIAlignEnum::kCLING_R),
        }),
    std::make_pair("Type", std::map<std::string, int>
        {
            std::make_pair("Layout",        0),
            std::make_pair("TreeBox",       1),
            std::make_pair("TextBox",       2),
            std::make_pair("ImageBox",      3),
            std::make_pair("ComboBox",      4),
            std::make_pair("UICanvas",      5),
            std::make_pair("GLCanvas",      6),
        }),
    std::make_pair("TextAlign", std::map<std::string, int>
        {
            std::make_pair("Default",           (int)UITextAlignEnum::kCLING_L),
            std::make_pair("Center",            (int)UITextAlignEnum::kCENTER_H | (int)UITextAlignEnum::kCENTER_V),
            std::make_pair("Left Top",          (int)UITextAlignEnum::kCLING_L | (int)UITextAlignEnum::kCLING_T),
            std::make_pair("Left Bottom",       (int)UITextAlignEnum::kCLING_L | (int)UITextAlignEnum::kCLING_B),
            std::make_pair("Left VCenter",      (int)UITextAlignEnum::kCLING_L | (int)UITextAlignEnum::kCENTER_V),
            std::make_pair("Right Top",         (int)UITextAlignEnum::kCLING_R | (int)UITextAlignEnum::kCLING_T),
            std::make_pair("Right Bottom",      (int)UITextAlignEnum::kCLING_R | (int)UITextAlignEnum::kCLING_B),
            std::make_pair("Right VCenter",     (int)UITextAlignEnum::kCLING_R | (int)UITextAlignEnum::kCENTER_V),
            std::make_pair("HCenter Top",       (int)UITextAlignEnum::kCENTER_H | (int)UITextAlignEnum::kCLING_T),
            std::make_pair("HCenter Bottom",    (int)UITextAlignEnum::kCENTER_H | (int)UITextAlignEnum::kCLING_B),
            std::make_pair("HCenter VCenter",   (int)UITextAlignEnum::kCENTER_H | (int)UITextAlignEnum::kCENTER_V),
        })
};

//  值字符串替换
static const std::map<std::string, std::map<std::string, std::string>> VALUE_STRING_2_REPLACE {
    std::make_pair("Type", std::map<std::string, std::string>
        {
            std::make_pair("Caption", "Name"),
            std::make_pair("Widget", "Layout"),
            std::make_pair("ListBox", "TreeBox"),
            std::make_pair("EditBox", "TextBox"),
        })
};

//  键字符串替换
static const std::map<std::string, std::string> KEY_STRING_2_REPLACE {
    std::make_pair("Position",  "Move"),
    std::make_pair("Caption",   "Name"),
};

//  值字符串转数字
bool IsHasKey(const std::map<std::string, std::map<std::string, int>> & m,
    const std::string & k0,
    const std::string & k1)
{
    auto it0 = m.find(k0);
    if (it0 != m.end())
    {
        auto it1 = it0->second.find(k1);
        return it1 != it0->second.end();
    }
    return false;
}

//  键字符串替换
bool IsHasKey(const std::map<std::string, std::map<std::string, std::string>> & m, 
            const std::string & k0, 
            const std::string & k1)
{
    auto it0 = m.find(k0);
    if (it0 != m.end())
    {
        auto it1 = it0->second.find(k1);
        return it1 != it0->second.end();
    }
    return false;
}

//  值字符串替换
bool IsHasKey(const std::map<std::string, std::string> & m, 
            const std::string & k)
{
    return m.find(k) != m.end();
}

std::string Replace(
    const std::map<std::string, std::map<std::string, int>> & m,
    const std::string & k0, 
    const std::string & k1)
{
    return std::to_string(m.find(k0)->second.find(k1)->second);
}

const std::string & Replace(
    const std::map<std::string, std::map<std::string, std::string>> & m, 
    const std::string & k0, 
    const std::string & k1)
{
    return m.find(k0)->second.find(k1)->second;
}

const std::string & Replace(
    const std::map<std::string,std::string> & m, 
    const std::string & k0)
{
    return m.find(k0)->second;
}

void CustomTransfer(mmc::JsonValue::Value json)
{
    for (auto & ele : json->At(JSON_PROPERTY))
    {
        if ('a' <= ele.mKey.at(0) && 'z' >= ele.mKey.at(0))
        {
            ele.mKey.at(0) -= 'a';
            ele.mKey.at(0) += 'A';
        }
        if (IsHasKey(KEY_STRING_2_REPLACE, ele.mKey))
        {
            ele.mKey = Replace(KEY_STRING_2_REPLACE, ele.mKey);
        }
        if (IsHasKey(VALUE_STRING_2_REPLACE, ele.mKey, ele.mValue->ToString()))
        {
            ele.mValue->Set(Replace(VALUE_STRING_2_REPLACE, ele.mKey, ele.mValue->ToString()));
        }
        if (IsHasKey(VALUE_STRING_2_NUMBER, ele.mKey, ele.mValue->ToString()))
        {
            ele.mValue->Set(Replace(VALUE_STRING_2_NUMBER, ele.mKey, ele.mValue->ToString()));
        }
    }
    for (auto ele : json->At(JSON_CHILDREN))
    {
        CustomTransfer(ele.mValue);
    }
}

mmc::JsonValue::Value ParseElement(TiXmlElement * xml);
void ParseProperty(TiXmlElement * xml, mmc::JsonValue::Value json);
void ParerChildren(TiXmlElement * xml, mmc::JsonValue::Value json);

mmc::JsonValue::Value CheckListField(mmc::JsonValue::Value json, const std::string & key)
{
    if (!json->IsHashKey(key))
    {
        json->Insert(mmc::JsonValue::FromValue(mmc::JsonValue::List()), key);
    }
    return json->At(key);
}

mmc::JsonValue::Value CheckHashField(mmc::JsonValue::Value json, const std::string & key)
{
    if (!json->IsHashKey(key))
    {
        json->Insert(mmc::JsonValue::FromValue(mmc::JsonValue::Hash()), key);
    }
    return json->At(key);
}

//  解析属性
void ParseProperty(TiXmlElement * xml, mmc::JsonValue::Value json)
{
    auto property = mmc::JsonValue::FromValue(mmc::JsonValue::Hash());
    for (auto attr = xml->FirstAttribute(); attr != nullptr; attr = attr->Next())
    { property->Insert(mmc::JsonValue::FromValue(attr->Value()), attr->Name()); }
    json->Insert(property, JSON_PROPERTY);
}

//  解析子项
void ParerChildren(TiXmlElement * xml, mmc::JsonValue::Value json)
{
    for (auto ele = xml->FirstChildElement(); ele != nullptr; ele = ele->NextSiblingElement())
    { 
        if (TiXmlNode::TINYXML_ELEMENT == ele->Type())
        {
            if (strcmp("Property",   ele->Value()) == 0 ||
                strcmp("UserString", ele->Value()) == 0)
            {
                CheckHashField(json, JSON_PROPERTY)->Insert(
                    mmc::JsonValue::FromValue(ele->Attribute("value")), 
                    ele->Attribute("key"));
            }
            else if (strcmp("Widget", ele->Value()) == 0)
            {
                CheckListField(json, JSON_CHILDREN)->Insert(ParseElement(ele));
            }
        }
    }
    CheckListField(json, JSON_CHILDREN);
}

//  解析节点
mmc::JsonValue::Value ParseElement(TiXmlElement * xml)
{
    auto json = mmc::JsonValue::FromValue(mmc::JsonValue::Hash());
    ParseProperty(xml, json);
    ParerChildren(xml, json);
    return json;
}

mmc::JsonValue::Value ParseXML(const std::string & url)
{
    TiXmlDocument doc;
    ASSERT_LOG(doc.LoadFile(url.c_str()), "输入路径错误: {0}!", url);
    return ParseElement(doc.FirstChildElement());
}

void Save(const mmc::JsonValue::Value json, const std::string & url)
{
    std::ofstream os(url);
    os << json->Print();
    os.close();
}

void Main(const std::string & xmlURL, const std::string & jsonURL)
{
    auto json = ParseXML(xmlURL);
    CustomTransfer(json->At(JSON_CHILDREN)->At(0));
    Save(json->At(JSON_CHILDREN)->At(0), jsonURL);
}

//  程序入口
int main(int argc, char **argv)
{
    //if (argc != 3)
    //{
    //    std::cout 
    //        << SFormat(
    //        "> 参数!\n"
    //        "  1: 输入路径(.xml)\n"
    //        "  2: 输出路径(.json)\n")
    //        << std::endl;
    //}
    //else
    //{
    //    Main(argv[1], argv[2]);
    //}
    Main("../../res/ui/window.layout", "../../res/ui/window.json");
    return 0;
}