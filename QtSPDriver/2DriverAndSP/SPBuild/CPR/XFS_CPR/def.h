#ifndef DEF_H
#define DEF_H

#include "QtTypeDef.h"

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义

//----------宏定义----------------------------------------------------

static const BYTE byXFSVRTU[17] = {"CPR00010100"};      // XFS_CPR 版本号
static const BYTE byDevVRTU[17] = {"Dev010100"};        // DevPTR 版本号

// 设备类型(票据售卖模块)
#define IDEVCPR_TYPE_BT8500M       "BT8500M"        // 新北洋BT-8500M票据打印机

#define IXFSCPR_TYPE_BT8500M        0               // 新北洋BT-8500M票据打印机

// 设备型号与程序内编号对应 定义
#define DEVTYPE_CHG(n)      (n == IXFSCPR_TYPE_BT8500M ? IDEVCPR_TYPE_BT8500M : "")

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define DTYPE_DPIx          51
#define DTYPE_DPIy          52
#define DTYPE_FONT          53
#define DTYPE_BANKNO        54      // 银行编号
#define DTYPE_NOTE_TYPE     55      // 票据类型

// 银行编号 定义
#define BANK_NO_ALL         0                   // INI指定银行编号:通用
#define BANK_NO_CSBC        1                   // INI指定银行编号:长沙银行
#define BANK_NO_PSBC        2                   // INI指定银行编号:邮储
#define BANK_NO_JIANGNAN    3                   // INI指定银行编号:江南银行

// 票据类别(INI中指定对应箱专用)
// 不同行/不同硬件对应票据类别通过以下值做转换
#define NOTE_TYPE_INV               0       // 未知
#define NOTE_TYPE_PTCD              1       // 普通存单
#define NOTE_TYPE_XPCD              2       // 芯片存单
#define NOTE_TYPE_DECD              3       // 大额存单
#define NOTE_TYPE_GZPZ              4       // 国债凭证
#define NOTE_TYPE_JSYWWTS           5       // 结算业务委托书
#define NOTE_TYPE_XJZP              6       // 现金支票
#define NOTE_TYPE_ZZZP              7       // 转账支票
#define NOTE_TYPE_QFJZP             8       // 清分机支票
#define NOTE_TYPE_YHHP              9       // 银行汇票
#define NOTE_TYPE_YHCDHP            10      // 银行承兑汇票
#define NOTE_TYPE_SYCDHP            11      // 商业承兑汇票
#define NOTE_TYPE_FQFJBP            12      // 非清分机本票
#define NOTE_TYPE_QFJBP             13      // 清分机本票

// JSON 相关
#define JSON_KEY_USE_AREA           "UseJsonArea"   // 是否使用JSON
#define JSON_KEY_IMAGE_FRONT_PATH   "ImageFront"    // 正面图像路径
#define JSON_KEY_IMAGE_BACK_PATH    "ImageBack"     // 背面图像路径
#define JSON_KEY_RFID_DATA          "RFIDData"      // RFID数据
#define JSON_KEY_NOTE_TYPE          "NoteType"      // 票据类型
#define JSON_KEY_IDEN_CNT           "IdenCount"     // 项数目
#define JSON_KEY_IDEN_NAME          "IdenName"      // 项名
#define JSON_KEY_IDEN_VALUE         "IdenValue"     // 项值
#define JSON_KEY_IDEN_TYPE_TEXT     "IdenText"      // 项类型:文本
#define JSON_KEY_IDEN_TYPE_PIC      "IdenPic"       // 项类型:图片
#define JSON_KEY_IDEN_TYPE_BAR      "IdenBar"       // 项类型:条码
#define JSON_KEY_IDEN_TYPE_MICR     "IdenMicr"      // 项类型:磁码
#define JSON_KEY_MEDIA_WIDTH        "MediaWidth"    // 介质宽(单位:MM)
#define JSON_KEY_MEDIA_HEIGHT       "MediaHeight"   // 介质高(单位:MM)
#define JSON_KEY_START_X            "StartX"        // 起始坐标X(单位:MM)
#define JSON_KEY_START_Y            "StartY"        // 起始坐标Y(单位:MM)
#define JSON_KEY_AREA_WIDTH         "AreaWidth"     // 可用宽(单位:MM)
#define JSON_KEY_AREA_HEIGHT        "AreaHeight"    // 可用高(单位:MM)
#define JSON_KEY_PIC_ZOOM           "PicZoom"       // 图片缩放比例
#define JSON_KEY_TEXT_FONT          "TextFont"      // 文本字体
#define JSON_KEY_TIMEOUT            "TimeOut"       // 超时时间(单位:毫秒)

#define JSON_KEY_ACCOUNT            "Account"       // 账号识别
#define JSON_KEY_AMTINFIG           "AmountInFigures"//
#define JSON_KEY_CHECKNO            "CheckNo"       //

// JSON例1: 适用于获取数据方式
// { "MediaWidth":XX,"MediaHeight"=XX,"UseJsonArea"=X,"ImageFront"="XXX","ImageBack"="XX",
//   "IdenCount"=XX,"IdenName"="XXX","IdenValue"="XXX","IdenName1"="XXX","IdenValue1"="XXX" ｝
// JSON例2: 适用于打印数据方式
// { "MediaWidth":XX,"MediaHeight"=XX,
//   "IdenText":{"IdenCount"=XX,"IdenName"="XXX","IdenValue"="XXX","IdenName1"="XXX","IdenValue1"="XXX"}}   // 打印文本
//   "IdenPic":{"IdenCount"=XX,"IdenName"="XXX","IdenValue"="XXX","IdenName1"="XXX","IdenValue1"="XXX"}}    // 打印图片
// }

#endif // DEF_H
