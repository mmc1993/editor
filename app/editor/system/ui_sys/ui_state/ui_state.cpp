#include "ui_state.h"
#include "../../interface/serializer.h"

bool UIState::FromStringParse(const std::string & key, const std::string & val)
{
    //  int
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kINT, key, val, " ", "Align", Align);
    //  bool
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsMulti", IsMulti);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsSelect", IsSelect);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsWindow", IsWindow);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsButton", IsButton);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsShowNav", IsShowNav);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsEditBox", IsEditBox);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsCanMove", IsCanMove);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsCanStretch", IsCanStretch);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsFullScreen", IsFullScreen);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsShowBorder", IsShowBorder);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsCanDragMove", IsCanDragMove);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsCanDragFree", IsCanDragFree);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsShowMenuBar", IsShowMenuBar);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsShowTitleBar", IsShowTitleBar);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBOOL, key, val, " ", "IsShowScrollBar", IsShowScrollBar);
    //  float
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kFLOAT, key, val, " ", "BorderNumber", BorderNumber);
    //  string
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kSTRING, key, val, " ", "Name", Name);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kSTRING, key, val, " ", "Tips", Tips);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kSTRING, key, val, " ", "LSkin", LSkin);
    //  vec2
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kVEC2, key, val, " ", "StretchMin", StretchMin);
    //  vec4
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kVEC4, key, val, " ", "Move", Move);
    //  string list
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kSTRING_LIST, key, val, ",", "MenuBar", MenuBar);
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
{
    glGenTextures(2, &mRenderTextures[0]);
    glGenFramebuffers(1,  &mRenderTarget);
}

UIStateGLCanvas::~UIStateGLCanvas()
{
    glDeleteTextures(2, &mRenderTextures[0]);
    glDeleteFramebuffers(1,  &mRenderTarget);
}
