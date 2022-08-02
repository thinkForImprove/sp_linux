/***************************************************************************
* 文件名称: ClassCommon.cpp
* 文件描述: 通用类实现
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年7月13日
* 文件版本: 1.0.0.1
***************************************************************************/


#include "ClassCommon.h"

//*************************************************************************
//************************** 动态库加载类 实现 *******************************
//*************************************************************************
CLibraryLoad::CLibraryLoad()
{
    //memset(m_szLoadDllPath, 0x00, sizeof(m_szLoadDllPath));
    m_bLoadIntfSucc = FALSE;
    m_vLibInst = nullptr;                                   // dlxxx方式库连接句柄
    m_nDlOpenMode = RTLD_NOW | RTLD_DEEPBIND;               // dlOpen命令模式
}

CLibraryLoad::~CLibraryLoad()
{
    vUnLoadLibrary();
    vDlUnLoadLibrary();
}

// 加载动态库(QLibrary方式)
INT CLibraryLoad::nLoadLibrary(LPSTR lpLoadFile)
{
    INT nRet = 0;

    if (lpLoadFile == nullptr)
    {
        return -1;
    }

    m_LoadLibrary.setFileName(lpLoadFile);
    m_bLoadIntfSucc = FALSE;

    // 加载动态库(已加载则跳过)
    if (m_LoadLibrary.isLoaded() != true)
    {
        if (m_LoadLibrary.load() != true)
        {
            return 1;
        }
    }

    // 动态库接口未加载成功时, 重新加载
    if (m_bLoadIntfSucc != TRUE)
    {
        // 先设置动态库接口加载成功标记为T
        m_bLoadIntfSucc = TRUE;

        // 调用动态库接口加载函数, 加载有失败则改加载成功标为F
        if ((nRet = nLoadLibIntf()) != SUCCESS)
        {
            return 2;
        }
    }

    return SUCCESS;
}

// 释放动态库(QLibrary方式)
void CLibraryLoad::vUnLoadLibrary()
{
    if (m_LoadLibrary.isLoaded())
    {
        m_LoadLibrary.unload();
        m_bLoadIntfSucc = FALSE;
        vInitLibFunc(); // 动态库接口函数初始化
    }
}

// 加载动态库接口(QLibrary方式)
INT CLibraryLoad::nLoadLibIntf()
{
    // 1.
    //LOAD_LIBINFO_FUNC(接口变量, 接口类型, "接口名");

    return SUCCESS;
}

// 加载动态库(dlxxx方式)
INT CLibraryLoad::nDlLoadLibrary(LPSTR lpLoadFile)
{
    INT nRet = 0;

    if (lpLoadFile == nullptr)
    {
        return -1;
    }

    m_bLoadIntfSucc = TRUE;

    // 加载动态库(已加载则跳过)
    if (dlopen(lpLoadFile, RTLD_NOLOAD) == nullptr && m_vLibInst == nullptr)
    {
        m_vLibInst = dlopen(lpLoadFile, m_nDlOpenMode);
        if (m_vLibInst == nullptr)
        {
            return 1;
        }
    }

    if (m_bLoadIntfSucc)
    {
        // 先设置动态库接口加载成功标记为T
        m_bLoadIntfSucc = TRUE;

        // 调用动态库接口加载函数, 加载有失败则改加载成功标为F
        if ((nRet = nDlLoadLibIntf()) != SUCCESS)
        {
            return 2;
        }
    }

    return SUCCESS;
}

// 释放动态库(dlxxx方式)
void CLibraryLoad::vDlUnLoadLibrary()
{
    if(m_vLibInst != nullptr)
    {
        dlclose(m_vLibInst);
        m_vLibInst = nullptr;
        m_bLoadIntfSucc = FALSE;
        vInitLibFunc(); // 动态库接口函数初始化
    }
}

// 加载动态库接口函数(dlxxx方式)
INT CLibraryLoad::nDlLoadLibIntf()
{
    // 1. 函数接口
    //LOAD_LIBINFO_FUNC_DL(接口变量, 接口类型, "接口名");

    return SUCCESS;
}

// 动态库接口初始化
void CLibraryLoad::vInitLibFunc()
{
    // 动态库接口函数初始化
    //ZLOpenDevice = nullptr;           // 1.
}

// 取错误信息
LPSTR CLibraryLoad::GetLibError(WORD wMode)
{
    MSET_0(m_szErrBuffer);

    if (wMode == 0)
    {
        sprintf(m_szErrBuffer, "%s", (LPCSTR)m_LoadLibrary.errorString().toStdString().c_str());
    } else
    {
        sprintf(m_szErrBuffer, "%s", dlerror());
    }

    return m_szErrBuffer;
}

// 动态库是否加载成功
BOOL CLibraryLoad::GetLoadLibIsSucc()
{
    return m_bLoadIntfSucc;
}

// 设置动态库是否加载成功
void CLibraryLoad::SetLoadLibIsSucc(BOOL bIsSucc)
{
    m_bLoadIntfSucc = bIsSucc;
}

// 设置dlOpen命令模式
// nMode: RTLD_LAZY: dlopen()返回前, 对库中未定义符号不执行解析
//        RTLD_NOW: dlopen()返回前, 解析共享对象中所有未定义符号
//        RTLD_BINDING_MASK:
//        RTLD_NOLOAD: 不加载库, 用于测试库是否已加载
//        RTLD_DEEPBIND: 在搜索全局符号前先搜索库内符号, 避免同名冲突
void CLibraryLoad::SetDlOpenMode(INT nMode)
{
    if (AND_IS0(nMode, RTLD_LAZY) &&
        AND_IS0(nMode, RTLD_NOW) &&
        AND_IS0(nMode, RTLD_BINDING_MASK) &&
        AND_IS0(nMode, RTLD_NOLOAD) &&
        AND_IS0(nMode, RTLD_DEEPBIND))
    {
        m_nDlOpenMode = RTLD_NOW | RTLD_DEEPBIND;
    } else
    {
        m_nDlOpenMode = nMode;
    }
}

// -------------------------------- END -----------------------------------
