#include "ui_state.h"

UIStateTree::UIStateTree()
{ }

UIStateImage::UIStateImage()
{ }

UIStateLayout::UIStateLayout()
{ }

UIStateTextBox::UIStateTextBox(): mBuffer(1024, '\0')
{ }

UIStateComboBox::UIStateComboBox()
{ }

UIStateUICanvas::UIStateUICanvas()
{ }

UIStateGLCanvas::UIStateGLCanvas()
{ }
