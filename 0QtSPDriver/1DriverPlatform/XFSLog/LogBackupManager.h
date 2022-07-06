#ifndef LOGBACKUPMANAGER_H
#define LOGBACKUPMANAGER_H
#include <string>
#include <QDir>
using namespace std;

class CLogBackupManager
{
public:
    void DeleteBackups();
    void SetBackupInfo(const char *pRootDir, const char *pSubFormat, int nMaxBackupCount);
    CLogBackupManager();
    virtual ~CLogBackupManager();
protected:
    bool RemoveDir(QString strDirName);

private:
    string m_sRootDir;
    string m_sSubFormat;
    int m_nMaxBackupCount;
    time_t  m_tLastDeleteBackupTime;    //最后删除备份时间
};

#endif // LOGBACKUPMANAGER_H
