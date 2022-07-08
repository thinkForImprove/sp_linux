#ifndef XFSFILELOGMANAGER_H
#define XFSFILELOGMANAGER_H

#include "XFSLogManager.h"
#include "XFSLogThread.h"
#include "TSingleton.h"

//文件日志管理器
class CXFSFileLogManager : public CXFSLogManager
{
    //----------------------- 单件接口 ---------------------
public:
    CXFSFileLogManager();
    virtual ~CXFSFileLogManager();

    //实现或重载父类虚函数
protected:
    virtual void LogToFile(const char *pStr)
    {
        m_LogThread.Log(pStr);
    }

    //私有的数据成员
private:
    CXFSLogThread       m_LogThread;            //日志线程

    DECLARE_SINGLETON(CXFSFileLogManager)
};

#endif // XFSFILELOGMANAGER_H
