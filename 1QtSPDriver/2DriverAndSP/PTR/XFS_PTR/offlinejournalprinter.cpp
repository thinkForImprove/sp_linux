#include "offlinejournalprinter.h"
#include "ndt_ptr_sp.h"
#include "log_lib.h"
#include "mng_transdev.h"


#include <QDate>
#include <QFile>
#include <QTextCodec>
#include "stdio.h"

#define ThisFile                        "Journal:OfflineJournalPrinter"
#define OFFLINE_FILE_PACK_TIME_DEF          (5 * 60)
#define OFFLINE_FILE_UPDATE_STATUS_TIME_DEF  (60)

#define PACK_TIME_VALUE_NAME            "offline_pack_interval"
#define UPDATE_STATUS_TIME_VALUE_NAME   "Offline_update_status_interval" // 状态不正常时更新状态时间注册表值名
#define LINE_NUM_VALUE_NAME             "offline_file_begin_line_num"     // 故障打印文件的开始行数
#define MAX_LINE_VALUE_NAME             "offline_file_max_line_num"       // 故障打印文件的最大行数

#define CONT_PRINT_TIME_VALUE_NAME      "continuous_print_time"
#define CONT_PRINT_BREAK_VALUE_NAME     "continuous_print_break_time"

#define OFFLINE_FILE_NAME_VALUE_NAME    "offline_file_name"
#define OFFLINE_PRINT_FILE_NAME_DEF     "journal_offline_print_data.txt"
#define NEED_BUFFER_PRINT_DATA          "need_buffer_print_data"

#define IDS_ERR_OPENFILE_FAILD          "open[%s]失败:原因=[%s]"

#define MAX_LINEBYTE  4096

// 连续打印状态
#define CONTPRINT_BREAK         (0) //正在休息
#define CONTPRINT_WAIT_PRINT    (1) //等待打印
#define CONTPRINT_PRINT         (2) //正在连续打印

//---------------------------------------------------
//                  COfflinePrintFile
//---------------------------------------------------
COfflinePrintFile::COfflinePrintFile(LPCSTR lpFileName)
{
    const char *const ThisModule = "COfflinePrintFile";

    this->m_strFileName  = QString(lpFileName);
    this->m_nBeginLineNum   = GetSPOfflineKeyValue(LINE_NUM_VALUE_NAME);
    this->m_nMaxLineNum = GetSPOfflineKeyValue(MAX_LINE_VALUE_NAME, 100000);

    QFile file(this->m_strFileName);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        QByteArray errByte = file.errorString().toLocal8Bit();
        QByteArray fileNameByte = this->m_strFileName.toLocal8Bit();
        log_write(LOGFILE, ThisFile, ThisModule, -1, IDS_ERR_OPENFILE_FAILD, fileNameByte.data(),
                  errByte.data());
    }
    else
    {
        QTextStream in(&file);
        in.setCodec("gbk");
        QString strTmp;
        int nLineCnt = 0;
        while (!in.atEnd())
        {
            strTmp = in.readLine(MAX_LINEBYTE);
            if (!strTmp.isEmpty())
            {
                if (nLineCnt >= this->m_nBeginLineNum)
                {
                    // add \n read will ignore \n
                    this->m_sPrintList.append(strTmp + "\n");
                }
                nLineCnt++;
            }
        }
    }
    file.close();

    this->m_nNeedBufferPrintData = GetSPKeyValue(NEED_BUFFER_PRINT_DATA, 0); //add by wangyz[2015007]
}
COfflinePrintFile::~COfflinePrintFile()
{
    this->Pack();
    this->m_sPrintList.clear();
}
int COfflinePrintFile::GetCount()
{
    QReadLocker locker(&this->m_syncMutex);
    return this->m_sPrintList.size();
}
int COfflinePrintFile::GetMaxCount() const
{
    return this->m_nMaxLineNum;
}

QString COfflinePrintFile::GetHead()
{
    QReadLocker locker(&this->m_syncMutex);
    return this->m_sPrintList.value(0, "");
}
void COfflinePrintFile::RemoveHead()
{
    QWriteLocker locker(&this->m_syncMutex);
    this->m_sPrintList.removeAt(0);

    this->m_nBeginLineNum++;
    SetSPOfflineKeyValue(LINE_NUM_VALUE_NAME, this->m_nBeginLineNum);
}
void COfflinePrintFile::Clear()
{
    QWriteLocker locker(&this->m_syncMutex);
    this->m_sPrintList.clear();

    QMutexLocker fileLocker(&this->m_syncFileMutex);
    QFile::remove(this->m_strFileName);

    this->m_nBeginLineNum = 0;
    SetSPOfflineKeyValue(LINE_NUM_VALUE_NAME, this->m_nBeginLineNum);
}
void COfflinePrintFile::Pack()
{
    const char *const ThisModule = "Pack";
    if (0 == this->m_nBeginLineNum)
    {
        return;
    }

    QMutexLocker filelocker(&this->m_syncFileMutex);
    QFile file(this->m_strFileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        file.close();
        QByteArray errByte = file.errorString().toLocal8Bit();
        QByteArray fileNameByte = this->m_strFileName.toLocal8Bit();
        log_write(LOGFILE, ThisFile, ThisModule, -1, IDS_ERR_OPENFILE_FAILD, fileNameByte.data(),
                  errByte.data());
        return;
    }

    QReadLocker locker(&this->m_syncMutex);
    QTextStream out(&file);
    out.setCodec("gbk");
    int nSize = this->m_sPrintList.size();
    for (int i = 0; i < nSize; ++i)
    {
        out << this->m_sPrintList.value(i);
    }
    out.flush();
    file.close();

    this->m_nBeginLineNum = 0;
    SetSPOfflineKeyValue(LINE_NUM_VALUE_NAME, this->m_nBeginLineNum);
}


bool COfflinePrintFile::AddLine(QTextStream *pOut, char *p, int nSize)
{
    const char *const ThisModule = "AddLine";
    if (NULL == p)
    {
        return false;
    }

    QWriteLocker locker(&this->m_syncMutex);

    QTextCodec *codec = QTextCodec::codecForLocale();
    QTextCodec *codec1 = QTextCodec::codecForName("gbk");
    if (NULL == codec1)
    {
        log_write(LOGFILE, ThisFile, ThisModule, -1, "无效的字符集gbk");
        return false;
    }
    QTextCodec::setCodecForLocale(codec1);
    if (nSize < MAX_LINEBYTE)
    {
        if (this->m_sPrintList.size() < this->m_nMaxLineNum)
        {
            (*pOut) << QString::fromLocal8Bit(p, nSize);
            this->m_sPrintList.append(QString::fromLocal8Bit(p, nSize));
        }
    }
    else
    {
        // split line
        int nTmpLen = nSize - MAX_LINEBYTE;
        char *pTmp = p;
        while (0 < nTmpLen)
        {
            if (this->m_sPrintList.size() < this->m_nMaxLineNum)
            {
                (*pOut) << QString::fromLocal8Bit(pTmp, MAX_LINEBYTE);
                this->m_sPrintList.append(QString::fromLocal8Bit(pTmp, MAX_LINEBYTE));
            }
            pTmp = p + MAX_LINEBYTE;
            nTmpLen = nTmpLen - MAX_LINEBYTE;
            if (0 > nTmpLen)
            {
                nTmpLen += MAX_LINEBYTE;
                if (this->m_sPrintList.size() < this->m_nMaxLineNum)
                {
                    (*pOut) << QString::fromLocal8Bit(pTmp, nTmpLen);;
                    this->m_sPrintList.append(QString::fromLocal8Bit(pTmp, nTmpLen));
                }
                break;
            }
        }
    }
    QTextCodec::setCodecForLocale(codec);
    return true;
}
// modified by liy 20150109 解决日志打印机打印大数据时重复打印问题
bool COfflinePrintFile::AddLines(const char *pStr, int nLen)
{
    const char *const ThisModule = "AddLines";
    if (NULL == pStr)
    {
        return true;
    }

    if (this->m_nNeedBufferPrintData)
    {
        QMutexLocker fileLocker(&this->m_syncFileMutex);
        QFile file(this->m_strFileName);
        if (!file.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text))
        {
            file.close();
            QByteArray errByte = file.errorString().toLocal8Bit();
            QByteArray fileNameByte = this->m_strFileName.toLocal8Bit();
            log_write(LOGFILE, ThisFile, ThisModule, -1, IDS_ERR_OPENFILE_FAILD, fileNameByte.data(),
                      errByte.data());
            return false;
        }
        QTextStream out(&file);
        out.setCodec("gbk");
        if ((-1) == nLen)
        {
            nLen = strlen(pStr);
        }

        char *p = (char *)pStr;
        LPCTSTR pEnd = NULL;
        int  nTmpLen = 0;
        while (NULL != p && 0 < nLen)
        {
            pEnd = strchr(p, '\n');
            nTmpLen = strlen(p);
            if (0 < nTmpLen)
            {
                this->AddLine(&out, p, nTmpLen);
            }
            if (NULL == pEnd)
            {
                // only one line or the last remain
                break;
            }
            else
            {
                p += nTmpLen;
            }
        }
        out.flush();
        file.close();
    }

    return true;
}

//////////////////////////////////////////////////////////////////////
// CWorkThead
/////////////////////////////////////////////////////////////////////
CWorkThead::CWorkThead(COfflineJournalPrinter *pJournalPrinter)
    : m_bIsStopped(true)
{
    this->m_pJournalPrinter = pJournalPrinter;
}
CWorkThead::~CWorkThead()
{}

void CWorkThead::startWork()
{
    this->m_bIsStopped = false;
    this->start();
}
void CWorkThead::stopWork()
{
    this->m_bIsStopped = true;
    this->quit();
    this->wait();
}

void CWorkThead::run()
{
    int nPrinterState, nPaperState, nTonerState;
    int nOldPrinterState, nOldPaperState, nOldTonerState;
    this->m_pJournalPrinter->UpdateStatus();
    this->m_pJournalPrinter->GetActualStatus(&nOldPrinterState, &nOldPaperState, &nOldTonerState);

    int nContinuousPrintStatus = CONTPRINT_BREAK;
    DWORD dwCurTime = GetTickCount();
    DWORD dwUpdateStatusInternal  = dwCurTime;
    DWORD dwContinuePrintTime = dwCurTime;
    DWORD dwOnIdleTime = dwCurTime;
    DWORD dwPrintListBackTime = dwCurTime;
    bool  bIsForceUpdateSta = false;
    while (!this->m_bIsStopped)
    {
        dwCurTime = GetTickCount();
        // 更新状态的时间间隔
        if (bIsForceUpdateSta || dwUpdateStatusInternal + this->m_pJournalPrinter->
            GetOfflineUpdateStatusTime() <   dwCurTime)
        {
            bIsForceUpdateSta = false;
            dwUpdateStatusInternal = dwCurTime;
            // 更新本地状态
            this->m_pJournalPrinter->GetActualStatus(&nPrinterState, &nPaperState, &nTonerState);
            if (nPrinterState != nOldPrinterState || nPaperState != nOldPaperState || nTonerState !=
                nOldTonerState)
            {
                nOldPrinterState = nPrinterState;
                nOldPaperState = nPaperState;
                nOldTonerState = nTonerState;
            }
        }

        // 备份列表的问题
        if (dwPrintListBackTime + this->m_pJournalPrinter->GetofflineListBackTime() < dwCurTime)
        {
            this->m_pJournalPrinter->Pack();
        }

        // 检查设备状态,是否可以执行打印
        if (nPrinterState == PTR_DEV_ONLINE &&
            (nPaperState == PTR_PAPER_FULL || nPaperState == PTR_PAPER_LOW) &&
            (nTonerState == PTR_TONER_FULL || nTonerState == PTR_TONER_LOW))
        {
            // 可以打印
            nContinuousPrintStatus = CONTPRINT_WAIT_PRINT;
        }

        switch (nContinuousPrintStatus)
        {
        case CONTPRINT_BREAK:
            {
                if (dwOnIdleTime + this->m_pJournalPrinter->GetOfflineSleepTime() < dwCurTime)
                {
                    nContinuousPrintStatus = CONTPRINT_WAIT_PRINT; // 转等待打印工作状态
                }
            }
            break;
        case CONTPRINT_WAIT_PRINT:
            {
                // 是否有打印数据
                if (this->m_pJournalPrinter->HasPrintData())
                {
                    // 执行打印
                    if (!PTR_SUCC(this->m_pJournalPrinter->PrintToDev()))
                    {
                        // 打印失败 强制刷设备状态，同时转休息状态
                        bIsForceUpdateSta = true;
                        nContinuousPrintStatus = CONTPRINT_BREAK;
                        dwOnIdleTime = dwCurTime;
                    }
                    else
                    {
                        this->m_pJournalPrinter->RemovePrintData();
                        // 转连续打印状态
                        nContinuousPrintStatus = CONTPRINT_PRINT;
                        dwOnIdleTime = dwCurTime;
                    }
                }
                else
                {
                    nContinuousPrintStatus = CONTPRINT_BREAK; // 无数据转休息状态
                    dwOnIdleTime = dwCurTime;
                }
            }
            break;
        case CONTPRINT_PRINT:
            {
                if (dwContinuePrintTime + this->m_pJournalPrinter->GetOfflineWorkTime() < dwCurTime)
                {
                    nContinuousPrintStatus = CONTPRINT_BREAK; // 转休息状态
                    dwOnIdleTime = dwCurTime;
                }
                else
                {
                    // 是否有打印数据
                    if (this->m_pJournalPrinter->HasPrintData())
                    {
                        // 执行打印
                        if (!PTR_SUCC(this->m_pJournalPrinter->PrintToDev()))
                        {
                            // 打印失败 强制刷设备状态,同时转休息状态
                            bIsForceUpdateSta = true;
                            nContinuousPrintStatus = CONTPRINT_BREAK; // 无数据转休息状态
                            dwOnIdleTime = dwCurTime;
                        }
                        else
                        {
                            this->m_pJournalPrinter->RemovePrintData();
                            // 转连续打印状态
                            nContinuousPrintStatus = CONTPRINT_PRINT;
                        }
                    }
                    else
                    {
                        nContinuousPrintStatus = CONTPRINT_BREAK; // 无数据转休息状态
                        dwOnIdleTime = dwCurTime;
                    }
                }
            }
            break;
        default:
            break;
        }

        msleep(50);
    }
}


//////////////////////////////////////////////////////////////////////
// COfflineJournalPrinter
//////////////////////////////////////////////////////////////////////

COfflineJournalPrinter::COfflineJournalPrinter(const char *pDocFileName)
    : CJournalPrinter(pDocFileName)
{
    this->m_pWorkThread = new CWorkThead(this);

    m_pFile                 = NULL;
}

COfflineJournalPrinter::~COfflineJournalPrinter()
{
    this->Close();
    if (NULL != this->m_pWorkThread)
    {
        delete this->m_pWorkThread;
        this->m_pWorkThread = NULL;
    }
}

int COfflineJournalPrinter::Open(const char *pDllName, const char *pDeviceName, int nPort, int nBauRate)
{
    QMutexLocker locker(&this->m_syncDevMutex);
    int nRet = CJournalPrinter::Open(pDllName, pDeviceName, nPort, nBauRate);


    this->StartWork();
    if (0 != nRet)
    {
        return nRet;
    }
    return nRet;
}
void COfflineJournalPrinter::Close()
{
    this->EndWork();
    QMutexLocker locker(&this->m_syncDevMutex);
    CJournalPrinter::Close();
}
int COfflineJournalPrinter::Init()
{
    if (this->OfflineFileReachLimit())
    {
        QMutexLocker locker(&this->m_syncDevMutex);
        return CJournalPrinter::Init();
    }
    else
    {
        return 0;
    }
}
int COfflineJournalPrinter::Print(const char *pStr, int nLen)
{
    if (NULL != this->m_pFile)
    {
        // WriteToDoc(pStr, nLen);
        // modified by liy 20150109 解决日志打印机打印大数据时重复打印问题
        if (!m_pFile->AddLines(pStr, nLen))
        {
            return ERR_PTR_OTHER;
        }
        return 0;
    }
    else
    {
        QMutexLocker locker(&this->m_syncDevMutex);
        return CJournalPrinter::Print(pStr, nLen);
    }
}
int COfflineJournalPrinter::CutPaper(BOOL bDetectBlackStripe, int nFeedPace)
{
    QMutexLocker locker(&this->m_syncDevMutex);
    return CJournalPrinter::CutPaper(bDetectBlackStripe, nFeedPace);
}
void COfflineJournalPrinter::UpdateStatus()
{
    QMutexLocker locker(&this->m_syncDevMutex);
    CJournalPrinter::UpdateStatus();
}

bool COfflineJournalPrinter::CanOfflineWork()
{
    return (NULL != m_pFile);
}
bool COfflineJournalPrinter::OfflineFileReachLimit()
{
    return (NULL == m_pFile || m_pFile->GetCount() >= m_pFile->GetMaxCount());
}
BOOL COfflineJournalPrinter::IsJournalPrinter()
{
    return 2;   // this return value indicate OFFLINE printer
}
void COfflineJournalPrinter::GetActualStatus(int *pPrinterStatus, int *pPaperStatus, int *pTonerStatus)
{
    QMutexLocker locker(&this->m_syncDevMutex);
    CJournalPrinter::GetStatus(pPrinterStatus, pPaperStatus, pTonerStatus);
}
int COfflineJournalPrinter::GetOfflineSleepTime() const
{
    return this->m_nContinuousPrintBreakTime;
}
int COfflineJournalPrinter::GetOfflineUpdateStatusTime() const
{
    return this->m_nUpdateStatusTime;
}
int COfflineJournalPrinter::GetOfflineWorkTime() const
{
    return this->m_nContinuousPrintTime;
}
int COfflineJournalPrinter::GetofflineListBackTime() const
{
    return this->m_nPackTime;
}

int COfflineJournalPrinter::PrintToDev()
{
    const char *const ThisModule = "PrintToDev";
    QString strData = this->m_pFile->GetHead();

    QTextCodec *codec = QTextCodec::codecForLocale();
    QTextCodec *codec1 = QTextCodec::codecForName("gbk");
    if (NULL == codec1)
    {
        log_write(LOGFILE, ThisFile, ThisModule, -1, "无效的字符集gbk");
        return (-14);
    }

    QMutexLocker locker(&this->m_syncDevMutex);
    QTextCodec::setCodecForLocale(codec1);
    QByteArray byteData = strData.toLocal8Bit();
    int nRet =  CJournalPrinter::PrintNoWriteDoc(byteData.data(), byteData.size());
    QTextCodec::setCodecForLocale(codec);
    return nRet;
}
void COfflineJournalPrinter::RemovePrintData()
{
    this->m_pFile->RemoveHead();
}
bool COfflineJournalPrinter::HasPrintData()
{
    if (0 < this->m_pFile->GetCount())
    {
        return true;
    }
    else
    {
        return false;
    }
}
void COfflineJournalPrinter::Pack()
{
    this->m_pFile->Pack();
}
void COfflineJournalPrinter::GetStatus(int *pPrinterStatus, int *pPaperStatus, int *pTonerStatus)
{
    QMutexLocker locker(&this->m_syncDevMutex);
    CJournalPrinter::GetStatus(pPrinterStatus, pPaperStatus, pTonerStatus);
    if (this->OfflineFileReachLimit())  // 如果达到限制，返回实际的设备状态
    {
        return;
    }
    if (pPrinterStatus)             // 向上返回正确的状态，确保应用正确运行
    {
        *pPrinterStatus = PTR_DEV_ONLINE;
    }
    if (pPaperStatus &&
        PTR_PAPER_FULL != *pPaperStatus  &&
        PTR_PAPER_LOW  != *pPaperStatus)
    {
        *pPaperStatus = PTR_PAPER_FULL;
    }
    if (pTonerStatus &&
        PTR_TONER_FULL != *pTonerStatus &&
        PTR_TONER_LOW  != *pTonerStatus)
    {
        *pTonerStatus = PTR_TONER_FULL;
    }
}

void COfflineJournalPrinter::PreStartWork()
{
    char OfflineJournalFile[MAX_PATH + 1] = {'\0'};

    strncpy(OfflineJournalFile, GetSPOfflineKeyValue(OFFLINE_FILE_NAME_VALUE_NAME,
                                                     OFFLINE_PRINT_FILE_NAME_DEF), sizeof(OfflineJournalFile) - 1);

    this->m_nPackTime   = GetSPOfflineKeyValue(PACK_TIME_VALUE_NAME, OFFLINE_FILE_PACK_TIME_DEF);
    this->m_nUpdateStatusTime = GetSPOfflineKeyValue(UPDATE_STATUS_TIME_VALUE_NAME,
                                                     OFFLINE_FILE_UPDATE_STATUS_TIME_DEF);
    this->m_nContinuousPrintTime = GetSPOfflineKeyValue(CONT_PRINT_TIME_VALUE_NAME, 20);
    this->m_nContinuousPrintBreakTime = GetSPOfflineKeyValue(CONT_PRINT_BREAK_VALUE_NAME, 3);

    this->m_nPackTime *= 1000;
    this->m_nUpdateStatusTime *= 1000;
    this->m_nContinuousPrintTime *= 1000;
    this->m_nContinuousPrintBreakTime *= 1000;

    // 打开文件
    m_pFile = new COfflinePrintFile(OfflineJournalFile);

}

BOOL COfflineJournalPrinter::StartWork()
{
    this->PreStartWork();
    this->m_pWorkThread->startWork();
}

void COfflineJournalPrinter::EndWork()
{
    this->m_pWorkThread->stopWork();

    if (NULL != this->m_pFile)
    {
        delete this->m_pFile;
    }
    this->m_pFile = NULL;
}

