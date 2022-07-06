// AdapterDLL.h: interface for the CAdapterDLL class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_ADAPTERDLL_H__913F8CA5_F58E_4C9E_B644_14DD7C61E781__INCLUDED_)
#define AFX_ADAPTERDLL_H__913F8CA5_F58E_4C9E_B644_14DD7C61E781__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "IBRMAdapter.h"
#include "assert.h"

//为动态加载适配器DLL服务的类
//重载了->操作符
class CAdapterDLL
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
    CAdapterDLL();

    //析构函数
    virtual ~CAdapterDLL();

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
    //HINSTANCE         m_hinstAdapter;     //适配器DLL实例句柄
    IBRMAdapter         *m_pAdapter;        //适配器对象指针
};

#endif // !defined(AFX_ADAPTERDLL_H__913F8CA5_F58E_4C9E_B644_14DD7C61E781__INCLUDED_)
