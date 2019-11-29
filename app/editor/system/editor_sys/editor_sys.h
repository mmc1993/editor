#pragma once

#include "../../include.h"
#include "project.h"

class Res;
class GLObject;
class Component;

class EditorSys {
private:
    UniquePtr<Project>              _project;
    std::vector<SharePtr<GLObject>> _selected;

public:
    /// 对象
    //  插入对象
    void OptInsertObject(SharePtr<GLObject> object, SharePtr<GLObject> parent);
    //  选中对象
    void OptSelectObject(SharePtr<GLObject> object, bool select, bool multi = false);
    void OptSelectObject(uint objectID, bool select, bool multi = false);
    //  修改名字
    void OptRenameObject(SharePtr<GLObject> object, const std::string & name);
    void OptRenameObject(uint objectID, const std::string & name);
    //  删除对象
    void OptDeleteObject(SharePtr<GLObject> object);
    void OptDeleteObject(uint objectID);
    //  修改状态
    void OptStateAddObject(SharePtr<GLObject> object, uint state);
    void OptStateAddObject(uint objectID, uint state);
    void OptStateSubObject(SharePtr<GLObject> object, uint state);
    void OptStateSubObject(uint objectID, uint state);
    void OptStateObject(SharePtr<GLObject> object, uint state);
    void OptStateObject(uint objectID, uint state);
    //  移动对象
    void OptMoveObject(SharePtr<GLObject> object, SharePtr<GLObject> target, int pos);  //  0, 1, 2 => 里面, 前面, 后面
    void OptMoveObject(uint objectID, SharePtr<GLObject> target, int pos);              //  0, 1, 2 => 里面, 前面, 后面
    void OptMoveObject(SharePtr<GLObject> object, uint targetID, int pos);              //  0, 1, 2 => 里面, 前面, 后面
    void OptMoveObject(uint objectID, uint targetID, int pos);                          //  0, 1, 2 => 里面, 前面, 后面

    /// 组件
    //  新增组件
    void OptAppendComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component);
    void OptAppendComponent(uint id, const SharePtr<Component> & component);
    //  删除组件
    void OptDeleteComponent(const SharePtr<GLObject> & object, const SharePtr<Component> & component);
    void OptDeleteComponent(uint id, const SharePtr<Component> & component);

    /// 项目
    //  新建项目
    void OptNewProject(const std::string & url);
    //  打开项目
    void OptOpenProject(const std::string & url);
    //  保存项目
    void OptSaveProject(const std::string & url);
    //  关闭项目
    void OptFreeProject();

    /// 资源
    //  删除资源
    void OptDeleteRes(uint id);
    void OptDeleteRes(Res * res);
    //  修改资源
    void OptModifyRes(uint id, const std::string & url);
    void OptModifyRes(Res * res, const std::string & url);
    //  设置资源类型
    void OptSetResType(uint id, uint type);
    void OptSetResType(Res * res, uint type);

//  工具接口
    void OpenDialogNewProject(const std::string & url);
    void OpenDialogOpenProject(const std::string & url);
    //  项目是否打开
    bool IsOpenProject();
    //  当前项目
    const UniquePtr<Project> & GetProject();
    //  生成/检查可用名字
    std::string ObjectName(const SharePtr<GLObject> & parent);
    std::string ObjectName(uint id);
    bool        ObjectName(const SharePtr<GLObject> & parent, const std::string & name);
    bool        ObjectName(uint id, const std::string & name);
    //  获取当前选中的对象
    const std::vector<SharePtr<GLObject>> & GetSelectedObjects();
};