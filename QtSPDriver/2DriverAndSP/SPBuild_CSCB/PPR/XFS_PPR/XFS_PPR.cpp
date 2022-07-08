#include "XFS_PPR.h"
#include "string.h"
#include "QtTypeInclude.h"
#include <QTextCodec>
#include <stdlib.h>
#include <unistd.h>

// PTR SP 版本号
BYTE    byVRTU[17] = {"HWPPRSTE00000001"};

// 事件日志
static const char *ThisFile = "XFS_PPR";

#define IDS_INFO_CUTPAPER_SUCCESS   "凭条打印机切纸成功"
#define IDS_INFO_RESET_DEVICE       "开始修复打印机"
#define IDS_INFO_RESET_INFO         "打印机复位成功,无纸,返回值=[%d] 传入参数为：[%u]"
#define IDS_INFO_RESET_SUCCESS      "打印机复位成功,返回值=[%d] 传入参数为：[%u]"
#define IDS_INFO_STARTUP_INFO       "Printer StartUp, DeviceName=[%s], DeviceDLLName=[%s]"
#define IDS_INFO_PAPER_TAKEN        "纸被取走"

#define IDS_ERR_REESET_ERROR        "打印机复位错误, 返回值=[%d]"
#define IDS_ERR_RESET_AND_CUTPAPER  "打印机复位ControlMedia错误,返回值=[%d]"
#define IDS_ERR_JPR_UNSUPP_COMMAND  "流水打印机不支持此指令[%u]"
#define IDS_ERR_RPR_UNSUPP_COMMAND  "凭条打印机不支持此指令[%u]"
#define IDS_ERR_NO_PAPER_WHENCUT    "切纸时发现无纸"
#define IDS_ERR_PAPER_JAMMED        "打印机卡纸"
#define IDS_ERR_CUTPAPER_ERROR      "切纸错误(检测黑标:[%d],进纸:[%d])"
#define IDS_ERR_DEVIVE_STA          "当前设备状态不为ONLINE，当前设备状态为：[%d]"
#define IDS_ERR_PRINTSTRING_FAILD   "PrintString失败:[%d]"
#define IDS_ERR_PRINTIMAGE_FAILD    "打印图片错误:[%d]"
#define IDS_ERR_PRINT_FAILD         "打印错误:[%d]"
#define IDS_ERR_INIT_ERROR          "初始化设备错误:[%d]"
#define IDS_ERR_COMPORT_ERROR       "串口参数不正确"
#define IDS_ERR_LOADDLL_FAILD       "LoadLibrary[%s]失败:Error()=[%d]"
#define IDS_ERR_GetProcAdd_FAILD    "GetProcAddress[%s], CreatePrinterDevice)失败"
#define IDS_ERR_CreateDev_FAILD     "调用[%s]的CreatePrinterDevice[%s]失败"
#define IDS_ERR_Open_FAIlD          "调用[%s]的Open()失败:%[d]"
#define IDS_ERR_NO_CHARSET          "系统缺少中文字符集"

void GetDocFileName(char *pDocFileName, DWORD dwLen);

//-----------------------------------------------------------------------------------
//-------------------------------------构造/析构--------------------------------------
CXFS_PPR::CXFS_PPR()
{
    m_WaitTaken = WTF_NONE;
    m_bNeedReset = true;
    bCancelInsertMedia = FALSE;
}

CXFS_PPR::~CXFS_PPR()
{

}

//-----------------------------------------------------------------------------------
// 开始运行SP
long CXFS_PPR::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseBCR
    if (0 != m_pBase.Load("SPBasePTR.dll", "CreateISPBasePTR", "PTR"))
    {
        Log(ThisModule, __LINE__, "加载SPBasePTR失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

//-----------------------------------------------------------------------------------
//--------------------------------------基本接口---------------------------------------
// Open设备及初始化相关
HRESULT CXFS_PPR::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    char szDevRPRVer[64] = { 0x00 };
    char szFWVersion[64] = { 0x00 };
    long lFWVerSize = 0;

    INT nRet = 0;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    InitStatus();
    InitCaps();
    InitPTRData(lpLogicalName);
    InitConfig();

    // 设备驱动动态库验证
    if (strlen(m_sConfig.szDevDllName) < 1)
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName配置项为空或读取失败.", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 加载设备驱动动态库
    if (m_pPrinter == nullptr)
    {
        if (m_pPrinter.Load(m_sConfig.szDevDllName, "CreateIDevPTR", DEVTYPE_CHG(m_sConfig.nDriverType)) != 0)
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DEVTYPE=%d|%s. ReturnCode:%s.",
                m_sConfig.szDevDllName, m_sConfig.nDriverType, DEVTYPE_CHG(m_sConfig.nDriverType),
                m_pPrinter.LastError().toUtf8().constData());
            return FALSE;
        }
    }

    // Open前下传初始参数
    //m_pDev->SetData(&(m_stMsrIniConfig.stMsrInitParamInfo), DATATYPE_INIT);
    // 打开连接
    nRet = m_pPrinter->Open(nullptr);
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "打开设备连接失败．ReturnCode:%d.", nRet);
        return ConvertErrCode(nRet);
    }

    // 设置不需要Reset
    //m_bNeedReset = false;

    // 执行复位
    nRet = m_pPrinter->Reset();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, nRet, "->Reset() Fail, ErrCode = %d, Return: %d.",
            nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }
    // 设备初始化
 /*   nRet = OnInit();
    if (nRet != 0)
    {
        Log(ThisModule, __LINE__, "设备初始化失败．ReturnCode:%d.", nRet);
        return ConvertErrCode(nRet);
    }*/
    // 更新扩展状态
    CHAR szDevVer[128] = { 0x00 };
    m_pPrinter->GetVersion(szDevVer, sizeof(szDevVer) - 1, GET_VER_DEVRPR);
    CHAR szFWVer[128] = { 0x00 };
    m_pPrinter->GetVersion(szFWVer, sizeof(szFWVer) - 1, GET_VER_FW);
    m_cExtra.AddExtra("VRTCount", "2");
    m_cExtra.AddExtra("VRT[00]_XFSPPR", (char*)byVRTU);
    m_cExtra.AddExtra("VRT[01]_DevPPR", szDevVer);
    m_cExtra.AddExtra("FirmwareVersion", szFWVer);
    m_cExtra.AddExtra("LastErrorCode", "0000000");
    m_cExtra.AddExtra("LastErrorDetail", "");

    // 更新一次状态
    OnStatus();

    Log(ThisModule, 1, "打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());

    return WFS_SUCCESS;
}

// 关闭设备
HRESULT CXFS_PPR::OnClose()
{
    THISMODULE(__FUNCTION__);

    if (m_pPrinter != nullptr)
    {
        Log(ThisModule, 1, "关闭设备连接成功.");
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}

// 实时状态更新
HRESULT CXFS_PPR::OnStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

//    DEVPTRSTATUS stDevStatus;
 //   int nRet = m_pPrinter->GetStatus(stDevStatus);
    UpdateDeviceStatus();
    return 0;
}

// Taken事件处理
HRESULT CXFS_PPR::OnWaitTaken()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    UpdateDeviceStatus();

    return WFS_SUCCESS;
}

// 命令取消
HRESULT CXFS_PPR::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_pPrinter != nullptr)
        m_pPrinter->CmdCancel();
    bCancelInsertMedia = TRUE;
    return WFS_SUCCESS;
}

// 固件升级
HRESULT CXFS_PPR::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}


//-----------------------------------------------------------------------------------
//-------------------------------重载CSPBaseClass的方法--------------------------------
HRESULT CXFS_PPR::OnInit()
{
    THISMODULE(__FUNCTION__);

    INT nRet = m_pPrinter->Init();
    //UpdateDeviceStatus(nRet);
    if (nRet < 0)
    {
        return ConvertErrCode(nRet);
    }
    //m_pPrinter->SetPrintFormat(m_stPrintFormat);
    return ConvertErrCode(nRet);
}

HRESULT CXFS_PPR::OnExit()
{
    if (nullptr != m_pPrinter)
    {
        m_pPrinter->Close();
    }

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//----------------------------------PTR类型接口(INFO)----------------------------------
HRESULT CXFS_PPR::GetStatus(LPWFSPTRSTATUS &lpStatus)
{
    m_sStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_sStatus;

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::GetCapabilities(LPWFSPTRCAPS &lpCaps)
{
    m_sCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_sCaps;

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::GetFormList(LPSTR &lpszFormList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    lpszFormList = (LPSTR)m_pData->GetNames();

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::GetReadFormList(LPSTR &lpszFormList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadReadForms();
    lpszFormList = (LPSTR)m_pData->GetReadFormNames();

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::GetMediaList(LPSTR &lpszMediaList)
{
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    lpszMediaList = (LPSTR)m_pData->GetNames(false);

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::GetQueryForm(LPCSTR lpFormName, LPWFSFRMHEADER &lpFrmHeader)
{
    THISMODULE(__FUNCTION__);
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadForms();
    long hRes;
    {
        CSPPrinterForm *pForm = m_pData->FindForm((LPCSTR)lpFormName);
        if (pForm)
        {
            if (pForm->IsLoadSucc())
            {
                m_pData->m_LastForm.ExtractFromForm(pForm);

                SIZE sizeForm = pForm->GetOrigSize();
                for (DWORD iChild = 0; iChild < pForm->GetSubItemCount(); iChild++)
                {
                    ISPPrinterItem *pItem = pForm->GetSubItem(iChild);
                    if (pItem == NULL)
                        continue;
                    SIZE sizeItem = pItem->GetOrigSize();
                    SIZE3 sizePosItem = pItem->GetOrigPosition();
                    if (sizePosItem.cx + sizeItem.cx > sizeForm.cx ||
                        sizePosItem.cy + sizeItem.cy > sizeForm.cy)
                    {
                        Log(ThisModule, -1, IDS_ERR_FORM_INVALID, pForm->GetName());
                        return WFS_ERR_PTR_FORMINVALID;
                    }
                }
                hRes = WFS_SUCCESS;
            }
            else
            {
                hRes = WFS_ERR_PTR_FORMINVALID;
            }
        }
        else
        {
            hRes = WFS_ERR_PTR_FORMNOTFOUND;
        }
    }
    lpFrmHeader = &m_pData->m_LastForm;

    return hRes;
}

HRESULT CXFS_PPR::GetQueryMeida(LPCSTR lpMediaName, LPWFSFRMMEDIA &lpFrmMedia)
{
    THISMODULE(__FUNCTION__);

    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadMedias();
    long hRes;
    {
        CSPPrinterMedia *pMedia = m_pData->FindMedia((LPCSTR)lpMediaName);
        if (pMedia)
        {
            if (pMedia->IsLoadSucc())
            {
                m_pData->m_LastMedia.ExtractFromMedia(pMedia);

                if (m_pData->m_LastMedia.wPrintAreaX + m_pData->m_LastMedia.wPrintAreaHeight > m_pData->m_LastMedia.wPrintAreaHeight ||
                    m_pData->m_LastMedia.wPrintAreaY + m_pData->m_LastMedia.wPrintAreaWidth  > m_pData->m_LastMedia.wPrintAreaWidth)
                {
                    return WFS_ERR_PTR_MEDIAINVALID;
                }

                hRes = WFS_SUCCESS;
            }
            else
            {
                hRes = WFS_ERR_PTR_MEDIAINVALID;
            }
        }
        else
        {
            hRes = WFS_ERR_PTR_MEDIANOTFOUND;
        }
    }
    lpFrmMedia = &m_pData->m_LastMedia;

    return hRes;
}

HRESULT CXFS_PPR::GetQueryField(const LPWFSPTRQUERYFIELD lpQueryField, LPWFSFRMFIELD *&lpszMediaList)
{
    THISMODULE(__FUNCTION__);
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    long hRes = WFS_SUCCESS;
    m_pData->LoadForms();
    {
        hRes = m_pData->FindField(lpQueryField->lpszFormName, lpQueryField->lpszFieldName);
        if (WFS_SUCCESS == hRes)
        {
            if (0 == m_pData->m_LastFields.GetCount())
            {
                hRes = WFS_ERR_PTR_FIELDNOTFOUND;
            }
            else
            {
                hRes = WFS_SUCCESS;

            }
        }
    }
    lpszMediaList = m_pData->m_LastFields;

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//---------------------------------PTR类型接口(EXECUTE)-------------------------------
// 介质控制
HRESULT CXFS_PPR::MediaControl(const LPDWORD lpdwMeidaControl)
{
    return ControlMedia(*lpdwMeidaControl);
}

// 格式化打印
HRESULT CXFS_PPR::PrintForm(const LPWFSPTRPRINTFORM lpPrintForm, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    bCancelInsertMedia = FALSE;         //TEST#30
    if (m_sStatus.fwDevice != WFS_PTR_DEVONLINE &&			//30-00-00-00（FT#0008）
        m_sStatus.fwDevice != WFS_PTR_DEVBUSY)				//30-00-00-00（FT#0008）
    {														//30-00-00-00（FT#0008）
        return WFS_ERR_HARDWARE_ERROR;						//30-00-00-00（FT#0008）
    }														//30-00-00-00（FT#0008）

    if (m_sStatus.fwPaper[0] == WFS_PTR_PAPEROUT)			//30-00-00-00（FT#0008）
    {														//30-00-00-00（FT#0008）
        return WFS_ERR_PTR_PAPEROUT;						//30-00-00-00（FT#0008）
    }														//30-00-00-00（FT#0008）

    SetPrtAlignMode(m_stPrtAlignModeList[m_sConfig.nDriverType]);
    // 设置边界
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    // 加载Form文件
    m_pData->LoadForms();
    // 加载Media文件
    m_pData->LoadMedias();
    LPWFSPTRPRINTFORM pIn = (LPWFSPTRPRINTFORM)lpPrintForm;

    HRESULT hRet = InnerPrintForm(pIn);

    return hRet;
}

// 格式化读
HRESULT CXFS_PPR::ReadForm(const LPWFSPTRREADFORM lpReadForm, LPWFSPTRREADFORMOUT &lpReadFormOut, DWORD dwTimeOut)
{    
    m_pPrinter->vSetCancelFlg(FALSE);           //test#30
    SIZE tmpSize = GetTwipsPerRowCol();
    m_pData->SetTwipsPerRowCol(&tmpSize);
    m_pData->LoadReadForms();
//    m_pData->LoadMedias();
    m_pData->m_ReadFormOut.Clear();
    LPWFSPTRREADFORM pIn = (LPWFSPTRREADFORM)lpReadForm;
    HRESULT hRes = InnerReadForm(pIn);
    if (WFS_SUCCESS == hRes)
    {
        lpReadFormOut = m_pData->m_ReadFormOut;
    }
    return hRes;
}

// 无格式打印
HRESULT CXFS_PPR::RawData(const LPWFSPTRRAWDATA lpRawData, LPWFSPTRRAWDATAIN &lpRawDataIn, DWORD dwTimeOut)
{
    LPWFSPTRRAWDATA pIn = (LPWFSPTRRAWDATA)lpRawData;
    long hRes = SendRawData(WFS_PTR_INPUTDATA == pIn->wInputData, pIn->ulSize, pIn->lpbData);
    if (WFS_SUCCESS == hRes)
    {
        lpRawDataIn = (LPWFSPTRRAWDATAIN)m_pData->m_InputRawData;
    }
    return hRes;
}

// 获得插入物理设备中的媒介的长宽度
HRESULT CXFS_PPR::MediaExtents(const LPWFSPTRMEDIAUNIT lpMediaUnit, LPWFSPTRMEDIAEXT &lpMediaExt)
{
    THISMODULE(__FUNCTION__);

    return WFS_ERR_UNSUPP_COMMAND;
}

// 将媒介回收计数由当前值归零
HRESULT CXFS_PPR::ResetCount(const LPUSHORT lpusBinNum)
{
    THISMODULE(__FUNCTION__);

    USHORT usBinNum = *lpusBinNum;
    int iBoxIdx = 0;
    if(lpusBinNum == nullptr){
        for(int i = 0; i < m_stNoteBoxList.wBoxCount; i++){
            m_stNoteBoxList.NoteBox[i].wNoteCount = 0;
        }
    }else{
        if(usBinNum >= 1){
            iBoxIdx  = usBinNum - 1;
        }
        m_stNoteBoxList.NoteBox[iBoxIdx].wNoteCount = 0;
    }
    FireRetractBinThreshold(usBinNum,WFS_PTR_RETRACTBINOK);
    return WFS_SUCCESS;
}

// 获取图象数据
HRESULT CXFS_PPR::ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);



    return WFS_SUCCESS;
}

// 复位
HRESULT CXFS_PPR::Reset(const LPWFSPTRRESET lpReset)
{
    THISMODULE(__FUNCTION__);

    LPWFSPTRRESET pIn = (LPWFSPTRRESET)lpReset;
    INT nRet = PTR_SUCCESS;
    HRESULT hRet = WFS_SUCCESS;

    if (pIn->dwMediaControl != WFS_PTR_CTRLEJECT && //退出媒介
        pIn->dwMediaControl != WFS_PTR_CTRLRETRACT && //根据usRetractBinNumber 规定将媒介回收入回收盒。
        pIn->dwMediaControl != WFS_PTR_CTRLEXPEL)
    {
        Log(ThisModule, nRet, "Input dwMediaControl[%d] Is Invalid, Return: %d.", pIn->dwMediaControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (pIn->dwMediaControl == WFS_PTR_CTRLEJECT)
    {
        if ((hRet = ControlMedia(pIn->dwMediaControl)) != WFS_SUCCESS)
        {
            Log(ThisModule, nRet, "Input dwMediaControl[%d] Is EjectMedia: ->ControlMedia(%d) Is Fail, Return: %d.",
                pIn->dwMediaControl, pIn->dwMediaControl, hRet);
            return hRet;
        }
    } else
    if (pIn->dwMediaControl == WFS_PTR_CTRLEXPEL)
    {
        Log(ThisModule, nRet, "Input dwMediaControl[%d] Is UnSupp, Not Return.");
    } else
    {
        LPUSHORT usOut = nullptr;
        if ((hRet = RetractMedia((const LPUSHORT)(&pIn->usRetractBinNumber), usOut)) != WFS_SUCCESS)
        {
            Log(ThisModule, nRet, "Input dwMediaControl[%d] Is RetractMedia: ->RetractMedia(%d, %d) Is Fail, Return: %d.",
                pIn->dwMediaControl, pIn->usRetractBinNumber, usOut, hRet);
            return hRet;
        }
    }

    // 执行复位
    nRet = m_pPrinter->Reset();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, nRet, "->Reset() Fail, ErrCode = %d, Return: %d.",
            nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    return WFS_SUCCESS;
}

// 媒介回收
HRESULT CXFS_PPR::RetractMedia(const LPUSHORT lpusBinNum, LPUSHORT &lpusBinNumOut)
{
    THISMODULE(__FUNCTION__);
    INT nRet = PTR_SUCCESS;
    USHORT usRetractBox = *lpusBinNum;
    lpRetractBoxIdx = lpusBinNum;

    if ((usRetractBox = m_stNoteBoxList.nIsRetractBox()) < 1)
    {
        Log(ThisModule, nRet, "Not Have RetractBox, Return: %d.", WFS_ERR_UNSUPP_COMMAND);
        return WFS_ERR_UNSUPP_COMMAND;
    }

    if (*lpusBinNum > 0)
    {
        if (m_stNoteBoxList.bIsRetractBox(*lpusBinNum) != TRUE)
        {
            Log(ThisModule, nRet, "Box[%d] Is Not RetractBox, Return: %d.", *lpusBinNum, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    if(*lpusBinNum == 0){
        return WFS_SUCCESS;
     }

    nRet = m_pPrinter->RetractMedia();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, nRet, "->RetractMedia Fail, ErrCode = %d, Return: %d.",
                           nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    lpusBinNumOut = lpRetractBoxIdx;
    int iBoxIdx = 0;
    if(*lpRetractBoxIdx >= 1){
        iBoxIdx = *lpRetractBoxIdx -= 1;
    }
    m_stNoteBoxList.NoteBox[iBoxIdx].wNoteCount += 1;
    WORD wNoteCnt = m_stNoteBoxList.NoteBox[iBoxIdx].wNoteCount;

    if(wNoteCnt >= m_stNoteBoxList.NoteBox[iBoxIdx].wFullThreshold){
        FireRetractBinThreshold(usRetractBox, WFS_PTR_RETRACTBINFULL);  //bin full
    }else if(wNoteCnt >= m_stNoteBoxList.NoteBox[iBoxIdx].wThreshold){
        FireRetractBinThreshold(usRetractBox, WFS_PTR_RETRACTBINHIGH);  //bin high
    }

    return WFS_SUCCESS;
}
// 纸张移动
HRESULT CXFS_PPR::DispensePaper(const LPWORD lpPaperSource)
{
    THISMODULE(__FUNCTION__);


    return WFS_SUCCESS;
}

// 指示灯控制
HRESULT CXFS_PPR::SetGuidanceLight(const LPWFSPTRSETGUIDLIGHT lpSetGuidLight)
{
    return WFS_ERR_UNSUPP_COMMAND;
}


//-----------------------------------------------------------------------------------
//-----------------------------------重载函数-------------------------------
//
HRESULT CXFS_PPR::SendRawData(BOOL bExpectResp, ULONG nSize, LPBYTE pData)
{
    const char *ThisModule = "SendRawData";

    if (0 == nSize)
    {
        return WFS_SUCCESS;
    }

    // 转码
    QTextCodec *codec = QTextCodec::codecForLocale();
    QTextCodec *codec1 = QTextCodec::codecForName("gb18030");
    if (nullptr == codec1) codec1 = QTextCodec::codecForName("gb2312");
    if (nullptr == codec1) codec1 = QTextCodec::codecForName("gbk");
    if (nullptr == codec1)
    {
        Log(ThisModule, -1, IDS_ERR_PRINTSTRING_FAILD);
        return WFS_ERR_PTR_CHARSETDATA;
    }
    QString strText = QString::fromUtf8((char *)pData, nSize);
    QTextCodec::setCodecForLocale(codec1);
    QByteArray tmpData = strText.toLocal8Bit();
    char *pTempCode = tmpData.data();
    int nTempSize = tmpData.size();
    ULONG ulDataSize = 0;
    BYTE *pBuf = new BYTE[nTempSize + 2];
    memset(pBuf, 0, nTempSize + 2);
    RemoveUnPrintableChar(nTempSize, (LPBYTE)pTempCode, ulDataSize, pBuf);
    QTextCodec::setCodecForLocale(codec);

    // 去除不可打印字符
    if ('\n' != pBuf[ulDataSize - 1])
    {
        pBuf[ulDataSize++] = '\n';
    }
    pBuf[ulDataSize] = 0;

    HRESULT hRes = PrintString((char *)pBuf, ulDataSize, FALSE);
    delete [] pBuf;
    pBuf = nullptr;

    if (WFS_SUCCESS != hRes)
    {
        Log(ThisModule, -1, IDS_ERR_PRINTSTRING_FAILD, hRes);
        return hRes;
    }

    if (bExpectResp)
    {
        //SetInputRawData(0, nullptr);
        m_pData->m_InputRawData.SetData(0, nullptr);
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::Reset(DWORD dwMediaControl, USHORT usBinIndex)
{
    Q_UNUSED(usBinIndex);
    const char *const ThisModule = "Reset";
    Log(ThisModule, 1, IDS_INFO_RESET_DEVICE);

    int nRet = m_pPrinter->Init();
    UpdateDeviceStatus();
    if (PTR_SUCCESS != nRet)
    {
        //if (m_sStatus.fwPaper[0] == WFS_ERR_PTR_PAPEROUT)   // 无纸
        if (ERR_PTR_NO_PAPER == nRet)
        {
            m_bNeedKeepJammedStatus = FALSE;
            Log(ThisModule, 1, IDS_INFO_RESET_INFO, nRet, dwMediaControl);
            return WFS_SUCCESS;
        }

        Log(ThisModule, -1, IDS_ERR_REESET_ERROR, nRet);
        return WFS_ERR_HARDWARE_ERROR;

    }
    m_bReset = TRUE;

    m_bNeedKeepJammedStatus = FALSE;
    Log(ThisModule, 1, IDS_INFO_RESET_SUCCESS, nRet, dwMediaControl);
    return WFS_SUCCESS;
}

/* 图片打印
     *  nDstWidth  期望宽度
     *  nDstHeight 期望高度
     */

HRESULT CXFS_PPR::PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight)
{
    const char *ThisModule = "PrintImage";

    int nRet = m_pPrinter->PrintImage(const_cast<char*>(szImagePath), nDstWidth, nDstHeight);

//    int nRet = m_pPrinter->PrintDataOrg(dwMediaWidth, dwMediaHeight, 0, 0, "", 1, 0,
 //                                       szImagePath, strlen(szImagePath), 2);
    UpdateDeviceStatus();
    if (PTR_SUCCESS != nRet)
    {
        Log(ThisModule, nRet, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY)
{
    const char *ThisModule = "PrintImageOrg";

    /*int nRet = m_pPrinter->PrintImageOrg(szImagePath, ulOrgX, ulOrgY);
    UpdateDeviceStatus();
    if (PCS_SUCCESS != nRet)
    {
        Log(ThisModule, nRet, "打印图片错误: %d", nRet);
        return WFS_ERR_HARDWARE_ERROR;
    }*/

    return WFS_SUCCESS;
}

long CXFS_PPR::PrintData2(const char *pBuffer, unsigned long ulDataLen, LONG ulOrgX, ULONG ulOrgY)
{
    const char *ThisModule = "PrintData2";

    int iRet = m_pPrinter->PrintDataOrg(pBuffer, ulDataLen, ulOrgX, ulOrgY);
    UpdateDeviceStatus();
    if (iRet)
    {
        Log(ThisModule, -1, "PrintData2 fail, ErrCode:%d", iRet);
    }
    return ConvertErrCode(iRet);

}

long CXFS_PPR::PrintData(const char *pBuffer, DWORD dwSize)
{
    const char *ThisModule = "PrintData";
    int iRet = m_pPrinter->PrintData(pBuffer, dwSize);
    UpdateDeviceStatus();
    if (iRet)
    {
        Log(ThisModule, -1, "PrintData fail, ErrCode:%d", iRet);
    }
    return ConvertErrCode(iRet);
}

long CXFS_PPR::PrintMICR(const char *pBuffer, DWORD dwSize)
{
     THISMODULE(__FUNCTION__);
 /*   int iRet = m_pPrinter->WriteRFID(pBuffer);
    UpdateDeviceStatus();
    if (iRet)
    {
        Log(ThisModule, -1, "Write MICR fail, ErrCode:%d", iRet);
    }
    return ConvertErrCode(iRet);*/
}

HRESULT CXFS_PPR::PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint)
{
    const char *const ThisModule = "PrintString";
    if (0 == dwSize)
    {
        return WFS_SUCCESS;
    }

    int iCurPos = 0, iLenToEnter = 0, iEnterNum = 0, iLastSize = 0;
    const char *pp = pBuffer;
    int PAGESIZE = m_sConfig.nPageSize;
    int PAGELINE = m_sConfig.nPageLine;
    // FORM打印时根据数据量判断是否需要换行
    bool bNeedCR = true;
    if (dwSize < (DWORD)PAGESIZE && FALSE != bIsFromPrint)
    {
        bNeedCR = false;
    }

    int nRet = PTR_SUCCESS;
    if (m_sConfig.bEnableSplit)
    {
        while (0 < dwSize)
        {
            iLastSize = dwSize > (DWORD)PAGESIZE ? PAGESIZE : dwSize;
            while (iCurPos < iLastSize)
            {
                if (pp[ iCurPos ] & 0x80)
                {
                    iCurPos++;
                }
                else
                {
                    if (pp[ iCurPos ] == '\n')
                    {
                        if (iEnterNum < PAGELINE)
                        {
                            ++iEnterNum;
                            iLenToEnter = iCurPos + 1;
                        }
                        else
                        {
                            iCurPos++;
                            break;
                        }
                    }
                }
                iCurPos++;
            }

            if ((iCurPos >= PAGESIZE) && ('\n' != pp[ iCurPos - 1 ]) && (0 != iLenToEnter))
            {
                iCurPos = iLenToEnter;
            }

            char cLastChar = ((char *)pp)[ iCurPos - 1 ];
            char cBackup;
            bool bChanged = false;
            // FORM打印时根据数据量判断是否需要换行
            if ('\n' != cLastChar  && bNeedCR)
            {
                cBackup = ((char *)pp)[ iCurPos ];
                ((char *)pp)[ iCurPos ] = '\n';
                iCurPos++;
                bChanged = true;
            }

            nRet = m_pPrinter->PrintData(const_cast<char *>(pp), iCurPos);

            if (bChanged)
            {
                ((char *)pp)[ iCurPos ] = cBackup;
                --iCurPos;
            }
            bChanged = false;

            if (PTR_SUCCESS != nRet)
            {
                break;

                Log(ThisModule, nRet, IDS_ERR_PRINT_FAILD, nRet);
            }
            pp = pp + iCurPos;
            dwSize -= iCurPos;
            iCurPos = 0;
            iEnterNum = 0;
            iLenToEnter = 0;
        }
    }
    else
    {
        nRet = m_pPrinter->PrintData(const_cast<char *>(pBuffer), dwSize);
    }
    UpdateDeviceStatus();
    return ConvertErrCode(nRet);
}

HRESULT CXFS_PPR::AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut)
{
    const char *const ThisModule = "AddPrintString";
    if (0 == dwSize)
    {
        return WFS_SUCCESS;
    }

    int iCurPos = 0, iLenToEnter = 0, iEnterNum = 0, iLastSize = 0;
    const char *pp = pBuffer;
    int PAGESIZE = m_sConfig.nPageSize;
    int PAGELINE = m_sConfig.nPageLine;
    // FORM打印时根据数据量判断是否需要换行
    bool bNeedCR = true;
    if (dwSize < (DWORD)PAGESIZE && FALSE != bIsFromPrint)
    {
        bNeedCR = false;
    }

    int nRet = PTR_SUCCESS;
    if (m_sConfig.bEnableSplit)
    {
        while (0 < dwSize)
        {
            iLastSize = dwSize > (DWORD)PAGESIZE ? PAGESIZE : dwSize;
            while (iCurPos < iLastSize)
            {
                if (pp[ iCurPos ] & 0x80)
                {
                    iCurPos++;
                }
                else
                {
                    if (pp[ iCurPos ] == '\n')
                    {
                        if (iEnterNum < PAGELINE)
                        {
                            ++iEnterNum;
                            iLenToEnter = iCurPos + 1;
                        }
                        else
                        {
                            iCurPos++;
                            break;
                        }
                    }
                }
                iCurPos++;
            }

            if ((iCurPos >= PAGESIZE) && ('\n' != pp[ iCurPos - 1 ]) && (0 != iLenToEnter))
            {
                iCurPos = iLenToEnter;
            }

            char cLastChar = ((char *)pp)[ iCurPos - 1 ];
            char cBackup;
            bool bChanged = false;
            // FORM打印时根据数据量判断是否需要换行
            if ('\n' != cLastChar  && bNeedCR)
            {
                cBackup = ((char *)pp)[ iCurPos ];
                ((char *)pp)[ iCurPos ] = '\n';
                iCurPos++;
                bChanged = true;
            }

            if ((dwSizeOut + iCurPos) < MAX_PRINTDATA_LEN)
            {
                memcpy(pBuffOut + dwSizeOut, pp, iCurPos);
                dwSizeOut += iCurPos;
            }
            else
            {
                Log(ThisModule, -1, "PrintData(%u) is more than MAXLEN", dwSizeOut + iCurPos);
            }
            //nRet = m_pPrinter->PrintData(const_cast<char *>( pp ), iCurPos );

            if (bChanged)
            {
                ((char *)pp)[ iCurPos ] = cBackup;
                --iCurPos;
            }
            bChanged = false;

            if (PTR_SUCCESS != nRet)
            {
                Log(ThisModule, nRet, IDS_ERR_PRINT_FAILD, nRet);
                //缺纸时打印应返回WFS_ERR_PTR_PAPEROUT
                if (ERR_PTR_NO_PAPER == nRet)
                {
                    return WFS_ERR_PTR_PAPEROUT;
                }
                else
                {
                    return WFS_ERR_HARDWARE_ERROR;
                }
            }
            pp = pp + iCurPos;
            dwSize -= iCurPos;
            iCurPos = 0;
            iEnterNum = 0;
            iLenToEnter = 0;
        }
    }
    else
    {
        //nRet = m_pPrinter->PrintData(const_cast<char *>(pBuffer), dwSize);
        memcpy(pBuffOut + dwSizeOut, pp, iCurPos);
        dwSizeOut += iCurPos;
        if (PTR_SUCCESS != nRet)
        {
            Log(ThisModule, nRet, IDS_ERR_PRINT_FAILD, nRet);
            // 缺纸时打印应返回WFS_ERR_PTR_PAPEROUT
            if (ERR_PTR_NO_PAPER == nRet)
            {
                return WFS_ERR_PTR_PAPEROUT;
            }
            else
            {
                return WFS_ERR_HARDWARE_ERROR;
            }
        }
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::EndFormToJSON(PrintContext *pContext, CJsonObject &cJsonData)
{
    #define JSON_ADD(JSON, KEY, CNT, STR, VAL) \
        memset(STR, 0x00, sizeof(STR)); \
        sprintf(STR, "%s%d", KEY, CNT); \
        cJson_Pic.Add(STR, VAL);

    //CJsonObject cJsonData;
    CJsonObject cJson_Text, cJson_Pic, cJson_Bar, cJson_Micr;
    WORD wTextCnt, wPicCnt, wBarCnt, wMicrCnt;
    CHAR szIdenKey[32];
    char szPrintData[MAX_PRINTDATA_LEN] = { 0x00 };

  //  cJsonData.Clear();
 //   cJsonData.Add(JSON_KEY_MEDIA_WIDTH, pContext->pForm->GetSize().cx);    // 介质宽(单位:MM)
 //   cJsonData.Add(JSON_KEY_MEDIA_HEIGHT, pContext->pForm->GetSize().cx);   // 介质高(单位:MM)


    PRINT_ITEMS *pItems = (PRINT_ITEMS *)pContext->pUserData;
    if (pContext->bCancel)  // 收到取消命令,不处理返回成功
    {
        if (pItems)
        {
            delete pItems;
            pItems = NULL;
        }
        pContext->pUserData = NULL;
        return WFS_SUCCESS;
    }

    // 排序打印ITEM
    qsort(pItems->pItems, pItems->nItemNum, sizeof(PRINT_ITEM *), ComparePrintItem);

    cJson_Text.Clear();
    cJson_Pic.Clear();
    cJson_Bar.Clear();
    cJson_Micr.Clear();
    wTextCnt = 0;
    wPicCnt = 0;
    wBarCnt = 0;
    wMicrCnt = 0;

    for (int i = 0; i < pItems->nItemNum; i++)
    {
        PRINT_STRING strFormat; // 记录格式化数据
        PRINT_ITEM *pItem = pItems->pItems[i];
        // 图片打印
        if (pItem->nFieldType == FT_GRAPHIC)
        {
            wPicCnt ++;
            JSON_ADD(cJson_Pic, JSON_KEY_IDEN_NAME, wPicCnt, szIdenKey, pItem->strImagePath);
            JSON_ADD(cJson_Pic, JSON_KEY_START_X, wPicCnt, szIdenKey, pItem->x);
            JSON_ADD(cJson_Pic, JSON_KEY_START_Y, wPicCnt, szIdenKey, pItem->y);
            JSON_ADD(cJson_Pic, JSON_KEY_PIC_ZOOM, wPicCnt, szIdenKey, 1.40);
        } else
        if (FT_MICR == pItem->nFieldType)   // 磁码打印
        {
            wMicrCnt ++;
            memset(szPrintData, 0x00, sizeof(szPrintData));
            memcpy(szPrintData, pItem->Text, pItem->nTextLen);
            JSON_ADD(cJson_Micr, JSON_KEY_IDEN_NAME, wMicrCnt, szIdenKey, szPrintData);
            JSON_ADD(cJson_Micr, JSON_KEY_START_X, wMicrCnt, szIdenKey, pItem->x);
            JSON_ADD(cJson_Micr, JSON_KEY_START_Y, wMicrCnt, szIdenKey, pItem->y);
        } else // 字符串打印
        {
            wTextCnt ++;
            memset(szPrintData, 0x00, sizeof(szPrintData));
            memcpy(szPrintData, pItem->Text, pItem->nTextLen);
            JSON_ADD(cJson_Micr, JSON_KEY_IDEN_NAME, wTextCnt, szIdenKey, szPrintData);
            JSON_ADD(cJson_Micr, JSON_KEY_START_X, wTextCnt, szIdenKey, pItem->x);
            JSON_ADD(cJson_Micr, JSON_KEY_START_Y, wTextCnt, szIdenKey, pItem->y);
            JSON_ADD(cJson_Micr, JSON_KEY_AREA_WIDTH, wTextCnt, szIdenKey, pItem->nWidth);
            JSON_ADD(cJson_Micr, JSON_KEY_AREA_HEIGHT, wTextCnt, szIdenKey, pItem->nHeight);
            //JSON_ADD(cJson_Micr, JSON_KEY_TEXT_FONT, wTextCnt, szIdenKey, pItem->nHeight);
        }
    }

    if (wTextCnt > 0)
    {
        cJson_Text.Add(JSON_KEY_IDEN_CNT, wTextCnt);
        cJsonData.Add(JSON_KEY_IDEN_TYPE_TEXT, cJson_Text);
    }
    if (wPicCnt > 0)
    {
        cJson_Pic.Add(JSON_KEY_IDEN_CNT, wPicCnt);
        cJsonData.Add(JSON_KEY_IDEN_TYPE_PIC, cJson_Pic);
    }
    if (wBarCnt > 0)
    {
        cJson_Bar.Add(JSON_KEY_IDEN_CNT, wBarCnt);
        cJsonData.Add(JSON_KEY_IDEN_TYPE_BAR, cJson_Bar);
    }
    if (wMicrCnt > 0)
    {
        cJson_Micr.Add(JSON_KEY_IDEN_CNT, wMicrCnt);
        cJsonData.Add(JSON_KEY_IDEN_TYPE_MICR, cJson_Micr);
    }

    //lpJsonStr = (LPSTR)cJsonData.ToString().c_str();

    // 删除自定义数据
    pContext->pUserData = NULL;
    if (pItems)
    {
        delete pItems;
    }

    return WFS_SUCCESS;
}

inline int MulDiv(int number, int numberator, int denominator)
{
    long long ret = number;
    ret *= numberator;
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
void CXFS_PPR::InitConfig()
{
    THISMODULE(__FUNCTION__);

    CHAR    szIniAppName[256];

    strcpy(m_sConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));
    m_sConfig.nDriverType = m_cXfsReg.GetValue("DriverType", DEV_PYCX_MB2);


    m_sConfig.type  = (PTR_TYPE)m_cXfsReg.GetValue("CONFIG", "type", 1);
    // 纸类型
    m_sConfig.bDetectBlackStripe = m_cXfsReg.GetValue("CONFIG", "feedblcakdetect", 1) != 0;
    // 走纸距离
    m_sConfig.nFeed = (int)m_cXfsReg.GetValue("CONFIG", "feedsize", (DWORD)0);
    m_sConfig.dwMarkHeader = m_cXfsReg.GetValue("CONFIG", "MarkHeader", 318);

    m_sConfig.nVerifyField = m_cXfsReg.GetValue("CONFIG", "verify_field", (DWORD)0);
    if (m_sConfig.nVerifyField < 0 || m_sConfig.nVerifyField > 2)
        m_sConfig.nVerifyField = 0;

    m_sConfig.nPageSize = m_cXfsReg.GetValue("CONFIG", "split_size", 2976);
    m_sConfig.nPageLine = m_cXfsReg.GetValue("CONFIG", "page_lines", 30);
    m_sConfig.nLineSize = m_cXfsReg.GetValue("CONFIG", "line_size", 50);    // 30-00-00-00(FT#0008)
    m_sConfig.bEnableSplit = m_cXfsReg.GetValue("CONFIG", "enabled_split", 1) != 0;

    // 读指定设备相关参数
    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_OPENMODE_%d", m_sConfig.nDriverType);

    //回收箱数目
    m_stNoteBoxList.wBoxCount = m_cXfsReg.GetValue("NOTEBOX_CONFIG", "NoteBoxCount", (INT)0);

    // 读指定回收箱相关参数
    for (INT i = 0; i < m_stNoteBoxList.wBoxCount; i ++)
    {
        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "NOTEBOX_%d", i + 1);

        // 票据箱序号
        m_stNoteBoxList.NoteBox[i].bIsHave = TRUE;
        m_stNoteBoxList.NoteBox[i].wBoxNo = i + 1;
        // 票据箱类型
        m_stNoteBoxList.NoteBox[i].wBoxType = m_cXfsReg.GetValue(szIniAppName, "BoxType", (INT)0);
        // 票据类型
        m_stNoteBoxList.NoteBox[i].wNoteType = m_cXfsReg.GetValue(szIniAppName, "NoteType", (INT)0);
        // 存折张数
        m_stNoteBoxList.NoteBox[i].wNoteCount = m_cXfsReg.GetValue(szIniAppName, "NoteCount", (INT)0);
        // 报警阀值(HIGH回收箱/LOW存储箱)
        m_stNoteBoxList.NoteBox[i].wThreshold = m_cXfsReg.GetValue(szIniAppName, "Threshold", (INT)0);
        // FULL报警阀值(回收箱使用)
        m_stNoteBoxList.NoteBox[i].wFullThreshold = m_cXfsReg.GetValue(szIniAppName, "FullThreshold", (INT)0);
    }


    LPCSTR lpFont = m_cXfsReg.GetValue("CONFIG", "FontType", "");
    strcpy(m_stPrintFormat.szFontType, lpFont);
    Log(ThisModule, 1, "加载FontType%s", m_stPrintFormat.szFontType);
    m_stPrintFormat.uFontSize = m_cXfsReg.GetValue("CONFIG", "FontSize", (DWORD)0);
    m_stPrintFormat.ulStyle = m_cXfsReg.GetValue("CONFIG", "Style", (DWORD)0);
    m_stPrintFormat.uWPI = m_cXfsReg.GetValue("CONFIG", "WPI", (DWORD)0);
    m_stPrintFormat.uLPI = m_cXfsReg.GetValue("CONFIG", "LPI", (DWORD)0);
    m_stPrintFormat.uLineHeight = m_cXfsReg.GetValue("CONFIG", "LineHeight", (DWORD)0);
    Log(ThisModule, 1, "Style%d", m_stPrintFormat.ulStyle);

    if (49 > m_sConfig.nPageSize)
    {
        m_sConfig.nPageSize = 50;
    }
    if (2976 < m_sConfig.nPageSize)
    {
        m_sConfig.nPageSize = 2976;
    }

    if (1 > m_sConfig.nPageLine)
    {
        m_sConfig.nPageLine = 1;
    }
    if (44 < m_sConfig.nPageLine)
    {
        m_sConfig.nPageLine = 44;
    }

    if (m_sConfig.nLineSize < 1 || m_sConfig.nLineSize > 50)    // 30-00-00-00(FT#0008)
    {                                                           // 30-00-00-00(FT#0008)
        m_sConfig.nLineSize = 50;                               // 30-00-00-00(FT#0008)
    }                                                           // 30-00-00-00(FT#0008)

    m_sConfig.nTakeSleep = m_cXfsReg.GetValue("TakeCfg", "TakeSleep", (DWORD)3);

    strcpy(m_sConfig.cTrackName1, m_cXfsReg.GetValue("TrackName1", ""));
    strcpy(m_sConfig.cTrackName2, m_cXfsReg.GetValue("TrackName2", ""));
    strcpy(m_sConfig.cTrackName3, m_cXfsReg.GetValue("TrackName3", ""));
}

// 初始化状态类变量
long CXFS_PPR::InitStatus()
{
    memset(&m_sStatus, 0x00, sizeof(WFSPTRSTATUS));
    m_sStatus.fwDevice      = WFS_PTR_DEVNODEVICE;
    for (INT i = 0; i < WFS_PTR_SUPPLYSIZE; i ++)
    {
        m_sStatus.fwPaper[i]    = WFS_PTR_PAPERUNKNOWN;
    }
    m_sStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
    m_sStatus.fwToner       = WFS_PTR_TONERUNKNOWN;
    m_sStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
    m_sStatus.fwInk         = WFS_PTR_INKUNKNOWN;
    m_sStatus.lppRetractBins = nullptr;
}

// 初始化能力值类变量
long CXFS_PPR::InitCaps()
{
    memset(&m_sCaps, 0x00, sizeof(WFSPTRCAPS));
    m_sCaps.fwType = WFS_PTR_TYPEPASSBOOK;
    m_sCaps.lpusMaxRetract = (const LPUSHORT)&m_stNoteBoxList.NoteBox[0].wFullThreshold;       // 每个回收箱可容纳媒介数目(INI获取)(结构体数组指针)
    m_sCaps.usRetractBins = 1;              // 回收箱个数(INI获取)
}

// 状态更新
long CXFS_PPR::UpdateStatus()
{
    return WFS_SUCCESS;
}

// 状态获取
void CXFS_PPR::UpdateDeviceStatus()
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
    BOOL    bNeedFirePaperTaken     = FALSE;    // 需要上报介质拿走事件
    memset(bNeedFirePaperStatus, FALSE, sizeof(bNeedFirePaperStatus));
    memset(bNeedFireRetractStatus, FALSE, sizeof(bNeedFireRetractStatus));


    WFSPTRSTATUS sLastStatus = m_sStatus;

    nRet = m_pPrinter->GetStatus(stDevStatus);
    if (nRet != PTR_SUCCESS)
    {
         m_sStatus.fwDevice = WFS_PTR_DEVHWERROR;
         memset(m_sStatus.fwPaper, WFS_PTR_PAPERUNKNOWN, sizeof(m_sStatus.fwPaper));
         m_sStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
         m_sStatus.fwToner       = WFS_PTR_TONERUNKNOWN;
         m_sStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
         m_sStatus.fwInk         = WFS_PTR_INKUNKNOWN;
         m_sStatus.lppRetractBins = nullptr;
    } else
    {
        m_sStatus.fwDevice = ConvertDeviceStatus(stDevStatus.wDevice);
        m_sStatus.fwMedia = ConvertMediaStatus(stDevStatus.wMedia);
        for (INT i = 0; i < 16; i ++)
        {
            m_sStatus.fwPaper[i] = ConvertPaperStatus(stDevStatus.wPaper[i]);
        }
        m_sStatus.fwToner = ConvertTonerStatus(stDevStatus.wToner);
        m_sStatus.fwInk = ConvertInkStatus(stDevStatus.wInk);

        WFSPTRRETRACTBINS stWFSRetractBin[16] = {0};
        for(int i = 0; i < m_stNoteBoxList.wBoxCount; i++){
           stWFSRetractBin[i].wRetractBin = WFS_PTR_RETRACTBINOK;
           stWFSRetractBin[i].usRetractCount= m_stNoteBoxList.NoteBox[i].wNoteCount;
        }
        m_sStatus.AddRetractBins(stWFSRetractBin[0]);

 /*       for (INT i = 0; i < 16; i++)
        {
            stWFSRetractBin[i].wRetractBin = ConvertRetractStatus(stDevStatus.stRetract[i].wBin);
            stWFSRetractBin[i].usRetractCount = stDevStatus.stRetract[i].usCount;
        }*/

        // Media原状态为出口邮票＆当前状态为通道内无票＆n_WaitTaken为准执行Taken,设置Taken事件上报标记
        if ((m_sStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT) &&
            (sLastStatus.fwDevice == WFS_PTR_MEDIAENTERING) && m_WaitTaken == WTF_TAKEN)
        {
            bNeedFirePaperTaken = TRUE;
            m_WaitTaken = WTF_NONE;
        }

        // 票箱变化事件
        for (INT i = 0; i < 16; i ++)
        {
            if (m_sStatus.fwPaper[i] != sLastStatus.fwPaper[i])
            {
                // 只有状态变为少或空时才Fire状态
                //if (m_sStatus.fwPaper[i] == WFS_PTR_PAPERLOW|| m_sStatus.fwPaper[i] == WFS_PTR_PAPEROUT)
                //{
                    bNeedFirePaperStatus[i] = TRUE;
                //}
            }
        }

        // 碳带变化事件
        if (m_sStatus.fwToner != sLastStatus.fwToner)
        {
            //只有当Toner状态变为少或空时才Fire状态
            //if (m_sStatus.fwToner == WFS_PTR_TONERLOW || m_sStatus.fwToner == WFS_PTR_TONEROUT)
            //{
                bNeedFireTonerStatus = TRUE;
            //}
        }

        // 墨盒变化事件
        if (m_sStatus.fwInk != sLastStatus.fwInk)
        {
            //只有当Ink状态变为少或空时才Fire状态
            //if (m_sStatus.fwToner == WFS_PTR_INKLOW || m_sStatus.fwToner == WFS_PTR_INKOUT)
            //{
                bNeedFireInkStatus = TRUE;
            //}
        }

        // 回收变化事件
        /*for (INT i = 0; i < 16; i ++)
        {
            if (m_sStatus.[i] != sLastStatus.fwPaper[i])
            {
                // 只有状态变为少或空时才Fire状态
                //if (m_sStatus.fwPaper[i] == WFS_PTR_PAPERLOW|| m_sStatus.fwPaper[i] == WFS_PTR_PAPEROUT)
                //{
                    bNeedFirePaperStatus[i] = TRUE;
                //}
            }
        }*/
    }

    // Device状态有变化&当前Device状态为HWERR,设置HWERR事件上报标记
    if (m_sStatus.fwDevice != sLastStatus.fwDevice)
    {
        bNeedFirePrinterStatus = TRUE;
        if (m_sStatus.fwDevice == WFS_PTR_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }


    //--------事件上报处理--------

    if (bNeedFireHWError == TRUE)   // 上报Device HWERR事件
    {
        FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
    }
    if (bNeedFirePrinterStatus == TRUE) // 上报状态变化事件
    {
        FireStatusChanged(m_sStatus.fwDevice);
    }

 /*   for (INT i = 0; i < 16; i ++)   // 上报票箱状态变化
    {
        if (bNeedFirePaperStatus[i] == TRUE && ConvertPaperCode(i + 1) != 0)
        {
            FirePaperThreshold(ConvertPaperCode(i + 1), m_sStatus.fwPaper[i]);
        }
    }*/

    if (bNeedFireTonerStatus == TRUE)   // 上报碳带状态变化
    {
        FireTonerThreshold(m_sStatus.fwToner);
    }

    if (bNeedFireInkStatus == TRUE)   // 上报墨盒状态变化
    {
        FireInkThreshold(m_sStatus.fwInk);
    }

    if (bNeedFirePaperTaken == TRUE)    // 上报Taken事件
    {
        FireMediaTaken();
        Log(ThisModule, 1, IDS_INFO_PAPER_TAKEN);
    }
    return;
}

// 介质控制处理
HRESULT CXFS_PPR::ControlMedia(DWORD dwControl)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;

    if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT ||     // 支持参数: 退票
        (dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT )
    {
        MEDIA_ACTION2 enInput = DIRECTION_FRONT;
        if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)      // 退票
        {
            enInput = DIRECTION_FRONT;
        } else
        if((dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT){
            enInput = DIRECTION_REAR;
        }

         nRet = m_pPrinter->MeidaControl2(enInput);
         if (nRet != PTR_SUCCESS)
         {
             if (nRet == ERR_PTR_JAMMED)
             {
                 m_bNeedKeepJammedStatus = TRUE;
             }

             Log(ThisModule, -1, "m_pPrinter->MeidaControl(%d) Fail, ErrCode = %d, ReturnCode: %d",
                 enInput, nRet, ConvertErrCode(nRet));
             return ConvertErrCode(nRet);
         }
         m_WaitTaken = WTF_TAKEN;
    } else
    {
        // 无效入参
        Log(ThisModule, -1, "接收ControlMedia参数[%d]无效, ReturnCode: %d.",
            dwControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    return WFS_SUCCESS;
}

// 格式化打印处理
HRESULT CXFS_PPR::InnerPrintForm(LPWFSPTRPRINTFORM pInData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    CSPPtrData *pData = (CSPPtrData *)m_pData;
    PrintContext pc;
    memset(&pc, 0, sizeof(pc));

    pc.pPrintData = pInData;

    pc.pForm = pData->FindForm(pInData->lpszFormName);
    if (!pc.pForm)
    {
        Log(ThisModule, -1, IDS_ERR_FORM_NOT_FOUND, pInData->lpszFormName);
        return WFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!pc.pForm->IsLoadSucc())
    {
        Log(ThisModule, -1, IDS_ERR_FORM_INVALID, pInData->lpszFormName);
        return WFS_ERR_PTR_FORMINVALID;
    }
    pc.pMedia = pData->FindMedia(pInData->lpszMediaName);
    if (!pc.pMedia)
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_NOT_FOUND, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIANOTFOUND;
    }
    if (!pc.pMedia->IsLoadSucc())
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_INVALID, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIAINVALID;
    }

    CSPPrinterMedia *pMedia = m_pData->FindMedia((LPCSTR)pInData->lpszMediaName);
    m_pData->m_LastMedia.ExtractFromMedia(pMedia);
    m_MediaTop = m_pData->m_LastMedia.wRestrictedAreaY;
    m_MediaBotton = m_pData->m_LastMedia.wRestrictedAreaHeight;

    // 检查是否Media可打印区域超过其自身大小
    do
    {
        RECT rectMedia = {0, 0, 0, 0};
        pc.pMedia->GetPrintArea(rectMedia);
        SIZE sizeMedia = pc.pMedia->GetSize();
        if (rectMedia.right > sizeMedia.cx ||
            rectMedia.bottom > sizeMedia.cy)
        {
            Log(ThisModule, -1, IDS_ERR_MEDIA_INVALID, pc.pMedia->GetName());
            return WFS_ERR_PTR_MEDIAINVALID;
        }
    } while (0);

    RECT rcMD;
    ((CSPPrinterForm *)pc.pForm)->GetMulDiv(rcMD);

    if (WFS_PTR_OFFSETUSEFORMDEFN == pInData->wOffsetX)
    {
        pInData->wOffsetX = pc.pForm->GetPosition().cx;
    }
    if (WFS_PTR_OFFSETUSEFORMDEFN == pInData->wOffsetY)
    {
        pInData->wOffsetY = pc.pForm->GetPosition().cy;
    }
    SIZE offset;
    offset.cx = MulDiv(pInData->wOffsetX, rcMD.left, rcMD.right);
    offset.cy = MulDiv(pInData->wOffsetY, rcMD.top, rcMD.bottom);

    SIZE sizeForm = pc.pForm->GetSize();
    SIZE sizeMedia = pc.pMedia->GetSize();
    if ((sizeMedia.cx > 0 && offset.cx + sizeForm.cx > sizeMedia.cx) ||
        (sizeMedia.cy > 0 && offset.cy + sizeForm.cy > sizeMedia.cy))
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_OVERFLOW, pInData->lpszFormName, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }
    RECT rc;
    pc.pMedia->GetPrintArea(rc);
    if (offset.cx < rc.left ||
        (rc.right - rc.left > 0 && sizeForm.cx + offset.cx > rc.right - rc.left) ||
        offset.cy < rc.top ||
        (rc.bottom - rc.top > 0 && sizeForm.cy + offset.cy > rc.bottom - rc.top))
    {
        Log(ThisModule, -1, IDS_ERR_MEDIA_OVERFLOW_PRINTAREA, pInData->lpszFormName, pInData->lpszMediaName);
        return WFS_ERR_PTR_MEDIAOVERFLOW;
    }

    FORMALIGN FormAlign = pc.pForm->GetAlign();
    if (WFS_PTR_ALNUSEFORMDEFN != pInData->wAlignment &&
        WFS_PTR_ALNTOPLEFT != pInData->wAlignment &&
        WFS_PTR_ALNTOPRIGHT != pInData->wAlignment &&
        WFS_PTR_ALNBOTTOMLEFT != pInData->wAlignment &&
        WFS_PTR_ALNBOTTOMRIGHT != pInData->wAlignment)
    {
        return WFS_ERR_INVALID_DATA;
    }
    if (WFS_PTR_ALNUSEFORMDEFN != pInData->wAlignment)
    {
        FormAlign = (FORMALIGN)(pInData->wAlignment - WFS_PTR_ALNTOPLEFT + TOPLEFT);
    }
    switch (FormAlign)
    {
    case TOPLEFT:
        break;  //(default)
    case TOPRIGHT:
        if (0 < sizeMedia.cx)
        {
            offset.cx = sizeMedia.cx - sizeForm.cx - offset.cx;
        }
        break;
    case BOTTOMLEFT:
        if (0 < sizeMedia.cy)
        {
            offset.cy = sizeMedia.cy - sizeForm.cy - offset.cy;
        }
        break;
    case BOTTOMRIGHT:
        if (0 <  sizeMedia.cx)
        {
            offset.cx = sizeMedia.cx - sizeForm.cx - offset.cx;
        }
        if (0 < sizeMedia.cy)
        {
            offset.cy = sizeMedia.cy - sizeForm.cy - offset.cy;
        }
        break;
    default:
        return WFS_ERR_INVALID_DATA;
    }

    CMultiString Fields = pInData->lpszFields;
    m_Fields = pInData->lpszFields;  //test by guojy

    //功能：打印字段内容或FRAME
    if (m_sConfig.nVerifyField > 0)
    {
        do
        {
            for (int i = 0; i < Fields.GetCount(); i++)
            {
                LPCSTR lpField = Fields.GetAt(i);
                if (NULL == lpField)
                {
                    continue;
                }
                char szFieldName[1024] = {0};
                for (int j = 0; j < (int)strlen(lpField) && j < 1023; j++)
                {
                    if ('=' != lpField[j]  && '\0' != lpField[j])
                    {
                        szFieldName[j] = lpField[j];
                    }
                }
                if (0 == strcmp(szFieldName, ""))
                {
                    continue;
                }
                DWORD iChild = 0;
                for (; iChild < pc.pForm->GetSubItemCount(); iChild++)
                {
                    if (0 == strcmp(pc.pForm->GetSubItem(iChild)->GetName(), szFieldName))
                    {
                        break;
                    }
                }
                if (iChild == pc.pForm->GetSubItemCount())
                {
                    Log(ThisModule, -1, IDS_ERR_FILED_NOT_FOUND, szFieldName);
                    if (m_sConfig.nVerifyField == 1)
                    {
                        return WFS_ERR_PTR_FIELDNOTFOUND;
                    } else
                    {
                        FireFieldWarning(pInData->lpszFormName, szFieldName, WFS_PTR_FIELDNOTFOUND);
                    }
                }
            }

        } while (0);
    }

    HRESULT hRes = StartForm(&pc);
    if (WFS_SUCCESS != hRes)
    {
        Log(ThisModule, Result2ErrorCode(hRes), IDS_ERR_START_FORM, pInData->lpszFormName);
        return hRes;
    }

    for (DWORD iChild = 0; iChild < pc.pForm->GetSubItemCount() && WFS_SUCCESS == hRes; iChild++)
    {
        ISPPrinterItem *pItem = pc.pForm->GetSubItem(iChild);
        SIZE3 SubOffset;
        SubOffset.cx = SubOffset.cy = SubOffset.cz = 0;
        pc.pSubform = NULL;
        if (ITEM_SUBFORM == pItem->GetItemType())
        {
            pc.pSubform = (ISPPrinterSubform *)pItem;
            SubOffset = pc.pSubform->GetPosition();
        }
        for (DWORD iField = 0; (!pc.pSubform || iField < pc.pSubform->GetSubItemCount()) && WFS_SUCCESS == hRes; iField++)
        {
            if (pc.pSubform)
            {
                pItem = pc.pSubform->GetSubItem(iField);
            }
            SIZE OffsetAll = offset;
            OffsetAll.cx += SubOffset.cx;
            OffsetAll.cy += SubOffset.cy;
            hRes = PrintFieldOrFrame(pc, pItem, OffsetAll, Fields);
            if (!pc.pSubform)
            {
                break;
            }
        }
    }

    if (WFS_SUCCESS != hRes)
    {
        pc.bCancel = TRUE;
    }

    HRESULT hResOld = hRes;
    hRes = EndForm(&pc);

    return  WFS_SUCCESS != hResOld ? hResOld : hRes;
}

HRESULT CXFS_PPR::InnerReadForm(LPWFSPTRREADFORM pInData)
{
    THISMODULE(__FUNCTION__);
    CSPPtrData *pData = static_cast<CSPPtrData *>(m_pData);

    HRESULT hRes = WFS_SUCCESS;
    ReadContext rc;
    ZeroMemory(&rc, sizeof(rc));
    rc.pReadData = pInData;
    BOOL bForm = FALSE;

    if(strlen(pInData->lpszFormName) != 0){
        bForm = TRUE;
        rc.pForm = pData->FindReadForm(pInData->lpszFormName);
        if (!rc.pForm)
        {
            Log(ThisModule, __LINE__, IDS_ERR_FORM_NOT_FOUND, pInData->lpszFormName);
            return WFS_ERR_PTR_FORMNOTFOUND;
        }
        if (!rc.pForm->IsLoadSucc())
        {
            Log(ThisModule, __LINE__, IDS_ERR_FORM_INVALID, pInData->lpszFormName);
            return WFS_ERR_PTR_FORMINVALID;
        }
    /*    rc.pMedia = pData->FindMedia(pInData->lpszMediaName);
        if (!rc.pMedia)
        {
            Log(ThisModule, __LINE__, IDS_ERR_MEDIA_NOT_FOUND, pInData->lpszMediaName);
            return WFS_ERR_PTR_MEDIANOTFOUND;
        }
        if (!rc.pMedia->IsLoadSucc())
        {
            Log(ThisModule, __LINE__, IDS_ERR_MEDIA_INVALID, pInData->lpszMediaName);
            return WFS_ERR_PTR_MEDIAINVALID;
        }*/

    }

    CMultiString Fields = pInData->lpszFieldNames;//  如果参数为空，即未指定任何磁道，则要读所有在Form文件中的磁道

    if (Fields.GetCount() == 0)
    {
        for (DWORD iChild = 0; iChild < rc.pForm->GetSubItemCount() && hRes == WFS_SUCCESS; iChild++)
        {
            ISPPrinterItem *pItem = rc.pForm->GetSubItem(iChild);
            Fields.Add(pItem->GetName());
        }
    }

    hRes = EndReadForm2(Fields,pInData->dwMediaControl);

    return hRes;
}

BOOL CXFS_PPR::NeedFormatString() const
{
    //    if (nullptr == m_pPrinter || m_pPrinter->IsJournalPrinter())
    //    {
    //        return FALSE;
    //    }

    /*if (m_sConfig.nDriverType == DEV_SNBC_BKC310 || m_sConfig.nDriverType == DEV_SNBC_BTNH80) // SNBC
    {
        return FALSE;
    }*/

    return FALSE;
}

void CXFS_PPR::RemoveUnPrintableChar(ULONG ulInSize, const LPBYTE pInData, ULONG &ulOutSize, LPBYTE pOutData)
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




//-----------------------------------------------------------------------------------
//--------------------------------------事件消息---------------------------------------
// 上报Device HWERR事件
void CXFS_PPR::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

// 上报状态变化事件
void CXFS_PPR::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

// 上报无媒介事件
void CXFS_PPR::FireNoMedia(LPCSTR szPrompt)
{
    //FireEvent(MFT_EE, WFS_EXEE_PTR_NOMEDIA, WFS_SUCCESS, (LPVOID)szPrompt);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_NOMEDIA, (LPVOID)szPrompt);
}

// 上报媒介放入事件
void CXFS_PPR::FireMediaInserted()
{
    //FireEvent(MFT_EE, WFS_EXEE_PTR_MEDIAINSERTED);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAINSERTED, nullptr);
}

// 上报Field错误事件
void CXFS_PPR::FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName  = (LPSTR)szFormName;
    fail.lpszFieldName = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    //FireEvent(MFT_EE, WFS_EXEE_PTR_FIELDERROR, WFS_SUCCESS, &fail);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDERROR, &fail);
}

// 上报Field警告事件
void CXFS_PPR::FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName   = (LPSTR)szFormName;
    fail.lpszFieldName  = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    //FireEvent(MFT_EE, WFS_EXEE_PTR_FIELDWARNING, WFS_SUCCESS, &fail);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDWARNING, &fail);
}

// 上报回收箱变化事件
void CXFS_PPR::FireRetractBinThreshold(USHORT BinNumber, WORD wStatus)
{
    WFSPTRBINTHRESHOLD BinThreshold;
    BinThreshold.usBinNumber     = BinNumber;
    BinThreshold.wRetractBin = wStatus;
    //FireEvent(MFT_UE, WFS_USRE_PTR_RETRACTBINTHRESHOLD, WFS_SUCCESS, &BinThreshold);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_RETRACTBINTHRESHOLD, &BinThreshold);
}

// 上报媒介取走事件
void CXFS_PPR::FireMediaTaken()
{
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIATAKEN);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIATAKEN, nullptr);
}

// 上报纸状态/票箱状态变化事件
void CXFS_PPR::FirePaperThreshold(WORD wSrc, WORD wStatus)
{
    WFSPTRPAPERTHRESHOLD PaperThreshold;
    PaperThreshold.wPaperSource     = wSrc;
    PaperThreshold.wPaperThreshold  = wStatus;
    //FireEvent(MFT_UE, WFS_USRE_PTR_PAPERTHRESHOLD, WFS_SUCCESS, &PaperThreshold);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_PAPERTHRESHOLD, &PaperThreshold);
}

// 上报碳带状态变化事件
void CXFS_PPR::FireTonerThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_TONERTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_TONERTHRESHOLD, &wStatus);
}

// 上报墨盒状态变化事件
void CXFS_PPR::FireInkThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_INKTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_INKTHRESHOLD, &wStatus);
}

// 上报灯状态变化事件
void CXFS_PPR::FireLampThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_LAMPTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_LAMPTHRESHOLD, &wStatus);
}

// 本事件指出在没有任何打印执行命令执行的情况下物理媒体已插入设备中。
// 本事件只有当媒介自动进入时才会生成。
void CXFS_PPR::FireSRVMediaInserted()
{
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIAINSERTED);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIAINSERTED, nullptr);
}

// 上报复位中检测到设备内有媒介事件
void CXFS_PPR::FireMediaDetected(WORD wPos, USHORT BinNumber)
{
    WFSPTRMEDIADETECTED md;
    md.wPosition            = wPos;
    md.usRetractBinNumber   = BinNumber;
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIADETECTED, WFS_SUCCESS, &md);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIADETECTED,  &md);
}


//-----------------------------------------------------------------------------------
//------------------------------------格式转换WFS-------------------------------------
// 错误码转换为WFS格式
WORD CXFS_PPR::ConvertErrCode(INT nRet)
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
        default:                    return WFS_ERR_HARDWARE_ERROR;    }
}

// 设备状态转换为WFS格式
WORD CXFS_PPR::ConvertDeviceStatus(WORD wDevStat)
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
WORD CXFS_PPR::ConvertMediaStatus(WORD wMediaStat)
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
WORD CXFS_PPR::ConvertPaperStatus(WORD wPaperStat)
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
WORD CXFS_PPR::ConvertTonerStatus(WORD wTonerStat)
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
WORD CXFS_PPR::ConvertInkStatus(WORD wInkStat)
{
    switch (wInkStat)
    {
    case INK_STAT_FULL    /* 墨盒状态满或正常 */  : return WFS_PTR_INKFULL;
    case INK_STAT_LOW     /* 墨盒少 */           : return WFS_PTR_INKLOW;
    case INK_STAT_OUT     /* 墨盒无 */           : return WFS_PTR_INKOUT;
    case INK_STAT_NOTSUPP /* 设备不支持该能力 */  : return WFS_PTR_INKNOTSUPP;
    case INK_STAT_UNKNOWN /* 不能确定当前状态 */  : return WFS_PTR_INKUNKNOWN;
    default: return WFS_PTR_INKUNKNOWN;    }
}

// Retract状态转换为WFS格式
WORD CXFS_PPR::ConvertRetractStatus(WORD wRetractStat)
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
WORD CXFS_PPR::ConvertPaperCode(INT nCode)
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
WORD CXFS_PPR::NoteTypeConvert(LPSTR lpNoteType, WORD wBank)
{
    #define IF_CMCP(X, Y, Z) \
        if (memcmp(X, Y, strlen(X)) == 0 && memcmp(X, Y, strlen(Y)) == 0) \
        return Z

    if (wBank == 1) // 邮储
    {
        IF_CMCP(lpNoteType, "1", NOTE_TYPE_PTCD);       // 普通存单
        else
        IF_CMCP(lpNoteType, "2", NOTE_TYPE_XPCD);       // 芯片存单
        else
        IF_CMCP(lpNoteType, "3", NOTE_TYPE_DECD);       // 大额存单
        else
        IF_CMCP(lpNoteType, "4", NOTE_TYPE_GZPZ);       // 国债凭证
        else
        IF_CMCP(lpNoteType, "5", NOTE_TYPE_JSYWWTS);    // 结算业务委托书
        else
        IF_CMCP(lpNoteType, "6", NOTE_TYPE_XJZP);       // 现金支票
        else
        IF_CMCP(lpNoteType, "7", NOTE_TYPE_ZZZP);       // 转账支票
        else
        IF_CMCP(lpNoteType, "8", NOTE_TYPE_QFJZP);      // 清分机支票
        else
        IF_CMCP(lpNoteType, "9", NOTE_TYPE_YHHP);       // 银行汇票
        else
        IF_CMCP(lpNoteType, "A", NOTE_TYPE_YHCDHP);     // 银行承兑汇票
        else
        IF_CMCP(lpNoteType, "B", NOTE_TYPE_SYCDHP);     // 商业承兑汇票
        else
        IF_CMCP(lpNoteType, "C", NOTE_TYPE_FQFJBP);     // 非清分机本票
        else
        IF_CMCP(lpNoteType, "D", NOTE_TYPE_QFJBP);      // 清分机本票
        else
        return NOTE_TYPE_INV;                           // 未知
    } else
    {
        return 0;
    }
}

// 指定票据类型是否对应票箱存在
WORD CXFS_PPR::NoteTypeIsHave(LPSTR lpNoteType, WORD wBox)
{
    // 票箱指定票据类型: 1普通存单/2芯片存单/3大额存单/4国债凭证/5结算业务委托书/6现金支票/7转账支票/
    //                 8清分机支票/9银行汇票/10银行承兑汇票/11商业承兑汇票/12非清分机本票/13清分机本票

    WORD wNoteTypeTmp = 0;
}

HRESULT CXFS_PPR::ReadItem(CMultiString cFieldInfo)
{
    THISMODULE(__FUNCTION__);
    char strFieldName[MAX_FIELD_SIZE] = {0};
    BOOL bIsTrack1 = FALSE;
    BOOL bIsTrack2 = FALSE;
    BOOL bIsTrack3 = FALSE;
    LPCSTR lpStr = nullptr;
    int iRet = 0;

    BOOL bIsNull1 = FALSE;
    BOOL bIsNull2 = FALSE;
    BOOL bIsNull3 = FALSE;

    char cReadFormInParam[64];
    char cReadFormOutParam[256];
    memset(cReadFormInParam, 0x00, sizeof(cReadFormInParam));
    memset(cReadFormOutParam, 0x00, sizeof(cReadFormOutParam));

    DEVPTRREADFORMIN DevReadFormIn;
    DEVPTRREADFORMOUT DevReadFormOut;
    DEVPTRREADFORMOUT DevReadFormOut3;
    DevReadFormIn.Clear();
    DevReadFormOut.Clear();

    DevReadFormIn.lpData = cReadFormInParam;
    DevReadFormOut.lpData = cReadFormOutParam;

    int iStrackNum = 0;
    char cFields[3000];
    LONG lMagneticType = 0;
    LONG lMagneticType3 = 0;
    memset(cFields, 0x00, sizeof(cFields));

    for(int i = 0; i < cFieldInfo.GetCount(); i++){
        lpStr = cFieldInfo.GetAt(i);
        memcpy(strFieldName, lpStr, strlen(lpStr));

        if(strcmp(m_sConfig.cTrackName1,strFieldName) == 0){
            bIsTrack1 = TRUE;
        }
        if(strcmp(m_sConfig.cTrackName2,strFieldName) == 0){
            bIsTrack2 = TRUE;
            lMagneticType = 0;
        }
        if(strcmp(m_sConfig.cTrackName3,strFieldName) == 0){
            bIsTrack3 = TRUE;
            lMagneticType3 = 1;
        }
    }

    if(bIsTrack1 == TRUE){
//        return  WFS_ERR_UNSUPP_DATA;
        m_pData->m_ReadFormOut.Add(m_sConfig.cTrackName1,"");
        memcpy(cFields + iStrackNum, m_sConfig.cTrackName1, strlen(m_sConfig.cTrackName1));
        iStrackNum += strlen(m_sConfig.cTrackName1);
        cFields[iStrackNum++] = '=';
        cFields[iStrackNum++] = '\0';
        bIsNull1 = TRUE;
    }

 //   memcpy(cFields + iStrackNum, m_sConfig.cTrackName2, strlen(m_sConfig.cTrackName2));
//    iStrackNum += strlen(m_sConfig.cTrackName2);
//    cFields[iStrackNum++] = '=';
 //   LPSTR lpStrTrackData = nullptr;
    if(bIsTrack2 == TRUE){
        memcpy(cReadFormInParam, m_sConfig.cTrackName2, strlen(m_sConfig.cTrackName2));
        iRet = m_pPrinter->ReadForm2(DevReadFormIn, DevReadFormOut, lMagneticType);
        if(iRet == ERR_PTR_JAMMED){
            return WFS_ERR_PTR_MEDIAJAMMED;
        }
        if(iRet == ERR_PTR_CANCEL){
            return WFS_ERR_CANCELED;
        }
  /*      iDataLen = strlen(DevReadFormOut.lpData);
        lpStrTrackData = new CHAR[iDataLen+2];
        memset(lpStrTrackData, NULL, (iDataLen+2));
        memcpy(lpStrTrackData, DevReadFormOut.lpData, iDataLen);
        lpStrTrackData[iDataLen] = '\0';
        if(bIsTrack3 == FALSE){
           lpStrTrackData[iDataLen + 1] = '\0';
       }*/
        if(strlen(DevReadFormOut.lpData) == 0){
            bIsNull2 = TRUE;
        }
        m_pData->m_ReadFormOut.Add(m_sConfig.cTrackName2,DevReadFormOut.lpData);
 //       if(lpStrTrackData != nullptr)
 //           delete[] lpStrTrackData;
//        m_pData->m_ReadFormOut.Add(m_sConfig.cTrackName2,DevReadFormOut.lpData);
 //       memcpy(cFields + iStrackNum, DevReadFormOut.lpData, strlen(DevReadFormOut.lpData));
 //       iStrackNum += strlen(DevReadFormOut.lpData);

    }
 //   cFields[iStrackNum++] = '\0';


    memcpy(cFields + iStrackNum, m_sConfig.cTrackName3, strlen(m_sConfig.cTrackName3));
    iStrackNum += strlen(m_sConfig.cTrackName3);
    cFields[iStrackNum++] = '=';
    if(bIsTrack3 == TRUE){
        memcpy(cReadFormInParam, m_sConfig.cTrackName3, strlen(m_sConfig.cTrackName3));
        iRet = m_pPrinter->ReadForm2(DevReadFormIn, DevReadFormOut,lMagneticType3);
        if(iRet == ERR_PTR_JAMMED){
            return WFS_ERR_PTR_MEDIAJAMMED;
        }
        if(iRet == ERR_PTR_CANCEL){
            return WFS_ERR_CANCELED;
        }
        if(strlen(DevReadFormOut.lpData) == 0){
            bIsNull3 = TRUE;
        }

  /*      iDataLen = strlen(DevReadFormOut.lpData);
        lpStrTrackData = new CHAR[iDataLen+2];
        memset(lpStrTrackData, NULL, (iDataLen+2));
        memcpy(lpStrTrackData, DevReadFormOut.lpData, iDataLen);
        lpStrTrackData[iDataLen] = '\0';
        lpStrTrackData[iDataLen + 1] = '\0';*/
        m_pData->m_ReadFormOut.Add(m_sConfig.cTrackName3,DevReadFormOut.lpData);
  //      memcpy(cFields + iStrackNum, DevReadFormOut.lpData, strlen(DevReadFormOut.lpData));
  //      iStrackNum += strlen(DevReadFormOut.lpData);
    }
  //  cFields[iStrackNum++] = '\0';
  //  cFields[iStrackNum++] = '\0';
//    iDataLen = strlen(DevReadFormOut.lpData);
//    *(DevReadFormOut.lpData+iDataLen +1) = '\0';

//    if(lpStrTrackData != nullptr)
 //       delete[] lpStrTrackData;
    if((bIsNull1 == TRUE) && (bIsNull2 == TRUE) && (bIsNull3 == TRUE)){
        return WFS_ERR_PTR_MEDIAINVALID;
    }else {
        return WFS_SUCCESS;
    }
}

HRESULT CXFS_PPR::GetFormFieldToJSON(LPSTR lpForm, CJsonObject &cJson)
{
    THISMODULE(__FUNCTION__);
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
    if (!rc.pForm->GetSubItemCount() < 1) // 无Field
    {
        Log(ThisModule, __LINE__, IDS_ERR_FIELD_EMPTY, lpForm);
        return WFS_ERR_PTR_FORMINVALID;
    }

    // 循环取Field
    cJson.Clear();
    cJson.Add(JSON_KEY_USE_AREA, 1);
    cJson.Add(JSON_KEY_MEDIA_WIDTH, rc.pForm->GetSize().cx);    // 介质宽(单位:MM)
    cJson.Add(JSON_KEY_MEDIA_HEIGHT, rc.pForm->GetSize().cx);   // 介质高(单位:MM)
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
        cJson.Add(szIdenKey, pItem->GetSize().cx);
        // 起始坐标Y(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_START_Y, iChild + 1);
        cJson.Add(szIdenKey, pItem->GetSize().cy);
        // 可用宽(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_WIDTH, iChild + 1);
        cJson.Add(szIdenKey, pItem->GetPosition().cy);
        // 可用高(单位:MM)
        ZeroMemory(szIdenKey, sizeof(szIdenKey));
        sprintf(szIdenKey, "%s%d", JSON_KEY_AREA_HEIGHT, iChild + 1);
        cJson.Add(szIdenKey, pItem->GetPosition().cy);
    }
    cJson.Add(JSON_KEY_IDEN_CNT, iChild);   // 项数目

    return WFS_SUCCESS;
}

HRESULT CXFS_PPR::EndForm(PrintContext *pContext)
{
    THISMODULE(__FUNCTION__);

    #define ROWCOL2MM(ROL, MM) \
    pContext->pForm->GetOrigUNIT(nullptr) == FORM_ROWCOLUMN ? \
        (int)((float)(ROL * MM / 10) + 0.5) : ROL

    SIZE stSize = GetPerRowColTwips2MM();
    PRINT_ITEMS *pItems = (PRINT_ITEMS *)pContext->pUserData;
    if (pContext->bCancel)
    {
        if (pItems)
        {
            delete pItems;
            pItems = NULL;
        }
        pContext->pUserData = NULL;
        return WFS_SUCCESS;
    }

    // 排序打印ITEM
    qsort(pItems->pItems, pItems->nItemNum, sizeof(PRINT_ITEM *), ComparePrintItem);

    //等待插折
    BOOL bMediaExist = FALSE;
    DEVPTRSTATUS cDevStatus;
    int iRet = 0;

    while(!bMediaExist && !bCancelInsertMedia /*&& !bInsertMediaTimeOut*/){
        if((iRet = m_pPrinter->GetStatus(cDevStatus)) >= 0){
            switch(cDevStatus.wMedia) {
            case WFS_PTR_MEDIANOTPRESENT:
                  CQtTime::Sleep(2000);
                break;
             case WFS_PTR_MEDIAPRESENT:  //存折已经在机器里面时，发printform,media状态不定是什么。
             case WFS_PTR_MEDIAENTERING:   //test#32
             case WFS_PTR_MEDIAUNKNOWN://test#32
             case WFS_PTR_MEDIAJAMMED://test#32
             case WFS_PTR_MEDIANOTSUPP://test#32
             case WFS_PTR_MEDIARETRACTED://test#32
                bMediaExist = TRUE;
                break;
            }
        }
    }
    if(bCancelInsertMedia){
        bCancelInsertMedia = FALSE;
        if (pItems)
        {
            delete pItems;
            pItems = NULL;
        }
        pContext->pUserData = NULL;
        return WFS_SUCCESS;
    }

    // 形成打印字串strupr
    int nCurColHeight = 1;
    PRINT_STRING s; // 记录当前行列位置
    BOOL bAppendNewLine = TRUE;
    int nRet = 0;
    int iLineStart = 9;
    char cFormNameTemp[128] = {0};

    char szPrintData[MAX_PRINTDATA_LEN] = {0};
    DWORD dwDataSize = 0;
    int iYBol = 0;
    int iYBolOld = 0;
    int iRowtemp = 1;
    int iRowBak = 1;
    int iSpaceRaw = 0;


    memcpy(cFormNameTemp,pContext->pPrintData->lpszFormName, strlen(pContext->pPrintData->lpszFormName));

    //for cscb form:RegularBook
    BOOL bUFlag = FALSE;
    BOOL bDFlag = FALSE;

    LPCSTR lpField = m_Fields.GetAt(0);
    char szFieldName[1024] = {0};
    for (int j = 0; j < (int)strlen(lpField) && j < 1023; j++)
    {
        if ('=' != lpField[j]  && '\0' != lpField[j])
        {
            szFieldName[j] = lpField[j];
        }
    }
    if((memcmp(szFieldName, "DepAmountU", 10) == 0 ) ||
       (memcmp(szFieldName, "DepAmountD",10) == 0 ) ||
       (memcmp(szFieldName, "DateU",5) == 0 ) ||
       (memcmp(szFieldName, "DateD",5) == 0 )){
        //for cscb form:RegularBook
        for (int i = 0; i < pItems->nItemNum && nRet >= 0; i++)
        {
            PRINT_STRING strFormat; // 记录格式化数据
            PRINT_ITEM *pItem = pItems->pItems[i];
            iYBol = pItem->y;
            if(i == 0){
                iYBolOld = iYBol;
            }

            while (pItem->y > s.m_nCurRow)
            {
                s.m_nCurRow += nCurColHeight;
                nCurColHeight = 1;
                s.m_nCurCol = 0;
                strFormat.Append('\n');
            }
            while (pItem->x > s.m_nCurCol)
            {
                s.m_nCurCol++;
                strFormat.Append(' ');
            }

            if((iYBol >= 10) && (iYBol <= 17)){
                bUFlag = TRUE;
                bDFlag = FALSE;
            }else if ((iYBol >= 21) && (iYBol <= 33)){
                bDFlag = TRUE;
                bUFlag = FALSE;
            }else{
                bDFlag = FALSE;
                bUFlag = FALSE;
             }

            //字串打印
            PRINT_STRING strText;
            strText.Append(strFormat.GetData(), strFormat.GetLen());
            strText.Append(pItem->Text, pItem->nTextLen);
            s.m_nCurCol += pItem->nWidth;
            if (nCurColHeight < pItem->nHeight)
            {
                nCurColHeight = pItem->nHeight;
            }
            if (pItem->nTextLen > 0)
            {
                memset(szPrintData, 0x00, sizeof(szPrintData));
                memcpy(szPrintData, pItem->Text, pItem->nTextLen);
                float fx = ROWCOL2MM(pItem->x, stSize.cx);
                float fy = ROWCOL2MM(pItem->y, stSize.cy);
                iSpaceRaw = m_stPrintFormat.uLPI;
                if(bUFlag == TRUE){
                    fy = fy + (pItem->y-iLineStart) * iSpaceRaw + 1;
                }else if(bDFlag == TRUE){
                    fy = fy + (pItem->y-iLineStart) * iSpaceRaw + 1;
                }else{
                    ;
                }
                nRet = PrintData2(pItem->Text, pItem->nTextLen, (float)fx*10, (float)fy*10);
            }
        }
    }else if(strcmp(cFormNameTemp, "CurrentBook") == 0){
        iSpaceRaw = m_stPrintFormat.uLPI + 1;
        iLineStart = 5;
        //for cscb form:PassbookFormPrint
        for (int i = 0; i < pItems->nItemNum && nRet >= 0; i++)
        {
            PRINT_STRING strFormat; // 记录格式化数据
            PRINT_ITEM *pItem = pItems->pItems[i];
            iYBol = pItem->y;
            if(i == 0){
                iYBolOld = iYBol;
            }

            while (pItem->y > s.m_nCurRow)
            {
                s.m_nCurRow += nCurColHeight;
                nCurColHeight = 1;
                s.m_nCurCol = 0;
                strFormat.Append('\n');
            }
            while (pItem->x > s.m_nCurCol)
            {
                s.m_nCurCol++;
                strFormat.Append(' ');
            }

            //字串打印
             PRINT_STRING strText;
            strText.Append(strFormat.GetData(), strFormat.GetLen());
            strText.Append(pItem->Text, pItem->nTextLen);
            s.m_nCurCol += pItem->nWidth;
            if (nCurColHeight < pItem->nHeight)
            {
                nCurColHeight = pItem->nHeight;
            }
            if (pItem->nTextLen > 0)
            {
                memset(szPrintData, 0x00, sizeof(szPrintData));
                memcpy(szPrintData, pItem->Text, pItem->nTextLen);
                float fx = ROWCOL2MM(pItem->x, stSize.cx);
                float fy = ROWCOL2MM(pItem->y, stSize.cy);

                if((iRowtemp == 1) && iYBol == iYBolOld){
                    fy = fy + (pItem->y-iLineStart) * iSpaceRaw + 0.5;
                }else if(iYBol != iYBolOld){
                    iRowtemp++;
                    fy = fy + (pItem->y-iLineStart) * iSpaceRaw + 1;
                    iRowBak = iRowtemp;
                    iYBolOld = iYBol;
                }else if(iYBol == iYBolOld){
                    fy = fy + (pItem->y-iLineStart) * iSpaceRaw + 1;
                }
 //               fy = fy + (pItem->y-iLineStart) * iSpaceRaw + 1;
                nRet = PrintData2(pItem->Text, pItem->nTextLen, (float)fx*10, (float)fy*10);
            }
        }
    }else{
        //for cscb form:PassbookFormPrint
        for (int i = 0; i < pItems->nItemNum && nRet >= 0; i++)
        {
            PRINT_STRING strFormat; // 记录格式化数据
            PRINT_ITEM *pItem = pItems->pItems[i];
            iYBol = pItem->y;
            if(i == 0){
                iYBolOld = iYBol;
            }

            while (pItem->y > s.m_nCurRow)
            {
                s.m_nCurRow += nCurColHeight;
                nCurColHeight = 1;
                s.m_nCurCol = 0;
                strFormat.Append('\n');
            }
            while (pItem->x > s.m_nCurCol)
            {
                s.m_nCurCol++;
                strFormat.Append(' ');
            }

            //字串打印
             PRINT_STRING strText;
            strText.Append(strFormat.GetData(), strFormat.GetLen());
            strText.Append(pItem->Text, pItem->nTextLen);
            s.m_nCurCol += pItem->nWidth;
            if (nCurColHeight < pItem->nHeight)
            {
                nCurColHeight = pItem->nHeight;
            }
            if (pItem->nTextLen > 0)
            {
                memset(szPrintData, 0x00, sizeof(szPrintData));
                memcpy(szPrintData, pItem->Text, pItem->nTextLen);
                float fx = ROWCOL2MM(pItem->x, stSize.cx);
                float fy = ROWCOL2MM(pItem->y, stSize.cy);
                if((iRowtemp == 1) && iYBol == iYBolOld){
                    fy += m_stPrintFormat.uLPI;
                }else if(iYBol != iYBolOld){
                    if(iRowtemp == m_MediaTop){
                        iSpaceRaw = m_MediaBotton * m_stPrintFormat.uLineHeight;
                        iRowtemp += m_MediaBotton;
                    }else{
                        iRowtemp++;
                    }
                    iRowBak = iRowtemp;
                    fy = fy + iSpaceRaw + iRowtemp * m_stPrintFormat.uLPI;
                    iYBolOld = iYBol;
                }else if(iYBol == iYBolOld){
                    fy = fy + iSpaceRaw + iRowBak * m_stPrintFormat.uLPI;
                    iYBolOld = iYBol;
                }
                nRet = PrintData2(pItem->Text, pItem->nTextLen, (float)fx*10, (float)fy*10);
            }else if(pItem->nFieldType == FT_GRAPHIC)
            {
                memset(szPrintData, 0x00, sizeof(szPrintData));
                memcpy(szPrintData, pItem->Text, pItem->nTextLen);
                float fx = ROWCOL2MM(pItem->x, stSize.cx);
                float fy = ROWCOL2MM(pItem->y, stSize.cy);
                if((iRowtemp == 1) && iYBol == iYBolOld){
                    fy += m_stPrintFormat.uLPI;
                }else if(iYBol != iYBolOld){
                    if(iRowtemp == m_MediaTop){
                        iSpaceRaw = m_MediaBotton * m_stPrintFormat.uLineHeight;
                        iRowtemp += m_MediaBotton;
                    }else{
                        iRowtemp++;
                    }
                    iRowBak = iRowtemp;
                    fy = fy + iSpaceRaw + iRowtemp * m_stPrintFormat.uLPI;
                    iYBolOld = iYBol;
                }else if(iYBol == iYBolOld){
                    fy = fy + iSpaceRaw + iRowBak * m_stPrintFormat.uLPI;
                    iYBolOld = iYBol;
                }
                nRet = PrintImage(pItem->strImagePath, (float)fx*10, (float)fy*10);
            }
        }
    }

    // 删除自定义数据
    pContext->pUserData = NULL;
    if (pItems)
    {
        delete pItems;
    }

    if (0 > nRet)
    {
     return nRet;
    }

    // 控制MEDIA
    int nResult = 0;
    DWORD dwMediaControl = pContext->pPrintData->dwMediaControl;
    if (0 != dwMediaControl)
    {
        nResult = ControlMedia(dwMediaControl);
    }

    return nResult;

}



