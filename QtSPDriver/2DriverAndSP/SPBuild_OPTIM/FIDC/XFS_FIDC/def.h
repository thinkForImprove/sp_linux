/***************************************************************
* 文件名称：def.h
* 文件描述：用于声明 XFS_XXX 与 DevXXX 共用的变量定义
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年11月6日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

//----------宏定义----------------------------------------------------

static const BYTE byXFSVRTU[17] = {"FIDC00020100"};     // XFS_FIDC 版本号
static const BYTE byDevVRTU[17] = {"Dev020100"};        // DevFIDC 版本号

// 设备类型
#define XFS_MT50                0                       // 明泰MT50
#define XFS_CJ201               1                       // CJ201
#define XFS_TMZ                 2                       // 天梦者
#define XFS_CRT603CZ7           3                       // 创自CRT-603-CZ7-6001

#define IDEV_MT50               "MT50"                  //
#define IDEV_CJ201              "CJ201"                 //
#define IDEV_TMZ                "TMZ"                   //
#define IDEV_CRT603CZ7          "CRT603CZ7"             // 创自CRT-603-CZ7-6001



// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_IMAGE_PAR           52                      // 设置图像参数

// GetVersion()使用
#define GET_VER_FW              1                       // 固件版本号

//----------------------鸣响相关定义-----------------------
typedef struct st_Config_Beep
{
    WORD    wSupp;              // 是否支持鸣响
    WORD    wCont;              // 控制方式
    WORD    wMesc;              // 一次鸣响时间(单位:毫秒)
    WORD    wInterval;          // 每次间隔时间(单位:毫秒)
    WORD    wCount;             // 每次鸣响次数
    st_Config_Beep()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Config_Beep));
    }
} STCONFIGBEEP, *LPSTCONFIGBEEP;


//----------------------指示灯相关定义-----------------------
typedef struct st_Config_Light
{
    WORD    wSupp;              // 是否支持
    WORD    wFaseDelayTime;     // 快速闪烁时间(单位:毫秒)
    WORD    wMiddleDelayTime;   // 中速闪烁时间(单位:毫秒)
    WORD    wSlowDelayTime;     // 慢速闪烁时间(单位:毫秒)
    WORD    wCont;              // 控制方式

    st_Config_Light()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_Config_Light));
    }
} STCONFIGLIGHT, *LPSTCONFIGLIGHT;

//----------------------图像相关定义-----------------------
typedef struct ST_IMAGE_PARAM
{
    WORD    wIDCCardType;                       // 证件图像保存类型
    CHAR    szIDCardImgSavePath[256];           // 证件图像存放位置
    CHAR    szIDCardImgName[256];               // 证件图像名(空不使用)

    ST_IMAGE_PARAM()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(ST_IMAGE_PARAM));
    }
} STIMAGEPAR, *LPSTIMAGEPAR;

#endif // DEF_H
