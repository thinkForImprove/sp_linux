#pragma once

#include "assert.h"

class QLoadDll
{
    //内部类，重载Release为私有的，防止调用->Release方法
    struct IBRMAdapterEx : public IBRMAdapter
    {
    private:    //重载Release为私有的，防止调用->Release方法
        //friend class CAdapterDLL;
        virtual void Release()
        {
            assert(false);
        }
    };

public:
    //释放动态库和内部资源
    void Release();

    //装载动态库
    //确保只装载一次，或先调用Release方法
    int Load(const char *pDllName);

    //构造函数
    QLoadDll();

    //析构函数
    virtual ~QLoadDll();

    //接口指针操作符
    operator IBRMAdapterEx *()
    {
        return (IBRMAdapterEx *)m_pAdapter;
    }

    //指针操作符
    IBRMAdapterEx *operator->()const
    {
        assert(m_pAdapter != NULL);
        return (IBRMAdapterEx *)m_pAdapter;
    }

private:
    HINSTANCE           m_hinstAdapter;     //适配器DLL实例句柄
    IBRMAdapter         *m_pAdapter;        //适配器对象指针
};

