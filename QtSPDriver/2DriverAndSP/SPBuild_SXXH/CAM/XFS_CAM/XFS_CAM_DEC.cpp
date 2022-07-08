#include "XFS_CAM.h"



HRESULT CXFS_CAM::InnerTakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = 0;
    WORD    wImageType;

    if (bDisplyOK != TRUE)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败: 命令序列错误,Display未执行．");
        return WFS_ERR_DEV_NOT_READY;
    }

    LPWFSCAMTAKEPICTEX	lpCmdData = NULL;
    lpCmdData = (LPWFSCAMTAKEPICTEX)&stTakePict;

    memset(szFileNameFromAp, 0x00, sizeof(szFileNameFromAp));

    if (lpCmdData->lpszPictureFile == NULL)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[NULL]无效．");
        return WFS_ERR_INVALID_DATA;
    }

    if (strlen(lpCmdData->lpszPictureFile) > 0)
    {
        sprintf(szFileNameFromAp, "%s", lpCmdData->lpszPictureFile);
    } else
    if (strlen(m_sCamIniConfig.szTakePicDefSavePath) > 0)
    {
        sprintf(szFileNameFromAp, "%s", m_sCamIniConfig.szTakePicDefSavePath);
    } else
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[%s]无效．", szFileNameFromAp);
        return WFS_ERR_INVALID_DATA;
    }

    if(*szFileNameFromAp == 0 || szFileNameFromAp[0] != '/')
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[%s]无效．", szFileNameFromAp);
        return WFS_ERR_INVALID_DATA;
    }

    std::string szFileName;
    std::string szFilePath = szFileNameFromAp;		// FilePath and FileName
    int iIndex = szFilePath.rfind('.');
    if (iIndex == -1)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[%s]无效．", szFileNameFromAp);
        return WFS_ERR_INVALID_DATA;
    } else
    {
        szFileName.clear();
        szFileName = szFilePath.substr(0, iIndex);

        std::string szEx = szFilePath.substr(iIndex, szFilePath.length() - iIndex);
        std::transform(szEx.begin(), szEx.end(), szEx.begin(), ::toupper);
        if (szEx.compare(".BASE64") == 0)
        {
            wImageType = PIC_BASE64;
        } else
        if (szEx.compare(".JPG") == 0)
        {
            wImageType = PIC_JPG;
        } else {
            wImageType = PIC_BMP;
        }
        wImageType = (wImageType | m_sCamIniConfig.wTakePicMakeFormatFlag);
    }

    // 目录验证
    if (bPathCheckAndCreate(szFileNameFromAp, FALSE) != TRUE)
    {
        Log(ThisModule, __LINE__, "摄像(TakePictureEx)失败:入参lpszPictureFile[%s]创建目录结构失败．", szFileNameFromAp);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 超时设置
    DWORD dwTimeOutSize = (m_sCamIniConfig.wTakePicTimeOut == 0 ? dwTimeout : m_sCamIniConfig.wTakePicTimeOut);	// 30-00-00-00(FT#0031)

    // 摄像模式Check
    WORD wCamera = TAKEPIC_PERSON;  // 缺省人脸

    // 指定银行特殊处理
    BOOL bBankSet = FALSE;  // 采用银行特殊完成标记(用于银行处理配置不完整则忽略的情况)
    if (m_sCamIniConfig.wBank == BANK_BCS)  // 长沙银行+云从摄像
    {
        if (m_wDeviceType == CAM_DEV_CLOUDWALK)
        {
            if (strlen(m_sCamIniConfig.stCamMethodBCS.szMT1_PerSonName) > 0)
            {
                std::string szFileNew = szFilePath.substr(0, szFilePath.rfind('/') + 1);
                szFileNew.append(m_sCamIniConfig.stCamMethodBCS.szMT1_PerSonName);
                m_pDev->SetData((char*)szFileNew.c_str(), DATATYPE_PERSON);
                wCamera = TAKEPIC_ROOM; // 全景
                bBankSet = TRUE;
            }
        }
    }

    if (bBankSet != TRUE)   // 银行处理配置不完整则忽略
    {
        switch(lpCmdData->wCamera)
        {
            case WFS_CAM_ROOM:      // XFS全景
                wCamera = TAKEPIC_ROOM;
                break;
            case WFS_CAM_PERSON:    // XFS人脸
                wCamera = TAKEPIC_PERSON;
                break;
        }
    }

    // 命令下发
    m_stStatus.fwDevice = WFS_CAM_DEVBUSY;
    bTakePicExRun = TRUE;
    hRet = m_pDev->TakePictureEx(szFileNameFromAp,
                                /*(LPSTR)(szFileName.c_str()),*///lpCmdData->lpszPictureFile,
                                 lpCmdData->lpszCamData,
                                 wImageType, TRUE, dwTimeOutSize, wCamera);				// 30-00-00-00(FT#0031)
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "摄像命令(TakePictureEx(%s, %s, %d, %d, %d))下发失败．RetCode: %d, ReturnCode: %d.",
                                    szFileNameFromAp,/*(LPSTR)(szFileName.c_str()), */lpCmdData->lpszCamData,
                                    wImageType, TRUE, dwTimeOutSize, hRet, hErrCodeChg(hRet));		// 30-00-00-00(FT#0031)
        //return WFS_ERR_HARDWARE_ERROR;
        bTakePicExRun = FALSE;
        return hErrCodeChg(hRet);
    }
    bTakePicExRun = FALSE;

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::InnerDisplay(const WFSCAMDISP &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    LPWFSCAMDISP lpDisplay = NULL;
    lpDisplay = (LPWFSCAMDISP)&stTakePict;

    if(lpDisplay->wAction != WFS_CAM_CREATE && lpDisplay->wAction != WFS_CAM_DESTROY)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口失败:入参wAction[%d]无效．ReturnCode:%d",
            lpDisplay->wAction, hRet);
        return WFS_ERR_INVALID_DATA;
    }
    Log(ThisModule, __LINE__, "摄像窗口(DisplayEx): X=%d, Y=%d, W=%d, H=%d.",
        lpDisplay->wX, lpDisplay->wY, lpDisplay->wWidth, lpDisplay->wHeight);

    if(lpDisplay->wAction == WFS_CAM_CREATE && (lpDisplay->wHeight == 0 || lpDisplay->wWidth == 0))
    {
        Log(ThisModule, __LINE__, "创建摄像窗口失败: 入参wHeight[%d]/wWidth[%d]无效．ReturnCode:%d",
                    lpDisplay->wHeight, lpDisplay->wWidth, hRet);
        return WFS_ERR_INVALID_DATA;
    }

    // 云从INI设置宽高转换
    if (m_wDeviceType == CAM_DEV_CLOUDWALK) // 云从宽高比例4:3
    {
        InneDisplayChg_CloudWalk(lpDisplay->wWidth, lpDisplay->wHeight, m_sCamIniConfig.usDispWHChange);
    }

    // 创建摄像窗口
    hRet = m_pDev->Display(lpDisplay->hWnd, lpDisplay->wAction, lpDisplay->wX, lpDisplay->wY, lpDisplay->wWidth, lpDisplay->wHeight);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口失败．RetCode: %d, ReturnCode: %d.", hRet, hErrCodeChg(hRet));
        return hErrCodeChg(hRet);
    }

    if (lpDisplay->wAction == WFS_CAM_CREATE)
    {
        if (m_wDeviceType == CAM_DEV_CLOUDWALK)
        {
            CAM_SHOWWIN_INFO stCamInfo;
            stCamInfo.hWnd = lpDisplay->hWnd;
            stCamInfo.wX = lpDisplay->wX;
            stCamInfo.wY = lpDisplay->wY;
            stCamInfo.wWidth = lpDisplay->wWidth;
            stCamInfo.wHeight = lpDisplay->wHeight;
            emit vSignShowWin(stCamInfo);
        }

        bDisplyOK = TRUE;
    } else
    if (lpDisplay->wAction == WFS_CAM_DESTROY)
    {
        if (m_wDeviceType == CAM_DEV_CLOUDWALK)
        {
            emit vSignHideWin();
        }

        bDisplyOK = FALSE;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_CAM::InnerDisplayEx(const WFSCAMDISPEX &stDisplayEx, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    LPWFSCAMDISPEX lpDisplay = NULL;
    lpDisplay = (LPWFSCAMDISPEX)&stDisplayEx;

    if(lpDisplay->wHeight == 0 || lpDisplay->wWidth == 0)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(DisplayEx)失败:入参wHeight[%d]/wWidth[%d]无效．ReturnCode:%d",
                    lpDisplay->wHeight, lpDisplay->wWidth, hRet);
        return WFS_ERR_INVALID_DATA;
    }

    Log(ThisModule, __LINE__, "摄像窗口(DisplayEx): X=%d, Y=%d, W=%d, H=%d.",
        lpDisplay->wX, lpDisplay->wY, lpDisplay->wWidth, lpDisplay->wHeight);

    // 云从INI设置宽高转换
    if (m_wDeviceType == CAM_DEV_CLOUDWALK) // 云从宽高比例4:3
    {
        InneDisplayChg_CloudWalk(lpDisplay->wWidth, lpDisplay->wHeight, m_sCamIniConfig.usDispWHChange);
    }

    // 创建窗口
    hRet = m_pDev->Display(lpDisplay->hWnd, WFS_CAM_CREATE, lpDisplay->wX, lpDisplay->wY, lpDisplay->wWidth, lpDisplay->wHeight);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(DisplayEx)失败．RetCode: %d, ReturnCode: %d, 进入关闭窗口处理.", hRet, hErrCodeChg(hRet));
        //return hErrCodeChg(hRet);
        hRet = hErrCodeChg(hRet);
        goto _DISPLAY_END;
    }

    // 云从自创建窗口 开启
    if (m_wDeviceType == CAM_DEV_CLOUDWALK)
    {
        CAM_SHOWWIN_INFO stCamInfo;
        stCamInfo.hWnd = lpDisplay->hWnd;
        stCamInfo.wX = lpDisplay->wX;
        stCamInfo.wY = lpDisplay->wY;
        stCamInfo.wWidth = lpDisplay->wWidth;
        stCamInfo.wHeight = lpDisplay->wHeight;
        emit vSignShowWin(stCamInfo);
    }
    bDisplyOK = TRUE;

    // 摄像拍照
    WFSCAMTAKEPICTEX stWfsTakePicEx;
    stWfsTakePicEx.wCamera = lpDisplay->wCamera;
    stWfsTakePicEx.lpszCamData = lpDisplay->pszTexData;
    stWfsTakePicEx.lpszPictureFile = lpDisplay->pszPictureFile;

    if ((hRet = InnerTakePictureEx(stWfsTakePicEx, dwTimeout)) != WFS_SUCCESS)
    {
        Log(ThisModule, __LINE__, "摄像拍照: ->InnerTakePictureEx()失败, ReturnCode: %d, 进入关闭窗口处理.", hRet);
        //return hRet;
    }

_DISPLAY_END:
    // 关闭窗口
    HRESULT hRetClose = m_pDev->Display(lpDisplay->hWnd, WFS_CAM_CREATE, lpDisplay->wX, lpDisplay->wY, lpDisplay->wWidth, lpDisplay->wHeight);
    if (hRetClose != 0)
    {
        Log(ThisModule, __LINE__, "关闭摄像窗口(DisplayEx)失败．RetCode: %d, ReturnCode: %d", hRetClose, hErrCodeChg(hRetClose));
        //return hErrCodeChg(hRetClose);
    }

    // 云从自创建窗口 关闭
    if (m_wDeviceType == CAM_DEV_CLOUDWALK)
    {
        emit vSignHideWin();
    }

    bDisplyOK = FALSE;

    return hRet;
}

HRESULT CXFS_CAM::InneDisplayChg_CloudWalk(WORD &wWidth, WORD &wHeight, WORD wMode)
{
    THISMODULE(__FUNCTION__);

    WORD wOLDWidth = wWidth;
    WORD wOLDHeight = wHeight;
    if (wMode == 1)    // 以宽分辨率为基准
    {
        wHeight = (WORD)(wOLDWidth * 1.0 / 4.0 * 3.0);
        Log(ThisModule, __LINE__,
            "Display() 入参宽高转换(云从): DispWHChange == 1|宽分辨率为基准: wWidth[%d], wHeight[%d]->[%d].",
            wWidth, wOLDWidth, wHeight);
    } else
    if (wMode == 2)    // 以高分辨率为基准
    {
        wWidth = (WORD)(wOLDHeight * 1.0 / 3.0 * 4.0);
        Log(ThisModule, __LINE__,
            "Display() 入参宽高转换(云从): DispWHChange == 2|高分辨率为基准: wHeight[%d], wWidth[%d]->[%d].",
            wHeight, wOLDWidth, wWidth);
    } else
    if (wMode == 3)    // Display 固定宽高
    {
        wWidth = m_sCamIniConfig.usDispFixWidth;
        wHeight = m_sCamIniConfig.usDispFixHeight;
        Log(ThisModule, __LINE__,
            "Display() 入参宽高转换(云从): DispWHChange == 3|INI固定宽高,"
            "wWidth[%d]->[%d], wHeight[%d]->[%d].",
            wOLDWidth, wWidth, wOLDHeight, wHeight);
    }

    return WFS_SUCCESS;
}

