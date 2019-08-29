#include "ui_frame.h"
#include "ui_parse/ui_parse.h"
#include "ui_state/ui_state.h"
#include "ui_class/ui_class.h"

UIFrame::UIFrame(): _uiRoot(nullptr)
{
}

UIFrame::~UIFrame()
{
    SetRoot(nullptr);
}

UIClass * UIFrame::GetRoot()
{
    return _uiRoot;
}

void UIFrame::SetRoot(UIClass * uiClass)
{
    SAFE_DELETE(_uiRoot);
    _uiRoot = uiClass;
}

void UIFrame::SetRoot(const std::string & url)
{
     SetRoot(UIParser::Parse(url));
     _uiRoot->ResetLayout();
}

void UIFrame::Update(float dt)
{
    _uiRoot->Update(dt);
    _uiRoot->Render(dt);
}

