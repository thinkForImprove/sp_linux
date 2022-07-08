#pragma once

/***************************************************************
 * 文件名称：IDevBase.h
 * 文件描述：设备驱动的公共部分
 *
 * 版本历史信息
 * 变更说明：建立文件
 * 变更日期：2019年6月15日
 * 文件版本：1.0.0.1
 ****************************************************************/

#include <string.h>
#include <stdio.h>
#include <QString>

#include "QtTypeDef.h"
#include "IDEV_ERROR.h"
#include "SimpleMutex.h"
#include "AutoQtHelpClass.h"
#include "ILogWrite.h"
/////////////////////////////////////////////////////////////////////////////////////
#define MAX_LEN_BUFFER    (256)  // 缓存大小最大值
#define MAX_LEN_FWVERSION MAX_LEN_BUFFER
/////////////////////////////////////////////////////////////////////////////////////
//接口类定义
struct IDevBase
{
    // 析构
    virtual ~IDevBase() = 0;
    // 释放接口
    virtual void Release() = 0;
    // 打开与设备的连接
    virtual long Open(const char *pMode = nullptr) = 0;
    // 由上层调用传入设备选项，用来处理一些预定义动作
    virtual void SetDeviceConfig(void *pConfig) = 0;
    // 关闭与设备的连接
    virtual long Close() = 0;
    // 取消当前指令，注意：此一般是针对耗时等待的指令
    virtual long Cancel(ulong ulReasion = 0, void *pData = nullptr) = 0;
    // 复位设备
    // ulAction : 复位时的功能
    // pData 保留
    virtual long Reset(ulong ulAction = 0, void *pData = nullptr) = 0;
    // 获取固件版本
    // pFWVersion：保存固件版本
    virtual long GetFWVersion(char szFWVersion[MAX_LEN_FWVERSION]) = 0;
    // 查询设备状态
    // pStatus特定的结构体
    virtual long GetStatus(void *pStatus = nullptr) = 0;
    // 获取硬件能力
    // pCaps 设备能力，根据设备类型自定义
    virtual long GetDeviceCaps(void *pCaps) = 0;
    // 获取当前设备的编码号，该值主要用在组装错误码上
    virtual long GetDeviceCode() = 0;
    // 得到最后一次错误
    virtual long GetLastError() = 0;
    // 得到错误的说明
    // lErrorCode 需要查明的错误码
    // pAryErrorString 制定的填充区域，用来填充错误原因
    // 这里需要注意的是，返回来的数据不是UTF8的，因为从第三方驱动拿到的数据很多都不是UTF8的所有为了统一，就统一不适用UTF8
    virtual long GetErrorString(long lErrorCode, char szErrorString[MAX_LEN_BUFFER]) = 0;
    // 是否需要升级固件
    virtual long IsNeedDevicePDL() = 0;
    // 执行升级固件
    virtual long UpdateDevicePDL() = 0;
};
/////////////////////////////////////////////////////////////////////////////////////
inline IDevBase::~IDevBase() {}
