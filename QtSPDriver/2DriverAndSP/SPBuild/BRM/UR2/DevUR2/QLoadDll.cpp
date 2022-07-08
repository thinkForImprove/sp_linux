// AdapterDLL.cpp: implementation of the CAdapterDLL class.
//
//////////////////////////////////////////////////////////////////////

#include "QLoadDll.h"

#define THISFILE    "AdapterDLL"

typedef int (*PCreateBRMAdapter)(IBRMAdapter *&);

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAdapterDLL::CAdapterDLL()
{
    m_hinstAdapter = NULL;
    m_pAdapter = NULL;
}

CAdapterDLL::~CAdapterDLL()
{
    Release();
}

int CAdapterDLL::Load(const char *pDllName)
{
    const char *ThisModule = "Load";

    assert(m_pAdapter == NULL && m_hinstAdapter == NULL);

    //装入DLL
    m_hinstAdapter = LoadLibrary(pDllName);
    if (m_hinstAdapter == NULL)
    {
        log_write(LOGFILE, THISFILE, ThisModule, -1,
                  "LoadLibrary(%s) failed(GetLastError=%d)",
                  pDllName, GetLastError());
        return WFS_ERR_INTERNAL_ERROR;
    }

    //查找创建函数
    PCreateBRMAdapter pCreateBRMAdapter =
    (PCreateBRMAdapter)GetProcAddress(m_hinstAdapter, "CreateBRMAdapter");
    if (pCreateBRMAdapter == NULL)
    {
        log_write(LOGFILE, THISFILE, ThisModule, -1,
                  "GetProcAddress(%s, CreateBRMAdapter) failed(GetLastError=%d)",
                  pDllName, GetLastError());
        Release();
        return WFS_ERR_INTERNAL_ERROR;
    }

    //创建接口
    int iRet = pCreateBRMAdapter(m_pAdapter);
    if (iRet < 0)
    {
        log_write(LOGFILE, THISFILE, ThisModule, -1,
                  "call %s.CreateBRMAdapter() failed(iRet = %d)",
                  pDllName, iRet);
        Release();
        return WFS_ERR_INTERNAL_ERROR;
    }

    return 0;
}

void CAdapterDLL::Release()
{
    if (m_pAdapter != NULL)
    {
        m_pAdapter->Release();
        m_pAdapter = NULL;
    }
    if (m_hinstAdapter != NULL)
    {
        FreeLibrary(m_hinstAdapter);
        m_hinstAdapter = NULL;
    }
}
