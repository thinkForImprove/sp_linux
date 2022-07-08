/***************************************************************
* 文件名称：XFS_DSR_DEC.cpp
* 文件描述：文档扫描模块命令子处理接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年11月6日
* 文件版本：1.0.0.1
****************************************************************/

#include "XFS_DSR.h"
#include "file_access.h"
#include "data_convertor.h"

//-----------------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_DSR::StartOpen(BOOL bReConn)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    char szDevDSRVer[64] = { 0x00 };
    char szFWVersion[64] = { 0x00 };

    if (m_stConfig.nDeviceType != DEV_BSD216)
    {
        Log(ThisModule, __LINE__, "Invalid Dev type, INI　指定了不支持的设备类型　[%d], Return :%d.",
            m_stConfig.nDeviceType, WFS_ERR_HARDWARE_ERROR);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 加载DevDSR动态库
    if (!LoadDevDll(ThisModule))
    {
        return WFS_ERR_HARDWARE_ERROR;
    }

    // Open前下传初始参数(非断线重连)
    if (bReConn == FALSE)
    {
        // 设置SDK路径
        if (strlen(m_stConfig.szSDKPath) > 0)
        {
            m_DSRinter->SetData(m_stConfig.szSDKPath, DTYPE_LIB_PATH);
        }

        // 设置设备打开模式
        m_DSRinter->SetData(&(m_stConfig.stDevOpenMode), SET_DEV_OPENMODE);
    }

    // 打开设备
    int nRet = m_DSRinter->Open(DEVTYPE2STR(m_stConfig.nDeviceType));
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "Open[%s] fail, ErrCode = %d, Return: %d",
            DEVTYPE2STR(m_stConfig.nDeviceType), nRet, ConvertErrCode(nRet));
        return ConvertErrCode(nRet);
    }

    nRet = OnInit();

    if (bReConn != TRUE)
    {
        SetInit();      // 非断线重连时初始功能设置
    }

    m_DSRinter->GetVersion(szDevDSRVer, 64, GET_VER_DEV);
    m_DSRinter->GetVersion(szFWVersion, 64, GET_VER_FW);

    m_cExtra.AddExtra("VRTCount", "3");
    m_cExtra.AddExtra("VRTDetail[00]", (char*)byVRTU_XFS);
    m_cExtra.AddExtra("VRTDetail[01]", szDevDSRVer);
    m_cExtra.AddExtra("VRTDetail[02]", szFWVersion);

    // 更新一次状态
    OnStatus();

    Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s.", m_cExtra.GetExtraInfo().c_str());

    return WFS_SUCCESS;
}

// Open设备后相关功能初始设置
HRESULT CXFS_DSR::SetInit()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// 等待介质放入
HRESULT CXFS_DSR::MediaInsertWait(DWORD &dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    INT nMediaWait = MEDINS_OK;             // 等待放折结果

    nMediaWait = MediaInsertWait_Pub(dwTimeOut);
    if (nMediaWait != MEDINS_OK)
    {
        // 取消
        if (nMediaWait == MEDINS_CANCEL)
        {
            return WFS_ERR_CANCELED;
        }
        // 介质JAM
        if (nMediaWait == MEDINS_JAM)
        {
            return WFS_ERR_PTR_MEDIAJAMMED;
        }
        // 放折超时
        if (nMediaWait == MEDINS_TIMEOUT)
        {
            return WFS_ERR_TIMEOUT;
        }
        // 断线
        if (nMediaWait == MEDINS_OFFLINE)
        {
            m_wNeedFireEvent[0] = 1;
            return WFS_ERR_HARDWARE_ERROR;
        }
        // 故障
        if (nMediaWait == MEDINS_HWERR)
        {            
            m_wNeedFireEvent[0] = 1;
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    // 介质放入,上报Insert事件
    Log(ThisModule, __LINE__, "介质插入,上报MediaInsert.");

    FireMediaInserted();

    return WFS_SUCCESS;
}

// 等待介质放入
// 返回: 参考[介质放入等待处理 宏定义]
int CXFS_DSR::MediaInsertWait_Pub(DWORD &dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    BOOL bMediaExist = FALSE;       // 介质是否就位
    BOOL bMediaJAM = FALSE;         // 介质JAM
    INT nRetMediaInsert = PTR_SUCCESS;

    DWORD dwTimeOutTmp = dwTimeOut; // 超时毫秒数
    qint64 dwLastTime = QDateTime::currentMSecsSinceEpoch();  // 超时计数:起始时间
    qint64 dwLocalTime = QDateTime::currentMSecsSinceEpoch(); // 超时计数:当前时间

    while (bMediaExist != TRUE && m_bCmdCanceled != TRUE && bMediaJAM != TRUE)
    {
        dwLocalTime = QDateTime::currentMSecsSinceEpoch(); // 取当前时间
        if (dwLocalTime - dwLastTime >= dwTimeOutTmp)
        {
            Log(ThisModule, __LINE__, "等待介质插入超时.");
            return MEDINS_TIMEOUT;   // 超时
        }

        //m_DSRinter->MeidaControl(MEDIA_CTR_PARTIALCUT, 500);    // 介质吸入,超时500毫秒
        UpdateDeviceStatus();       // 更新当前设备状态
        if (m_stStatus.fwDevice != WFS_PTR_DEVONLINE)   // 设备处于非ONLINE状态
        {
            switch(m_stStatus.fwDevice)
            {
                case WFS_PTR_DEVOFFLINE:
                case WFS_PTR_DEVNODEVICE:
                case WFS_PTR_DEVPOWEROFF:
                {
                    Log(ThisModule, __LINE__, "介质插入等待中设备断线.");
                    return MEDINS_OFFLINE;
                }
                case WFS_PTR_DEVHWERROR:
                case WFS_PTR_DEVUSERERROR:
                {
                    if (m_stStatus.fwMedia != WFS_PTR_MEDIAJAMMED)
                    {
                        Log(ThisModule, __LINE__, "介质插入等待中设备故障.");
                        return MEDINS_HWERR;
                    }
                }
            }
        }

        switch(m_stStatus.fwMedia)    // 检查Media状态
        {
            case WFS_PTR_MEDIANOTPRESENT:   // 入口及打印位置无介质(休眠500毫秒)
            case WFS_PTR_MEDIAUNKNOWN:      // 介质状态未知
            case WFS_PTR_MEDIANOTSUPP:      // 不支持获取介质状态
            case WFS_PTR_MEDIARETRACTED:    // 介质被复位回收
                //usleep(1000 * 500);
                //dwTimeOutCount = dwTimeOutCount + 500;
                //dwTimeOut = (dwTimeOut - 500 < 0 ? 0 : dwTimeOut - 500);
                break;
            case WFS_PTR_MEDIAENTERING:     // 介质在出口,介质吸入
                nRetMediaInsert = m_DSRinter->MeidaControl(MEDIA_CTR_PARTIALCUT, 500);    // 介质吸入,超时500毫秒
                if (nRetMediaInsert != PTR_SUCCESS)
                {                    
                    UpdateDeviceStatus();
                    if (m_stStatus.fwMedia == WFS_PTR_MEDIAJAMMED)
                    {
                        Log(ThisModule, __LINE__, "介质JAM.");
                        bMediaJAM = TRUE;
                        break;
                    } else
                    if (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT)
                    {
                        bMediaExist = TRUE;         // 设置介质就位
                        break;
                    }
                    Log(ThisModule, __LINE__, "介质在出口,介质吸入失败, MeidaControl(%d) RetCode : %d.",
                         MEDIA_CTR_PARTIALCUT, nRetMediaInsert);
                    return MEDINS_HWERR;
                }
                break;
            case WFS_PTR_MEDIAPRESENT:      // 介质就位
                bMediaExist = TRUE;         // 设置介质就位
                break;
            case WFS_PTR_MEDIAJAMMED:       // 介质JAM
                bMediaJAM = TRUE;           // 设置介质JAM
                break;
        }
    }

    // 放入取消
    if (m_bCmdCanceled == TRUE)
    {
        m_bCmdCanceled = FALSE;
        Log(ThisModule, __LINE__, "介质插入等待已取消.");
        return MEDINS_CANCEL;
    }

    // 介质JAM
    if (bMediaJAM == TRUE)
    {
        Log(ThisModule, __LINE__, "介质JAM.");
        return MEDINS_JAM;
    }

    return MEDINS_OK;
}

// 执行自动复位
int CXFS_DSR::RunAutoReset(LONG lErrCode)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;

    for (INT i = 0; i < m_stConfig.nAutoResetErrListCnt; i ++)
    {
        if (m_stConfig.nAutoResetErrList[i] == lErrCode)
        {
            nRet = m_DSRinter->Reset();
            if (nRet != PTR_SUCCESS)
            {
                Log(ThisModule, __LINE__,
                    "错误码[%d], 执行自动复位: ->Reset() Fail, ErrCode = %d, 不做错误处理.",
                   lErrCode, nRet);
            }
            Log(ThisModule, __LINE__,  "错误码[%d], 执行自动复位: ->Reset() Succ.", lErrCode);
        }
    }

    return 0;
}


//-----------------------------------------------------------------------------------
//-----------------------------------接口-------------------------------
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


HRESULT CXFS_DSR::InnerReadImage(LPWFSPTRIMAGEREQUEST lpInData, LPWFSPTRIMAGE *&lppOutData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);

    LPWFSPTRIMAGEREQUEST lpIn = nullptr;
    LPWFSPTRIMAGE lpOut = nullptr;
    READIMAGEIN stScanImageIn;
    READIMAGEOUT stScanImageOut;
    HRESULT hRes = WFS_SUCCESS;

    INT nRet = 0;

    // Check: 入参
    if (lpInData == nullptr)
    {
        Log(ThisModule, __LINE__, "入参无效: Is NULL, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    lpIn = (LPWFSPTRIMAGEREQUEST)lpInData;
    if (lpInData->fwImageSource == 0)
    {
        Log(ThisModule, __LINE__, "入参无效: lpImgRequest->fwImageSource == 0, Return: %d.", WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    INT nImageSource = WFS_PTR_IMAGEFRONT | WFS_PTR_IMAGEBACK | WFS_PTR_CODELINE;
    if ((lpInData->fwImageSource & nImageSource) == 0)
    {
        Log(ThisModule, __LINE__, "入参无效: lpImgRequest->fwImageSource == %d, Return: %d.",
            lpInData->fwImageSource, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    // --------检查入参并组织下发到DevDSR的入参--------
    stScanImageIn.Clear();
    if ((lpIn->fwImageSource & WFS_PTR_IMAGEFRONT) == WFS_PTR_IMAGEFRONT)   // 扫描正面图像
    {
        if (lpIn->lpszFrontImageFile == nullptr || strlen(lpIn->lpszFrontImageFile) < 1)
        {
            Log(ThisModule, __LINE__,
                "入参lpszFrontImageFile支持IMAGEFRONT[扫描正面图像], 未设置图像路径参数. Return: %d.",
                WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        // 正面图像路径check
        if (FileAccess::create_directory_by_path(lpIn->lpszFrontImageFile, true) != true)
        {
            Log(ThisModule, __LINE__,
                "入参lpszFrontImageFile支持IMAGEFRONT[扫描正面图像], 指定图像路径[%s]建立失败. Return: %d.",
               lpIn->lpszFrontImageFile, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        // 正面图像格式
        switch(lpIn->wFrontImageType)
        {
            case WFS_PTR_IMAGETIF:  stScanImageIn.wImageType[0] = IMAGE_TYPE_TIF; break;
            case WFS_PTR_IMAGEWMF:  stScanImageIn.wImageType[0] = IMAGE_TYPE_WMF; break;
            case WFS_PTR_IMAGEBMP:  stScanImageIn.wImageType[0] = IMAGE_TYPE_BMP; break;
            case WFS_PTR_IMAGEJPG:  stScanImageIn.wImageType[0] = IMAGE_TYPE_JPG; break;
            default: stScanImageIn.wImageType[0] = IMAGE_TYPE_BMP; break;
        }

        // 正面图像色彩格式
        switch(lpIn->wFrontImageColorFormat)
        {
            case WFS_PTR_IMAGECOLORBINARY: stScanImageIn.wColorFormat[0] = IMAGE_COLOR_BIN; break;
            case WFS_PTR_IMAGECOLORGRAYSCALE: stScanImageIn.wColorFormat[0] = IMAGE_COLOR_GARY; break;
            case WFS_PTR_IMAGECOLORFULL: stScanImageIn.wColorFormat[0] = IMAGE_COLOR_FULL; break;
            default: stScanImageIn.wColorFormat[0] = IMAGE_COLOR_FULL; break;
        }

        stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_FRONT);
        memcpy(stScanImageIn.szImageFrontPath, lpIn->lpszFrontImageFile, strlen(lpIn->lpszFrontImageFile));
    }

    if ((lpIn->fwImageSource & WFS_PTR_IMAGEBACK) == WFS_PTR_IMAGEBACK) // 扫描背面图像
    {
        if (lpIn->lpszBackImageFile == nullptr || strlen(lpIn->lpszBackImageFile) < 1)
        {
            Log(ThisModule, __LINE__,
                "入参lpszFrontImageFile支持IMAGEBACK[扫描背面图像], 未设置图像保存路径参数. Return: %d.",
                WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        // 背面图像路径check
        if (FileAccess::create_directory_by_path(lpIn->lpszBackImageFile, true) != true)
        {
            Log(ThisModule, __LINE__,
                "入参lpszFrontImageFile支持IMAGEFRONT[扫描背面图像], 指定图像保存路径[%s]建立失败. Return: %d.",
               lpIn->lpszBackImageFile, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        // 背面图像格式
        switch(lpIn->wBackImageType)
        {
            case WFS_PTR_IMAGETIF:  stScanImageIn.wImageType[1] = IMAGE_TYPE_TIF; break;
            case WFS_PTR_IMAGEWMF:  stScanImageIn.wImageType[1] = IMAGE_TYPE_WMF; break;
            case WFS_PTR_IMAGEBMP:  stScanImageIn.wImageType[1] = IMAGE_TYPE_BMP; break;
            case WFS_PTR_IMAGEJPG:  stScanImageIn.wImageType[1] = IMAGE_TYPE_JPG; break;
            default: stScanImageIn.wImageType[1] = IMAGE_TYPE_BMP; break;
        }

        // 背面图像色彩格式
        switch(lpIn->wBackImageColorFormat)
        {
            case WFS_PTR_IMAGECOLORBINARY: stScanImageIn.wColorFormat[1] = IMAGE_COLOR_BIN; break;
            case WFS_PTR_IMAGECOLORGRAYSCALE: stScanImageIn.wColorFormat[1] = IMAGE_COLOR_GARY; break;
            case WFS_PTR_IMAGECOLORFULL: stScanImageIn.wColorFormat[1] = IMAGE_COLOR_FULL; break;
            default: stScanImageIn.wColorFormat[1] = IMAGE_COLOR_FULL; break;
        }

        stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_BACK);
        memcpy(stScanImageIn.szImageBackPath, lpIn->lpszBackImageFile, strlen(lpIn->lpszBackImageFile));
    }

    if ((lpIn->fwImageSource & WFS_PTR_CODELINE) == WFS_PTR_CODELINE) // 代码行
    {
        stScanImageIn.wInMode = (stScanImageIn.wInMode | IMAGE_MODE_CODELINE);

        switch(lpIn->wCodelineFormat)
        {
            case WFS_PTR_CODELINECMC7: stScanImageIn.wCodeLine = IMAGE_CODE_CMC7; break;
            case WFS_PTR_CODELINEE13B: stScanImageIn.wCodeLine = IMAGE_CODE_E13B; break;
            case WFS_PTR_CODELINEOCR: stScanImageIn.wCodeLine = IMAGE_CODE_OCR; break;
            default: stScanImageIn.wCodeLine = IMAGE_CODE_CMC7; break;
        }
    }

    // 等待插折(按不同打印机分支处理)
    nRet = MediaInsertWait(dwTimeOut);
    if (nRet != WFS_SUCCESS)
    {
        return nRet;
    }

    nRet = PTR_SUCCESS;
    stScanImageOut.Clear();
    nRet = m_DSRinter->ReadImage(stScanImageIn, stScanImageOut);
    stScanImageIn.Clear();
    if (nRet != PTR_SUCCESS)
    {
        Log(ThisModule, __LINE__, "图像扫描: ->ScanImage() Fail, ErrCode = %d, Return: %d.",
            nRet, ConvertErrCode(nRet));
        //return ConvertErrCode(nRet);
    }

    // 记录应答Data到Log
    Log(ThisModule, __LINE__, "图像扫描: ->ScanImage() succ, wInMode = %d, wResult = %d, lpData=%s.",
        stScanImageOut.wInMode, stScanImageOut.wResult,
        stScanImageOut.lpData == nullptr ? "(null)" : stScanImageOut.lpData);

    // 组织应答
    lpOut = new WFSPTRIMAGE();
    lpOut->wImageSource = lpIn->fwImageSource;
    lpOut->ulDataLength = 0;
    lpOut->lpbData = nullptr;
    if (stScanImageOut.wResult == READIMAGE_RET_OK)
    {
        lpOut->wStatus = WFS_PTR_DATAOK;
    } else
    if (stScanImageOut.wResult == READIMAGE_RET_MISSING)
    {
        lpOut->wStatus = WFS_PTR_DATASRCMISSING;
    } else
    if (stScanImageOut.wResult == READIMAGE_RET_NOTSUPP)
    {
        lpOut->wStatus = WFS_PTR_DATASRCNOTSUPP;
    }

    if (stScanImageOut.wResult == READIMAGE_RET_OK ||
        stScanImageOut.wResult == READIMAGE_RET_MISSING)
    {
        if (((stScanImageOut.wInMode & IMAGE_MODE_CODELINE) == IMAGE_MODE_CODELINE) &&
            stScanImageOut.lpData != nullptr)
        {
            lpOut->ulDataLength = stScanImageOut.ulDataSize;
            lpOut->lpbData = new BYTE[stScanImageOut.ulDataSize + 1];
            memset(lpOut->lpbData, 0x00, stScanImageOut.ulDataSize + 1);
            memcpy(lpOut->lpbData, stScanImageOut.lpData, stScanImageOut.ulDataSize);
        }
    }

    lppOutData = new LPWFSPTRIMAGE[2];
    lppOutData[0] = lpOut;
    lppOutData[1] = nullptr;

    stScanImageOut.Clear();

    return WFS_SUCCESS;
}

// 设备复位命令子处理
HRESULT CXFS_DSR::InnerReset(DWORD dwMediaControl, USHORT usBinIndex)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = PTR_SUCCESS;
    HRESULT hRet = WFS_SUCCESS;
    BOOL bRunReset = TRUE;

    // CHECK
    // WFS_PTR_CTRLEJECT 退出媒介
    // WFS_PTR_CTRLRETRACT 根据usRetractBinNumber 规定将媒介回收入回收盒
    // WFS_PTR_CTRLEXPEL 从出口抛出媒介
    WORD wCheckCmd = WFS_PTR_CTRLEJECT | WFS_PTR_CTRLRETRACT;

    if (dwMediaControl != 0)    // 参数检查
    {
        if ((dwMediaControl & wCheckCmd) == 0)
        {
            Log(ThisModule, __LINE__, "入参:MediaControl[%d] 无效, Return: %d.",
                dwMediaControl, WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }

        if ((dwMediaControl & WFS_PTR_CTRLEXPEL) == WFS_PTR_CTRLEXPEL)
        {
            Log(ThisModule, __LINE__, "入参:MediaControl[%d] 不支持, Return: %d.", WFS_ERR_UNSUPP_DATA);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    // 执行复位
    if (bRunReset == TRUE)
    {
        nRet = m_DSRinter->Reset();
        if (nRet != PTR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "复位: ->Reset() Fail, ErrCode = %d, Return: %d.",
               nRet, ConvertErrCode(nRet));
            return ConvertErrCode(nRet);
        }
    }
    m_bCmdCanceled = FALSE;
#if 0
    // 复位后参数动作处理
    if (dwMediaControl != 0)
    {
        hRet = InnerMediaControl(dwMediaControl);
        if (hRet != WFS_SUCCESS && hRet != WFS_ERR_PTR_NOMEDIAPRESENT)
        {
            Log(ThisModule, __LINE__, "复位后参数动作处理: ->InnerMediaControl(%d) Fail, Return: %d.",
               dwMediaControl, hRet);
            return hRet;
        }
    }
#endif

    return WFS_SUCCESS;
}

// MediaControl介质控制子处理
HRESULT CXFS_DSR::InnerMediaControl(DWORD dwControl)
{
    THISMODULE(__FUNCTION__);

    INT nRet = PTR_SUCCESS;

    MEDIA_ACTION emMediaAct = MEDIA_CTR_NOTACTION;  // 无动作

    DWORD dwCtrEj_Ret = (WFS_PTR_CTRLEJECT | WFS_PTR_CTRLRETRACT);

    // 无效入参(参数冲突)
    if ((dwControl & dwCtrEj_Ret) == dwCtrEj_Ret)
    {
        Log(ThisModule, __LINE__, "接收ControlMedia参数[%d]无效,入参冲突(不能同时退出和回收), ReturnCode: %d.",
            dwControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)
    {
        // 介质检查(退出/回收)
        UpdateDeviceStatus();       // 更新当前设备状态
        if (m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT)    // 没有介质
        {
            Log(ThisModule, __LINE__, "介质控制: 设备内部没有介质, ReturnCode: %d.", WFS_ERR_PTR_NOMEDIAPRESENT);
            return WFS_ERR_PTR_NOMEDIAPRESENT;
        } else
        if (m_stStatus.fwMedia == WFS_PTR_MEDIAJAMMED)        // 介质JAM
        {
            Log(ThisModule, __LINE__, "介质控制: 介质处于JAM状态, ReturnCode: %d.", WFS_ERR_PTR_MEDIAJAMMED);
            return WFS_ERR_PTR_MEDIAJAMMED;
        }

        if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT &&      // 退出介质+已在出口
             m_stStatus.fwMedia == WFS_PTR_MEDIAENTERING)
        {
            Log(ThisModule, __LINE__, "介质控制: 介质在出口, ReturnCode: %d.", WFS_SUCCESS);
            m_WaitTaken = WTF_TAKEN;
            return WFS_SUCCESS;
        }

        // 下发命令组织入参
        if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)      // 退出介质
        {
            emMediaAct = MEDIA_CTR_EJECT;
        } else
        if((dwControl & WFS_PTR_CTRLRETRACT) == WFS_PTR_CTRLRETRACT)    // 回收介质
        {
            emMediaAct = MEDIA_CTR_RETRACT;
        } else
        if((dwControl & WFS_PTR_CTRLFLUSH) == WFS_PTR_CTRLFLUSH)
        {
            emMediaAct = MEDIA_CTR_FLUSH;
        }

        // 下发命令
        nRet = m_DSRinter->MeidaControl(emMediaAct);
        if (nRet != PTR_SUCCESS)
        {
            Log(ThisModule, __LINE__,
                "介质控制: MeidaControl(%d) Fail, ErrCode: %d, ReturnCode: %d.",
                emMediaAct, nRet, ConvertErrCode(nRet));
            return ConvertErrCode(nRet);
        }
        if ((dwControl & WFS_PTR_CTRLEJECT) == WFS_PTR_CTRLEJECT)      // 退出介质
        {
            m_WaitTaken = WTF_TAKEN;
        }        
    } else
    {
        // 无效入参
        Log(ThisModule, __LINE__, "接收ControlMedia参数[%d]无效, ReturnCode: %d.",
            dwControl, WFS_ERR_UNSUPP_DATA);
        return WFS_ERR_UNSUPP_DATA;
    }

    return WFS_SUCCESS;
}

//-----------------------------------------------------------------------------------
//-----------------------------------重载接口-------------------------------
//
HRESULT CXFS_DSR::OnInit()
{
    THISMODULE(__FUNCTION__);

    /*long nRet = m_DSRinter->Init();
    UpdateDeviceStatus();
    if (nRet < 0)
    {
        return ConvertErrCode(nRet);
    }

    return ConvertErrCode(nRet);*/
    return WFS_SUCCESS;
}

//
HRESULT CXFS_DSR::OnExit()
{
    if (nullptr != m_DSRinter)
    {
        m_DSRinter->Close();
    }

    return WFS_SUCCESS;
}


//-----------------------------------------------------------------------------------
//--------------------------------------功能处理接口-----------------------------------
// 加载DevXXX动态库
bool CXFS_DSR::LoadDevDll(LPCSTR ThisModule)
{
    if (m_DSRinter == nullptr)
    {
        if (m_DSRinter.Load(m_stConfig.szDevDllName, "CreateIDevPTR", DEVTYPE2STR(m_stConfig.nDeviceType)) != 0)
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s, DriverType=%d|%s, ERR:%s",
                m_stConfig.szDevDllName, m_stConfig.nDeviceType,
                DEVTYPE2STR(m_stConfig.nDeviceType), m_DSRinter.LastError().toUtf8().constData());
            return false;
        } else
        {
            Log(ThisModule, __LINE__, "加载库: DriverDllName=%s, DriverType=%d|%s, Succ.",
                m_stConfig.szDevDllName, m_stConfig.nDeviceType, DEVTYPE2STR(m_stConfig.nDeviceType));
        }
    }
    return (m_DSRinter != nullptr);
}

// 加载INI设置
void CXFS_DSR::InitConifig()
{
    THISMODULE(__FUNCTION__);

    CHAR    szIniAppName[MAX_PATH];
    CHAR    szBuffer[MAX_PATH];
    INT     nTmp;

    // DevDSR动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));

    // 设备类型
    m_stConfig.nDeviceType = m_cXfsReg.GetValue("DriverType", (DWORD)DEV_BSD216);

    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.nOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.nOpenFailRet != 0 && m_stConfig.nOpenFailRet != 1)
    {
        m_stConfig.nOpenFailRet = 0;
    }

    memset(szIniAppName, 0x00, sizeof(szIniAppName));
    sprintf(szIniAppName, "DEVICE_SET_%d", m_stConfig.nDeviceType);

    // 设备SDK库路径
    strcpy(m_stConfig.szSDKPath, m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

    //-----------------用于RESET相关定义----------------
    // 需要自动复位的错误码列表[数字,以","分隔,最多64个],缺省空
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue("RESET_CFG", "AutoReset_ErrList", ""));

    m_stConfig.nAutoResetErrListCnt = 0;
    MSET_0(m_stConfig.nAutoResetErrList);

    nTmp = DataConvertor::split_string(szBuffer, ',', nullptr, 0);
    if (nTmp > 0)
    {
        char szErrList[nTmp][CONST_VALUE_260];
        DataConvertor::split_string(szBuffer, ',', szErrList, nTmp);

        for (INT i = 0; i < nTmp; i ++)
        {
            if (atoi(szErrList[i]) != 0)
            {
                m_stConfig.nAutoResetErrList[m_stConfig.nAutoResetErrListCnt ++] = atoi(szErrList[i]);
            }
        }
    }

    //-----------------用于Event相关定义----------------
    // 是否上报WFS_SYSE_HARDWARE_ERROR事件,0不上报,1上报,缺省1
    m_stConfig.nReportHWErrEvent = m_cXfsReg.GetValue("EVENT_CFG", "ReportHWErrEvent", (DWORD)1);
    if (m_stConfig.nReportHWErrEvent != 0 && m_stConfig.nReportHWErrEvent != 1)
    {
        m_stConfig.nReportHWErrEvent = 1;
    }

    //-----------------用于回收相关定义----------------
    // 是否支持回收(0不支持/1支持,缺省0)
    m_stRetractCfg.nRetractSup = m_cXfsReg.GetValue("RETRACT_CFG", "RetractSup", (DWORD)0);
    if (m_stRetractCfg.nRetractSup != 0 && m_stRetractCfg.nRetractSup != 1)
    {
        m_stRetractCfg.nRetractSup = 0;
    }

    // 回收满数目,缺省20
    m_stRetractCfg.dwRetractVol = m_cXfsReg.GetValue("RETRACT_CFG", "RetractVolume", (DWORD)20);

    // 回收将满阀值,缺省15
    m_stRetractCfg.dwFullThreshold = m_cXfsReg.GetValue("RETRACT_CFG", "FullThreshold", (DWORD)15);

    // 回收计数,缺省0
    m_stRetractCfg.dwRetractCnt = m_cXfsReg.GetValue("RETRACT_CFG", "RetractCount", (DWORD)0);

}

// 初始化Cen标准状态类
long CXFS_DSR::InitStatus()
{
    memset(&m_stStatus, 0x00, sizeof(WFSPTRSTATUS));
    m_stStatus.fwDevice      = WFS_PTR_DEVNODEVICE;
    m_stStatus.fwPaper[0]    = WFS_PTR_PAPERUNKNOWN;
    m_stStatus.fwMedia       = WFS_PTR_MEDIAUNKNOWN;
    m_stStatus.fwToner       = WFS_PTR_TONERFULL;
    m_stStatus.fwLamp        = WFS_PTR_LAMPNOTSUPP;
    m_stStatus.fwInk         = WFS_PTR_INKNOTSUPP;

    // 回收设置
    if (m_stRetractCfg.nRetractSup == 0)    // 不支持回收
    {
        m_stStatus.lppRetractBins = nullptr;
    } else
    {
        // 申请回收箱计数buffer
        m_stStatus.lppRetractBins = new LPWFSPTRRETRACTBINS[m_stRetractCfg.wRetBoxCount + 1];
        memset(m_stStatus.lppRetractBins, 0x00, sizeof(LPWFSPTRRETRACTBINS) * (m_stRetractCfg.wRetBoxCount + 1));
        m_stStatus.lppRetractBins[0] = new WFSPTRRETRACTBINS;
        m_stStatus.lppRetractBins[1] = nullptr;
    }

    m_stStatus.Copy(&m_stStatusOLD);
}

// 初始化Cen标准能力值类
long CXFS_DSR::InitCaps()
{
    m_sCaps.fwType = WFS_PTR_TYPESCANNER;          // 设备类型
    m_sCaps.fwReadForm = 0;
    m_sCaps.fwWriteForm = 0;
    m_sCaps.fwControl = WFS_PTR_CTRLEJECT;
    m_sCaps.bMediaTaken = TRUE;
    m_sCaps.fwImageType = WFS_PTR_IMAGETIF | WFS_PTR_IMAGEBMP | WFS_PTR_IMAGEJPG;
    m_sCaps.usRetractBins = 0;              // 回收箱个数
    m_sCaps.lpusMaxRetract = nullptr;       // 每个回收箱可容纳媒介数目

    return WFS_SUCCESS;
}

// DevXXX动态库错误码 转换为 Cen标准错误码
long CXFS_DSR::ConvertErrCode(long lRes)
{
    switch (lRes)
    {
        case PTR_SUCCESS:           return WFS_SUCCESS;
        case ERR_PTR_PARAM_ERR:     return WFS_ERR_UNSUPP_DATA;
        case ERR_PTR_COMM_ERR:      return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_NO_PAPER:      return WFS_ERR_PTR_PAPEROUT;
        case ERR_PTR_JAMMED:        return WFS_ERR_PTR_PAPERJAMMED;
        case ERR_PTR_NOT_OPEN:      return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_HEADER:        return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_CUTTER:        return WFS_ERR_HARDWARE_ERROR;
        case ERR_PTR_TONER:         return WFS_ERR_HARDWARE_ERROR;      // INK或色带故障
        case ERR_PTR_STACKER_FULL:  return WFS_ERR_PTR_STACKERFULL;     // 用户没有取走
        case ERR_PTR_NO_RESUME:     return WFS_ERR_HARDWARE_ERROR;      // 不可恢复的错误
        case ERR_PTR_CAN_RESUME:    return WFS_ERR_HARDWARE_ERROR;      // 可恢复的错误
        case ERR_PTR_FORMAT_ERROR:  return WFS_ERR_UNSUPP_DATA;         // 打印字串格式错误
        case ERR_PTR_CHRONIC:       return WFS_ERR_HARDWARE_ERROR;      // 软件故障
        case ERR_PTR_HWERR:         return WFS_ERR_HARDWARE_ERROR;      // 硬件故障
        case ERR_PTR_IMAGE_ERROR:   return WFS_ERR_HARDWARE_ERROR;      // 图片相关错误
        case ERR_PTR_NO_DEVICE:     return WFS_ERR_HARDWARE_ERROR;      // 指定名的设备不存在
        case ERR_PTR_UNSUP_CMD:     return WFS_ERR_UNSUPP_COMMAND;      // 不支持的指令
        case ERR_PTR_DATA_ERR:      return WFS_ERR_HARDWARE_ERROR;      // 收发数据错误
        case ERR_PTR_TIMEOUT:       return WFS_ERR_TIMEOUT;             // 超时
        case ERR_PTR_DRVHND_ERR:    return WFS_ERR_HARDWARE_ERROR;      // 驱动错误
        case ERR_PTR_DRVHND_REMOVE: return WFS_ERR_HARDWARE_ERROR;      // 驱动丢失
        case ERR_PTR_USB_ERR:       return WFS_ERR_HARDWARE_ERROR;      // USB/COM/连接错误
        case ERR_PTR_DEVBUSY:       return WFS_ERR_HARDWARE_ERROR;      // 设备忙
        case ERR_PTR_OTHER:         return WFS_ERR_HARDWARE_ERROR;      // 其它错误，如调用API错误等
        case ERR_PTR_DEVUNKNOWN:    return WFS_ERR_HARDWARE_ERROR;      // 设备未知
        case ERR_PTR_NOMEDIA:       return WFS_ERR_PTR_NOMEDIAPRESENT;  // 指定位置无介质
        case ERR_PTR_HAVEMEDIA:     return WFS_ERR_HARDWARE_ERROR;      // 指定位置有介质
        case ERR_PTR_PAPER_ERR:     return WFS_ERR_PTR_MEDIAINVALID;    // 介质异常
        case ERR_PTR_JSON_ERR:      return WFS_ERR_HARDWARE_ERROR;      // JSON处理错误
        case ERR_PTR_SCAN_FAIL:     return WFS_ERR_HARDWARE_ERROR;      // 扫描失败
        case ERR_PTR_DATA_DISCERN:  return WFS_ERR_HARDWARE_ERROR;      // 数据识别失败
        case ERR_PTR_NO_MEDIA:      return WFS_ERR_PTR_NOMEDIAPRESENT;  // 通道无纸
        case ERR_PTR_RETRACTFULL:   return WFS_ERR_PTR_RETRACTBINFULL;  // 回收箱满
        case ERR_PTR_MS_BLANK:      return WFS_ERR_PTR_MEDIAINVALID;    // 无效数据 // 空白磁条/磁条无效
        case ERR_PTR_OFFLINE:       return WFS_ERR_HARDWARE_ERROR;      // 设备断线
        default:                    return WFS_ERR_HARDWARE_ERROR;
    }
}

// 设备状态实时更新
void CXFS_DSR::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题

    INT     nRet = PTR_SUCCESS;

    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFirePaperStatus    = FALSE;
    BOOL    bNeedFireTonerStatus    = FALSE;
    BOOL    bNeedFireHWError        = FALSE;
    BOOL    bNeedFirePaperTaken     = FALSE;
    BOOL    bNeedFireMediaInserted  = FALSE;

    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log

    // 特定情形事件上报
    if (m_wNeedFireEvent[0] == 1)
    {
        if (m_stConfig.nReportHWErrEvent == 0)   // INI设定不上报WFS_SYSE_HARDWARE_ERROR事件
        {
            Log(ThisModule, __LINE__, "产生特定情形事件上报: HWEvent:%d,%d,INI设定不上报|",
                    WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
            m_wNeedFireEvent[0] = 0;
        } else
        {
            FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
            sprintf(szFireBuffer + strlen(szFireBuffer), "特定情形事件上报: HWEvent:%d,%d|",
                    WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
            m_wNeedFireEvent[0] = 0;
            m_stStatusOLD.fwDevice = WFS_PTR_DEVHWERROR;
        }
    }

    DEVPTRSTATUS stDevStatus;
    nRet = m_DSRinter->GetStatus(stDevStatus);

    m_stStatus.fwInk                  = WFS_PTR_INKNOTSUPP;
    m_stStatus.fwLamp                 = WFS_PTR_LAMPNOTSUPP;
    m_stStatus.lppRetractBins         = nullptr;
    m_stStatus.usMediaOnStacker       = 0;
    m_stStatus.lpszExtra              = nullptr;


    //WFSPTRSTATUS m_stStatusOLD;
    //memcpy(&m_stStatusOLD, &m_stStatus, sizeof(WFSPTRSTATUS));

    //----------------------设备状态处理----------------------
    switch (nRet)
    {
        // 打印机出纸口有纸设备状态为ONLINE
        case PTR_SUCCESS:
        case ERR_PTR_PARAM_ERR:
        case ERR_PTR_UNSUP_CMD:
            m_stStatus.fwDevice = WFS_PTR_DEVONLINE;
            break;
        case ERR_PTR_COMM_ERR:
        case ERR_PTR_NOT_OPEN:
        case ERR_PTR_NO_DEVICE:
            m_stStatus.fwDevice = WFS_PTR_DEVOFFLINE;
            break;
        default:
            m_stStatus.fwDevice = WFS_PTR_DEVHWERROR;
            break;
    }

    if (m_stStatus.fwDevice == WFS_PTR_DEVOFFLINE)
    {
        m_stStatus.fwMedia = WFS_PTR_MEDIAUNKNOWN;
        m_stStatus.fwPaper[0] = WFS_PTR_PAPERUNKNOWN;
    }

    // Device == ONLINE && 有命令在执行中,设置Device = BUSY
    if (m_stStatus.fwDevice == WFS_PTR_DEVONLINE && m_bCmdRuning == TRUE)    // 命令执行中
    {
        m_stStatus.fwDevice == WFS_PTR_DEVBUSY;
    }

    //----------------------Media状态处理----------------------
    m_stStatus.fwMedia = ConvertMediaStatus(stDevStatus.wMedia);
    if (m_stStatus.fwMedia == WFS_PTR_MEDIAJAMMED)
    {
        m_stStatus.fwDevice = WFS_PTR_DEVHWERROR;
    }

    //----------------------纸状态处理----------------------
    int nPaperStatus = ConvertPaperStatus(stDevStatus.wPaper[0]);

    if (m_stStatus.fwPaper[0] != (WORD)nPaperStatus)
    {
        m_stStatus.fwPaper[0] = (WORD)nPaperStatus;
    }
    for (int i = 1; i < WFS_PTR_SUPPLYSIZE; i++)
    {
        m_stStatus.fwPaper[i] = WFS_PTR_PAPERNOTSUPP;
    }

    //----------------------状态检查----------------------
    // Device状态变化检查
    if (m_stStatus.fwDevice != m_stStatusOLD.fwDevice)
    {
        bNeedFireStatusChanged = TRUE;
        if (m_stStatus.fwDevice == WFS_PTR_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }

    // Media状态变化检查
    if (m_WaitTaken == WTF_TAKEN)   // Taken事件检查
    {
        if (m_stStatus.fwMedia == WFS_PTR_MEDIANOTPRESENT &&
            (m_stStatusOLD.fwMedia == WFS_PTR_MEDIAPRESENT || m_stStatusOLD.fwMedia == WFS_PTR_MEDIAENTERING))
        {
            bNeedFirePaperTaken = TRUE;
        }
    }

    // MediaInsert事件检查
    /*if (m_stStatusOLD.fwMedia == WFS_PTR_MEDIANOTPRESENT &&
        (m_stStatus.fwMedia == WFS_PTR_MEDIAPRESENT || m_stStatus.fwMedia == WFS_PTR_MEDIAENTERING))
    {
        bNeedFireMediaInserted = TRUE;
    }*/

    // 纸状态变化检查,只有当纸状态变为少或空时才Fire状态
    if (m_stStatusOLD.fwPaper[0] !=  m_stStatus.fwPaper[0])
    {
        if (m_stStatus.fwPaper[0] == WFS_PTR_PAPERLOW ||
            m_stStatus.fwPaper[0] == WFS_PTR_PAPEROUT ||
            m_stStatus.fwPaper[0] == WFS_PTR_PAPERFULL)
        {
            bNeedFirePaperStatus = TRUE;
        }
    }


    //----------------------事件上报----------------------
    if (bNeedFireHWError)
    {
        if (m_stConfig.nReportHWErrEvent == 0)   // INI设定不上报WFS_SYSE_HARDWARE_ERROR事件
        {
           Log(ThisModule, __LINE__, "产生HWEvent:%d,%d,INI设置不上报|",
               WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
        } else
        {
            FireHWEvent(WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
            sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%d|",
                    WFS_SYSE_HARDWARE_ERROR, WFS_ERR_ACT_NOACTION);
        }
    }

    if (bNeedFireStatusChanged)
    {
        FireStatusChanged(m_stStatus.fwDevice);
        sprintf(szFireBuffer + strlen(szFireBuffer), "StatusChange:%d|",  m_stStatus.fwDevice);
    }

    if (bNeedFirePaperStatus)
    {
        FirePaperThreshold(WFS_PTR_PAPERUPPER, m_stStatus.fwPaper[0]);
        sprintf(szFireBuffer + strlen(szFireBuffer), "PaperThreshold:%d|",  m_stStatus.fwPaper[0]);
    }

    if (bNeedFireTonerStatus)
    {
        FireTonerThreshold(m_stStatus.fwToner);
        sprintf(szFireBuffer + strlen(szFireBuffer), "TonerThreshold:%d|",  m_stStatus.fwToner);
    }

    if (bNeedFirePaperTaken)
    {
        FireMediaTaken();
        Log(ThisModule, __LINE__, IDS_INFO_PAPER_TAKEN);
        sprintf(szFireBuffer + strlen(szFireBuffer), "PaperTaken|");
        m_WaitTaken = WTF_NONE;
    }

    if (bNeedFireMediaInserted == TRUE && m_bCmdRuning == TRUE)
    {
        FireMediaInserted();
        Log(ThisModule, __LINE__, "介质插入");
        sprintf(szFireBuffer + strlen(szFireBuffer), "MediaInsert|");
    }

    // 回收盒状态变化检查
    if (m_stRetractCfg.dwRetractCnt >= m_stRetractCfg.dwRetractVol)  // 已满
    {
        if (m_stRetractCfg.dwRetractCnt != m_dwRetractCntEvent)
        {
            FireRetractBinThreshold(m_stRetractCfg.wRetBoxCount, WFS_PTR_RETRACTBINFULL);
            m_dwRetractCntEvent = m_stRetractCfg.dwRetractVol;
            m_stRetractCfg.wRetractStat = 2;
            sprintf(szFireBuffer + strlen(szFireBuffer), "RetractBinThreshold(%d, FULL)|",
                    m_dwRetractCntEvent);
        }
    } else
    if (m_stRetractCfg.dwRetractCnt >= m_stRetractCfg.dwFullThreshold)  // 将满
    {
        if (m_stRetractCfg.dwRetractCnt != m_dwRetractCntEvent)
        {
            FireRetractBinThreshold(m_stRetractCfg.wRetBoxCount, WFS_PTR_RETRACTBINHIGH);
            m_dwRetractCntEvent = m_stRetractCfg.dwRetractCnt;
            m_stRetractCfg.wRetractStat = 1;
            sprintf(szFireBuffer + strlen(szFireBuffer), "RetractBinThreshold(%d, HIGH)|",
                    m_dwRetractCntEvent);
        }
    } else  // 正常
    {
        if (m_stRetractCfg.wRetractStat > 0)
        {
            FireRetractBinThreshold(m_stRetractCfg.wRetBoxCount, WFS_PTR_RETRACTBINOK);
            m_dwRetractCntEvent = m_stRetractCfg.dwRetractCnt;
            m_stRetractCfg.wRetractStat = 0;
            sprintf(szFireBuffer + strlen(szFireBuffer), "RetractBinThreshold(%d, OK)|",
                    m_dwRetractCntEvent);
        }
    }

    // 比较两次状态记录LOG
    //if (memcmp(&m_stStatusOLD, &m_stStatus, sizeof(WFSPTRSTATUS)) != 0)
    if (m_stStatus.Diff(m_stStatusOLD) == 1)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|Media:%d->%d%s|Paper[0]:%d->%d%s|"
                            "Ink:%d->%d%s|Toner:%d->%d%s|Lamp:%d->%d%s|; 事件上报记录: %s.",
            m_stStatusOLD.fwDevice, m_stStatus.fwDevice, (m_stStatusOLD.fwDevice != m_stStatus.fwDevice ? " *" : ""),
            m_stStatusOLD.fwMedia, m_stStatus.fwMedia, (m_stStatusOLD.fwMedia != m_stStatus.fwMedia ? " *" : ""),
            m_stStatusOLD.fwPaper[0], m_stStatus.fwPaper[0], (m_stStatusOLD.fwPaper[0] != m_stStatus.fwPaper[0] ? " *" : ""),
            m_stStatusOLD.fwInk, m_stStatus.fwInk, (m_stStatusOLD.fwInk != m_stStatus.fwInk ? " *" : ""),
            m_stStatusOLD.fwToner, m_stStatus.fwToner, (m_stStatusOLD.fwToner != m_stStatus.fwToner ? " *" : ""),
            m_stStatusOLD.fwLamp, m_stStatus.fwLamp, (m_stStatusOLD.fwLamp != m_stStatus.fwLamp ? " *" : ""),
            szFireBuffer);
        m_stStatus.Copy(&m_stStatusOLD);
    }

    return;
}

