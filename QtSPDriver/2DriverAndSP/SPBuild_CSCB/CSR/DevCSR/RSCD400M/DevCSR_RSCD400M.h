/***************************************************************
* 文件名称：DevCSR_RSCD400M.h
* 文件描述：RSC-D400M票据受理模块功能处理接口封装 头文件
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年4月4日
* 文件版本：1.0.0.1
****************************************************************/

#ifndef DEVCSR_RSCD400M_H
#define DEVCSR_RSCD400M_H

#include <time.h>

#include "IDevPTR.h"
#include "DevCSR.h"
#include "def.h"
#include "DevImpl_RSCD400M.h"
#include "QtTypeInclude.h"
#include "cjson_object.h"
#include "data_convertor.h"

#define LOG_NAME_DEVCSR     "DevCSR_RSCD400M.log"

// 支持的字体结构体定义
typedef
struct st_PrtFont
{
    CHAR szFontName[32];
    CHAR szFontPath[256];
}STPRTFONT, *LPSTPRTFONT;
typedef
struct st_PrtFont_List
{
    BOOL      bIdHave[32];
    STPRTFONT stPrtFont[32];

    st_PrtFont_List()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(st_PrtFont_List));
    }
    USHORT GetCount()
    {
        INT nCount = 0;
        for (INT i = 0; i < 32; i ++)
        {
            if (bIdHave[i] == TRUE)
            {
                nCount ++;
            }
        }
        return nCount;
    }
    CHAR* GetFontPath(CHAR* lpFontName)
    {
        INT nCount = 0;
        for (INT i = 0; i < 32; i ++)
        {
            if (bIdHave[i] == TRUE &&
                (MCMP_IS0(lpFontName, stPrtFont[i].szFontName)))
            {
                return stPrtFont[i].szFontPath;
            }
        }
        return nullptr;
    }
    void ImportFontData(const CHAR* lpData)
    {
        INT nIdx = 0;
        CHAR szAppList[32][CONST_VALUE_260];
        CHAR szKeyList[2][CONST_VALUE_260];
        INT  nAppCnt = 0, nKeyCnt = 0;

        if (lpData == nullptr)
        {
            return;
        }

        // 解析分割FontName+FontPath列表
        memset(szAppList, 0x00, sizeof(szAppList));
        nAppCnt = DataConvertor::split_string(lpData, '|', szAppList, 32);
        if (nAppCnt == 0)
        {
            return;
        }

        // 解析分割FontName+FontPath
        for (INT i = 0; i < nAppCnt; i ++)
        {
            if (nIdx > 31)
            {
                return;
            }

            memset(szKeyList, 0x00, sizeof(szKeyList));
            nKeyCnt = DataConvertor::split_string(szAppList[i], ',', szKeyList, 64);
            if (nKeyCnt < 2 || strlen(szKeyList[0]) < 1 || strlen(szKeyList[1]) < 1)
            {
                continue;
            }

            memcpy(stPrtFont[nIdx].szFontName, szKeyList[0], strlen(szKeyList[0]));

            if (szKeyList[1][0] != '/') // 相对路径时重新组合
            {
                sprintf(stPrtFont[nIdx].szFontPath, "%s%s%s", LINUXPATHLIB, DLL_DEVLIB_PATH, szKeyList[1]);
            } else
            {
                memcpy(stPrtFont[nIdx].szFontPath, szKeyList[1], strlen(szKeyList[1]));
            }

            bIdHave[nIdx] = TRUE;
            nIdx ++;
        }
    }
}STPRTFONTLIST, *LPSTPRTFONTLIST;


//-------------------------------------------------------------------------------
class CDevCSR_RSCD400M : public IDevPTR, public CLogManage
{

public:
    CDevCSR_RSCD400M();
    ~CDevCSR_RSCD400M();

public:    
    virtual void Release()                                                              // 释放接口
    {
        return;
    }
    virtual int Open(const char *pMode);                                                // 打开与设备的连接
    virtual int Close();                                                                // 关闭与设备的连接
    virtual int Init()                                                                  // 设备初始化
    {
        return PTR_SUCCESS;
    }
    virtual int Reset();                                                                // 设备复位
    virtual int ResetEx(MEDIA_ACTION enMediaAct, unsigned short usParam);               // 设备复位
    virtual int GetStatus(DEVPTRSTATUS &stStatus);                                      // 取设备状态
    virtual int PrintData(const char *pStr, unsigned long ulDataLen);                   // 打印字串(无指定打印坐标)
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
    virtual int ReadImage(DEVPTRREADIMAGEIN stImageIn, DEVPTRREADIMAGEOUT &stImageOut); // ReadImage获取
    virtual int MeidaControl(MEDIA_ACTION enMediaAct, unsigned short usParam = 0);      // 介质控制
    virtual int SetData(void *vData, WORD wDataType = 0);                               // 设置数据
    virtual int GetData(void *vData, WORD wDataType = 0);                               // 获取数据
    virtual void GetVersion(char* szVer, long lSize, ushort usType);                    // 获取版本

private:
    INT     ConvertErrorCode(INT nRet);                                                 // IMPL错误码转换为PCS错误码
    CHAR*   ConvertErrCodeToStr(INT nRet);                                              // PCS错误码含义
    void    DiffDevStat(SCANNERSTATUS stStat, SCANNERSTATUS stOLD);

private:
    CSimpleMutex                m_cMutex;
    CDevImpl_RSCD400M           m_devRSCD400M;
    SCANNERSTATUS               m_stScanStatOLD;                                        // 记录上一次取到的状态
    CHAR                        m_szErrStr[1024];
    STPRTFONTLIST               m_stFontList;                                           // 字体支持列表

    std::string                 m_stdOpenMode;

    CJsonObject                 m_JsonData;
    std::string                 m_stdJsonText;
    std::string                 m_stdJsonPic;    

    USHORT                      m_usDPIx;                                               // X方向DPI
    USHORT                      m_usDPIy;                                               // y方向DPI

    INT                         m_nGetStatErrOLD;                                       // 取状态接口上一次错误码
    INT                         m_nGetOpenErrOLD;
};

#endif // DEVCSR_RSCD400M_H
