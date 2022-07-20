/***************************************************************
* 文件名称：ComInfo.h
* 文件描述：
* 版本历史信息
* 变更说明：
* 变更日期：
* 文件版本：1.0.0.1
****************************************************************/

#pragma once

#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include <string.h>

//----------宏定义----------------------------------------------------
// Camera设备类型
#define CAM_DEV_CLOUDWALK   0   // 云从双目摄像(用于INI中配置)


// Camera 打开方式
#define CAM_OPEN_DEVIDX 0   // 按序号打开
#define CAM_OPEN_VIDPID 1   // 按VID/PID打开

// 摄像保存类型
#define PIC_BASE64  1       // BASE64格式
#define PIC_JPG     2       // JPG格式
#define PIC_BMP     4       // BMP格式

// 银行类型
#define BANK_NOALL  0       // 通用
#define BANK_PINGAN 1       // 平安银行

// 摄像保存路径缺省值(平安银行)
#define IMAGEINFOFILE					"/home/CAMCfg/CameraInfo.ini"                       // TakePic保存PicInfo缺省全路径
#define IMAGESAVEPATH_BASE64			"/home/SPCameraNo/Data/BASE64"						// TakePic保存Pic缺省路径
#define IMAGESAVEPATH_JPG				"/home/SPCameraNo/Data/JPG"							// TakePic保存Pic缺省路径
#define IMAGESAVEPATH_BMP				"/home/SPCameraNo/Data/BMP"							// TakePic保存Pic缺省路径
#define IMAGESAVEINFOFILE				"/home/SPCameraNo/Data"							//
#define SPCAMERASNOCFG					"/home/SPCameraNo/Config/SPCamerasNoCfg.ini"		//
#define SPCAMERASUPFILE                 "/home/projects/wsap/cfg/Camera.ini"
#define CHECKOPENFILE                   "/etc/xfs/logical_services/Camera.ini"


// 摄像数据共享内存名
#define SHAREDMEMNAME_CAMDATA           "CamShowSharedMemData"
// 摄像数据共享内存Size
#define SHAREDMEMSIZE_CAMDATA           1024 * 1024 * 10

// 云从摄像初始化使用
typedef
struct st_cam_cw_init_param
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
    st_cam_cw_init_param()
    {
        memset(this, 0x00, sizeof(st_cam_cw_init_param));
    }
} ST_CAM_CW_INIT_PARAM, *LPST_CAM_CW_INIT_PARAM;

// 摄像保存相关(平安银行使用)
typedef
struct st_cam_save_image_cfg
{
    CHAR	byTakePicDefSavePath[MAX_PATH];										// TakePic 缺省保存目录(INI配置)
    BOOL	bIsNeedDailyRecord;													// 是否启用单天累加文件
    CHAR	byImageInfoFile[MAX_PATH];											// CameraInfo全路径
    CHAR	byImageSavePathBASE64[MAX_PATH];									// CameraPic BASE64备份保存全路径
    CHAR	byImageSavePathJPG[MAX_PATH];										// CameraPic JPG备份保存全路径
    CHAR	byImageSavePathBMP[MAX_PATH];										// CameraPic BMP备份保存全路径
    WORD	wSaveTime;															// CameraPic备份保存天数
    CHAR	byImageSaveInfoPath[MAX_PATH];										//
    CHAR	bySPCamerasNoCfgFile[MAX_PATH];										//
    WORD	wTakePicMakeFormatFlag;
    st_cam_save_image_cfg()
    {
        memset(this, 0x00, sizeof(st_cam_save_image_cfg));
    }
}ST_CAM_SAVEIMAGE_CFG, *LPST_CAM_SAVEIMAGE_CFG;

// 设备打开方式
typedef
struct st_cam_opentype
{
    WORD wOpenType;     // 打开方式，0:表示序号打开 1：表示vid pid打开
    WORD wVisCamIdx;    // 可见光模组枚举序号
    WORD wNisCamIdx;    // 红外光模组枚举序号
    CHAR szVisVid[4+1]; // 可见光模组vid
    CHAR szVisPid[4+1]; // 可见光模组pid
    CHAR szNisVid[4+1]; // 红外光模组vid
    CHAR szNisPid[4+1]; // 红外光模组pid
    st_cam_opentype()
    {
        memset(this, 0x00, sizeof(st_cam_opentype));
    }
}ST_CAM_OPENTYPE, *LPST_CAM_OPENTYPE;

// 摄像窗口参数信息
typedef
struct st_cam_showwin_info
{
    DWORD   hWnd;       // 窗口ID
    WORD    wX;         // 坐标X
    WORD    wY;         // 坐标Y
    WORD    wWidth;     // 窗口宽
    WORD    wHeight;    // 窗口高
    st_cam_showwin_info()
    {
        memset(this, 0x00, sizeof(st_cam_showwin_info));
    }

}CAM_SHOWWIN_INFO, *LPCAM_SHOWWIN_INFO;

// ini获取
typedef
struct st_cam_ini_config
{
    char                 szDevDllName[256];
    ST_CAM_CW_INIT_PARAM stCamCwInitParam;
    ST_CAM_OPENTYPE      stCamOpenType;
    ST_CAM_SAVEIMAGE_CFG stCamSaveImageCfg;
    UINT                 unWinRefreshTime;      // 摄像刷新时间(云从使用)
    WORD                 wTakePicTimeOut;
    WORD                 WBank;                 // 指定银行，0不指定/1平安银行
    CHAR                 szCameraSupFile[MAX_PATH];
    WORD                 wIsInstallCAM;
    CHAR                 szCamDataSMemName[32+1];
    ULONG                ulCamDataSMemSize;
    WORD                 wCamDevType;
    CHAR                 szCamCheckOpenFile[MAX_PATH];
    WORD                 szCamOpenSetLogicName;
    st_cam_ini_config()
    {
        memset(this, 0x00, sizeof(st_cam_ini_config));
    }
}ST_CAM_INI_CONFIG, *LPST_CAM_INI_CONFIG;

// 设备处理类初始化入参
typedef
struct st_cam_dev_init_param
{

    WORD wBank;
    ST_CAM_CW_INIT_PARAM stCamCwInitParam;
    ST_CAM_OPENTYPE      stCamOpenType;
    ST_CAM_SAVEIMAGE_CFG stCamSaveImageCfg;
    CHAR                 szCamDataSMemName[32+1];
    ULONG                ulCamDataSMemSize;
    st_cam_dev_init_param()
    {
        memset(this, 0x00, sizeof(st_cam_dev_init_param));
    }
}ST_CAM_DEV_INIT_PARAM, *LPST_CAM_DEV_INIT_PARAM;


