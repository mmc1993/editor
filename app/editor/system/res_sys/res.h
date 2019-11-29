#pragma once

#include "../../include.h"
#include "../interface/serializer.h"

class Res {
public:
    enum TypeEnum {
        kNull,      //  无
        kTxt,       //  文本
        kImg,       //  图片
        kMap,       //  地图
        kFont,      //  字体
        kObj,       //  对象
        kVar,       //  变量
        kBlueprint, //  蓝图
    };

    class Ref {
    public:
        Ref(Res * owner);
        ~Ref();

        Ref * Clone();
        bool Modify();
        bool Modify(bool modify);

        template <class T>
        T Instance()
        {
            ASSERT_LOG(_owner != nullptr, "");
            Modify(false);
            auto instance = _owner->Instance();
            return  std::any_cast<T>(instance);
        }

    private:
        Ref(const Ref & other) = delete;
        Ref & operator=(const Ref & other) = delete;

    private:
        Res * _owner;
        bool  _modify;
    };

public:
    Res(uint id);
    ~Res();

    std::any Instance();
    std::string Path();
    uint GetRefCount();
    uint GetID();
    
    Ref * AppendRef();
    void  DeleteRef(Ref * ref);

    TypeEnum Type();
    TypeEnum Type(TypeEnum type);

    void BindMeta(const uint          val);
    void BindMeta(const std::string & val);

private:
    void WakeRefs();

private:
    uint                _id;
    TypeEnum            _type;  //  类型
    std::vector<Ref *>  _refs;  //  引用列表
    std::string         _metas; //  字符元数据
    uint                _metai; //  数值元数据
};