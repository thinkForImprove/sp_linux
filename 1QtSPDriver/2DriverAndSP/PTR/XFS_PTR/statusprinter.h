#ifndef STATUSPRINTER_H
#define STATUSPRINTER_H
//#include "DeviceDLLLoader.h"
//#include "iprinterdevice.h"
//#include "XDTSPUtil.h"
//#include "string.h"
//#include "log_lib.h"

#include <string.h>
#include <errno.h>
#include <sys/stat.h>
#include <asm/errno.h>
#include <QTime>
#include <QDate>


inline bool PTR_SUCC(int nRet)
{
    return ERR_PTR_CHRONIC == nRet || ERR_PTR_SUCCESS == nRet;
}

class CStatusPrinter
{
public:
    CStatusPrinter(): m_bNeedRepair(TRUE)
    {
        m_nPort = 0;
        m_nBauRate = 0;
    }
    virtual~CStatusPrinter()
    {
        Close();
    }
public:
    // 是否为流水打印机
    virtual BOOL IsJournalPrinter() const = 0;

    // 返回：0 成功；1 DLL装入失败；2 入口函数不存在; 3 创建实例失败; <0 其它错误
    virtual int Open(const char *pDllName, const char *pDeviceName, int nPort, int nBauRate)
    {
        int nRet = m_Printer.Load(pDllName, "CreatePrinterDevice", pDeviceName);
        if (0 > nRet)
        {
            return 3;
        }
        if (0 < nRet)
        {
            return nRet;
        }
        nRet = m_Printer->Open(nPort, nBauRate);
        m_nPort = nPort;
        m_nBauRate = nBauRate;
        m_bNeedRepair = FALSE;
        return nRet;
    }
    virtual void Close()
    {
        if (!(!m_Printer))
        {
            m_Printer->Close();
        }
        m_Printer.Release();
    }
    // 初始化设备
    // 返回：>=0成功，<0失败
    virtual int Init()
    {
        int nRet;
        if (TestAndRepairCom())
        {
            nRet = m_Printer->Init();
            if (ERR_PTR_COMM_ERR == nRet)
            {
                m_bNeedRepair = TRUE;
            }

            return nRet;
        }
        return ERR_PTR_COMM_ERR;
    }

    // 打印字串
    // pStr，要打印的字串，nLen，数据长度，如果为-1，pStr以\0结束
    // 返回：>=0成功，<0失败
    virtual int Print(const char *pStr, int nLen)
    {
        if (TestAndRepairCom())
        {
            int nRet = m_Printer->Print(pStr, nLen);
            if (ERR_PTR_COMM_ERR == nRet)
            {
                m_bNeedRepair = TRUE;
            }

            return nRet;
        }
        return ERR_PTR_COMM_ERR;
    }
    // add by huanghc for 新增图像打印 20090326
    // szImagePath:图片路径
    // nDstWidth: 期望的宽度
    // nDstHeight:期望的高度
    virtual int PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight)
    {
        if (TestAndRepairCom())
        {
            int nRet = m_Printer->PrintImage(szImagePath, nDstWidth, nDstHeight);
            if (ERR_PTR_COMM_ERR == nRet)
            {
                m_bNeedRepair = TRUE;
            }

            return nRet;
        }
        return ERR_PTR_COMM_ERR;
    }
    //　切纸
    //　bDetectBlackStripe，是否检测黑标
    //　nFeedPace，切纸前走纸的步数
    //　返回：>=0成功，<0失败
    virtual int CutPaper(BOOL bDetectBlackStripe, int nFeedPace)
    {
        if (TestAndRepairCom())
        {
            int nRet = m_Printer->CutPaper(bDetectBlackStripe, nFeedPace);
            if (ERR_PTR_COMM_ERR == nRet)
            {
                m_bNeedRepair = TRUE;
            }
            return nRet;
        }
        return ERR_PTR_COMM_ERR;
    }
    // 得到打印机状态，保存到成员变量中
    virtual void UpdateStatus()
    {
        if (TestAndRepairCom())
        {
            m_Printer->UpdateStatus();
        }
    }
    BOOL TestAndRepairCom()
    {
        if (m_bNeedRepair)
        {
            m_Printer->Close();
            int nRet = m_Printer->Open(m_nPort, m_nBauRate);
            if (0 > nRet)
            {
                return FALSE;
            }
            m_bNeedRepair = FALSE;
        }
        return TRUE;
    }
    virtual void GetStatus(int *pPrinterStatus, int *pPaperStatus, int *pTonerStatus)
    {
        if (!m_Printer)
        {
            if (NULL != pPrinterStatus)
            {
                *pPrinterStatus = PTR_DEV_HWERROR;
            }
            if (NULL != pPaperStatus)
            {
                *pPaperStatus = PTR_PAPER_UNKNOWN;
            }
            if (NULL != pTonerStatus)
            {
                *pTonerStatus = PTR_TONER_FULL;
            }
            return;
        }
        m_Printer->GetStatus(pPrinterStatus, pPaperStatus, pTonerStatus);
    }
private:
    //CDeviceDLLLoader<IPrinterDevice> m_Printer;
    BOOL                             m_bNeedRepair;
    int                              m_nPort;
    int                              m_nBauRate;
};

class CReceiptPrinter : public CStatusPrinter
{
public:
    CReceiptPrinter()
    {
    }
    virtual ~CReceiptPrinter()
    {
    }
public:
    virtual BOOL IsJournalPrinter() const
    {
        return FALSE;
    }
};

class CJournalPrinter : public CStatusPrinter
{
public:
    CJournalPrinter(const char *pDocFileName)
    {
        memset(&m_tLastWrite, 0x00, sizeof(m_tLastWrite));
        strncpy(m_DocFileName, pDocFileName, MAX_PATH);
        m_DocFileName[MAX_PATH] = '\0';
    }
    virtual ~CJournalPrinter()
    {
    }
public:
    virtual BOOL IsJournalPrinter() const
    {
        return TRUE;
    }

    virtual int CutPaper(BOOL bDetectBlackStripe, int nFeedPace)
    {
        Q_UNUSED(bDetectBlackStripe);
        Q_UNUSED(nFeedPace);
        return ERR_PTR_SUCCESS;
    }

    virtual int Print(const char *pStr, int nLen)
    {
        // 20091109 在WriteToDoc函数内部读注册表标志，同时控制在线和离线是否记录电子日志
        // 注册表默认配置为1，表示都记录电子日志
        // BOOL bWriteToDoc = atol((GetXdtDSRegValue(NULL, "PrinterWriteToDoc", "0")));
        // if (bWriteToDoc)
        // WriteToDoc(pStr, nLen);
        return CStatusPrinter::Print(pStr, nLen);
    }
protected:
    int PrintNoWriteDoc(const char *pStr, int nLen)
    {
        return CStatusPrinter::Print(pStr, nLen);
    }

    void WriteToDoc(const char *pStr, int nLen)
    {

        const char *const ThisFile = "JournalPrinter";
        const char *const ThisModule = "WriteToDoc";

        // 20091109 在WriteToDoc函数内部读注册表标志，同时控制在线和离线是否记录电子日志避免补打重复记录电子日志
        // 0表示都不记，注册表默认配置为1，表示都记录电子日志
        // 此处由于获取不到sp配置名，暂时写死
        BOOL bWriteToDoc = atol((GetSPIniValue("ndt_ptr_jp76_sp", "default", "printer_writetodoc", "1")));
        if (!bWriteToDoc)
        {
            return;
        }

        if ('\0' == pStr[0] || '\0' == m_DocFileName[0])
        {
            return;
        }

        if ((-1) == nLen)
        {
            nLen = strlen(pStr);
        }

        //*************format the filename************
        QDate tCurDate = QDate::currentDate();
        if (tCurDate.year() == m_tLastWrite.year &&
            tCurDate.month() == m_tLastWrite.month &&
            tCurDate.day() == m_tLastWrite.day)     // 日期未变，用老的文件名
        {
        }
        else
        {
            char szJournalDir[256] = {0};
            sprintf(szJournalDir, "%s\\Journal",  LOGPATH);

            if ((-1) == mkdir(szJournalDir, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH))
            {
                if (EEXIST != errno)
                {
                    log_write(LOGFILE, ThisFile, ThisModule, -1,
                              "创建打印映像文件夹失败:CreateDirectory(%s ...):errno=%d",
                              m_DocFileName, errno);
                }
            }

            sprintf(m_DocFileName, "%s\\Journal%04d%02d%02d.doc", szJournalDir, tCurDate.year(), tCurDate.month(), tCurDate.day());
            m_tLastWrite.year = tCurDate.year();
            m_tLastWrite.month = tCurDate.month();
            m_tLastWrite.day   = tCurDate.day();
        }

        //********************************************
        FILE *fp = fopen(m_DocFileName, "a+t");
        if (NULL == fp)
        {
            // 故障
            log_write(LOGFILE, ThisFile, ThisModule, -1,
                      "打开打印映像文件失败:fopen(%s ...):errno=%d",
                      m_DocFileName, errno);
        }
        else if (fwrite(pStr, sizeof(char), nLen, fp) != (size_t)nLen)
        {
            log_write(LOGFILE, ThisFile, ThisModule, -1, \
                      "写打印映像文件失败:fwrite(%60.60s ...)", pStr);
        }
        if (NULL != fp)
        {
            fclose(fp);
        }
    }

    char                m_DocFileName[MAX_PATH + 1];
    SYSTEMTIME          m_tLastWrite;
};

#endif // STATUSPRINTER_H
