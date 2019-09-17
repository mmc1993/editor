#include "ui_state.h"
#include "../../tools/parser_tool.h"

void UIState::Parse(const std::string & key, const std::string & val)
{
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kINT, key, val, Align, Align);

    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsMulti, IsMulti);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsWindow, IsWindow);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsButton, IsButton);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsShowNav, IsShowNav);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsEditBox, IsEditBox);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsCanMove, IsCanMove);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsCanStretch, IsCanStretch);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsFullScreen, IsFullScreen);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsShowBorder, IsShowBorder);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsShowMenuBar, IsShowMenuBar);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsShowTitleBar, IsShowTitleBar);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kBOOL, key, val, IsShowScrollBar, IsShowScrollBar);

    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kFLOAT, key, val, BorderNumber, BorderNumber);

    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kSTRING, key, val, Name, Name);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kSTRING, key, val, Tips, Tips);
    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kSTRING, key, val, LSkin, LSkin);

    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kVEC2, key, val, StretchMin, StretchMin);

    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kVEC4, key, val, Move, Move);

    PARSER_REG_MEMBER(tools::ValueParser::TypeEnum::kSTRING_LIST, key, val, MenuBar, MenuBar);
}

UIStateLayout::UIStateLayout()
{ }

UIStateTreeBox::UIStateTreeBox(): mSelect(false)
{ }

UIStateTextBox::UIStateTextBox(): mBuffer(1024, '\0')
{ }

UIStateImageBox::UIStateImageBox()
{ }

UIStateComboBox::UIStateComboBox()
{ }

UIStateUICanvas::UIStateUICanvas()
{ }

UIStateGLCanvas::UIStateGLCanvas()
{ }
