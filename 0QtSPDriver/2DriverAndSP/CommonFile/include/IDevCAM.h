#pragma once

//////////////////////////////////////////////////////////////////////////
// 返回码
#define CAM_SUCCESS          (0)
#define ERR_CONFIG           (-1)
#define ERR_OPEN_CAMER       (-2)
#define ERR_INVALID_PARAM    (-3)
#define ERR_LOAD_IMAGE       (-4)
#define ERR_FRAME            (-5)
#define ERR_IMWRITE          (-6)
#define ERR_SAVE_IMAGE       (-7)
#define ERR_OTHER            (-8)
#define ERR_TIMEOUT          (-9)

//////////////////////////////////////////////////////////////////////////
#define CAM_CAMERAS_SIZE     (8)
#define MAX_SIZE             (400)
#define MAX_CAMDATA          (4096)
//////////////////////////////////////////////////////////////////////////
enum DEVICE_STATUS
{
    DEVICE_OFFLINE                  = 0,
    DEVICE_ONLINE                   = 1,
    DEVICE_HWERROR                  = 2
};

enum MEDIA_STATUS
{
    MEDIA_OK                        = 0,
    MEDIA_HIGH                      = 1,
    MEDIA_FULL                      = 2,
    MEDIA_UNKNOWN                   = 3,
    MEDIA_NOTSUPP                   = 4
};

enum CAM_POS
{
    CAM_ROOM                        = 0,
    CAM_PERSON                      = 1,
    CAM_EXITSLOT                    = 2
};

enum CAM_STATUS
{
    STATUS_NOTSUPP                  = 0,
    STATUS_OK                       = 1,
    STATUS_INOP                     = 2,
    STATUS_UNKNOWN                  = 3
};

typedef struct tag_dev_cam_status
{
    WORD   fwDevice;
    WORD   fwMedia[CAM_CAMERAS_SIZE];
    WORD   fwCameras[CAM_CAMERAS_SIZE];
    USHORT usPictures[CAM_CAMERAS_SIZE];
    char   szErrCode[8];    // 三位的错误码

    tag_dev_cam_status() { Clear(); }
    void Clear()
    {
        memset(this, 0x00, sizeof(tag_dev_cam_status));
        for (auto &it : fwMedia)
        {
            it = MEDIA_NOTSUPP;// 初始不支持
        }
    }
} DEVCAMSTATUS, *LPDEVCAMSTATUS;

typedef struct cam_cw_init_param    // 云从摄像初始化使用
{
    short nModelMode;                           // 模型加载方式, 0:文件加载 1: 内存加载
    short nLivenessMode;                        // 活体类型, 0:不活检 1: 红外活体 2:结构光活体
    short nLicenseType;                         // 授权类型, 1:芯片授权 2：hasp授权 3:临时授权 4:云从相机绑定授权
    char  szConfigFile[MAX_PATH];               // 算法矩阵文件,可以不写，使用默认
    char  szFaceDetectFile[MAX_PATH];           // 人脸检测模型,可以不写，使用默认
    char  szKeyPointDetectFile[MAX_PATH];       // 关键点检测模型,可以不写，使用默认
    char  szKeyPointTrackFile[MAX_PATH];        // 关键点跟踪模型,可以不写，使用默认
    char  szFaceQualityFile[MAX_PATH];          // 人脸质量评估模型,可以不写，使用默认
    char  szFaceLivenessFile[MAX_PATH];         // 活体模型,可以不写，使用默认
    char  szFaceKeyPointTrackFile[MAX_PATH];    // 人脸检测模型,可以不写，使用默认
    short nGpu;                                 // 是否使用GPU(1/使用GPU,-1使用CPU)
    short nMultiThread;                         // 是否多线程检测
} CAM_CW_INIT_PARAM, *LPCAM_CW_INIT_PARAM;

typedef struct tag_dev_cam_init_param
{
    WORD wOpenType;     // 打开方式，0:表示序号打开 1：表示vid pid打开
    WORD wVisCamIdx;    // 可见光模组枚举序号
    WORD wNisCamIdx;    // 红外光模组枚举序号
    CHAR szVisVid[4+1]; // 可见光模组vid
    CHAR szVisPid[4+1]; // 可见光模组pid
    CHAR szNisVid[4+1]; // 红外光模组vid
    CHAR szNisPid[4+1]; // 红外光模组pid
    CAM_CW_INIT_PARAM stCWParam;
} CAM_INIT_PARAM, *LPCAM_INIT_PARAM;

//////////////////////////////////////////////////////////////////////////
//#define DVECAM_NO_VTABLE  __declspec(novtable)
//////////////////////////////////////////////////////////////////////////
struct /*DVECAM_NO_VTABLE*/ IDevCAM
{
    // 释放接口
    virtual void Release() = 0;
    // 打开连接
    virtual long Open(LPCSTR lpMode) = 0;
    // 关闭连接
    virtual long Close() = 0;
    // 复位
    virtual long Reset() = 0;
    // 读取设备信息
    virtual long GetDevInfo(char *pInfo) = 0;
    // 取状态
    virtual long GetStatus(DEVCAMSTATUS &stStatus) = 0;
    // 打开窗口(窗口句柄，动作指示:1创建窗口/0销毁窗口, X/Y坐标,窗口宽高)
    virtual long Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wHidth, WORD wHeight) = 0;
    // 拍照 传下来的字符lpData按：”FileName=C:\TEMP\TEST;Text=HELLO;TextPositionX=160;TextPositionY=120;CaptureType=0”格式
    virtual long TakePicture(WORD wCamera, LPCSTR lpData) = 0;
    // 拍照(保存文件名,水印字串，图片类型是否连续检测)(1BASE64/2JPG/4BMP;T连续检测)
    virtual long TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue, WORD wTimeOut) = 0;

    // 摄像初始化参数使用
    virtual void vSetCamInitParam(CAM_INIT_PARAM stInitParam) = 0;
};


extern "C" long CreateIDevCAM(LPCSTR lpDevType, IDevCAM *&p);
//////////////////////////////////////////////////////////////////////////