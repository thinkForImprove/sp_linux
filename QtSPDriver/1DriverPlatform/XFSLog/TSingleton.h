//TSingleton.h
#if !defined(TSINGLETON_H)
#define TSINGLETON_H

#ifdef SINGLETON_SUPPORT_MULT_THREAD
#include "SimpleMutex.h"
extern CSimpleCriticalSection g_SingletonCritSect;
#endif //SINGLETON_SUPPORT_MULT_THREAD

template <class TYPE>
TYPE *SingletonGetInstance(TYPE *&pInstance)
{
#ifdef SINGLETON_SUPPORT_MULT_THREAD
    CAutoUnlockMutex _auto_unlock(&g_SingletonCritSect);
#endif //SINGLETON_SUPPORT_MULT_THREAD

    if (pInstance == NULL)
    {
        pInstance = new TYPE();
        atexit(TYPE::DestroyInstance);
    }
    return pInstance;
}

template <class TYPE>
void SingletonDestroyInstance(TYPE *&pInstance)
{
#ifdef SINGLETON_SUPPORT_MULT_THREAD
    CAutoUnlockMutex _auto_unlock(&g_SingletonCritSect);
#endif //SINGLETON_SUPPORT_MULT_THREAD

    if (pInstance != NULL)
    {
        delete pInstance;
        pInstance = NULL;
    }
}

#define DECLARE_SINGLETON(Class)\
    public: \
    static Class *GetInstance();\
    static void DestroyInstance();\
    protected:\
    static Class *m_pInstance;

#define IMPLEMENT_SINGLETON(Class)\
    Class *Class::m_pInstance = NULL;\
    Class *Class::GetInstance() { return SingletonGetInstance(m_pInstance); }\
    void Class::DestroyInstance() { SingletonDestroyInstance(m_pInstance); }

#endif //TSINGLETON_H
