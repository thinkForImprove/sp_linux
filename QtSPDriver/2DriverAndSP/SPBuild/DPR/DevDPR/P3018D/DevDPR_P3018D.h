/***************************************************************
* 文件名称：DevDPR_P3018D.h
* 文件描述：P3018D打印模块功能处理接口封装 头文件
*         用于处理XFS_XXX与SDK之间数据转换和衔接
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年8月23日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVDPR_P3018D_H
#define DEVDPR_P3018D_H

#include <time.h>
#include <iostream>
#include <sstream>

#include "IDevPTR.h"
#include "DevDPR.h"
#include "def.h"
#include "DevImpl_P3018D.h"
#include "QtTypeInclude.h"
#include "cjson_object.h"
#include "data_convertor.h"

#define LOG_NAME_DEVDPR     "DevDPR_P3018D.log"

#define JSON_GET(JSON, KEY, VALUE) \
    if (JSON.Get(KEY, VALUE) != true) \
    {\
        Log(ThisModule, __LINE__, "打印字串: ->JSON.Get(%s) Fail, JSON=[%s],Return %s.", \
            KEY, VALUE.c_str(), ERR_PTR_JSON_ERR); \
        return ERR_PTR_JSON_ERR; \
    }

#define SET_KEYNAME(KEY, CNT, DEST) \
    memset(DEST, 0x00, sizeof(DEST)); \
    sprintf(DEST, "%s%d", KEY, CNT);
//-------------------------------------------------------------------------------
class CDevDPR_P3018D : public IDevPTR, public CLogManage
{

public:
    CDevDPR_P3018D();
    ~CDevDPR_P3018D();

public:    // IDevPTR定义接口实现
    virtual void Release()                                                              // 释放接口
    {
        return;
    }
    virtual int Open(const char* pMode);                                                     // 打开与设备的连接
    virtual int Close();                                                                // 关闭与设备的连接
    virtual int Init()                                                                  // 设备初始化
    {
        return PTR_SUCCESS;
    }
    virtual int Reset();                                                                // 设备复位
    virtual int ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam)                // 设备复位
    {
        return PTR_SUCCESS;
    }
    virtual int GetStatus(DEVPTRSTATUS &stStatus);                                      // 取设备状态
    virtual int PrintData(const char *pStr, unsigned long ulDataLen)                    // 打印字串(无指定打印坐标)
    {
        return PTR_SUCCESS;
    }
    virtual int PrintDataOrg(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY)
    {
        return PTR_SUCCESS;
    }
    virtual int PrintImage(const char *szImagePath, unsigned long ulWidth, unsigned long ulHeight) // 图片打印(无指定打印坐标)
    {
        return PTR_SUCCESS;
    }
    virtual int PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY) // 指定坐标打印图片
    {
        return PTR_SUCCESS;
    }
    virtual int ReadForm(DEVPTRREADFORMIN stReadIn, DEVPTRREADFORMOUT &stReadOut)       // ReadForm获取
    {
        return PTR_SUCCESS;
    }
    virtual int ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut)  // ReadImage获取
    {
        return PTR_SUCCESS;
    }
    virtual int MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam = 0)       // 介质控制
    {
        return PTR_SUCCESS;
    }
    virtual int SetData(void *vData, WORD wDataType = 0);                                // 设置数据
    virtual int GetData(void *vData, WORD wDataType = 0);                                // 获取数据
    virtual void GetVersion(char* szVer, long lSize, ushort usType);                     // 获取版本

private:
    INT     ConvertErrorCode(INT nRet);                                                 // IMPL错误码转换为DPR错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);                                              // DPR错误码含义
    void    DiffDevStat(SCANNERSTATUS stStat, SCANNERSTATUS stOLD);
    CHAR*   ConvertImplErrCodeToStr(INT nRet);                                          // IMPL错误码含义
    USHORT  CheckSleepTimeisVaild(USHORT time);                                         // 检查是否为有效的睡眠时间
    void    GetDeviceInfo(LPSTR json, DEVPTRSTATUS& dev);                               // 解析底层返回的设备状态
    WORD    StringToWORD(std::string str);                                              // 字符串转WORD
    DWORD   StringToDWORD(std::string str);                                             // 字符串转DWORD
    BYTE    SwitchDevStat(LPBYTE stat);                                                 // 设备状态:字符串转换为整型

private:
    CSimpleMutex                m_cMutex;
    CDevImpl_P3018D             m_devP3018D;
    CHAR                        m_szErrStr[1024];

    std::string                 m_stdOpenMode;

    CJsonObject                 m_JsonData;
    std::string                 m_stdJsonText;
    std::string                 m_stdJsonPic;    

    USHORT                      m_usDPIx;                                               // X方向DPI
    USHORT                      m_usDPIy;                                               // y方向DPI
    USHORT                      m_usSleep;                                              // 睡眠时间

    INT                         m_nGetStatErrOLD;                                       // 取状态接口上一次错误码
    INT                         m_nGetOpenErrOLD;

    HANDLE                      m_handle;                                               // 打印机句柄
    LPSTR                       m_szJson;                                               // 输入/出打印机数据
    std::string                 m_stdDevName;                                           // 打印机名
};

#endif // DEVDPR_P3018D_H
