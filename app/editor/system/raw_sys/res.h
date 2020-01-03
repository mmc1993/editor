#pragma once

#include "serializer.h"
#include "raw.h"

class GLObject;

class Res : public Serializer {
public:
    enum TypeEnum {
        kNull,       //  无
        kJson,       //  文本
        kImg,        //  图片
        kMap,        //  地图
        kFont,       //  字体
        kObj,        //  对象
        kVar,        //  变量
        kBlueprint,  //  蓝图
        Length,
    };

    static const std::string sTypeString[TypeEnum::Length];

    class Ptr {
    public:
        Ptr(Res* owner)
            : mModify(true)
            , mOwner(owner)
        { }

        ~Ptr()
        {
            if (mOwner != nullptr)
            {
                mOwner->DeletePtr(this);
            }
        }

        bool operator==(const Ptr & other)
        {
            return mOwner == other.mOwner;
        }

        uint GetID()
        {
            return mOwner->GetID();
        }

        bool Valid()
        {
            return mOwner != nullptr;
        }

        void Invalid()
        {
            mOwner = nullptr;
        }

        bool Modify()
        {
            return mModify;
        }

        bool Modify(bool modify)
        {
            mModify=modify;
            return mModify;
        }

        Ptr * Clone()
        {
            return mOwner->AppendPtr();
        }

        std::string Path()
        {
            ASSERT_LOG(mOwner != nullptr, "");
            return mOwner->Path();
        }

        template <class T>
        SharePtr<T> Instance()
        {
            ASSERT_LOG(mOwner != nullptr, "");
            Modify(false);
            return mOwner->Instance<T>();
        }

    private:
        Res* mOwner;
        bool mModify;
    };

    class Ref : public Serializer {
    public:
        Ref(Ptr * owner = nullptr): mOwner(owner)
        { }

        Ref(const Ref & other)
        {
            *this = other;
        }

        Ref(Ref && other)
        {
            *this = std::move(other);
        }

        Ref & operator=(Ref && other)
        {
            mValue.reset();
            mOwner.reset();
            mValue.swap(other.mValue);
            mOwner.swap(other.mOwner);
            return *this;
        }

        Ref & operator=(const Ref & other)
        {
            mValue = other.mValue;
            mOwner.reset(other.mOwner && other.mOwner->Valid()
                       ? other.mOwner->Clone()
                       : nullptr);
            return *this;
        }

        bool operator==(const Ref & other)
        {
            return  mOwner != nullptr
                &&  other.mOwner != nullptr
                && *mOwner == *other.mOwner
                ||  mOwner ==  other.mOwner;
        }

        bool operator!=(const Ref & other)
        {
            return !operator==(other);
        }

        ~Ref()
        { }

        bool Check()
        {
            if (mOwner && !mOwner->Valid())
            {
                mOwner.reset();
                mValue.reset();
            }
            return mOwner != nullptr;
        }
        
        bool Modify()
        {
            return mOwner->Modify();
        }

        std::string Path()
        {
            return Check() ? mOwner->Path() : std::string();
        }

        template <class T>
        SharePtr<T> Instance()
        {
            ASSERT_LOG(mOwner,"");
            if (mOwner->Modify())
            {
                mValue = mOwner->Instance<T>();
            }
            return std::any_cast<SharePtr<T>>(mValue);
        }

        virtual void EncodeBinary(std::ostream & os, Project * project) override;
        virtual void DecodeBinary(std::istream & is, Project * project) override;

    private:
        UniquePtr<Ptr> mOwner;
        std::any       mValue;
    };

public:
    Res(Project * owner, uint id = ~0);
    ~Res();

    //  实例化对象
    template <class T>
    SharePtr<T> Instance()
    { 
        if constexpr        (std::is_same_v<T, GLObject>)
        {
            return InstanceObj();
        }
        else if constexpr   (std::is_same_v<T, RawTexture>)
        {
            return InstanceTex();
        }
        else if constexpr   (std::is_same_v<T, RawImage>)
        {
            return InstanceImg();
        }
        else if constexpr (std::is_same_v<T, RawFont>)
        {
            return InstanceFont();
        }
        else if constexpr (std::is_same_v<T, RawMap>)
        {
            return InstanceMap();
        }
        else if constexpr (std::is_same_v<T, mmc::Json>)
        {
            return InstanceJson();
        }
        else
        {
            static_assert(false);
        }
    }

    std::string Path();
    uint GetRefCount();
    uint GetID();

    void WakeRefs();
    Ptr * AppendPtr();
    Ref   AppendRef();
    void  DeletePtr(Ptr * ptr);

    TypeEnum Type();
    TypeEnum Type(TypeEnum type);

    const        std::string & TypeString();
    static const std::string & TypeString(uint type);

    template <class T>
    void Meta(const T & val)
    {
        if (!mMeta.has_value() || std::any_cast<T &>(mMeta) != val)
        {
            mMeta = val; WakeRefs();
        }
    }

    template <class T>
    T Meta()
    {
        return std::any_cast<T>(mMeta);
    }

    virtual void EncodeBinary(std::ostream & os, Project * project) override;
    virtual void DecodeBinary(std::istream & is, Project * project) override;

private:
    SharePtr<GLObject>   InstanceObj();
    SharePtr<RawTexture> InstanceTex();
    SharePtr<RawImage>   InstanceImg();
    SharePtr<RawFont>    InstanceFont();
    SharePtr<RawMap>     InstanceMap();
    SharePtr<mmc::Json>  InstanceJson();

private:
    uint                mID;
    TypeEnum            mType;      //  类型
    std::any            mMeta;      //  元数据
    std::vector<Ptr *>  mPtrs;      //  引用列表
    Project *           mOwner;     //  项目归属
};