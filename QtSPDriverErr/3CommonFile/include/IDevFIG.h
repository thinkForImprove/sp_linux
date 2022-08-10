#pragma once
/***************************************************************
* 文件名称：IDevFIG.h
* 文件描述：声明指纹仪底层对外提供的所有的控制指令接口及测试指令接口
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年2月26日
* 文件版本：1.0.0.1
****************************************************************/
#include <string.h>
#include <QtCore/qglobal.h>

// 设备类型
#define IDEV_TYPE_HX        "HX"        // HX-F200C(海鑫)
#define IDEV_TYPE_TCM       "TCM"       // TCM042/TCM028(天诚盛业)
#define IDEV_TYPE_SM205     "SM205"     // 中正
#define IDEV_TYPE_WEL401    "WL"

#if defined IDEVFIG_LIBRARY
#define DEVFIG_EXPORT     Q_DECL_EXPORT
#else
#define DEVFIG_EXPORT     Q_DECL_IMPORT
#endif

#define FIG_IMAGE_WIDTH_INTI_SIZE   256
#define FIG_IMAGE_HEIGHT_INIT_SIZE  360
#define FIG_IMAGE_BMP_HEAD_SIZE     1078
#define FIG_IMAGE_RAW_DATA_SIZE     (FIG_IMAGE_WIDTH_INTI_SIZE * FIG_IMAGE_HEIGHT_INIT_SIZE)
#define FIG_IMAGE_BMP_DATA_SIZE     (FIG_IMAGE_WIDTH_INTI_SIZE * FIG_IMAGE_HEIGHT_INIT_SIZE + FIG_IMAGE_BMP_HEAD_SIZE)
// -----------------功能入口函数返回值-------------------------------------------------------
//成功
#define ERR_FIG_SUCCESS             (0)     // 操作成功
//状态错误
#if defined(SET_BANK_CMCB)
#define ERR_FIG_PARAM_ERR           (-1)    // 参数错误
#define ERR_FIG_COMM_ERR            (-2)    // 通讯错误
#define ERR_FIG_NO_PAPER            (-3)    // 打印机缺纸
#define ERR_FIG_JAMMED              (-4)    // 堵纸等机械故障
#define ERR_FIG_NOT_OPEN            (-6)    // 设备没有打开
#else
#define ERR_FIG_NOT_OPEN            (-1)    // 设备没有打开
#define ERR_FIG_PARAM_ERR           (-2)    // 参数错误
#define ERR_FIG_COMM_ERR            (-3)    // 通讯错误
#define ERR_FIG_NO_PAPER            (-4)    // 打印机缺纸
#define ERR_FIG_JAMMED              (-5)    // 堵纸等机械故障
#endif
#define ERR_FIG_HEADER              (-7)    // 打印头故障
#define ERR_FIG_CUTTER              (-8)    // 切刀故障
#define ERR_FIG_TONER               (-9)    // INK或色带故障
#define ERR_FIG_STACKER_FULL        (-10)   // 用户没有取走
#define ERR_FIG_NO_RESUME           (-11)   // 不可恢复的错误
#define ERR_FIG_CAN_RESUME          (-12)   // 可恢复的错误
#define ERR_FIG_FORMAT_ERROR        (-13)   // 打印字串格式错误
#define ERR_FIG_CHRONIC             (-14)   // 慢性故障
#define ERR_FIG_HWERR               (-15)   // 硬件故障
#define ERR_FIG_IMAGE_ERROR         (-16)   // 打印图片相关错误
#define ERR_FIG_NO_DEVICE           (-17)   // 如果指定名的设备不存在，CreatePrinterDevice返回
#define ERR_FIG_UNSUP_CMD           (-18)   // 不支持的指令
#define ERR_FIG_DATA_ERR            (-19)   // 收发数据错误
#define ERR_FIG_TIMEOUT             (-20)   // 超时
#define ERR_FIG_DRVHND_ERR          (-21)   // 驱动错误
#define ERR_FIG_DRVHND_REMOVE       (-22)   // 驱动丢失
#define ERR_FIG_USB_ERR             (-23)   // USB错误
#define ERR_FIG_DEVBUSY             (-25)   // 设备忙
#define ERR_FIG_OTHER               (-26)   // 其它错误，如调用API错误等
#define ERR_FIG_DEVUNKNOWN          (-27)   // 设备未知
#define ERR_FIG_GETTEM_ERR          (-203)  //GET Template Fail
#define ERR_FIG_GETFEATURE_ERR      (-255)  //GET Feature Fail
#define ERR_FIG_MATCHFAIL           (-201)  //Match Fail

// 释放内存
#define FUNC_BUFFER_FREE_RETURN(LPBYTE) \
    if(!LPBYTE){   \
        free(LPBYTE); \
    LPBYTE = nullptr; \
    }

// 指位表
typedef struct _finger_code
{
    unsigned char Right_Thumb;                  // 右手拇指
    unsigned char Right_Forefinger;             // 右手食指
    unsigned char Right_Middle_Finger;          // 右手中指
    unsigned char Right_Ring_Finger;            // 右手环指
    unsigned char Right_Little_Finger;          // 右手小指
    unsigned char Left_Thumb;                   // 左手拇指
    unsigned char Left_Forefinger;              // 左手食指
    unsigned char Left_Middle_Finger;           // 左手中指
    unsigned char Left_Ring_Finger;             // 左手环指
    unsigned char Left_Little_Finger;           // 左手小指
    unsigned char Right_Uncertain;              // 右手不确定指位
    unsigned char Left_Uncertain;               // 左手不确定指位
    unsigned char Other_Uncertain;              // 其他不确定指位

    void init()
    {
        Right_Thumb             = 0x0B;
        Right_Forefinger        = 0x0C;
        Right_Middle_Finger     = 0x0D;
        Right_Ring_Finger       = 0x0E;
        Right_Little_Finger     = 0x0F;
        Left_Thumb              = 0x10;
        Left_Forefinger         = 0x11;
        Left_Middle_Finger      = 0x12;
        Left_Ring_Finger        = 0x13;
        Left_Little_Finger      = 0x14;
        Right_Uncertain         = 0x61;
        Left_Uncertain          = 0x62;
        Other_Uncertain         = 0x63;
    }

} STFINGERCODE, *LPSTFINGERCODE;

// 采集器相关数据
typedef struct _print_data
{
    int nBright;                     // 采集器当前的亮度
    int nContrast;                   // 采集器当前对比度
    int nOriginX;                    // 图像的采集位置X
    int nOriginY;                    // 图像的采集位置Y
    int nWidth;                      // 图像的宽度
    int nHeight;                     // 图像的高度

    void init()
    {
        nBright = 150;
        nContrast = 150;
        nOriginX = 0;
        nOriginY = 0;
        nWidth = FIG_IMAGE_WIDTH_INTI_SIZE;
        nHeight = FIG_IMAGE_HEIGHT_INIT_SIZE;
    }

} STPRINTDATA, *LPSTPRINTDATA;

typedef struct _finger_print_ver
{
    double          dCapVersion;        // 采集接口版本
    char            FprVersion[10];     // 指纹接口版本
    char            pszDesc[1024];      // 接口说明
#if defined(SET_BANK_CMCB) | defined(SET_BANK_CSCB)
    char            pszDevSN[256];  // 设备序列号
#endif

    _finger_print_ver()
    {
        dCapVersion = 0.00;
        memset(FprVersion, 0, sizeof(FprVersion));
        memset(pszDesc, 0, sizeof(pszDesc));
#if defined(SET_BANK_CMCB) | defined(SET_BANK_CSCB)
        memset(pszDevSN, 0, sizeof(pszDevSN));
#endif
    }

}STFPRVERSION, *LPSTFPRVERSION;

struct IDevFIG
{
    virtual int Release() = 0;
    /************************************************************
    ** 功能：打开与设备的连接
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Open() = 0;

    /************************************************************
    ** 功能：关闭与设备的连接
    ** 输入：无
    ** 输出：无
    ** 返回：无
    ************************************************************/
    virtual void Close() = 0;

    /************************************************************
    ** 功能：设备初始化
    ** 输入: 无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int Init() = 0;

    /************************************************************
    ** 功能：获取接口版本
    ** 输入：STFPRVERSION 接口结构体
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetVersion(STFPRVERSION &stFprVersion) = 0;

    /************************************************************
    ** 功能：获取指纹模板
    ** 输入：无
    ** 输出：pData 为图像数据, pFeature 为模板数据, npLen 为模板数据长度
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int GetFingerTemplateData(unsigned char * pData, unsigned char * pFeature, int *npLen)// = 0;
	{
		return ERR_FIG_UNSUP_CMD;
	}

    /************************************************************
    ** 功能：保存指纹图片
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int SaveFingerImage(const char* pImgPath, const char* pImgBuf)// = 0;
	{
		return ERR_FIG_UNSUP_CMD;
	}
	
#if defined(SET_BANK_CMCB)
	/************************************************************
    ** 功能：采集指纹特征
    ** 输入：无
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CollectFPData() = 0;
#endif

    /************************************************************
    ** 功能：提取指纹特征
    ** 输入：无
    ** 输出：pFingerImgBuf 为指纹数据，pFingerBmpBuf 为图像数据，nImgLen　为指纹数据长度，pFeatureData 为指纹图像特征数据指针, dwTimeOut 为超时时间
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int FeatureExtract(unsigned char * pFingerImgBuf, unsigned char * pFingerBmpBuf, unsigned char * pFeatureData, int *nImgLen, unsigned int dwTimeOut) = 0;

    /************************************************************
    ** 功能：比对指纹特征
    ** 输入：pFeatureData1 为指纹特征数据 1, pFeatureData2 为指纹特征数据 2
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int CompareFPData(unsigned char * pFeatureData1, unsigned char * pFeatureData2) = 0;

    /************************************************************
    ** 功能：检查是否有手指按捺
    ** 输入：起始时间dwStart, 超时时间dwTimeOut
    ** 输出：无
    ** 返回：见返回错误码定义
    ************************************************************/
    virtual int ChkFingerPressed() = 0;
};

/************************************************************
** 功能：获取设备连接handle
** 输入：lpDevType: 设备类型(缺省HOTS，当前版本指定为NULL)
** 输出：iPrtDevHandle
** 返回：见返回错误码定义
************************************************************/
extern "C" DEVFIG_EXPORT long CreateIDevFIG(const char *lpDevType, IDevFIG *&pDev);
