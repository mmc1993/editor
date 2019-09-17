#pragma once

#include "../../include.h"
#include "component/gl_object.h"
#include "component/component.h"

class EditorSys {
    //  数据区
public:
    //  根对象
    GLObject * mRootObject;
    //  当前选中对象
    std::vector<GLObject *> mSelected;

    //  接口区
public:
    EditorSys();
    //  生成名字
    std::string GenerateObjectName(GLObject * object) const;
    //  检查名字
    bool CheckRename(GLObject * object, const std::string & name) const;
};