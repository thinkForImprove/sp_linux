/***************************************************************
* 文件名称: DevImpl_JDY5001A0809.h
* 文件描述: 封装摄像模块底层指令,提供控制接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
****************************************************************/

#include "DevImpl_JDY5001A0809.h"

// open()必需头文件
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
// close()必需头文件
#include <unistd.h>
// ioctl()必需头文件
#include <sys/ioctl.h>

//#include <linux/videodev2.h>



static const char *ThisFile = "DevImpl_JDY5001A0809.cpp";

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

/************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::OpenDevice(LPSTR lpVid, LPSTR lpPid)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 通过Vid/Pid检索设备索引
    INT nIdx = CDevicePort::SearchVideoIdxFromVidPid(lpVid, lpPid);
    if (nIdx < 0)
    {
        return IMP_ERR_VIDEOIDX_NOTFOUND;
    }

    // cv::VideoCapture()打开设备
    nRet = m_cvVideoCapt.open(nIdx);
    if (nRet != true || m_cvVideoCapt.isOpened() != true)
    {
        return IMP_ERR_DEVICE_OPENFAIL;
    }

    // 保存设备VIDPID
    MCPY_NOLEN(m_szDevVidPid[0], lpVid);
    MCPY_NOLEN(m_szDevVidPid[1], lpPid);

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    memset(m_nRetErrOLD, 0, sizeof(m_nRetErrOLD));

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::CloseDevice()
{
    if (m_bDevOpenOk == TRUE)
    {
        m_cvVideoCapt.release();
    }

    m_bDevOpenOk = FALSE;

    m_stImageData.Clear();

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 设备是否Open
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
BOOL CDevImpl_JDY5001A0809::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

/************************************************************
 * 功能: 复位设备
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::ResetDevice()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 取设备状态
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::GetDeviceStatus()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    if (CDevicePort::SearchVideoIdxFromVidPid(m_szDevVidPid[0], m_szDevVidPid[1]) == DP_RET_NOTHAVE)
    {
        /*if (m_bDevOpenOk == TRUE)
        {
            CloseDevice();
        }*/
        return DEV_NOTFOUND;
    }

    if (m_bDevOpenOk == TRUE)
    {
        if (m_cvVideoCapt.isOpened() != true)
        {
            return DEV_OFFLINE;
        }
        return DEV_OK;
    } else
    {
        return DEV_NOTOPEN;
    }

    return DEV_OK;
}

/************************************************************
 * 功能: 设置视频捕获宽高
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::GetVideoCaptWH(INT &nWidth, INT &nHeight)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    Mat cvReadMat;

    // 检查设备状态
    CHECK_DEVICE_STAT(GetDeviceStatus());

    if (m_cvVideoCapt.read(cvReadMat) != true)
    {
        if (m_cvVideoCapt.read(cvReadMat) != true)
        {
            if (m_cvVideoCapt.read(cvReadMat) != true)
            {
                return IMP_ERR_UNKNOWN;
            }
        }
    }

    if (m_cvMatImg.empty())
    {
        return IMP_ERR_UNKNOWN;
    }

    nWidth = cvReadMat.cols;
    nHeight = cvReadMat.rows;

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 设置视频捕获宽高
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::SetVideoCaptWH(INT nWidth, INT nHeight)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    Mat cvReadMat;

    // 检查设备状态
    CHECK_DEVICE_STAT(GetDeviceStatus());

    m_cvVideoCapt.set(CV_CAP_PROP_FRAME_WIDTH, nWidth);
    m_cvVideoCapt.set(CV_CAP_PROP_FRAME_HEIGHT, nHeight);

    if (m_cvVideoCapt.read(cvReadMat) != true)
    {
        if (m_cvVideoCapt.read(cvReadMat) != true)
        {
            if (m_cvVideoCapt.read(cvReadMat) != true)
            {
                return IMP_ERR_SET_RESO;
            }
        }
    }

    if (m_cvMatImg.empty())
    {
        return IMP_ERR_SET_RESO;
    }

    if (cvReadMat.cols != nWidth ||
        cvReadMat.rows != nHeight)
    {
        return IMP_ERR_SET_RESO;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 设置摄像模式
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
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

/************************************************************
 * 功能: 获取摄像模式
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
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

/************************************************************
 * 功能: 设置摄像模式
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::SetVideoCaptMode(EN_VIDEOMODE enVM, DOUBLE duData)
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    bool bRet = FALSE;

    if (duData == 99999999.00)
    {
        return IMP_ERR_PARAM_INVALID;
    }

    switch(enVM)
    {
        case VM_WIDTH:          // 宽度
        {
            bRet = m_cvVideoCapt.set(CV_CAP_PROP_FRAME_WIDTH, duData);
            break;
        }
        case VM_HEIGHT:         // 高度
        {
            bRet = m_cvVideoCapt.set(CV_CAP_PROP_FRAME_HEIGHT, duData);
            break;
        }
        case VM_FPS:            // 帧率(帧/秒)
        {
            bRet = m_cvVideoCapt.set(CV_CAP_PROP_FPS, duData);
            break;
        }
        case VM_BRIGHT:         // 亮度
        {
            bRet = m_cvVideoCapt.set(CV_CAP_PROP_BRIGHTNESS, duData);
            break;
        }
        case VM_CONTRAST:       // 对比度
        {
            bRet = m_cvVideoCapt.set(CV_CAP_PROP_CONTRAST, duData);
            break;
        }
        case VM_SATURATION:     // 饱和度
        {
            bRet = m_cvVideoCapt.set(CV_CAP_PROP_SATURATION, duData);
            break;
        }
        case VM_HUE:            // 色调
        {
            bRet = m_cvVideoCapt.set(CV_CAP_PROP_HUE, duData);
            break;
        }
        case VM_EXPOSURE:       // 曝光
        {
            bRet = m_cvVideoCapt.set(CV_CAP_PROP_EXPOSURE, duData);
            break;
        }
    }
    return bRet == true ? IMP_SUCCESS : IMP_ERR_UNKNOWN;
}

/************************************************************
 * 功能: 获取摄像模式
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
DOUBLE CDevImpl_JDY5001A0809::GetVideoCaptMode(EN_VIDEOMODE enVM)
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    switch(enVM)
    {
        case VM_WIDTH:          // 宽度
            return m_cvVideoCapt.get(CV_CAP_PROP_FRAME_WIDTH);
        case VM_HEIGHT:         // 高度
            return m_cvVideoCapt.get(CV_CAP_PROP_FRAME_HEIGHT);
        case VM_FPS:            // 帧率(帧/秒)
            return m_cvVideoCapt.get(CV_CAP_PROP_FPS);
        case VM_BRIGHT:         // 亮度
            return m_cvVideoCapt.get(CV_CAP_PROP_BRIGHTNESS);
        case VM_CONTRAST:       // 对比度
            return m_cvVideoCapt.get(CV_CAP_PROP_CONTRAST);
        case VM_SATURATION:     // 饱和度
            return m_cvVideoCapt.get(CV_CAP_PROP_SATURATION);
        case VM_HUE:            // 色调
            return m_cvVideoCapt.get(CV_CAP_PROP_HUE);
        case VM_EXPOSURE:       // 曝光
            return m_cvVideoCapt.get(CV_CAP_PROP_EXPOSURE);
    }

    return (DOUBLE)0.0;
}

/************************************************************
 * 功能: 取图像帧数据
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::GetVideoImage(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight, WORD wFlip)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    Mat cvMatImg_RGB;
    INT nBuffSize = 0;
    BOOL bIsChange = FALSE;

    nRet = m_cvVideoCapt.read(m_cvMatImg);
    if (nRet != true)
    {        
        CHECK_DEVICE_STAT(GetDeviceStatus());   // 检查设备状态
        return IMP_ERR_GET_IMGDATA_FAIL;
    }

    if (m_cvMatImg.empty())
    {
        return IMP_ERR_GET_IMGDATA_ISNULL;
    }

    // 镜像转换
    if (wFlip == EN_CLIP_LR)    // 左右转换
    {
        flip(m_cvMatImg, m_cvMatImg, 1);
    } else
    if (wFlip == EN_CLIP_UD)    // 上下转换
    {
        flip(m_cvMatImg, m_cvMatImg, 0);
    } else
    if (wFlip == EN_CLIP_UDLR)  // 上下左右转换
    {
        flip(m_cvMatImg, m_cvMatImg, -1);
    }

    // 通道转换: BGR->RGB
    cvtColor(m_cvMatImg, cvMatImg_RGB, cv::COLOR_BGR2RGB);    

    // 图像数据转换
    if (nWidth > 0 && nHeight > 0)
    {
        cv::resize(cvMatImg_RGB, cvMatImg_RGB, cv::Size(nWidth, nHeight));
        bIsChange = TRUE;
    }

    if (cvMatImg_RGB.cols != m_stImageData.nWidth ||
        cvMatImg_RGB.rows != m_stImageData.nHeight ||
        cvMatImg_RGB.channels() != m_stImageData.nFormat)
    {
        m_stImageData.Clear();

        nBuffSize = cvMatImg_RGB.cols * cvMatImg_RGB.rows * cvMatImg_RGB.channels() + 1 + 5;

        m_stImageData.ucImgData = (UCHAR*)malloc(sizeof(UCHAR) * nBuffSize);
        if (m_stImageData.ucImgData == nullptr)
        {
            return IMP_ERR_GET_IMGDATA_BUFFER;
        }
        m_stImageData.nWidth = cvMatImg_RGB.cols;
        m_stImageData.nHeight = cvMatImg_RGB.rows;
        m_stImageData.nFormat = cvMatImg_RGB.channels();
        memset(m_stImageData.ucImgData, 0x00, nBuffSize);
    } else
    {
        nBuffSize = m_stImageData.ulImagDataLen + 1;
    }

    sprintf((CHAR*)m_stImageData.ucImgData, "%c%c%c%c%c",
            m_stImageData.nWidth / 255, m_stImageData.nWidth % 255,
            m_stImageData.nHeight / 255, m_stImageData.nHeight % 255,
            m_stImageData.nFormat);
    m_stImageData.ulImagDataLen = nBuffSize - 1;
    MCPY_LEN(m_stImageData.ucImgData + 5, cvMatImg_RGB.data, nBuffSize - 1 - 5);

    lpImgData->nWidth = m_stImageData.nWidth;
    lpImgData->nHeight = m_stImageData.nHeight;
    lpImgData->nFormat = m_stImageData.nFormat;
    lpImgData->ulImagDataLen = m_stImageData.ulImagDataLen;
    lpImgData->ucImgData = m_stImageData.ucImgData;

    lpImgData->nOtherParam[0] = 0;
    if (bIsChange == TRUE)
    {
        lpImgData->nOtherParam[0] = 1;
        lpImgData->nOtherParam[1] = m_cvMatImg.cols;
        lpImgData->nOtherParam[2] = m_cvMatImg.rows;
    }

    return IMP_SUCCESS;
}

/************************************************************
 * 功能: 保存图像文件
 * 参数: 无
 * 返回值: 参考错误码
************************************************************/
INT CDevImpl_JDY5001A0809::SaveImageFile(LPSTR lpFileName)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    INT nRet = IMP_SUCCESS;

    // 检查设备状态
    CHECK_DEVICE_STAT(GetDeviceStatus());

    // 颜色顺序转换: BGR->RGB
    Mat cvMatImg;
    cvtColor(m_cvMatImg, cvMatImg, cv::COLOR_BGR2RGB);

    nRet = imwrite(lpFileName, m_cvMatImg);
    if (nRet != true)
    {
        return IMP_ERR_SAVE_IMAGE_FILE;
    }

    return IMP_SUCCESS;
}

//----------------------------------对外参数设置接口----------------------------------
// Impl错误码转换解释字符串
LPSTR CDevImpl_JDY5001A0809::ConvertCode_Impl2Str(INT nErrCode)
{
#define JDY5001A0809_CASE_CODE_STR(IMP, CODE, STR) \
    case IMP: \
        sprintf(m_szErrStr, "%d|%s", CODE, STR); \
        return m_szErrStr;

    memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nErrCode)
    {
        JDY5001A0809_CASE_CODE_STR(IMP_SUCCESS, nErrCode, "成功")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_LOAD_LIB, nErrCode, "动态库加载错误")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_PARAM_INVALID, nErrCode, "参数无效")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_UNKNOWN, nErrCode, "未知错误")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_DEVICE_NOTFOUND, nErrCode, "设备未找到")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_VIDPID_NOTFOUND, nErrCode, "VID/PID未找到")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_VIDEOIDX_NOTFOUND, nErrCode, "未找到摄像设备索引号")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_DEVICE_OPENFAIL, nErrCode, "设备打开失败/未打开")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_DEVICE_OFFLINE, nErrCode, "设备断线")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_GET_IMGDATA_FAIL, nErrCode, "取帧图像数据失败")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_GET_IMGDATA_ISNULL, nErrCode, "取帧图像数据为空")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_GET_IMGDATA_BUFFER, nErrCode, "取帧图像数据空间异常")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_GET_IMGDATA_CHANGE, nErrCode, "帧图像数据转换失败")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_SAVE_IMAGE_FILE, nErrCode, "帧图像数据保存到文件失败")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_SET_VMODE, nErrCode, "摄像参数设置错误")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_SET_RESO, nErrCode, "分辨率设置失败")
        JDY5001A0809_CASE_CODE_STR(IMP_ERR_SHAREDMEM_RW, nErrCode, "共享内存读写失败")
        default :
            sprintf(m_szErrStr,  "%d|%s", nErrCode, "未知Code");
            break;
    }

    return (LPSTR)m_szErrStr;
}

