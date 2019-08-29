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

//  控件对齐
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

//  字段替换
static const std::map<std::string, std::string> FieldMap{
    std::make_pair("Position", "Move")
};

//  值转换到字符串
static const std::map<std::string, std::map<std::string, std::string>> StringMap{
    std::make_pair("Type", std::map<std::string, std::string>
        {
            std::make_pair("Widget", "Layout"),
            std::make_pair("Layout", "Window"),
        })
};

//  值转换到枚举
static const std::map<std::string, std::map<std::string, int>> NumberMap{
    std::make_pair("Align", std::map<std::string, int>
        {
            std::make_pair("Default",               (int)UIAlignEnum::kDEFAULT),
            std::make_pair("Stretch",               (int)UIAlignEnum::kSTRETCH_H    |   (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Center",                (int)UIAlignEnum::kCENTER_H     |   (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Left Top",              (int)UIAlignEnum::kCLING_L      |   (int)UIAlignEnum::kCLING_T),
            std::make_pair("Left Bottom",           (int)UIAlignEnum::kCLING_L      |   (int)UIAlignEnum::kCLING_B),
            std::make_pair("Left Vstretch",         (int)UIAlignEnum::kCLING_L      |   (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Left Vcenter",          (int)UIAlignEnum::kCLING_L      |   (int)UIAlignEnum::kCENTER_V),
            std::make_pair("Right Top",             (int)UIAlignEnum::kCLING_R      |   (int)UIAlignEnum::kCLING_T),
            std::make_pair("Right Bottom",          (int)UIAlignEnum::kCLING_R      |   (int)UIAlignEnum::kCLING_B),
            std::make_pair("Right Vstretch",        (int)UIAlignEnum::kCLING_R      |   (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Right Vcenter",         (int)UIAlignEnum::kCLING_R      |   (int)UIAlignEnum::kCENTER_V),
            std::make_pair("Hstretch Top",          (int)UIAlignEnum::kSTRETCH_H    |   (int)UIAlignEnum::kCLING_T),
            std::make_pair("Hstretch Bottom",       (int)UIAlignEnum::kSTRETCH_H    |   (int)UIAlignEnum::kCLING_B),
            std::make_pair("Hstretch Vstretch",     (int)UIAlignEnum::kSTRETCH_H    |   (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Hstretch Vcenter",      (int)UIAlignEnum::kSTRETCH_H    |   (int)UIAlignEnum::kCENTER_V),
            std::make_pair("Hcenter Top",           (int)UIAlignEnum::kCENTER_H     |   (int)UIAlignEnum::kCLING_T),
            std::make_pair("Hcenter Bottom",        (int)UIAlignEnum::kCENTER_H     |   (int)UIAlignEnum::kCLING_B),
            std::make_pair("Hcenter Vstretch",      (int)UIAlignEnum::kCENTER_H     |   (int)UIAlignEnum::kSTRETCH_V),
            std::make_pair("Hcenter Vcenter",       (int)UIAlignEnum::kCENTER_H     |   (int)UIAlignEnum::kCENTER_V),
        }),
    std::make_pair("Type", std::map<std::string, int>
        {
            std::make_pair("Tree",      0),
            std::make_pair("Image",     1),
            std::make_pair("Button",        2),
            std::make_pair("Layout",        3),
            std::make_pair("Window",        4),
            std::make_pair("EditBox",       5),
            std::make_pair("TextBox",       6),
            std::make_pair("ComboBox",      7),
            std::make_pair("UICanvas",      8),
            std::make_pair("GLCanvas",      9),
        })
};

bool HasKey(const std::map<std::string, std::map<std::string, std::string>> & m, 
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

bool HasKey(const std::map<std::string, std::string> & m, 
            const std::string & k)
{
    return m.find(k) != m.end();
}

bool HasKey(const std::map<std::string, std::map<std::string, int>> & m,
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

const std::string & CheckString(
    const std::map<std::string, std::map<std::string, std::string>> & m, 
    const std::string & k0, 
    const std::string & k1)
{
    return m.find(k0)->second.find(k1)->second;
}

const std::string & CheckString(
    const std::map<std::string,std::string> & m, 
    const std::string & k0)
{
    return m.find(k0)->second;
}

std::string CheckNumber(
    const std::map<std::string, std::map<std::string, int>> & m,
    const std::string & k0, 
    const std::string & k1)
{
    return std::to_string(m.find(k0)->second.find(k1)->second);
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
        if (HasKey(FieldMap, ele.mKey))
        {
            ele.mKey = CheckString(FieldMap, ele.mKey);
        }
        if (HasKey(StringMap, ele.mKey, ele.mValue->ToString()))
        {
            ele.mValue->Set(CheckString(StringMap, ele.mKey, ele.mValue->ToString()));
        }
        if (HasKey(NumberMap, ele.mKey, ele.mValue->ToString()))
        {
            ele.mValue->Set(CheckNumber(NumberMap, ele.mKey, ele.mValue->ToString()));
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
    CustomTransfer(json);
    Save(json, jsonURL);
}

//  程序入口
int main(int argc, char **argv)
{
    if (argc != 3)
    {
        std::cout 
            << SFormat(
            "> 参数!\n"
            "  1: 输入路径(.xml)\n"
            "  2: 输出路径(.json)\n")
            << std::endl;
    }
    else
    {
        Main(argv[1], argv[2]);
    }
    //Main("test.layout", "test.json");
    return 0;
}