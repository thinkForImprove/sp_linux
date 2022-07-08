//DeviceDLLSingleLoader.h
#ifndef DEVICE_DLL_LOADER_H
#define DEVICE_DLL_LOADER_H

#include <QLibrary>
#include <QMessageBox>

template<class DeviceInterface>
class DeviceSOLoader
{
    typedef int (*LPCREATEDEVICE)(const char *pName, DeviceInterface *&ppDevice);
public:
    DeviceSOLoader()
    {
        m_pDevice = NULL;
        m_DeviceLib = NULL;
    }

    virtual ~DeviceSOLoader()
    {
        Release();
    }

    //0,成功；1，DLL不存在；2，入口函数不存在；<0，CreateFunc返回值
    int Load(const char *pDllName, const char *lpszCreateFuncName)
    {
        Release();

        m_DeviceLib = new QLibrary(pDllName);
        if (!m_DeviceLib->load())
        {
            QMessageBox::information(NULL, "Error", "Load失败!");
            Release();
            return 1;
        }

        LPCREATEDEVICE pCreate = (LPCREATEDEVICE)m_DeviceLib->resolve(lpszCreateFuncName);
        if (!pCreate)
        {
            QMessageBox::information(NULL, "Error", "Create失败!");
            Release();
            return 2;
        }

        int nRet = pCreate(NULL, m_pDevice);
        if (nRet < 0)
        {
            QMessageBox::information(NULL, "Error", "pCreate失败!");
            Release();
            return nRet;
        }

        return 0;
    }

    bool operator==(DeviceInterface *p)
    {
        return m_pDevice == p;
    }
    bool operator!=(DeviceInterface *p)
    {
        return m_pDevice != p;
    }

    bool operator!()
    {
        return !m_pDevice;
    }

    DeviceInterface *operator->()
    {
        return m_pDevice;
    }

    void Release()
    {

        if (m_pDevice != NULL)
        {
            m_pDevice->Release();
            m_pDevice = NULL;
        }

        if (m_DeviceLib != NULL)
        {
            delete (m_DeviceLib);
            m_DeviceLib = NULL;
        }
    }
protected:
    DeviceInterface *m_pDevice;
    //void* m_hHandle;
    QLibrary *m_DeviceLib;
};

#endif //DEVICE_DLL_LOADER_H


