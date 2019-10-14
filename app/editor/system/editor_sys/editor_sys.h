#pragma once

#include "../../include.h"
#include "project.h"

class GLObject;
class Component;

class EditorSys {
private:
    UniquePtr<Project>              _project;
    std::vector<SharePtr<GLObject>> _selected;

public:
    /// 对象
    //  插入对象
    void OptInsertObject(const SharePtr<GLObject> & object, const SharePtr<GLObject> & parent);
    //  选中对象
    void OptSelectObject(const SharePtr<GLObject> & object, bool select, bool multi = false);
    //  修改名字
    void OptRenameObject(const SharePtr<GLObject> & object, const std::string & name);
    //  删除对象
    void OptDeleteObject(const SharePtr<GLObject> & object);

    /// 组件
    //  新增组件
    void OptAppendComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component);
    //  删除组件
    void OptDeleteComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component);

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
    void OpenDialogOpenProject(const std::string & url);
    //  项目是否打开
    bool IsOpenProject() const;
    //  生成/检查可用名字
    std::string ObjectName(const SharePtr<GLObject> & parent) const;
    bool        ObjectName(const SharePtr<GLObject> & parent, const std::string & name) const;
    //  获取当前选中的对象
    const std::vector<SharePtr<GLObject>> & GetSelectedObjects() const;
    //  当前项目
    const UniquePtr<Project> & GetProject();
};