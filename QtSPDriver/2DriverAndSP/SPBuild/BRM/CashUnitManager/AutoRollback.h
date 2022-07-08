#pragma once
#include "QtTypeDef.h"
//根据返回码进行自动回滚辅助类
template <class TYPE>
struct CAutoRollback
{
    //构造函数
    //pSrc：要保存和恢复的对象指针，不可为NULL
    //pRet：返回码地址，不可为NULL
    CAutoRollback(TYPE *pSrc, long *pRet)
    {
        assert(pSrc != NULL);
        assert(pRet != NULL);
        m_pRet = pRet;
        m_pSrc = pSrc;
        m_pOld = new TYPE(FALSE);
        m_pOld->CopyFrom(*pSrc, TRUE);
    }

    //析构函数
    //如果需要回滚，回滚到原数据
    ~CAutoRollback()
    {
        RollbackByRetcode();
    }

    //内部成员函数
protected:
    //回滚到原数据
    void RollbackByRetcode()
    {
        if (m_pSrc != NULL)
        {
            assert(m_pOld != NULL);
            assert(m_pRet != NULL);
            if (*m_pRet < 0)
            {
                m_pSrc->CopyFrom(*m_pOld, FALSE);
            }

            delete m_pOld;
            m_pOld = NULL;
            m_pSrc = NULL;
            m_pRet = NULL;
        }
    }

    //内部成员变量
protected:
    TYPE *m_pOld;   //保存老的数据
    TYPE *m_pSrc;   //保存源指针
    long *m_pRet;   //保存返回码变量的地址
};

