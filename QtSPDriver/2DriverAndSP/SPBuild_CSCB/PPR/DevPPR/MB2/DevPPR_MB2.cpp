#include "DevPPR_MB2.h"

#include <qnamespace.h>
#include <unistd.h>
#include <QObject>

// 版本号
CHAR  byVRTU[17]  = "DevPPRR01000000";

static const char *ThisModule = "DevPPR_MB2.cpp";

//////////////////////////////////////////////////////////////////////////

CDevPPR_MB2::CDevPPR_MB2(LPCSTR lpDevType)
{
    SetLogFile(LOG_NAME_DEVPPR, ThisModule,lpDevType);  // 设置日志文件名和错误发生的文件
    m_bPrinting = FALSE;
}

CDevPPR_MB2::~CDevPPR_MB2()
{
    Close();
}

// 释放接口
void CDevPPR_MB2::Release()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return;
}

int Open(const char *pMode);
// 打开与设备的连接
int CDevPPR_MB2::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    int iRet = WFS_SUCCESS;

    // 建立MB2连接
    if (m_devMB2.IsDeviceOpen() != TRUE)
    {
        if ((iRet = m_devMB2.DeviceOpen((LPSTR)lpMode)) != IMP_SUCCESS)
        {
 //           Log(ThisModule, __LINE__, "建立MB2连接: ->DeviceOpen(%s) Fail, ErrCode=%d, ",lpMode, iRet);
 //           return ERR_DSR_COMM_ERR;
            Log(ThisModule, __LINE__, "建立CPR连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
                lpMode, iRet, ConvertErrCodeToStr(ConvertErrorCode(iRet)));
            return ConvertErrorCode(iRet);
        }
    }
    m_stdOpenMode.append("USB");

    return PTR_SUCCESS;
}

// 关闭与设备的连接
int CDevPPR_MB2::Close()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    m_devMB2.DeviceClose();

    return PTR_SUCCESS;
}

// 设备初始化
int CDevPPR_MB2::Init()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return PTR_SUCCESS;
}

// 设备复位
int CDevPPR_MB2::ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return PTR_SUCCESS;
}



// 设备复位
int CDevPPR_MB2::Reset()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;
    if ((nRet = m_devMB2.nResetDev()) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "CPR设备初始化: ->nResetDev() Fail, ErrCode=%d, Return %s.",
            nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }

/*    if ((nRet = m_devMB2.DeviceOpen((LPSTR)m_stdOpenMode.c_str())) != IMP_SUCCESS)
    {
        Log(ThisModule, __LINE__, "CPR设备初始化后重新建立连接: ->DeviceOpen(%s) Fail, ErrCode=%d, Return %s.",
            m_stdOpenMode.c_str(), nRet, ConvertErrCodeToStr(ConvertErrorCode(nRet)));
        return ConvertErrorCode(nRet);
    }*/

    return PTR_SUCCESS;
}

// 取设备状态
int CDevPPR_MB2::GetStatus(DEVPTRSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    LONG    nRet = IMP_SUCCESS;
//    INT    nDevStat[12];

    //设备状态初始化
    stStatus.wToner = WFS_PTR_TONERNOTSUPP;
    stStatus.wInk = WFS_PTR_INKNOTSUPP;
 /*   for(int i = 0; i < m_stNoteBoxList->; i++){
        stStatus.stRetract[i].wBin = WFS_PTR_RETRACTBINOK;
        stStatus.stRetract[i].usCount = 0;

    }*/


    stStatus.wDevice = DEV_STAT_ONLINE;
    stStatus.wMedia = MEDIA_STAT_NOTPRESENT;
    for (INT i = 0; i < 16; i ++){
        stStatus.wPaper[i] = PAPER_STAT_NOTSUPP;
    }
    stStatus.wDevice =  DEV_STAT_OFFLINE;
    stStatus.wMedia = WFS_PTR_MEDIAUNKNOWN;
    usPaperStat = WFS_PTR_PAPERUNKNOWN;	// 状态:纸状态未知

    unsigned char cDevStatus;
    if((nRet = m_devMB2.nGetDevStatus(&cDevStatus)) >= 0)
    {
        //设备状态
        switch(nRet) {
        case STATUS_NOMAL:
        case STATUS_MEDIA_NONE:
        case STATUS_MEDIA_PREENTERING:
        case STATUS_MEDIA_ENTERING:
        case STATUS_MEDIA_PRESENT:
        case STATUS_MEDIA_INSERTED_ALL:
        case STATUS_MEDIA_NEAR_END:
        case STATUS_MEDIA_MAGNETIC_UNREAD:
            stStatus.wDevice = WFS_STAT_DEVONLINE;
            break;
        case STATUS_MEDIA_JAMMED:
             stStatus.wDevice =  WFS_STAT_DEVHWERROR;
            break;
        default:
            stStatus.wDevice =  WFS_STAT_DEVHWERROR;
        }
    }
    if((nRet = m_devMB2.nGetMediaStatus(&cDevStatus)) >= 0)
    {
        // 根据纸状态设置Media状态
        switch(nRet) {
        case STATUS_MEDIA_NONE:
            stStatus.wMedia = WFS_PTR_MEDIANOTPRESENT;
            usPaperStat = WFS_PTR_PAPEROUT; // 状态:无纸
            break;
        case STATUS_MEDIA_PREENTERING:
        case STATUS_MEDIA_ENTERING:
            stStatus.wMedia = WFS_PTR_MEDIAENTERING;
            usPaperStat = WFS_PTR_PAPERLOW; // 状态：纸少
            break;
        case STATUS_MEDIA_PRESENT:
        case STATUS_MEDIA_INSERTED_ALL:
        case STATUS_MEDIA_NEAR_END:
            stStatus.wMedia = WFS_PTR_MEDIAPRESENT;
            usPaperStat = WFS_PTR_PAPERFULL; // 状态:纸满
            break;
        case STATUS_MEDIA_JAMMED:
            stStatus.wMedia = WFS_PTR_MEDIAJAMMED;
            usPaperStat = WFS_PTR_PAPERJAMMED; // 状态:纸被卡住
            break;
        default:
            stStatus.wMedia = WFS_PTR_MEDIAUNKNOWN;
            usPaperStat = WFS_PTR_PAPERUNKNOWN;	// 状态:纸状态未知
            break;
        }
    }
/*    for(int i = 0; i < 16; i++){
        stStatus.wPaper[i] = usPaperStat;
    }*/

    if (stStatus.wDevice == DEV_STAT_OFFLINE ||
        stStatus.wDevice == DEV_STAT_POWEROFF)
    {
        return ERR_PTR_NOT_OPEN;
    } else
//    if (stStatus.wDevice == DEV_STAT_HWERROR ||
//        stStatus.wDevice == DEV_STAT_USERERROR)
//    {
//        return ERR_PTR_HWERR;
//    }else
//    if (stStatus.wDevice == DEV_STAT_BUSY) {
//        return ERR_PTR_DEVBUSY;
//    } else
    if (stStatus.wDevice == DEV_STAT_NODEVICE)
    {
        return ERR_PTR_DEVUNKNOWN;
    }

    return PTR_SUCCESS;
}

// 打印字串(无指定打印坐标)
int CDevPPR_MB2::PrintData(const char *pStr, unsigned long ulDataLen)
{
    INT nRet = PTR_SUCCESS;
    nRet = m_devMB2.LineModePrintText((LPSTR)pStr, ulDataLen, 100,100);
    if (nRet != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }
    return PTR_SUCCESS;
}

int CDevPPR_MB2::PrintDataOrg(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    m_bPrinting = TRUE;
    INT nRet = PTR_SUCCESS;
    if ((nRet = m_devMB2.LineModePrintText((LPSTR)pStr, ulDataLen, ulOrgX, ulOrgY)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }
    m_bPrinting = FALSE;
    return PTR_SUCCESS;
}

// 图片打印(无指定打印坐标)
int CDevPPR_MB2::PrintImage(char *szImagePath, unsigned long ulWidth, unsigned long ulHeight)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;
    nRet = m_devMB2.nPrintImage(szImagePath,ulWidth,ulHeight);
    if(nRet != 0){
        return nRet;
    }

    return PTR_SUCCESS;
}

// 指定坐标打印图片
int CDevPPR_MB2::PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT    nRet = IMP_SUCCESS;


    return PTR_SUCCESS;
}

// 扫描信息获取
int CDevPPR_MB2::ReadForm2(DEVPTRREADFORMIN stScanIn, DEVPTRREADFORMOUT &stScanOut, LONG lMagenticType)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    DEVPTRREADFORMIN stScanInParam = stScanIn;

    LONG lRet = 0;
    lRet = m_devMB2.nMsRead(stScanOut.lpData, lMagenticType);
    if(lRet == STATUS_MEDIA_JAMMED){
        Log(ThisModule, __LINE__, "ReadForm2: ->nMsRead fail, Media jam! ErrCode=%d, Return %s.",
           lRet, ConvertErrCodeToStr(ConvertErrorCode(lRet&0xFF)));
        return  ERR_PTR_JAMMED;
    }
    if(lRet == ERR_PTR_CANCEL){
        Log(ThisModule, __LINE__, "ReadForm2: ->nMsRead fail, command cancel! ErrCode=%d, Return %s.",
           lRet, ConvertErrCodeToStr(ConvertErrorCode(lRet&0xFF)));
        return  lRet;
    }

    if (lRet < 0)
    {
        Log(ThisModule, __LINE__, "ReadForm: ->nMsRead(%s) Fail, ErrCode=%d, Return %s.",
            m_stdOpenMode.c_str(), lRet, ConvertErrCodeToStr(ConvertErrorCode(lRet&0xFF)));
        return ConvertErrorCode(lRet&0xFF);
    }
    return PTR_SUCCESS;
}

int CDevPPR_MB2::ReadForm(DEVPTRREADFORMIN stScanIn, DEVPTRREADFORMOUT &stScanOute)
{
    return PTR_SUCCESS;
}


// 扫描信息获取
int CDevPPR_MB2::ReadImage(DEVPTRREADIMAGEIN stScanIn, DEVPTRREADIMAGEOUT &stScanOut)
{


    return PTR_SUCCESS;
}

int CDevPPR_MB2::RetractMedia()
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = PTR_SUCCESS;
    if(nRet = m_devMB2.nRetractMediaToBox() != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }
    return PTR_SUCCESS;
}
// 介质控制
int CDevPPR_MB2::MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);
    return PTR_SUCCESS;
}

// 介质控制
int CDevPPR_MB2::MeidaControl2(MEDIA_ACTION2 enMediaAct, unsigned short usParam)
{
    THISMODULE(__FUNCTION__);
    // AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = PTR_SUCCESS;
    if ((nRet = m_devMB2.nControlMedia(enMediaAct)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }
    return PTR_SUCCESS;
}

// 设置数据
int CDevPPR_MB2::SetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case 0:
        {
            break;
        }
        default:
            break;
    }

    return PTR_SUCCESS;
}

// 获取数据
int CDevPPR_MB2::GetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case 0:
            break;
        default:
            break;
    }

    return PTR_SUCCESS;
}

// 获取版本号(1DevCPR版本/2固件版本/3设备软件版本/4其他)
void CDevPPR_MB2::GetVersion(char* szVer, long lSize, ushort usType)
{
    CHAR szFWVer[128] = { 0x00 };
    if (szVer != nullptr)
    {
        switch(usType)
        {
            case GET_VER_DEVRPR:    // DevRPR版本号
                memcpy(szVer, byVRTU, strlen(byVRTU));
                    break;
            case GET_VER_FW:        // 固件版本号
                m_devMB2.nGetFirmwareVersion(szFWVer);
                memcpy(szVer, szFWVer,strlen(szFWVer));
                break;
            default:
                break;
        }
    }
}

INT CDevPPR_MB2::ConvertErrorCode(INT nRet)
{
    switch(nRet)
    {
        // > 100: Impl处理返回
        case IMP_SUCCESS:                   // 成功->操作成功
            return PTR_SUCCESS;
        default:
            return ERR_PTR_OTHER;
    }
}

CHAR* CDevPPR_MB2::ConvertErrCodeToStr(INT nRet)
{
    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    switch(nRet)
    {
        case PTR_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "操作成功");
            return m_szErrStr;

        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "其他错误/未知错误");
            return m_szErrStr;
    }
}

long CDevPPR_MB2::CmdCancel()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);// 取消接口不用互斥

    if (m_bPrinting)// 防止多取消，造成取状态失败
    {
        m_bPrinting = false;

    }
    m_devMB2.lSetCancelFlg(TRUE);           //TEST#30

    return ERR_DEVPORT_SUCCESS;
}

//test#30
void CDevPPR_MB2::vSetCancelFlg(BOOL bCancelFlg)
{
    m_devMB2.lSetCancelFlg(bCancelFlg);           //TEST#30

}
//////////////////////////////////////////////////////////////////////////
