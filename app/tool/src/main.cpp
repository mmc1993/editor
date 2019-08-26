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

static const std::map<std::string, std::map<std::string, std::string>> StringMap{
    std::make_pair("type", std::map<std::string, std::string>
        {
            std::make_pair("Widget", "Layout")
        })
};

static const std::map<std::string, std::vector<std::string>> NumberMap{
    std::make_pair("align", std::vector<std::string>
        {
            "Default",
            "Stretch",
            "Center",
            "Left Top",
            "Left Bottom",
            "Left Vstretch",
            "Left Vcenter",
            "Right Top",
            "Right Bottom",
            "Right Vstretch",
            "Right Vcenter",
            "Hstretch Top",
            "Hstretch Bottom",
            "Hstretch Vstretch",
            "Hstretch Vcenter",
            "Hcenter Top",
            "Hcenter Bottom",
            "Hcenter Vstretch",
            "Hcenter Vcenter",
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

bool HasKey(const std::map<std::string, std::vector<std::string>> & m, 
            const std::string & k0)
{
    auto it0 = m.find(k0);
    return it0 != m.end();
}

const std::string & CheckString(
    const std::map<std::string, std::map<std::string, std::string>> & m, 
    const std::string & k0, 
    const std::string & k1)
{
    return m.find(k0)->second.find(k1)->second;
}

std::string CheckNumber(
    const std::map<std::string, std::vector<std::string>> & m,
    const std::string & k0, 
    const std::string & k1)
{
    auto & a = m.find(k0)->second;
    auto it = std::find(a.begin(), a.end(), k1);
    auto i = std::distance(a.begin(), it);
    return std::to_string(i);
}

void CustomTransfer(mmc::JsonValue::Value json)
{
    for (auto & ele : json->At(JSON_PROPERTY))
    {
        if (HasKey(StringMap, ele.mKey, ele.mValue->ToString()))
        {
            ele.mValue->Set(CheckString(StringMap, ele.mKey, ele.mValue->ToString()));
        }
        if (HasKey(NumberMap, ele.mKey))
        {
            ele.mValue->Set(CheckNumber(NumberMap, ele.mKey, ele.mValue->ToString()));
        }
        if ('a' <= ele.mKey.at(0) && 'z' >= ele.mKey.at(0))
        {
            ele.mKey.at(0) -= 'a';
            ele.mKey.at(0) += 'A';
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