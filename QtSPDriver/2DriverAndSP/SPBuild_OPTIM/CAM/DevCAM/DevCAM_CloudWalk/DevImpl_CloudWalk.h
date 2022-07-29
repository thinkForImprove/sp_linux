/***************************************************************************
* 文件名称: DevImpl_CloudWalk.h
* 文件描述: 封装摄像模块底层指令,提供控制接口 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
***************************************************************************/
#pragma once

#ifndef DEVIMPL_CLOUDWALK_H
#define DEVIMPL_CLOUDWALK_H

#include <string>
#include "opencv2/opencv.hpp"
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtTypeInclude.h"
#include "QtDLLLoader.h"
#include "cloudwalk.h"
#include "../DevCAM_DEF/DevImpl_DEF.h"

using namespace std;
using namespace cv;

/***************************************************************************
// 返回值/错误码　宏定义 (0~-300通用定义, 见DevImpl_DEF.h)
***************************************************************************/
// 设备返回错误码
#define IMP_ERR_DEV_SUCCESS             75002000    // 成功
#define	IMP_ERR_DEV_75002001            75002001    // 输入参数错误
#define IMP_ERR_DEV_75002002            75002002    // 相机不存在
#define IMP_ERR_DEV_75002003            75002003    // 相机打开失败
#define IMP_ERR_DEV_75002004            75002004    // 相机关闭失败
#define IMP_ERR_DEV_75002005            75002005    // 非云从相机
#define IMP_ERR_DEV_75002006            75002006    // 读取芯片授权码失败
#define IMP_ERR_DEV_75002007            75002007    // 创建相机句柄失败
#define IMP_ERR_DEV_75002008            75002008    // 相机句柄为空
#define IMP_ERR_DEV_75002009            75002009    // 不支持的相机类型
#define IMP_ERR_DEV_75002010            75002010    // 参数指针为空
#define IMP_ERR_DEV_75002011            75002011    // 成像参数读取失败
#define IMP_ERR_DEV_75002012            75002012    // 成像参数设置失败
#define IMP_ERR_DEV_75002013            75002013    // 相机参数读取失败
#define IMP_ERR_DEV_75002014            75002014    // 相机参数设置失败
#define IMP_ERR_DEV_75002015            75002015    // 枚举相机设备失败
#define IMP_ERR_DEV_75002016            75002016    // 枚举相机分辨率失败
#define IMP_ERR_DEV_75002017            75002017    // 相机运行中
#define IMP_ERR_DEV_75002018            75002018    // 操作未知的错误
#define IMP_ERR_DEV_75002019            75002019    // 设置相机休眠失败
#define IMP_ERR_DEV_75002020            75002020    // 设置相机唤醒失败
#define IMP_ERR_DEV_75002021            75002021    // 相机设备断开
#define IMP_ERR_DEV_75000108            75000108    // 输入参数错误
#define IMP_ERR_DEV_75000109            75000109    // 无效的句柄
#define IMP_ERR_DEV_75000110            75000110    // 读取芯片授权码失败
#define IMP_ERR_DEV_75000111            75000111    // 检测器未初始化
#define IMP_ERR_DEV_75000112            75000112    // 未知错误catch
#define IMP_ERR_DEV_75000113            75000113    // 设置人脸检测回调函数错误
#define IMP_ERR_DEV_75000114            75000114    // 设置活体检测回调函数错误
#define IMP_ERR_DEV_75000115            75000115    // 设置日志信息回调函数错误
#define IMP_ERR_DEV_75000116            75000116    // 抠取人脸图像错误
#define IMP_ERR_DEV_75000117            75000117    // 创建com服务错误
#define IMP_ERR_DEV_75000118            75000118    // 重复创建检测器
#define IMP_ERR_DEV_75000119            75000119    // base64解码错误
#define IMP_ERR_DEV_75000120            75000120    // 检测到换脸
#define IMP_ERR_DEV_75000121            75000121    // 创建检测器错误
#define IMP_ERR_DEV_75000123            75000123    // 检测到多张人脸
#define IMP_ERR_DEV_26121000            26121000    // 成功or合法
#define IMP_ERR_DEV_26121001            26121001    // 空图像
#define IMP_ERR_DEV_26121002            26121002    // 图像格式不支持
#define IMP_ERR_DEV_26121003            26121003    // 没有人脸
#define IMP_ERR_DEV_26121004            26121004    // ROI设置失败
#define IMP_ERR_DEV_26121005            26121005    // 最小最大人脸设置失败
#define IMP_ERR_DEV_26121006            26121006    // 数据范围错误
#define IMP_ERR_DEV_26121007            26121007    // 未授权
#define IMP_ERR_DEV_26121008            26121008    // 尚未初始化
#define IMP_ERR_DEV_26121009            26121009    // 加载检测模型失败
#define IMP_ERR_DEV_26121010            26121010    // 加载关键点模型失败
#define IMP_ERR_DEV_26121011            26121011    // 加载质量评估模型失败
#define IMP_ERR_DEV_26121012            26121012    // 加载活体检测模型失败
#define IMP_ERR_DEV_26121013            26121013    // 检测失败
#define IMP_ERR_DEV_26121014            26121014    // 提取关键点失败
#define IMP_ERR_DEV_26121015            26121015    // 对齐人脸失败
#define IMP_ERR_DEV_26121016            26121016    // 质量评估失败
#define IMP_ERR_DEV_26121017            26121017    // 活体检测错误
#define IMP_ERR_DEV_26121018            26121018    // 时间戳不匹配
#define IMP_ERR_DEV_26121019            26121019    // 获取检测参数失败
#define IMP_ERR_DEV_26121020            26121020    // 设置检测参数失败
#define IMP_ERR_DEV_26121021            26121021    // 获取版本信息失败
#define IMP_ERR_DEV_26121022            26121022    // 删除文件或文件夹失败
#define IMP_ERR_DEV_26121030            26121030    // 人脸检测默认值
#define IMP_ERR_DEV_26121031            26121031    // color图像通过检测
#define IMP_ERR_DEV_26121032            26121032    // 人脸距离太近
#define IMP_ERR_DEV_26121033            26121033    // 人脸距离太远
#define IMP_ERR_DEV_26121034            26121034    // 人脸角度不满足要求
#define IMP_ERR_DEV_26121035            26121035    // 人脸清晰度不满足要求
#define IMP_ERR_DEV_26121036            26121036    // 检测到闭眼，仅在设置参数时eyeopen为true且检测到闭眼
#define IMP_ERR_DEV_26121037            26121037    // 检测到张嘴，仅在设置参数时mouthopen为true且检测到多人时返回
#define IMP_ERR_DEV_26121038            26121038    // 检测到人脸过亮，仅在设置参数时brightnessexc为true且人脸过亮时返回
#define IMP_ERR_DEV_26121039            26121039    // 检测到人脸过暗，仅在设置参数时brightnessins为true且人脸过暗时返回
#define IMP_ERR_DEV_26121040            26121040    // 检测到人脸置信度过低，仅在设置参数时confidence为true且人脸置信度过低时返回
#define IMP_ERR_DEV_26121041            26121041    // 检测到人脸存在遮挡，仅在设置参数时occlusion为true且检测到遮挡时返回
#define IMP_ERR_DEV_26121042            26121042    // 检测到黑框眼镜，仅在设置参数时blackspec为true且检测到黑框眼镜时返回
#define IMP_ERR_DEV_26121043            26121043    // 检测到墨镜，仅在设置参数时sunglass为true且检测到墨镜时返回
#define IMP_ERR_DEV_26121044            26121044    // 检测到口罩,仅在设置参数时proceduremask>-1且检测到口罩时返回
#define IMP_ERR_DEV_26121080            26121080    // 活体检测默认值
#define IMP_ERR_DEV_26121081            26121081    // 活体
#define IMP_ERR_DEV_26121082            26121082    // 非活体
#define IMP_ERR_DEV_26121083            26121083    // 人脸肤色检测未通过
#define IMP_ERR_DEV_26121084            26121084    // 可见光和红外人脸不匹配
#define IMP_ERR_DEV_26121085            26121085    // 红外输入没有人脸
#define IMP_ERR_DEV_26121086            26121086    // 可见光输入没有人脸


/***************************************************************************
// 无分类　宏定义
***************************************************************************/
#define LOG_NAME                            "DevImpl_CloudWalk.log"     // 缺省日志名
#define DLL_DEVLIB_NAME                     "libcwlivdetengine.so"      // 缺省动态库名


// 加载动态库接口
#define LOAD_LIBINFO_FUNC(LPFUNC, FUNC, FUNC2) \
    FUNC = (LPFUNC)m_LoadLibrary.resolve(FUNC2); \
    if(!FUNC) {   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

// 活检获取图像类型
#define GET_IMG_ROOM                        0x01    // 全景
#define GET_IMG_PERSON                      0x02    // 人脸
#define GET_IMG_NIS                         0x04    // 红外

// 云从摄像初始化参数结构体
typedef
struct st_Detect_Init_Param
{
    SHORT nModelMode;                           // 模型加载方式, 0:文件加载 1: 内存加载
    SHORT nLivenessMode;                        // 活体类型, 0:不活检 1: 红外活体 2:结构光活体
    SHORT nLicenseType;                         // 授权类型, 1:芯片授权 2：hasp授权 3:临时授权 4:云从相机绑定授权
    CHAR  szConfigFile[MAX_PATH];               // 算法矩阵文件,可以不写，使用默认
    CHAR  szFaceDetectFile[MAX_PATH];           // 人脸检测模型,可以不写，使用默认
    CHAR  szKeyPointDetectFile[MAX_PATH];       // 关键点检测模型,可以不写，使用默认
    CHAR  szKeyPointTrackFile[MAX_PATH];        // 关键点跟踪模型,可以不写，使用默认
    CHAR  szFaceQualityFile[MAX_PATH];          // 人脸质量评估模型,可以不写，使用默认
    CHAR  szFaceLivenessFile[MAX_PATH];         // 活体模型,可以不写，使用默认
    CHAR  szFaceKeyPointTrackFile[MAX_PATH];    // 人脸检测模型,可以不写，使用默认
    SHORT nGpu;                                 // 是否使用GPU(1/使用GPU,-1使用CPU)
    SHORT nMultiThread;                         // 是否多线程检测
    st_Detect_Init_Param()
    {
        memset(this, 0x00, sizeof(st_Detect_Init_Param));
    }
} STDETECTINITPAR, *LPSTDETECTINITPAR;

/***************************************************************************
// 封装类: 命令编辑、发送接收等处理。
***************************************************************************/
class CDevImpl_CloudWalk : public CLogManage, public CDevImpl_DEF
{
public:
    CDevImpl_CloudWalk();
    CDevImpl_CloudWalk(LPSTR lpLog);
    CDevImpl_CloudWalk(LPSTR lpLog, LPCSTR lpDevType);
    virtual ~CDevImpl_CloudWalk();

public:
    INT OpenDevice();                                               // 打开设备(加载检测对象)
    INT OpenDevice(STDETECTINITPAR stInitPar);                      // 打开设备
    INT CloseDevice();                                              // 关闭设备
    BOOL IsDeviceOpen();                                            // 设备是否Open成功
    BOOL GetIsLiveSucc();                                           // 获取活检成功标记
    INT GetLiveErrCode();                                           // 获取活检错误码
    INT GetViewData(cv::Mat &cvMatView);                            // 获取视频流数据
    INT GetVideoData(LPSTIMGDATA lpImgData, INT nWidth, INT nHeight,
                     DWORD dwParam);                                // 获取视频流数据
    //INT GetDetectImageRoom(CWLiveImage &stImg, CWSize stDispSize);  // 获取活检全景图像数据
    //INT GetDetectImagePerson(CWLiveImage &stImg);                   // 获取活检人脸图像数据
    //INT GetDetectImageNis(CWLiveImage &stImg, CWSize stDispSize);   // 获取活检红外图像数据
    BOOL GetNirImgSup();                                            // 当前SDK是否支持红外图片生成
    LPSTR ConvertCode_Impl2Str(LONG lErrCode);                      // 错误码解析

public:    // DevXXX.SetData相关
    INT SetReConFlag(BOOL bFlag);                                   // 设置断线重连标记
    INT SetLibPath(LPCSTR lpPath);                                  // 设置动态库路径(DeviceOpen前有效)
    void SetSDKVersion(WORD wVer);                                  // 设置SDK版本

public:     // 接口函数封装
    // 1.3 获取COM服务及算法版本号
    INT GetVersion(LPSTR lpVer, long lSize);
    // 1.4 注册相机回调回显函数(绘制人脸框及显示回调函数)
    INT RegisterPreviewHandle(HANDLE hWnd);
    INT RegisterPreviewHandle(HANDLE hWnd, preview_func fuPreView);
    // 1.5 注册检测回调函数对象(活体检测及回调设置)
    INT RegisterDetectHandle(HANDLE hWnd);
    INT RegisterDetectHandle(HANDLE hWnd,
                              face_det_func fuFaceDet,
                              live_det_func fuLiveDet);
    // 1.6 加载检测模型及设置检测参数,初始化检测器(加载算法模型)
    INT InitDetectorParam(short modelMode, short livenessMode, short licenseType,
                          char* configFile, char* faceDetectFile,
                          char* faceKeyPointDetectFile, char* faceKeyPointTrackFile,
                          char* faceQualityFile, char* faceLivenessFile, short gpu,
                          short multiThread);
    // 1.7 是否进行人脸追踪显示
    INT EnableFaceTrace(BOOL Enable);
    // 1.9 依据相机在设备显示的序号开启相机
    INT OpenCamera(SHORT deviceIndex, SHORT deviceMode, SHORT width, SHORT height);
    // 1.10 依据相机硬件信息开启相机(VID/PID)
    INT OpenCameraEx(LPSTR vid, LPSTR pid, SHORT deviceMode, SHORT width, SHORT height);
    // 1.11 关闭当前程序打开的所有相机设备
    INT CloseAllCameras();
    // 1.12 枚举当前设备上的所有相机
    INT EnumCameras(CWCameraDevice **cameraLists);
    // 1.13 枚举相机支持的所有分辨率(序号)
    INT EnumResolutions(SHORT deviceIndex, CWResolution** resolutionLists);
    // 1.14 枚举相机支持的所有分辨率(VIDPID)
    INT EnumResolutionsEx(LPSTR vid, LPSTR pid, CWResolution** resolutionLists);
    // 1.15 开启活体检测
    INT StartLiveDetect(BOOL isContinue);
    // 1.16 停止连续活体检测
    INT StopLiveDetect();
    // 1.17 抓拍当前画面帧
    INT CaptureCurrentFrame(CWSize size, BOOL isBase64, CWLiveImage* image);
    // 1.18 保存当前帧至指定文件路径参数
    INT SaveCurrentFrame(LPSTR fileName);
    // 1.19 开启语音提示功能
    INT EnableVoiceTip(BOOL Enable, LPCSTR lpTipFile);
    // 1.20 获取本次活体检测最佳人脸图像
    INT GetBestFace(float ratio, BOOL isBase64, CWLiveImage* image, INT type = 1);
    // 1.21 保存活体最佳人脸到指定路径
    INT SaveBestFace(LPSTR fileName, float ratio);
    // 1.22 通过序号得到相机状态
    INT GetCameraStatus(short index, CWCameraState* status);
    // 1.23 通过相机硬件信息VID/PID得到相机状态
    INT GetCameraStatusEx(LPSTR vid, LPSTR pid, CWCameraState* state);
    // 1.26 将图片转换成base64编码格式
    INT Image2Base64Format(LPSTR imageFilePath,  std::string* strBase64);//LPSTR strBase64);
    // 1.28 依照相机在设备上的序号休眠相机
    INT SleepCamera(SHORT index, BOOL IsSleep);
    // 1.29 通过相机硬件信息VID/PID休眠相机
    INT SleepCameraEx(LPSTR vid, LPSTR pid, BOOL IsSleep);
    // 1.32 获得相机固件版本信息，依照相机在设备上的序号
    INT GetFirmwareVersion(short deviceIndex, LPSTR lpFmVer);
    // 1.33 获得相机固件版本信息，依照相机在设备VID/PID
    INT GetFirmwareVersionEx(LPSTR vid, LPSTR pid, LPSTR lpFmVer);

private:
    void    Init();

private:    // 变量定义
    CSimpleMutex    m_cMutex;
    CHAR            m_szDevType[64];                                    // 设备类型
    HANDLE          m_cwDetector;                                       // 设备句柄
    BOOL            m_bDevOpenOk;                                       // 设备Open标记
    BOOL            m_bLiveDetectSucc;                                  // 活检成功标记
    WORD            m_wSDKVersion;                                      // SDK版本(SP内定义)
    BOOL            m_bReCon;                                           // 是否断线重连状态
    CHAR            m_szErrStr[1024];                                   // 错误码解析
    INT             m_nRetErrOLD[12];                                   // 处理错误值保存(0:库加载/1:设备连接/
                                                                        //  2:设备状态/3介质状态/4检查设备状态/
                                                                        //  5:设置设备列表/6:设备列表)
    Mat             m_cvMatView;                                        // 视频流数据
    INT             m_nLiveErrCode;                                     // 活体检测错误码保存
    STIMGDATA       m_stImageData;                                      // 保存图像帧数据
    BOOL            m_GetDataRunning;

private: // 接口加载(QLibrary方式)
    BOOL    bLoadLibrary();                                             // 加载动态库
    void    vUnLoadLibrary();                                           // 释放动态库
    BOOL    bLoadLibIntf();                                             // 加载动态库接口
    void    vInitLibFunc();                                             // 动态库接口初始化

private: // 接口加载(QLibrary方式)
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;

public: // 摄像图框及图像获取函数
    void preview(const uchar* data, int width, int height, int channels, int type = 0);
    void showFace(const long errCode, const long timestamp, const CWLiveFaceDetectInformation_t* pFaceInformations, int nFaceNumber);
    void showLive(int errCode, const float* scores, const float* distances, int nFaceNumber);

private: // 动态库接口定义
    pcwEngineCreateDetector         cwEngineCreateDetector;         // 1.1 创建检测对象
    pcwEngineReleaseDetector        cwEngineReleaseDetector;        // 1.2 释放检测对象及COM服务
    pcwEnginGetVersion              cwEnginGetVersion;              // 1.3 获取COM服务及算法版本号
    pcwEngineRegisterPreviewHandle  cwEngineRegisterPreviewHandle;  // 1.4 注册相机回调回显函数
    pcwEngineRegisterDetectHandle   cwEngineRegisterDetectHandle;   // 1.5 注册检测回调函数对象
    pcwEngineInitDetectorParam      cwEngineInitDetectorParam;      // 1.6 加载检测模型及设置检测参数
    pcwEngineEnableFaceTrace        cwEngineEnableFaceTrace;        // 1.7 是否进行人脸追踪显示
    pcwEngineOpenCamera             cwEngineOpenCamera;             // 1.9 依据相机在设备显示的序号开启相机
    pcwEngineOpenCameraEx           cwEngineOpenCameraEx;           // 1.10 依据相机硬件信息开启相机
    pcwEngineCloseAllCameras        cwEngineCloseAllCameras;        // 1.11 关闭当前程序打开的所有相机设备
    pcwEngineEnumCameras            cwEngineEnumCameras;            // 1.12 枚举当前设备上的所有相机
    pcwEngineEnumResolutions        cwEngineEnumResolutions;        // 1.13 枚举相机支持的所有分辨率(序号)
    pcwEngineEnumResolutionsEx      cwEngineEnumResolutionsEx;      // 1.14 枚举相机支持的所有分辨率(VIDPID)
    pcwEngineStartLiveDetect        cwEngineStartLiveDetect;        // 1.15 开启活体检测
    pcwEngineStopLiveDetect         cwEngineStopLiveDetect;         // 1.16 停止连续活体检测
    pcwEngineCaptureCurrentFrame    cwEngineCaptureCurrentFrame;    // 1.17 抓拍当前画面帧
    pcwEngineSaveCurrentFrame       cwEngineSaveCurrentFrame;       // 1.18 保存当前帧至指定文件路径参数
    pcwEngineEnableVoiceTip         cwEngineEnableVoiceTip;         // 1.19 开启语音提示功能
    pcwEngineGetBestFace            cwEngineGetBestFace;            // 1.20 获取本次活体检测最佳人脸图像
    pcwEngineSaveBestFace           cwEngineSaveBestFace;           // 1.21 保存活体最佳人脸到指定路径
    pcwEngineGetBestFace1           cwEngineGetBestFace1;           // 1.20 获取本次活体检测最佳人脸图像
    pcwEngineSaveBestFace1          cwEngineSaveBestFace1;          // 1.21 保存活体最佳人脸到指定路径
    pcwEngineGetCameraStatus        cwEngineGetCameraStatus;        // 1.22 通过序号得到相机状态
    pcwEngineGetCameraStatusEx      cwEngineGetCameraStatusEx;      // 1.23 通过相机硬件信息VID/PID得到相机状态    
    pcwEngineImage2Base64Format     cwEngineImage2Base64Format;     // 1.26 将图片转换成base64编码格式    
    pcwEngineSleepCamera            cwEngineSleepCamera;            // 1.28 依照相机在设备上的序号休眠相机
    pcwEngineSleepCameraEx          cwEngineSleepCameraEx;          // 1.29 依照相机硬件信息VID/PID休眠相机
    pcwEngineGetFirmwareVersion     cwEngineGetFirmwareVersion;     // 1.32 获得相机固件版本信息，依照相机在设备上的序号
    pcwEngineGetFirmwareVersionEx   cwEngineGetFirmwareVersionEx;   // 1.33 获得相机固件版本信息，依照相机在设备VID/PID
};

#endif // DEVIMPL_CLOUDWALK_H

// -------------------------------------- END --------------------------------------

