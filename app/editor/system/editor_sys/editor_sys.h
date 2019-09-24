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
    void OptMetaSelectObject(UIObject * uiObject, bool select, bool multi = false);
    //  删除对象
    void OptMetaDeleteObject(UIObject * uiObject);
    //  插入对象
    void OptMetaInsertObject(UIObject * uiObject, UIObject * insUIObject);
    //  修改名字
    void OptMetaRenameObject(UIObject * uiObject, const std::string & name);

    /// 组件
    //  新增组件
    void OptMetaAppendComponent(UIObject * uiObject, Component * component);
    //  删除组件
    void OptMetaDeleteComponent(UIObject * uiObject, Component * component);

    /// 项目
    //  新建项目
    void OptMetaNewProject(const std::string & url);
    //  打开项目
    bool OptMetaOpenProject(const std::string & url);
    //  保存项目
    bool OptMetaSaveProject(const std::string & url);
    //  关闭项目
    void OptMetaFreeProject();

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