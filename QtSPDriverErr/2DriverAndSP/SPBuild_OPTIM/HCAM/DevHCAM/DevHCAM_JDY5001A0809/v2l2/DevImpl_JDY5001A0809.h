/***************************************************************
* 文件名称: DevImpl_JDY5001A0809.h
* 文件描述: 封装摄像模块底层指令,提供控制接口 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年6月6日
* 文件版本: 1.0.0.1
****************************************************************/
#pragma once

#ifndef DEVIMPL_JDY5001A0809_H
#define DEVIMPL_JDY5001A0809_H

#include "QtTypeDef.h"
#include "ILogWrite.h"
#include "QtDLLLoader.h"

using namespace std;

/*************************************************************
// 返回值/错误码　宏定义
// <0 : USB/COM接口处理返回
// 0~100: 硬件设备返回
// > 100: Impl处理返回
*************************************************************/
// > 100: Impl处理返回
#define IMP_SUCCESS                 0               // 成功
#define IMP_ERR_PARAM_INVALID       -9902           // 参数无效
#define IMP_ERR_UNKNOWN             -9909           // 未知错误

#define IMP_ERR_OPEN_FAIL               1               // Open失败


/*************************************************************
// 无分类　宏定义
*************************************************************/
#define LOG_NAME                        "DevImpl_JDY5001A0809.log"   // 缺省日志名

enum EN_DEVSTAT
{
    DEV_OK          = 0,    // 设备正常
    DEV_NOTOPEN     = 1,    // 设备未打开
};


/*************************************************************
// 封装类: 命令编辑、发送接收等处理。
*************************************************************/
class CDevImpl_JDY5001A0809 : public CLogManage
{
public:
    CDevImpl_JDY5001A0809();
    CDevImpl_JDY5001A0809(LPSTR lpLog);
    CDevImpl_JDY5001A0809(LPSTR lpLog, LPCSTR lpDevType);
    virtual ~CDevImpl_JDY5001A0809();

public:
    INT     OpenDevice(LPSTR lpMode);                               // 打开设备
    INT     CloseDevice();                                          // 关闭设备
    BOOL    IsDeviceOpen();                                         // 设备是否Open
    INT     ResetDevice();                                          // 复位设备
    INT     GetDeviceStatus();                                      // 取设备状态
    INT     SetVideoCaptureMode(INT nWidth, INT nHeight);           // 设置视频捕获模式


private:
    void    Init();                                                 // 参数初始化
    INT     SetReConFlag(BOOL bFlag);                               // 设置断线重连标记
    LPSTR   ConvertCode_Impl2Str(INT nErrCode);                     // Impl错误码转换解释字符串

private:
    CHAR            m_szDevType[64];                                // 设备类型
    INT             m_nDevHandle;                                   // 设备句柄
    BOOL            m_bDevOpenOk;                                   // 设备是否Open
    BOOL            m_bReCon;                                       // 是否断线重连状态
    INT             m_nRetErrOLD[8];                                // 处理错误值保存(0:USB动态库/1:设备连接/
                                                                    //  2:设备初始化/3/4)

};


#endif // DEVIMPL_JDY5001A0809
