/***************************************************************
* 文件名称：DevPPR_MB2.cpp
* 文件描述：MB2存折打印模块功能处理接口封装
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年10月20日
* 文件版本：1.0.0.1
****************************************************************/

#include "DevCAM_ZLF1000A3.h"
#include "opencv2/opencv.hpp"

#include <unistd.h>
#include <qimage.h>
#include <sys/syscall.h>

// CAM 版本号
//BYTE    byDevVRTU[17] = {"DevCAM00000100"};

static const char *ThisFile = "DevCAM_ZLF1000A3.cpp";

//////////////////////////////////////////////////////////////////////////////
// 对外接口调用处理                                                            //
//////////////////////////////////////////////////////////////////////////////

CDevCAM_ZLF1000A3::CDevCAM_ZLF1000A3()
    : m_devImpl(LOG_NAME_DEV)
{
     SetLogFile(LOG_NAME_DEV, ThisFile);  // 设置日志文件名和错误发生的文件
     Init();
}

CDevCAM_ZLF1000A3::CDevCAM_ZLF1000A3(LPCSTR lpDevType)
    : m_devImpl(LOG_NAME_DEV, (LPSTR)lpDevType)
{
     SetLogFile(LOG_NAME_DEV, ThisFile, lpDevType);  // 设置日志文件名和错误发生的文件
     Init();
}

CDevCAM_ZLF1000A3::~CDevCAM_ZLF1000A3()
{
    m_threadDispRunStat = ENDING;
}

void CDevCAM_ZLF1000A3::Init()
{
    m_wCamType = CAM_TYPE_DOC;
    m_qSharedMemData = nullptr;
    memset(m_szSharedDataName, 0x00, sizeof(m_szSharedDataName));
    m_ulSharedDataSize = 0;
    m_threadDispRunStat = ENDING;

    m_nPid = (INT)getpid();
}

void CDevCAM_ZLF1000A3::Release()
{
    return;
}

// 打开设备
long CDevCAM_ZLF1000A3::Open(LPCSTR lpMode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 建立Cam连接(文档/人物)
    if (m_devImpl.IsDeviceOpen() != TRUE)
    {
        // Open前设定摄像类型
        if (lpMode != nullptr)
        {
            if (MCMP_IS0(lpMode, ZL_CAMTYPE_DOC))
            {
                m_wCamType = CAM_TYPE_DOC;
            } else
            if (MCMP_IS0(lpMode, ZL_CAMTYPE_PER))
            {
                m_wCamType = CAM_TYPE_PER;
            }
        }

        if ((nRet = m_devImpl.DeviceOpen(m_wCamType)) != IMP_SUCCESS)
        {
            return ConvertErrorCode(nRet);
        }
    }

    // 连接共享内存(摄像)
    if (bConnSharedMemory(m_szSharedDataName) != TRUE)
    {
        return ERR_OPEN_CAMER;
    }

    // 获取分辨率支持列表
    UINT unResoBuf[30][2];
    INT nResoCnt = m_devImpl.GetResolut(unResoBuf);
    if (nResoCnt > 0)
    {
        CHAR szResoPrt[1024] = { 0x00 };
        for (INT i = 0; i < nResoCnt; i ++)
        {
            sprintf(szResoPrt + strlen(szResoPrt), "%d*%d|",
                    unResoBuf[i][0], unResoBuf[i][1]);
        }
        Log(ThisFile, __LINE__, "设备支持的分辨率列表: %s", szResoPrt);
    } else
    {
        Log(ThisFile, __LINE__, "获取分辨率列表: GetResolut() = %d", nResoCnt);
    }

    return CAM_SUCCESS;
}

// 关闭设备
long CDevCAM_ZLF1000A3::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_threadDispRunStat = ENDING;
    m_devImpl.DeviceClose();

    return CAM_SUCCESS;
}

// 设备初始化
long CDevCAM_ZLF1000A3::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nRet = IMP_SUCCESS;

    // 关闭设备并重新打开
    m_devImpl.DeviceClose();
    if ((nRet = m_devImpl.DeviceOpen(m_wCamType)) != IMP_SUCCESS)
    {
        return ConvertErrorCode(nRet);
    }

    return CAM_SUCCESS;
}

// 获取设备信息
long CDevCAM_ZLF1000A3::GetDevInfo(char *pInfo)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return CAM_SUCCESS;
}

// 获取设备状态
long CDevCAM_ZLF1000A3::GetStatus(DEVCAMSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    INT nStat = DEVICE_OK;

    // 状态结构题初始化(OFFLINE)
    stStatus.fwDevice      = DEVICE_OFFLINE;
    for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        stStatus.fwMedia[i] = MEDIA_UNKNOWN;
    for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        stStatus.fwCameras[i] = STATUS_NOTSUPP;
    for (int i = 0; i < CAM_CAMERAS_SIZE; i ++)
        stStatus.usPictures[i] = CAM_ROOM;

    // 检查设备是否OPEN
    if (m_devImpl.IsDeviceOpen() != TRUE)
    {
        return ERR_OPEN_CAMER;
    }

    // 获取设备状态并检查
    nStat = m_devImpl.GetDevStatus();
    switch(nStat)
    {
        case DEVICE_OK:         // 启动且正在运行
        case DEVICE_OPENED:     // 设备已打开
            stStatus.fwDevice   = DEVICE_ONLINE;
            stStatus.fwMedia[1] = MEDIA_OK;
            stStatus.fwCameras[1] = STATUS_OK;
            break;
        case DEVICE_NOT_FOUND:   // 未找到设备
            stStatus.fwDevice   = DEVICE_NODEVICE;
            break;
        case DEVICE_CONNECTED:   // 设备已连接未打开
            stStatus.fwDevice   = DEVICE_OFFLINE;
            stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            stStatus.fwCameras[1] = STATUS_UNKNOWN;
            break;
        default:
            stStatus.fwDevice   = DEVICE_HWERROR;
            stStatus.fwMedia[1] = MEDIA_UNKNOWN;
            stStatus.fwCameras[1] = STATUS_UNKNOWN;
            break;
    }

    return CAM_SUCCESS;
}

// 命令取消
long CDevCAM_ZLF1000A3::Cancel()
{
    m_bCancel = TRUE;
}

// 打开窗口(窗口句柄，动作指示:1创建窗口/0销毁窗口, X/Y坐标,窗口宽高)
long CDevCAM_ZLF1000A3::Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;

    if (wAction == CAMERA_OPEN)
    {
        if (m_bDisplayOpenOK == TRUE)   // Display已打开窗口，直接return
        {
            return CAM_SUCCESS;
        }

        // 设置分辨率
        /*nRet = m_devImpl.SetResolut(wWidth, wHeight);
        if (nRet != IMP_SUCCESS)
        {
            Log(ThisFile, __LINE__, "设置分辨率: SetResolut(%d, %d) = %d, Fail, Return: %d",
                wWidth, wHeight, nRet, ConvertErrorCode(nRet));
            return ConvertErrorCode(nRet);
        }*/

        // 启动数据流获取线程
        if (m_threadDispDataGet.joinable() != true)
        {
            m_threadDispDataGet = std::thread(&CDevCAM_ZLF1000A3::ThreadDispGetData, this, wWidth, wHeight, 30);
            if (m_threadDispDataGet.joinable())
            {
                m_threadDispDataGet.detach();
            } else
            {
                Log(ThisFile, __LINE__, "创建数据流获取线程: Fail, Return: %d", ERR_PROCESS);
                return ERR_PROCESS;
            }
        } else
        {
            Log(ThisFile, __LINE__, "数据流获取线程占用中,无法创建, Return: %d", ERR_PROCESS);
            return ERR_PROCESS;
        }

        m_bDisplayOpenOK = TRUE;

    } else
    if (wAction == CAMERA_PAUSE)    // 暂停
    {
        if (m_bDisplayOpenOK != TRUE)   // Display未打开窗口，直接return
        {
            return CAM_SUCCESS;
        }
        m_threadDispRunStat = PAUSE;
    } else
    if (wAction == CAMERA_RESUME)    // 恢复
    {
        if (m_bDisplayOpenOK != TRUE)   // Display未打开窗口，直接return
        {
            return CAM_SUCCESS;
        }
        m_threadDispRunStat = RUNNING;
    } else
    {
        m_threadDispRunStat = ENDING;
        m_bDisplayOpenOK = FALSE;
    }

    return CAM_SUCCESS;
}

long CDevCAM_ZLF1000A3::TakePicture(WORD wCamera, LPCSTR lpData)
{
    return CAM_SUCCESS;
}

long CDevCAM_ZLF1000A3::TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue, DWORD dwTimeOut, WORD wCamara)
{
    THISMODULE(__FUNCTION__);

    INT nRet = IMP_SUCCESS;


    nRet = m_devImpl.SaveImage(lpFileName);
    if (nRet != IMP_SUCCESS)
    {
        Log(ThisFile, __LINE__, "摄像拍照: SaveImage(%s) = %d, Fail, Return: %d",
            lpFileName, nRet, ConvertErrorCode(nRet));
        return ConvertErrorCode(nRet);
    }

    return CAM_SUCCESS;
}

long CDevCAM_ZLF1000A3::SetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DATATYPE_INIT:
        {
            memcpy(m_szSharedDataName,
                    ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->szCamDataSMemName,
                   strlen(((LPST_CAM_DEV_INIT_PARAM)vInitPar)->szCamDataSMemName));
            m_ulSharedDataSize = ((LPST_CAM_DEV_INIT_PARAM)vInitPar)->ulCamDataSMemSize;
            break;
        }
        default:
            break;
    }

    return CAM_SUCCESS;
}

long CDevCAM_ZLF1000A3::GetData(void *vInitPar, WORD wDataType)
{
    switch(wDataType)
    {
        case DATATYPE_INIT:
            break;
        default:
            break;
    }

    return CAM_SUCCESS;
}

// 版本号(1DevCam版本/2固件版本/3其他)
void CDevCAM_ZLF1000A3::GetVersion(char* szVer, long lSize, ushort usType)
{
    if (usType == 1)
    {
        memcpy(szVer, byDevVRTU, strlen((char*)byDevVRTU) > lSize ? lSize : strlen((char*)byDevVRTU));
    }
    if (usType == 2)
    {

    }
    if (usType == 3)
    {

    }
}

// 创建/关联共享内存
BOOL CDevCAM_ZLF1000A3::bConnSharedMemory(LPSTR lpSharedName)
{

    m_qSharedMemData = new QSharedMemory(lpSharedName);
    if (m_qSharedMemData == nullptr)
    {
        Log(ThisFile, __LINE__, "bConnSharedMemory()->创建共享内存[%s]实例失败", lpSharedName);
        return FALSE;
    }
    if (m_qSharedMemData->attach(QSharedMemory::ReadWrite) != true)    // 读写方式关联
    {
        if (m_qSharedMemData->isAttached() != true)   // 判断共享内存的关联状态
        {
            Log(ThisFile, __LINE__, "bConnSharedMemory()->关联共享内存[%s]无法关联", lpSharedName);
            return FALSE;
        }
    }

    if (m_qSharedMemData->size() < 1)
    {
        Log(ThisFile, __LINE__, "bConnSharedMemory()->关联共享内存[%s] size[%s] < 1",
                          m_qSharedMemData->size(), lpSharedName);
        return FALSE;
    }

    return TRUE;
}

// 解除关联共享内存
void CDevCAM_ZLF1000A3::bDisConnSharedMemory()
{
    if (m_qSharedMemData != nullptr)
    {
        if (m_qSharedMemData->isAttached())   // 检测程序当前是否关联共享内存
            m_qSharedMemData->detach();      // 解除关联
        delete m_qSharedMemData;
        m_qSharedMemData = nullptr;
    }
}

// Display命令窗口显示数据流获取处理
void CDevCAM_ZLF1000A3::ThreadDispGetData(WORD wWidth, WORD wHeight, DWORD dwSleepTime)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    auto Data2SharedMemory = [&](LPBYTE lpSndBuf, ULONG ulSndBufLen)
    {
        // 图像数据写入共享内存
        if (m_qSharedMemData->isAttached())
        {
            if (m_qSharedMemData->lock() == true)
            {
                memcpy(m_qSharedMemData->data(), lpSndBuf, ulSndBufLen);
                m_qSharedMemData->unlock();
            }
        }
        return;
    };

    uchar szZeroBuf[6] = { 0x00 };      // 用于初始化共享内存数据
    uchar* ucDataBuf = nullptr;
    uchar* ucSendBuf = nullptr;
    LONG lTimeStamp = 0;
    ULONG ulDataBufLen = 0;//unWidth * unHeight * 4 + 1;
    ULONG ulSendBufLen = 0;
    INT nW = 0, nH = 0, nC = 4;
    QImage qImgBuffer;
    cv::Mat srcMat, dstMat;
    WORD wErrCode = 0;

    Log(ThisFile, __LINE__, "数据流线程启动[%ld/PID=%d]: Width = %d, Height = %d, SleepTime = %d",
         syscall(SYS_gettid), m_nPid, wWidth, wHeight, dwSleepTime);

    Data2SharedMemory(szZeroBuf, 5);    // 初始化共享内存数据


    m_threadDispRunStat = RUNNING;
    while (m_threadDispRunStat != ENDING) // 非结束
    {
        QCoreApplication::processEvents();

        usleep(1000 * dwSleepTime);  // 间隔获取时间(毫秒)

        if (m_threadDispRunStat == PAUSE)   // 暂停
        {
            continue;
        } else
        if (m_threadDispRunStat == ENDING)   // 结束
        {
            break;
        } else
        {
            // 写入共享内存数据空间申请
            if (ulSendBufLen != (wWidth * wHeight * nC + 5 + 1))
            {
                ulSendBufLen = wWidth * wHeight * nC + 5 + 1;
                if (ucSendBuf != nullptr)
                {
                    free(ucSendBuf);
                    ucSendBuf = nullptr;
                }
                ucSendBuf = (uchar *)malloc(sizeof(INT) * ulSendBufLen);
                if (ucSendBuf == nullptr)
                {
                    Log(ThisFile, __LINE__, "写入共享内存数据空间申请失败");
                    break;
                }
                sprintf((char*)ucSendBuf, "%c%c%c%c%c", wWidth / 255, wWidth % 255, wHeight / 255, wHeight % 255, nC);
            }
            //memset(ucSendBuf + 5, 0x00,  ulSendBufLen - 5);

            // 获取设备当前分辨率
            if (m_devImpl.GetVideoData(nullptr, 0, (LPINT)&nW, (LPINT)&nH, &lTimeStamp, TRUE) != IMP_SUCCESS)
            {
                if (wErrCode != 1)
                {
                    Log(ThisFile, __LINE__, "1: 获取设备当前分辨率失败");
                    wErrCode = 1;
                }
                continue;
            } else
            {
                if (wErrCode == 1)
                {
                    wErrCode = 0;
                }
            }


            // 设备当前分辨率(无效)
            if (nW < 1 || nH < 1)
            {
                if (wErrCode != 2)
                {
                    Log(ThisFile, __LINE__, "2: 获取设备当前分辨率[%d*%d]无效", nW, nH);
                    wErrCode = 2;
                }
                continue;
            } else
            {
                if (wErrCode == 2)
                {
                    wErrCode = 0;
                }
            }

            if (nW != wWidth || nH != wHeight)  // 设备当前分辨率与入参宽高不一致,申请数据空间
            {
                if (ulDataBufLen != (nW * nH * nC + 1))
                {
                    ulDataBufLen = nW * nH * nC + 1;
                    if (ucDataBuf != nullptr)
                    {
                        free(ucDataBuf);
                        ucDataBuf = nullptr;
                    }
                    ucDataBuf = (uchar *)malloc(sizeof(INT) * (ulDataBufLen));
                    if (ucDataBuf == nullptr)
                    {
                        if (wErrCode != 3)
                        {
                            Log(ThisFile, __LINE__, "3: 申请数据设备分辨率[%d*%d]指定数据空间失败", nW, nH);
                            wErrCode = 3;
                        }
                        break;
                    } else
                    {
                        if (wErrCode == 3)
                        {
                            wErrCode = 0;
                        }
                    }
                    if(!qImgBuffer.isNull())
                    {
                        free(qImgBuffer.bits());
                    }
                    qImgBuffer = QImage((uchar*)ucDataBuf, nW, nH, QImage::Format_ARGB32);
                }

                // 获取图像数据
                //memset(ucDataBuf, 0x00, ulDataBufLen);
                if (m_devImpl.GetVideoData((LPINT)(ucDataBuf), ulDataBufLen,
                                           (LPINT)&nW, (LPINT)&nH, &lTimeStamp, TRUE) != IMP_SUCCESS )
                {
                    if (wErrCode != 4)
                    {
                        Log(ThisFile, __LINE__, "4: 获取图像数据[%d*%d]失败", nW, nH);
                        wErrCode = 4;
                    }
                    continue;
                } else
                {
                    if (wErrCode == 4)
                    {
                        wErrCode = 0;
                    }
                }

                if (nW < 1 || nH < 1)
                {
                    if (wErrCode != 5)
                    {
                        Log(ThisFile, __LINE__, "5: 获取图像数据, 返回分辨率值[%d*%d]无效", nW, nH);
                        wErrCode = 5;
                    }
                    continue;
                } else
                {
                    if (wErrCode == 5)
                    {
                        wErrCode = 0;
                    }
                }

                // 图像数据转换
                srcMat = cv::Mat(qImgBuffer.height(), qImgBuffer.width(), CV_8UC4,
                               (void*)qImgBuffer.constBits(),  qImgBuffer.bytesPerLine());
                cv::resize(srcMat, dstMat, cv::Size(wWidth, wHeight));
                if (dstMat.cols * dstMat.rows * dstMat.channels() > (ulSendBufLen - 5 - 1))
                {
                    continue;
                }

                memcpy(ucSendBuf + 5, dstMat.data, dstMat.cols * dstMat.rows * dstMat.channels());
            } else
            {
                if (m_devImpl.GetVideoData((LPINT)(ucSendBuf + 5), ulSendBufLen - 5,
                                           (LPINT)&nW, (LPINT)&nH, &lTimeStamp, TRUE) != IMP_SUCCESS )
                {
                    if (wErrCode != 6)
                    {
                        Log(ThisFile, __LINE__, "6: 获取图像数据[%d*%d]失败", nW, nH);
                        wErrCode = 6;
                    }
                    continue;
                } else
                {
                    if (wErrCode == 6)
                    {
                        wErrCode = 0;
                    }
                }

                if (nW < 1 || nH < 1)
                {
                    if (wErrCode != 7)
                    {
                        Log(ThisFile, __LINE__, "7: 获取图像数据, 返回分辨率值[%d*%d]无效", nW, nH);
                        wErrCode = 7;
                    }
                    continue;
                } else
                {
                    if (wErrCode == 7)
                    {
                        wErrCode = 0;
                    }
                }
            }

            // 图像数据写入共享内存
            Data2SharedMemory(ucSendBuf, ulSendBufLen - 1);
        }
    }

    if (ucDataBuf != nullptr)
    {
        free(ucDataBuf);
        ucDataBuf = nullptr;
    }

    if (ucSendBuf != nullptr)
    {
        free(ucSendBuf);
        ucSendBuf = nullptr;
    }

    Log(ThisFile, __LINE__, "数据流线程结束[%ld/PID=%d]", syscall(SYS_gettid), m_nPid);
    return;
}

// 转换为IDevCAM返回码/错误码
INT CDevCAM_ZLF1000A3::ConvertErrorCode(INT nRet)
{
    switch(nRet)
    {
        // >0: Impl处理返回
        case IMP_SUCCESS:                   // 0:成功, ERR_SUCCESS/IMP_DEV_ERR_SUCCESS
            return CAM_SUCCESS;
        case IMP_ERR_LOAD_LIB:              // -1:动态库加载失败
            return ERR_OTHER;
        case IMP_ERR_PARAM_INVALID:         // -2:参数无效
            return IMP_ERR_DEV_PARAM;
        case IMP_ERR_DEV_NOTOPEN:           // 1:设备未打开
            return ERR_OPEN_CAMER;
        case IMP_ERR_DEV_GETVDATA:          // 2:获取视频/图像数据失败
            return ERR_OTHER;
        case IMP_ERR_DEV_PARAM:             // 3:SDK接口参数错误
            return IMP_ERR_DEV_PARAM;
        default:
            return ERR_OTHER;
    }
}

CHAR* CDevCAM_ZLF1000A3::ConvertErrCodeToStr(INT nRet)
{
    /*memset(m_szErrStr, 0x00, sizeof(m_szErrStr));
    switch(nRet)
    {
        case PTR_SUCCESS:
            sprintf(m_szErrStr, "%d|%s", nRet, "操作成功");
            return m_szErrStr;

        default:
            sprintf(m_szErrStr, "%d|%s", nRet, "未定义错误");
            return m_szErrStr;
    }*/
    return nullptr;
}


//////////////////////////////////////////////////////////////////////////






