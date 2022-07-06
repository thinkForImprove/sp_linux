#pragma once

//////////////////////////////////////////////////////////////////////////
#include <QLibrary>
#include "QtTypeDef.h"
//////////////////////////////////////////////////////////////////////////
#define WINPATH             "C:/CFES/BIN/"
#define LINUXPATH           "/usr/local/CFES/BIN/"
//////////////////////////////////////////////////////////////////////////
template<class DllType>
class CQtDLLLoader
{
public:
    class CAutoRelease
    {
    public:
        CAutoRelease(CQtDLLLoader *p): m_pDll(p), m_bRelease(true) {}
        ~CAutoRelease() { if (m_pDll != nullptr && m_bRelease) m_pDll->Release(); }
        void NoRelease() { m_bRelease = false; }
    private:
        CQtDLLLoader *m_pDll;
        bool m_bRelease;
    };
public:
    typedef long(*LPCREATE_DLL)(DllType *&ppDll);
    typedef long(*LPCREATE_DLL_TYPE)(LPCSTR lpType, DllType *&ppDll);
    typedef long(*LPCREATE_DLL_NAME_TYPE)(LPCSTR lpName, LPCSTR lpType, DllType *&ppDll);
public:
    CQtDLLLoader(): m_pDll(nullptr) { }
    virtual ~CQtDLLLoader() { Release(); }
public:
    // 0，成功；1，DLL不存在；2，入口函数不存在；<0，CreateFunc返回值
    long Load(LPCSTR lpDllName, LPCSTR lpszCreateFuncName)
    {
        CAutoRelease _auto(this);
        if (!LoadDll(lpDllName))
            return 1;
        LPCREATE_DLL pCreate = (LPCREATE_DLL)m_cLib.resolve(lpszCreateFuncName);
        if (pCreate == nullptr)
            return 2;
        long lRet = pCreate(m_pDll);
        if (lRet < 0)
            return lRet;
        _auto.NoRelease();
        return 0;
    }
    long Load(LPCSTR lpDllName, LPCSTR lpszCreateFuncName, LPCSTR lpType)
    {
        CAutoRelease _auto(this);
        if (!LoadDll(lpDllName))
            return 1;
        LPCREATE_DLL_TYPE pCreate = (LPCREATE_DLL_TYPE)m_cLib.resolve(lpszCreateFuncName);
        if (pCreate == nullptr)
            return 2;
        long lRet = pCreate(lpType, m_pDll);
        if (lRet < 0)
            return lRet;
        _auto.NoRelease();
        return 0;
    }
    long Load(LPCSTR lpDllName, LPCSTR lpszCreateFuncName, LPCSTR lpType, LPCSTR lpName)
    {
        CAutoRelease _auto(this);
        if (!LoadDll(lpDllName))
            return 1;
        LPCREATE_DLL_NAME_TYPE pCreate = (LPCREATE_DLL_NAME_TYPE)m_cLib.resolve(lpszCreateFuncName);
        if (pCreate == nullptr)
            return 2;
        long lRet = pCreate(lpName, lpType, m_pDll);
        if (lRet < 0)
            return lRet;
        _auto.NoRelease();
        return 0;
    }

    bool operator==(DllType *p) {return m_pDll == p;}
    bool operator!=(DllType *p) {return m_pDll != p;}
    bool operator!() {return !m_pDll;}
    DllType *operator->() {return m_pDll;}
    DllType *GetType() {return m_pDll;}
    operator DllType *() {return m_pDll;}
    QString LastError() {return m_cLib.errorString();}

    void Release()
    {
        if (m_pDll != nullptr)
            m_pDll->Release();
        if (m_cLib.isLoaded())
            m_cLib.unload();
        m_pDll = nullptr;
    }
private:
    bool LoadDll(LPCSTR lpDllName)
    {
        Release();
        QString strDllName(QString::fromLocal8Bit(lpDllName));
#ifdef Q_OS_WIN
        strDllName.prepend(WINPATH);
        m_cLib.setFileName(strDllName);
#else
        strDllName = strDllName.left(strDllName.lastIndexOf("."));
        strDllName.prepend(LINUXPATH);
        m_cLib.setFileName(strDllName);
#endif
        return m_cLib.load();
    }

protected:
    DllType     *m_pDll;
    QLibrary     m_cLib;
};
//////////////////////////////////////////////////////////////////////////
