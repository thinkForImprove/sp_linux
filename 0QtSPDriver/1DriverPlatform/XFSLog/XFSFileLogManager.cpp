#include "XFSFileLogManager.h"
#include "TSingleton.cpp"

//-------------------------------- 单件实现 -----------------
IMPLEMENT_SINGLETON(CXFSFileLogManager)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXFSFileLogManager::CXFSFileLogManager()
{

}

CXFSFileLogManager::~CXFSFileLogManager()
{

}

//------------------- 外部创建接口函数实现 -----------------------
extern "C" IXFSLogManager *XFSLogManagerGetInstance()
{
    IXFSLogManager *p = CXFSFileLogManager::GetInstance();
    return p;
}

extern "C" void XFSLogManagerDestroyInstance()
{
    CXFSFileLogManager::DestroyInstance();
}
