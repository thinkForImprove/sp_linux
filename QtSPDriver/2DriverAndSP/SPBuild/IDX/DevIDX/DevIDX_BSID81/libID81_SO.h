//**********************************************************************************************************************
//  Copyright (C), 2016, 山东新北洋信息技术股份有限公司
//  文件名: libID81_SO.h
//  版本 : V1.0
//  日期:  2016-5-11
//  描述:  加载动态库头文件
//  函数列表:
//**********************************************************************************************************************
#ifndef _LIBID81_SO_H_
#define	_LIBID81_SO_H_

#include "string.h"

bool load_so(char *cErrorInfo);

/**********************************************************************************************************************
* 证件类型宏定义
**********************************************************************************************************************/
#define IDDIGITALCOPIER_TW		 0x4	//台湾通行证
#define IDDIGITALCOPIER_HK_MACAO 0x5	//港澳通行证

/**********************************************************************************************************************
          GBK
* 二代证电子信息结构体宏定义
**********************************************************************************************************************/
typedef struct IDInfo
{
    char name[72];				//姓名
	char sex[4];				//性别
    char nation[20];			//民族
	char birthday[20];			//出生日期
    char address[200];			//住址信息
	char number[40];			//身份证号码
    char department[120];		//签发机关
	char timeLimit[36];			//有效日期
	char Image[256];		    //头像信息文件
}IDInfo;
#define Null_IDinfo	{{0},{0},{0},{0},{0},{0},{0},{0},{0}}

typedef struct IDInfo_GBK
{
    char name[32];				//姓名
    char sex[4];				//性别
    char nation[12];			//民族
    char birthday[20];			//出生日期
    char address[72];			//住址信息
    char number[40];			//身份证号码
    char department[32];		//签发机关
    char timeLimit[36];         //有效日期
    char Image[256];		    //头像信息文件
    IDInfo_GBK()
    {
        memset(name, 0, 32);
        memset(sex, 0, 4);
        memset(nation, 0, 12);
        memset(birthday, 0, 20);
        memset(address, 0, 72);
        memset(number, 0, 40);
        memset(department, 0, 32);
        memset(timeLimit, 0, 36);
        memset(Image, 0, 256);
    }
}IDInfo_GBK;

/**********************************************************************************************************************
* 二代证电子信息结构体宏定义
**********************************************************************************************************************/
typedef struct IDInfoEx
{
    char name[72];				//姓名
	char sex[4];				//性别
    char nation[20];			//民族
	char birthday[20];			//出生日期
    char address[200];			//住址信息
	char number[40];			//身份证号码
    char department[120];		//签发机关
	char timeLimit[36];			//有效日期
	char Image[256];			//头像信息文件
	char FingerData[1024];		//指纹信息数据
	int  iFingerDataLen;
}IDInfoEx;
#define Null_IDinfoEx	{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}}

typedef struct IDInfoEx_GBK
{
    char name[32];				//姓名
    char sex[4];				//性别
    char nation[12];			//民族
    char birthday[20];			//出生日期
    char address[72];			//住址信息
    char number[40];			//身份证号码
    char department[32];		//签发机关
    char timeLimit[36];         //有效日期
    char Image[256];			//头像信息文件
    char FingerData[1024];		//指纹信息数据
    int  iFingerDataLen;

    IDInfoEx_GBK()
    {
        memset(name, 0, 32);
        memset(sex, 0, 4);
        memset(nation, 0, 12);
        memset(birthday, 0, 20);
        memset(address, 0, 72);
        memset(number, 0, 40);
        memset(department, 0, 32);
        memset(timeLimit, 0, 36);
        memset(Image, 0, 256);
        memset(FingerData, 0, 1024);
        iFingerDataLen = 0;
    }
}IDInfoEx_GBK;

/**********************************************************************************************************************
* 外国人居住证电子信息结构体宏定义
**********************************************************************************************************************/
typedef struct IDInfoForeign
{
	char NameENG[120];				//英文姓名		
	char Sex[4];					//性别
	char IDCardNO[30];				//证件号码
    char Nation[40];				//国籍
    char NameCHN[120];				//中文姓名
	char TimeLimitBegin[16];		//证件签发日期开始
	char TimeLimitEnd[16];			//证件签发日期结束
	char Born[36];					//出生日期
    char IDVersion[8];				//证件版本
    char Department[40];            //签发机关
    char IDType[4];					//证件类型
    char Reserve[12];				//保留信息
	char Image[1024];				//头像
}IDInfoForeign;
#define Null_IDinfoForeign	{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}}

typedef struct IDInfoForeign_GBK
{
    char NameENG[120];			//英文姓名
    char Sex[4];				//性别
    char IDCardNO[30];			//证件号码
    char Nation[6];             //国籍
    char NameCHN[72];			//中文name
    char TimeLimitBegin[16];	//证件签发日期开始
    char TimeLimitEnd[16];		//证件签发日期结束
    char Born[36];				//出生日期
    char IDVersion[4];			//证件版本
    char Department[8];         //签发机关
    char IDType[2];             //证件类型
    char Reserve[6];			//保留信息
    char Image[1024];			//头像

    IDInfoForeign_GBK()
    {
        memset(NameENG, 0, 120);
        memset(Sex, 0, 4);
        memset(IDCardNO, 0, 30);
        memset(Nation, 0, 6);
        memset(NameCHN, 0, 72);
        memset(TimeLimitBegin, 0, 16);
        memset(TimeLimitEnd, 0, 16);
        memset(Born, 0, 36);
        memset(IDVersion, 0, 4);
        memset(Department, 0, 8);
        memset(IDType, 0, 2);
        memset(Reserve, 0, 6);
        memset(Image, 0, 1024);
    }
}IDInfoForeign_GBK;

/**********************************************************************************************************************
* 港澳台通行证电子信息结构体宏定义
**********************************************************************************************************************/
typedef struct IDInfoGAT
{
    char name[72];				//姓名
	char sex[4];				//性别
    char nation[20];			//民族
	char birthday[20];			//出生日期
    char address[200];			//住址信息
	char number[40];			//身份证号码
    char department[120];		//签发机关
	char timeLimit[36];			//有效日期
	char passport[20];			//通行证号码
	char issue[6];				//签发次数
	char Image[256];			//头像信息文件
	char FingerData[1024];		//指纹信息数据
	int  iFingerDataLen;
}IDInfoGAT;
#define Null_IDinfoGAT	{{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}}

typedef struct IDInfoGAT_GBK
{
    char name[32];				//姓名
    char sex[4];				//性别
    char nation[12];			//民族
    char birthday[20];			//出生日期
    char address[72];			//住址信息
    char number[40];			//身份证号码
    char department[32];		//签发机关
    char timeLimit[36];		//有效日期
    char passport[20];			//通行证号码
    char issue[6];				//签发次数
    char Image[256];			//头像信息文件
    char FingerData[1024];		//指纹信息数据
    int  iFingerDataLen;
    char IDType[2];			//证件类型

    IDInfoGAT_GBK()
    {
        memset(name, 0, 32);
        memset(sex, 0, 4);
        memset(nation, 0, 12);
        memset(birthday, 0, 20);
        memset(address, 0, 72);
        memset(number, 0, 40);
        memset(department, 0, 32);
        memset(timeLimit, 0, 36);
        memset(passport, 0, 20);
        memset(issue, 0, 6);
        memset(Image, 0, 256);
        memset(FingerData, 0, 1024);
        iFingerDataLen = 0;
        memset(IDType, 0, 2);
    }

}IDInfoGAT_GBK;

/**********************************************************************************************************************
* 台湾通行证信息结构体宏定义
**********************************************************************************************************************/
typedef struct
{
    unsigned posFront[2];
    unsigned posRear[2];
    char name[32];			//姓名
    char sex[4];			//性别
    char birthday[20];		//出生日期
    char timeLimit[36];		//有效日期
    char department[32];	//签发机关
    char addr[32];			//签发地点
    char number[64];		//证件号码
    char changeTime[32];	//换证次数
    char idNum[64];			//身份证号码
    char no[260];			//序列号
    char rev0[32];
    char rev1[32];
}TWCardInfo;
#define Null_TWCardInfo	{{0}, {0}, {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}}

typedef struct TWCardInfo_GBK
{
    unsigned posFront[2];	//正面中间的"陆"字左上角坐标，单位：毫米
    unsigned posRear[2];	//反面左边的"台"字左上角坐标，单位：毫米
    char name[32];			//姓名
    char sex[4];			//性别
    char birthday[20];		//出生日期
    char timeLimit[36];	//有效日期
    char department[32];	//签发机关
    char addr[32];			//签发地点
    char number[64];		//证件号码
    char changeTime[32];	//换证次数
    char idNum[64];		//身份证号码
    char no[260];			//序列号
    char rev0[32];
    char rev1[32];
    TWCardInfo_GBK()
    {
        memset(posFront, 0, 2);
        memset(posRear, 0, 2);
        memset(name, 0, 32);
        memset(sex, 0, 4);
        memset(birthday, 0, 20);
        memset(timeLimit, 0, 36);
        memset(department, 0, 32);
        memset(addr, 0, 32);
        memset(number, 0, 64);
        memset(changeTime, 0, 32);
        memset(idNum, 0, 64);
        memset(no, 0, 260);
        memset(rev0, 0, 32);
        memset(rev1, 0, 32);
    }
}TWCardInfo_GBK;

/**********************************************************************************************************************
* 港澳通行证信息结构体宏定义
**********************************************************************************************************************/
typedef struct
{
    unsigned posFront[2];
    unsigned posRear[2];
    char no[64];			//序号
    char name[32];			//姓名
    char sex[4];			//性别
    char birthday[20];		//出生日期
    char timeLimit[36];		//有效日期
    char department[32];	//签发机关
    char addr[32];			//签发地点
    char number[64];		//证件号码
    char bar0[128];			//条码0
    char bar1[128];			//条码1
    char rev0[32];
    char rev1[32];
}HKMacaoCardInfo;
#define Null_HKMacaoCardInfo {{0}, {0}, {0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0},{0}}

typedef struct HKMacaoCardInfo_GBK
{
    unsigned posFront[2];	//正面中间的"澳"字左上角坐标，单位：毫米
    unsigned posRear[2];	//反面左边的"往"字左上角坐标，单位：毫米
    char no[64];			//序号
    char name[32];			//name
    char sex[4];			//性别
    char birthday[20];		//出生日期
    char timeLimit[36];	//有效日期
    char department[32];	//签发机关
    char addr[32];			//签发地点
    char number[64];		//证件号码
    char bar0[128];		//条码0
    char bar1[128];		//条码1
    char rev0[32];
    char rev1[32];
    HKMacaoCardInfo_GBK()
    {
        memset(posFront, 0, 2);
        memset(posRear, 0, 2);
        memset(no, 0, 64);
        memset(name, 0, 32);
        memset(sex, 0, 4);
        memset(birthday, 0, 20);
        memset(timeLimit, 0, 36);
        memset(department, 0, 32);
        memset(addr, 0, 32);
        memset(number, 0, 64);
        memset(bar0, 0, 128);
        memset(bar1, 0, 128);
    }
}HKMacaoCardInfo_GBK;

/**********************************************************************************************************************
* 设备状态结构体宏定义
**********************************************************************************************************************/
typedef struct
{
    int iStatusProcess;
    int iStatusCoverOpen;
    int iStatusPowerOff;
    int iStatusLowVoltage;
    int iStatusInputSensorHaveCard;
    int iStatusCardJam;
    int iStatusBoot;
    int iStatusMiddleSensorHaveCard;
    int iStatusScanSensorHaveCard;
}DEVSTATUS;

/**********************************************************************************************************************
* 返回值宏定义
**********************************************************************************************************************/
#define	IDDIGITALCOPIER_NO_ERROR					0x00								//正常
#define	IDDIGITALCOPIER_NO_DEVICE					(IDDIGITALCOPIER_NO_ERROR + 0x01)	//无设备
#define	IDDIGITALCOPIER_PORT_ERROR					(IDDIGITALCOPIER_NO_ERROR + 0x02)	//端口错误
#define	IDDIGITALCOPIER_TABPAR_NONE					(IDDIGITALCOPIER_NO_ERROR + 0x03)	//参数文件错误
#define	IDDIGITALCOPIER_HAVE_NOT_INIT				(IDDIGITALCOPIER_NO_ERROR + 0x04)	//未初始化
#define	IDDIGITALCOPIER_INVALID_ARGUMENT			(IDDIGITALCOPIER_NO_ERROR + 0x05)	//无效参数
#define	IDDIGITALCOPIER_TIMEOUT_ERROR				(IDDIGITALCOPIER_NO_ERROR + 0x06)	//超时错误
#define	IDDIGITALCOPIER_STATUS_COVER_OPENED			(IDDIGITALCOPIER_NO_ERROR + 0x07)	//上盖打开
#define	IDDIGITALCOPIER_STATUS_PASSAGE_JAM			(IDDIGITALCOPIER_NO_ERROR + 0x08)	//塞卡
#define	IDDIGITALCOPIER_OUT_OF_MEMORY				(IDDIGITALCOPIER_NO_ERROR + 0x09)	//内存溢出
#define	IDDIGITALCOPIER_NO_ID_DATA			        (IDDIGITALCOPIER_NO_ERROR + 0x0A)	//没有二代证数据
#define	IDDIGITALCOPIER_NO_IMAGE_DATA				(IDDIGITALCOPIER_NO_ERROR + 0x0B)	//没有图像数据
#define	IDDIGITALCOPIER_IMAGE_PROCESS_ERROR			(IDDIGITALCOPIER_NO_ERROR + 0x0C)	//图像处理错误
#define	IDDIGITALCOPIER_IMAGE_JUDGE_DIRECTION_ERROR	(IDDIGITALCOPIER_NO_ERROR + 0x0D)	//判断图像方向错误
#define IDDIGITALCOPIER_CLOSE_FAILED				(IDDIGITALCOPIER_NO_ERROR + 0x0E)	//关闭端口失败
#define	IDDIGITALCOPIER_IDDATA_PROCESS_ERROR		(IDDIGITALCOPIER_NO_ERROR + 0x0F)	//身份证电子信息处理错误
#define IDDIGITALCOPIER_SENSORVALIDATE	            (IDDIGITALCOPIER_NO_ERROR + 0x10)	//传感器校验错误
#define IDDIGITALCOPIER_VOLTAGE_LOW					(IDDIGITALCOPIER_NO_ERROR + 0x11)	//电压低
#define IDDIGITALCOPIER_CIS_CORRECTION_ERROR	    (IDDIGITALCOPIER_NO_ERROR + 0x12)	//校正错误
#define IDDIGITALCOPIER_NO_CARD						(IDDIGITALCOPIER_NO_ERROR + 0x13)	//无卡
#define IDDIGITALCOPIER_FIRMWARE_ERROR				(IDDIGITALCOPIER_NO_ERROR + 0x14)	//未知错误
#define IDDIGITALCOPIER_SAVE_IMAGE_ERROR			(IDDIGITALCOPIER_NO_ERROR + 0x15)	//保存位图错误
#define	IDDIGITALCOPIER_POWER_OFF					(IDDIGITALCOPIER_NO_ERROR + 0x16)	//掉电错误
#define	IDDIGITALCOPIER_INPUT_BOOT					(IDDIGITALCOPIER_NO_ERROR + 0x17)	//BOOT错误
#define	IDDIGITALCOPIER_BUTTON_UP					(IDDIGITALCOPIER_NO_ERROR + 0x18)	//按键抬起
#define	IDDIGITALCOPIER_RECOGNISE_FAILED			(IDDIGITALCOPIER_NO_ERROR + 0x19)	//识别错误
#define IDDIGITALCOPIER_SCAN_ERROR					(IDDIGITALCOPIER_NO_ERROR + 0x1A)	//扫描错误
#define IDDIGITALCOPIER_FEED_ERROR					(IDDIGITALCOPIER_NO_ERROR + 0x1B)	//走卡错误
#define	IDDIGITALCOPIER_MAX_CODE					(IDDIGITALCOPIER_NO_ERROR + 0x1C)	//最大错误码

/**********************************************************************************************************************
* 设备名称、ID结构体
**********************************************************************************************************************/
typedef struct ScannerInfoRec
{
    unsigned	DeviceID;
}ScannerInfoRec;
#define Null_ScannerInfoRec {0}//{0x00},

/**********************************************************************************************************************
* 动态库输出函数
**********************************************************************************************************************/
#ifdef __cplusplus
extern "C" {
#endif

//枚举扫描设备
typedef int (*mEnumScannerDevice)(ScannerInfoRec *ScannerInfo, unsigned *DeviceNumber);

//打开设备
typedef int (*mOpenConnection)(unsigned DeviceID);

//关闭设备
typedef int (*mCloseConnection)(unsigned DeviceID);

//检测是否放入卡
typedef int (*mCheckHaveIdCard)(unsigned DeviceID, int CheckTime);

//检测是否被取走
typedef int (*mTakeOutIdCard)(unsigned DeviceID, int CheckTime);

//启动扫描
typedef int (*mStartScanIdCard)(unsigned DeviceID);

//读取当前图像数据块
typedef int (*mSavePicToMemory)(unsigned DeviceID, char* cFrontImgBuf,char* cRearImgBuf, int* iFrontLen, int* iRearLen);

//保存图像数据块到文件
typedef int (*mSavePicToFile)(unsigned DeviceID, char *cBmpBuf, int iBufLen,char *FileName, int Format);
typedef int (*mSavePicToFileII)(unsigned DeviceID, char *cBmpBuf, int iBufLen, char *FileName, int Format, float zoomScale);

//吞卡
typedef int (*mRetainIdCard)(unsigned DeviceID);

//退卡
typedef int (*mBackIdCard)(unsigned DeviceID);

//出卡并持卡
typedef int (*mBackAndHoldIdCard)(unsigned DeviceID);

//获取二代证信息
typedef int (*mGetID2Info)(unsigned DeviceID, IDInfo *IDCard, char *HeadImageName);

//获取二代证信息
typedef int (*mGetID2InfoEx)(unsigned DeviceID, IDInfoEx *IDCard, char *HeadImageName);
typedef int (*mGetIDInfoForeign)(unsigned DeviceID, IDInfoForeign *IDCard, char *HeadImageName);
typedef int (*mGetIDInfoGAT)(unsigned DeviceID, IDInfoGAT *IDCard, char *HeadImageName);
typedef int (*mGetIDCardType)(unsigned DeviceID,int* iCardType);
typedef int (*mGetAllTypeIdInfo)(unsigned DeviceID,int iCardType,void* idinfo,char* ImageName);

//获取最近一次的错误码
typedef int (*mGetLastErrorCode)();

//获取最近一次的错误描述
typedef int (*mGetLastErrorStr)(char* errStr);

//获取固件版本信息
typedef int (*mGetFWVersion)(unsigned DeviceID,char *cVersionInfo);

//获取软件版本信息
typedef int (*mGetSWVersion)(char *cVersionInfo);

//获取设备状态
typedef int (*mGetDevStatus)(unsigned DeviceID,DEVSTATUS *DevStatus);

//复位设备
typedef int (*mResetDevice)(unsigned DeviceID);

//软复位
typedef int (*mSoftResetDevice)(unsigned DeviceID,int iMode);

//固件升级
typedef int (*mUpdateOnLine)(unsigned DeviceID, char *FileName);

//CIS校验
typedef int (*mCISCalibrate)(unsigned DeviceID);

//传感器校验
typedef int (*mSensorCalibrate)(unsigned DeviceID);

//设置按键强制退卡使能
typedef int (*mSetButtonEnable)(unsigned DeviceID,int iMode);

//设置自动进卡使能
typedef int (*mSetAutoFeedEnable)(unsigned DeviceID, int iMode);

//设置上电、复位初始化吸卡模式
typedef int (*mSetInitFeedMode)(unsigned DeviceID,int iMode);

//设置二代证芯片图像存储格式
typedef int (*mSetHeadFileFormat)(unsigned DeviceID, int Format);

//通过图像获取通行证信息
typedef int (*mGetID2InfoFromImage)(unsigned DeviceID, char* cFrontImgBuf, int iFrontLen, char* cRearImgBuf, int iRearLen, IDInfo *mIDInfo);

//从图像获取二代证信息
typedef int (*mGetPassportInfoFromImage)(unsigned DeviceID, char* cFrontImgBuf, int iFrontLen, char* cRearImgBuf, int iRearLen,
                                         unsigned* cardStyle, TWCardInfo* TWCard, HKMacaoCardInfo *HKMacaoCard);
//退卡到识别位置
typedef int (*mBackIdCardToRerec)(unsigned DeviceID);
#ifdef __cplusplus
}
#endif

#endif


