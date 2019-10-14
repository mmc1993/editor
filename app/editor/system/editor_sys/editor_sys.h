#pragma once

#include "../../include.h"
#include "project.h"

class UIObject;
class GLObject;
class Component;

class EditorSys {
private:
    UniquePtr<Project>              _project;
    std::vector<SharePtr<UIObject>> _selected;

public:
//  操作原始接口
    /// 对象
    //  插入对象
    void OptInsertObject(const SharePtr<UIObject> & uiObject, const SharePtr<UIObject> & parentUIObject);
    //  选中对象
    void OptSelectObject(const SharePtr<UIObject> & uiObject, bool select, bool multi = false);
    //  修改名字
    void OptRenameObject(const SharePtr<UIObject> & uiObject, const std::string & name);
    //  删除对象
    void OptDeleteObject(const SharePtr<UIObject> & uiObject);

    /// 组件
    //  新增组件
    void OptAppendComponent(const SharePtr<UIObject> & uiObject, const SharePtr<Component> & component);
    //  删除组件
    void OptDeleteComponent(const SharePtr<UIObject> & uiObject, const SharePtr<Component> & component);

    /// 项目
    //  新建项目
    void OptNewProject(const std::string & url);
    //  打开项目
    void OptOpenProject(const std::string & url);
    //  保存项目
    void OptSaveProject(const std::string & url);
    //  关闭项目
    void OptFreeProject();

//  工具接口
    void OpenDialogNewProject(const std::string & url);
    bool OpenDialogOpenProject(const std::string & url);
    //  项目是否打开
    bool IsOpenProject() const;
    //  生成/检查可用名字
    std::string ObjectName(const SharePtr<GLObject> & parent) const;
    std::string ObjectName(const SharePtr<UIObject> & parent) const;
    bool        ObjectName(const SharePtr<UIObject> & parent, const std::string & name) const;
    //  获取当前选中的对象
    const std::vector<SharePtr<UIObject>> & GetSelectedObjects() const;
    //  当前项目
    const UniquePtr<Project> & GetProject();
};