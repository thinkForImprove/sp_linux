#ifndef DEVPPR_MB2_H
#define DEVPPR_MB2_H

#include <time.h>

#include "IDevPPR.h"
#include "DevPPR.h"
#include "DevImpl_MB2.h"
#include "QtTypeInclude.h"
#include <cjson_object.h>
#include "../XFS_PPR/def.h"
#include "../MB2/MB2Def.h"

#define LOG_NAME_DEVPPR     "DevPPR_MB2.log"

//-------------------------------------------------------------------------------
class CDevPPR_MB2 : public IDevPPR, public CLogManage
{

public:
    CDevPPR_MB2(LPCSTR lpDevType);
    virtual ~CDevPPR_MB2();

public:    
    virtual void Release();                                                             // 释放接口
    virtual int Open(const char *pMode);                                                // 打开与设备的连接
    virtual int Close();                                                                // 关闭与设备的连接
    virtual int Init();                                                                 // 设备初始化
    virtual int Reset();                                                                // 设备复位
    virtual int ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam = 0);
    virtual int GetStatus(DEVPTRSTATUS &stStatus);                                      // 取设备状态
    virtual int PrintData(const char *pStr, unsigned long ulDataLen);                   // 打印字串(无指定打印坐标)
    virtual int PrintDataOrg(const char *pStr, unsigned long ulDataLen, unsigned long ulOrgX, unsigned long ulOrgY);
    virtual int PrintImage(char *szImagePath, unsigned long ulWidth, unsigned long ulHeight); // 图片打印(无指定打印坐标)
    virtual int PrintImageOrg(const char* szImagePath, unsigned long ulOrgX, unsigned long ulOrgY); // 指定坐标打印图片
    virtual int ReadForm(DEVPTRREADFORMIN stScanIn, DEVPTRREADFORMOUT &stScanOut);
    virtual int ReadForm2(DEVPTRREADFORMIN stScanIn, DEVPTRREADFORMOUT &stScanOut, LONG lMagenticType);
    virtual int ReadImage(DEVPTRREADIMAGEIN stScanIn, DEVPTRREADIMAGEOUT &stScanOut);   // 扫描信息获取
    virtual int MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam = 0);      // 介质控制
    virtual int MeidaControl2(MEDIA_ACTION2 enMediaAct, unsigned short usParam = 0);      // 介质控制
    virtual int SetData(void *vData, WORD wDataType = 0);                               // 设置数据
    virtual int GetData(void *vData, WORD wDataType = 0);                               // 获取数据
    virtual void GetVersion(char* szVer, long lSize, ushort usType);                    // 获取版本
//    virtual int ReadFields(DEVPTRREADIMAGEIN stScanIn, DEVPTRREADIMAGEOUT &stScanOut);   //读取磁道数据
    virtual int RetractMedia();                                                            //回收
    virtual long CmdCancel();
    virtual void vSetCancelFlg(BOOL);                //test#30
private:
    INT     ConvertErrorCode(INT nRet);                                         // IMPL错误码转换为PPR错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);                                      // PPR错误码含义

private:
    CSimpleMutex                m_cMutex;
    CDevImpl_MB2                m_devMB2;
    CHAR                        m_szErrStr[1024];

    std::string                 m_stdOpenMode;

    CJsonObject                 m_JsonData;
    std::string                 m_stdJsonText;
    std::string                 m_stdJsonPic;
    unsigned short              usDevStat;
    unsigned short              usPaperStat;
    BOOL                        m_bPrinting;
};

#endif // DEVPPR_MB2_H
