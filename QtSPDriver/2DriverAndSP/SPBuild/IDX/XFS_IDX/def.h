#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义


// 设备类型
#define IDEV_TYPE_BSID81            "0"     // BS-ID81

#define DEV_TYPE_BSID81             0       // BS-ID81

#define DEVTYPE_CHG(a)  a == 0 ? IDEV_TYPE_BSID81 : ""

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define DATATYPE_INIT               0   // 初始化数据
#define DATATYPE_ResetDevice        1   // 复位设备(BSID81)
#define DATATYPE_SetDevIsNotOpen    2   // 设置设备Open标记=NotOpen
#define DTYPE_LIB_PATH              3   // 设置/获取动态库路径
#define DTYPE_LIB_VER               4   // 设置/获取动态库版本
#define SET_DEV_RECON               51  // 设置断线重连标记


// GetVersion()使用
#define GET_VER_DEVRPR          1               // DevMSR版本号
#define GET_VER_FW              2               // 固件版本号

// DEVIDX初始化参数结构体
typedef struct st_idx_dev_init_param
{
    WORD    wBankNo;                // 银行编号
    CHAR    szHeadImgSavePath[256]; // 证件头像存放位置
    CHAR    szScanImgSavePath[256]; // 扫描图像存放位置
    CHAR    szHeadImgName[256];     // 头像名(空不使用)
    CHAR    szScanImgFrontName[256];// 证件正面图像名(空不使用)
    CHAR    szScanImgBackName[256]; // 证件背面图像名(空不使用)
    WORD    wScanImgSaveType;       // 证件扫描图像保存类型
    FLOAT   fScanImgSaveZoomSc;     // 证件扫描图像保存图片缩放比例
    WORD    wEjectMode;             // 退卡动作模式(0保留在门口/1完全弹出)
    WORD    wScanImageFrontIsInfor; // 证件扫描图像是否以人像信息面为正面

    st_idx_dev_init_param()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_idx_dev_init_param));
    }
} STIDXDEVINITPARAM, *LPSTIDXDEVINITPARAM;


#endif // DEF_H
