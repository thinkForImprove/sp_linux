#ifndef DevCRD_ACTU6SS39_H
#define DevCRD_ACTU6SS39_H

#include <time.h>

#include "IDevCRD.h"
#include "DevCRD.h"
#include "DevImpl_ACTU6SS39.h"
#include "QtTypeInclude.h"
#include "../../XFS_UKEY/def.h"

#define LOG_NAME_DEVCRD     "DevCRD_ACTU6SS39.log"


//-------------------------------------------------------------------------------
class CDevCRD_ACTU6SS39 : public IDevCRD, public CLogManage
{

public:
    CDevCRD_ACTU6SS39();
    ~CDevCRD_ACTU6SS39();

public:
    // 释放端口
    virtual void Release();
    // 打开与设备的连接
    virtual int Open(const char *pMode);
    // 设备初始化
    virtual int Init(int emActFlag);
    // 关闭与设备的连接
    virtual int Close();
    // 设备复位
    virtual int Reset(int nMode = 0, int nParam = 0);
    // 读取设备状态
    virtual int GetDevStat(STCRDDEVSTATUS &stStat);
    // 读取设备信息
    virtual int GetUnitInfo(STCRDUNITINFO &stInfo);
    // 发卡
    virtual int DispenseCard(const int nUnitNo, const int nMode);
    // 弹卡
    virtual int EjectCard(const int nMode);
    // 回收卡
    virtual int RetainCard(const int nMode);

public:
    // 设置数据
    virtual int SetData(void *vData, int nSize, WORD wDataType = 0);
    // 获取数据
    virtual int GetData(void *vData, int *nSize, WORD wDataType = 0);
    // 获取版本号(1DevCRD版本/2固件版本/3设备软件版本/4其他)
    virtual void GetVersion(char* szVer, long lSize, ushort usType);

private:
    // 信息扫描
    INT GetScanUKey(LPSTR lpData, INT *nDataLen);

private:
    // Impl错误码转换为DevCRD错误码
    INT     ConvertErrorCode(INT nRet);
    // DevCRD错误码含义
    CHAR*   ConvertErrCodeToStr(INT nRet);

private:
    CSimpleMutex                m_cMutex;
    CQtDLLLoader<IDevCRD>       m_pDev;
    //DEVCRDSTATUS                m_stStatus;

    CDevImpl_ACTU6SS39             m_devACTU6SS39;
    //ST_CRD_ACTU6SS39_INIT_PARAM    m_stCRDACTU6SS39InitParam;
    CHAR                        m_szErrStr[1024];
    INT                         m_nParam[12];   // 机器参数
};

#endif // DevCRD_ACTU6SS39_H
