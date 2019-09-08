#include "ui_parse.h"
#include "../ui_state/ui_state.h"
#include "../ui_class/ui_class.h"

UIClass * UIParser::Parse(const std::string & url)
{
    auto json = mmc::JsonValue::FromFile(url);
    ASSERT_LOG(json, "Parse: {0}", url);
    return Parse(json);
}

UIClass * UIParser::Parse(const mmc::JsonValue::Value json)
{
    auto object = CreateObject(std::stoi(json->At("__Property", "Type")->ToString()));
    ASSERT_LOG(object != nullptr, "");
    Parse__Property(json, object);
    Parse__Children(json, object);
    return object;
}

void UIParser::Parse__Property(const mmc::JsonValue::Value json, UIClass * object)
{
    for (auto ele : json->At("__Property"))
    {
        ASSERT_LOG(ele.mValue->GetType() == mmc::JsonValue::Type::kSTRING, "{0}", ele.mKey);
        ParseUIData(object->GetState()->mData, ele.mKey, ele.mValue->ToString());
    }
}

void UIParser::Parse__Children(const mmc::JsonValue::Value json, UIClass * object)
{
    for (auto ele : json->At("__Children"))
    {
        object->AddChild(Parse(ele.mValue));
    }
}

UIClass * UIParser::CreateObject(const int type)
{
    switch ((UITypeEnum)type)
    {
    case UITypeEnum::kTREE: return new UIClassTree(new UIStateTree());
    case UITypeEnum::kIMAGE: return new UIClassImage(new UIStateImage());
    case UITypeEnum::kLAYOUT: return new UIClassLayout(new UIStateLayout());
    case UITypeEnum::kTEXTBOX: return new UIClassTextBox(new UIStateTextBox());
    case UITypeEnum::kCOMBOBOX: return new UIClassComboBox(new UIStateComboBox());
    case UITypeEnum::kUICONVAS: break;
    case UITypeEnum::kGLCONVAS: break;
    }
    ASSERT_LOG(false, "Error Type: {0}!", type);
    return nullptr;
}
