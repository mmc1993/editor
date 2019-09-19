ok 窗口菜单
ok 弹出菜单
ok 拉伸方向判断


对象
组件


图层
对象:
    组件0
    组件1
    组件2



Project {
    std::vector<UIOBject *> mUIObjects;
    std::vector<GLOBject *> mGLObjects;
    std::map<void *, size_t> mObj2Objs;

    GLOBject * GetObject(const UIOBject * object);
    UIOBject * GetObject(const GLOBject * object);
    const std::string & GetPath() const;
    bool Load();
    void Free();
}