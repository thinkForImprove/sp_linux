#include "XFS_FIG.h"
#include "string.h"
#include "file_access.h"
#include "QtTypeInclude.h"
#include <QTextCodec>
#include <stdlib.h>
#include <unistd.h>

// PTR SP 版本号
BYTE    byVRTU[17] = {"HWSFISTE00000100"};
static const char *DEVTYPE = "FIG";

#define LOG_NAME "XFS_FIG.log"
// 事件日志
static const char *ThisFile = "XFX_FIG";

//BOOL g_bExitScanFingerThread = FALSE;

//void *ThreadFingerPressed(void *pDev)
//{
//    THISMODULE(__FUNCTION__);

//    CXFS_FIG *pXFS = static_cast<CXFS_FIG *>(pDev);
//    if (pXFS == nullptr)
//        return nullptr;

//    int nRet = 0;
//    int nIndex = 0;
//    FINGERPRESSEDMODE finger;
//    while(!g_bExitScanFingerThread)
//    {
//        // 0:present 1:not_present
//        nRet = pXFS->GetFingerPointer()->ChkFingerPressed();
//        if (nRet == 0)
//        {
//            //手指按下
//            finger = FINGER_PRESENT;
//            pXFS->SetFingerMode(finger);
//        } else if (nRet == 1 && pXFS->GetFingerMode() == FINGER_PRESENT)
//        {
//            // 上发手指移开事件
//            finger = FINGER_NOTPRESENT;
//            if (pXFS->GetIsFireTakenEvent())
//                pXFS->FireMediaTaken();
//        } else if (nRet == ERR_FIG_NOT_OPEN)
//        {
//            nIndex++;
//            if (nIndex > 5)
//            {
//                nIndex = 0;
//                pXFS->SetOpenAndRetryOpen(FALSE, FALSE);
//                pXFS->UpdateDevStatus(0);
//            }
//        } else if (nRet == 1 || nRet == 0)
//            pXFS->SetOpenAndRetryOpen(TRUE, TRUE);

//        if (pXFS->GetOpen() && !pXFS->GetRetryOpen())
//        {
//            pXFS->SetOpenAndRetryOpen(TRUE, TRUE);
//            pXFS->UpdateDevStatus(0);
//        }

//        std::this_thread::sleep_for(std::chrono::seconds(1));
//    }

//    return (void *)0;
//}

CXFS_FIG::CXFS_FIG()
{
    SetLogFile(LOG_NAME, ThisFile, DEVTYPE);
    m_bOpen = FALSE;                                // 设备打开标志
    ZeroMemory(m_cFilePath, sizeof(m_cFilePath));   // 保存图像路径
    ZeroMemory(m_szDevType, sizeof(m_szDevType));   // 保存设备型号
    m_bReset = FALSE;                               // 设备复位标志
    m_iRawDataLen = FIG_IMAGE_RAW_DATA_SIZE;        // 图像数据大小
    m_iBmpDataLen = FIG_IMAGE_BMP_DATA_SIZE;        // BMP 图像数据大小
    m_bIsPressed = FINGER_UNKNOWN;                  // 指纹是否按下
    m_bIsFireTakenEvent = TRUE;                     // 是否发送手指移走事件标志
    memset(&m_stStatusOld, 0x00, sizeof(WFSPTRSTATUS));
    m_bIsDevBusy = FALSE;                           // 设备是否正在读取指纹
    m_bRetryOpen = FALSE;                           // 是否重连
    m_skLastSuccMode.push(COLLECT_UNKNOWN);
    m_pszFeaData = nullptr;
    m_bisNextMatch = FALSE;                         // 是否时下一个比对
}

CXFS_FIG::~CXFS_FIG()
{
    StopThread();

    while (!m_stFeatureData.empty())
        m_stFeatureData.pop();

    while (!m_skLastSuccMode.empty())
        m_skLastSuccMode.pop();

    if (m_pszFeaData != nullptr)
    {
        delete[] m_pszFeaData;
        m_pszFeaData = nullptr;
    }
}


long CXFS_FIG::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseFIG
    if (0 != m_pBase.Load("SPBaseFIG.dll", "CreateISPBaseFIG", "FIG"))
    {
        Log(ThisModule, __LINE__, "加载SPBasePTR失败");
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return WFS_SUCCESS;
}

HRESULT CXFS_FIG::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = 0;
    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();
    char cCapVersion[5] = {0};

    InitConifig();
    InitStatus();
    InitCaps();

    if ((access(FINGERFEATUREDIRECTORY, F_OK)) == -1)
    {
        mkdir(FINGERFEATUREDIRECTORY, S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    if (!LoadDevDll(ThisModule))
        return WFS_ERR_HARDWARE_ERROR;

    nRet = m_pFinger->Open();
    if (nRet < 0)
    {
        Log(ThisModule, -1, "Open fail , ErrCode :%d", nRet);
        nRet = WFS_ERR_HARDWARE_ERROR;
    }

    // 初始化
    nRet = m_pFinger->Init();
    if (nRet < 0)
    {
        Log(ThisModule, -1, "Init fail , ErrCode :%d", nRet);
        nRet = WFS_ERR_HARDWARE_ERROR;
    }

    // 获取版本号
    nRet = m_pFinger->GetVersion(stFprVersion);
    if (nRet < 0)
    {
        Log(ThisModule, -1, "GetVersion fail , ErrCode:%d", nRet);
        nRet = WFS_ERR_HARDWARE_ERROR;
    }

    // 延时启动
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    // 启动线程
    StartThread();

    // 上报手指移开事件线程
//    m_thFingerPressed = std::thread(&CXFS_FIG::ThreadFingerPressed, this);
//    m_thFingerPressed.detach();

    m_bOpen = TRUE;
    UpdateDevStatus(nRet);

    sprintf(cCapVersion, "%.2f", stFprVersion.dCapVersion);
    //m_cExtra.AddExtra("SPVer", "1.0.1");
    m_cExtra.AddExtra("VRTCount", "3");
    m_cExtra.AddExtra("VRTDetail[00]", (LPSTR)byVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", (LPSTR)cCapVersion);
    m_cExtra.AddExtra("FingerVendor", m_szDevType);
    m_cExtra.AddExtra("FirmwareVersion", (LPSTR)stFprVersion.pszDesc);
    m_cExtra.AddExtra("LastErrorDetail", "");

    return nRet;
}

HRESULT CXFS_FIG::OnClose()
{
    THISMODULE(__FUNCTION__);
    Log(ThisModule, 1, "FIG SP Close ");
    if (m_pFinger != nullptr)
    {
        Log(ThisModule, 1, "m_pFinger->Close()");
        m_pFinger->Close();
    }
    m_bOpen = FALSE;
    return WFS_SUCCESS;
}

// 状态更新
HRESULT CXFS_FIG::OnStatus()
{
    UpdateStatus();

#if 1
    int nRet;
    nRet = m_pFinger->GetVersion(stFprVersion);
    if (nRet != 0)
    {
//        m_sStatus.fwDevice = WFS_PTR_DEVOFFLINE;
        m_pFinger->Close();
        nRet = m_pFinger->Open();
        if(nRet == 0)
        {
            m_bOpen = TRUE;
        }else {
           nRet = ERR_FIG_COMM_ERR;
        }
        UpdateDevStatus(nRet);
    }
#endif
    return WFS_SUCCESS;
}

HRESULT CXFS_FIG::OnWaitTaken()
{
    return WFS_SUCCESS;
}

HRESULT CXFS_FIG::OnCancelAsyncRequest()
{
    m_SemCancel.SetEvent();
    return WFS_SUCCESS;
}

HRESULT CXFS_FIG::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return WFS_SUCCESS;
}

// FIG类型接口
// INFOR
HRESULT CXFS_FIG::GetStatus(LPWFSPTRSTATUS &lpStatus)
{
    m_sStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpStatus = &m_sStatus;

    return WFS_SUCCESS;
}

HRESULT CXFS_FIG::GetCapabilities(LPWFSPTRCAPS &lpCaps)
{
    m_cCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpCaps = &m_cCaps;

    return WFS_SUCCESS;
}

// EXECUTE
HRESULT CXFS_FIG::ReadImage(const LPWFSPTRIMAGEREQUEST lpImgRequest, LPWFSPTRIMAGE *&lppImage, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    Log(ThisModule, 0, "begin ReadImage ...");
    m_pData.Clear();                                                // 数据清零
    int nRet = 0;
    WORD wImageSource = 0;
    int iImgCount = 0;                                              // 应答图像数
    LPWFSPTRIMAGEREQUEST lpReadImage = (LPWFSPTRIMAGEREQUEST)lpImgRequest;
    ULONG dwStart = CQtTime::GetSysTick();                          // 记录初始时间
    BYTE pszBmpData[FIG_IMAGE_BMP_DATA_SIZE];                       // 采集bmp数据
    BYTE pszRawData[FIG_IMAGE_RAW_DATA_SIZE];                       // 采集图形数据
    BYTE pszErrorFeatureData[MAX_FEATURE_SIZE];                     // 错误返回空的特征数据
    BYTE pszCapFeatureData[MAX_FEATURE_SIZE];                       // 采集的指纹图像特征数据
    BYTE pszFPFeatureData[MAX_FEATURE_SIZE];                        // 比对的指纹图像特征数据
    BYTE m_pszModelFeatureData[MAX_FEATURE_SIZE];                   // 指纹模板数据
    std::string ImageFile;                                          // 获取文件名后缀
    std::string BASE64Data;                                         // 编码为BASE64的数据
    wImageSource = lpReadImage->fwImageSource;
    m_bIsDevBusy = FALSE;

    memset(pszBmpData, 0x00, FIG_IMAGE_BMP_DATA_SIZE);
    memset(pszRawData, 0x00, FIG_IMAGE_RAW_DATA_SIZE);
    memset(pszErrorFeatureData, 0x00, MAX_FEATURE_SIZE);
    memset(pszCapFeatureData, 0x00, MAX_FEATURE_SIZE);
    memset(pszFPFeatureData, 0x00, MAX_FEATURE_SIZE);
    memset(m_pszModelFeatureData, 0x00, MAX_FEATURE_SIZE);

    // 根据ImageSource确定返回图像数据数目
    if((wImageSource & WFS_PTR_IMAGEFRONT) == WFS_PTR_IMAGEFRONT) {     // 请求正面图像
        iImgCount++;
    }
    if((wImageSource & WFS_PTR_IMAGEBACK) == WFS_PTR_IMAGEBACK) {		// 请求背面图像
        iImgCount++;
    }
    if((wImageSource & WFS_PTR_CODELINE) == WFS_PTR_CODELINE) {         // 请求代码行
        iImgCount++;
    }

    if (iImgCount == 0)
        return WFS_ERR_HARDWARE_ERROR;

    while (TRUE)
    {
        if (!m_SemCancel.WaitForEvent(10))
        {
           break;
        }
    }

    Log(ThisModule, 0, "m_SemCancel.WaitForEvent break.");
    //获取指纹模版数据
    if ((wImageSource & WFS_PTR_CODELINE) == WFS_PTR_CODELINE)
    {
        m_bIsDevBusy = TRUE;
        UpdateDevStatus(nRet);

        if (!lpReadImage->lpszFrontImageFile)
        {
            Log(ThisModule, __LINE__, "下发保存模板数据路径为空");
            return WFS_ERR_INVALID_DATA;
        }

        int nIndex = 0;
        int *nLen = 0;
        WFSPTRMEDIAPRESENTED Media;
        FINGERPRESSEDMODE LastStatus = FINGER_UNKNOWN;

        while(nIndex < 3)
        {
            if (CQtTime::GetSysTick() - dwStart > dwTimeOut)
            {
                nRet = WFS_ERR_TIMEOUT;
            }

            // 用户取消
            if (m_SemCancel.WaitForEvent(10))
            {
                Log(ThisModule, __LINE__, "检测到取消");
                nRet = WFS_ERR_CANCELED;
            }

            if (nRet < 0)
            {
                m_bIsDevBusy = FALSE;
                UpdateDevStatus(0);
                return nRet;
            }

            if (m_bIsPressed == LastStatus)
            {
                usleep(200);
                continue;
            }
            m_LockStatus.lock();
            LastStatus = m_bIsPressed;
            m_LockStatus.unlock();

            if (m_bIsPressed == FINGER_PRESENT)
            {
                UpdateStatus();
                if (nIndex == 0)
                {
                    Log(ThisModule, 0, "采集模板：第一次按下手指");
                    FireFingerPressed();
                    m_bIsFireTakenEvent = FALSE;
                }

                nRet = m_pFinger->GetFingerTemplateData(pszBmpData, m_pszModelFeatureData, nLen);
                UpdateDevStatus(nRet);
                if (nRet == ERR_FIG_CAN_RESUME)
                {
                    // 指纹采集重试3次
                    for (int i = 0; i < 3; i++)
                    {
                        nRet = m_pFinger->GetFingerTemplateData(pszBmpData, m_pszModelFeatureData, nLen);
                        if (nRet != ERR_FIG_CAN_RESUME)
                            break;
                    }
                }

                m_bIsDevBusy = FALSE;
                if (nRet == ERR_FIG_NOT_OPEN) {
                    Log(ThisModule, 0, "连接断开, ret: ERR_FIG_NOT_OPEN");
                    return WFS_ERR_CONNECTION_LOST;
                } else if (nRet < 0) {
                    Log(ThisModule, 0, "采集模板指纹失败,无效的指纹, ret: %d", nRet);
                    FireNoMedia("无效的指纹");
                    m_bIsDevBusy = FALSE;
                    if (m_stConfig.wReturnOpenVal == 0)
                        return WFS_ERR_INVALID_DATA;
                    else
                        goto APIEND;
                }

                if (nIndex == 2)
                    m_bIsFireTakenEvent = TRUE;

                nIndex++;
                Media.usWadIndex = nIndex;
                Media.usTotalWads = 3;
                FireFingerPresented(Media);
            } else
                usleep(1000 * 500);
        }

//        解码
//        int out = 0;
//        BASE64Data = BASE64Decode((LPCSTR)m_pszModelFeatureData, MAX_FEATURE_SIZE, out);
//        if (out <= MAX_FEATURE_SIZE)
//            memcpy((LPSTR)m_pszModelFeatureData, BASE64Data.c_str(), BASE64Data.length());

        // 保存模板数据
        UpdateStatus();
        if (lpReadImage->lpszFrontImageFile)
        {
            if (m_pszModelFeatureData[0] != 0x00 && m_pszModelFeatureData[1] != 0x00)
            {
                nRet = SaveDataToImage(m_pszModelFeatureData, lpReadImage->lpszFrontImageFile, MAX_FEATURE_SIZE);
                if (nRet < 0)
                    goto END;
            }
        }

        // 压入成功标志
        m_skLastSuccMode.push(COLLECT_CODELINE);
        if (m_pszFeaData != nullptr){
            delete[] m_pszFeaData;
            m_pszFeaData = nullptr;
        }

        m_pszFeaData = new BYTE[MAX_FEATURE_SIZE];
        memcpy(m_pszFeaData, m_pszModelFeatureData, MAX_FEATURE_SIZE);
        m_stFeatureData.push(m_pszFeaData);

        m_bisNextMatch = FALSE;
        if (m_stConfig.wBASE64Mode)
        {
            BASE64Data.clear();
            BASE64Data = BASE64Encode(m_pszModelFeatureData, MAX_FEATURE_SIZE);
            m_pData.Add(wImageSource, WFS_PTR_DATAOK, BASE64Data.length(), (LPBYTE)BASE64Data.c_str());
            Log(ThisModule, __LINE__, "ReadImage返回数据|ret=WFS_SUCCESS|ImageSource:<WFS_PTR_CODELINE>|Status:<WFS_PTR_DATAOK>|Size:<%d>|Data:<%s>",
                BASE64Data.length(), GetData((LPBYTE)BASE64Data.c_str(), BASE64Data.length()));
        } else {
            m_pData.Add(wImageSource, WFS_PTR_DATAOK, MAX_FEATURE_SIZE, m_pszModelFeatureData);
            Log(ThisModule, __LINE__, "ReadImage返回数据|ret=WFS_SUCCESS|ImageSource:<WFS_PTR_CODELINE>|Status:<WFS_PTR_DATAOK>|Size:<512>|Data:<%s>",
                GetData(m_pszModelFeatureData, MAX_FEATURE_SIZE));
        }
        lppImage = (LPWFSPTRIMAGE*)m_pData;
        return WFS_SUCCESS;
    }
    //采集指纹特征
    if((wImageSource & WFS_PTR_IMAGEFRONT) == WFS_PTR_IMAGEFRONT)
    {
        m_bIsDevBusy = TRUE;
        UpdateDevStatus(nRet);

        while(true)
        {
            if (CQtTime::GetSysTick() - dwStart > dwTimeOut)
            {
                nRet = WFS_ERR_TIMEOUT;
            }

            // 用户取消
            if (m_SemCancel.WaitForEvent(10))
            {
                Log(ThisModule, __LINE__, "检测到取消");
                nRet = WFS_ERR_CANCELED;
            }

            if (nRet < 0)
            {
                m_bIsDevBusy = FALSE;
                UpdateDevStatus(0);
                return nRet;
            }

            if (m_bIsPressed == FINGER_PRESENT)
            {
                UpdateStatus();
                FireFingerPressed();
                m_bIsFireTakenEvent = FALSE;
                break;
            } else
                usleep(1000 * 50);
        }

        // 1 枚指纹图像特征提取
        nRet = m_pFinger->FeatureExtract(pszRawData, pszBmpData, pszCapFeatureData, &m_iRawDataLen, dwTimeOut);
        m_bIsDevBusy = FALSE;
        UpdateDevStatus(nRet);
        if (nRet == ERR_FIG_TIMEOUT)
        {
            Log(ThisModule,0,"time out");
            return WFS_ERR_TIMEOUT;
        } else if (nRet < 0)
        {
            Log(ThisModule, 0, "采集指纹特征失败,无效的指纹, ret: %d", nRet);
            FireNoMedia("无效的指纹");
            goto APIEND;
        }

        m_bIsFireTakenEvent = TRUE;
        UpdateStatus();
        // 将特征数据保存到SP中 (默认路径: /usr/local/CFES/DATA/FORM/FIG/FPFeatureData.bmp)
        // 测试lpszFrontImageFile写死路径: /usr/local/CFES/DATA/FORM/FIG/Fp.bmp
//        nRet = SaveDataToImage(pszCapFeatureData, m_stConfig.pszFeatureDataPath, MAX_FEATURE_SIZE);
//        if (nRet < 0)
//            goto END;

        // 区分bmp和jpg格式图片
        if (lpReadImage->lpszFrontImageFile)
        {
            ImageFile = lpReadImage->lpszFrontImageFile;
            ImageFile = getStringLastNChar(ImageFile, 4);

            if ((memcmp(m_szDevType, IDEV_TYPE_TCM, 3) == 0))
            {
                nRet = m_pFinger->SaveFingerImage(lpReadImage->lpszFrontImageFile, (LPCSTR)pszRawData);
                UpdateDevStatus(nRet);
                if (nRet < 0)
                    goto APIEND;
            }
            else if((memcmp(m_szDevType, IDEV_TYPE_WEL401, 2) == 0))
            {
                nRet = m_pFinger->SaveFingerImage(lpReadImage->lpszFrontImageFile, NULL);
                Log(ThisModule,1,"save end");
                UpdateDevStatus(nRet);
                if (nRet < 0)
                    goto APIEND;
            }
            else if (ImageFile.compare(".bmp") == 0)
            {
                // 将数据保存到BMP图像
                if (memcmp(m_szDevType, IDEV_TYPE_SM205, 5) == 0)
                {
                    //nRet = SaveDataToImage(pszBmpData, lpReadImage->lpszFrontImageFile, sizeof(pszBmpData));
                    nRet = SaveDataToImage(pszRawData, lpReadImage->lpszFrontImageFile, sizeof(pszRawData));
                    if (nRet < 0)
                        goto APIEND;

                } else {
                    nRet = WriteBMP(lpReadImage->lpszFrontImageFile, pszRawData, FIG_IMAGE_WIDTH_INTI_SIZE, FIG_IMAGE_HEIGHT_INIT_SIZE);
                    if (nRet < 0)
                    {
                        Log(ThisModule, 0, "WriteBMP fail. filename: %s", lpReadImage->lpszFrontImageFile);
                        goto APIEND;
                    }
                }

                if (!isImgFileExists(lpReadImage->lpszFrontImageFile))
                    goto END;

                Log(ThisModule, __LINE__, "Save BMP to SP, BMP Data Path is %s", lpReadImage->lpszFrontImageFile);
            } else {
                nRet = SaveDataToImage(pszRawData, lpReadImage->lpszFrontImageFile, m_iRawDataLen);
                if (nRet < 0)
                    goto APIEND;

                if (!isImgFileExists(lpReadImage->lpszFrontImageFile))
                    goto END;
            }
        }

        // 压入成功标志
        m_skLastSuccMode.push(COLLECT_FRONT);

        if (m_pszFeaData != nullptr){
            delete[] m_pszFeaData;
            m_pszFeaData = nullptr;
        }

        m_pszFeaData = new BYTE[MAX_FEATURE_SIZE];
        memcpy(m_pszFeaData, pszCapFeatureData, MAX_FEATURE_SIZE);
        m_stFeatureData.push(m_pszFeaData);

        m_bisNextMatch = FALSE;
        if (m_stConfig.wBASE64Mode)
        {
            BASE64Data.clear();
            BASE64Data = BASE64Encode(pszCapFeatureData, sizeof(pszCapFeatureData));
            m_pData.Add(wImageSource, WFS_PTR_DATAOK, BASE64Data.length(), (LPBYTE)BASE64Data.c_str());
            Log(ThisModule, __LINE__, "ReadImage返回数据|ret=WFS_SUCCESS|ImageSource:<WFS_PTR_IMAGEFRONT>|Status:<WFS_PTR_DATAOK>|Size:<%d>|Data:<%s>",
                BASE64Data.length(), GetData((LPBYTE)BASE64Data.c_str(), BASE64Data.length()));
        } else {
            m_pData.Add(wImageSource, WFS_PTR_DATAOK, MAX_FEATURE_SIZE, pszCapFeatureData);
            Log(ThisModule, __LINE__, "ReadImage返回数据|ret=WFS_SUCCESS|ImageSource:<WFS_PTR_IMAGEFRONT>|Status:<WFS_PTR_DATAOK>|Size:<512>|Data:<%s>",
                GetData(pszCapFeatureData, MAX_FEATURE_SIZE));
        }
        lppImage = (LPWFSPTRIMAGE*)m_pData;
        return WFS_SUCCESS;
    }
    //比对指纹特征
    else if((wImageSource & WFS_PTR_IMAGEBACK) == WFS_PTR_IMAGEBACK)
    {
        if (iImgCount < 1)
            goto END;

        UpdateStatus();
        if (lpReadImage->lpszBackImageFile)
        {
            if (!isImgFileExists(lpReadImage->lpszBackImageFile))
            {
                Log(ThisModule, __LINE__, "比对文件不存在，下发路径为:<%s>", lpReadImage->lpszBackImageFile);
            }

            nRet = ReadFeatureData(pszFPFeatureData, lpReadImage->lpszBackImageFile, MAX_FEATURE_SIZE);
            UpdateDevStatus(nRet);
            if (nRet < 0)
                goto APIEND;
        } else
        {
            Log(ThisModule, __LINE__, "lpszBackImageFile is invalid, lpszBackImageFile : %s", lpReadImage->lpszBackImageFile);
            goto APIEND;
        }

        // 比对指纹特征: 下发的路径图像数据 与 模板数据/暂存SP的图像数据对比
        Log(ThisModule, __LINE__, "lpszBackImageFile is %s, 开始比对.", lpReadImage->lpszBackImageFile);
        LASTSUCCESSFINGERMODE last = COLLECT_UNKNOWN;
        if (m_skLastSuccMode.size() > 0 && !m_bisNextMatch)
        {
            last = m_skLastSuccMode.top();
            m_skLastSuccMode.pop();
        } else
            goto APIEND;

        m_bisNextMatch = TRUE;
        if (last == COLLECT_FRONT)
        {
            memset(pszCapFeatureData, 0x00, MAX_FEATURE_SIZE);
            memcpy(pszCapFeatureData, m_stFeatureData.top(), MAX_FEATURE_SIZE);
            m_stFeatureData.pop();
            nRet = m_pFinger->CompareFPData(pszFPFeatureData, pszCapFeatureData);
        }
        else if (last == COLLECT_CODELINE)
        {
            memset(m_pszModelFeatureData, 0x00, MAX_FEATURE_SIZE);
            memcpy(m_pszModelFeatureData, m_stFeatureData.top(), MAX_FEATURE_SIZE);
            m_stFeatureData.pop();
            nRet = m_pFinger->CompareFPData(m_pszModelFeatureData, pszFPFeatureData);
        }
        else
        {
            goto APIEND;
        }
        Log(ThisModule,1,"match end");
        if (nRet < 0)
        {
            Log(ThisModule, 0, "比对指纹特征失败,无效的指纹, ret: %d", nRet);
            FireNoMedia("无效的指纹");
            m_pData.Add(wImageSource, WFS_PTR_DATASRCNOTSUPP, 0, pszErrorFeatureData);
            lppImage = (LPWFSPTRIMAGE*)m_pData;
            Log(ThisModule, __LINE__, "ReadImage返回数据|ret=WFS_SUCCESS|ImageSource:<WFS_PTR_IMAGEBACK>|Status:<WFS_PTR_DATASRCNOTSUPP>|Size:<0>|Data:<%s>",
                GetData(pszErrorFeatureData, 0));
            m_bIsDevBusy = FALSE;
            return WFS_SUCCESS;
        }

        // 清除暂存SP的特征数据
//        if (remove(m_stConfig.pszFeatureDataPath) !=0  || isImgFileExists(m_stConfig.pszFeatureDataPath))
//        {
//            Log(ThisModule, __LINE__, "清除暂存SP的特征数据文件失败，暂存在SP数据路径:%s, 记忆值路径:%s", m_stConfig.pszFeatureDataPath, FINGERFEATUREPATH);
//            goto END;
//        }

        if (m_stConfig.wBASE64Mode)
        {
            Log(ThisModule,1,"before clear");
            BASE64Data.clear();
            Log(ThisModule,1,"before BASE64Encode");
            BASE64Data = BASE64Encode(pszFPFeatureData, sizeof(pszFPFeatureData));
            Log(ThisModule,1,"before add");
            m_pData.Add(wImageSource, WFS_PTR_DATAOK, BASE64Data.length(), (LPBYTE)BASE64Data.c_str());
            Log(ThisModule, __LINE__, "ReadImage返回数据|ret=WFS_SUCCESS|ImageSource:<WFS_PTR_IMAGEBACK>|Status:<WFS_PTR_DATAOK>|Size:<%s>|Data:<%s>",
                BASE64Data.length(), GetData((LPBYTE)BASE64Data.c_str(), BASE64Data.length()));
        } else {
            m_pData.Add(wImageSource, WFS_PTR_DATAOK, MAX_FEATURE_SIZE, pszFPFeatureData);
            Log(ThisModule, __LINE__, "ReadImage返回数据|ret=WFS_SUCCESS|ImageSource:<WFS_PTR_IMAGEBACK>|Status:<WFS_PTR_DATAOK>|Size:<512>|Data:<%s>",
                GetData(pszFPFeatureData, MAX_FEATURE_SIZE));
        }
        m_bIsDevBusy = FALSE;
        lppImage = (LPWFSPTRIMAGE*)m_pData;
        return WFS_SUCCESS;
    }

APIEND:
    if(((wImageSource & WFS_PTR_IMAGEFRONT) == WFS_PTR_IMAGEFRONT) || ((wImageSource & WFS_PTR_CODELINE) == WFS_PTR_CODELINE))
    {
        m_pData.Add(wImageSource, WFS_PTR_DATASRCINVALID, 0, pszErrorFeatureData);
        Log(ThisModule, __LINE__, "ReadImage返回数据|ret=WFS_SU CCESS|ImageSource:<%d>|Status:<WFS_PTR_DATASRCINVALID>|Size:<0>|Data:<%s>",
            wImageSource, GetData(pszErrorFeatureData, 0));
    }
    else if((wImageSource & WFS_PTR_IMAGEBACK) == WFS_PTR_IMAGEBACK)
    {
        m_pData.Add(wImageSource, WFS_PTR_DATASRCMISSING, 0, pszErrorFeatureData);
        Log(ThisModule, __LINE__, "ReadImage返回数据|ret=WFS_SU CCESS|ImageSource:<%d>|Status:<WFS_PTR_DATASRCMISSING>|Size:<0>|Data:<%s>",
            wImageSource, GetData(pszErrorFeatureData, 0));
    }
    lppImage = (LPWFSPTRIMAGE*)m_pData;

    m_bIsDevBusy = FALSE;
    return m_stConfig.wReturnOpenVal == 0 ? WFS_ERR_HARDWARE_ERROR : WFS_SUCCESS;

END:
    m_bIsDevBusy = FALSE;
    return WFS_ERR_HARDWARE_ERROR;
}

HRESULT CXFS_FIG::Reset(const LPWFSPTRRESET lpReset)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = WFS_SUCCESS;
    m_bIsDevBusy = FALSE;       //　设备空闲
    m_pFinger->Close();
    nRet = m_pFinger->Open();
    if(nRet == 0)
    {
        Log(ThisModule,__LINE__,"Reset succeed,nRet = %d",nRet);
        m_bOpen = TRUE;
    }else {
       nRet = ERR_FIG_DEVUNKNOWN;
       Log(ThisModule,__LINE__,"Reset failed,nRet = %d",nRet);
        }
    UpdateDevStatus(nRet);
    return nRet;
}

void CXFS_FIG::Run()
{
    ThreadFingerPressed();
}

// Fire消息
void CXFS_FIG::FireHWEvent(DWORD dwHWAct, char *pErr)
{
    m_pBase->FireHWErrorStatus(dwHWAct, pErr);
}

void CXFS_FIG::FireStatusChanged(WORD wStatus)
{
    m_pBase->FireStatusChanged(wStatus);
}

void CXFS_FIG::FireNoMedia(LPCSTR szPrompt)
{
    //FireEvent(MFT_EE, WFS_EXEE_PTR_NOMEDIA, WFS_SUCCESS, (LPVOID)szPrompt);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_NOMEDIA, (LPVOID)szPrompt);
}
void CXFS_FIG::FireFingerPressed()
{
    //FireEvent(MFT_EE, WFS_EXEE_PTR_MEDIAINSERTED);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAINSERTED, nullptr);
}

void CXFS_FIG::FireFingerPresented(WFSPTRMEDIAPRESENTED MediaPresented)
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_MEDIAPRESENTED, &MediaPresented);
}

void CXFS_FIG::FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName  = (LPSTR)szFormName;
    fail.lpszFieldName = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    //FireEvent(MFT_EE, WFS_EXEE_PTR_FIELDERROR, WFS_SUCCESS, &fail);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDERROR, &fail);
}

void CXFS_FIG::FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure)
{
    WFSPTRFIELDFAIL fail;
    fail.lpszFormName   = (LPSTR)szFormName;
    fail.lpszFieldName  = (LPSTR)szFieldName;
    fail.wFailure    = wFailure;
    //FireEvent(MFT_EE, WFS_EXEE_PTR_FIELDWARNING, WFS_SUCCESS, &fail);
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PTR_FIELDWARNING, &fail);
}

void CXFS_FIG::FireMediaTaken()
{
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIATAKEN);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIATAKEN, nullptr);
}

void CXFS_FIG::FireInkThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_INKTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_INKTHRESHOLD, &wStatus);
}
void CXFS_FIG::FireLampThreshold(WORD wStatus)
{
    //FireEvent(MFT_UE, WFS_USRE_PTR_LAMPTHRESHOLD, WFS_SUCCESS, &wStatus);
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_PTR_LAMPTHRESHOLD, &wStatus);
}

void CXFS_FIG::FireMediaDetected(WORD wPos, USHORT BinNumber)
{
    WFSPTRMEDIADETECTED md;
    md.wPosition            = wPos;
    md.usRetractBinNumber   = BinNumber;
    //FireEvent(MFT_SE, WFS_SRVE_PTR_MEDIADETECTED, WFS_SUCCESS, &md);
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PTR_MEDIADETECTED,  &md);
}

int CXFS_FIG::InitConifig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 设置采集器其他参数, 0: 不设置(默认), 1: 设置
    m_stConfig.wNeedSetUp               = (WORD)m_cXfsReg.GetValue("CONFIG", "NeedSetUp", (DWORD)0);
    // 0: 上下对调(默认), 1: 左右对调
    m_stConfig.usImageSwap              = (WORD)m_cXfsReg.GetValue("CONFIG", "ImageSwap", (DWORD)0);
    // 更新状态时间
    m_stConfig.dwUpdateTime             = (DWORD)m_cXfsReg.GetValue("CONFIG", "UpdateTime", (DWORD)0);
    // 设置采集指纹超时时间
    m_stConfig.dwTotalTimeOut           = (DWORD)m_cXfsReg.GetValue("CONFIG", "TotalTimeOut", (DWORD)0);
    // 暂存SP的特征数据路径
    strcpy(m_stConfig.pszFeatureDataPath, m_cXfsReg.GetValue("CONFIG", "FeatureDataPath", ""));
    // 是否设置返回数据格式为BASE64加密方式
    m_stConfig.wBASE64Mode              = (WORD)m_cXfsReg.GetValue("CONFIG", "BASE64Mode", (DWORD)0);

    // -----[DEVICE_CONFIG]下参数------------
    // 设备类型(0/海鑫HX-F200C,1/天诚盛业TCM042)
    m_stConfig.wDeviceType = (WORD)m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (DWORD)0);

    //非接口故障时, 设置ReadImage命令返回值: 0-正常返回(默认)/1-只返回SUCCESS
    m_stConfig.wReturnOpenVal = (WORD)m_cXfsReg.GetValue("RETVALUE", "ReturnOpenVal", (DWORD)0);

    return 0;
}

// 状态初始化
void CXFS_FIG::InitStatus()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    memset(&m_sStatus, 0x00, sizeof(WFSPTRSTATUS));

    m_sStatus.fwDevice                  = WFS_PTR_DEVNODEVICE;
    m_sStatus.fwMedia                   = WFS_PTR_MEDIANOTPRESENT;
    m_sStatus.fwToner                   = WFS_PTR_TONERNOTSUPP;
    m_sStatus.fwLamp                    = WFS_PTR_LAMPNOTSUPP;
    m_sStatus.fwInk                     = WFS_PTR_INKNOTSUPP;
    m_sStatus.fwLamp                    = WFS_PTR_LAMPNOTSUPP;
}

// 能力值初始化
void CXFS_FIG::InitCaps()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    memset(&m_cCaps, 0x00, sizeof(WFSPTRCAPS));

    m_cCaps.wClass                      = WFS_SERVICE_CLASS_PTR;                                    // 逻辑服务类
    m_cCaps.fwType                      = WFS_PTR_TYPESCANNER;                                      // 设备类型
    m_cCaps.bCompound                   = FALSE;
    m_cCaps.wResolution                 = 0;                                                        // 打印清晰度:中等
    m_cCaps.fwReadForm                  = WFS_PTR_READIMAGE;                                        // 是否支持读介质:具有成像功能 raw: 0
    m_cCaps.fwWriteForm                 = 0;
    m_cCaps.fwExtents                   = 0;                                                        // 是否支持测量介质:不支持
    m_cCaps.fwControl                   = 0;
    m_cCaps.usMaxMediaOnStacker         = 0;
    m_cCaps.bAcceptMedia                = FALSE;
    m_cCaps.bMultiPage                  = FALSE;
    m_cCaps.fwPaperSources              = 0;
    m_cCaps.bMediaTaken                 = FALSE;
    m_cCaps.usRetractBins               = 0;
    m_cCaps.lpusMaxRetract              = nullptr;
    m_cCaps.fwImageType                 = WFS_PTR_IMAGEBMP;
    m_cCaps.fwFrontImageColorFormat     = 0;
    m_cCaps.fwBackImageColorFormat      = 0;
    m_cCaps.fwCodelineFormat            = WFS_PTR_CODELINECMC7;
    m_cCaps.fwImageSource               = WFS_PTR_IMAGEFRONT | WFS_PTR_IMAGEBACK | WFS_PTR_CODELINE;
    m_cCaps.fwCharSupport               = WFS_PTR_ASCII;
    m_cCaps.bDispensePaper              = FALSE;
}

bool CXFS_FIG::LoadDevDll(LPCSTR ThisModule)
{

    if (m_pFinger == nullptr)
    {
        char szDevDllName[256] = { 0 };

        if (m_stConfig.wDeviceType == 0)
            memcpy(m_szDevType, IDEV_TYPE_HX, 2);
        else if (m_stConfig.wDeviceType == 1)
            memcpy(m_szDevType, IDEV_TYPE_TCM, 3);
        else if (m_stConfig.wDeviceType == 2)
            memcpy(m_szDevType, IDEV_TYPE_SM205, 5);
        else if (m_stConfig.wDeviceType == 3)
            memcpy(m_szDevType, IDEV_TYPE_WEL401, 2);
        else
            return false;

        strcpy(szDevDllName, m_cXfsReg.GetValue("DriverDllName", ""));
        Log(ThisModule, __LINE__, "llload: lib=%s", szDevDllName);

        if (0 != m_pFinger.Load(szDevDllName, "CreateIDevFIG", m_szDevType))
        {
            Log(ThisModule, __LINE__, "加载库失败: DriverDllName=%s", szDevDllName);
            return false;
        }
    }
    return (m_pFinger != nullptr);
}

// 状态更新子处理
int CXFS_FIG::UpdateDevStatus(int nRet)
{
    THISMODULE(__FUNCTION__);

    WORD fwDevice = WFS_PTR_DEVONLINE;

    memcpy(&m_stStatusOld, &m_sStatus, sizeof(WFSPTRSTATUS));

    if (!m_bOpen)
    {
        fwDevice = WFS_PTR_DEVHWERROR;
        goto CHK;
    }

    if (m_bReset && fwDevice == WFS_PTR_DEVHWERROR)
    {
        fwDevice = WFS_PTR_DEVONLINE;
        m_bReset = FALSE;
        goto CHK;
    }

    if (m_bIsDevBusy)
    {
        fwDevice = WFS_PTR_DEVBUSY;
        goto CHK;
    }

//    if (nRet < 0 && nRet != ERR_FIG_NOT_OPEN)
//        fwDevice = WFS_PTR_DEVHWERROR;
//    else if (nRet == ERR_FIG_NOT_OPEN || m_bOpen != 1)
//        fwDevice = WFS_PTR_DEVOFFLINE;

    if (nRet < 0)
    {
        if (nRet == ERR_FIG_COMM_ERR)
            fwDevice = WFS_PTR_DEVOFFLINE;
        else if (nRet == ERR_FIG_GETTEM_ERR)
            fwDevice = WFS_PTR_DEVONLINE;
        else if (nRet == ERR_FIG_GETFEATURE_ERR)
            fwDevice = WFS_PTR_DEVONLINE;
        else if (nRet == ERR_FIG_TIMEOUT)
            fwDevice = WFS_PTR_DEVONLINE;
        else if (nRet == ERR_FIG_MATCHFAIL)
            fwDevice = WFS_PTR_DEVONLINE;
        else
            fwDevice = WFS_PTR_DEVHWERROR;
    }
CHK:
    // 判断状态是否有变化
    if (m_stStatusOld.fwDevice != fwDevice)
    {
        if (fwDevice != WFS_PTR_DEVBUSY && m_stStatusOld.fwDevice != WFS_PTR_DEVBUSY)
            m_pBase->FireStatusChanged(fwDevice);
        // 故障时，也要上报故障事件
        if (fwDevice == WFS_PTR_DEVHWERROR)
        {
            m_pBase->FireHWErrorStatus(WFS_ERR_ACT_RESET, m_cExtra.GetErrDetail("ErrorDetail"));
        }
    }
    m_sStatus.fwDevice = fwDevice;
    return WFS_SUCCESS;
}

void CXFS_FIG::UpdateStatus()
{
    WORD fwMedia = WFS_PTR_MEDIAUNKNOWN;
    if (m_bIsPressed == FINGER_PRESENT)
        fwMedia = WFS_PTR_MEDIAPRESENT;
    else if (m_bIsPressed == FINGER_NOTPRESENT)
        fwMedia = WFS_PTR_MEDIANOTPRESENT;
    else if (m_bIsPressed == FINGER_UNKNOWN)
        fwMedia = WFS_PTR_MEDIAUNKNOWN;
    else
        fwMedia = WFS_PTR_MEDIANOTSUPP;

    m_sStatus.fwMedia = fwMedia;
}

int CXFS_FIG::WriteBMP(LPSTR file, LPBYTE Input, int BMP_X, int BMP_Y)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    BYTE ImgTemp[FIG_IMAGE_WIDTH_INTI_SIZE * FIG_IMAGE_HEIGHT_INIT_SIZE] = {0};
    BYTE head[FIG_IMAGE_BMP_HEAD_SIZE]={
        /***************************/
        //file header
        0x42,0x4d,//file type
        //0x36,0x6c,0x01,0x00, //file size***ERR_NONE
        0x0,0x0,0x0,0x00, //file size***
        0x00,0x00, //reserved
        0x00,0x00,//reserved
        0x36,0x4,0x00,0x00,//head byte***
        /***************************/
        //infoheader
        0x28,0x00,0x00,0x00,//struct size

        //0x00,0x01,0x00,0x00,//map width***
        0x00,0x00,0x0,0x00,//map width***
        //0x68,0x01,0x00,0x00,//map height***
        0x00,0x00,0x00,0x00,//map height***

        0x01,0x00,//must be 1
        0x08,0x00,//color count***
        0x00,0x00,0x00,0x00, //compression
        //0x00,0x68,0x01,0x00,//data size***
        0x00,0x00,0x00,0x00,//data size***
        0x00,0x00,0x00,0x00, //dpix
        0x00,0x00,0x00,0x00, //dpiy
        0x00,0x00,0x00,0x00,//color used
        0x00,0x00,0x00,0x00,//color important

    };
    memcpy(ImgTemp , Input , FIG_IMAGE_WIDTH_INTI_SIZE * FIG_IMAGE_HEIGHT_INIT_SIZE);
    FILE *fh;

    if(!(fh  = fopen( file, "wb" )))
        return WFS_ERR_INTERNAL_ERROR;

    int i,j;

    long num;
    num = BMP_X;   head[18] = num & 0xFF;
    num = num>>8;  head[19] = num & 0xFF;
    num = num>>8;  head[20] = num & 0xFF;
    num = num>>8;  head[21] = num & 0xFF;

    num = BMP_Y;   head[22] = num & 0xFF;
    num = num>>8;  head[23] = num & 0xFF;
    num = num>>8;  head[24] = num & 0xFF;
    num = num>>8;  head[25] = num & 0xFF;

    j = 0;
    for (i = 54; i < FIG_IMAGE_BMP_HEAD_SIZE; i = i + 4)
    {
        head[i] = head[i + 1] = head[i + 2] = j;
        head[i+3] = 0;
        j++;
    }

    fwrite(head, sizeof(char), FIG_IMAGE_BMP_HEAD_SIZE, fh);

    if(m_stConfig.usImageSwap == 0)
    {
        //图像上下对调，因为传感器读取图像的(默认)
        char tmp;
        for(j = 0; j < BMP_Y / 2; j++)
        {
            for(i = 0; i < (BMP_X); i++)
            {
                tmp = ImgTemp[j * BMP_X + i];
                ImgTemp[j * BMP_X + i] = ImgTemp[(BMP_Y - j - 1) * BMP_X + i];
                ImgTemp[(BMP_Y - j - 1) * BMP_X + i] = tmp;
            }
        }
    } else
    {
        //图像左右对调，因为传感器读取图像的
        char tmp;
        for(j = 0; j < BMP_Y; j++)
        {
            for(i = 0; i < (BMP_X / 2); i++)
            {
                tmp = Input[j * BMP_X + i];
                Input[j * BMP_X + i] = Input[j * BMP_X + BMP_X - i - 1];
                Input[j * BMP_X + BMP_X - i - 1] = tmp;
            }
        }
    }

    fseek(fh, FIG_IMAGE_BMP_HEAD_SIZE * sizeof(char), SEEK_SET);
    fwrite(ImgTemp, sizeof(char), BMP_X * BMP_Y,fh);

    fclose(fh);
    return WFS_SUCCESS;
}

int CXFS_FIG::SaveDataToImage(LPBYTE pszFeatureData, LPCSTR filename, int nLen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(!FileAccess::create_directory_by_path(filename, true))
    {
       Log(ThisFile, 1,"SaveDataToImage, [%s] mkdir fail", filename);
       return WFS_ERR_INTERNAL_ERROR;
    }

    FILE *fp = fopen(filename, "w+");

    if (fp == 0) {
        Log(ThisModule, 0, "路径不存在, filename: %s", filename);
        return WFS_ERR_INTERNAL_ERROR;
    }

    fwrite(pszFeatureData, nLen, 1, fp);
    fclose(fp);

    return WFS_SUCCESS;
}

int CXFS_FIG::ReadFeatureData(LPBYTE pData, LPCSTR filename, int nLen)
{
    FILE* fp = fopen(filename, "rb");
    if (fp == NULL) {
        return WFS_ERR_HARDWARE_ERROR;
    }
    if (pData == NULL)
    {
        return WFS_ERR_HARDWARE_ERROR;
    }
    fseek(fp, 0, SEEK_SET);
    if (fread(pData, 1, nLen, fp) == 0) {
        return WFS_ERR_HARDWARE_ERROR;
    }

    fclose(fp);

    return WFS_SUCCESS;
}

std::string CXFS_FIG::BASE64Encode(const unsigned char* Data, int DataByte)
{
    // 编码表
    const char EncodeTable[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    // 返回值
    string strEncode;
    unsigned char Tmp[4]={0};
    int LineLength=0;
    for(int i=0;i<(int)(DataByte / 3);i++)
    {
        Tmp[1] = *Data++;
        Tmp[2] = *Data++;
        Tmp[3] = *Data++;
        strEncode+= EncodeTable[Tmp[1] >> 2];
        strEncode+= EncodeTable[((Tmp[1] << 4) | (Tmp[2] >> 4)) & 0x3F];
        strEncode+= EncodeTable[((Tmp[2] << 2) | (Tmp[3] >> 6)) & 0x3F];
        strEncode+= EncodeTable[Tmp[3] & 0x3F];
        if(LineLength+=4,LineLength==76) {strEncode+="\r\n";LineLength=0;}
    }
    // 对剩余数据进行编码
    int Mod=DataByte % 3;
    if(Mod==1)
    {
        Tmp[1] = *Data++;
        strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4)];
        strEncode+= "==";
    }
    else if(Mod==2)
    {
        Tmp[1] = *Data++;
        Tmp[2] = *Data++;
        strEncode+= EncodeTable[(Tmp[1] & 0xFC) >> 2];
        strEncode+= EncodeTable[((Tmp[1] & 0x03) << 4) | ((Tmp[2] & 0xF0) >> 4)];
        strEncode+= EncodeTable[((Tmp[2] & 0x0F) << 2)];
        strEncode+= "=";
    }

    return strEncode;
}

std::string CXFS_FIG::BASE64Decode(const char* Data, int DataByte, int& OutByte)
{
    // 解码表
    const char DecodeTable[] =
    {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,// '+'
        0, 0,// '/'53, 54, 55, 56, 57, 58, 59, 60, 61, // '0'-'9'
        0, 0, 0, 0, 0, 0,
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12,
        14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, // 'A'-'Z'
        0, 0, 0, 0, 0,
        27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38,
        40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, // 'a'-'z'
    };
    // 返回值
    string strDecode;
    int nValue;
    int i= 0;
    while (i < DataByte)
    {
        if (*Data != '\r' && *Data!='\n')
        {
            nValue = DecodeTable[*Data++] << 18;
            nValue += DecodeTable[*Data++] << 12;
            strDecode+=(nValue & 0x00FF0000) >> 16;
            OutByte++;
            if (*Data != '=')
            {
                nValue += DecodeTable[*Data++] << 6;
                strDecode+=(nValue & 0x0000FF00) >> 8;
                OutByte++;
                if (*Data != '=')
                {
                    nValue += DecodeTable[*Data++];
                    strDecode+=nValue & 0x000000FF;
                    OutByte++;
                }
            }
            i += 4;
        }
        else// 回车换行,跳过
        {
            Data++;
            i++;
        }
     }
    return strDecode;
}

BOOL CXFS_FIG::isImgFileExists(LPSTR filename)
{
    // 文件存在
    if ((access(filename, F_OK)) == -1)
        return FALSE;

    // 文件可读
    if ((access(filename, R_OK)) == -1)
        return FALSE;

    // 文件可写
    if ((access(filename, W_OK)) == -1)
        return FALSE;

    return TRUE;
}

int CXFS_FIG::GetFeaFromFile(LPSTR filename, LPBYTE pszBmpData, LPBYTE pFea, int nLen)
{
    int ret;
    //int len = 0;

    ret = ReadFeatureData(pFea, filename, nLen);
    try {
        //ret = m_pFinger->FeatureExtract(nullptr, pszBmpData, pFea, &len, 0);
    }
    catch (exception& e)
    {
        ret = WFS_ERR_HARDWARE_ERROR;
    }
    if (ret != 0)
    {
        ret = WFS_ERR_HARDWARE_ERROR;
    }

    return ret;
}

std::string CXFS_FIG::getStringLastNChar(std::string str, ULONG lastN)
{
    return str.substr(str.size() - lastN);
}

void *CXFS_FIG::ThreadFingerPressed()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int nRet = 0;
    int nIndex = 0;
    while(!m_bQuitRun)
    {
        // 0:present 1:not_present
        nRet = m_pFinger->ChkFingerPressed();
        if (nRet == 0)
        {
            //手指按下
            m_LockStatus.lock();
            m_bIsPressed = FINGER_PRESENT;
            m_LockStatus.unlock();
        } else if (nRet == 1 && m_bIsPressed == FINGER_PRESENT)
        {
            // 上发手指移开事件
            m_LockStatus.lock();
            m_bIsPressed = FINGER_NOTPRESENT;
            m_LockStatus.unlock();
            if (m_bIsFireTakenEvent)
                FireMediaTaken();
        } else if (nRet == ERR_FIG_NOT_OPEN)
        {
            nIndex++;
            if (nIndex > 3)
            {
                nIndex = 0;
                m_bOpen = FALSE;
                m_bRetryOpen = FALSE;
                UpdateDevStatus(0);
            }
        } else if (nRet == 1 || nRet == 0)
        {
            m_bRetryOpen = TRUE;
        }

        if (!m_bOpen && m_bRetryOpen)
        {
            m_bOpen = TRUE;
            UpdateDevStatus(0);
        }

        //std::this_thread::sleep_for(std::chrono::seconds(1));
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    return (void *)0;
}

LPCSTR CXFS_FIG::GetImageSource(WORD ImageSource)
{
    string pImg;

    switch (ImageSource) {
        case WFS_PTR_IMAGEFRONT:
            pImg = "WFS_PTR_IMAGEFRONT";
            break;
        case WFS_PTR_IMAGEBACK:
            pImg = "WFS_PTR_IMAGEBACK";
            break;
        case WFS_PTR_CODELINE:
            pImg = "WFS_PTR_CODELINE";
            break;
    default:
        break;
    }

    return pImg.c_str();
}

LPCSTR CXFS_FIG::GetDataSrcStatus(WORD DataSrcStatus)
{
    string pSrc;

    switch (DataSrcStatus) {
        case WFS_PTR_DATAOK:
            pSrc = "WFS_PTR_DATAOK";
            break;
        case WFS_PTR_DATASRCNOTSUPP:
            pSrc = "WFS_PTR_DATASRCNOTSUPP";
            break;
        case WFS_PTR_DATASRCMISSING:
            pSrc = "WFS_PTR_DATASRCMISSING";
            break;
        case WFS_PTR_DATASRCINVALID:
            pSrc = "WFS_PTR_DATASRCINVALID";
            break;
    default:
        break;
    }

    return pSrc.data();
}

LPCSTR CXFS_FIG::GetDataSize(DWORD Size)
{
    string pSrc;
    sprintf((LPSTR)pSrc.c_str(), "%d", Size);
    return pSrc.data();
}

LPCSTR CXFS_FIG::GetData(LPBYTE pData, DWORD size)
{
    string data;
    char szBuf[32] = {0};
    for (DWORD i = 0; i < size; i++)
    {
        memset(szBuf, 0x00, sizeof(szBuf));
        sprintf(szBuf, "%02.02X ", (CHAR)(pData[i]));
        data.append(szBuf);
    }

    return data.data();
}
