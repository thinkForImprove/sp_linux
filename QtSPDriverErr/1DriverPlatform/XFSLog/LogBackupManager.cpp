#include "LogBackupManager.h"
#include <time.h>
#include <string>
using namespace std;
#include <algorithm>
#include <functional>
#include <stdio.h>

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CLogBackupManager::CLogBackupManager()
{
    m_tLastDeleteBackupTime = 0;
}

CLogBackupManager::~CLogBackupManager()
{

}

void CLogBackupManager::SetBackupInfo(const char *pRootDir,
                                      const char *pSubFormat, int nMaxBackupCount)
{
    m_sRootDir = pRootDir;
    m_sSubFormat = pSubFormat;
    m_nMaxBackupCount = nMaxBackupCount;
}

//删除日志备份
void CLogBackupManager::DeleteBackups()
{
    //测试上次处理备份时间，如果不大于指定时间，则返回
    time_t tCur = time(NULL);
    if (tCur - m_tLastDeleteBackupTime < 12 * 3600)
    {
        return;
    }
    m_tLastDeleteBackupTime = tCur;

    //找到所有子目录，保存到列表中
    QStringList qDirList;
    char szPath[256];
    sprintf(szPath, "%s/%s", m_sRootDir.c_str(), m_sSubFormat.c_str());
    QDir dir(szPath);
    dir.setFilter(QDir::Dirs);
    dir.setSorting(QDir::Name | QDir::Reversed);
    qDirList = dir.entryList();
    if (qDirList.size() <= 2)   //目录为空时:qDirList为".",".."
        return;
    qDirList.removeOne(".");
    qDirList.removeOne("..");
    //如果找到的目录数不大于最大备份数，返回
    if (qDirList.size() <= m_nMaxBackupCount)
        return;

    //删除最后的目录，直到目录个数满足条件
    while (qDirList.size() > m_nMaxBackupCount)
    {
        QString strFilePath = QString("%1/%2").arg(m_sRootDir.c_str()).arg(qDirList.back());
        if (!RemoveDir(strFilePath))
            break;
        qDirList.removeLast();
    }
}

bool CLogBackupManager::RemoveDir(QString strDirName)
{
    //查找目录下的文件,存在则删除
    QDir dir(strDirName);
    dir.setFilter(QDir::Files);
    QStringList qFileList = dir.entryList();
    while (qFileList.size() > 0)
    {
        if (false == dir.remove(qFileList.back()))
            return false;
        qFileList.removeLast();
    }
    //查找目录下的子目录,存在则删除
    dir.setFilter(QDir::Dirs);
    QStringList qSubDirList = dir.entryList();
    if (qSubDirList.size() <= 2)   //子目录为空
        return dir.rmdir(strDirName);
    qSubDirList.removeOne(".");
    qSubDirList.removeOne("..");
    while (qSubDirList.size() > 0)
    {
        if (!RemoveDir(QString("%1%2").arg(strDirName).arg(qSubDirList.back())))
            return false;
        qSubDirList.removeLast();
    }
    return dir.rmdir(strDirName);
}

