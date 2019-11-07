#include "ui_state.h"
#include "../../interface/serializer.h"

bool UIState::FromStringParse(const std::string & key, const std::string & val)
{
    //  int
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kInt, key, val, " ", "Align", Align);
    //  bool
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsMulti", IsMulti);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsSelect", IsSelect);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsWindow", IsWindow);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsButton", IsButton);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsShowNav", IsShowNav);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsEditBox", IsEditBox);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsCanMove", IsCanMove);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsSameline", IsSameline);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsCanStretch", IsCanStretch);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsFullScreen", IsFullScreen);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsShowBorder", IsShowBorder);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsCanDragMove", IsCanDragMove);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsCanDragFree", IsCanDragFree);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsShowMenuBar", IsShowMenuBar);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsShowTitleBar", IsShowTitleBar);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kBool, key, val, " ", "IsShowScrollBar", IsShowScrollBar);
    //  float
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kFloat, key, val, " ", "BorderNumber", BorderNumber);
    //  string
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kString, key, val, " ", "Name", Name);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kString, key, val, " ", "Tips", Tips);
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kString, key, val, " ", "LSkin", LSkin);
    //  vec2
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kVector2, key, val, " ", "StretchMin", StretchMin);
    //  vec4
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kVector4, key, val, " ", "Move", Move);
    //  string list
    FROM_STRING_PARSE_VARIABLE(interface::Serializer::StringValueTypeEnum::kStringList, key, val, ",", "MenuBar", MenuBar);
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
