#pragma once

#include "../../include.h"

class Project;
class UIObject;
class GLObject;
class Component;

class EditorSys {
private:
    Project *               _project;
    std::vector<UIObject *> _selected;

public:
    EditorSys();

//  操作原始接口
    /// 对象
    //  选中对象
    void OptSelectObject(UIObject * uiObject, bool select, bool multi = false);
    //  删除对象
    void OptDeleteObject(UIObject * uiObject);
    //  插入对象
    void OptInsertObject(UIObject * uiObject, UIObject * insUIObject);
    //  修改名字
    void OptRenameObject(UIObject * uiObject, const std::string & name);

    /// 组件
    //  新增组件
    void OptAppendComponent(UIObject * uiObject, Component * component);
    //  删除组件
    void OptDeleteComponent(UIObject * uiObject, Component * component);

    /// 项目
    //  新建项目
    void OptNewProject(const std::string & url);
    //  打开项目
    bool OptOpenProject(const std::string & url);
    //  保存项目
    bool OptSaveProject(const std::string & url);
    //  关闭项目
    void OptFreeProject();

//  工具接口
    void OpenDialogNewProject(const std::string & url);
    bool OpenDialogOpenProject(const std::string & url);
    //  项目是否打开
    bool IsOpenProject() const;
    //  生成可用名字
    std::string ObjectName(GLObject * parent) const;
    std::string ObjectName(UIObject * parent) const;
    //  检查可用名字
    bool        ObjectName(UIObject * parent, const std::string & name) const;
    //  获取当前选中的对象
    const std::vector<UIObject *> & GetSelectedObjects() const;
    //  当前项目
    Project * GetProject();
};