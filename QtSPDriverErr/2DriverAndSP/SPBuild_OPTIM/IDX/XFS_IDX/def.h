#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义
//----------宏定义----------------------------------------------------
static const BYTE byXFSVRTU[17] = {"IDX00020100"};      // XFS_IDX 版本号
static const BYTE byDevVRTU[17] = {"Dev020100"};        // DevIDX 版本号

// 设备类型
#define XFS_TYPE_BSID81             0       // BS-ID81
#define IDEV_TYPE_BSID81            "0"     // BS-ID81
#define XFS_TYPE_DMTM1M             1       // DMT-M1-M
#define IDEV_TYPE_DMTM1M            "1"     // DMT-M1-M


#define DEVTYPE_CHG(a) \
    a == XFS_TYPE_BSID81 ? IDEV_TYPE_BSID81 : \
    (a == XFS_TYPE_DMTM1M ? IDEV_TYPE_DMTM1M : "")

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define SET_LIB_VER                 51  // 设置动态库版本
#define SET_IMAGE_PAR               52  // 设置图像参数
#define SET_DEBUG_MODE              53  // 开启调试模式


// 证件图像参数结构体
typedef struct st_idx_image_param
{
    WORD    wBankNo;                // 银行编号
    CHAR    szHeadImgSavePath[256]; // 证件头像存放位置
    CHAR    szScanImgSavePath[256]; // 扫描图像存放位置
    CHAR    szHeadImgName[256];     // 头像名(空不使用)
    CHAR    szScanImgFrontName[256];// 证件正面图像名(空不使用)
    CHAR    szScanImgBackName[256]; // 证件背面图像名(空不使用)
    WORD    wScanImgSaveType;       // 证件扫描图像保存类型
    FLOAT   fScanImgSaveZoomSc;     // 证件扫描图像保存图片缩放比例
    WORD    wScanImageFrontIsInfor; // 证件扫描图像是否以人像信息面为正面

    st_idx_image_param()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_idx_image_param));
    }
} STIMAGEPARAM, *LPSTIMAGEPARAM;


#endif // DEF_H
