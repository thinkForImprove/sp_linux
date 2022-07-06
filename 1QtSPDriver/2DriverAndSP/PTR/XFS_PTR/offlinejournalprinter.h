#ifndef OFFLINEJOURNALPRINTER_H
#define OFFLINEJOURNALPRINTER_H

#include "statusprinter.h"
#include <QReadWriteLock>
#include <QMutex>
#include <QTextStream>
#include <QThread>

// 为支持缺纸写入文件功能
DWORD GetSPOfflineKeyValue(LPCSTR lpValueName, DWORD dwDefault = 0);
void SetSPOfflineKeyValue(LPCSTR lpValueName, DWORD dwValue);
LPCTSTR GetSPOfflineKeyValue(LPCSTR lpValueName, LPCTSTR szDefault);
DWORD GetSPKeyValue(LPCSTR lpValueName, DWORD dwDefault);

// 支持缺纸写入文件功能在打印机类，支持CJournalPrinterCtrl的所有PUBLIC方法
// 新增方法:
//    StartWork : 要在调用其它方法前调用
//    EndWork   : 要在终止前调用，析构函数调用了EndWork
// 修改方法：
//    GetStatus : 增加两个返回状态: JOURNAL_NP_WRITE_FILE和JOURNAL_HE_WRITE_FILE
//    Print     : 如果StartWork参数指定可以缺纸打印到文件，把内容写到文件和内存列表中
//    其它方法  : 都先申请互斥，再调用CJournalPrinterCtrl的相应方法
// 新增类：
//    CAutoLock : 对象构造时自动进入临界区，析构时自动退出临界区
//    COfflinePrintFile : 管理打印文件，非线程安全的

// 打印机缺纸或故障时打印文件的管理
class COfflinePrintFile
{
public:
    COfflinePrintFile(LPCSTR lpFileName);
    virtual ~COfflinePrintFile();
public:
    inline int GetCount();
    inline int GetMaxCount() const;

    QString GetHead();
    void RemoveHead();

    void Clear();
    void Pack();

    // modified by liy 20150109 解决日志打印机打印大数据时重复打印问题
    bool AddLines(const char *pStr, int nLen);
protected:
    bool AddLine(QTextStream *pOut, char *p, int nSize);
protected:
    QReadWriteLock      m_syncMutex;
    QMutex              m_syncFileMutex;
    QString             m_strFileName;         // 保存故障打印文件的名字
    int                 m_nBeginLineNum;       // 打印文件的开始行数
    int                 m_nMaxLineNum;         // 故障打印支持的最大行数
    QStringList         m_sPrintList;          // 打印内容列表
    int                 m_nNeedBufferPrintData; // 无纸时是否记录离线文件0-不记录，1-记录
};

class COfflineJournalPrinter;

class CWorkThead : public QThread
{
    Q_OBJECT
public: CWorkThead(COfflineJournalPrinter *pJournalPrinter);
    ~CWorkThead();
public:
    void startWork();
    void stopWork();
protected:
    void run();
private:
    volatile bool m_bIsStopped;
    COfflineJournalPrinter *m_pJournalPrinter;
};


class COfflineJournalPrinter : public CJournalPrinter
{

public:
    COfflineJournalPrinter(const char *pDocFileName);
    virtual~COfflineJournalPrinter();
public:
    virtual int Open(const char *pDllName, const char *pDeviceName, int nPort, int nBauRate);
    virtual void Close();

    // 初始化设备
    // 返回：>=0成功，<0失败
    virtual int Init();

    // 打印字串
    // pStr，要打印的字串，nLen，数据长度，如果为-1，pStr以\0结束
    // 返回：>=0成功，<0失败
    virtual int Print(const char *pStr, int nLen);

    // 切纸
    // nFeedPace，切纸前走纸的步数
    // 返回：>=0成功，<0失败
    virtual int CutPaper(BOOL bDetectBlackStripe, int nFeedPace);

    // 得到打印机状态，保存到成员变量中
    virtual void UpdateStatus();

    inline bool CanOfflineWork();
    inline bool OfflineFileReachLimit();

    virtual BOOL IsJournalPrinter();

    // 内部调用这个函数而不是GetStatus以反映正确的设备状态
    void GetActualStatus(int *pPrinterStatus, int *pPaperStatus,
                         int *pTonerStatus);

    int GetOfflineSleepTime() const;        // 工作线程休息时间
    int GetOfflineUpdateStatusTime() const; // 工作线程更新状态的时间
    int GetOfflineWorkTime() const;         // 工作线程连续工作时间
    int GetofflineListBackTime() const;     // 工作线程列表备份时间

    int   PrintToDev();
    void  RemovePrintData();
    bool  HasPrintData();

    void Pack();
protected:
    BOOL StartWork();
    void EndWork();

    virtual void GetStatus(int *pPrinterStatus, int *pPaperStatus,
                           int *pTonerStatus);
private:
    void PreStartWork();
protected:
    QMutex              m_syncDevMutex;
    CWorkThead         *m_pWorkThread;
    COfflinePrintFile   *m_pFile;                   // 故障打印的文件，在StartWork中分配，用来指示是否打开故障打印到文件功能
    int                 m_nUpdateStatusTime;        //设备故障时更新设备状态的时间间隔
    int                 m_nContinuousPrintTime;     //最小连续打印时间(ms)
    int                 m_nContinuousPrintBreakTime;//连续打印休息时间(ms)
    int                 m_nPackTime;        //PACK文件的时间间隔
};

#endif // OFFLINEJOURNALPRINTER_H
