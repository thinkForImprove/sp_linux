/***************************************************************************
* 文件名称: ClassCommon.h
* 文件描述: 声明通用类头文件
*          该文件定义的类是抽取所有模块处理中的通用部分,所有模块都可继承使用
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年7月13日
* 文件版本: 1.0.0.1
***************************************************************************/

#ifndef CLASS_COMMON_H
#define CLASS_COMMON_H

#include "QtTypeDef.h"

#include <qlibrary.h>
#include <dlfcn.h>


//*************************************************************************
//****************************** 动态库加载类 *******************************
//*************************************************************************

//-------------------------------------------------------------------------
//------------------------------- 宏定义 -----------------------------------
// 加载动态库接口(QLibrary方式)
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        SetLoadLibIsSucc(FALSE); \
        return FAIL;   \
    }

// dlxxx加载动态库接口
#define LOAD_LIBINFO_FUNC_DL(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)dlsym(m_vLibInst, FUNC2); \
    if(!FUNC) {   \
        SetLoadLibIsSucc(FALSE); \
        return FAIL;   \
    }

// 加载模式
#define LOADDLL_MODE_QLIB           0       // QLibrary方式
#define LOADDLL_MODE_DL             1       // dlxxx方式)

//-------------------------------------------------------------------------
//------------------------------- 类定义 -----------------------------------
// 使用说明: 1. 继承该类, 根据动态库接口重写bLoadLibIntf()和bLoadLibIntf()/bDlLoadLibIntf()
//         2. 设置动态库路径变量;
//         3. 根据m_bLoadIntfFail检查是否需要加载动态库, 按加载方式分别调用bLoadLibrary()+
//              bLoadLibIntf()或者bDlLoadLibrary()+bDlLoadLibIntf()
//         4. vUnLoadLibrary()/vDlUnLoadLibrary()用于释放动态库, 清零相关变量
class CLibraryLoad
{
public:
    CLibraryLoad();
    ~CLibraryLoad();

public:     // 接口加载(QLibrary方式)
    virtual INT nLoadLibrary(LPSTR lpLoadFi);                           // 加载动态库(QLibrary方式)[非必要不重写]
    virtual void vUnLoadLibrary();                                      // 释放动态库(QLibrary方式)[非必要不重写]
    virtual INT nLoadLibIntf();                                         // 加载动态库接口(QLibrary方式)[根据需要进行重写]

public:     // 接口加载(dlxxx方式)
    virtual INT  nDlLoadLibrary(LPSTR lpLoadFi);                        // 加载动态库(dlxxx方式)[非必要不重写]
    virtual void vDlUnLoadLibrary();                                    // 释放动态库(dlxxx方式)[非必要不重写]
    virtual INT nDlLoadLibIntf();                                       // 加载动态库接口(dlxxx方式)[根据需要进行重写]

public:     // QLibrary方式与dlxxx方式通用接口
    virtual void vInitLibFunc();                                        // 动态库接口初始化[根据需要进行重写]

public:     // QLibrary方式与dlxxx方式通用接口(不支持重写)
    LPSTR GetLibError(WORD wMode);                                      // 取错误信息
    BOOL GetLoadLibIsSucc();                                            // 动态库是否加载成功
    void SetLoadLibIsSucc(BOOL bIsSucc = TRUE);                         // 设置动态库是否加载成功
    void SetDlOpenMode(INT nMode = RTLD_NOW | RTLD_DEEPBIND);           // 设置dlOpen命令模式

public:     // 接口加载变量
    QLibrary    m_LoadLibrary;                                          // QLibrary方式库连接句柄
    void*       m_vLibInst;                                             // dlxxx方式库连接句柄

private:    // 接口加载变量
    INT         m_nDlOpenMode;                                          // dlOpen命令模式
    BOOL        m_bLoadIntfSucc;                                        // 动态库及接口加载是否成功
    CHAR        m_szErrBuffer[1024 * 10];                               // 错误信息保存变量
};

#endif // CLASS_COMMON_H

// -------------------------------- END -----------------------------------
