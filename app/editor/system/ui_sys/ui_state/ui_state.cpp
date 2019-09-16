#include "ui_state.h"

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
