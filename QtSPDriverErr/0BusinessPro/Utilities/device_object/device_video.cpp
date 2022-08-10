/********************************************************************************
* 文件名称: device_video.cpp
* 文件描述: 定义静态库的函数(摄像设备处理相关)
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
*********************************************************************************/

#include "framework.h"
#include "device_object.h"

//-----------------------------构造/析构/初始化-----------------------------
CDeviceVideo::CDeviceVideo()
{
    m_bDevOpenOk = FALSE;                           // 设备是否Open
    MSET_0(m_szDevVidPid);                          // 保存设备VIDPID
    MSET_0(m_szErrorStr);
    m_nErrorOLD = DP_RET_SUCC;
    m_nVideoX = -1;                                 // Open成功的Video序
}

CDeviceVideo::~CDeviceVideo()
{
    Close();
}

/***************************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::Open(INT nVideoX)
{
    INT nRet = DP_RET_SUCC;

    Close();

    // cv::VideoCapture()打开设备
    nRet = m_cvVideoCapt.open(nVideoX);
    if (nRet != true || m_cvVideoCapt.isOpened() != true)
    {
        return DP_RET_OPENFAIL;
    }

    m_nVideoX = nVideoX;

    return DP_RET_SUCC;
}

/***************************************************************************
 * 功能: 打开设备
 * 参数: lpMode 入参
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::Open(LPSTR lpVid, LPSTR lpPid)
{
    INT nRet = DP_RET_SUCC;

    Close();

    // 通过Vid/Pid检索设备索引
    m_nVideoX = CDevicePort::SearchVideoIdxFromVidPid(lpVid, lpPid);
    if (m_nVideoX < 0)
    {
        return DP_RET_NOTHAVE;
    }

    // cv::VideoCapture()打开设备
    nRet = m_cvVideoCapt.open(m_nVideoX);
    if (nRet != true || m_cvVideoCapt.isOpened() != true)
    {
        return DP_RET_OPENFAIL;
    }

    // 保存设备VIDPID
    MCPY_NOLEN(m_szDevVidPid[0], lpVid);
    MCPY_NOLEN(m_szDevVidPid[1], lpPid);

    // 设备Open标记=T
    m_bDevOpenOk = TRUE;

    return DP_RET_SUCC;
}

/***************************************************************************
 * 功能: 关闭设备
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::Close()
{
    if (m_bDevOpenOk == TRUE || m_cvVideoCapt.isOpened() == true)
    {
        m_cvVideoCapt.release();
    }

    m_nVideoX = -1;

    m_bDevOpenOk = FALSE;

    m_stImageData.Clear();

    return DP_RET_SUCC;
}

/***************************************************************************
 * 功能: 设备是否Open
 * 参数: 无
 * 返回值: T/F
***************************************************************************/
BOOL CDeviceVideo::IsOpen()
{
    return ((m_bDevOpenOk == TRUE && m_cvVideoCapt.isOpened() == true) ? TRUE : FALSE);
}

/***************************************************************************
 * 功能: 复位设备
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::Reset()
{
    return DP_RET_SUCC;
}

/***************************************************************************
 * 功能: 取设备状态
 * 参数: 无
 * 返回值: 参考错误码(DP_RET_NOTHAVE/DP_RET_OFFLINE/DP_RET_SUCC/DP_RET_NOTOPEN)
***************************************************************************/
INT CDeviceVideo::GetStatus()
{
    if (CDevicePort::SearchVideoIdxFromVidPid(m_szDevVidPid[0], m_szDevVidPid[1]) == DP_RET_NOTHAVE)
    {
        return DP_RET_NOTHAVE;
    }

    if (m_bDevOpenOk == TRUE)
    {
        if (m_cvVideoCapt.isOpened() != true)
        {
            return DP_RET_OFFLINE;
        }
        return DP_RET_SUCC;
    } else
    {
        return DP_RET_NOTOPEN;
    }

    return DP_RET_SUCC;
}

/***************************************************************************
 * 功能: 取摄像宽高
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::GetVideoWH(INT &nWidth, INT &nHeight)
{
    INT nRet = DP_RET_SUCC;

    Mat cvReadMat;

    // 检查设备状态
    nRet = GetStatus();
    if (nRet != DP_RET_SUCC)
    {
        return nRet;
    }

    nWidth = (INT)m_cvVideoCapt.get(CV_CAP_PROP_FRAME_WIDTH);
    nHeight = (INT)m_cvVideoCapt.get(CV_CAP_PROP_FRAME_HEIGHT);

    /*if (m_cvVideoCapt.read(cvReadMat) != true)
    {
        return DP_RET_FAIL;
    }

    m_cvVideoCapt.read(cvReadMat);

    if (cvReadMat.empty())
    {
        return DP_RET_FAIL;
    }

    nWidth = cvReadMat.cols;
    nHeight = cvReadMat.rows;*/

    return DP_RET_SUCC;
}

/***************************************************************************
 * 功能: 设置视频捕获宽高
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::SetVideoWH(INT nWidth, INT nHeight)
{
    INT nRet = DP_RET_SUCC;

    Mat cvReadMat;

    // 检查设备状态
    nRet = GetStatus();
    if (nRet != DP_RET_SUCC)
    {
        return nRet;
    }

    m_cvVideoCapt.set(CV_CAP_PROP_FRAME_WIDTH, nWidth);
    m_cvVideoCapt.set(CV_CAP_PROP_FRAME_HEIGHT, nHeight);

    /*if (m_cvVideoCapt.read(cvReadMat) != true)
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
    }*/

    return DP_RET_SUCC;
}

/***************************************************************************
 * 功能: 设置摄像模式
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::SetVideoMode(EN_VIDEOMODE enVM, DOUBLE duData)
{
    BOOL bRet = FALSE;

    if (duData == 99999999.00)
    {
        return DP_RET_INPUT_INV;
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

    return bRet == TRUE ? DP_RET_SUCC : DP_RET_FAIL;
}

/***************************************************************************
 * 功能: 获取摄像模式
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
DOUBLE CDeviceVideo::GetVideoMode(EN_VIDEOMODE enVM)
{
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

/***************************************************************************
 * 功能: 取图像帧数据
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::GetVideoImage(LPSTIMAGEDATA lpImgData, INT nWidth, INT nHeight, WORD wFlip)
{
    AutoMutex(m_cMutex);

    INT nRet = DP_RET_SUCC;

    Mat cvMatImg_RGB;
    INT nBuffSize = 0;
    BOOL bIsChange = FALSE;

    // 检查设备状态
    nRet = GetStatus();
    if (nRet != DP_RET_SUCC)
    {
        return nRet;
    }

    // 取图像帧数据
    nRet = m_cvVideoCapt.read(m_cvMatImg);
    if (nRet != true)
    {
        return DP_RET_GETIMG_FAIL;
    }

    if (m_cvMatImg.empty())
    {
        return DP_RET_IMGDATA_INV;
    }

    // 镜像转换
    if (wFlip == 1)    // 左右转换
    {
        flip(m_cvMatImg, m_cvMatImg, 1);
    } else
    if (wFlip == 2)    // 上下转换
    {
        flip(m_cvMatImg, m_cvMatImg, 0);
    } else
    if (wFlip == 3)  // 上下左右转换
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
            return DP_RET_MEMAPPLY_FAIL;
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

    return DP_RET_SUCC;
}

/***************************************************************************
 * 功能: 保存图像文件
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::SaveImageFile(LPSTR lpFileName)
{
    INT nRet = DP_RET_SUCC;

    // 检查设备状态
    nRet = GetStatus();
    if (nRet != DP_RET_SUCC)
    {
        return nRet;
    }
    // 颜色顺序转换: BGR->RGB
    Mat cvMatImg;
    cvtColor(m_cvMatImg, cvMatImg, cv::COLOR_BGR2RGB);

    nRet = imwrite(lpFileName, m_cvMatImg);
    if (nRet != true)
    {
        return DP_RET_IMG2FILE;
    }

    return DP_RET_SUCC;
}

/***************************************************************************
 * 功能: 取Open成功的Video序号
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
INT CDeviceVideo::GetOpenVideoX()
{
    return m_nVideoX;
}

/***************************************************************************
 * 功能: 取错误码中文解析(最近一次错误)
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
LPSTR CDeviceVideo::GetErrorStr()
{
    return GetErrorStr(m_nErrorOLD);
}

/***************************************************************************
 * 功能: 取错误码中文解析(指定错误值)
 * 参数: 无
 * 返回值: 参考错误码
***************************************************************************/
LPSTR CDeviceVideo::GetErrorStr(INT nErrNo)
{
#define CASE_ERROR_STR(ERR, NO, STR) \
    case ERR: \
        sprintf(m_szErrorStr, "%d|%s", NO, STR); \
        return m_szErrorStr;

    memset(m_szErrorStr, 0x00, sizeof(m_szErrorStr));

    switch(nErrNo)
    {
        CASE_ERROR_STR(DP_RET_SUCC, nErrNo, "成功/存在/已打开/正常")
        CASE_ERROR_STR(DP_RET_FAIL, nErrNo, "失败/错误")
        CASE_ERROR_STR(DP_RET_INST_ERR, nErrNo, "有关实例化失败")
        CASE_ERROR_STR(DP_RET_INPUT_INV, nErrNo, "无效的入参")
        CASE_ERROR_STR(DP_RET_NOTHAVE, nErrNo, "不存在")
        CASE_ERROR_STR(DP_RET_OPENFAIL, nErrNo, "Open错误")
        CASE_ERROR_STR(DP_RET_NOTOPEN, nErrNo, "没有Open")
        CASE_ERROR_STR(DP_RET_OFFLINE, nErrNo, "已Open但已断线")
        CASE_ERROR_STR(DP_RET_GETIMG_FAIL, nErrNo, "取图像数据失败")
        CASE_ERROR_STR(DP_RET_IMGDATA_INV, nErrNo, "图像数据无效")
        CASE_ERROR_STR(DP_RET_MEMAPPLY_FAIL, nErrNo, "内存申请失败")
        CASE_ERROR_STR(DP_RET_IMG2FILE, nErrNo, "图像数据保存到文件失败")
        default:
            sprintf(m_szErrorStr, "%d|未知错误", nErrNo);
            return m_szErrorStr;
    }
}

// -------------------------------------- END --------------------------------------

