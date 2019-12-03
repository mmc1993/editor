#include "ui_delegate_explorer.h"

bool UIDelegateExplorer::OnCallEventMessage(UIEventEnum e, const UIEvent::Event & param, const SharePtr<UIObject>& object)
{
    switch (e)
    {
    case UIEventEnum::kKey:
        break;
    case UIEventEnum::kMouse:
        break;
    case UIEventEnum::kEdit:
        break;
    case UIEventEnum::kMenu:
        break;
    case UIEventEnum::kDrag:
        break;
    case UIEventEnum::kInit:
        break;
    default:
        break;
    }
    return false;
}
