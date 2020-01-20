// ok 窗口菜单
// ok 弹出菜单
// ok 拉伸方向判断


// 对象
// 组件


// 图层
// 对象:
//     组件0
//     组件1
//     组件2

//  工程类
// class Project {
// public:
//     GLOBject * mRoot;

// public:
//     const std::string & GetFolderPath() const;
//     const std::string & GetFilePath() const;
//     bool Load(const std::string & url);
//     bool Save(const std::string & url);
//     void Free();
// }

//  提供二进制序列化/反序列化
//  提供字符串序列化/反序列化
//  序列化接口
// class Serializer {
// public:
//     virtual void Encode(std::ofstream & os) = 0;
//     virtual void Decode(std::ifstream & is) = 0;
// }

ok  //  event_enum 放进 EventSys
ok  //  event_listen 放进 EventSys
ok  //  parser_tool.h 放进 Serializer
ok  //  UIEventEnum 剥离


// 需要序列化的对象:
//     Project
//     GLObject
//     Component


// 拖放参数:
//     bool IsCanDrag,

//     struct Drag {
//         glm::vec2 mBegWorld;
//         glm::vec2 mEndWorld;
//     }

//     struct Drag {
//         UIObject * mObject;
//         glm::vec2 mBegWorld;
//         glm::vec2 mEndWorld;
//     }

// 使用智能指针

// 为GLObject生成唯一ID
// EditorSys只操作GLObject

//  图集/图片合并
//  资源缓存改用WeakPtr

//  GLCanvas
//     切换模式: 回车

// 编辑模式:
//     绘制边框, 显示控制点
//     左键双击边框 -> 新增控制点
//     左键单击控制点 -> 选中控制点
//     左键按住控制点 -> 拖动控制点
//     右键按住控制点 -> 新增控制点
//     Delete        -> 删除控制点

//  拖动模式:
//      绘制边框
//      左键按住边框内 -> 拖动对象
//      Delete        -> 删除对象

// 精灵
// 瓦片地图
// 光照
// 贝塞尔曲线
// 可视区裁剪
// 分层渲染
// 对象锁定
// 对象可见
// 对象拖放
// 文字

// _windows
// UISys.Update()
// UISys.OpenWindow()
// UISys.FreeWindow()
// UISys.IsWindowExist()

// 资源管理器:
//     文件对象
//     抽象对象
// 操作:
//     检索
//     查看引用
//     移动资源
//     删除资源

// 文件对象(Txt, Img, Map, Font)
// 抽象对象(Obj, Var, Blueprint)

// 文件对象ID => 文件路径
// 抽象对象ID => 对象ID

// 文件对象路径 => 文件路径
// 抽象对象路径 => 对象路径

// 文件对象元数据 => 文件路径
// 抽象对象元数据 => 对象ID

// 文件对象实例 => Raw实例
// 抽象对象实例 => Obj实例


// class Res {
// public:
//     enum TagEnum {
//         kNull,      //  无
//         kTxt,       //  文本
//         kImg,       //  图片
//         kMap,       //  地图
//         kFont,      //  字体
//         kObj,       //  对象
//         kVar,       //  变量
//         kBlueprint, //  蓝图
//     }

//     class Ref {
//     public:
//         ~Ref();
//         Ref(uint owner);
//         Ref(const Ref & other);
//         Ref & operator=(const Ref & other);

//         bool IsModify()
//         {
//             return _modify;
//         }

//         void SetModify()
//         {
//             _modify = true;
//         }

//         template <class T>
//         T Load()
//         {
//             _modify = false;
//             return std::any_cast<T>(_parent->Load());
//         }

//     private:
//         Res * _parent;
//         bool  _modify;
//     }

// public:
//     Res();
//     ~Res();
//     std::any Load();
//     Ref * Insert();
//     void  Delete(Ref * ref);

// private:
//     uint                _id;
//     TagEnum             _tag;   //  标签
//     std::vector<Ref *>  _refs;  //  引用集
//     std::string         _metas; //  字符元数据
//     uint                _metai; //  数值元数据
// }

//  作用
//  生成资源列表
//  查找/移动/删除资源
//  资源按标签分类

//  引用关系
//  Res 资源对象
//  Ref 引用对象

// --  Serach
// --      Type0 Type1|Word0 Word1

// --  Menu
// --      Delete
// --      Rename
// --      SetType
// --      Reference

// --  Show
// --      Type|Path > Word0 Word1

// OnCallEventMessage 消除第三个参数
// 移动 pack_atlas tool








// 地形破坏
// 博客素材：
//     地图
//     地形

//     裁剪区网格
//     裁剪区硬边缘，动态图
//     裁剪区软边缘，动态图

//     地形擦除，动态图
//     地形擦除，遮挡剔除，动态图



导航网格
变形
变量
网络
粒子
行为树

特效:
    漩涡
    水波
    图集合并
    波函数坍塌