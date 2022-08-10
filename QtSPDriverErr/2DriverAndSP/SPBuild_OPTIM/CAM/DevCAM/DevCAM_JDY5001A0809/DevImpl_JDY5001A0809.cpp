/***************************************************************************
* 文件名称: DevImpl_JDY5001A0809.h
* 文件描述: 封装摄像模块底层指令,提供控制接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
***************************************************************************/

#include "DevImpl_JDY5001A0809.h"


static const char *ThisFile = "DevImpl_JDY5001A0809.cpp";

//-----------------------------构造/析构/初始化-----------------------------
CDevImpl_JDY5001A0809::CDevImpl_JDY5001A0809()
{
    SetLogFile(LOG_NAME, ThisFile);                 // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_JDY5001A0809::CDevImpl_JDY5001A0809(LPSTR lpLog)
{
    SetLogFile(lpLog, ThisFile);                    // 设置日志文件名和错误发生的文件
    Init();
}

CDevImpl_JDY5001A0809::CDevImpl_JDY5001A0809(LPSTR lpLog, LPCSTR lpDevType)
{
    SetLogFile(lpLog, ThisFile, lpDevType);         // 设置日志文件名和错误发生的文件
    MSET_0(m_szDevType);
    MCPY_NOLEN(m_szDevType, strlen(lpDevType) > 0 ? lpDevType : "JDY-5001A-0809");
    Init();
}

// 参数初始化
void CDevImpl_JDY5001A0809::Init()
{
    MSET_0(m_szDevType);                            // 设备类型
    m_bDevOpenOk = FALSE;                           // 设备是否Open
    MSET_0(m_szDevVidPid);                          // 保存设备VIDPID
    memset(m_nRetErrOLD, 0, sizeof(INT) * 8);
}

CDevImpl_JDY5001A0809::~CDevImpl_JDY5001A0809()
{
    m_stImageData.Clear();
}

/***************************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::OpenDevice(LPSTR lpVid, LPSTR lpPid)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = DP_RET_SUCC;

    m_clDevVideo.Close();

    nRet = m_clDevVideo.Open(lpVid, lpPid);
    if (nRet != DP_RET_SUCC)
    {
        if (m_nRetErrOLD[1] != nRet)
        {
            Log(ThisModule, __LINE__,
                "打开设备: CDeviceVideo.Open(%s, %s, Video%d) fail, ErrCode: %s, Return: %s",
                lpVid, lpPid, m_clDevVideo.GetOpenVideoX(), m_clDevVideo.GetErrorStr(nRet),
                ConvertCode_Impl2Str(ConvertCode_DPErr2Impl(nRet)));
            m_nRetErrOLD[1] = nRet;
        }
        return ConvertCode_DPErr2Impl(nRet);
    }

    // 保存设备VIDPID
    MCPY_NOLEN(m_szDevVidPid[0], lpVid);
    MCPY_NOLEN(m_szDevVidPid[1], lpPid);

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    Log(ThisModule, __LINE__, "打开设备: CDeviceVideo.Open(%s, %s, video%d) Succ",
         lpVid, lpPid, m_clDevVideo.GetOpenVideoX());

    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::CloseDevice()
{
    if (m_bDevOpenOk == TRUE)
    {
        m_clDevVideo.Close();
    }

    m_bDevOpenOk = FALSE;

    m_stImageData.Clear();

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 设备是否Open
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
BOOL CDevImpl_JDY5001A0809::IsDeviceOpen()
{
    return ((m_bDevOpenOk == TRUE && m_clDevVideo.IsOpen() == TRUE) ? TRUE : FALSE);
}

/***************************************************************************
 * 功能: 复位设备
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::ResetDevice()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 取设备状态
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::GetDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nStat = m_clDevVideo.GetStatus();

    if (m_nRetErrOLD[2] != nStat)
    {
        Log(ThisModule, __LINE__,
            "取设备状态: m_clDevVideo.GetStatus(): %s -> %s",
            m_clDevVideo.GetErrorStr(m_nRetErrOLD[2]),
            m_clDevVideo.GetErrorStr(nStat));
        m_nRetErrOLD[2] = nStat;
    }

    switch(nStat)
    {
        case DP_RET_SUCC:       // 成功/存在/已打开/正常
            return DEV_OK;
        case DP_RET_NOTHAVE:    // 不存在
            return DEV_NOTFOUND;
        case DP_RET_NOTOPEN:    // 没有Open
            return DEV_NOTOPEN;
        case DP_RET_OFFLINE:    // 已Open但已断线
            return DEV_OFFLINE;
        default:
            return DEV_UNKNOWN;
    }
}

/***************************************************************************
 * 功能: 取摄像宽高
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::GetVideoCaptWH(INT &nWidth, INT &nHeight)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = DP_RET_SUCC;

    nRet = m_clDevVideo.GetVideoWH(nWidth, nHeight);
    if (nRet != DP_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "取摄像宽高: CDeviceVideo.GetVideoWH(%d, %d) fail, ErrCode: %s, Return: %s",
            nWidth, nHeight, m_clDevVideo.GetErrorStr(nRet),
            ConvertCode_Impl2Str(ConvertCode_DPErr2Impl(nRet)));
        return ConvertCode_DPErr2Impl(nRet);
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 设置摄像宽高
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::SetVideoCaptWH(INT nWidth, INT nHeight)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = DP_RET_SUCC;

    nRet = m_clDevVideo.SetVideoWH(nWidth, nHeight);
    if (nRet != DP_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "设置摄像宽高: CDeviceVideo.SetVideoWH(%d, %d) fail, ErrCode: %s, Return: %s",
            nWidth, nHeight, m_clDevVideo.GetErrorStr(nRet),
            ConvertCode_Impl2Str(ConvertCode_DPErr2Impl(nRet)));
        return ConvertCode_DPErr2Impl(nRet);
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 设置摄像模式
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::SetVideoCaptMode(STVIDEOPAMAR stVideoPar)
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    SetVideoCaptMode(VM_BRIGHT, stVideoPar.duBright);           // 亮度
    SetVideoCaptMode(VM_CONTRAST, stVideoPar.duContrast);       // 对比度
    SetVideoCaptMode(VM_SATURATION, stVideoPar.duSaturation);   // 饱和度
    SetVideoCaptMode(VM_HUE, stVideoPar.duHue);                 // 色调
    SetVideoCaptMode(VM_EXPOSURE, stVideoPar.duExposure);       // 曝光

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 获取摄像模式
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::GetVideoCaptMode(STVIDEOPAMAR &stVideoPar)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    stVideoPar.duWidth = GetVideoCaptMode(VM_WIDTH);            // 宽度
    stVideoPar.duHeight = GetVideoCaptMode(VM_HEIGHT);          // 高度
    stVideoPar.duFPS = GetVideoCaptMode(VM_FPS);                // 帧率(帧/秒)
    stVideoPar.duBright = GetVideoCaptMode(VM_BRIGHT);          // 亮度
    stVideoPar.duContrast = GetVideoCaptMode(VM_CONTRAST);      // 对比度
    stVideoPar.duSaturation = GetVideoCaptMode(VM_SATURATION);  // 饱和度
    stVideoPar.duHue = GetVideoCaptMode(VM_HUE);                // 色调
    stVideoPar.duExposure = GetVideoCaptMode(VM_EXPOSURE);      // 曝光

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 设置摄像模式
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::SetVideoCaptMode(EN_VIDEOMODE enVM, DOUBLE duData)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = DP_RET_SUCC;

    nRet = m_clDevVideo.SetVideoMode(enVM, duData);
    if (nRet != DP_RET_SUCC)
    {
        Log(ThisModule, __LINE__,
            "设置摄像模式: CDeviceVideo.SetVideoMode(%d, %f) fail, ErrCode: %s, Return: %s",
            enVM, duData, m_clDevVideo.GetErrorStr(nRet),
            ConvertCode_Impl2Str(ConvertCode_DPErr2Impl(nRet)));
        return ConvertCode_DPErr2Impl(nRet);
    }
}

/***************************************************************************
 * 功能: 获取摄像模式
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
DOUBLE CDevImpl_JDY5001A0809::GetVideoCaptMode(EN_VIDEOMODE enVM)
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    return m_clDevVideo.GetVideoMode(enVM);
}

/***************************************************************************
 * 功能: 取图像帧数据
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::GetVideoImage(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight, WORD wFlip)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    nRet = m_clDevVideo.GetVideoImage(lpImgData, nWidth, nHeight, wFlip);
    if (nRet != DP_RET_SUCC)
    {
        return ConvertCode_DPErr2Impl(nRet);
    }

    return IMP_SUCCESS;
}

/***************************************************************************
 * 功能: 保存图像文件
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDevImpl_JDY5001A0809::SaveImageFile(LPSTR lpFileName)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    nRet = m_clDevVideo.SaveImageFile(lpFileName);
    if (nRet != DP_RET_SUCC)
    {
        return ConvertCode_DPErr2Impl(nRet);
    }
    return IMP_SUCCESS;
}

//----------------------------------对外参数设置接口----------------------------------
// DeviceVideo错误码转换为Impl错误码
INT CDevImpl_JDY5001A0809::ConvertCode_DPErr2Impl(INT nErrCode)
{
#define CASE_DEVICEVIDEO_2_IMPL(DP, IMPL) \
    case DP: return IMPL;

    switch(nErrCode)
    {
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_SUCC, IMP_SUCCESS);                          // 成功/存在/已打开/正常
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_FAIL, IMP_ERR_UNKNOWN);                      // 失败/错误
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_INST_ERR, IMP_ERR_UNKNOWN);                  // 有关实例化失败
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_INPUT_INV, IMP_ERR_PARAM_INVALID);           // 无效的入参
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_NOTHAVE, IMP_ERR_NODEVICE);                  // 不存在
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_OPENFAIL, IMP_ERR_OPENFAIL);                 // Open错误
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_NOTOPEN, IMP_ERR_NOTOPEN);                   // 没有Open
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_OFFLINE, IMP_ERR_OFFLINE);                   // 已Open但已断线
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_GETIMG_FAIL, IMP_ERR_GET_IMGDATA_FAIL);      // 取图像数据失败
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_IMGDATA_INV, IMP_ERR_GET_IMGDATA_ISNULL);    // 图像数据无效
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_MEMAPPLY_FAIL, IMP_ERR_GET_IMGDATA_BUFFER);  // 内存申请失败
        CASE_DEVICEVIDEO_2_IMPL(DP_RET_IMG2FILE, IMP_ERR_SAVE_IMAGE_FILE);          // 图像数据保存到文件失败
        default: return IMP_ERR_UNKNOWN;
    }
}

// Impl错误码转换解释字符串
LPSTR CDevImpl_JDY5001A0809::ConvertCode_Impl2Str(INT nErrCode)
{
#define JDY5001A0809_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%d|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));

    if (CHK_ERR_ISDEF(nErrCode) == TRUE)
    {
        DEF_ConvertCode_Impl2Str(nErrCode, m_szErrStr);
    } else
    {
        switch(nErrCode)
        {
            JDY5001A0809_CASE_CODE_STR(IMP_SUCCESS, nErrCode, "成功")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_LOAD_LIB, nErrCode, "动态库加载错误")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_PARAM_INVALID, nErrCode, "参数无效")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_UNKNOWN, nErrCode, "未知错误")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_NOTOPEN, nErrCode, "设备未Open")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_OPENFAIL, nErrCode, "设备打开失败/未打开")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_NODEVICE, nErrCode, "未找到有效设备")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_OFFLINE, nErrCode, "设备断线")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_NODETECT, nErrCode, "未检测到活体")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_INTEREXEC, nErrCode, "接口执行错误")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_SET_VMODE, nErrCode, "摄像参数设置错误")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_SET_RESO, nErrCode, "分辨率设置失败")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_SHAREDMEM_RW, nErrCode, "共享内存读写失败")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_VIDPID_NOTFOUND, nErrCode, "VID/PID未找到")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_VIDEOIDX_NOTFOUND, nErrCode, "未找到摄像设备索引号")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_GET_IMGDATA_FAIL, nErrCode, "取帧图像数据失败")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_GET_IMGDATA_ISNULL, nErrCode, "取帧图像数据为空")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_GET_IMGDATA_BUFFER, nErrCode, "取帧图像数据空间异常")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_GET_IMGDATA_CHANGE, nErrCode, "帧图像数据转换失败")
            JDY5001A0809_CASE_CODE_STR(IMP_ERR_SAVE_IMAGE_FILE, nErrCode, "帧图像数据保存到文件失败")
            default :
                sprintf(m_szErrStr, "%d|%s", nErrCode, "未知Code");
                break;
        }
    }

    return (LPSTR)m_szErrStr;
}

// -------------------------------------- END --------------------------------------
