#ifndef DEF_H
#define DEF_H

// 用于声明 XFS_XXX 与 DevXXX 共用的变量定义

// 设备类型(票据售卖模块)
// 设备类型(票据售卖模块)
#define IDEVCSR_TYPE_P3018D        "P3018D"         // P3018D打印机
#define IXFSCSR_TYPE_P3018D        0                // P3018D打印机

#define JSON_DATA_SIZE              1024            // 打印机解析json字符串长度大小
// 设备型号与程序内编号对应 定义
#define DEVTYPE_CHG(n)      (n == IXFSCSR_TYPE_P3018D ? IDEVCSR_TYPE_P3018D : "")

// SetData()/GetData()使用执行类别(50以上为各模块自行定义)
#define DTYPE_DPIx          51
#define DTYPE_DPIy          52
#define DTYPE_FONT          53
#define DTYPE_SLEEP         54          // 设置睡眠时间   SetData()
#define DTYPE_DELCURJOB     55          // 删除当前打印作业 SetData()
#define DTYPE_ERRINFO       56          // 获取设备状态描述信息 GetData()
#define DTYPE_JOBNUM        57          // 获取已打印页数  GetData()

#define CONVERTFILEPATH     "/usr/local/CFES/DATA/FORM/DPR"

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

#define JSON_KEY_PRINTNAME          "PrinterName"   // 打印机名
#define JSON_KEY_TONER              "toner"         // 碳粉剩余百分比
#define JSON_KEY_LIFE               "life"          // 硒鼓剩余百分比
#define JSON_KEY_BLACK              "black"         // 黑色墨剩余百分比
#define JSON_KEY_TOTAL_PRINT        "totalprint"    // 已打印总张数
#define JSON_KEY_A4_PRINT           "a4print"       // 已打印 A4 张数
#define JSON_KEY_A5_PRINT           "a5print"       // 已打印 A5 张数
#define JSON_KEY_LESS_PAPER         "lessPaper"     // 打印机即将缺纸, “1”表示即将缺纸，“0”表示纸张足够
#define JSON_KEY_LESS_TONER         "lessToner"     // 硒鼓或粉盒即将耗尽, “1”表示即将耗尽，“0”表示硒鼓或粉盒正常

// JSON例1: 适用于获取数据方式
// { "MediaWidth":XX,"MediaHeight"=XX,"UseJsonArea"=X,"ImageFront"="XXX","ImageBack"="XX",
//   "IdenCount"=XX,"IdenName"="XXX","IdenValue"="XXX","IdenName1"="XXX","IdenValue1"="XXX" ｝
// JSON例2: 适用于打印数据方式
// { "MediaWidth":XX,"MediaHeight"=XX,
//   "IdenText":{"IdenCount"=XX,"IdenName"="XXX","IdenValue"="XXX","IdenName1"="XXX","IdenValue1"="XXX"}}   // 打印文本
//   "IdenPic":{"IdenCount"=XX,"IdenName"="XXX","IdenValue"="XXX","IdenName1"="XXX","IdenValue1"="XXX"}}    // 打印图片
// }
#endif // DEF_H
