#pragma once
#include "GenerateCustomSNInfo.h"
#include "ISaveBRMCustomSNInfo.h"
#include "ILogWrite.h"
#include "AutoQtHelpClass.h"
#include "XFSCDM.H"
#include "XFSCIM.H"
#include "IBRMAdapter.h"
#include "ISMConfig.h"          //30-00-00-00(FS#0019)

enum FSNGENMODE
{
    FSNGENMODE_UNUSE    = 0,
    FSNGENMODE_NORMAL   = 1,
};

enum NOTELEVEL
{
    NOTELEVEL_LEVEL2 = 2,
    NOTELEVEL_LEVEL3 = 3,
    NOTELEVEL_LEVEL4 = 4,
};


typedef struct _note_sn_info_
{
    NOTELEVEL unotelevel;
    USHORT      usIndex;
    USHORT      usNoteVersion;
    USHORT      usValue;
    string          szImageFilePath;
    string          szOperationTime;
    string          szSerialNumber;
    string          szCurrency;

} NOTESNINFO, *LPNOTESNINFO;


typedef struct _periodtime
{
    SYSTEMTIME sBegin;
    SYSTEMTIME sEnd;
} SPERIODTIME;

typedef struct _snlist_info_details
{
    USHORT         usIndex;
    SYSTEMTIME stime;
    USHORT          uValue;
    BOOL               bvalid;
    char                szSerialNo[32];
    BYTE                szCurrency[3];
    char                szDesPosition[32];
    _snlist_info_details()
    {
        memset(szSerialNo, 0, 32 * sizeof(char));
        memset(szCurrency, 0, 3 * sizeof(BYTE));
        memset(szDesPosition, 0, 32 * sizeof(char));
    }
} STSNLISTINFODETAILS, *LPSTSNLISTINFODETAILS;

//配置是否生成各动作对应的FSN文件
struct stConfigNeedFSN
{
    BOOL bNeedFSNWhenCashIn;
    BOOL bNeedFSNWhenCashInEnd;
    BOOL bNeedFSNWhenDispense;
    BOOL bNeedFSNWhenRetractSlot;
    BOOL bNeedFSNWhenRetractStacker;
};

struct stSNSaveConfig
{
    BYTE GenImgWhenCashIn;
    BYTE GenImgWhenCashInEnd;
    BYTE GenImgWhenRetract;
    BYTE GenImgWhenDispense;
    BYTE GenImgWhenReject;
};

class CSaveCusSNInfo :  public ISaveCusSNInfo, public CLogManage
{
public:
    CSaveCusSNInfo();
    virtual ~CSaveCusSNInfo();
    void  GetSaveImgDir(const SYSTEMTIME &stSysTime,
                        EnumSaveSerialOperation OperCmd, char lpSaveImgPath[MAX_PATH]);

    //传入所有图片的相关信息
    void OnSNInfoGeneration(const STSNINFO &stSNInfo);
    bool IsNeedSaveSNImg(EnumSaveSerialOperation OperCmd);
    bool IsNeedSaveFullImg(EnumSaveSerialOperation OperCmd);
    void GetSaveImgConfig(STIMGCONFIG &stImgConfig);
    virtual void SetLfsCmdId(ADP_LFS_CMD_ID eLfsCmdId) override {                 //test#8
        m_eLfsCmdId = eLfsCmdId;                                //test#8
    }                                                           //test#8

    bool SaveToDatabase();
    void vSetFWVerInfo(char cFwVersionInof[512]);               //30-00-00-00(FS#00001)
private:
    //生成SN文件
    void GenerateSNInfo(const LPSTSNINFODETAILS lpSnInfoDetails);
    void GenerateSNFile();
    void GenerateFSNFileInfo(const char* szSNFilePath, int iFSNFileNum = 1);    //30-00-00-00(FS#0019)

    //生成FSN文件
    void GenerateFSNInfo(const LPSTSNINFODETAILS lpSNInfoDetails, LPSTFSNBody lpFSNBody);
    void GenerateFSNInfo(const LPSTSNINFODETAILS lpSNInfoDetails, LPSTFSNBody18 lpFSNBody18);
    void GenerateFSNFile(LPFSNFileData pdata);
    void GenerateFSNFileHeader(LPFSNFileData pdata);
    void GenerateFSNFileHeader18(LPFSNFileData pdata);          //30-00-00-00(FS#00001)

    BOOL IsNeedFSNFileByOper(const LPSTSNINFODETAILS lp);

    void InitConfig();

    void ClearOverdueFiles(DWORD dwSaveDays);
    //生成JRN文件
    void GenerateJRNInfo(const LPSTSNINFODETAILS lpSnInfoDetails, string &strJRNInfo);
    void GenerateJRNFile(const string strJRNInfo);

    BOOL CreateFile(LPCSTR lpFullName);
    //删除过期文件及目录
    void CheckAndDeleteImage(QString qMainDir);
    bool DeleteDir(const QString &path);

    //生成单日累加信息文件
    void GenerateDailyCumulativeFile(const STSNINFO &stSNInfo);
    void CheckAndDeleteDailyCumulativeFile();
    void CheckAndDeleteDailyCumulativeFileFSN();                        //30-00-00-00(FS#0019)
    void GenerateDailyCumulativeFileFSN(LPCSTR lpszFsnFilePath);        //30-00-00-00(FS#0019)
    //设置FSN文件名
    void vSetFSNFileNameList(const char *cFSNNameList, UCHAR byType=0);          //30-00-00-00(FS#00001)

    std::string     m_sFilePath;    //装入数据的文件路径 //30-00-00-00(FS#0019)
    int GetDeviceNoInfo(string sFileName,LPSTR sDeviceInfo);  //获取ＡＰ设定信息      //30-00-00-00(FS#0019)
    //查找节点
    //ppNode：返回找到节点，如未找到，返回NULL
    //..., 键名，可为多级，最后必须以NULL结束
    BOOL FindNode(ISMConfigNode **ppNode, ...);     //30-00-00-00(FS#0019)
    void GetValueAndVersionByDenomId(BYTE byDenomId, USHORT &usValue, USHORT &usNoteVersion);    //30-00-00-00(FS#0019)
    void SetLevelNErrorCode(LPCSTR lpszErrorCode, Uint16 uErrorCode[3]);     //30-00-00-00(FS#0019)

    //清空冠字号文件
    bool ClearSNFile();
    std::string GetFSNFileFullPath();
private:
    vector<NOTESNINFO> m_vPABNoteSNInfoLV2;
    vector<NOTESNINFO> m_vPABNoteSNInfoLV3;
    vector<NOTESNINFO> m_vPABNoteSNInfoLV4;
    SYSTEMTIME m_sOperationTime;
    string m_strLastDelSeriPictDate;
    string m_strImageSaveDir;
    string m_strConfigFilePath;
    string m_strFSNSaveDri;
    string m_strSNInfoSaveDir;
    string m_strSPPath;
    string m_strDefImgPath;
    int m_strSNFileDir2ndSup;       //test#6
    string m_strfinanIns;           //30-00-00-00(FS#0001)
    string m_strVirtualApFilePath;  //30-00-00-00(FS#0001)
    string m_strMachineNo;          //机具编号 //30-00-00-00(FS#0001)
    string m_strMachineType;        //30-00-00-00(FS#0001)
    string m_strMachineModel;       //30-00-00-00(FS#0001)
    string m_strMakerName;          //30-00-00-00(FS#0001)
    string m_strFinanInst;          //30-00-00-00(FS#0019)
    string m_strFinanInstOutlet;    //30-00-00-00(FS#0019)
    string m_strOperator;           //30-00-00-00(FS#0019)
    string m_strEnableTime;         //30-00-00-00(FS#0019)
    string m_SpecialChar;           //30-00-00-00(FS#0001)
    string m_CurrencyVal;           //30-00-00-00(FS#0001)
    bool   m_bFSN18;                //30-00-00-00(FS#0001)
    string m_strFsnFileSuffix;      //30-00-00-00(FS#0001)
    bool   m_bFSNSubDateDirSupp;    //30-00-00-00(FS#0001)
    int    m_iLevelNIdxFormat;      //30-00-00-00(FT#0017)
    string m_strFSNInfoSaveDir;     //30-00-00-00(FS#0019)
    string m_sDateKeepDay;          //30-00-00-00(FS#0019)
    int    m_FSNFileNameIdx;          //30-00-00-00(FS#0019)

    Uint16 m_uLevel2ErrorCode[3];   //30-00-00-00(FS#0019)
    Uint16 m_uLevel3ErrorCode[3];   //30-00-00-00(FS#0019)
    Uint16 m_uLevel4ErrorCode[3];   //30-00-00-00(FS#0019)

private:
    FSNGENMODE          m_uFGMode;
    IMAGETYPE           m_ImgType;
    unsigned int        m_uFSNMode;
    stSNSaveConfig      m_stSNConfig;
    DWORD               m_dwSaveSNDays;
    bool                m_bGenFSNWhenOperFail;
    bool                m_bGenJRN;
    bool                m_bGenSNFile;
    string              m_FSNFileNamelist[128];                 //30-00-00-00(FS#00001)

    EnumSaveSerialOperation m_oper;
    stConfigNeedFSN m_sNeedFSN;
    ADP_LFS_CMD_ID  m_eLfsCmdId;                            //test#8

    //累加交易冠字号信息文本文件配置项
    DWORD               m_dwDailyCumulativeFileSaveDays;    //test#8
    bool                m_bIsNeedDailyCumulativeFile;       //test#8
    string              m_strDailyCumulativeFileDir;        //test#8
    char    cFwVersionInfo[512];                        //30-00-00-00(FS#00001)

    DWORD               m_dwDailyCumulativeFileSaveDaysFSN;         //30-00-00-00(FS#0019)
    bool                m_bIsNeedDailyCumulativeFileFSN;            //30-00-00-00(FS#0019)
    string              m_strDailyCumulativeFileDirFSN;             //30-00-00-00(FS#0019)

private:
    //vector<STSNLISTINFODETAILS> m_vSNListInfo;
    vector<STSNINFODETAILS> m_vSNInfoDetails;
    string m_strSNDBFilePath;
    int m_iSNDBSaveDays;
    ISMConfig *m_pConfig;       //配置文件          //30-00-00-00(FS#0019)
    ISMConfigNode *m_pRootNode; //根节点           //30-00-00-00(FS#0019)
    string m_strDeviceNoFilePath;  //保存设备番号文件路径       //30-00-00-00(FS#0019)
};

