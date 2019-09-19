#include "ui_parse.h"
#include "../ui_state/ui_state.h"
#include "../ui_object/ui_object.h"
#include "../../editor_sys/delegate/main/uievent_delegate_main.h"

// ...
//  UI事件委托映射表
// ...
static std::map<std::string, UIObject::UIEventDelegate *(*)()> s_DelegateMap = {
    std::make_pair("editor/editor/delegate/main/uievent_delegate_main_stage", &std::create<UIObject::UIEventDelegate, UIEventDelegateMainStage>),
    std::make_pair("editor/editor/delegate/main/uievent_delegate_main_obj_list", &std::create<UIObject::UIEventDelegate, UIEventDelegateMainObjList>),
    std::make_pair("editor/editor/delegate/main/uievent_delegate_main_res_list", &std::create<UIObject::UIEventDelegate, UIEventDelegateMainResList>),
    std::make_pair("editor/editor/delegate/main/uievent_delegate_main_com_list", &std::create<UIObject::UIEventDelegate, UIEventDelegateMainComList>),
};

UIObject * UIParser::Parse(const std::string & url)
{
    auto json = mmc::JsonValue::FromFile(url);
    ASSERT_LOG(json, "Parse: {0}", url);
    return Parse(json);
}

UIObject * UIParser::Parse(const mmc::JsonValue::Value json)
{
    auto object = CreateObject(std::stoi(json->At("__Property", "Type")->ToString()));
    ASSERT_LOG(object != nullptr, "");
    Parse__Property(json, object);
    Parse__Children(json, object);
    return object;
}

void UIParser::Parse__Property(const mmc::JsonValue::Value json, UIObject * object)
{
    for (auto ele : json->At("__Property"))
    {
        ASSERT_LOG(ele.mValue->GetType() == mmc::JsonValue::Type::kSTRING, "{0}", ele.mKey);
        if (ele.mKey == "EventDelegate")
        {
            ASSERT_LOG(s_DelegateMap.count(ele.mValue->ToString()) == 1, ele.mValue->ToString().c_str());
            object->BindDelegate(s_DelegateMap.at(ele.mValue->ToString())());
        }
        else
        {
            object->GetState()->FromStringParse(ele.mKey, ele.mValue->ToString());
        }
    }
}

void UIParser::Parse__Children(const mmc::JsonValue::Value json, UIObject * object)
{
    for (auto ele : json->At("__Children"))
    {
        object->AddObject(Parse(ele.mValue));
    }
}

UIObject * UIParser::CreateObject(const int type)
{
    switch ((UITypeEnum)type)
    {
    case UITypeEnum::kLAYOUT: return new UIClassLayout();
    case UITypeEnum::kTREEBOX: return new UIClassTreeBox();
    case UITypeEnum::kTEXTBOX: return new UIClassTextBox();
    case UITypeEnum::kIMAGEBOX: return new UIClassImageBox();
    case UITypeEnum::kCOMBOBOX: return new UIClassComboBox();
    case UITypeEnum::kUICONVAS: return new UIClassUICanvas();
    case UITypeEnum::kGLCONVAS: return new UIClassGLCanvas();
    }
    ASSERT_LOG(false, "Error Type: {0}!", type);
    return nullptr;
}
