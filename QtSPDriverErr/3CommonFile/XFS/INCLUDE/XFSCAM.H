/******************************************************************************
 * *
 * xfscam.h XFS - Camera (CAM) definitions *
 * *
 * Version 3.20 (March 02 2011) *
 * *
 ******************************************************************************/
#ifndef __INC_XFSCAM__H
#define __INC_XFSCAM__H
#ifdef __cplusplus
extern "C"
{
#endif
#include "XFSAPI.H"
/* be aware of alignment */
#pragma pack(push, 1)

/* values of WFSCAMCAPS.wClass */

/* XFS_COMMON.H 中定义
#ifdef QT_WIN_LINUX_XFS

    #define WFS_SERVICE_CLASS_CAM   (10)
    #define WFS_SERVICE_VERSION_CAM (0x1403) /* Version 3.20 * /
#else

    #define WFS_SERVICE_CLASS_CAM   (10)
    #define WFS_SERVICE_VERSION_CAM 0x0001
#endif
*/

#define WFS_SERVICE_NAME_CAM "CAM"
#define CAM_SERVICE_OFFSET   (WFS_SERVICE_CLASS_CAM * 100)

/* CAM Info Commands */
#define WFS_INF_CAM_STATUS       (CAM_SERVICE_OFFSET + 1)
#define WFS_INF_CAM_CAPABILITIES (CAM_SERVICE_OFFSET + 2)

/* CAM Execute Commands */
#define WFS_CMD_CAM_TAKE_PICTURE (CAM_SERVICE_OFFSET + 1)
#define WFS_CMD_CAM_RESET        (CAM_SERVICE_OFFSET + 2)

#ifdef _PISA_STD_
#define     WFS_CMD_CAM_TAKE_PICTURE_EX         (CAM_SERVICE_OFFSET + 3)
#define     WFS_CMD_CAM_DISPLAY                 (CAM_SERVICE_OFFSET + 4) 
#define     WFS_CMD_CAM_FACE_COMPARISON         (CAM_SERVICE_OFFSET + 5)
#define     WFS_CMD_CAM_RECORD                  (CAM_SERVICE_OFFSET + 6)
#define     WFS_CMD_CAM_DISPLAY_HC              (CAM_SERVICE_OFFSET + 7)
#define     WFS_CMD_CAM_IMPORT_KEY              (CAM_SERVICE_OFFSET + 8)
#define     WFS_CMD_CAM_GENERATE_KCV            (CAM_SERVICE_OFFSET + 9)
#define     WFS_CMD_CAM_GET_SIGNATURE           (CAM_SERVICE_OFFSET + 10)
#define     WFS_CMD_CAM_DISPLAYEX               (CAM_SERVICE_OFFSET + 95)
#else
#ifdef ICBC_VERSION
    #define WFS_CMD_CAM_DISPLAY         (CAM_SERVICE_OFFSET + 3)
    #define WFS_CMD_CAM_TAKE_PICTURE_EX (CAM_SERVICE_OFFSET + 4)
    #define WFS_CMD_CAM_RECORD          (CAM_SERVICE_OFFSET + 5)
#elif defined(PINGAN_VERSION)	// 平安银行
    #define WFS_CMD_CAM_DISPLAY         (CAM_SERVICE_OFFSET + 3)
    #define WFS_CMD_CAM_TAKE_PICTURE_EX (CAM_SERVICE_OFFSET + 4)
#else
    #define WFS_CMD_CAM_TAKE_PICTURE_EX (CAM_SERVICE_OFFSET + 3)
    #define WFS_CMD_CAM_DISPLAY         (CAM_SERVICE_OFFSET + 99)
    #define WFS_CMD_CAM_GET_SIGNATURE   (CAM_SERVICE_OFFSET + 97)
    #define WFS_CMD_CAM_DISPLAYEX       (CAM_SERVICE_OFFSET + 95)
#endif
#endif

/* CAM Messages */
#define WFS_USRE_CAM_MEDIATHRESHOLD (CAM_SERVICE_OFFSET + 1)
#define WFS_EXEE_CAM_INVALIDDATA    (CAM_SERVICE_OFFSET + 2)

#define WFS_USRE_CAM_STARTSIGNING   (CAM_SERVICE_OFFSET + 3)

#define WFS_EXEE_CAM_LIVEERROR      (CAM_SERVICE_OFFSET + 50)   // 活体检测图像错误事件(非标准)

/* values of WFSCAMSTATUS.fwDevice */
#define WFS_CAM_DEVONLINE         WFS_STAT_DEVONLINE
#define WFS_CAM_DEVOFFLINE        WFS_STAT_DEVOFFLINE
#define WFS_CAM_DEVPOWEROFF       WFS_STAT_DEVPOWEROFF
#define WFS_CAM_DEVNODEVICE       WFS_STAT_DEVNODEVICE
#define WFS_CAM_DEVHWERROR        WFS_STAT_DEVHWERROR
#define WFS_CAM_DEVUSERERROR      WFS_STAT_DEVUSERERROR
#define WFS_CAM_DEVBUSY           WFS_STAT_DEVBUSY
#define WFS_CAM_DEVFRAUDATTEMPT   WFS_STAT_DEVFRAUDATTEMPT
#define WFS_CAM_DEVPOTENTIALFRAUD WFS_STAT_DEVPOTENTIALFRAUD

/* number of cameras supported/length of WFSCAMSTATUS.fwCameras field */
#define WFS_CAM_CAMERAS_SIZE (8)
#define WFS_CAM_CAMERAS_MAX  (WFS_CAM_CAMERAS_SIZE - 1)

#ifdef _PISA_STD_
#define     WFS_CAM_CREATE                      (0) 
#define     WFS_CAM_DESTROY                     (1) 
#define     WFS_CAM_PAUSE                       (2) 
#define     WFS_CAM_RESUME                      (3) 
#define     WFS_CAM_ERASE                       (4) 
#define     WFS_CAM_PENASERASER                 (5)
#define     WFS_CAM_ERASERASPEN                 (6)  
#else
#ifdef ICBC_VERSION
    #define WFS_CAM_CREATE  (0)
    #define WFS_CAM_DESTROY (1)
    #define WFS_CAM_PAUSE   (2)
    #define WFS_CAM_RESUME  (3)
#elif defined(PINGAN_VERSION)
    #define WFS_CAM_CREATE  (0)
    #define WFS_CAM_DESTROY (1)
    #define WFS_CAM_EXTRA   (3)
    #define WFS_CMD_HIGHCAMERA (4)
#else
    #define WFS_CAM_CREATE  (0)
    #define WFS_CAM_DESTROY (1)
    #define WFS_CAM_PAUSE   (2)
    #define WFS_CAM_RESUME  (3)
    #define WFS_CAM_ERASE   (4)
#endif
#endif

/* indices of WFSCAMSTATUS.fwMedia[...]
WFSCAMSTATUS.fwCameras [...]
WFSCAMSTATUS.usPictures[...]
WFSCAMCAPS.fwCameras [...]
WFSCAMTAKEPICT.wCamera */
#define WFS_CAM_ROOM     (0)
#define WFS_CAM_PERSON   (1)
#define WFS_CAM_EXITSLOT (2)

#define WFS_CAM_EXTRA    (3)
#define WFS_CAM_HIGHTCAMERA (4)
#define WFS_CAM_PANORAMIC (6)

/* values of WFSCAMSTATUS.fwMedia */
#define WFS_CAM_MEDIAOK      (0)
#define WFS_CAM_MEDIAHIGH    (1)
#define WFS_CAM_MEDIAFULL    (2)
#define WFS_CAM_MEDIAUNKNOWN (3)
#define WFS_CAM_MEDIANOTSUPP (4)

/* values of WFSCAMSTATUS.fwCameras */
#define WFS_CAM_CAMNOTSUPP (0)
#define WFS_CAM_CAMOK      (1)
#define WFS_CAM_CAMINOP    (2)
#define WFS_CAM_CAMUNKNOWN (3)

/* values of WFSCAMCAPS.fwType */
#define WFS_CAM_TYPE_CAM (1)

/* values of WFSCAMCAPS.fwCameras */
#define WFS_CAM_NOT_AVAILABLE (0)
#define WFS_CAM_AVAILABLE     (1)

/* values of WFSCAMCAPS.fwCamData */
#define WFS_CAM_NOTADD  (0)
#define WFS_CAM_AUTOADD (1)
#define WFS_CAM_MANADD  (2)

/* values of WFSCAMCAPS.fwCharSupport */
#define WFS_CAM_ASCII   (0x0001)
#define WFS_CAM_UNICODE (0x0002)
/* values of WFSCAMSTATUS.wAntiFraudModule */
#define WFS_CAM_AFMNOTSUPP        (0)
#define WFS_CAM_AFMOK             (1)
#define WFS_CAM_AFMINOP           (2)
#define WFS_CAM_AFMDEVICEDETECTED (3)
#define WFS_CAM_AFMUNKNOWN        (4)



/* values of WFSCAMDISP.wTextPosition */ 
#define     WFS_CAM_TEXT_POS_DEFAULT          (0)
#define     WFS_CAM_TEXT_POS_LEFT_TOP         (1)
#define     WFS_CAM_TEXT_POS_LEFT_BOTTOM      (2)
#define     WFS_CAM_TEXT_POS_RIGHT_TOP        (3)
#define     WFS_CAM_TEXT_POS_RIGHT_BOTTOM     (4)
#define     WFS_CAM_TEXT_POS_TOP              (5)
#define     WFS_CAM_TEXT_POS_BOTTOM           (6)
#define     WFS_CAM_TEXT_POS_CENTOR           (7)

/* values of WFSCAMDISP.wBkShow*/
#define     WFS_CAM_BK_SHOW_DEFAULT           (0)
#define     WFS_CAM_BK_SHOW_CENTOR            (1)
#define     WFS_CAM_BK_SHOW_STRETCH           (2)
/*values  LFSCAMSIGNDATA.status,
LFSCAMGETSIGNATUREMINGWOUT.status,
LFSCAMGETSIGNATUREMIWOUT.status  */
#define     WFS_CAM_DATAOK                      (1)
#define     WFS_CAM_DATAMISSING                 (2)
#define     WFS_CAM_DATASRCNOTSUPP              (3)
#define     WFS_CAM_DATASRCMISSING              (4)

/* values LPWFSCAMLIVEERROR.wLiveError 非标准 */
#define WFS_CAM_LIVE_ERR_OK                     (0)     // 活检图像正常
#define WFS_CAM_LIVE_ERR_VIS_NOFACE             (1)     // 可见光未检测到人脸
#define WFS_CAM_LIVE_ERR_FACE_SHELTER           (2)     // 人脸有遮挡(五官有遮挡,戴镜帽等)
#define WFS_CAM_LIVE_ERR_FACE_ANGLE_FAIL        (3)     // 人脸角度不满足要求(低头/抬头/侧脸等)
#define WFS_CAM_LIVE_ERR_FACE_EYECLOSE          (4)     // 检测到闭眼
#define WFS_CAM_LIVE_ERR_FACE_MOUTHOPEN         (5)     // 检测到张嘴
#define WFS_CAM_LIVE_ERR_FACE_SHAKE             (6)     // 检测到人脸晃动/模糊
#define WFS_CAM_LIVE_ERR_FACE_MULTIPLE          (7)     // 检测到多张人脸
#define WFS_CAM_LIVE_ERR_IS_UNLIVE              (8)     // 检测到非活体
#define WFS_CAM_LIVE_ERR_FACE_TOOFAR            (9)     // 人脸离摄像头太远
#define WFS_CAM_LIVE_ERR_FACE_TOONEAR           (10)    // 人脸离摄像头太近
#define WFS_CAM_LIVE_ERR_NIS_NOFACE             (11)    // 红外光未检测到人脸
#define WFS_CAM_LIVE_ERR_UNKNOWN                (99)    // 其他/未知错误

/* XFS CAM Errors */
#define WFS_ERR_CAM_CAMNOTSUPP       (-(CAM_SERVICE_OFFSET + 0))
#define WFS_ERR_CAM_MEDIAFULL        (-(CAM_SERVICE_OFFSET + 1))
#define WFS_ERR_CAM_CAMINOP          (-(CAM_SERVICE_OFFSET + 2))
#define WFS_ERR_CAM_CHARSETNOTSUPP   (-(CAM_SERVICE_OFFSET + 3))
#define WFS_ERR_CAM_FILEIOERROR      (-(CAM_SERVICE_OFFSET + 4))
#ifdef _PISA_STD_
#define WFS_ERR_CAM_KEYNOTFOUND           (-(CAM_SERVICE_OFFSET + 5)) 
#define WFS_ERR_CAM_ACCESSDENIED          (-(CAM_SERVICE_OFFSET + 6)) 
#define WFS_ERR_CAM_KEYINVALID            (-(CAM_SERVICE_OFFSET + 7)) 
#define WFS_ERR_CAM_NOKEYRAM              (-(CAM_SERVICE_OFFSET + 8)) 
#define WFS_ERR_CAM_INVALIDKEYLENGTH      (-(CAM_SERVICE_OFFSET + 9)) 
#define WFS_ERR_CAM_INVALID_HWND          (-(CAM_SERVICE_OFFSET + 10))
#define WFS_ERR_CAM_NOTSIGN_ERROR         (-(CAM_SERVICE_OFFSET + 11))
#else
#define WFS_ERR_CAM_ACCESSDENIED     (-(CAM_SERVICE_OFFSET + 98))
#define WFS_ERR_CAM_KEYINVALID       (-(CAM_SERVICE_OFFSET + 97))
#define WFS_ERR_CAM_NOKEYRAM         (-(CAM_SERVICE_OFFSET + 96))
#define WFS_ERR_CAM_INVALIDKEYLENGTH (-(CAM_SERVICE_OFFSET + 95))
#define WFS_ERR_CAM_INVALID_WND      (-(CAM_SERVICE_OFFSET + 92))
#define WFS_ERR_CAM_NOTSIGN_ERROR    (-(CAM_SERVICE_OFFSET + 91))
#endif

//自定义错误
#define WFS_ERR_CAM_LIVENESSFAIL     (-(CAM_SERVICE_OFFSET + 99))

/* values of WFSCAMTAKEPICTSIZE.wScan_Size */
#define     WFS_CAM_SIZEALL                         (0)
#define     WFS_CAM_SIZEAUTO                        (1)
#define     WFS_CAM_SIZEA4                          (2)
#define     WFS_CAM_SIZEA5                          (3)
#define     WFS_CAM_SIZEA6                          (4)
#define     WFS_CAM_SIZEA7                          (5)
#define     WFS_CAM_SIZECALLINGCARD	                (6)
#define     WFS_CAM_SIZEID                          (7)
#define     WFS_CAM_SIZECUSTOM                      (8)
#define     WFS_CAM_SIZEAJUSTEDTOWINDOW             (9)
#define     WFS_CAM_SIZEAJUSTEDTOFRAME              (10)
#define     WFS_CAM_SIZEAJUSTEDTOFRAMEANDCHECKFACE  (11)

/* values of WFSCAMIMPORT.wAlgorithm,WFSCAMGETSIGNATUREENCRYPT.wAlgorithm */
#define     WFS_CAM_CRYPTNONE                  (0)
#define     WFS_CAM_CRYPTDESECB                (1)
#define     WFS_CAM_CRYPTDESCBC                (2)
#define     WFS_CAM_CRYPT3DESECB               (3)
#define     WFS_CAM_CRYPT3DESCBC               (4)
#define     WFS_CAM_CRYPTSM2                   (5)
#define     WFS_CAM_CRYPTSM3                   (6)
#define     WFS_CAM_CRYPTSM4ECB                (7)
#define     WFS_CAM_CRYPTSM4CBC                (8)

/* values of WFSCAMIMPORT.wClear */
#define     WFS_CAM_CLEARNONE                  (0)
#define     WFS_CAM_CLEARKEY                   (1)
#define     WFS_CAM_CLEARALL                   (2)

/* values of WFSCAMIMPORT.wKey_Check_Mode */
#define     WFS_CAM_KCVNONE                    (0) 
#define     WFS_CAM_KCVSELF                    (1) 
#define     WFS_CAM_KCVZERO                    (2) 
    /*=================================================================*/
    /* CAM Info Command Structures */
    /*=================================================================*/
    typedef struct _wfs_cam_status
    {
        WORD fwDevice;
        WORD fwMedia[WFS_CAM_CAMERAS_SIZE];
        WORD fwCameras[WFS_CAM_CAMERAS_SIZE];
        USHORT usPictures[WFS_CAM_CAMERAS_SIZE];
        LPSTR lpszExtra;
        WORD wAntiFraudModule;
    } WFSCAMSTATUS, *LPWFSCAMSTATUS;

    typedef struct _wfs_cam_caps
    {
        WORD wClass;
        WORD fwType;
        WORD fwCameras[WFS_CAM_CAMERAS_SIZE];
        USHORT usMaxPictures;
        WORD fwCamData;
        USHORT usMaxDataLength;
        WORD fwCharSupport;
        LPSTR lpszExtra;
        BOOL bPictureFile;
        BOOL bAntiFraudModule;
    } WFSCAMCAPS, *LPWFSCAMCAPS;
    /*=================================================================*/
    /* CAM Execute Command Structures */
    /*=================================================================*/
    typedef struct _wfs_cam_take_picture
    {
        WORD wCamera;
        LPSTR lpszCamData;
        LPWSTR lpszUNICODECamData;
#ifdef ICBC_VERSION
        LPSTR lpstrPath;
        WORD wScanSize;
#endif
    } WFSCAMTAKEPICT, *LPWFSCAMTAKEPICT;

typedef struct _wfs_hex_cam_data
{
    USHORT                  usLength;
    LPBYTE                  lpbData;
} WFSXCAMDATA, *LPWFSXCAMDATA;

    typedef struct _wfs_cam_take_picture_ex
    {
        WORD wCamera;
        LPSTR lpszCamData;
        LPWSTR lpszUNICODECamData;
        LPSTR lpszPictureFile;
#ifdef _PISA_STD_
	WORD   wScanSize;
	LPSTR  lpstrExtra;
#else   
#if defined(PINGAN_VERSION)
        WORD  wScanSize;
#endif
#endif
    } WFSCAMTAKEPICTEX, *LPWFSCAMTAKEPICTEX;

#ifdef _PISA_STD_
typedef struct _wfs_cam_display
{
    WORD                     wCamera;
    WORD                     wAction;
    WORD                     wX;
    WORD                     wY;
    WORD                     wWidth;
    WORD                     wHeight;
    DWORD                    hWnd;
    WORD                     wHpixel;
    WORD                     wVpixel;
    LPSTR                    lpstrTextData;
    LPWSTR                   lpstrUNICODETextData;
    WORD                     wFontWidth;
    WORD                     wFontHeight;
    LPSTR                    lpstrFace;
    WORD                     wTextPosition;
    LPSTR                    lpstrBackgroundFile;
    WORD                     wBkShow;
    LPSTR                    lpstrKey;
    LPSTR                    lpstrEncKey;
    LPWFSXCAMDATA            lpxKeyEncKey;
    WORD                     wAlgorithm;
    LPSTR                    lpstrExtra;
}WFSCAMDISP, *LPWFSCAMDISP;
#else
    typedef struct _wfs_cam_display
    {
        WORD wCamera;
        WORD wAction;
        WORD wWidth;
        WORD wHeight;
        WORD wX;
        WORD wY;
        DWORD hWnd;
    #if defined(SET_BANK_CMBC) | defined(SET_BANK_SXXH) | defined(SET_BANK_ICBC)
        WORD wHpixel;
        WORD wVpixel;
        LPSTR pszTexData;
    #else//if defined(SET_BANK_JIANGNAN)
        WORD wHpixel;
        WORD wVpixel;
    #endif
    } WFSCAMDISP, *LPWFSCAMDISP;
#endif

typedef struct _wfs_cam_face_comparison 
{
    WORD                     wCamera;
    LPSTR                    lpstrPictureFileA;
    LPSTR                    lpstrPictureFileB;
    LPSTR                    lpstrExtra;
} WFSCAMFACECOMPARISON, *LPWFSCAMFACECOMPARISON;

typedef struct _wfs_cam_face_comparison_out
{
    WORD                     wScore;
    LPSTR                    lpstrExtra;
} WFSCAMFACECOMPARISONOUT, *LPWFSCAMFACECOMPARISONOUT;

#ifdef _PISA_STD_
typedef struct _wfs_cam_display_hc
{
    WORD                    wCamera;
    WORD                    wAction;
    WORD                    wWidth;
    WORD                    wHeight;
    WORD                    wX;
    WORD                    wY;
    DWORD                   hWnd;
    WORD                    wHpixel;
    WORD                    wVpixel;
    WORD                    wRotateAngle;
    WORD                    wColorType;
    WORD                    wCropType;
    WORD                    wCropSize;
    LPSTR                   lpstrTexData;
    LPSTR                   lpstrExtra;
} WFSCAMDISPHC, *LPWFSCAMDISPHC;
#endif
typedef struct _wfs_cam_display_ex
{
    WORD                    wCamera;
    LPSTR                   pszPictureFile;
    LPSTR                   pszTexData;
    WORD                    wWidth;            // 窗口宽
    WORD                    wHeight;           // 窗口高
    WORD                    wX;                // 窗口X坐标
    WORD                    wY;                // 窗口Y坐标
    DWORD                   hWnd;             // 窗口句柄
    WORD                    wHpixel;           // 水平像素
    WORD                    wVpixel;           // 垂直像素
} WFSCAMDISPEX, *LPWFSCAMDISPEX;

//#ifdef _PISA_STD_
//typedef struct _wfs_cam_get_signature
//{
//    WORD                     wCamera;
//    LPSTR                    lpstrPictureFile;
//    LPSTR                    lpstrTrackFile;
//    LPSTR                    lpstrExtra;
//} WFSCAMGETSIGNATURE, *LPWFSCAMGETSIGNATURE;
//#else
typedef struct _wfs_cam_get_signature
{
    WORD   camera;
    LPSTR  cam_data;
    LPWSTR unicode_cam_data;
    LPSTR  picture_file;
    DWORD  wnd;
    LPSTR  key;
    LPSTR  enc_key;
} WFSCAMGETSIGNATURE, *LPWFSCAMGETSIGNATURE;
//#endif

typedef struct _wfs_cam_sign_data
{
    WORD    status;
    ULONG   data_length;
    LPBYTE  data;
}WFSCAMSIGNDATA, *LPWFSCAMSIGNDATA;

#ifdef _PISA_STD_
typedef struct _wfs_cam_record
{
    WORD                    wCamera;
    WORD                    wAction;
    WORD                    wHpixel;
    WORD                    wVpixel;
    WORD                    wFps;
    WORD                    wAudioOption;
    LPSTR                   lpszRecordFile;
    LPSTR                   lpstrExtra;
} WFSCAMRECORD, *LPWFSCAMRECORD;
#else
#ifdef ICBC_VERSION
    typedef struct _wfs_cam_record
    {
        WORD wCamera;
        WORD wAction;
        WORD wHpixel;
        WORD wVpixel;
        WORD wFps;
        WORD wAudioOption;
        LPSTR lpszRecordFile;
    } WFSCAMRECORD, *LPWFSCAMRECORD;
#endif
#endif

#ifdef _PISA_STD_
typedef struct _wfs_cam_import
{ 
    LPSTR                   lpstrKey;
    LPSTR                   lpstrEncKey;
    LPWFSXCAMDATA           lpxValue;
    LPWFSXCAMDATA           lpxControlVector;
    WORD                    wClear;
    WORD                    wAlgorithm;
    WORD                    wKeyCheckMode;
    LPWFSXCAMDATA           lpxKeyCheckValue;
    LPSTR                   lpstrExtra;
} WFSCAMIMPORT, *LPWFSCAMIMPORT;


typedef struct _wfs_cam_get_signature_out
{
    WORD                     wStatus;
    ULONG                    ulDataLength;
    LPBYTE                   lpbData;
    LPSTR                    lpstrExtra;
} WFSCAMGETSIGNATUREOUT, *LPWFSCAMGETSIGNATUREOUT;
#endif
    /*=================================================================*/
    /* CAM Message Structures */
    /*=================================================================*/
    // 活体图像检测错误事件
    typedef struct _wfs_cam_live_error
    {
        WORD              wLiveError;
    } WFSCAMLIVEERROR, * LPWFSCAMLIVEERROR;

/* restore alignment */
#pragma pack(pop)
#ifdef __cplusplus
} /*extern "C"*/
#endif
#endif /* __INC_XFSCAM__H */
