/***************************************************************************
* 文件名称： TCF261.h
* 文件描述： 天诚盛业摄像相关
* 版本历史信息
* 变更说明：
* 变更日期：
* 文件版本：1.0.0.1
***************************************************************************/

#ifndef TCF261_H
#define TCF261_H


#include "QtTypeDef.h"

#ifndef CALL_MODE
    #ifdef _WIN32
        #define CALL_MODE	__stdcall
    #else
        #define CALL_MODE
    #endif
#endif

/***************************************************************************
// 无分类　宏定义
***************************************************************************/
// 设备状态
#define FR_RET_SUCC     0       // 执行成功
#define FR_RET_FAIL     -1      // 执行失败
#define FR_RET_PARA     -2      // 参数错误
#define FR_RET_NDEV     -3      // 打开设备失败
#define FR_RET_NINI     -4      // 未打开设备
#define FR_RET_BUSY     -5      // 设备繁忙
#define FR_RET_DATA     -6      // 图像数据不正确
#define FR_RET_NLNK     -7      // 设备断开
#define FR_RET_NDRV     -8      // 加载设备库失败
#define FR_RET_NALG     -9      // 加载算法库失败
#define FR_RET_NMEM     -10     // 内存分配失败
#define FR_RET_ARDY     -11     // 已经打开设备
#define FR_RET_AUTH     -12     // 算法授权失败
#define FP_RET_IPCF     -13     // IPC错误

// 回调返回状态
#define FR_CALLBACK_RESULT_SUCC         100     // 活体检测成功
#define FR_CALLBACK_RESULT_FAIL         101     // 活体检测失败
#define FR_CALLBACK_RESULT_TIMEOUT      102     // 活体检测超时
#define FR_CALLBACK_RESULT_ERROR        103     // 从设备取图失败
#define FR_CALLBACK_RESULT_CANCEL       104     // 活体检测取消
#define FR_CALLBACK_RESULT_NOFACE       105     // 没有检测到人脸
#define FR_CALLBACK_RESULT_MULTIFADE    106     // 检测到多个人脸
#define FR_CALLBACK_RESULT_HEADPOS      107     // 头部姿态不正常
#define FR_CALLBACK_RESULT_EMOTION      108     // 有闭眼张嘴等表情动作
#define FR_CALLBACK_RESULT_MOTIVE       109     // 有运动模糊
#define FR_CALLBACK_RESULT_BRIGHT       110     // 亮度不符合
#define FR_CALLBACK_RESULT_NOTCENTER    111     // 人脸未居中
#define FR_CALLBACK_RESULT_UNKNOW       112     // 未知错误


//----回调函数定义--------------------------------------------------------------------------
// 1. 抓拍回调
typedef int (CALL_MODE *CaptureCallBackDetectInfo)(int *nResultID, void *lpParam);

int onDetectInfo(int *nResultID, void *lpParam);

//----接口函数定义-------------------------------------------------------------------------------
// 1. 初始化SDK
typedef int (CALL_MODE *FR_CREATEFACECALLBACK)(CaptureCallBackDetectInfo pDetectInfo, const char *parentDir);
// 2. 反初始化/释放SDK
typedef int (CALL_MODE *FR_DESTROYFACECALLBACK)();
// 3. 打开人脸设备
typedef int (CALL_MODE *FR_STARTCAMERA)();
// 4. 关闭人脸设备
typedef int (CALL_MODE *FR_STOPCAMERA)();
// 5. 暂停预览画面
typedef int (CALL_MODE *FR_PAUSEANDPLAY)(bool isPause);
// 6. 抓拍一张图片
typedef int (CALL_MODE *FR_TAKEPICTRUE)(const char* strFilePath);
// 7. 开始人脸活检抓拍
typedef int (CALL_MODE *FR_STARTLIVEDETECT)(int mode, const char* strContent,  const char* strFilePath);
// 8. 停止人脸活检抓拍
typedef int (CALL_MODE *FR_STOPLIVEDETECT)();
// 9. 建立预览窗体
typedef int (CALL_MODE *FR_CREATEWINDOW)(unsigned long ulWndHandle, int iX, int iY, int iWidth, int iHeight);
// 10. 关闭预览窗体
typedef int (CALL_MODE *FR_CLOSEWINDOW)();
// 11. 比对两张人脸照片
typedef int (CALL_MODE *FR_FACECOMPARE)(const char*pccPathA, const char*pccPathB, unsigned short ScoreIn, unsigned short &ScoreOut);
// 12. 人脸摄像头状态
typedef void (CALL_MODE *FR_GETSTATUS)(int *piStatus);
// 13. 获取设备版本信息
typedef int (CALL_MODE *FR_GETFIRMWAREVERSION)(char szVersion[64]);
// 14. 未知
typedef int (CALL_MODE *FR_HIDETIPS)(bool isHide);

#endif // TCF261_H

// -------------------------------------- END --------------------------------------

