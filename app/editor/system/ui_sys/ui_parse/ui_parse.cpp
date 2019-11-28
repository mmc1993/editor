#include "ui_parse.h"
#include "../ui_object/ui_object.h"
#include "../../editor_sys/delegate/main/uievent_delegate_main.h"

// ...
//  UI事件委托映射表
// ...
static std::map<std::string, UIEvent::DelegateHandler *(*)()> s_DelegateMap = {
    std::make_pair("editor/editor/delegate/main/uievent_delegate_main_stage", []() { return (UIEvent::DelegateHandler *)new UIEventDelegateMainStage(); }),
    std::make_pair("editor/editor/delegate/main/uievent_delegate_main_global", []() { return (UIEvent::DelegateHandler *)new UIEventDelegateMainGlobal(); }),
    std::make_pair("editor/editor/delegate/main/uievent_delegate_main_obj_list", []() { return (UIEvent::DelegateHandler *)new UIEventDelegateMainObjList(); }),
    std::make_pair("editor/editor/delegate/main/uievent_delegate_main_res_list", []() { return (UIEvent::DelegateHandler *)new UIEventDelegateMainResList(); }),
    std::make_pair("editor/editor/delegate/main/uievent_delegate_main_com_list", []() { return (UIEvent::DelegateHandler *)new UIEventDelegateMainComList(); }),
};

SharePtr<UIObject> UIParser::CreateObject(const int type)
{
    switch ((UITypeEnum)type)
    {
    case UITypeEnum::kOther: return std::create_ptr<UIObject>();
    case UITypeEnum::kLayout: return std::create_ptr<UIObjectLayout>();
    case UITypeEnum::kTreeBox: return std::create_ptr<UIObjectTreeBox>();
    case UITypeEnum::kTextBox: return std::create_ptr<UIObjectTextBox>();
    case UITypeEnum::kImageBox: return std::create_ptr<UIObjectImageBox>();
    case UITypeEnum::kComboBox: return std::create_ptr<UIObjectComboBox>();
    case UITypeEnum::kUICanvas: return std::create_ptr<UIObjectUICanvas>();
    case UITypeEnum::kGLCanvas: return std::create_ptr<UIObjectGLCanvas>();
    }
    ASSERT_LOG(false, "Error Type: {0}!", type);
    return nullptr;
}

SharePtr<UIObject> UIParser::Parse(const std::string & url)
{
    auto json = mmc::Json::FromFile(url);
    ASSERT_LOG(json, "Parse: {0}", url);
    return Parse(json);
}

SharePtr<UIObject> UIParser::Parse(const mmc::Json::Pointer json)
{
    auto object = CreateObject(std::stoi(json->At("__Property", "Type")->ToString()));
    ASSERT_LOG(object != nullptr, "");
    Parse__Property(json, object);
    Parse__Children(json, object);
    return object;
}

void UIParser::Parse__Property(const mmc::Json::Pointer json, const SharePtr<UIObject> & object)
{
    for (auto ele : json->At("__Property"))
    {
        ASSERT_LOG(ele.mVal->GetType() == mmc::Json::Type::kSTRING, "{0}", ele.mKey);
        if (ele.mKey == "EventDelegate")
        {
            ASSERT_LOG(s_DelegateMap.count(ele.mVal->ToString()) == 1, ele.mVal->ToString().c_str());
            object->BindDelegate(s_DelegateMap.at(ele.mVal->ToString())());
        }
        else
        {
            object->GetState()->FromStringParse(ele.mKey, ele.mVal->ToString());
        }
    }
}

void UIParser::Parse__Children(const mmc::Json::Pointer json, const SharePtr<UIObject> & object)
{
    for (auto ele : json->At("__Children"))
    {
        object->InsertObject(Parse(ele.mVal));
    }
}
