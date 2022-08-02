/***************************************************************************
* 文件名称: XFS_CAM_DEC.cpp
* 文件描述: 摄像头模块命令子处理接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2021年4月4日
* 文件版本: 1.0.0.1
***************************************************************************/

#include "XFS_CAM.h"

#include "file_access.h"
#include "data_convertor.h"


//-------------------------------------------------------------------------
// Open设备及初始化相关子处理
// BOOL bReConn: 是否重连
HRESULT CXFS_CAM::InnerOpen(BOOL bReConn/* = FALSE*/)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = CAM_SUCCESS;
    BOOL bIsSuccess = TRUE;     // 用来标记是否有设备没有成功Open

    // ------------------------------------------------------------------------
    // 按ROOM/PERSON/EXITSLOT/EXTRA/HIGHTCAMERA/PANORAMIC摄像模式顺序Open设备
    // 同一个设备只Open一次, 可多个摄像模式共用一个设备
    // ------------------------------------------------------------------------

    for (WORD i = 0; i < LCMODE_CNT; i ++)
    {
        // 设备模式支持启用且设备未Open
        if (MI_Enable(i) == TRUE && AND_IS0(m_wIsDevOpenFlag, LCMODE_TO_CMODE(i)))
        {
            // 检查是否有同一设备已Open,有则共享连接
            if (OpenFrontChkDevIsOpen(i, TRUE) != TRUE)
            {
                // Open前下传初始参数
                OpenFrontSendDevParam(bReConn, i, MI_GetDevType(i));

                // Open设备
                nRet = MI_DevDll(i)->Open(DEVTYPE2STR(MI_GetDevType(i)));
                if (nRet != CAM_SUCCESS)
                {
                    if (m_nRetErrOLD[i + 1] != nRet)
                    {
                        Log(ThisModule, __LINE__, "%s%s Device Open[%s] fail, ErrCode = %d",
                            bReConn == TRUE ? "断线重连" : "",  MI_ModeName(i),
                            DEVTYPE2STR(MI_GetDevType(i)), nRet);
                        m_nRetErrOLD[i + 1] = nRet;
                    }
                    SetErrorDetail(nullptr, i);
                    bIsSuccess = FALSE;
                    continue;
                }
            }

            // 设置设备成功Open标记
            m_wIsDevOpenFlag |= LCMODE_TO_CMODE(i);
            Log(ThisModule, __LINE__, "%s%s Device Open[%s] succ, ErrCode = %d",
                bReConn == TRUE ? "断线重连" : "",  MI_ModeName(i),
                DEVTYPE2STR(MI_GetDevType(i)), nRet);
        }
    }

    if (bIsSuccess != TRUE)
    {
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 组织扩展数据
    UpdateExtra();

    // 更新一次状态()
    UpdateDeviceStatus();

    if (bReConn == TRUE)
    {
        Log(ThisModule, __LINE__, "断线重连成功, Extra=%s.",
            m_cStatExtra.GetExtraInfo().c_str());
    } else
    {
        Log(ThisModule, __LINE__, "打开设备连接成功, Extra=%s.",
            m_cStatExtra.GetExtraInfo().c_str());
    }

    return bIsSuccess == TRUE ? WFS_SUCCESS : WFS_ERR_HARDWARE_ERROR;
}

// 加载DevHCAM动态库
BOOL CXFS_CAM::LoadDevCAMDll(LPCSTR ThisModule)
{
    BOOL bIsLoad = TRUE;

    for (WORD i = 0; i < LCMODE_CNT; i ++)
    {
        if (MI_Enable(i) == TRUE)
        {
            if (MI_DevDll(i) == nullptr)
            {
                if (MI_DevDll(i).Load(m_stConfig.szDevDllName, "CreateIDevCAM",
                                      DEVTYPE2STR(MI_GetDevType(i))) != 0)
                {
                    Log(ThisModule, __LINE__,
                        "加载库失败: DriverDllName=%s, CamMode=%s, DriverType=%d|%s, ERR:%s",
                        m_stConfig.szDevDllName, MI_ModeName(i),
                        MI_GetDevType(i), DEVTYPE2STR(MI_GetDevType(i)),
                        MI_DevDll(i).LastError().toUtf8().constData());
                    SetErrorDetail((LPSTR)EC_CAM_XFS_libCAMLoadFail);
                    bIsLoad = FALSE;
                } else
                {
                    Log(ThisModule, __LINE__,
                        "加载库: DriverDllName=%s, CamMode=%s, DriverType=%d|%s, Succ.",
                        m_stConfig.szDevDllName, MI_ModeName(i),
                        MI_GetDevType(i), DEVTYPE2STR(MI_GetDevType(i)));
                }
            }
        }
    }

    if (bIsLoad == FALSE)
    {
        UnLoadDevCAMDll();
        return FALSE;
    }

    return TRUE;
}

// 卸载DevXXX动态库
BOOL CXFS_CAM::UnLoadDevCAMDll()
{
    for (WORD i = 0; i < LCMODE_CNT; i ++)
    {
        MI_RelDevDll(i);
    }

    return TRUE;
}

// 读INI设置
INT CXFS_CAM::InitConfig()
{
    THISMODULE(__FUNCTION__);

    CHAR    szIniAppName[MAX_PATH];
    CHAR    szBuffer[MAX_PATH];
    INT     nTmp;

    // DevHCAM动态库名
    strcpy(m_stConfig.szDevDllName, m_cXfsReg.GetValue("HCAMDriverDllName", ""));

    // ------------------------[CONFIG]下参数读取------------------------
    // 设备未连接时状态显示(0:NODEVICE, 1:OFFLINE, 缺省0)
    m_stConfig.wDevNotFoundStat = m_cXfsReg.GetValue("CONFIG", "DevNotFoundStat", (DWORD)0);

    // 截取画面帧的分辨率(Width),缺省0
    //m_stConfig.wFrameResoWidth = m_cXfsReg.GetValue("CONFIG", "FrameResoWidth", (DWORD)0);

    // 截取画面帧的分辨率(Height),缺省0
    //m_stConfig.wFrameResoHeight = m_cXfsReg.GetValue("CONFIG", "FrameResoHeight", (DWORD)0);

    // 镜像转换, 0:不转换, 1:左右转换, 2:上下转换, 3:上下左右转换, 缺省0
    m_stConfig.wDisplayFlip = m_cXfsReg.GetValue("CONFIG", "DisplayFlip", (DWORD)0);
    if (m_stConfig.wDisplayFlip < 0 || m_stConfig.wDisplayFlip > 3)
    {
        m_stConfig.wDisplayFlip = 0;
    }
    m_stConfig.stVideoPar.nOtherParam[0] = m_stConfig.wDisplayFlip;

    // Display采用图像帧方式刷新时,取图像帧数据接口错误次数上限,缺省500
    m_stConfig.stInitPar.nParInt[2] =
            m_cXfsReg.GetValue("CONFIG", "DisplayGetVideoMaxErrCnt", (DWORD)500);


    // ------------------------[DEVICE_CFG]下参数读取------------------------
    // 环境摄像设备类型(缺省0)
    m_stConfig.wDeviceRoomType =
            (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "DeviceRoomType", (DWORD)0);
    MI_SetDevType(LCMODE_ROOM, m_stConfig.wDeviceRoomType);

    // 人脸摄像设备类型(缺省0)
    m_stConfig.wDevicePersonType =
            (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "DevicePersonType", (DWORD)0);
    MI_SetDevType(LCMODE_PERSON, m_stConfig.wDevicePersonType);

    // 出口槽摄像设备类型(缺省0)
    m_stConfig.wDeviceExitSlotType =
            (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "DeviceExitSlotType", (DWORD)0);
    MI_SetDevType(LCMODE_EXITSLOT, m_stConfig.wDeviceExitSlotType);

    // 扩展摄像设备类型(缺省0)
    m_stConfig.wDeviceExtraType =
            (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "DeviceExtraType", (DWORD)0);
    MI_SetDevType(LCMODE_EXTRA, m_stConfig.wDeviceExtraType);

    // 高拍摄像设备类型(缺省0)
    m_stConfig.wDeviceHightType =
            (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "DeviceHightType", (DWORD)0);
    MI_SetDevType(LCMODE_HIGHT, m_stConfig.wDeviceHightType);

    // 全景摄像设备类型(缺省0)
    m_stConfig.wDevicePanoraType =
            (WORD)m_cXfsReg.GetValue("DEVICE_CFG", "DevicePanoraType", (DWORD)0);
    MI_SetDevType(LCMODE_PANORA, m_stConfig.wDevicePanoraType);


    //-----------------------云从(YC0C98)设备参数获取---------------------------
    // 检索是否存在指定设备类型
    if (m_stCamModeInfo.SearchIsDeviceType(XFS_YC0C98) == TRUE)
    {
        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "DEVICE_SET_%d", XFS_YC0C98);

        // STDEVICEOPENMODE.nOtherParam[0]: 保存设备类型编号
        m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[0] = XFS_YC0C98;

        // 设备SDK库路径
        strcpy(m_stConfig.szSDKPath[LIDX_YC0C98],
               m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

        // 打开方式,0表示序号打开/1表示vid pid打开,缺省0
        m_stConfig.stDevOpenMode[LIDX_YC0C98].wOpenMode =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "OpenType", (DWORD)0);
        if (m_stConfig.stDevOpenMode[LIDX_YC0C98].wOpenMode == 0) // 序号方式打开
        {
            // 可见光模组枚举序号,缺省1(整型方式存储VID变量保存)
            m_stConfig.stDevOpenMode[LIDX_YC0C98].nHidVid[0] =
                   (WORD)m_cXfsReg.GetValue(szIniAppName, "visCamIdx", 1);
            // 红外光模组枚举序号,缺省1(整型方式存储PID变量保存)
            m_stConfig.stDevOpenMode[LIDX_YC0C98].nHidVid[1] =
                   (WORD)m_cXfsReg.GetValue(szIniAppName, "nisCamIdx", (DWORD)0);
        } else  // VidPid方式打开
        {
            // 可见光模组VidPid,缺省0C45:C812
            strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szHidVid[0],
                   m_cXfsReg.GetValue(szIniAppName, "Vis_Vid", "0C45"));
            strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szHidPid[0],
                   m_cXfsReg.GetValue(szIniAppName, "Vis_Pid", "C812"));
            // 红外光模组VidPid,缺省0C45:C811
            strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szHidVid[1],
                   m_cXfsReg.GetValue(szIniAppName, "Nis_Vid", "0C45"));
            strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szHidPid[1],
                   m_cXfsReg.GetValue(szIniAppName, "Nis_Pid", "C811"));
        }

        //******************************************************************
        // 云从检测参数设置加载 计入 STDEVICEOPENMODE.nOtherParam/szOtherParams
        //     其他参数使用下标从10开始, 0～9作为通用参数保留
        // 模型加载方式 -> nOtherParam[10]; 活体类型 -> nOtherParam[11];
        // 授权类型 -> nOtherParam[12]; 是否使用GPU -> nOtherParam[13];
        // 是否多线程检测 -> nOtherParam[14];
        // 算法矩阵文件 -> szOtherParams[10]; 人脸检测模型 -> szOtherParams[11];
        // 关键点检测模型 -> szOtherParams[12]; 关键点跟踪模型 -> szOtherParams[13];
        // 人脸质量评估模型 -> szOtherParams[14]; 活体模型 -> szOtherParams[15];
        // 人脸检测模型,可以不写 ->szOtherParams[16]
        // 模型加载方式, 0:文件加载 1: 内存加载 ->nOtherParam[10]
        //******************************************************************
        // 模型加载方式, 0:文件加载 1: 内存加载,缺省0
        m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[10] =
                m_cXfsReg.GetValue(szIniAppName, "modelMode", (DWORD)1);
        // 活体类型, 0:不活检 1: 红外活体 2:结构光活体
        m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[11] =
                m_cXfsReg.GetValue(szIniAppName, "livenessMode", (DWORD)1);
        // 授权类型, 1:芯片授权 2: hasp授权 3:临时授权 4:云从相机绑定授权
        m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[12] =
                m_cXfsReg.GetValue(szIniAppName, "licenseType", (DWORD)1);
        // 算法矩阵文件,可以不写，使用默认
        strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[10],
               m_cXfsReg.GetValue(szIniAppName, "configFile", ""));
        // 人脸检测模型,可以不写，使用默认
        strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[11],
               m_cXfsReg.GetValue(szIniAppName, "faceDetectFile", ""));
        // 关键点检测模型,可以不写，使用默认
        strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[12],
               m_cXfsReg.GetValue(szIniAppName, "keyPointDetectFile", ""));
        // 关键点跟踪模型,可以不写，使用默认
        strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[13],
               m_cXfsReg.GetValue(szIniAppName, "keyPointTrackFile", ""));
        // 人脸质量评估模型,可以不写，使用默认
        strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[14],
               m_cXfsReg.GetValue(szIniAppName, "faceQualityFile", ""));
        // 活体模型,可以不写，使用默认
        strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[15],
               m_cXfsReg.GetValue(szIniAppName, "faceLivenessFile", ""));
        // 人脸检测模型,可以不写，使用默认
        strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[16],
               m_cXfsReg.GetValue(szIniAppName, "faceKeyPointTrackFile", ""));
        // 是否使用GPU(1/使用GPU,-1使用CPU)
        m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[13] =
                m_cXfsReg.GetValue(szIniAppName, "GPU", (DWORD)0);
        if (m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[13] == 0)
        {
            m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[13] = -1;
        }
        // 是否多线程检测
        m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[14] =
                m_cXfsReg.GetValue(szIniAppName, "multiThread", 1);

        //******************************************************************
        // 计入 STDEVICEOPENMODE.nOtherParam/szOtherParams
        //     其他参数使用下标从10开始, 0～9作为通用参数保留
        // 是否打开语音提示支持 -> nOtherParam[15];
        // 截取画面帧的分辨率(Width) ->  nOtherParam[1];
        // 截取画面帧的分辨率(Height) ->  nOtherParam[2];
        // 设置SDK版本,缺省0 -> nOtherParam[18];
        // 摄像刷新时间 -> nOtherParam[19];
        // 语音提示配置文件 -> szOtherParams[17];
        //******************************************************************
        // 是否打开语音提示支持(0打开/1关闭,缺省0)
        m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[15] =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "VoiceOpen", (DWORD)0);

        // 语音提示配置文件(VoiceOpen=1时必须指定,否则VoiceOpen配置无效)
        if (m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[15] == 1)
        {
            strcpy(m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[17],
                   m_cXfsReg.GetValue(szIniAppName, "VoiceTipFile", ""));
        }

        // 设置SDK版本,缺省0
        m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[18] =
                m_cXfsReg.GetValue(szIniAppName, "SDKVersion", (DWORD)0);
        if (m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[18] < 0 ||
            m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[18] > 1)
        {
            m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[18] = 0;
        }

        InitConfigDef(szIniAppName, LIDX_YC0C98);
    }

    //-----------------------天诚盛业(TCF261)设备参数获取-------------------------
    if (m_stCamModeInfo.SearchIsDeviceType(XFS_TCF261) == TRUE)
    {
        // STDEVICEOPENMODE.nOtherParam[0]: 保存设备类型编号
        m_stConfig.stDevOpenMode[LIDX_TCF261].nOtherParam[0] = XFS_TCF261;

        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "DEVICE_SET_%d", XFS_TCF261);

        // 设备SDK库路径
        strcpy(m_stConfig.szSDKPath[LIDX_TCF261],
               m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

        // 打开方式(0:指定VidPid, 1:不指定VidPid, 缺省1)
        m_stConfig.stDevOpenMode[LIDX_TCF261].wOpenMode =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "OpenType", (DWORD)1);
        if (m_stConfig.stDevOpenMode[LIDX_TCF261].wOpenMode == 1) // 序号方式打开
        {
            // 可见光模组VidPid,缺省735F:2218
            strcpy(m_stConfig.stDevOpenMode[LIDX_TCF261].szHidVid[0],
                   m_cXfsReg.GetValue(szIniAppName, "Vis_Vid", "735F"));
            strcpy(m_stConfig.stDevOpenMode[LIDX_TCF261].szHidPid[0],
                   m_cXfsReg.GetValue(szIniAppName, "Vis_Pid", "2218"));
            // 红外光模组VidPid,缺省735F:2218
            strcpy(m_stConfig.stDevOpenMode[LIDX_TCF261].szHidVid[1],
                   m_cXfsReg.GetValue(szIniAppName, "Nis_Vid", "735F"));
            strcpy(m_stConfig.stDevOpenMode[LIDX_TCF261].szHidPid[1],
                   m_cXfsReg.GetValue(szIniAppName, "Nis_Pid", "2217"));
        }

        InitConfigDef(szIniAppName, LIDX_TCF261);
    }

    //-----------------------哲林(ZLF1000A3)设备参数获取-------------------------
    if (m_stCamModeInfo.SearchIsDeviceType(XFS_ZLF1000A3) == TRUE)
    {
        // STDEVICEOPENMODE.nOtherParam[0]: 保存设备类型编号
        m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].nOtherParam[0] = XFS_ZLF1000A3;

        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "DEVICE_SET_%d", XFS_ZLF1000A3);

        // 设备SDK库路径
        strcpy(m_stConfig.szSDKPath[LIDX_ZLF1000A3],
               m_cXfsReg.GetValue(szIniAppName, "SDK_Path", ""));

        // 打开方式(0:指定VidPid, 1:不指定VidPid, 缺省1)
        m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].wOpenMode =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "OpenType", (DWORD)1);
        if (m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].wOpenMode == 1) // 序号方式打开
        {
            // VidPid,缺省VidPid,缺省3C4D:A3E8:A3E8
            strcpy(m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].szHidVid[0],
                   m_cXfsReg.GetValue(szIniAppName, "Vid", "3C4D"));
            strcpy(m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].szHidPid[0],
                   m_cXfsReg.GetValue(szIniAppName, "Pid", "A3E8"));
        }

        //******************************************************************
        // 参数设置加载 计入 STDEVICEOPENMODE.nOtherParam
        //     其他参数使用下标从10开始, 0～9作为通用参数保留
        // 设备指定使用模式 -> nOtherParam[10];
        // 图像帧是否绘制切边区域 -> nOtherParam[11];]
        //******************************************************************
        // 设备指定使用模式(0:文档模式, 1:人脸模式, 缺省0)
        m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].nOtherParam[10] =
                m_cXfsReg.GetValue(szIniAppName, "ApplyMode", (DWORD)0);
        // 图像帧是否绘制切边区域(0:不绘制, 1:绘制, 缺省1)
        m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].nOtherParam[11] =
                m_cXfsReg.GetValue(szIniAppName, "DrawCutRect", (DWORD)1);

        InitConfigDef(szIniAppName, LIDX_ZLF1000A3);
    }


    //--------------健德源单目(JDY-5001A-0809)设备参数获取-------------------
    if (m_stCamModeInfo.SearchIsDeviceType(XFS_JDY5001A0809) == TRUE)
    {
        // STDEVICEOPENMODE.nOtherParam[0]: 保存设备类型编号
        m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].nOtherParam[0] = XFS_JDY5001A0809;

        memset(szIniAppName, 0x00, sizeof(szIniAppName));
        sprintf(szIniAppName, "DEVICE_SET_%d", XFS_JDY5001A0809);

        // 打开方式(0:序列方式打开, 1:VidPid打开, 缺省0)
        m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].wOpenMode =
                (WORD)m_cXfsReg.GetValue(szIniAppName, "OpenMode", (DWORD)0);
        // 设备序列,缺省
        strcpy(m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].szDevPath[0],
               m_cXfsReg.GetValue(szIniAppName, "VideoX", "/dev/video0"));
        // VID/PID
        strcpy(m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].szHidVid[0],
               m_cXfsReg.GetValue(szIniAppName, "Vid", ""));
        strcpy(m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].szHidPid[0],
               m_cXfsReg.GetValue(szIniAppName, "Pid", ""));

        InitConfigDef(szIniAppName, LIDX_JDY5001A0809);
    }

    // ---------------[VIDEO_CONFIG]摄像显示参数设置参数读取-------------------
    // 亮度(-255 ~ 255)(RGB相关)
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue("VIDEO_CONFIG", "Video_Bright", "99999999"));
    m_stConfig.stVideoPar.duBright = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                             DataConvertor::str2number(szBuffer);
    // 对比度
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue("VIDEO_CONFIG", "Video_Contrast", "99999999"));
    m_stConfig.stVideoPar.duContrast = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                               DataConvertor::str2number(szBuffer);
    // 饱和度    
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue("VIDEO_CONFIG", "Video_Saturation", "99999999"));
    m_stConfig.stVideoPar.duSaturation = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                               DataConvertor::str2number(szBuffer);
    // 色调
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue("VIDEO_CONFIG", "Video_Hue", "99999999"));
    m_stConfig.stVideoPar.duHue = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                          DataConvertor::str2number(szBuffer);
    // 曝光
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue("VIDEO_CONFIG", "Video_Exposure", "99999999"));
    m_stConfig.stVideoPar.duExposure = strlen(szBuffer) == 0 ? (DOUBLE)99999999.00 :
                                                               DataConvertor::str2number(szBuffer);


    // ------------------------[CAMERA_SHAREMEMORY]下参数读取------------------------
    // 摄像数据共享内存名(32位,缺省CamShowSharedMemData)
    strcpy(m_stConfig.szCamDataSMemName,
           m_cXfsReg.GetValue("CAMERA_SHAREMEMORY", "ShowSharedMemName", SHAREDMEMNAME_CAMDATA));
    //摄像数据共享内存Size(缺省10M)
    MSET_0(szBuffer);
    strcpy(szBuffer, m_cXfsReg.GetValue("CAMERA_SHAREMEMORY", "ShowSharedMemSize", "10485760"));
    m_stConfig.ulCamDataSMemSize = strlen(szBuffer) == 0 ? (ULONG)SHAREDMEMSIZE_CAMDATA :
                                                           DataConvertor::str2number(szBuffer);

    MCPY_NOLEN(m_stConfig.stInitPar.szParStr[0], m_stConfig.szCamDataSMemName);
    m_stConfig.stInitPar.lParLong[0] = m_stConfig.ulCamDataSMemSize;

    //--------------------------[TAKEPIC_CONFIG]命令相关设置参数获取--------------------------
    // TakePictrue/TakePictrueEx超时(单位:毫秒,缺省0)
    m_stConfig.dwTakePicTimeOut = m_cXfsReg.GetValue("TAKEPIC_CONFIG", "TakePicTimeOut", (DWORD)0);
    if (m_stConfig.dwTakePicTimeOut < 0)
    {
        m_stConfig.dwTakePicTimeOut = 0;
    }

    // TakePicture命令缺省保存目录
    strcpy(m_stConfig.szTakePicDefSavePath,
           m_cXfsReg.GetValue("TAKEPIC_CONFIG", "TakePicDefSavePath", ""));
    if (strlen((char*)m_stConfig.szTakePicDefSavePath) < 5 ||
        m_stConfig.szTakePicDefSavePath[0] != '/')
    {
        MSET_0(m_stConfig.szTakePicDefSavePath);
    }


    //--------------------------Open命令相关设置参数获取--------------------------
    // Open失败时返回值(0原样返回/1返回SUCCESS,缺省0)
    m_stConfig.wOpenFailRet = m_cXfsReg.GetValue("OPEN_CONFIG", "OpenFailRet", (DWORD)0);
    if (m_stConfig.wOpenFailRet != 0 && m_stConfig.wOpenFailRet != 1)
    {
        m_stConfig.wOpenFailRet = 0;
    }


    //--------------------------Reset命令相关设置参数获取--------------------------
    // Reset执行时是否支持关闭摄像窗口(0:不支持,1支持,缺省0)
    m_stConfig.wResetCloseDiaplay = m_cXfsReg.GetValue("RESET_CONFIG", "ResetCloseDiaplay", (DWORD)0);
    if (m_stConfig.wResetCloseDiaplay < 0 || m_stConfig.wResetCloseDiaplay > 1)
    {
        m_stConfig.wResetCloseDiaplay = 0;
    }


    //--------------------------事件消息相关设置参数获取--------------------------
    // 是否支持上报 活体图像检测错误事件(0不支持/1支持,缺省0)
    m_stConfig.wLiveErrorSup = m_cXfsReg.GetValue("EVENT_CONFIG", "LiveErrorSup", (DWORD)0);
    if (m_stConfig.wLiveErrorSup > 1)
    {
        m_stConfig.wLiveErrorSup = 0;
    }

    //--------------------------指定银行相关设置参数获取--------------------------
    // 指定银行，缺省0
    m_stConfig.wBank = (WORD)m_cXfsReg.GetValue("BANK", "BankNo", (DWORD)BANK_NOALL);
    m_stConfig.stInitPar.nParInt[1] = m_stConfig.wBank;

    //--------------------------指定错误码相关设置参数获取--------------------------
    // 是否支持Status显示错误码(0:不支持, 1:支持显示当前错误码, 2:支持显示当前和上一次错误码, 缺省2)
    m_stConfig.wErrDetailShowSup = m_cXfsReg.GetValue("ERRDETAIL_CONFIG", "ErrDetailShowSup", (DWORD)2);
    if (m_stConfig.wErrDetailShowSup < 0 || m_stConfig.wErrDetailShowSup > 2)
    {
        m_stConfig.wErrDetailShowSup = 2;
    }

    // 当前错误码Key名, 最多31个字符, 缺省ErrorDetail
    MSET_0(szBuffer);
    MSET_0(m_stConfig.szErrDetailKeyName);
    strcpy(szBuffer, m_cXfsReg.GetValue("ERRDETAIL_CONFIG", "ErrDetailKeyName", "ErrorDetail"));
    if (strlen(szBuffer) == 0)
    {
        sprintf(m_stConfig.szErrDetailKeyName, "ErrorDetail");
    } else
    {
        MCPY_NOLEN(m_stConfig.szErrDetailKeyName, szBuffer);
    }

    // 上一次错误码Key名, 最多31个字符, 缺省LastErrorDetail
    MSET_0(szBuffer);
    MSET_0(m_stConfig.szLastErrDetailKeyName);
    strcpy(szBuffer, m_cXfsReg.GetValue("ERRDETAIL_CONFIG", "LastErrDetailKeyName", "LastErrorDetail"));
    if (strlen(szBuffer) == 0)
    {
        sprintf(m_stConfig.szLastErrDetailKeyName, "ErrorDetail");
    } else
    {
        MCPY_NOLEN(m_stConfig.szLastErrDetailKeyName, szBuffer);
    }

    //--------------------------[SPECIAL_MODEL]相关设置参数获取--------------------------
    // 处理1. 云从摄像，TakePictureEx入参指定生成全景图时，需要同时生成指定名的人脸图
    //       设置生成的人脸图片名,缺省空(该处理无效)
    MSET_0(m_stConfig.stInitPar.szParStr[10]);
    strcpy(m_stConfig.stInitPar.szParStr[10],
           m_cXfsReg.GetValue("SPECIAL_MODEL", "MT1_PerSonName", ""));

    // 处理2. 生成人像红外图,设置红外图后缀,自动添加在TakePic命令入参文件名后,缺省空(该处理无效)
    MSET_0(m_stConfig.stInitPar.szParStr[11]);
    strcpy(m_stConfig.stInitPar.szParStr[11],
           m_cXfsReg.GetValue("SPECIAL_MODEL", "MT2_NirImgSuffix", ""));

    PrintIniData();  // INI配置输出

    return WFS_SUCCESS;
}

// INI配置输出
INT CXFS_CAM::PrintIniData()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
#define PRINT_INI_BUF(STR1, STR2) \
    MSET_0(szBuff); \
    sprintf(szBuff, STR1, STR2); \
    qsIniPrt.append(szBuff);

#define PRINT_INI_BUF2(STR1, STR2, STR3) \
    MSET_0(szBuff); \
    sprintf(szBuff, STR1, STR2, STR3); \
    qsIniPrt.append(szBuff);

    QString qsIniPrt = "";
    CHAR szBuff[256] = { 0x00 };

    // default
    PRINT_INI_BUF("\n\t\t\t\t DevCAM动态库名: default->HCAMDriverDllName = %s", m_stConfig.szDevDllName);

    // CONFIG
    PRINT_INI_BUF("\n\t\t\t\t 设备未连接时状态显示(0:NODEVICE,1:OFFLINE): CONFIG->DevNotFoundStat = %d", m_stConfig.wDevNotFoundStat);
    PRINT_INI_BUF("\n\t\t\t\t 截取画面帧的分辨率(Width): CONFIG->FrameResoWidth = %d", m_stConfig.wFrameResoWidth);
    PRINT_INI_BUF("\n\t\t\t\t 截取画面帧的分辨率(Height): CONFIG->FrameResoHeight = %d", m_stConfig.wFrameResoHeight);
    PRINT_INI_BUF("\n\t\t\t\t 镜像转换(0:不转换,1:左右转换,2:上下转换,3:上下左右转换): CONFIG->DisplayFlip = %d", m_stConfig.wDisplayFlip);
    PRINT_INI_BUF("\n\t\t\t\t isplay采用图像帧方式刷新时,取图像帧数据接口错误次数上限: CONFIG->DisplayGetVideoMaxErrCnt = %d", m_stConfig.stInitPar.nParInt[2]);

    // DEVICE_CFG
    PRINT_INI_BUF("\n\t\t\t\t 环境摄像设备类型(0无效): DEVICE_CFG->DeviceRoomType = %d", m_stConfig.wDeviceRoomType);
    PRINT_INI_BUF("\n\t\t\t\t 人脸摄像设备类型(0无效): DEVICE_CFG->DevicePersonType = %d", m_stConfig.wDevicePersonType);
    PRINT_INI_BUF("\n\t\t\t\t 出口槽摄像设备类型(0无效): DEVICE_CFG->DeviceExitSlotType = %d", m_stConfig.wDeviceExitSlotType);
    PRINT_INI_BUF("\n\t\t\t\t 扩展摄像设备类型(0无效): DEVICE_CFG->DeviceExtraType = %d", m_stConfig.wDeviceExtraType);
    PRINT_INI_BUF("\n\t\t\t\t 高拍摄像设备类型(0无效): DEVICE_CFG->DeviceHightType = %d", m_stConfig.wDeviceHightType);
    PRINT_INI_BUF("\n\t\t\t\t 全景摄像设备类型(0无效): DEVICE_CFG->DevicePanoraType = %d", m_stConfig.wDevicePanoraType);

    // 云从(YC0C98)
    if (m_stCamModeInfo.SearchIsDeviceType(XFS_YC0C98) == TRUE)
    {
        qsIniPrt.append("\n\t\t\t\t ---------------------------------云从设备参数---------------------------------");

        PRINT_INI_BUF2("\n\t\t\t\t 设备SDK库路径: DEVICE_SET_%d->SDK_Path = %s", XFS_YC0C98, m_stConfig.szSDKPath[LIDX_YC0C98]);
        PRINT_INI_BUF2("\n\t\t\t\t 打开方式(0:序号,1:VidPid): DEVICE_SET_%d->OpenType = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].wOpenMode);
        if (m_stConfig.stDevOpenMode[LIDX_YC0C98].wOpenMode == 0) // 序号方式打开
        {
            PRINT_INI_BUF2("\n\t\t\t\t 可见光模组枚举序号: DEVICE_SET_%d->visCamIdx = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nHidVid[0]);
            PRINT_INI_BUF2("\n\t\t\t\t 红外光模组枚举序号: DEVICE_SET_%d->NisCamIdx = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nHidVid[1]);
        } else
        {
            PRINT_INI_BUF2("\n\t\t\t\t 可见光模组Vid: DEVICE_SET_%d->Vis_Vid = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szHidVid[0]);
            PRINT_INI_BUF2("\n\t\t\t\t 可见光模组Pid: DEVICE_SET_%d->Vis_Pid = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szHidPid[0]);
            PRINT_INI_BUF2("\n\t\t\t\t 红外光模组Vid: DEVICE_SET_%d->Nis_Vid = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szHidVid[1]);
            PRINT_INI_BUF2("\n\t\t\t\t 红外光模组Pid: DEVICE_SET_%d->Nis_Pid = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szHidPid[1]);
        }
        PRINT_INI_BUF2("\n\t\t\t\t 模型加载方式(0:文件加载,1:内存加载): DEVICE_SET_%d->modelMode = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[10]);
        PRINT_INI_BUF2("\n\t\t\t\t 活体类型(0:不活检,1:红外活体,2:结构光活体): DEVICE_SET_%d->livenessMode = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[11]);
        PRINT_INI_BUF2("\n\t\t\t\t 授权类型(1:芯片授权,2:hasp授权,3:临时授权,4:云从相机绑定授权): DEVICE_SET_%d->licenseType = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[12]);
        PRINT_INI_BUF2("\n\t\t\t\t 算法矩阵文件: DEVICE_SET_%d->configFile = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[10]);
        PRINT_INI_BUF2("\n\t\t\t\t 人脸检测模型: DEVICE_SET_%d->faceDetectFile = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[11]);
        PRINT_INI_BUF2("\n\t\t\t\t 关键点检测模型: DEVICE_SET_%d->keyPointDetectFile = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[12]);
        PRINT_INI_BUF2("\n\t\t\t\t 关键点跟踪模型: DEVICE_SET_%d->keyPointTrackFile = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[13]);
        PRINT_INI_BUF2("\n\t\t\t\t 人脸质量评估模型: DEVICE_SET_%d->faceQualityFile = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[14]);
        PRINT_INI_BUF2("\n\t\t\t\t 活体模型,可以不写，使用默认: DEVICE_SET_%d->faceLivenessFile = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[15]);
        PRINT_INI_BUF2("\n\t\t\t\t 人脸检测模型: DEVICE_SET_%d->faceKeyPointTrackFile = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[16]);
        PRINT_INI_BUF2("\n\t\t\t\t 是否使用GPU(1/使用GPU,-1使用CPU): DEVICE_SET_%d->GPU = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[13]);
        PRINT_INI_BUF2("\n\t\t\t\t 是否多线程检测: DEVICE_SET_%d->multiThread = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[14]);
        PRINT_INI_BUF2("\n\t\t\t\t 是否打开语音提示支持(0:打开,1:关闭): DEVICE_SET_%d->VoiceOpen = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[15]);
        PRINT_INI_BUF2("\n\t\t\t\t 语音提示配置文件: DEVICE_SET_%d->VoiceTipFile = %s", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].szOtherParams[17]);
        PRINT_INI_BUF2("\n\t\t\t\t 设置SDK版本: DEVICE_SET_%d->SDKVersion = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[18]);
        PRINT_INI_BUF2("\n\t\t\t\t 摄像刷新时间(毫秒): DEVICE_SET_%d->WinRefreshTime = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[19]);
        PRINT_INI_BUF2("\n\t\t\t\t 摄像窗口显示方式(0:SDK控制,1:SP内处理,2:外接程序窗口): DEVICE_SET_%d->ShowWinMode = %d", XFS_YC0C98, m_wDeviceShowWinMode[XFS_YC0C98]);
        PRINT_INI_BUF2("\n\t\t\t\t 摄像窗口外接程序: DEVICE_SET_%d->ShowWinMode = %s", XFS_YC0C98, m_szWinProcessPath[XFS_YC0C98]);
        PRINT_INI_BUF2("\n\t\t\t\t 截取画面帧的分辨率: DEVICE_SET_%d->FrameResoWidth = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[1]);
        PRINT_INI_BUF2("\n\t\t\t\t 截取画面帧的分辨率: DEVICE_SET_%d->FrameResoHeight = %d", XFS_YC0C98, m_stConfig.stDevOpenMode[LIDX_YC0C98].nOtherParam[2]);
    }

    // 天诚盛业(XFS_TCF261)
    if (m_stCamModeInfo.SearchIsDeviceType(XFS_TCF261) == TRUE)
    {
        qsIniPrt.append("\n\t\t\t\t ---------------------------------天诚盛业设备参数---------------------------------");

        PRINT_INI_BUF2("\n\t\t\t\t 设备SDK库路径: DEVICE_SET_%d->SDK_Path = %s", XFS_TCF261, m_stConfig.szSDKPath[LIDX_TCF261]);
        PRINT_INI_BUF2("\n\t\t\t\t 打开方式(0:指定VidPid, 1:不指定VidPid): DEVICE_SET_%d->OpenMode = %d", XFS_TCF261, m_stConfig.stDevOpenMode[LIDX_TCF261].wOpenMode);
        if (m_stConfig.stDevOpenMode[LIDX_TCF261].wOpenMode == 1)
        {
            PRINT_INI_BUF2("\n\t\t\t\t 可见光模组Vid: DEVICE_SET_%d->Vis_Vid = %s", XFS_TCF261, m_stConfig.stDevOpenMode[LIDX_TCF261].szHidVid[0]);
            PRINT_INI_BUF2("\n\t\t\t\t 可见光模组Pid: DEVICE_SET_%d->Vis_Pid = %s", XFS_TCF261, m_stConfig.stDevOpenMode[LIDX_TCF261].szHidPid[0]);
            PRINT_INI_BUF2("\n\t\t\t\t 红外光模组Vid: DEVICE_SET_%d->Nis_Vid = %s", XFS_TCF261, m_stConfig.stDevOpenMode[LIDX_TCF261].szHidVid[1]);
            PRINT_INI_BUF2("\n\t\t\t\t 红外光模组Pid: DEVICE_SET_%d->Nis_Pid = %s", XFS_TCF261, m_stConfig.stDevOpenMode[LIDX_TCF261].szHidPid[1]);
        }
        PRINT_INI_BUF2("\n\t\t\t\t 摄像刷新时间(毫秒): DEVICE_SET_%d->WinRefreshTime = %d", XFS_TCF261, m_stConfig.stDevOpenMode[LIDX_TCF261].nOtherParam[19]);
        PRINT_INI_BUF2("\n\t\t\t\t 摄像窗口显示方式(0:SDK控制,1:SP内处理,2:外接程序窗口): DEVICE_SET_%d->ShowWinMode = %d", XFS_TCF261, m_wDeviceShowWinMode[LIDX_TCF261]);
        PRINT_INI_BUF2("\n\t\t\t\t 摄像窗口外接程序: DEVICE_SET_%d->ShowWinMode = %s", XFS_TCF261, m_szWinProcessPath[LIDX_TCF261]);
        PRINT_INI_BUF2("\n\t\t\t\t 截取画面帧的分辨率: DEVICE_SET_%d->FrameResoWidth = %d", XFS_TCF261, m_stConfig.stDevOpenMode[LIDX_TCF261].nOtherParam[1]);
        PRINT_INI_BUF2("\n\t\t\t\t 截取画面帧的分辨率: DEVICE_SET_%d->FrameResoHeight = %d", XFS_TCF261, m_stConfig.stDevOpenMode[LIDX_TCF261].nOtherParam[2]);
    }

    // 哲林(ZLF1000A3)
    if (m_stCamModeInfo.SearchIsDeviceType(XFS_ZLF1000A3) == TRUE)
    {
        qsIniPrt.append("\n\t\t\t\t ---------------------------------哲林设备参数---------------------------------");

        PRINT_INI_BUF2("\n\t\t\t\t 设备SDK库路径: DEVICE_SET_%d->SDK_Path = %s", XFS_ZLF1000A3, m_stConfig.szSDKPath[LIDX_ZLF1000A3]);
        PRINT_INI_BUF2("\n\t\t\t\t 打开方式(0:指定VidPid, 1:不指定VidPid): DEVICE_SET_%d->OpenMode = %d", XFS_ZLF1000A3, m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].wOpenMode);
        if (m_stConfig.stDevOpenMode[XFS_ZLF1000A3].wOpenMode == 1)
        {
            PRINT_INI_BUF2("\n\t\t\t\t Vid: DEVICE_SET_%d->Vid = %s", XFS_ZLF1000A3, m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].szHidVid[0]);
            PRINT_INI_BUF2("\n\t\t\t\t Pid: DEVICE_SET_%d->Pid = %s", XFS_ZLF1000A3, m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].szHidPid[0]);
        }
        PRINT_INI_BUF2("\n\t\t\t\t 摄像刷新时间(毫秒): DEVICE_SET_%d->WinRefreshTime = %d", XFS_ZLF1000A3, m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].nOtherParam[19]);
        PRINT_INI_BUF2("\n\t\t\t\t 摄像窗口显示方式(0:SDK控制,1:SP内处理,2:外接程序窗口): DEVICE_SET_%d->ShowWinMode = %d", XFS_ZLF1000A3, m_wDeviceShowWinMode[LIDX_ZLF1000A3]);
        PRINT_INI_BUF2("\n\t\t\t\t 摄像窗口外接程序: DEVICE_SET_%d->ShowWinMode = %s", XFS_ZLF1000A3, m_szWinProcessPath[LIDX_ZLF1000A3]);
        PRINT_INI_BUF2("\n\t\t\t\t 截取画面帧的分辨率: DEVICE_SET_%d->FrameResoWidth = %d", XFS_ZLF1000A3, m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].nOtherParam[1]);
        PRINT_INI_BUF2("\n\t\t\t\t 截取画面帧的分辨率: DEVICE_SET_%d->FrameResoHeight = %d", XFS_ZLF1000A3, m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].nOtherParam[2]);
        PRINT_INI_BUF2("\n\t\t\t\t 设备指定使用模式(0:文档模式,1:人脸模式): DEVICE_SET_%d->ApplyMode = %d", XFS_ZLF1000A3, m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].nOtherParam[10]);
        PRINT_INI_BUF2("\n\t\t\t\t 图像帧是否绘制切边区域(0:不绘制,1:绘制): DEVICE_SET_%d->DrawCutRect = %d", XFS_ZLF1000A3, m_stConfig.stDevOpenMode[LIDX_ZLF1000A3].nOtherParam[11]);
    }


    // 建德源(XFS_JDY5001A0809)
    if (m_stCamModeInfo.SearchIsDeviceType(XFS_JDY5001A0809) == TRUE)
    {
        qsIniPrt.append("\n\t\t\t\t ---------------------------------建德源设备参数---------------------------------");

        PRINT_INI_BUF2("\n\t\t\t\t 设备SDK库路径: DEVICE_SET_%d->SDK_Path = %s", XFS_JDY5001A0809, m_stConfig.szSDKPath[LIDX_JDY5001A0809]);
        PRINT_INI_BUF2("\n\t\t\t\t 打开方式(0:设备序列,1:VidPid): DEVICE_SET_%d->OpenMode = %d", XFS_JDY5001A0809, m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].wOpenMode);
        if (m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].wOpenMode == 0) // 序号方式打开
        {
            PRINT_INI_BUF2("\n\t\t\t\t 设备序列: DEVICE_SET_%d->VideoX = %s", XFS_JDY5001A0809, m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].szDevPath[0]);
        } else
        {
            PRINT_INI_BUF2("\n\t\t\t\t Vid: DEVICE_SET_%d->Vid = %s", XFS_JDY5001A0809, m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].szHidVid[0]);
            PRINT_INI_BUF2("\n\t\t\t\t Pid: DEVICE_SET_%d->Pid = %s", XFS_JDY5001A0809, m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].szHidPid[0]);
        }
        PRINT_INI_BUF2("\n\t\t\t\t 摄像刷新时间(毫秒): DEVICE_SET_%d->WinRefreshTime = %d", XFS_JDY5001A0809, m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].nOtherParam[19]);
        PRINT_INI_BUF2("\n\t\t\t\t 摄像窗口显示方式(0:SDK控制,1:SP内处理,2:外接程序窗口): DEVICE_SET_%d->ShowWinMode = %d", XFS_JDY5001A0809, m_wDeviceShowWinMode[LIDX_JDY5001A0809]);
        PRINT_INI_BUF2("\n\t\t\t\t 摄像窗口外接程序: DEVICE_SET_%d->ShowWinMode = %s", XFS_JDY5001A0809, m_szWinProcessPath[LIDX_JDY5001A0809]);
        PRINT_INI_BUF2("\n\t\t\t\t 截取画面帧的分辨率: DEVICE_SET_%d->FrameResoWidth = %d", XFS_JDY5001A0809, m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].nOtherParam[1]);
        PRINT_INI_BUF2("\n\t\t\t\t 截取画面帧的分辨率: DEVICE_SET_%d->FrameResoHeight = %d", XFS_JDY5001A0809, m_stConfig.stDevOpenMode[LIDX_JDY5001A0809].nOtherParam[2]);
    }


    // VIDEO_CONFIG
    PRINT_INI_BUF("\n\t\t\t\t 亮度: VIDEO_CONFIG->Video_Bright = %f", m_stConfig.stVideoPar.duBright);
    PRINT_INI_BUF("\n\t\t\t\t 对比度: VIDEO_CONFIG->Video_Contrast = %f", m_stConfig.stVideoPar.duContrast);
    PRINT_INI_BUF("\n\t\t\t\t 饱和度: VIDEO_CONFIG->Video_Saturation = %f", m_stConfig.stVideoPar.duSaturation);
    PRINT_INI_BUF("\n\t\t\t\t 色调: VIDEO_CONFIG->Video_Hue = %f", m_stConfig.stVideoPar.duHue);
    PRINT_INI_BUF("\n\t\t\t\t 曝光: VIDEO_CONFIG->Video_Exposure = %f", m_stConfig.stVideoPar.duExposure);

    // CAMERA_SHAREMEMORY
    PRINT_INI_BUF("\n\t\t\t\t 摄像数据共享内存名: CAMERA_SHAREMEMORY->ShowSharedMemName = %s", m_stConfig.szCamDataSMemName);
    PRINT_INI_BUF("\n\t\t\t\t 摄像数据共享内存Size: CAMERA_SHAREMEMORY->ShowSharedMemSize = %ld", m_stConfig.ulCamDataSMemSize);


    // TAKEPIC_CONFIG
    PRINT_INI_BUF("\n\t\t\t\t TakePictrue/TakePictrueEx超时(毫秒): TAKEPIC_CONFIG->TakePicTimeOut = %d", m_stConfig.dwTakePicTimeOut);
    PRINT_INI_BUF("\n\t\t\t\t TakePicture命令缺省保存目录: TAKEPIC_CONFIG->TakePicDefSavePath = %s", m_stConfig.szTakePicDefSavePath);

    // OPEN_CONFIG
    PRINT_INI_BUF("\n\t\t\t\t Open失败时返回值(0:原样返回,1:返回SUCCESS): OPEN_CONFIG->OpenFailRet = %d", m_stConfig.wOpenFailRet);

    // RESET_CONFIG
    PRINT_INI_BUF("\n\t\t\t\t Reset执行时是否支持关闭摄像窗口(0:不支持,1支持): RESET_CONFIG->ResetCloseDiaplay = %d", m_stConfig.wResetCloseDiaplay);

    // BANK
    PRINT_INI_BUF("\n\t\t\t\t 指定银行: BANK->BankNo = %d", m_stConfig.wBank);

    // ERRDETAIL_CONFIG
    PRINT_INI_BUF("\n\t\t\t\t 是否支持Status显示错误码(0:不支持,1:支持显示当前错误码,2:支持显示当前和上一次错误码): ERRDETAIL_CONFIG->ErrDetailShowSup = %d", m_stConfig.wErrDetailShowSup);
    PRINT_INI_BUF("\n\t\t\t\t 当前错误码Key名: ERRDETAIL_CONFIG->ErrDetailKeyName = %s", m_stConfig.szErrDetailKeyName);
    PRINT_INI_BUF("\n\t\t\t\t 上一次错误码Key名: ERRDETAIL_CONFIG->LastErrDetailKeyName = %s", m_stConfig.szLastErrDetailKeyName);

    // SPECIAL_MODEL
    PRINT_INI_BUF("\n\t\t\t\t 设置生成的人脸图片名: SPECIAL_MODEL->MT1_PerSonName = %s", m_stConfig.stInitPar.szParStr[10]);
    PRINT_INI_BUF("\n\t\t\t\t 生成人像红外图后缀: SPECIAL_MODEL->MT2_NirImgSuffix = %s", m_stConfig.stInitPar.szParStr[11]);

    Log(ThisModule, __LINE__, "INI配置取得如下: %s", qsIniPrt.toStdString().c_str());

    return WFS_SUCCESS;
}

// 状态结构体实例初始化
void CXFS_CAM::InitStatus()
{
    m_stStatus.Clear();
    m_stStatus.fwDevice = WFS_CAM_DEVNODEVICE;
}

// 能力值结构体实例初始化
void CXFS_CAM::InitCaps()
{
    m_stCaps.wClass = WFS_SERVICE_CLASS_CAM;
    m_stCaps.fwCameras[WFS_CAM_ROOM] =
            MI_Enable(LCMODE_ROOM) == TRUE ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_PERSON] =
            MI_Enable(LCMODE_PERSON) == TRUE ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_EXITSLOT] =
            MI_Enable(LCMODE_EXITSLOT) == TRUE ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_EXTRA] =
            MI_Enable(LCMODE_EXTRA) == TRUE ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_HIGHTCAMERA] =
            MI_Enable(LCMODE_HIGHT) == TRUE ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_PANORAMIC] =
            MI_Enable(LCMODE_PANORA) == TRUE ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE;
    m_stCaps.lpszExtra = nullptr;
}

// 更新扩展数据
void CXFS_CAM::UpdateExtra()
{
    CHAR szVerBuff[256] = { 0x00 };
    CHAR szVersion[64] = { 0x00 };

    // 组织状态扩展数据
    m_cStatExtra.Clear();
    m_cStatExtra.AddExtra("VRTCount", "2");
    m_cStatExtra.AddExtra("VRTDetail[00]", (char*)byXFSVRTU);
    m_cStatExtra.AddExtra("VRTDetail[01]", (char*)byDevVRTU);

    //--------------------------取固件版本写入扩展数据--------------------------
    MCPY_NOLEN(szVerBuff, "FW:");
    for (WORD i = 0; i < LCMODE_CNT; i ++)
    {
        // 检查是否有同一设备已读取
        BOOL bIsGet = TRUE;
        if (MI_Enable(i) == TRUE && MI_DevDll(i) != nullptr)
        {
            for (WORD n = 0; n < i; n ++)
            {
                if (MI_GetDevType(i) == MI_GetDevType(n))
                {
                    bIsGet = FALSE;
                    break;
                }
            }

            // 读取版本
            if (bIsGet == TRUE)
            {
                MSET_0(szVersion);
                MI_DevDll(i)->GetVersion(GET_VER_FW, szVersion, sizeof(szVersion));
                if (strlen(szVersion) > 0)
                {
                    sprintf(szVerBuff + strlen(szVerBuff), " %s:%s",
                            DEVTYPE2STR(MI_GetDevType(i)), szVersion);
                }
            }
        }
    }

    // 固件版本写入扩展数据
    if (strlen(szVerBuff) - 3 > 0)
    {
        m_cStatExtra.AddExtra("VRTCount", "3");
        m_cStatExtra.AddExtra("VRTDetail[02]", szVerBuff);
    }

    m_cCapsExtra.Clear();
    m_cCapsExtra.CopyFrom(m_cStatExtra);
}

// 设备状态实时更新
WORD CXFS_CAM::UpdateDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    AutoMutex(*m_pMutexGetStatus);      // 必须加此互斥，防止同时读写数据问题

    INT     nRet = CAM_SUCCESS;
    //WORD    fwDevice = WFS_CAM_DEVHWERROR;
    DWORD   dwHWAct = WFS_ERR_ACT_NOACTION;
    CHAR    szHWDesc[1024] = { 0x00 };

    CHAR    szFireBuffer[1024] = { 0x00 };      // 事件上报记录Log

    BOOL    bNeedFireStatusChanged  = FALSE;
    BOOL    bNeedFireHWError        = FALSE;

    BOOL    bIsDevOffLine = FALSE;

    STDEVCAMSTATUS stDevStatus;

    for (WORD i = 0; i < LCMODE_CNT; i ++)
    {
        if (MI_Enable(i) == TRUE)
        {
            if (AND_IS0(m_wIsDevOpenFlag, LCMODE_TO_CMODE(i)))   // 未Open成功
            {
                m_stStatus.fwDevice = WFS_CAM_DEVOFFLINE;
                m_stStatus.fwMedia[LCMODE_TO_WMODE(i)] = WFS_CAM_MEDIAUNKNOWN;
                m_stStatus.fwCameras[LCMODE_TO_WMODE(i)] = WFS_CAM_CAMUNKNOWN;
                bIsDevOffLine = TRUE;
            } else
            {
                if (MI_DevDll(i) == nullptr)
                {
                    m_stStatus.fwDevice = WFS_CAM_DEVHWERROR;
                    m_stStatus.fwMedia[LCMODE_TO_WMODE(i)] = WFS_CAM_MEDIAUNKNOWN;
                    m_stStatus.fwCameras[LCMODE_TO_WMODE(i)] = WFS_CAM_CAMUNKNOWN;
                    bIsDevOffLine = TRUE;
                } else
                {
                    // 取设备状态
                    nRet = MI_DevDll(i)->GetStatus(stDevStatus);
                    switch (nRet)// 返回值处理
                    {
                        ;
                    }

                    //----------------------Device状态处理----------------------
                    m_stStatus.fwDevice = ConvertDeviceStatus2WFS(stDevStatus.wDevice);

                    // 设备状态处理(INI配置无设备时Device状态)
                    if (m_stConfig.wDevNotFoundStat != 0 &&
                        m_stStatus.fwDevice == WFS_CAM_DEVNODEVICE)
                    {
                        m_stStatus.fwDevice = WFS_CAM_DEVOFFLINE;
                    }

                    //----------------------Media状态处理----------------------
                    m_stStatus.fwMedia[LCMODE_TO_WMODE(i)] = ConvertMediaStatus2WFS(stDevStatus.wMedia[0]);

                    //----------------------Cameras状态处理----------------------
                    m_stStatus.fwCameras[LCMODE_TO_WMODE(i)] = ConvertCamerasStatus2WFS(stDevStatus.fwCameras[0]);
                }
            }

            // 非连接状态下关闭摄像窗口
            if (m_stStatus.fwDevice == WFS_CAM_DEVOFFLINE ||
                m_stStatus.fwDevice == WFS_STAT_DEVPOWEROFF ||
                m_stStatus.fwDevice == WFS_STAT_DEVNODEVICE)
            {
                if (AND_IS1(m_wIsDevOpenFlag, LCMODE_TO_CMODE(i)))
                {
                    m_wIsDevOpenFlag ^= LCMODE_TO_CMODE(i);
                }

                if (m_wDisplayOK == LCMODE_TO_CMODE(i) &&   // 断线时窗口有显示:执行关闭
                    MI_DevDll(i) != nullptr)
                {
                    STDISPLAYPAR stDispPar;
                    stDispPar.wAction = WFS_CAM_DESTROY;
                    MI_DevDll(i)->Display(stDispPar);

                    if (m_wWindowsRunMode == WIN_RUN_SHARED)
                    {
                        emit vSignHideWin();
                    }
                    m_wDisplayOK = 0;
                    m_wWindowsRunMode = 0;
                }
                bIsDevOffLine = TRUE;
            }
        }
    }

    if (bIsDevOffLine == TRUE)
    {
        m_stStatus.fwDevice = WFS_CAM_DEVOFFLINE;
    }

    // Device == ONLINE && 有命令在执行中,设置Device = BUSY
    if (m_stStatus.fwDevice == WFS_CAM_DEVONLINE && m_bCmdRunning == TRUE)    // 命令执行中
    {
        m_stStatus.fwDevice = WFS_CAM_DEVBUSY;
    }

    //----------------------状态检查----------------------
    // Device状态变化检查
    if (m_stStatus.fwDevice != m_stStatusOLD.fwDevice)
    {
        bNeedFireStatusChanged = TRUE;
        if (m_stStatus.fwDevice == WFS_CAM_DEVHWERROR)
        {
            bNeedFireHWError = TRUE;
        }
    }

    //----------------------事件上报----------------------
    // 硬件故障事件
    if (bNeedFireHWError)
    {
        FireHWEvent(dwHWAct, szHWDesc);
        sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%s|", dwHWAct, szHWDesc);
    }

    // 设备状态变化事件
    if (bNeedFireStatusChanged)
    {
        FireStatusChanged(m_stStatus.fwDevice);
        sprintf(szFireBuffer + strlen(szFireBuffer), "StatusChange:%d|",  m_stStatus.fwDevice);
    }

    // 其他检查上报
    if (stDevStatus.nOtherCode[0] == 1) // 上报Display命令及关联错误
    {
        dwHWAct = ConvertEventAct2WFS(stDevStatus.nOtherCode[2]);
        FireHWEvent(dwHWAct, szHWDesc);
        sprintf(szFireBuffer + strlen(szFireBuffer), "HWEvent:%d,%s|", dwHWAct, szHWDesc);

        // 关闭窗口
        if (m_wWindowsRunMode == WIN_RUN_SHARED)
        {
            emit vSignHideWin();
        }
        m_wDisplayOK = 0;
        m_wWindowsRunMode = 0;
    }

    // 比较两次状态记录LOG
    if (m_stStatus.Diff(m_stStatusOLD) == true)
    {
        Log(ThisModule, __LINE__, "状态结果比较: Device:%d->%d%s|"
                                  "Media[ROOM]:%d->%d%s|Media[PERSON]:%d->%d%s|Media[EXTRA]:%d->%d%s|Media[HIGH]:%d->%d%s|"
                                  "Cameras[ROOM]:%d->%d%s|Cameras[PERSON]:%d->%d%s|Cameras[EXTRA]:%d->%d%s|Cameras[HIGH]:%d->%d%s|; "
                                  "事件上报记录: %s.",
            m_stStatusOLD.fwDevice, m_stStatus.fwDevice, (m_stStatusOLD.fwDevice != m_stStatus.fwDevice ? " *" : ""),
            m_stStatusOLD.fwMedia[WFS_CAM_ROOM], m_stStatus.fwMedia[WFS_CAM_ROOM],
             (m_stStatusOLD.fwMedia[WFS_CAM_ROOM] != m_stStatus.fwMedia[WFS_CAM_ROOM] ? " *" : ""),
            m_stStatusOLD.fwMedia[WFS_CAM_PERSON], m_stStatus.fwMedia[WFS_CAM_PERSON],
             (m_stStatusOLD.fwMedia[WFS_CAM_PERSON] != m_stStatus.fwMedia[WFS_CAM_PERSON] ? " *" : ""),
            m_stStatusOLD.fwMedia[WFS_CAM_EXTRA], m_stStatus.fwMedia[WFS_CAM_EXTRA],
             (m_stStatusOLD.fwMedia[WFS_CAM_EXTRA] != m_stStatus.fwMedia[WFS_CAM_EXTRA] ? " *" : ""),
            m_stStatusOLD.fwMedia[WFS_CAM_HIGHTCAMERA], m_stStatus.fwMedia[WFS_CAM_HIGHTCAMERA],
             (m_stStatusOLD.fwMedia[WFS_CAM_HIGHTCAMERA] != m_stStatus.fwMedia[WFS_CAM_HIGHTCAMERA] ? " *" : ""),
            m_stStatusOLD.fwCameras[WFS_CAM_ROOM], m_stStatus.fwCameras[WFS_CAM_ROOM],
             (m_stStatusOLD.fwCameras[WFS_CAM_ROOM] != m_stStatus.fwCameras[WFS_CAM_ROOM] ? " *" : ""),
            m_stStatusOLD.fwCameras[WFS_CAM_PERSON], m_stStatus.fwCameras[WFS_CAM_PERSON],
             (m_stStatusOLD.fwCameras[WFS_CAM_PERSON] != m_stStatus.fwCameras[WFS_CAM_PERSON] ? " *" : ""),
            m_stStatusOLD.fwCameras[WFS_CAM_EXTRA], m_stStatus.fwCameras[WFS_CAM_EXTRA],
             (m_stStatusOLD.fwCameras[WFS_CAM_EXTRA] != m_stStatus.fwCameras[WFS_CAM_EXITSLOT] ? " *" : ""),
            m_stStatusOLD.fwCameras[WFS_CAM_HIGHTCAMERA], m_stStatus.fwCameras[WFS_CAM_HIGHTCAMERA],
             (m_stStatusOLD.fwCameras[WFS_CAM_HIGHTCAMERA] != m_stStatus.fwCameras[WFS_CAM_HIGHTCAMERA] ? " *" : ""),
            szFireBuffer);
        m_stStatusOLD.Copy(m_stStatus);
    }

    return 0;
}

// 摄像处理
HRESULT CXFS_CAM::InnerTakePicture(const WFSCAMTAKEPICT &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WFSCAMTAKEPICTEX stCamTakePictEx;

    stCamTakePictEx.wCamera = stTakePict.wCamera;
    //MCPY_NOLEN(stCamTakePictEx.lpszCamData, stTakePict.lpszCamData);
    //MCPY_NOLEN(stCamTakePictEx.lpszUNICODECamData, stTakePict.lpszUNICODECamData);
    stCamTakePictEx.lpszCamData = stTakePict.lpszCamData;
    stCamTakePictEx.lpszUNICODECamData = stTakePict.lpszUNICODECamData;

    if (strlen(m_stConfig.szTakePicDefSavePath) > 5)
    {
        //MCPY_NOLEN(stCamTakePictEx.lpszPictureFile, m_stConfig.szTakePicDefSavePath);
        stCamTakePictEx.lpszPictureFile = strdup(m_stConfig.szTakePicDefSavePath);
    } else
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败 :INI->TakePicDefSavePath[%s]设置图像保存文件无效, Return: %d.",
            m_stConfig.szTakePicDefSavePath, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    }

    HRESULT hRet = InnerTakePictureEx(stCamTakePictEx, dwTimeout);

    stCamTakePictEx.lpszCamData = nullptr;
    stCamTakePictEx.lpszUNICODECamData = nullptr;
    free(stCamTakePictEx.lpszPictureFile);

    return hRet;
}

// 拍照处理扩展
HRESULT CXFS_CAM::InnerTakePictureEx(const WFSCAMTAKEPICTEX &stTakePict, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CAM_SUCCESS;
    LPWFSCAMTAKEPICTEX	lpCmdData = nullptr;
    lpCmdData = (LPWFSCAMTAKEPICTEX)&stTakePict;
    STTAKEPICTUREPAR stTakePicPar;

    // 检查是否Display已正确执行
    if (m_wDisplayOK == 0)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 命令序列错误, Display未执行, Return: %d", WFS_ERR_DEV_NOT_READY);
        SetErrorDetail((LPSTR)EC_CAM_XFS_NotDisplay);
        return WFS_ERR_DEV_NOT_READY;
    }

    // 检查摄像模式与Display是否一致
    if (stTakePict.wCamera != WFS_CAM_ROOM && stTakePict.wCamera != WFS_CAM_PERSON &&
        stTakePict.wCamera != WFS_CAM_EXITSLOT && stTakePict.wCamera != WFS_CAM_EXTRA &&
        stTakePict.wCamera != WFS_CAM_HIGHTCAMERA && stTakePict.wCamera != WFS_CAM_PANORAMIC)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参摄像模式无效(不支持), Return: %d", stTakePict.wCamera, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_XFS_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    // 检查摄像模式与Display是否一致
    if (WMODE_TO_CMODE(stTakePict.wCamera) != m_wDisplayOK)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: Display摄像窗口模式非入参Camera[%d]指定模式, Return: %d",
            stTakePict.wCamera, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_XFS_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    // 图片文件名及路径检查
    if (lpCmdData->lpszPictureFile == NULL)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[NULL]无效, Return: %d", WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    }

    MSET_0(m_szFileName);
    if (strlen(lpCmdData->lpszPictureFile) > 5)
    {
        sprintf(m_szFileName, "%s", lpCmdData->lpszPictureFile);
    } else
    if (strlen(m_stConfig.szTakePicDefSavePath) > 0)
    {
        sprintf(m_szFileName, "%s", m_stConfig.szTakePicDefSavePath);
    } else
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[%s]无效, Return: %d.",
            lpCmdData->lpszPictureFile, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    }

    if (m_szFileName[0] != '/')
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[%s]无效, Return: %d.",
            lpCmdData->lpszPictureFile, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    }

    // 检查扩展名
    std::string stdFilePath = m_szFileName;
    int iIndex = stdFilePath.rfind('.');
    if (iIndex == -1)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[%s]无效, 未指定图片文件扩展名, Return: %d.",
            m_szFileName, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_CAM_XFS_FilePathInv);
        return WFS_ERR_INVALID_DATA;
    } else
    {
        std::string stdTmp;
        stdTmp = stdFilePath.substr(0, iIndex);

        std::string szEx = stdFilePath.substr(iIndex, stdFilePath.length() - iIndex);
        std::transform(szEx.begin(), szEx.end(), szEx.begin(), ::toupper);
        if (szEx.compare(".BASE64") == 0)
        {
            stTakePicPar.wPicType = PIC_BASE64;
        } else
        if (szEx.compare(".JPG") == 0)
        {
            stTakePicPar.wPicType = PIC_JPG;
        } else
        {
            stTakePicPar.wPicType = PIC_BMP;
        }
    }

    // 目录验证
    if (FileAccess::create_directory_by_path(m_szFileName, true) != true)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参lpszPictureFile[%s]创建目录结构失败, Return: %d.",
            m_szFileName, WFS_ERR_INTERNAL_ERROR);
        SetErrorDetail((LPSTR)EX_XFS_FilePathCrtErr);
        return WFS_ERR_INTERNAL_ERROR;
    }

    // 根据INI配置超时设置
    stTakePicPar.dwTimeOut =
            (m_stConfig.dwTakePicTimeOut == 0 ? dwTimeout : m_stConfig.dwTakePicTimeOut);

    // 组织命令下发入参
    //stTakePicPar.wCameraAction = lpCmdData->wCamera;                // 摄像模式
    MCPY_NOLEN(stTakePicPar.szCamData,
               lpCmdData->lpszCamData == nullptr ? "" : lpCmdData->lpszCamData);     // 水印
    MCPY_NOLEN(stTakePicPar.szFileName, m_szFileName);              // 保存图片文件名

    // 检查DevCAM连接是否正常
    if (MI_DevDll(WMODE_TO_LCMODE(stTakePict.wCamera)) == nullptr)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: DevCAM(%s)连接为NULL, Return: %d.",
            MI_ModeName(WMODE_TO_LCMODE(stTakePict.wCamera)),
            WFS_ERR_INTERNAL_ERROR);
        SetErrorDetail(EX_XFS_ConnInvalid);
        return ConvertDevErrCode2WFS(nRet);
    }

    // TakePic命令下发前,根据INI配置选择开启 活体图像检测错误事件 线程
    if (m_stConfig.wLiveErrorSup == 1)
    {
        // 建立线程，循环检查活体图像结果
        m_bLiveEventWaitExit = FALSE; // 指示线程结束标记:F
        if (!m_thRunLiveEventWait.joinable())
        {
            m_thRunLiveEventWait = std::thread(&CXFS_CAM::ThreadLiveEvent_Wait, this,
                                               WMODE_TO_DMODE(stTakePict.wCamera));
            if (m_thRunLiveEventWait.joinable())
            {
                m_thRunLiveEventWait.detach();
            }
        }
    }

    // 按不同摄像模式下发命令
    stTakePicPar.wCameraAction = WMODE_TO_DMODE(stTakePict.wCamera);
    nRet = MI_DevDll(WMODE_TO_LCMODE(stTakePict.wCamera))->TakePicture(stTakePicPar);
    m_bLiveEventWaitExit = TRUE;    // TakePic命令返回则立即结束活体图像结果检查线程
    if (nRet != CAM_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "拍照: ->TakePicture() fail, ErrCode: %s, Return: %d.",
            ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
        SetErrorDetail(nullptr, WMODE_TO_LCMODE(stTakePict.wCamera));
        return ConvertDevErrCode2WFS(nRet);
    }

    // TakePicture命令完成后处理
    InnerTakePicAfterMothod();

    return WFS_SUCCESS;
}

// 窗口处理
HRESULT CXFS_CAM::InnerDisplay(const WFSCAMDISP &stDisplay, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    INT nRet = CAM_SUCCESS;

    LPWFSCAMDISP lpDisplay = NULL;
    lpDisplay = (LPWFSCAMDISP)&stDisplay;

    WORD wDisplayMode = 0;
    WORD wLCMode = 0;

    // 入参摄像模式检查
    if (lpDisplay->wCamera != WFS_CAM_ROOM && lpDisplay->wCamera != WFS_CAM_PERSON &&
        lpDisplay->wCamera != WFS_CAM_EXITSLOT && lpDisplay->wCamera != WFS_CAM_EXTRA &&
        lpDisplay->wCamera != WFS_CAM_HIGHTCAMERA && lpDisplay->wCamera != WFS_CAM_PANORAMIC)
    {
        Log(ThisModule, __LINE__,
            "拍照: 失败: 入参摄像模式无效(不支持), Return: %d", lpDisplay->wCamera, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_XFS_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    wLCMode = WMODE_TO_LCMODE(lpDisplay->wCamera);

    // 检查INI设定摄像模式支持
    if (MI_Enable(wLCMode) != TRUE)    // 模式不支持
    {
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 失败: 入参Camera[%d]无效, INI设定[%s]模式不支持, Return: %d",
            lpDisplay->wCamera, m_stCamModeInfo.stModeList[wLCMode].szModeName,
            WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_XFS_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    } else
    {
        // 已有摄像窗口打开, 非入参指定模式
        if (m_wDisplayOK > 0 &&
            m_wDisplayOK != WMODE_TO_CMODE(lpDisplay->wCamera))
        {
            Log(ThisModule, __LINE__,
                "创建摄像窗口: 失败: 窗口已打开, 非入参Camera[%d]指定模式, Return: %d",
                lpDisplay->wCamera, WFS_ERR_INVALID_DATA);
            SetErrorDetail((LPSTR)EC_XFS_ParInvalid);
            return WFS_ERR_INVALID_DATA;
        }
    }
    wDisplayMode = WMODE_TO_CMODE(lpDisplay->wCamera);

    // 窗口控制检查
    if(lpDisplay->wAction != WFS_CAM_CREATE &&      // 创建摄像窗口
       lpDisplay->wAction != WFS_CAM_DESTROY &&     // 销毁摄像窗口
       lpDisplay->wAction != WFS_CAM_PAUSE &&       // 暂停摄像窗口
       lpDisplay->wAction != WFS_CAM_RESUME)
    {
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 失败: 入参wAction[%d]无效．Return: %d",
            lpDisplay->wAction, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_XFS_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    // 创建窗口时宽高检查
    if(lpDisplay->wAction == WFS_CAM_CREATE &&
       (lpDisplay->wHeight == 0 || lpDisplay->wWidth == 0))
    {
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 失败: 入参wHeight[%d]/wWidth[%d]无效．Return: %d",
            lpDisplay->wHeight, lpDisplay->wWidth, WFS_ERR_INVALID_DATA);
        SetErrorDetail((LPSTR)EC_XFS_ParInvalid);
        return WFS_ERR_INVALID_DATA;
    }

    // 根据INI配置设置水平分辨率参数
    if (lpDisplay->wHpixel == 0 &&
        m_stConfig.stDevOpenMode[MI_GetDevType(wLCMode)].nOtherParam[2] > 0)
    {
        lpDisplay->wHpixel = m_stConfig.stDevOpenMode[MI_GetDevType(wLCMode)].nOtherParam[2];
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 入参 水平分辨率Hpixel=%d, INI设置截取画面分辨率>0, 设置水平分辨率Hpixel为%d.",
            lpDisplay->wHpixel, m_stConfig.stDevOpenMode[MI_GetDevType(wLCMode)].nOtherParam[2]);
    }

    // 根据INI配置设置垂直分辨率参数
    if (lpDisplay->wVpixel == 0 &&
        m_stConfig.stDevOpenMode[MI_GetDevType(wLCMode)].nOtherParam[3] > 0)
    {
        lpDisplay->wVpixel = m_stConfig.stDevOpenMode[MI_GetDevType(wLCMode)].nOtherParam[3];
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 入参 垂直分辨率Vpixel=%d, INI设置截取画面分辨率>0, 设置垂直分辨率Vpixel为%d.",
            lpDisplay->wVpixel, m_stConfig.stDevOpenMode[MI_GetDevType(wLCMode)].nOtherParam[3]);
    }

    // 检查DevCAM连接是否正常
    if (MI_DevDll(WMODE_TO_LCMODE(wLCMode)) == nullptr)
    {
        Log(ThisModule, __LINE__,
            "创建摄像窗口: 失败: DevCAM(%s)连接为NULL, Return: %d.",
            MI_ModeName(WMODE_TO_LCMODE(wLCMode)),
            WFS_ERR_INTERNAL_ERROR);
        SetErrorDetail(EX_XFS_ConnInvalid);
        return ConvertDevErrCode2WFS(nRet);
    }

    // 命令下发
    STDISPLAYPAR stDisplayPar;
    stDisplayPar.dwTimeOut = dwTimeout;
    stDisplayPar.hWnd = lpDisplay->hWnd;
    stDisplayPar.wCamera = lpDisplay->wCamera;
    stDisplayPar.wAction = lpDisplay->wAction;
    stDisplayPar.wX = lpDisplay->wX;
    stDisplayPar.wY = lpDisplay->wY;
    stDisplayPar.wWidth = lpDisplay->wWidth;
    stDisplayPar.wHeight = lpDisplay->wHeight;
    stDisplayPar.wHpixel = lpDisplay->wHpixel;
    stDisplayPar.wVpixel = lpDisplay->wVpixel;

    // 按不同摄像模式下发命令
    nRet = MI_DevDll(wLCMode)->Display(stDisplayPar);
    if (nRet != CAM_SUCCESS)
    {
        Log(ThisModule, __LINE__,
            "创建摄像窗口: ->Display(%d|%s) fail, ErrCode: %s, Return: %d",
            lpDisplay->wCamera, MI_ModeName(wLCMode),
            ConvertDevErrCodeToStr(nRet), ConvertDevErrCode2WFS(nRet));
        SetErrorDetail(nullptr, wLCMode);
        return ConvertDevErrCode2WFS(nRet);
    } else
    {
        if (lpDisplay->wAction == WFS_CAM_CREATE)
        {
            // 命令成功且为创建窗口参数时, 设定窗口建立方式
            m_wWindowsRunMode = m_wDeviceShowWinMode[MI_GetDevType(wLCMode)];
        }
    }

    // 根据窗口显示方式打开窗口
    if (lpDisplay->wAction == WFS_CAM_CREATE)
    {
        if (m_wWindowsRunMode == WIN_RUN_SHARED)
        {
            STSHOWWININFO stCamInfo;
            stCamInfo.hWnd = lpDisplay->hWnd;
            stCamInfo.wX = lpDisplay->wX;
            stCamInfo.wY = lpDisplay->wY;
            stCamInfo.wWidth = lpDisplay->wWidth;
            stCamInfo.wHeight = lpDisplay->wHeight;
            emit vSignShowWin(stCamInfo);
        }
        m_wDisplayOK = wDisplayMode;
    } else
    if (lpDisplay->wAction == WFS_CAM_DESTROY)
    {
        if (m_wWindowsRunMode == WIN_RUN_SHARED)
        {
            emit vSignHideWin();
        }

        m_wDisplayOK = 0;
        m_wWindowsRunMode = 0;
    }

    return WFS_SUCCESS;
}

//--------------------------------窗口处理---------------------------------------
// 显示窗口(启动SP内建窗口及共享内存计时器)
void CXFS_CAM::vSlotShowWin(STSHOWWININFO stCamShowInfo)
{
    int nRet = 0;

    if (showWin != nullptr)
    {
        showWin->hide();
        delete showWin;
        showWin = nullptr;
    }

    QWidget *myWidget = nullptr;
    myWidget = QWidget::createWindowContainer(QWindow::fromWinId((WId)stCamShowInfo.hWnd));

    showWin = new MainWindow(myWidget);

    if ((nRet = showWin->nConnSharedMem(m_stConfig.szCamDataSMemName)) > 0)
    {
        showWin->Release();
        return;
    }

    showWin->ShowWin(stCamShowInfo.wX, stCamShowInfo.wY,
                     stCamShowInfo.wWidth, stCamShowInfo.wHeight);

    showWin->vTimerStart(m_stConfig.unWinRefreshTime);

    //showWin->show();
}

// 结束显示窗口
void CXFS_CAM::vSlotHideWin()
{
    if (showWin != nullptr)
    {
        showWin->hide();
        delete showWin;
        showWin = nullptr;
    }
}

//-----------------------------共享内存处理----------------------------------------
// 创建共享内存
BOOL CXFS_CAM::bCreateSharedMemory(LPSTR lpMemName, ULONG ulSize)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 创建共享内存Data
    if (m_qSharedMemData == nullptr)
    {
        m_qSharedMemData = new QSharedMemory(lpMemName);   // 构造实例对象 + 为实例对象指定关键字(给共享内存命名)
        if (m_qSharedMemData == nullptr)
        {
            Log(ThisModule, __LINE__, "wCreateSharedMemory->创建SharedMemeory(%s)实例失败. ", lpMemName);
            return FALSE;
        }
    }

    if (m_qSharedMemData->create(ulSize, QSharedMemory::ReadWrite) != true)
    {
        if (m_qSharedMemData->error() == QSharedMemory::AlreadyExists)
        {
            Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s)共享内存已存在,不继续创建. ", lpMemName);
            m_qSharedMemData->attach();
            if (m_qSharedMemData->isAttached() != true)
            {
                Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s, %d)共享内存挂载失败, ReturnCode:%d|%s. ",
                    lpMemName, ulSize, m_qSharedMemData->error(), m_qSharedMemData->errorString().toUtf8().data());
                SetErrorDetail((LPSTR)EC_XFS_ShareMemCrt);
                return FALSE;
            }
        } else
        {
            Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s, %d)共享内存创建失败, ReturnCode:%d|%s. ",
                lpMemName, ulSize, m_qSharedMemData->error(), m_qSharedMemData->errorString().toUtf8().data());
            SetErrorDetail((LPSTR)EC_XFS_ShareMemCrt);
            return FALSE;
        }
    }

    Log(ThisModule, __LINE__, "wCreateSharedMemory()->SharedMemeory(%s, %d)共享内存创建完成. ",  lpMemName, ulSize);

    return TRUE;
}

// 销毁共享内存
void CXFS_CAM::SharedMemRelease()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if (m_qSharedMemData != nullptr)
    {
        if (m_qSharedMemData->isAttached())
        {
            m_qSharedMemData->detach();
            delete m_qSharedMemData;
            m_qSharedMemData = nullptr;
        }
    }

    return;
}

//-----------------------------其他处理----------------------------------------
// TakePicture命令完成后处理
INT CXFS_CAM::InnerTakePicAfterMothod()
{
    THISMODULE(__FUNCTION__);

    return WFS_SUCCESS;
}

// 设置ErrorDetail错误码
INT CXFS_CAM::SetErrorDetail(LPSTR lpCode, INT nIdx)
{
    if (lpCode == nullptr)
    {        
        if (nIdx >= 0)
        {
            CHAR szErrCode[1024] = { 0x00 };
            MI_DevDll(nIdx)->GetData(GET_DEV_ERRCODE, szErrCode);
            m_clErrorDet.SetErrCode(szErrCode + 1);
        }
    } else
    {
        m_clErrorDet.SetErrCode(lpCode);
    }

    return WFS_SUCCESS;
}

// Open前检查INI设定摄像模式和设备模式是否正常
HRESULT CXFS_CAM::ChkOpenIsCamDevMode()
{
    THISMODULE(__FUNCTION__);

    for (WORD i = 0; i < LCMODE_CNT; i ++)
    {
        if (MI_Enable(i) == TRUE)
        {
            if (CHK_ISDEVTYPE(MI_GetDevType(i)) != TRUE)
            {
                Log(ThisModule, __LINE__,
                    "Open fail, INI设定[%s]的设备类型[%d]不支持, Return :%d.",
                    MI_ModeName(i), MI_GetDevType(i), WFS_ERR_DEV_NOT_READY);
                SetErrorDetail((LPSTR)EC_XFS_DevNotSupp);
                return WFS_ERR_DEV_NOT_READY;
            }
        } else
        {
            Log(ThisModule, __LINE__, "INI设定不支持[%s].", MI_ModeName(i));
        }
    }

    return WFS_SUCCESS;
}

// Open前检查摄像窗口显示模式
HRESULT CXFS_CAM::ChkOpenShowWinMode()
{
    THISMODULE(__FUNCTION__);

    WORD wWinShowMode = 0;

    for (WORD i = 0; i < LCMODE_CNT; i ++)
    {
        if (MI_Enable(i) == TRUE)       // 支持的摄像模式
        {
            wWinShowMode |= m_wDeviceShowWinMode[MI_GetDevType(i)];
        }
    }

    // 设备采用SDK创建窗口方式(不处理)
    if (AND_IS1(wWinShowMode, WIN_RUN_SDK))
    {
        ;
    }

    // 设备采用SP创建内部窗口,共享内存传递数据(建共享内存)
    if (AND_IS1(wWinShowMode, WIN_RUN_SHARED))
    {
        if (bCreateSharedMemory(m_stConfig.szCamDataSMemName,
                                m_stConfig.ulCamDataSMemSize) != TRUE)
        {
            Log(ThisModule, __LINE__, "摄像共享内存[%s|%ld]创建失败, Return: %d",
                m_stConfig.szCamDataSMemName, m_stConfig.ulCamDataSMemSize,
                WFS_ERR_INTERNAL_ERROR);
            SetErrorDetail((LPSTR)EC_XFS_ShareMemCrt);
            return WFS_ERR_INTERNAL_ERROR;
        }

        qRegisterMetaType<STSHOWWININFO>("STSHOWWININFO");
        connect(this, SIGNAL(vSignShowWin(STSHOWWININFO)), this, SLOT(vSlotShowWin(STSHOWWININFO)), Qt::AutoConnection);
        connect(this, SIGNAL(vSignHideWin()), this, SLOT(vSlotHideWin()), Qt::AutoConnection);
    }

    // SP创建外接程序窗口,共享内存传递数据()
    if (AND_IS1(wWinShowMode, WIN_RUN_SHARED_PROG))
    {
        ;
    }

    return WFS_SUCCESS;
}

// 检查是否有同一设备已Open,有则共享连接
// wCamMode: 设备模式下标
BOOL CXFS_CAM::OpenFrontChkDevIsOpen(WORD wCamMode, BOOL bIsLog)
{
    THISMODULE(__FUNCTION__);

    for (WORD i = 0; i < LCMODE_CNT; i ++)
    {
        if (i == wCamMode)      // 同一下标不处理
        {
            continue;
        }

        if (MI_GetDevType(wCamMode) > 0 &&                  // 设备类型非0(无效)
            MI_GetDevType(wCamMode) == MI_GetDevType(i))    // 同一设备
        {
            // 同一设备已Open成功, 共享统一连接, 否则返回FALSE
            if (AND_IS1(m_wIsDevOpenFlag, LCMODE_TO_CMODE(i)) &&
                MI_DevDll(i) != nullptr)
            {
                MI_RelDevDll(wCamMode);
                MI_DevDll(wCamMode) = MI_DevDll(i).GetType();
                if (bIsLog == TRUE)
                {
                    Log(ThisModule, __LINE__, "%s 与 %s 使用同一设备(连接), Dev实例复制, 不再次Open",
                        MI_ModeName(wCamMode), MI_ModeName(i));
                }
                return TRUE;
            }
        }
    }

    return FALSE;
}

// 设备Open前参数传递
INT CXFS_CAM::OpenFrontSendDevParam(BOOL bReConn, WORD wCamMode, WORD wDeviceType)
{
    // Open前下传初始参数(非断线重连)
    if (bReConn == FALSE)
    {
        // 设置SDK路径(需指定设备类型)
        if (strlen(m_stConfig.szSDKPath[wDeviceType]) > 0)
        {
            MI_DevDll(wCamMode)->SetData(SET_LIB_PATH, m_stConfig.szSDKPath[wDeviceType]);
        }

        // 设置初始化参数(共用)
        MI_DevDll(wCamMode)->SetData(SET_DEV_INIT, &(m_stConfig.stInitPar));

        // 设置设备打开模式(需指定设备类型)
        MI_DevDll(wCamMode)->SetData(SET_DEV_OPENMODE, &(m_stConfig.stDevOpenMode[wDeviceType]));

        // 设置设备摄像模式(共用)
        MI_DevDll(wCamMode)->SetData(SET_DEV_VIDEOMODE, &(m_stConfig.stVideoPar));
    }

    return WFS_SUCCESS;
}

// INI共通变量配置获取
INT CXFS_CAM::InitConfigDef(LPCSTR lpsKeyName, WORD wDeviceType)
{
    THISMODULE(__FUNCTION__);

    CHAR    szBuffer[MAX_PATH];

    // 使用0～9通用参数范围
    // 摄像刷新时间(毫秒,缺省30)
    m_stConfig.stDevOpenMode[wDeviceType].nOtherParam[1] =
            m_cXfsReg.GetValue(lpsKeyName, "WinRefreshTime", (DWORD)30);
    if (m_stConfig.stDevOpenMode[wDeviceType].nOtherParam[1] < 0)
    {
        m_stConfig.stDevOpenMode[wDeviceType].nOtherParam[1] = 30;
    }

    // 截取画面帧的分辨率(Width),缺省:0
    m_stConfig.stDevOpenMode[wDeviceType].nOtherParam[2] =
            m_cXfsReg.GetValue(lpsKeyName, "FrameResoWidth", (DWORD)0);
    if (m_stConfig.stDevOpenMode[wDeviceType].nOtherParam[2] < 0)
    {
        m_stConfig.stDevOpenMode[wDeviceType].nOtherParam[2] = 0;
    }

    // 截取画面帧的分辨率(Height),缺省:0
    m_stConfig.stDevOpenMode[wDeviceType].nOtherParam[3] =
            m_cXfsReg.GetValue(lpsKeyName, "FrameResoHeight", (DWORD)0);
    if (m_stConfig.stDevOpenMode[wDeviceType].nOtherParam[3] < 0)
    {
        m_stConfig.stDevOpenMode[wDeviceType].nOtherParam[3] = 0;
    }

    // 摄像窗口显示方式(0:SDK控制, 1:SP内处理, 2:外接程序窗口, 缺省0)
    m_wDeviceShowWinMode[wDeviceType] = m_cXfsReg.GetValue(lpsKeyName, "ShowWinMode", (DWORD)0);
    if (m_wDeviceShowWinMode[wDeviceType] == 1)
    {
        m_wDeviceShowWinMode[wDeviceType] = WIN_RUN_SHARED;
    } else
    if (m_wDeviceShowWinMode[wDeviceType] == 2)
    {
        m_wDeviceShowWinMode[wDeviceType] = WIN_RUN_SHARED_PROG;
        // 摄像窗口外接程序(绝对路径或相对路径, 缺省CwCamShow), ShowWinMode=1时必须指定
        // 注意: 相对路径会在/usr/local/CFES/BIN目录下寻找
        memset(szBuffer, 0x00, sizeof(szBuffer));
        strcpy(szBuffer, m_cXfsReg.GetValue(lpsKeyName, "WinProcessPath", "CwCamShow"));
        memset(m_szWinProcessPath[wDeviceType], 0x00, sizeof(m_szWinProcessPath));
        if (strlen(szBuffer) == 0)
        {
            sprintf(m_szWinProcessPath[wDeviceType], "/usr/local/CFES/BIN/CwCamShow");
        } else
        if (strlen(szBuffer) == 1)
        {
            if (szBuffer[0] == '/')
            {
                sprintf(m_szWinProcessPath[wDeviceType], "/usr/local/CFES/BIN/CwCamShow");
            } else
            {
                sprintf(m_szWinProcessPath[wDeviceType], "/usr/local/CFES/BIN/%s", szBuffer);
            }
        } else
        {
            if (szBuffer[0] == '/')
            {
                sprintf(m_szWinProcessPath[wDeviceType], "%s", szBuffer);
            } else
            {
                sprintf(m_szWinProcessPath[wDeviceType], "/usr/local/CFES/BIN/%s", szBuffer);
            }
        }
    } else
    {
        m_wDeviceShowWinMode[wDeviceType] = WIN_RUN_SDK;
    }

    return WFS_SUCCESS;
}

// LiveEvent循环执行
void CXFS_CAM::ThreadLiveEvent_Wait(WORD wMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    WORD wError = 0;
    WORD wGetErr = 0;

    while(true)
    {
        if (m_bLiveEventWaitExit == TRUE)
        {
            break;
        }

        MI_DevDll(wMode)->GetData(GET_DEV_LIVESTAT, &wGetErr);

        if (wGetErr > 0 && wGetErr != wError)
        {
            wError = wGetErr;
            FireEXEE_LIVEERROR_Event(ConvertLiveErr2WFS(wError));
        }

        usleep(1000 * 20);
    }

    m_bLiveEventWaitExit = TRUE;
}

// -------------------------------------- END --------------------------------------

