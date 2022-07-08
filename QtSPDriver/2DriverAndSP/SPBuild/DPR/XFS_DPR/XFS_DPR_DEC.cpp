/***************************************************************
* 文件名称：XFS_CSR_DEC.cpp
* 文件描述：文档打印模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年8月23日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_DPR.h"
#include "data_convertor.h"

//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_DPR::StartOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = 0;

    if (bReConn == FALSE)   // 非重连状态,避免重复调用
    {
        // Open前下传初始参数
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_pPrinter->SetData(m_stConfig.szSDKPath, DTYPE_LIB_PATH);
        }
    }

    // 打开连接
    nRet = m_pPrinter->Open(nullptr);
    if (nRet != PTR_SUCCESS)
    {
        if (bReConn == FALSE)   // 非重连状态
        {
            Log(ThisModule, __LINE__, "打开设备连接失败．ReturnCode:%d.", ConvertErrCode(nRet));
        } else
        {
            if (m_nReConRet != nRet)
            {
                Log(ThisModule, __LINE__, "断线重连:打开设备连接失败．ReturnCode:%d.", ConvertErrCode(nRet));
                m_nReConRet = nRet;
            }
        }

        return ConvertErrCode(nRet);
    }
    m_nReConRet = nRet;

    // 设置不需要Reset
    //m_bNeedReset = false;

    // 设备初始化
//    nRet = OnInit();
//    if (nRet != 0)
//    {
//        Log(ThisModule, __LINE__, "设备初始化失败．ReturnCode:%d.", nRet);
//        return ConvertErrCode(nRet);
//    }

    // 更新扩展状态
    CHAR szDevVer[128] = { 0x00 };
    m_pPrinter->GetVersion(szDevVer, sizeof(szDevVer) - 1, 4);

    // 固件版本
    CHAR szFWVer[128] = { 0x00 };
    m_pPrinter->GetVersion(szFWVer, sizeof(szFWVer) - 1, 2);

    m_cExtra.AddExtra("VRTCount", "1");
    //m_cExtra.AddExtra("VRT[00]_XFSDPR", (char*)byVRTU);
    m_cExtra.AddExtra("VRT[01]_DevDPR", szDevVer);
    m_cExtra.AddExtra("FirmwareVersion", szFWVer);
    m_cExtra.AddExtra("LastErrorCode", m_strLastErrorInfo.c_str());
    m_cExtra.AddExtra("LastErrorDetail", "");

    // 更新一次状态
    OnStatus();

    if (bReConn == FALSE)   // 非重连状态
    {
        Log(ThisModule, 1, "打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, 1, "断线重连:打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());
    }

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//-----------------------------------重载函数-------------------------------
//
HRESULT CXFS_DPR::Reset(DWORD dwMediaControl, USHORT usBinIndex)
{
    Q_UNUSED(usBinIndex);
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    Log(ThisModule, 1, IDS_INFO_RESET_DEVICE);

    int nRet = m_pPrinter->Reset();
    UpdateDeviceStatus();
    if (PTR_SUCCESS != nRet)
    {
        //if (m_stStatus.fwPaper[0] == WFS_ERR_PTR_PAPEROUT)   // 无纸
        if (ERR_PTR_NO_PAPER == nRet)
        {
            m_bNeedKeepJammedStatus = FALSE;
            Log(ThisModule, 1, IDS_INFO_RESET_INFO, nRet, dwMediaControl);
            return WFS_SUCCESS;
        }

        Log(ThisModule, __LINE__, IDS_ERR_REESET_ERROR, nRet);
        return WFS_ERR_HARDWARE_ERROR;

    }
    m_bReset = TRUE;

    m_bNeedKeepJammedStatus = FALSE;
    Log(ThisModule, 1, IDS_INFO_RESET_SUCCESS, nRet, dwMediaControl);
    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::SendRawData(WORD wInputData, ULONG ulSize, LPBYTE lpbData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = 0;
    int nPrintType = 0;
    int nPaperNum = 0;
    PANTUMPRINTDATA stPrint;

    if (wInputData != WFS_PTR_NOINPUTDATA)
    {
        Log(ThisModule, 1, "SendRawData: 入参错误<wInputData>, wInputData != WFS_PTR_NOINPUTDATA, ret WFS_ERR_INVALID_DATA");
        return WFS_ERR_INVALID_DATA;
    }

    QString qData = (LPSTR)lpbData;
//    for (int i = 0; i < ulSize; i++)
//        qData += QString::number(lpbData[i]);

    if (qData.isEmpty())
    {
        Log(ThisModule, 1, "SendRawData: 入参错误<lpbData>, qData is empty. ulSize=%d, ret WFS_ERR_INVALID_DATA", ulSize);
        return WFS_ERR_INVALID_DATA;
    }

    // 拆分";"字符串
    QMap<QString,QString> qKVmap;
    QStringList qlKVList = qData.split(";");
    QString qFile = "File[";
    m_qlRawDataFileList.clear();
    if (qlKVList.size() > 0)
    {
        QStringList qlKList;
        qlKList.clear();
        for (int i = 0; i < qlKVList.size(); i++)
        {
            // 拆分"k=v"字符串
            qlKList = qlKVList.at(i).split("=");
            qKVmap.insert(qlKList.at(0), qlKList.at(1));
        }

        if (qKVmap.size() > 0)
        {
            auto it = qKVmap.find("PrintType");
            if (it != qKVmap.end())
            {
                nPrintType = it.value().toInt();
                stPrint.type = nPrintType;
            }

            auto it2 = qKVmap.find("PaperNum");
            if (it2 != qKVmap.end())
            {
                nPaperNum = it2.value().toInt();
                stPrint.num = nPaperNum;
            }

            int j = 0;
            QString qOutFileName;
            while (true)
            {
                auto it3 = qKVmap.find(qFile + QString::number(j) + "]");
                if (it3 != qKVmap.end())
                {
                    if (!isFileValid(it3.value(), qOutFileName, j))
                    {
                        nRet = ERR_PTR_PARAM_ERR;
                        break;
                    }
                    m_qlRawDataFileList.push_back(qOutFileName);
                    j++;
                } else
                    break;
            }

            if (nRet != 0)
                return nRet;
        }
    }

    m_stPrint.Clear();
    memcpy(&m_stPrint, &stPrint, sizeof(stPrint));

    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::ExecShellBash(LPSTR szInCmd, LPSTR szOutCmd, int nOutDataLen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (szInCmd == nullptr)
        return ERR_PTR_PARAM_ERR;

    FILE *stream;
    stream = popen(szInCmd, "r");

    fread(szOutCmd, sizeof(char), nOutDataLen, stream);
    pclose(stream);

    return WFS_SUCCESS;
}

BOOL CXFS_DPR::isFileValid(QString fullFileName, QString& OutFileName, int nFileIndex)
{
    THISMODULE(__FUNCTION__);

    QFile RawFile;
    QFileInfo fileInfo(fullFileName);
    RawFile.setFileName(fullFileName);

    if(fileInfo.exists())
    {
        QString suffix = fileInfo.suffix();
        if (suffix.compare("html") != 0 &&
            suffix.compare("xml") != 0  &&
            suffix.compare("txt") != 0  &&
            suffix.compare("pdf") != 0  &&
            suffix.compare("log") != 0  &&
            suffix.compare("png") != 0  &&
            suffix.compare("jpg") != 0  &&
            suffix.compare("bmp") != 0) {
            Log(ThisModule, 1, "isFileExist: 文件存在,　但文件格式不支持或后缀名错误, 路径为:%s", fullFileName.toStdString().c_str());
            return FALSE;
        }

//        RawFile.open(QIODevice::ReadOnly);
//        QPdfWriter *pdfW = new QPdfWriter(&RawFile);
//        pdfW->setPageSize(QPagedPaintDevice::A4);
//        pdfW->setResolution(QPrinter::ScreenResolution);
//        pdfW->setPageMargins(QMarginsF(40, 40, 40, 40));

        // 转换HTML为PDF
        if (suffix.compare("html") == 0)
        {
            // 执行命令转换pdf
            //OutFileName = fullFileName.left(fullFileName.size() - 4).append("pdf");
            OutFileName.append(CONVERTFILEPATH)
                       .append("/html")
                       .append(QString::number(nFileIndex))
                       .append(".pdf");
            char szOutCmd[1024] = {0};
            QString qCmd = "wkhtmltopdf " + fullFileName + " " + OutFileName;
            if (ExecShellBash((LPSTR)qCmd.toStdString().c_str(),szOutCmd, 1024) != PTR_SUCCESS)
            {
                Log(ThisModule, 1, "文件存在,　html -> pdf 转换失败, 路径为:%s, 失败原因:%s",
                    fullFileName.toStdString().c_str(), szOutCmd);
                return FALSE;
            }

            QString qOutCmd = "";
            qOutCmd = szOutCmd;

            if (!qOutCmd.isEmpty())
            {
                Log(ThisModule, 1, "文件存在,　html -> pdf 转换失败, 路径为:%s, 失败原因:%s",
                    fullFileName.toStdString().c_str(), szOutCmd);
                return FALSE;
            }

//            需头文件//<QtPrintSupport/QPrinter>
                     //<QtPrintSupport/QtPrintSupport>
                     //<QPdfWriter>
                     //<QTextDocument>
//            if (PrinterHTMLFormat(qHTML) != PTR_SUCCESS)
//            {
//                Log(ThisModule, 1, "打印html文件失败, 路径为:%s", fullFileName.toStdString().c_str());
//                return FALSE;
//            }

        } else if (suffix.compare("xml") == 0) {
            // xml -> txt

            QString qXML;
            bool ok = RawFile.open(QIODevice::ReadOnly);
            if (!ok)
            {
                Log(ThisModule, 1, "xml文件打开失败, 路径为:%s", fullFileName.toStdString().c_str());
                return FALSE;
            }

            while(!RawFile.atEnd())
            {
                QString line = RawFile.readLine();
                qXML += line;
            }

            if (qXML.isEmpty())
            {
                Log(ThisModule, 1, "xml文件存在,　但内容为空, 路径为:%s", fullFileName.toStdString().c_str());
                return FALSE;
            }

            //OutFileName = fullFileName.left(fullFileName.size() - 3).append("txt");
            OutFileName.append(CONVERTFILEPATH)
                       .append("/xml")
                       .append(QString::number(nFileIndex))
                       .append(".txt");
            if (PrinterXMLFormat(OutFileName, qXML) != PTR_SUCCESS)
            {
                Log(ThisModule, 1, "打印xml文件失败, 路径为:%s", fullFileName.toStdString().c_str());
                return FALSE;
            }

            RawFile.close();
        } else {
            OutFileName = fullFileName;
        }

        Log(ThisModule, 1, "isFileExist: 文件存在,　路径为:%s", fullFileName.toStdString().c_str());
        return TRUE;
    }

    return FALSE;
}

//HRESULT CXFS_DPR::PrinterHTMLFormat(QString qHTML)
//{
//    QStringList qlPrintList = QPrinterInfo::availablePrinterNames();

//    QPrinter printer;
//    printer.printerName();

//    printer.setNumCopies(m_stPrint.num);
//    printer.setPageOrder(QPrinter::FirstPageFirst);

//    printer.setPageMargins(40, 40, 40, 40, QPrinter::Millimeter);
//    // 纵向打印
//    if (m_stPrint.type == 0 || m_stPrint.type == 4)
//        printer.setOrientation(QPrinter::Portrait);
//    else if (m_stPrint.type = 2)
//        printer.setOrientation(QPrinter::Landscape);
//    else
//        return -1;

//    printer.setPaperSource(QPrinter::PaperSource::Auto);
//    printer.setPageSize(QPrinter::A4);

//    QTextDocument *pDoc = new QTextDocument;
//    pDoc->setHtml(qHTML);
//    pDoc->print(&printer);
//    pDoc->end();

//    return PTR_SUCCESS;
//}

HRESULT CXFS_DPR::PrinterXMLFormat(QString filename, QString qXML)
{
    THISMODULE(__FUNCTION__);

    if (qXML.isEmpty())
    {
        Log(ThisModule, 1, "读取xml文件失败, 路径为:%s");
        return ERR_PTR_DATA_ERR;
    }

    QFile file;
    file.setFileName(filename);
    if (!file.exists())
    {
        bool ok = file.open(QIODevice::WriteOnly | QIODevice::Text);
        if (!ok)
        {
            Log(ThisModule, 1, "创建文件<%s>失败", filename.toStdString().c_str());
            return ERR_PTR_CHRONIC;
        }

        file.write(qXML.toStdString().data(), qXML.length());
    } else {
        file.open(QIODevice::WriteOnly | QIODevice::Text);
        file.write(qXML.toStdString().data(), qXML.length());
    }

    file.close();
    return PTR_SUCCESS;
}

void CXFS_DPR::ClearFileList()
{

    if (m_stConfig.wRemoveFile != 0)
    {
        for (int i = 0; i < m_qlRawDataFileList.size(); i++)
        {
            remove(m_qlRawDataFileList.at(i).toStdString().c_str());
        }

        m_qlRawDataFileList.clear();
        return;
    }

    for (int i = 0; i < m_qlRawDataFileList.size(); i++)
    {
        DelConvertFormatFile(m_qlRawDataFileList.at(i));
    }

    m_qlRawDataFileList.clear();
}

void CXFS_DPR::DelConvertFormatFile(const QString &qFile)
{
    QFileInfo file(qFile);
    if (!file.exists())
    {
        return;
    }

    if (!qFile.contains("/usr/local/CFES/DATA/FORM/DPR"))
        return;

    CHAR    szCmd[MAX_PATH] = { 0x00 };
    char    szOutData[MAX_EXT] = { 0x00 };

    //sprintf(szCmd, "rm -rf %s/*.*", m_stConfig.szConvertFilePath);
    sprintf(szCmd, "rm -rf %s", qFile.toStdString().c_str());
    ExecShellBash(szCmd, szOutData, MAX_EXT);
}

HRESULT CXFS_DPR::EndForm(PrintContext *pContext)
{
    THISMODULE(__FUNCTION__);
    return WFS_SUCCESS;
}

inline int MulDiv(int number, int numberator, int denominator)
{
    long long ret = number;
    ret *= numberator;
    //ret += numberator;
    if (0 == denominator)
    {
        ret = (-1);
    }
    else
    {
        ret /= denominator;
    }
    return (int) ret;
}


//-----------------------------------------------------------------------------------
//--------------------------------------功能处理---------------------------------------
// 读INI
void CXFS_DPR::InitConifig()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    CHAR    szIniAppName[256];

    // 底层设备控制动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 设备类型(0/Pantum P3018D)
    m_stConfig.nDriverType = m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (INT)0);

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.nOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (INT)0);
    if (m_stConfig.nOpenFailRet != 0 && m_stConfig.nOpenFailRet != 1)
    {
        m_stConfig.nOpenFailRet = 0;
    }

    // ----------------------------------------设备分类相关信息获取----------------------------------------
    // 根据设备类型获取相关参数
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDriverType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    // 底层设备控制动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 获取P3018D设备INI设置
    if (m_stConfig.nDriverType == IXFSCSR_TYPE_P3018D)
    {
        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDriverType);
    }

    // ----------------------------------------银行分类相关信息获取----------------------------------------
    // 根据银行获取特殊设置
    m_stConfig.usBank = m_cXfsReg.GetValue("BANK_CONFIG", "BankNo", (INT)0);

    // 设置指定银行INI AppName
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "BANK_SET_%d", m_stConfig.usBank);

    // ----------------------------------------其他相关信息获取----------------------------------------
    // 指定银行，特殊需求
    m_stConfig.usBank = m_cXfsReg.GetValue("BANK_CONFIG", "BankNo", (DWORD)0);

    // 打印完成是否删除文件
    m_stConfig.wRemoveFile = m_cXfsReg.GetValue("PRINTER_OPTION", "PrinterSuccRemove", (DWORD)0);

    // 设置当前打印任务优先级
    m_stConfig.wJobPriority = m_cXfsReg.GetValue("PRINTER_OPTION", "JobPriority", (DWORD)0);
    if (m_stConfig.wJobPriority > 100 || m_stConfig.wJobPriority < 0)
        m_stConfig.wJobPriority = 50;

    // 打印机名称
    strcpy(m_stConfig.szPrinterName, m_cXfsReg.GetValue("PRINTER_NAME", "PrinterName", ""));

    // 打印时用于转换后文件存放路径
    strcpy(m_stConfig.szConvertFilePath, m_cXfsReg.GetValue("PRINTER_NAME", "ConvertFilePath", CONVERTFILEPATH));
    if (strlen((char*)m_stConfig.szConvertFilePath) < 2 ||
        m_stConfig.szConvertFilePath[0]  != '/') {
        memset(m_stConfig.szConvertFilePath, 0x00, sizeof(m_stConfig.szConvertFilePath));
        memcpy(m_stConfig.szConvertFilePath, CONVERTFILEPATH, strlen(CONVERTFILEPATH));
    }
}

// 初始化状态类变量
long CXFS_DPR::InitStatus()
{
    memset(&m_stStatus, 0x00, sizeof(WFSPTRSTATUS));
    m_stStatus.fwDevice      = WFS_PTR_DEVNODEVICE;
    for (INT i = 0; i < WFS_PTR_SUPPLYSIZE; i ++)
    {
        m_stStatus.fwPaper[i]    = WFS_PTR_PAPERUNKNOWN;
    }
    m_stStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
    m_stStatus.fwToner       = WFS_PTR_TONERUNKNOWN;
    m_stStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
    m_stStatus.fwInk         = WFS_PTR_INKUNKNOWN;
    m_stStatus.lppRetractBins = nullptr;
}

// 初始化能力值类变量
long CXFS_DPR::InitCaps()
{
    memset(&m_sCaps, 0x00, sizeof(WFSPTRCAPS));
    m_sCaps.wClass                = WFS_SERVICE_CLASS_PTR;
    m_sCaps.fwType                = WFS_PTR_TYPEDOCUMENT;
    m_sCaps.bCompound             = TRUE;
    m_sCaps.wResolution           = WFS_PTR_RESMED;
    m_sCaps.fwReadForm            = WFS_PTR_READPAGEMARK;
    m_sCaps.fwWriteForm           = WFS_PTR_WRITETEXT;
    m_sCaps.fwExtents             = FALSE;
    m_sCaps.fwControl             = WFS_PTR_CTRLEJECT;
    m_sCaps.usMaxMediaOnStacker   = FALSE;
    m_sCaps.bAcceptMedia          = TRUE;
    m_sCaps.bMultiPage            = TRUE;
    m_sCaps.fwPaperSources        = WFS_PTR_PAPERUPPER;
    m_sCaps.bMediaTaken           = TRUE;
    m_sCaps.usRetractBins         = 0;
    m_sCaps.lpusMaxRetract        = nullptr;
    m_sCaps.fwImageType           = FALSE;
    m_sCaps.fwFrontImageColorFormat = FALSE;
    m_sCaps.fwBackImageColorFormat = FALSE;
    m_sCaps.fwCodelineFormat      = FALSE;
    m_sCaps.fwImageSource         = 0;
    m_sCaps.fwCharSupport         = WFS_PTR_ASCII;
    m_sCaps.bDispensePaper        = TRUE;

    m_cExtra.AddExtra("bCountPaper", "1");
    m_cExtra.AddExtra("bStamp", "0");
    m_sCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
}

// 状态获取
void CXFS_DPR::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题

    INT nRet = PTR_SUCCESS;
    DEVPTRSTATUS stDevStatus;
    BOOL    bNeedFirePrinterStatus  = FALSE;    // 需要上报打印状态变化事件
    BOOL    bNeedFirePaperStatus[16];           // 需要上报票箱状态变化事件
    BOOL    bNeedFireTonerStatus    = FALSE;    // 需要上报碳带状态变化事件
    BOOL    bNeedFireInkStatus      = FALSE;    // 需要上报墨盒状态变化事件
    BOOL    bNeedFireRetractStatus[16];         // 需要上报回收箱状态变化事件
    BOOL    bNeedFireHWError        = FALSE;    // 需要上报硬件错误事件
    BOOL    bNeedFireMediaTaken     = FALSE;    // 需要上报介质拿走事件
    BOOL    bNeedFireMediaInsert    = FALSE;    // 需要上报介质放入事件
    memset(bNeedFirePaperStatus, FALSE, sizeof(bNeedFirePaperStatus));
    memset(bNeedFireRetractStatus, FALSE, sizeof(bNeedFireRetractStatus));


    WFSPTRSTATUS stLastStatus = m_stStatus;

    nRet = m_pPrinter->GetStatus(stDevStatus);
    if (nRet != PTR_SUCCESS && nRet != ERR_PTR_CANCEL)
    {
        if (nRet == ERR_PTR_NOT_OPEN)
        {
            m_stStatus.fwDevice = WFS_PTR_DEVOFFLINE;
        } else
        if (nRet == ERR_PTR_NO_DEVICE)
        {
            m_stStatus.fwDevice = WFS_PTR_DEVNODEVICE;
        } else
        if (nRet == ERR_PTR_DEVBUSY){
            m_stStatus.fwDevice = WFS_PTR_DEVBUSY;
        } else
            m_stStatus.fwDevice = WFS_PTR_DEVHWERROR;
        for (INT i = 0; i < WFS_PTR_SUPPLYSIZE; i ++)
        {
            m_stStatus.fwPaper[i]    = WFS_PTR_PAPERUNKNOWN;
        }
        m_stStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
        m_stStatus.fwToner       = WFS_PTR_TONERUNKNOWN;
        m_stStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
        m_stStatus.fwInk         = WFS_PTR_INKUNKNOWN;
        m_stStatus.lppRetractBins = nullptr;
    } else
    {
        m_stStatus.fwDevice = ConvertDeviceStatus(stDevStatus.wDevice);
        m_stStatus.fwMedia = ConvertMediaStatus(stDevStatus.wMedia);
        for (INT i = 0; i < 16; i ++)
        {
            m_stStatus.fwPaper[i] = ConvertPaperStatus(stDevStatus.wPaper[i]);
        }
        m_stStatus.fwToner = ConvertTonerStatus(stDevStatus.wToner);
        m_stStatus.fwInk = ConvertInkStatus(stDevStatus.wInk);
        m_stDPRStatus = stDevStatus.stDPRStatus;

        //--------事件检查处理--------
        // Media原状态为出口+通道内无票＆n_WaitTaken为准执行Taken,设置Taken事件上报标记
        if ((m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT) &&
            (stLastStatus.fwMedia == WFS_PTR_MEDIAENTERING) && m_WaitTaken == WTF_TAKEN)
        {
            bNeedFireMediaTaken = TRUE;
            m_WaitTaken = WTF_NONE;
        }

        // 票箱变化事件
        for (INT i = 0; i < 16; i ++)
        {
            if (m_stStatus.fwPaper[i] != stLastStatus.fwPaper[i])
            {
                if (m_stStatus.fwPaper[i] == WFS_PTR_PAPERLOW ||
                    m_stStatus.fwPaper[i] == WFS_PTR_PAPEROUT ||
                    m_stStatus.fwPaper[i] == WFS_PTR_PAPERFULL)
                {
                    bNeedFirePaperStatus[i] = TRUE;
                }
            }
        }

        // 碳带变化事件
        if (m_stStatus.fwToner != stLastStatus.fwToner)
        {
            if (m_stStatus.fwToner == WFS_PTR_TONERLOW ||
                m_stStatus.fwToner == WFS_PTR_TONEROUT ||
                m_stStatus.fwToner == WFS_PTR_TONERFULL)
            {
                bNeedFireTonerStatus = TRUE;
            }
        }

        // 墨盒变化事件
        if (m_stStatus.fwInk != stLastStatus.fwInk)
        {
            if (m_stStatus.fwToner == WFS_PTR_INKLOW ||
                m_stStatus.fwToner == WFS_PTR_INKOUT ||
                m_stStatus.fwToner == WFS_PTR_INKFULL)
            {
                bNeedFireInkStatus = TRUE;
            }
        }

        // Device状态有变化&当前Device状态为HWERR,设置HWERR事件上报标记
        if (m_stStatus.fwDevice != stLastStatus.fwDevice)
        {
            if (m_stStatus.fwDevice != WFS_PTR_DEVBUSY && m_stStatus.fwDevice != WFS_PTR_DEVBUSY)
                bNeedFirePrinterStatus = TRUE;
            if (m_stStatus.fwDevice == WFS_PTR_DEVHWERROR)
            {
                bNeedFireHWError = TRUE;
            }
        }

        //--------事件上报处理--------

        if (bNeedFireHWError)           // 上报Device HWERR事件
        {
            FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
        }

        if (bNeedFirePrinterStatus)     // 上报状态变化事件
        {
            FireStatusChanged(m_stStatus.fwDevice);
        }

        /*for (INT i = 0; i < 16; i ++)           // 上报票箱状态变化
        {
            if (bNeedFirePaperStatus[i] == TRUE && ConvertPaperCode(i + 1) != 0)
            {
                FirePaperThreshold(ConvertPaperCode(i + 1), m_stStatus.fwPaper[i]);
            }
        }*/

        if (bNeedFireTonerStatus)       // 上报碳带状态变化
        {
            FireTonerThreshold(m_stStatus.fwToner);
        }

        if (bNeedFireInkStatus)         // 上报墨盒状态变化
        {
            FireInkThreshold(m_stStatus.fwInk);
        }

        if (bNeedFireMediaTaken)        // 上报Taken事件
        {
            FireMediaTaken();
            Log(ThisModule, 1, IDS_INFO_PAPER_TAKEN);
        }

        if (bNeedFireMediaInsert)       // 上报Insert事件
        {
            FireMediaInserted();
        }
    }

    return;
}

// 介质控制处理
HRESULT CXFS_DPR::ControlMedia(DWORD dwControl)
{
    THISMODULE(__FUNCTION__);

    HRESULT hRes = PTR_SUCCESS;
    char pszOutData[4098] = { 0x00 };

    if ((dwControl & WFS_PTR_CTRLFLUSH) == WFS_PTR_CTRLFLUSH)
    {
        // 执行打印命令
        if (m_pszInData == nullptr)
        {
            Log(ThisModule, hRes, "->malloc() Fail, 申请内存失败, m_pszInData == nullptr. ret=WFS_ERR_INTERNAL_ERROR");
            return WFS_ERR_INTERNAL_ERROR;
        }

        hRes = ExecShellBash(m_pszInData, pszOutData, 4098);
        if (hRes != WFS_SUCCESS)
        {
            Log(ThisModule, hRes, "->ExecShellBash() Fail, ErrCode = %d, Return: %d.",
                hRes, ConvertErrCode(hRes));
            return ConvertErrCode(hRes);
        }

        QString pResult = pszOutData;
        if (pResult.contains("lp: 错误"))
        {
            Log(ThisModule, hRes, "->ExecShellBash() Fail, 打印失败, Return: %s. ret=WFS_ERR_INTERNAL_ERROR",
                pszOutData);
            return WFS_ERR_INTERNAL_ERROR;
        }

        ClearFileList();
    } else {
        // 无效入参
        Log(ThisModule, __LINE__, "接收ControlMedia参数[%d]无效, ReturnCode: %d.",
            dwControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    m_WaitTaken = WTF_TAKEN;
    if (m_pszInData != nullptr)
    {
        memset(m_pszInData, 0x00, strlen(m_pszInData + 1));
    }

    return WFS_SUCCESS;
}

// 格式化打印处理
HRESULT CXFS_DPR::InnerPrintForm(LPWFSPTRPRINTFORM pInData)
{
    THISMODULE(__FUNCTION__);
    return  WFS_SUCCESS;
}

HRESULT CXFS_DPR::InnerReadImage(LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    return  WFS_SUCCESS;
}

HRESULT CXFS_DPR::InnerReadForm(LPWFSPTRREADFORM pInData)
{
    THISMODULE(__FUNCTION__);
    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::InnerRawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    // 拆解字符串或JSON,整合数据组织SP内部使用字符串或JSON,调用IDevPTR->PrintData()接口打印
    // 为今后其他设备扩展接入的方便考虑,可参考CPR/CSR定义同一的JSON,避免XFS_XXX与DevXXX的重复修改
    if (m_stConfig.usBank == BANK_NO_SXXH)  // 山西信合
    {
        ;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::MediaRetract(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    THISMODULE(__FUNCTION__);
    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::MediaInsertWait(DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    return WFS_SUCCESS;
}

void CXFS_DPR::RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData)
{
    ulOutSize = 0;
    /*
     * 去除字符串中非ASCII码字符(0x00~0x7F)，去除非汉字字符(低字节A1~FE,高字节B0~F7)
     * 增加支持中文全角字符打印（低字节A0~FF,高字节A1~A3）
     */
    for (ULONG i = 0; i < ulInSize; i++)
    {
        if ((pInData[i] >= 0xB0) && (pInData[i] <= 0xF7)
            && ((pInData[i + 1] >= 0xA1) && (pInData[i + 1] <= 0xFE))
           )
        {
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
        }
        else if ((pInData[i] >= 0xA1) && (pInData[i] <= 0xA3)
                 && ((pInData[i + 1] >= 0xA1) && (pInData[i + 1] <= 0xFF))
                )
        {
            if (0xA1 == pInData[i])
            {
                if (0xA0 == pInData[i + 1] || 0xA1 == pInData[i + 1] || 0xFF == pInData[i + 1])
                {
                    i++;
                    continue;
                }
            }
            else if (0xA2 == pInData[i])
            {
                if (0xA0 == pInData[i + 1] || 0xAB == pInData[i + 1] || 0xAC == pInData[i + 1] ||
                    0xAD == pInData[i + 1] || 0xAE == pInData[i + 1] || 0xAF == pInData[i + 1] ||
                    0xB0 == pInData[i + 1] || 0xEF == pInData[i + 1] || 0xF0 == pInData[i + 1] ||
                    0xFD == pInData[i + 1] || 0xFE == pInData[i + 1] || 0xFF == pInData[i + 1])
                {
                    i++;
                    continue;
                }
            }
            else if (0xA3 == pInData[i])
            {
                if (0xA0 == pInData[i + 1] || 0xFF == pInData[i + 1])
                {
                    i++;
                    continue;
                }
            }
            pOutData[ulOutSize++] = pInData[i];
            pOutData[ulOutSize++] = pInData[i + 1];
            i++;
        }
        else if (pInData[i] >= 0x80)
        {
            i++;
        }
        else if ((pInData[i] == 0x0A)
                 || ((pInData[i] > 0x1F) && (pInData[i] < 0x7F)))
        {
            pOutData[ulOutSize++] = pInData[i];
        }
    }
}

// 设置回收箱计数并记录
void CXFS_DPR::SetRetractBoxCount(USHORT usBoxNo, USHORT usCnt, BOOL bIsAdd)
{
    THISMODULE(__FUNCTION__);

    return;
}

//-----------------------------------------------------------------------------------
//------------------------------------格式转换WFS-------------------------------------
// 错误码转换为WFS格式
INT CXFS_DPR::ConvertErrCode(INT nRet)
{
    switch (nRet)
    {
        case PTR_SUCCESS:           return WFS_SUCCESS;
        case ERR_PTR_PARAM_ERR:     return WFS_ERR_UNSUPP_DATA;
        case ERR_PTR_COMM_ERR:      return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_NO_PAPER:      return WFS_ERR_PTR_PAPEROUT;
        case ERR_PTR_JAMMED:        return WFS_ERR_PTR_PAPERJAMMED;
        case ERR_PTR_NOT_OPEN:      return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_HEADER:        return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_CUTTER:        return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_TONER:         return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_STACKER_FULL:  return WFS_ERR_PTR_STACKERFULL;
        case ERR_PTR_NO_RESUME:     return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_CAN_RESUME:    return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_FORMAT_ERROR:  return WFS_ERR_UNSUPP_DATA;
        case ERR_PTR_CHRONIC:       return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_HWERR:         return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_IMAGE_ERROR:   return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_UNSUP_CMD:     return WFS_ERR_UNSUPP_COMMAND;
        case ERR_PTR_NO_DEVICE:     return WFS_ERR_HARDWARE_ERROR;
        default:                    return WFS_ERR_HARDWARE_ERROR;
    }
}

// 设备状态转换为WFS格式
WORD CXFS_DPR::ConvertDeviceStatus(WORD wDevStat)
{
    switch (wDevStat)
    {
        case DEV_STAT_ONLINE     /* 设备正常 */     : return WFS_PTR_DEVONLINE;
        case DEV_STAT_OFFLINE    /* 设备脱机 */     : return WFS_PTR_DEVOFFLINE;
        case DEV_STAT_POWEROFF   /* 设备断电 */     : return WFS_PTR_DEVPOWEROFF;
        case DEV_STAT_NODEVICE   /* 设备不存在 */    : return WFS_PTR_DEVNODEVICE;
        case DEV_STAT_HWERROR    /* 设备故障 */     : return WFS_PTR_DEVHWERROR;
        case DEV_STAT_USERERROR  /*  */             : return WFS_PTR_DEVUSERERROR;
        case DEV_STAT_BUSY       /* 设备读写中 */    : return WFS_PTR_DEVBUSY;
        defaule: return WFS_PTR_DEVOFFLINE;
    }
}

// Media状态转换为WFS格式
WORD CXFS_DPR::ConvertMediaStatus(WORD wMediaStat)
{
    switch (wMediaStat)
    {
        case MEDIA_STAT_PRESENT   /* 通道内有票 */               : return WFS_PTR_MEDIAPRESENT;
        case MEDIA_STAT_NOTPRESENT/* 通道内无票 */               : return WFS_PTR_MEDIANOTPRESENT;
        case MEDIA_STAT_JAMMED    /* 通道内有票且票被夹住 */       : return WFS_PTR_MEDIAJAMMED;
        case MEDIA_STAT_NOTSUPP   /* 不支持检测通道内是否有票 */    : return WFS_PTR_MEDIANOTPRESENT;
        case MEDIA_STAT_UNKNOWN   /* 通道内票状态未知 */           : return WFS_PTR_MEDIAUNKNOWN;
        case MEDIA_STAT_ENTERING  /* 票在出票口 */                : return WFS_PTR_MEDIAENTERING;
        default: return WFS_PTR_MEDIAUNKNOWN;
    }
}

// Paper状态转换为WFS格式
WORD CXFS_DPR::ConvertPaperStatus(WORD wPaperStat)
{
    switch (wPaperStat)
    {
        case PAPER_STAT_FULL      /* 票据满 */          : return WFS_PTR_PAPERFULL;
        case PAPER_STAT_LOW       /* 票据少 */          : return WFS_PTR_PAPERLOW;
        case PAPER_STAT_OUT       /* 票据无 */          : return WFS_PTR_PAPEROUT;
        case PAPER_STAT_NOTSUPP   /* 设备不支持该能力 */  : return WFS_PTR_PAPERNOTSUPP;
        case PAPER_STAT_UNKNOWN   /* 不能确定当前状态 */  : return WFS_PTR_PAPERUNKNOWN;
        case PAPER_STAT_JAMMED    /* 票据被卡住 */       : return WFS_PTR_PAPERJAMMED;
        defaule: return WFS_PTR_PAPERUNKNOWN;
    }
}

// Toner状态转换为WFS格式
WORD CXFS_DPR::ConvertTonerStatus(WORD wTonerStat)
{
    switch (wTonerStat)
    {
        case TONER_STAT_FULL      /* 碳带状态满或正常 */    : return WFS_PTR_TONERFULL;
        case TONER_STAT_LOW       /* 碳带少 */            : return WFS_PTR_TONERLOW;
        case TONER_STAT_OUT       /* 碳带无 */            : return WFS_PTR_TONEROUT;
        case TONER_STAT_NOTSUPP   /* 设备不支持该能力 */    : return WFS_PTR_TONERNOTSUPP;
        case TONER_STAT_UNKNOWN   /* 不能确定当前状态 */    : return WFS_PTR_TONERUNKNOWN;
        default: return WFS_PTR_TONERUNKNOWN;
    }
}

// Ink状态转换为WFS格式
WORD CXFS_DPR::ConvertInkStatus(WORD wInkStat)
{
    switch (wInkStat)
    {
        case INK_STAT_FULL    /* 墨盒状态满或正常 */  : return WFS_PTR_INKFULL;
        case INK_STAT_LOW     /* 墨盒少 */           : return WFS_PTR_INKLOW;
        case INK_STAT_OUT     /* 墨盒无 */           : return WFS_PTR_INKOUT;
        case INK_STAT_NOTSUPP /* 设备不支持该能力 */  : return WFS_PTR_INKNOTSUPP;
        case INK_STAT_UNKNOWN /* 不能确定当前状态 */  : return WFS_PTR_INKUNKNOWN;
        default: return WFS_PTR_INKUNKNOWN;
    }
}

// Retract状态转换为WFS格式
WORD CXFS_DPR::ConvertRetractStatus(WORD wRetractStat)
{
    switch (wRetractStat)
    {
        case RETRACT_STAT_OK/* 回收箱正常 */   : return WFS_PTR_RETRACTBINOK;
        case RETRACT_STAT_FULL/* 回收箱满 */   : return WFS_PTR_RETRACTBINFULL;
        case RETRACT_STAT_HIGH/* 回收箱将满 */   : return WFS_PTR_RETRACTBINHIGH;
        default: return WFS_PTR_RETRACTBINOK;
    }
}

// 票箱号转换为WFS格式
WORD CXFS_DPR::ConvertPaperCode(INT nCode)
{
    switch (nCode)
    {
        case 1: return WFS_PTR_PAPERUPPER;      // 票箱1
        case 2: return WFS_PTR_PAPERLOWER;      // 票箱2
        case 3: return WFS_PTR_PAPEREXTERNAL;   // 票箱3
        case 4: return WFS_PTR_PAPERAUX;        // 票箱4
        default: return 0;
    }
}

// 指定票据类型转换为DevCPR定义
WORD CXFS_DPR::NoteTypeConvert(LPSTR lpNoteType, WORD wBank)
{
    #define IF_CMCP(X, Y, Z) \
        if (memcmp(X, Y, strlen(X)) == 0 && memcmp(X, Y, strlen(Y)) == 0) \
        return Z


}

// 指定票据类型是否对应票箱存在
WORD CXFS_DPR::NoteTypeIsHave(LPSTR lpNoteType, WORD wBox)
{
    // 票箱指定票据类型: 1普通存单/2芯片存单/3大额存单/4国债凭证/5结算业务委托书/6现金支票/7转账支票/
    //                 8清分机支票/9银行汇票/10银行承兑汇票/11商业承兑汇票/12非清分机本票/13清分机本票

    WORD wNoteTypeTmp = 0;

}

HRESULT CXFS_DPR::GetFormFieldToJSON(LPSTR lpForm, CJsonObject &cJson)
{
    THISMODULE(__FUNCTION__);

    // FORM行列值转换为MM(1MM单位),有小数位四舍五入
    #define ROWCOL2MM(ROL, MM) \
        rc.pForm->GetOrigUNIT(nullptr) == FORM_ROWCOLUMN ? \
            (int)((ROL * MM + 5) / 10) : ROL

    SIZE stSize = GetPerRowColTwips2MM();
    // 设置边界
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    // 加载Form文件
    m_pData->LoadForms();
    // 加载Media文件
    m_pData->LoadMedias();

    CSPPtrData *pData = static_cast<CSPPtrData *>(m_pData);

    ReadContext rc;
    ZeroMemory(&rc, sizeof(rc));

    rc.pForm = pData->FindForm(lpForm);
    if (!rc.pForm)
    {
        Log(ThisModule, __LINE__, IDS_ERR_FORM_NOT_FOUND, lpForm);
        return WFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!rc.pForm->IsLoadSucc())
    {
        Log(ThisModule, __LINE__, IDS_ERR_FORM_INVALID, lpForm);
        return WFS_ERR_PTR_FORMINVALID;
    }
    if (rc.pForm->GetSubItemCount() < 1) // 无Field
    {
        Log(ThisModule, __LINE__, IDS_ERR_FIELD_EMPTY, lpForm);
        return WFS_ERR_PTR_FORMINVALID;
    }

    // 循环取Field
    cJson.Clear();
    cJson.Add(JSON_KEY_USE_AREA, 1);
    cJson.Add(JSON_KEY_MEDIA_WIDTH, ROWCOL2MM(rc.pForm->GetOrigSize().cx, stSize.cx));    // 介质宽(单位:MM)
    cJson.Add(JSON_KEY_MEDIA_HEIGHT, ROWCOL2MM(rc.pForm->GetOrigSize().cy, stSize.cy));   // 介质高(单位:MM)
    ISPPrinterItem *pItem = nullptr;
    CHAR szIdenKey[32];
    DWORD iChild = 0;
    for (iChild = 0; iChild < rc.pForm->GetSubItemCount(); iChild++)
    {
        pItem = rc.pForm->GetSubItem(iChild);
        if (pItem == nullptr)
        {
            Log(ThisModule, __LINE__, "From[%s]->GetSubItem(%d) Fail", lpForm);
            return WFS_ERR_PTR_FIELDERROR;
        }
        // 项名
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_IDEN_NAME, iChild + 1);
        cJson.Add(szIdenKey, std::string(pItem->GetName()));
        // 起始坐标X(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_START_X, iChild + 1);
        cJson.Add(szIdenKey, ROWCOL2MM(pItem->GetOrigPosition().cx, stSize.cx));
        // 起始坐标Y(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_START_Y, iChild + 1);
        cJson.Add(szIdenKey, ROWCOL2MM(pItem->GetOrigPosition().cy, stSize.cy));
        // 可用宽(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_WIDTH, iChild + 1);
        cJson.Add(szIdenKey, ROWCOL2MM(pItem->GetOrigSize().cx, stSize.cx));
        // 可用高(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_HEIGHT, iChild + 1);
        cJson.Add(szIdenKey, ROWCOL2MM(pItem->GetOrigSize().cy, stSize.cy));
    }
    cJson.Add(JSON_KEY_IDEN_CNT, iChild);   // 项数目

    return WFS_SUCCESS;
}

HRESULT CXFS_DPR::GetDefFieldToJSON(LPSTR lpStr, CJsonObject &cJson)
{
    THISMODULE(__FUNCTION__);

    cJson.Clear();

    if (lpStr == nullptr || strlen(lpStr) < 1)
    {
        cJson.Add(NOTE_GETJSON);
        return WFS_SUCCESS;
    }

    // 解析lpStr
    CHAR szAppList[64][CONST_VALUE_260];
    CHAR szKeyList[12][CONST_VALUE_260];
    CHAR szIdenKey[32];
    INT  nAppCnt = 0, nKeyCnt = 0;
    INT  nIdenCnt = 0;

    memset(szAppList, 0x00, sizeof(szAppList));
    nAppCnt = DataConvertor::split_string(lpStr, '|', szAppList, 64);
    if (nAppCnt == 0)
    {
        cJson.Add(NOTE_GETJSON);
        return WFS_SUCCESS;
    }

    // 取第一个分割串处理
    memset(szKeyList, 0x00, sizeof(szKeyList));
    nKeyCnt = DataConvertor::split_string(szAppList[0], ',', szKeyList, 64);
    if (nKeyCnt < 3)
    {
        cJson.Add(NOTE_GETJSON);
        return WFS_SUCCESS;
    }
    cJson.Add(JSON_KEY_USE_AREA, atoi(szKeyList[0]));
    cJson.Add(JSON_KEY_MEDIA_WIDTH, atoi(szKeyList[1]));    // 介质宽(单位:MM)
    cJson.Add(JSON_KEY_MEDIA_HEIGHT, atoi(szKeyList[2]));   // 介质高(单位:MM)

    for (INT i = 1; i < nAppCnt; i ++)
    {
        memset(szKeyList, 0x00, sizeof(szKeyList));
        if (DataConvertor::split_string(szAppList[i], ',', szKeyList, 64) < 5)
        {
            continue;
        }
        // 项名
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_IDEN_NAME, nIdenCnt + 1);
        cJson.Add(szIdenKey, std::string(szKeyList[0]));
        // 起始坐标X(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_START_X, nIdenCnt + 1);
        cJson.Add(szIdenKey, atoi(szKeyList[1]));
        // 起始坐标Y(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_START_Y, nIdenCnt + 1);
        cJson.Add(szIdenKey, atoi(szKeyList[2]));
        // 可用宽(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_WIDTH, nIdenCnt + 1);
        cJson.Add(szIdenKey, atoi(szKeyList[3]));
        // 可用高(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_HEIGHT, nIdenCnt + 1);
        cJson.Add(szIdenKey, atoi(szKeyList[4]));

        nIdenCnt ++;
    }
    cJson.Add(JSON_KEY_IDEN_CNT, nIdenCnt);   // 项数目

    return WFS_SUCCESS;
}

