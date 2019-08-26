#include "ui_parse.h"
#include "../ui_state/ui_state.h"
#include "../ui_class/ui_class.h"

UIClass * UIParser::Parse(const std::string & url)
{
    auto json = mmc::JsonValue::FromFile(url);
    ASSERT_LOG(json, "Parse: {0}", url);
    return Parse("MyGUI", json);
}

UIClass * UIParser::Parse(const std::string & tag, const mmc::JsonValue::Value json)
{
    ASSERT_LOG(tag == "MyGUI" || tag == "Weiget", "{0}", tag);
    auto object = CreateObject(tag == "MyGUI"
                             ? (UITypeEnum::kWINDOW)
                             : (UITypeEnum)json->ToInt());
    ASSERT_LOG(object != nullptr, "");
    Parse__Property(json, object);
    Parse__Children(json, object);
    return object;
}

void UIParser::Parse__Property(const mmc::JsonValue::Value json, UIClass * object)
{
    ParseCustomData(json->At("__Property"), object->GetState<UIState>());
}

void UIParser::Parse__Children(const mmc::JsonValue::Value json, UIClass * object)
{
    for (auto ele : json->At("__Children"))
    {
        if (ele.mKey == "Property" || ele.mKey == "Userdata")
        {
            ParseCustomData(ele.mValue, object->GetState<UIState>());
        }
        else if (ele.mKey == "Weiget")
        {
            for (auto val : ele.mValue)
            {
                object->AddChild(Parse(ele.mKey, val.mValue));
            }
        }
    }
}

void UIParser::ParseCustomData(const mmc::JsonValue::Value json, UIState & state)
{
    for (auto ele : json)
    {
        ASSERT_LOG(ele.mValue->GetType() == mmc::JsonValue::Type::kSTRING, "{0}", ele.mKey);
        SET_DATA(state.mData, ele.mKey, ele.mValue->ToString());
    }
}

UIClass * UIParser::CreateObject(UITypeEnum type)
{
    switch (type)
    {
    case UITypeEnum::kTREE:
        break;
    case UITypeEnum::kIMAGE:
        break;
    case UITypeEnum::kBUTTON:
        break;
    case UITypeEnum::kLAYOUT:
        break;
    case UITypeEnum::kWINDOW:
        return new UIClassWindow(new UIStateWindow());
    case UITypeEnum::kEDITBOX:
        break;
    case UITypeEnum::kTEXTBOX:
        break;
    case UITypeEnum::kCOMBOBOX:
        break;
    case UITypeEnum::kUICONVAS:
        break;
    case UITypeEnum::kGLCONVAS:
        break;
    default:
        break;
    }
    ASSERT_LOG(false, "Error Type: {0}!", (int)type);
    return nullptr;
}




















//void UIParser::ParseAttr(const mmc::JsonValue::Value json, UIState * state)
//{
//    //  对齐方式
//    static const char * ALIGNMENT[] = {
//        "Default",
//        "Stretch",
//        "Center",
//        "Left Top",
//        "Left Bottom",
//        "Left Vstretch",
//        "Left Vcenter",
//        "Right Top",
//        "Right Bottom",
//        "Right Vstretch",
//        "Right Vcenter",
//        "Hstretch Top",
//        "Hstretch Bottom",
//        "Hstretch Vstretch",
//        "Hstretch Vcenter",
//        "Hcenter Top",
//        "Hcenter Bottom",
//        "Hcenter Vstretch",
//        "Hcenter Vcenter",
//    };
//
//    //  方位
//    auto val = xml->Attribute("position");
//    ASSERT_LOG(val != nullptr, "Not Position");
//    auto pos = string_tool::Split(val, " ");
//    state->mMove.x = std::stof(pos.at(0));
//    state->mMove.y = std::stof(pos.at(1));
//    state->mMove.z = std::stof(pos.at(2));
//    state->mMove.w = std::stof(pos.at(3));
//
//    //  名字
//    val = xml->Attribute("name");
//    ASSERT_LOG(val != nullptr, "Not Name");
//    state->mName = val;
//
//    //  对齐
//    val = xml->Attribute("align");
//    ASSERT_LOG(val != nullptr, "Not Alignment");
//    state->mAlignment.mOrigin = state->mMove;
//    auto it = std::find(std::begin(ALIGNMENT), std::end(ALIGNMENT), std::string(val));
//    ASSERT_LOG(it != std::end(ALIGNMENT), "Not Alignment");
//    state->mAlignment.mType = UIState::Alignment::TypeEnum(
//                 std::distance(std::begin(ALIGNMENT), it));
//}
//
//void UIParser::ParseProp(const mmc::JsonValue::Value json, UIState * statee)
//{
//    for (auto ele = xml->FirstChildElement("Property"); ele != nullptr;
//              ele = ele->NextSiblingElement("Property"))
//    {
//        auto key = ele->Attribute("key");
//        if (strcmp(key, "Alpha") == 0)
//        {
//            state->mColor.a = std::stof(ele->Attribute("value"));
//        }
//        else if (strcmp(key, "Colour") == 0)
//        {
//            auto color = string_tool::Split(ele->Attribute("value"), " ");
//            state->mColor.r = std::stof(color.at(0));
//            state->mColor.g = std::stof(color.at(1));
//            state->mColor.b = std::stof(color.at(2));
//        }
//        else if (strcmp(key, "Enabled") == 0)
//        {
//            state->mEnabled = strcmp("true", ele->Attribute("value")) == 0;
//        }
//        else if (strcmp(key, "NeedKey") == 0)
//        {
//            state->mEnabledKey = strcmp("true", ele->Attribute("value")) == 0;
//        }
//        else if (strcmp(key, "NeedMouse") == 0)
//        {
//            state->mEnabledMouse = strcmp("true", ele->Attribute("value")) == 0;
//        }
//        else if (strcmp(key, "Visible") == 0)
//        {
//            state->mVisible = strcmp("true", ele->Attribute("value")) == 0;
//        }
//    }
//}
//
//void UIParser::ParseUser(const mmc::JsonValue::Value json, UIState * state)
//{
//    for (auto ele = xml->FirstChildElement("UserString"); ele != nullptr;
//              ele = ele->NextSiblingElement("UserString"))
//    {
//        state->mUserData.emplace(ele->Attribute("key"), ele->Attribute("value"));
//    }
//}
