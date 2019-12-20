#pragma once

#include "../include.h"

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
//  条件成立, 继续执行
//  条件不成立, 打印日志, 中断程序
#define ASSERT_LOG(cond, fmt, ...)                      \
{                                                       \
    if (!(cond))                                        \
    {                                                   \
        std::cout << "Line: " << __LINE__     << " "    \
                  << "Func: " << __FUNCTION__ << " "    \
                  << SFormat(fmt, __VA_ARGS__)          \
                  << std::endl;                         \
        abort();                                        \
    }                                                   \
}

//  条件成立, 返回后续参数
//  条件不成立, 中断程序
#define ASSERT_RET(cond, ...)	                        \
{                                                       \
    ASSERT_LOG(cond,"");                                \
    return __VA_ARGS__;                                 \
}

//  条件成立, 返回后续参数
//  条件不成立, 继续执行
#define CHECK_RET(cond, ...)	                        \
{                                                       \
    if (!(cond))                                        \
    {                                                   \
        return __VA_ARGS__;                             \
    }                                                   \
}
#else
#define ASSERT_LOG
#define ASSERT_RET
#define CHECK_RET
#endif