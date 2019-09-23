#pragma once

#include "../../include.h"

class Project;
class UIObject;
class GLObject;

class EditorSys {
public:
    Project * mProject;

    //  当前选中对象
    std::vector<UIObject *> mSelected;

public:
    EditorSys();

    bool OpenProjectDialog(const std::string & url);
    bool OpenProject(const std::string & url);
    bool SaveProject(const std::string & url);
    void FreeProject();
    bool IsOpenProject() const;
    //  生成名字
    std::string ObjectName(GLObject * parent) const;
    //  修改名字
    bool        ObjectName(GLObject * object, const std::string & name) const;
};