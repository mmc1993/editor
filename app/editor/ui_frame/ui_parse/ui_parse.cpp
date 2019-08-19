#include "ui_parse.h"
#include "../ui_state/ui_state.h"
#include "../ui_class/ui_class.h"

static const std::unordered_map<std::string, UIClass * (*)(const TiXmlElement *)> PARSER_FUNCs = {
    std::make_pair(WEIGET_CONTAINER,     UIParserDDContainer::Parse),
    std::make_pair(WEIGET_BUTTON,        UIParserButton::Parse),
};

UIClass * UIParser::Parse(const std::string & url)
{
    TiXmlDocument xml;
    ASSERT_LOG(xml.LoadFile(url.c_str()), "Parse: {0}", url);
    return Parse(xml.FirstChildElement());
}

UIClass * UIParser::Parse(const TiXmlElement * xml)
{
    ASSERT_LOG(xml != nullptr,                       "Parse Error.");
    ASSERT_LOG((strcmp(xml->Value(), "MyGUI") == 0), "Parse Error.");
    return Parse(xml, new UIClassWindow(new UIStateWindow()));
}

UIClass * UIParser::Parse(const TiXmlElement * xml, UIClass * parent)
{
    for (auto ele = xml->FirstChildElement("Widget"); ele != nullptr;
              ele = ele->NextSiblingElement("Widget"))
    {
        ASSERT_LOG(PARSER_FUNCs.find(ele->Attribute("type")) != PARSER_FUNCs.end(),
                   "ParseChildren Error: {0}", ele->Attribute("type"));
        parent->AddChild(PARSER_FUNCs.at(ele->Attribute("type"))(ele));
        Parse(ele, parent);
    }
    return parent;
}

void UIParser::ParseAttr(const TiXmlElement * xml, UIState * state)
{
    //  对齐方式
    static const char * ALIGNMENT[] = {
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
    };

    //  方位
    auto val = xml->Attribute("position");
    ASSERT_LOG(val != nullptr, "Not Position");
    auto pos = string_tool::Split(val, " ");
    state->mMove.x = std::stof(pos.at(0));
    state->mMove.y = std::stof(pos.at(1));
    state->mMove.z = std::stof(pos.at(2));
    state->mMove.w = std::stof(pos.at(3));

    //  名字
    val = xml->Attribute("name");
    ASSERT_LOG(val != nullptr, "Not Name");
    state->mName = val;

    //  对齐
    val = xml->Attribute("align");
    ASSERT_LOG(val != nullptr, "Not Alignment");
    state->mAlignment.mOrigin = state->mMove;
    auto it = std::find(std::begin(ALIGNMENT), std::end(ALIGNMENT), std::string(val));
    ASSERT_LOG(it != std::end(ALIGNMENT), "Not Alignment");
    state->mAlignment.mType = UIState::Alignment::TypeEnum(
                 std::distance(std::begin(ALIGNMENT), it));

    //  皮肤
    val = xml->Attribute("skin");
    if (val != nullptr)
    {
        //  MMC TODO:
    }
}

void UIParser::ParseProp(const TiXmlElement * xml, UIState * state)
{
    for (auto ele = xml->FirstChildElement("Property"); ele != nullptr;
              ele = ele->NextSiblingElement("Property"))
    {
        auto key = ele->Attribute("key");
        if (strcmp(key, "Alpha") == 0)
        {
            state->mColor.a = std::stof(ele->Attribute("value"));
        }
        else if (strcmp(key, "Colour") == 0)
        {
            auto color = string_tool::Split(ele->Attribute("value"), " ");
            state->mColor.r = std::stof(color.at(0));
            state->mColor.g = std::stof(color.at(1));
            state->mColor.b = std::stof(color.at(2));
        }
        else if (strcmp(key, "Enabled") == 0)
        {
            state->mEnabled = strcmp("true", ele->Attribute("value")) == 0;
        }
        else if (strcmp(key, "NeedKey") == 0)
        {
            state->mEnabledKey = strcmp("true", ele->Attribute("value")) == 0;
        }
        else if (strcmp(key, "NeedMouse") == 0)
        {
            state->mEnabledMouse = strcmp("true", ele->Attribute("value")) == 0;
        }
        else if (strcmp(key, "Visible") == 0)
        {
            state->mVisible = strcmp("true", ele->Attribute("value")) == 0;
        }
    }
}

void UIParser::ParseUser(const TiXmlElement * xml, UIState * state)
{
    for (auto ele = xml->FirstChildElement("UserString"); ele != nullptr;
              ele = ele->NextSiblingElement("UserString"))
    {
        state->mUserData.emplace(ele->Attribute("key"), ele->Attribute("value"));
    }
}

UIClass * UIParserDDContainer::Parse(const TiXmlElement * xml)
{
    auto state = new UIStateDDContainer();
    UIParser::ParseAttr(xml, state);
    UIParser::ParseProp(xml, state);
    return new UIClassDDContainer(state);
}

UIClass * UIParserButton::Parse(const TiXmlElement * xml)
{
    auto state = new UIStateButton();
    UIParser::ParseAttr(xml, state);
    UIParser::ParseProp(xml, state);
    return new UIClassButton(state);
}

