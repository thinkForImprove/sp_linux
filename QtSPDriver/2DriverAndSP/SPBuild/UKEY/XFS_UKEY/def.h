#ifndef DEF_H
#define DEF_H

#include "QtTypeDef.h"

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义

// CRD 版本号
static const BYTE    byDevVRTU[17] = {"DevCRD00000100"};

// 设备类型(UKEY模块)
#define IDEVUKEY_TYPE_ACTU6SS39         "ACT-U6-SS39"       // 驰卡ACT-U6-SS39
#define IDEVUKEY_TYPE_ACTU6SG5          "ACT-U6-SG5"        // 驰卡ACT-U6-SG5

#define IXFSUKEY_TYPE_ACTU6SS39         0                   // 驰卡ACT-U6-SS39
#define IXFSUKEY_TYPE_ACTU6SG5          1                   // 驰卡ACT-U6-SG5

// 设备型号与程序内编号对应 定义
#define DEVTYPE_CHG(n)      (n == IXFSUKEY_TYPE_ACTU6SS39 ? IDEVUKEY_TYPE_ACTU6SS39 : \
                             (n == IXFSUKEY_TYPE_ACTU6SG5 ? IDEVUKEY_TYPE_ACTU6SG5 : ""))

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define DTYPE_LIB_PATH                  51
#define DTYPE_SET_SNDTIMEOUT            52      // 设置报文发送超时时间
#define DTYPE_SET_RCVTIMEOUT            53      // 设置报文接收超时时间
#define DTYPE_SET_POWEROFFMODE          54      // 掉电处理模式
#define DTYPE_GET_DEVPARAM              55      // 取机器参数
#define DTYPE_SCANQR_OPEN               60      // 打开扫描二维码功能
#define DTYPE_SCANQR_CLOSE              61      // 关闭扫描二维码功能
#define DTYPE_SCANBAR_OPEN              62      // 打开扫描条形码功能
#define DTYPE_SCANBAR_CLOSE             63      // 关闭扫描条形码功能
#define DTYPE_GET_UKEYNO                60      // 读UKEY编号

// DevXXX: DispenseCard: 命令参数
#define DC_BOX2FSCAN        0       // UKey从箱发到前端扫描位置
#define DC_BOX2DOOR         1       // UKey从箱直接发到出口
#define DC_BOX2FSCAN_NOS    2       // UKey从箱发到前端扫描位置,不扫描

// DevXXX: EjectCard: 命令参数
#define DC_FSCAN2DOOR       0       // UKey从前端扫描位置箱发出口
#define DC_DOOR2FSCAN       1       // UKey从出口发到前端扫描位置


// 设备初始化参数
typedef struct st_DevInit_Param
{
    INT nParam[32];
    st_DevInit_Param()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_DevInit_Param));
    }
} STDEVINITPAR, *LPSTDEVINITPAR;



#endif // DEF_H
