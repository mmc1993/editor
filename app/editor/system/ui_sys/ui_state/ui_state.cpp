#include "ui_state.h"
#include "../ui_parse/ui_parse.h"

bool UIState::FromStringParse(const std::string & key, const std::string & val)
{
    //  int
    if (key == "Align")             return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kInt, val, " ", &Align);
    //  bool
    if (key == "IsMulti")           return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsMulti);
    if (key == "IsModel")           return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsModel);
    if (key == "IsSelect")          return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsSelect);
    if (key == "IsWindow")          return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsWindow);
    if (key == "IsButton")          return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsButton);
    if (key == "IsShowNav")         return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsShowNav);
    if (key == "IsEditBox")         return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsEditBox);
    if (key == "IsCanMove")         return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsCanMove);
    if (key == "IsSameline")        return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsSameline);
    if (key == "IsCanStretch")      return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsCanStretch);
    if (key == "IsFullScreen")      return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsFullScreen);
    if (key == "IsEditReturn")      return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsEditReturn);
    if (key == "IsShowBorder")      return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsShowBorder);
    if (key == "IsCanDragMove")     return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsCanDragMove);
    if (key == "IsCanDragFree")     return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsCanDragFree);
    if (key == "IsShowMenuBar")     return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsShowMenuBar);
    if (key == "IsShowTitleBar")    return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsShowTitleBar);
    if (key == "IsShowScrollBar")   return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kBool, val, " ", &IsShowScrollBar);
    //  float
    if (key == "BorderNumber")      return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kFloat, val, " ", &BorderNumber);
    //  string
    if (key == "Name")              return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kString, val, " ", &Name);
    if (key == "Tips")              return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kString, val, " ", &Tips);
    if (key == "LSkin")             return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kString, val, " ", &LSkin);
    //  vec2
    if (key == "StretchMin")        return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kVector2, val, " ", &StretchMin);
    //  vec4
    if (key == "Move")              return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kVector4, val, " ", &Move);
    if (key == "Color")             return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kVector4, val, " ", &Color);
    //  string list
    if (key == "MenuBar")           return UIParser::FromStringParse(UIParser::StringValueTypeEnum::kStringList, val, ",", &MenuBar);
    return false;
}

UIStateLayout::UIStateLayout()
{ }

UIStateTreeBox::UIStateTreeBox()
{ }

UIStateTextBox::UIStateTextBox()
{ }

UIStateImageBox::UIStateImageBox()
{ }

UIStateComboBox::UIStateComboBox()
{ }

UIStateUICanvas::UIStateUICanvas()
{ }

UIStateGLCanvas::UIStateGLCanvas()
    : mTargetCommandStack(0)
{
    glGenFramebuffers(1,  &mRenderTarget);
    mOperation.mViewScale = 3;
}

UIStateGLCanvas::~UIStateGLCanvas()
{
    glDeleteFramebuffers(1,  &mRenderTarget);
}
