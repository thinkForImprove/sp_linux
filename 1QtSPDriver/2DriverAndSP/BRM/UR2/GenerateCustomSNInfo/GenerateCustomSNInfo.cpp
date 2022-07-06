#include "GenerateCustomSNInfo.h"
#include "INIFileReader.h"
#include <sstream>
#include <string>
#include <QDir>
#include <iomanip>
#include <chrono>
#include "sndatabase.h"
#ifdef QT_WIN32
const char *STRING_PATH_FILESNINFOR    = "C:\\SNRInfo.ini";
const char *STR_SAVEIMAGE_MAINDIR =  "D:\\OCRSPIMG";   //自定义冠字码图片保存路径
const char *STR_SAVEJRNFILE_DIR = "C:\\xBankTrans\\Trace\\OCR\\TEMP";//JRN文件保存路径
const char *STR_SAVEFSNDEF_DIR = "D:\\LOG\\Image\\FSN";
#else
const char *STRING_PATH_FILESNINFOR = "/usr/local/LOG/SNRInfo.ini";
const char *STR_SAVEIMAGE_MAINDIR = "/usr/local/LOG/Img";
const char *STR_SAVEJRNFILE_DIR = "/usr/local/LOG";
const char *STR_SAVEFSNDEF_DIR = "/usr/local/LOG/FSN";
const char *STR_DAILY_CUMULATIVE_FILE_DIR = "/usr/local/LOG/";      //test#8
const char *STR_DAILY_CUMULATIVE_AP_CONFIG_FILE = "/data/SPSerialNo/Config/SPSerialNoCfg.ini";
const char *STR_DAILY_CUMULATIVE_FILE_DIR_FSN = "/data/SPFsnInfo/Data/";                        //30-00-00-00(FS#0019)
const char *STR_DAILY_CUMULATIVE_AP_CONFIG_FILE_FSN = "/data/SPFsnInfo/Config/SPFsnCfg.ini";    //30-00-00-00(FS#0019)
const char *STRING_PATH_FILESNINFOR_FSN = "/home/WSAP/DATA/FSNInfo.ini";                        //30-00-00-00(FS#0019)

#endif

static time_t s_LastDelTime = 0;
const char *ThisFile = "GenCusSNInfo";

CINIReader cINITemp;        //30-00-00-00(FS#0019)

extern "C" Q_DECL_EXPORT int CreateSaveSNInfoInstance(ISaveCusSNInfo *&pInst)
{
    pInst = new CSaveCusSNInfo();
    return WFS_SUCCESS;
}

inline bool IsDirExist(QString fullPath, bool bMKWhenNotExit = true)
{
    QDir dir(fullPath);
    if (dir.exists())
    {
        return true;
    }
    else if (bMKWhenNotExit)
    {
        return dir.mkpath(fullPath);
    }
    else
    {
        return false;
    }
}

CSaveCusSNInfo::CSaveCusSNInfo()
{
    SetLogFile(LOGFILE, ThisFile, "SN");
    m_eLfsCmdId = ADP_OTHER;
    m_pConfig = nullptr;                                        //30-00-00-00(FS#0019)
    m_FSNFileNameIdx = 0;                                       //30-00-00-00(FS#0019)
    memset(m_uLevel2ErrorCode, 0, sizeof(m_uLevel2ErrorCode));  //30-00-00-00(FS#0019)
    memset(m_uLevel3ErrorCode, 0, sizeof(m_uLevel3ErrorCode));  //30-00-00-00(FS#0019)
    memset(m_uLevel4ErrorCode, 0, sizeof(m_uLevel4ErrorCode));  //30-00-00-00(FS#0019)
    InitConfig();
}

CSaveCusSNInfo::~CSaveCusSNInfo()
{
    m_vPABNoteSNInfoLV2.clear();
    m_vPABNoteSNInfoLV3.clear();
    m_vPABNoteSNInfoLV4.clear();

    if(m_pConfig){
        m_pConfig->Release();
        m_pConfig = nullptr;
    }
}

void  CSaveCusSNInfo::GetSaveImgDir(const SYSTEMTIME &stSysTime,
                                    EnumSaveSerialOperation OperCmd, char lpSaveImgPath[MAX_PATH])
{
    const char *ThisMode = "GetSaveImgPath";
    if (lpSaveImgPath == nullptr)
    {
        Log(ThisMode, __LINE__, "lpSaveImgPath == nullptrptr");
        return;
    }

    if (s_LastDelTime != 0)
    {
        time_t tCurr = time(nullptr);
        if ((tCurr - s_LastDelTime) > (24 * 60 * 60))
        {
            CheckAndDeleteImage(m_strImageSaveDir.c_str());
            CheckAndDeleteDailyCumulativeFile();                    //test#8
            CheckAndDeleteDailyCumulativeFileFSN();                 //30-00-00-00(FS#0019)
            //更新最后一次删除时间
            s_LastDelTime = tCurr;                                  //test#8
        }
    } else {
        //启动后第一次动作时删除
        CheckAndDeleteImage(m_strImageSaveDir.c_str());         //test#8
        CheckAndDeleteDailyCumulativeFile();                    //test#8
        CheckAndDeleteDailyCumulativeFileFSN();                 //30-00-00-00(FS#0019)
        s_LastDelTime = time(NULL);                             //test#8
    }

    m_oper = OperCmd;
    memset(lpSaveImgPath, 0, MAX_PATH * sizeof(char));
    m_sOperationTime = stSysTime;

    char szFirstSubDir[MAX_PATH] = {0};
    //得到1级子目录名
    sprintf(szFirstSubDir, "%04d%02d%02d",
            stSysTime.wYear,
            stSysTime.wMonth,
            stSysTime.wDay);

    if(m_strSNFileDir2ndSup == 1){                      //test#6
        //得到2级子目录名
        char szSecondSubDir[MAX_PATH] = {0};
        sprintf(szSecondSubDir, "%02d%02d%02d",
                stSysTime.wHour,
                stSysTime.wMinute,
                stSysTime.wSecond);

        sprintf(lpSaveImgPath, "%s/%s/%s", m_strImageSaveDir.c_str(), szFirstSubDir, szSecondSubDir);
    }else{                                                                                  //test#6
         sprintf(lpSaveImgPath, "%s/%s", m_strImageSaveDir.c_str(), szFirstSubDir);         //test#6
     }                                                                                      //test#6

    return;
}

void CSaveCusSNInfo::OnSNInfoGeneration(const STSNINFO &stSNInfo)
{
    const char *ThisModule = "OnSNInfoGeneration";
    if (stSNInfo.dwCount <= 0)
    {
        Log(ThisModule, __LINE__, "stSNInfo.dwCount = 0");
//        ClearSNFile();
        return;
    }

    string strJRNInfo;

    m_vSNInfoDetails.clear();
    for (DWORD i = 0; i < stSNInfo.dwCount; i++)
    {
        LPSTSNINFODETAILS lpSNInfoDetails = (LPSTSNINFODETAILS)stSNInfo.lppSNInfoDetail[i];
        GenerateSNInfo(lpSNInfoDetails);
        GenerateJRNInfo(lpSNInfoDetails, strJRNInfo);
        m_vSNInfoDetails.push_back(*lpSNInfoDetails);
    }
    GenerateSNFile();
    GenerateJRNFile(strJRNInfo);

    if (m_uFGMode == FSNGENMODE_NORMAL)
    {
        //根据动作类型判断是否需要生成文件
        LPSTSNINFODETAILS lp = (LPSTSNINFODETAILS)stSNInfo.lppSNInfoDetail[0];
        if (!IsNeedFSNFileByOper(lp))
        {
            Log(ThisModule, __LINE__, "IsNeedFSNFileByOper is false");
            return;
        }
        if (stSNInfo.iOperResult < 0 && !m_bGenFSNWhenOperFail)
        {
            return;
        }
        //30-00-00-00(FS#0001) add start
        LPFSNFileData pData = new FSNFileData;
        if(pData == nullptr){
            Log(ThisModule, __LINE__, "new FSNFileData fail.");
            return;
        }
        memset(pData, 0, sizeof(FSNFileData));
        if(m_bFSN18){
            pData->fsnHead.stFSNHead18.Counter = stSNInfo.dwCount;
            pData->fsnBody.pstFSNBody18 = new LPSTFSNBody18[stSNInfo.dwCount];
            memset(pData->fsnBody.pstFSNBody18, 0, sizeof(LPSTFSNBody18) * stSNInfo.dwCount);
            for (DWORD i = 0; i < stSNInfo.dwCount; i++)
            {
                pData->fsnBody.pstFSNBody18[i] = new STFSNBody18;
                memset(pData->fsnBody.pstFSNBody18[i], 0, sizeof(STFSNBody18));
                LPSTFSNBody18 pFSNBody = pData->fsnBody.pstFSNBody18[i];
                LPSTSNINFODETAILS lpSNInfoDetails = (LPSTSNINFODETAILS)stSNInfo.lppSNInfoDetail[i];
                GenerateFSNInfo(lpSNInfoDetails, pFSNBody);
            }

            GenerateFSNFileHeader18(pData);
        } else {
            pData->fsnHead.stFSNHead.Counter = stSNInfo.dwCount;
            pData->fsnBody.pstFSNBody = new LPSTFSNBody[stSNInfo.dwCount];
            memset(pData->fsnBody.pstFSNBody, 0, sizeof(LPSTFSNBody) * stSNInfo.dwCount);
            for (DWORD i = 0; i < stSNInfo.dwCount; i++)
            {
                pData->fsnBody.pstFSNBody[i] = new STFSNBody;
                memset(pData->fsnBody.pstFSNBody[i], 0, sizeof(STFSNBody));
                LPSTFSNBody pFSNBody = pData->fsnBody.pstFSNBody[i];
                LPSTSNINFODETAILS lpSNInfoDetails = (LPSTSNINFODETAILS)stSNInfo.lppSNInfoDetail[i];
                GenerateFSNInfo(lpSNInfoDetails, pFSNBody);
            }

            GenerateFSNFileHeader(pData);
        }

        GenerateFSNFile(pData);

        if (pData != nullptr)
        {
            int iCount = m_bFSN18 ? pData->fsnHead.stFSNHead18.Counter : pData->fsnHead.stFSNHead.Counter;
            for (Uint32 i = 0; i < iCount; i++)
            {
                if(m_bFSN18){
                    if (pData->fsnBody.pstFSNBody18[i] != nullptr)
                    {
                        delete pData->fsnBody.pstFSNBody18[i];
                        pData->fsnBody.pstFSNBody[i] = nullptr;
                    }
                } else {
                    if (pData->fsnBody.pstFSNBody[i] != nullptr)
                    {
                        delete pData->fsnBody.pstFSNBody[i];
                        pData->fsnBody.pstFSNBody[i] = nullptr;
                    }
                }
            }

            if(m_bFSN18){
                delete []pData->fsnBody.pstFSNBody18;
                pData->fsnBody.pstFSNBody18 = nullptr;
            } else {
                delete []pData->fsnBody.pstFSNBody;
                pData->fsnBody.pstFSNBody = nullptr;
            }

            delete pData;
            pData = nullptr;
        }       
        //30-00-00-00(FS#0001) add end
    }

    if(m_bIsNeedDailyCumulativeFile){                   //test#8
        GenerateDailyCumulativeFile(stSNInfo);          //test#8
    }                                                   //test#8
}

void CSaveCusSNInfo::GenerateSNInfo(const LPSTSNINFODETAILS lpSnInfoDetails)
{
    if (!m_bGenSNFile)
    {
        return;
    }

    NOTESNINFO PABNoteInfo;
    PABNoteInfo.szImageFilePath = lpSnInfoDetails->szSNFullPath;
    PABNoteInfo.usIndex = lpSnInfoDetails->uIndex;

    PABNoteInfo.szSerialNumber = lpSnInfoDetails->szSerialNO;
	PABNoteInfo.szCurrency = lpSnInfoDetails->szCurrencyID;

    GetValueAndVersionByDenomId(lpSnInfoDetails->DnoCode, PABNoteInfo.usValue, PABNoteInfo.usNoteVersion);

    if (lpSnInfoDetails->OperCmd == SAVESERIALOPERAT_D )
    {
        if (lpSnInfoDetails->DestCass == CASS_ROOM_ID_CS)
        {
            PABNoteInfo.unotelevel = NOTELEVEL_LEVEL4;
        }
        else
        {
            PABNoteInfo.unotelevel = NOTELEVEL_LEVEL2;
        }
    }
    else if (lpSnInfoDetails->OperCmd == SAVESERIALOPERAT_S)
    {
        PABNoteInfo.unotelevel = NOTELEVEL_LEVEL4;
    }
    else
    {
        switch (lpSnInfoDetails->NoteCateGory)
        {
        case NOTECATEGORY_4:
        {
            PABNoteInfo.unotelevel = NOTELEVEL_LEVEL4;
            break;
        }
        default:
        {
            PABNoteInfo.unotelevel = NOTELEVEL_LEVEL2;
            break;
        }
        }
    }

    //30-00-00-00(FT#0016) add start
    if(PABNoteInfo.unotelevel == NOTELEVEL_LEVEL2){
        if(PABNoteInfo.usValue == 0){
            PABNoteInfo.szCurrency = "UNK";
        }
        m_vPABNoteSNInfoLV2.push_back(PABNoteInfo);
    } else if(PABNoteInfo.unotelevel == NOTELEVEL_LEVEL3){
        m_vPABNoteSNInfoLV3.push_back(PABNoteInfo);
    } else {
        m_vPABNoteSNInfoLV4.push_back(PABNoteInfo);
    }
    //30-00-00-00(FT#0016) add end

    return;
}

BOOL CSaveCusSNInfo::CreateFile(LPCSTR lpFullName)
{
    QFile qFile;
    if (qFile.exists(lpFullName))
    {
        return TRUE;
    }

    qFile.setFileName(lpFullName);
    if (!qFile.open(QIODevice::ReadWrite | QIODevice::Text))
    {
        Log("CreateFile", __LINE__, "CreateFile(%s) Failed", lpFullName);
        return FALSE;
    }
    qFile.close();
    return TRUE;
}

void CSaveCusSNInfo::GenerateSNFile()
{
    if (m_vPABNoteSNInfoLV2.empty()
        && m_vPABNoteSNInfoLV3.empty()
        && m_vPABNoteSNInfoLV4.empty())
    {
        return;
    }

    if (!QFile::remove(m_strSNInfoSaveDir.c_str()))
    {
        Log("GenerateSNFile", __LINE__, "DeleteDir(%s) Failed", m_strSNInfoSaveDir.c_str());
    }

    //检查目录是否存在，不存在则创建
    string strSNRInfoFileDir = m_strSNInfoSaveDir.substr(0, m_strSNInfoSaveDir.rfind("/"));
    if(!IsDirExist(strSNRInfoFileDir.c_str())){
        Log("GenerateSNFile", __LINE__, "IsDirExist(%s) Failed", strSNRInfoFileDir.c_str());
        return;
    }

    if (!CreateFile(m_strSNInfoSaveDir.c_str()))
        return;

    CINIFileReader ConfigFile;
    ConfigFile.LoadINIFile(m_strSNInfoSaveDir.c_str());

    char szSection[128] = "Cash_Info";

    ConfigFile.SetNewSection(szSection);
    CINIWriter cINI = ConfigFile.GetWriterSection(szSection);
    cINI.AddValue("LEVEL4_COUNT", m_vPABNoteSNInfoLV4.size());
    cINI.AddValue("LEVEL3_COUNT", m_vPABNoteSNInfoLV3.size());
    cINI.AddValue("LEVEL2_COUNT", m_vPABNoteSNInfoLV2.size());

    char szTimeBuff[32];
    sprintf(szTimeBuff, "%04d-%02d-%02d %02d:%02d:%02d", m_sOperationTime.wYear, m_sOperationTime.wMonth, m_sOperationTime.wDay, m_sOperationTime.wHour,
            m_sOperationTime.wMinute, m_sOperationTime.wSecond);
    cINI.AddValue("OperationTime", szTimeBuff);

    cINI.Save();
    vector<NOTESNINFO>::const_iterator it;
    int i = 1;
    for (it = m_vPABNoteSNInfoLV4.begin(); it != m_vPABNoteSNInfoLV4.end(); it++, i++)
    {
        if(m_iLevelNIdxFormat == 0){                    //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL4_%03d", i);       //30-00-00-00(FT#0017)
        } else {                                        //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL4_%d", i);         //30-00-00-00(FT#0017)
        }                                               //30-00-00-00(FT#0017)
        ConfigFile.SetNewSection(szSection);
        cINI = ConfigFile.GetWriterSection(szSection);
        cINI.AddValue("Index", (*it).usIndex);
        cINI.AddValue("Currency", (*it).szCurrency);
        cINI.AddValue("Value", (*it).usValue);
        cINI.AddValue("NoteVersion", (*it).usNoteVersion);
        cINI.AddValue("SerialNumber", (*it).szSerialNumber);
        cINI.AddValue("ImageFile", (*it).szImageFilePath);
        cINI.Save();
    }

    for (i = 1, it = m_vPABNoteSNInfoLV3.begin(); it != m_vPABNoteSNInfoLV3.end(); it++, i++)
    {
        if(m_iLevelNIdxFormat == 0){                                //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL3_%03d", i);                   //30-00-00-00(FT#0017)
        } else {                                                    //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL3_%d", i);                     //30-00-00-00(FT#0017)
        }                                                           //30-00-00-00(FT#0017)
        ConfigFile.SetNewSection(szSection);
        cINI = ConfigFile.GetWriterSection(szSection);
        cINI.AddValue("Index", (*it).usIndex);
        cINI.AddValue("Currency", (*it).szCurrency);
        cINI.AddValue("Value", (*it).usValue);
        cINI.AddValue("NoteVersion", (*it).usNoteVersion);
        cINI.AddValue("SerialNumber", (*it).szSerialNumber);
        cINI.AddValue("ImageFile", (*it).szImageFilePath);
        cINI.Save();
    }

    for (i = 1, it = m_vPABNoteSNInfoLV2.begin(); it != m_vPABNoteSNInfoLV2.end(); it++, i++)
    {
        if(m_iLevelNIdxFormat == 0){                    //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL2_%03d", i);       //30-00-00-00(FT#0017)
        } else {                                        //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL2_%d", i);         //30-00-00-00(FT#0017)
        }                                               //30-00-00-00(FT#0017)
        ConfigFile.SetNewSection(szSection);
        cINI = ConfigFile.GetWriterSection(szSection);
        cINI.AddValue("Index", (*it).usIndex);
        cINI.AddValue("Currency", (*it).szCurrency);
        cINI.AddValue("Value", (*it).usValue);
        cINI.AddValue("NoteVersion", (*it).usNoteVersion);
        cINI.AddValue("SerialNumber", (*it).szSerialNumber);
        cINI.AddValue("ImageFile", (*it).szImageFilePath);
        cINI.Save();
    }
    m_vPABNoteSNInfoLV2.clear();
    m_vPABNoteSNInfoLV3.clear();
    m_vPABNoteSNInfoLV4.clear();
}

//30-00-00-00(FS#0019) 循环覆写文件
/*
 *@param szSNFilePath:FSN文件全路径，多个时分号隔开
 */
void CSaveCusSNInfo::GenerateFSNFileInfo(const char* szSNFilePath, int iFSNFileNum/* = 1*/)
{

    if (!QFile::remove(m_strFSNInfoSaveDir.c_str()))
    {
        Log("GenerateFSNFileInfo", __LINE__, "DeleteDir(%s) Failed", m_strFSNInfoSaveDir.c_str());
    }

    if (!CreateFile(m_strFSNInfoSaveDir.c_str()))
        return;

    CINIFileReader ConfigFile;
    if(!ConfigFile.LoadINIFile(m_strFSNInfoSaveDir.c_str())){
        Log("GenerateFSNFileInfo", __LINE__, "加载配置文件(%s)失败", m_strFSNInfoSaveDir.c_str());
        return;
    }

    char szSection[128] = "Fsn_Info";
    if(ConfigFile.SetNewSection(szSection)){
        CINIWriter cINI = ConfigFile.GetWriterSection(szSection);

        //OperationTime
        char szTimeBuff[32];
        sprintf(szTimeBuff, "%04d-%02d-%02d %02d:%02d:%02d", m_sOperationTime.wYear, m_sOperationTime.wMonth, m_sOperationTime.wDay,
                m_sOperationTime.wHour, m_sOperationTime.wMinute, m_sOperationTime.wSecond);
        cINI.AddValue("OperationTime", szTimeBuff);

        //FSNFile
        stringstream ssValue;
        ssValue << iFSNFileNum << "|" << szSNFilePath;
        cINI.AddValue("FSNFile", ssValue.str().c_str());
        ssValue.str("");

        //FsnVersion
        if(m_bFSN18){
            cINI.AddValue("FsnVersion", "2018");
        } else {
            cINI.AddValue("FsnVersion", "2014");
        }

        cINI.Save();
    } else {
        Log("GenerateFSNFileInfo", __LINE__, "SetNewSection(%s) 失败", szSection);
    }

    return;
 }

void CSaveCusSNInfo::GenerateFSNInfo(const LPSTSNINFODETAILS lpSNInfoDetails, LPSTFSNBody lpFSNBody)
{
    if (lpSNInfoDetails == nullptr || lpFSNBody == nullptr)
    {
        return;
    }
    //处理时间
    if (lpSNInfoDetails->sSTime.wYear >= 1980)
    {
        lpFSNBody->Date = ((lpSNInfoDetails->sSTime.wYear - 1980) << 9) + (lpSNInfoDetails->sSTime.wMonth << 5) + lpSNInfoDetails->sSTime.wDay;
        lpFSNBody->Time = (lpSNInfoDetails->sSTime.wHour << 11) + (lpSNInfoDetails->sSTime.wMinute << 5) + (lpSNInfoDetails->sSTime.wSecond >> 1);
    }

    //
    for (Uint16 j = 0; j < (sizeof(lpFSNBody->MoneyFlag) / sizeof(Uint16)); j++)
    {
        lpFSNBody->MoneyFlag[j] = lpSNInfoDetails->szCurrencyID[j];
    }

    lpFSNBody->Ver = 2;  //人民币年版标识，0：1990，1：1999，2:2005; 3:2015, 4:2019
    GetValueAndVersionByDenomId(lpSNInfoDetails->DnoCode, lpFSNBody->Valuta, lpFSNBody->Ver);

    memset(lpFSNBody->ErrorCode, 0, 3 * sizeof(Uint16));  //假币特征码(1-12)，真币时为0

    // 0为假币或可疑币， 1 为真币， 2为残币，3为旧币
    switch (lpSNInfoDetails->NoteCateGory)
    {
    case NOTECATEGORY_4:
    {
        lpFSNBody->tfFlag = 1;
        memcpy(lpFSNBody->ErrorCode, m_uLevel4ErrorCode, sizeof(m_uLevel4ErrorCode));
        break;
    }
    default:
    {
        lpFSNBody->tfFlag = 0;
        memcpy(lpFSNBody->ErrorCode, m_uLevel2ErrorCode, sizeof(m_uLevel2ErrorCode));
        break;
    }
    }

    string strtemp = lpSNInfoDetails->szSerialNO;
    if (!strtemp.empty())
    {
        lpFSNBody->CharNUM = strtemp.length();
        Uint16 *p = lpFSNBody->SNo;
        int i = 0;
        char *pSN = lpSNInfoDetails->szSerialNO;
        while (*pSN != 0 && i < 12)
        {
            *p++ = *pSN++;
            i++;
        }
    }

    {
        char szMachineSNo[24];
        sprintf(szMachineSNo, "%d/%s/UR2", lpSNInfoDetails->sSTime.wYear, m_strMakerName.c_str());
        Uint16 *p = lpFSNBody->MachineSNo;
        int i = 0;
        char *pSN = szMachineSNo;
        while (*pSN != 0 && i < 24)
        {
            *p++ = *pSN++;
            i++;
        }
    }
    lpFSNBody->ImageSNo = lpSNInfoDetails->ImageSNo;
}

void CSaveCusSNInfo::GenerateFSNInfo(const LPSTSNINFODETAILS lpSNInfoDetails, LPSTFSNBody18 lpFSNBody18)
{
    if (lpSNInfoDetails == nullptr || lpFSNBody18 == nullptr)
    {
        return;
    }

    LPSTFSNBody18 lpFSNBody = lpFSNBody18;
    //处理时间
    if (lpSNInfoDetails->sSTime.wYear >= 1980)
    {
        lpFSNBody->Date = ((lpSNInfoDetails->sSTime.wYear - 1980) << 9) + (lpSNInfoDetails->sSTime.wMonth << 5) + lpSNInfoDetails->sSTime.wDay;
        lpFSNBody->Time = (lpSNInfoDetails->sSTime.wHour << 11) + (lpSNInfoDetails->sSTime.wMinute << 5) + (lpSNInfoDetails->sSTime.wSecond >> 1);
    }

    //
    for (Uint16 j = 0; j < (sizeof(lpFSNBody->MoneyFlag) / sizeof(Uint16)); j++)
    {
        lpFSNBody->MoneyFlag[j] = lpSNInfoDetails->szCurrencyID[j];
    }

    lpFSNBody->Ver = 2;  //人民币年版标识，0：1990，1：1999，2:2005; 3:2015, 4:2019
    USHORT usValue = 0;
    GetValueAndVersionByDenomId(lpSNInfoDetails->DnoCode, usValue, lpFSNBody->Ver);
    lpFSNBody->Valuta = usValue;

    memset(lpFSNBody->ErrorCode, 0, 3 * sizeof(Uint16));  //假币特征码(1-12)，真币时为0

    // 0为假币或可疑币， 1 为真币， 2为残币，3为旧币
    switch (lpSNInfoDetails->NoteCateGory)
    {
    case NOTECATEGORY_4:
    {
        lpFSNBody->tfFlag = 1;
        memcpy(lpFSNBody->ErrorCode, m_uLevel4ErrorCode, sizeof(m_uLevel4ErrorCode));
        break;
    }
    default:
    {
        lpFSNBody->tfFlag = 0;
        memcpy(lpFSNBody->ErrorCode, m_uLevel2ErrorCode, sizeof(m_uLevel2ErrorCode));
        break;
    }
    }

    string strtemp = lpSNInfoDetails->szSerialNO;
    if (!strtemp.empty())
    {
        lpFSNBody->CharNUM = strtemp.length();
        Uint16 *p = lpFSNBody->SNo;
        int i = 0;
        char *pSN = lpSNInfoDetails->szSerialNO;
        while (*pSN != 0 && i < 12)
        {
            *p++ = *pSN++;
            i++;
        }
    }

    lpFSNBody->ImageSNo = lpSNInfoDetails->ImageSNo;
}

void CSaveCusSNInfo::GenerateFSNFileHeader(LPFSNFileData pdata)
{
    if (pdata == nullptr)
    {
        Log("GenerateFSNFileHeader", __LINE__, "pdata == nullptr");
        return;
    }
    //以下为固定值
    pdata->fsnHead.stFSNHead.HeadStart[0] = 20;
    pdata->fsnHead.stFSNHead.HeadStart[1] = 10;
    pdata->fsnHead.stFSNHead.HeadStart[2] = 7;
    pdata->fsnHead.stFSNHead.HeadStart[3] = 26;

    pdata->fsnHead.stFSNHead.HeadString[0] = 0;
    pdata->fsnHead.stFSNHead.HeadString[1] = 1;
    pdata->fsnHead.stFSNHead.HeadString[2] = 0x2E;
    pdata->fsnHead.stFSNHead.HeadString[3] = 'S';
    pdata->fsnHead.stFSNHead.HeadString[4] = 'N';
    pdata->fsnHead.stFSNHead.HeadString[5] = 'o';

    pdata->fsnHead.stFSNHead.HeadEnd[0] = 0;
    pdata->fsnHead.stFSNHead.HeadEnd[1] = 1;
    pdata->fsnHead.stFSNHead.HeadEnd[2] = 2;
    pdata->fsnHead.stFSNHead.HeadEnd[3] = 3;
}

//30-00-00-00(FS#0001)
void CSaveCusSNInfo::GenerateFSNFileHeader18(LPFSNFileData pdata)
{
    if (pdata == nullptr)
    {
        Log("GenerateFSNFileHeader", __LINE__, "pdata == nullptr");
        return;
    }
    //以下为固定值
    pdata->fsnHead.stFSNHead18.HeadStart[0] = 20;
    pdata->fsnHead.stFSNHead18.HeadStart[1] = 10;
    pdata->fsnHead.stFSNHead18.HeadStart[2] = 7;
    pdata->fsnHead.stFSNHead18.HeadStart[3] = 26;

    pdata->fsnHead.stFSNHead18.HeadString[0] = 0;
    pdata->fsnHead.stFSNHead18.HeadString[1] = 2;           //文件版本标识 1:2014 2:2018
    pdata->fsnHead.stFSNHead18.HeadString[2] = 0x2E;        //数据是否包含图像序列号　0x2E:含 0x2D:不含
    pdata->fsnHead.stFSNHead18.HeadString[3] = 'S';
    pdata->fsnHead.stFSNHead18.HeadString[4] = 'N';
    pdata->fsnHead.stFSNHead18.HeadString[5] = 'o';

    pdata->fsnHead.stFSNHead18.HeadEnd[0] = 0;
    pdata->fsnHead.stFSNHead18.HeadEnd[1] = 1;
    pdata->fsnHead.stFSNHead18.HeadEnd[2] = 2;
    pdata->fsnHead.stFSNHead18.HeadEnd[3] = 3;

    //机具信息记录格式标识
    pdata->fsnHead.stFSNHead18.HeadDataTypeFlag = 1;
    //冠字号码记录数量
//     pdata->fsnHead.stFSNHead18.Counter = dwCount;
    //金融机构缩写
    int iCopySize = qMin(m_strfinanIns.size(), sizeof(pdata->fsnHead.stFSNHead18.FinanIns)/sizeof(Uint16));
    for(int i = 0; i < iCopySize; i++){
        pdata->fsnHead.stFSNHead18.FinanIns[i] = m_strfinanIns[i];
    }

    //设备启动时间 0000 Enabletime
    iCopySize = qMin(m_strEnableTime.size(), sizeof(pdata->fsnHead.stFSNHead18.Enabletime));
    memcpy((char *)&pdata->fsnHead.stFSNHead18.Enabletime, m_strEnableTime.c_str(), iCopySize);

    int i = 0;
    //机具编号  公司缩写/机具编号
    iCopySize = qMin(m_strMakerName.size(), (size_t)4);
    for(i = 0; i < iCopySize; i++){
        pdata->fsnHead.stFSNHead18.MachineSNo[i] =  m_strMakerName[i];
    }
    iCopySize = qMin(m_strMachineNo.size(), sizeof(pdata->fsnHead.stFSNHead18.MachineSNo)/sizeof(Uint16) - 4);
    for(i = 0; i < iCopySize; i++){
        pdata->fsnHead.stFSNHead18.MachineSNo[i+4] = m_strMachineNo[i];
    }
    //机具类型
    iCopySize = qMin(m_strMachineType.size(), sizeof(pdata->fsnHead.stFSNHead18.MachineType)/sizeof(Uint16));
    for(i = 0; i < iCopySize; i++){
        pdata->fsnHead.stFSNHead18.MachineType[i] = m_strMachineType[i];
    }
    //机具型号
    iCopySize = qMin(m_strMachineModel.size(), sizeof(pdata->fsnHead.stFSNHead18.MachineModel)/sizeof(Uint16));
    for(i = 0; i < iCopySize; i++){
        pdata->fsnHead.stFSNHead18.MachineModel[i] = m_strMachineModel[i];
    }
    //硬件版本号
    for(i = 0; i < 8; i++){
        pdata->fsnHead.stFSNHead18.HardVerNo[i] = cFwVersionInfo[87 + i];
    }
    //软件版本号
    for(i = 0; i < 8; i++){
        pdata->fsnHead.stFSNHead18.AuthSoftVerNo[i] = cFwVersionInfo[106 + i];
    }
    //适用卷别
    for(int m = 0; m < 6; m++){
       pdata->fsnHead.stFSNHead18.Applidenom[m] = 1;
    }

    //银行的金融机构编码
    iCopySize = qMin(m_strFinanInst.size(), sizeof(pdata->fsnHead.stFSNHead18.FinanInst)/sizeof(Uint16));
    for(i = 0; i < iCopySize; i++){
        pdata->fsnHead.stFSNHead18.FinanInst[i] = m_strFinanInst[i];
    }

    //网点的金融机构编码
    iCopySize = qMin(m_strFinanInstOutlet.size(), sizeof(pdata->fsnHead.stFSNHead18.FinanInstOutlet)/sizeof(Uint16));
    for(i = 0; i < iCopySize; i++){
        pdata->fsnHead.stFSNHead18.FinanInstOutlet[i] = m_strFinanInstOutlet[i];
    }

    //操作人员
    iCopySize = qMin(m_strOperator.size(), sizeof(pdata->fsnHead.stFSNHead18.Operator)/sizeof(Uint16));
    for(i = 0; i < iCopySize; i++){
        pdata->fsnHead.stFSNHead18.Operator[i] = m_strOperator[i];
    }
}

void CSaveCusSNInfo::GenerateFSNFile(LPFSNFileData pdata)
{
    if (pdata == nullptr)
    {
        Log("GenerateFSNFile", __LINE__, "pdata == nullptr");
        return;
    }

    char szFileName[MAX_PATH] = {0};

    char szFileNameBak[MAX_PATH] = {0};
    SYSTEMTIME st;
    CQtTime::GetLocalTime(st);

    char szSNFilePath[MAX_PATH] = {0};
    char szFSNDirPath[MAX_PATH] = {0};

    char szSNFilePathBak[MAX_PATH] = {0};
    char szFSNDirPathBak[MAX_PATH] = {0};
    char cMachiNo[21] = {0};                    //30-00-00-00(FS#0001)

    if(m_bFSNSubDateDirSupp){
        sprintf(szFSNDirPath, "%s/%04d%02d%02d", m_strFSNSaveDri.empty() ? STR_SAVEFSNDEF_DIR : m_strFSNSaveDri.c_str(), st.wYear, st.wMonth, st.wDay);
        sprintf(szFSNDirPathBak, "%s_BAK/%04d%02d%02d", m_strFSNSaveDri.empty() ? STR_SAVEFSNDEF_DIR : m_strFSNSaveDri.c_str(), st.wYear, st.wMonth, st.wDay);
    } else {
        sprintf(szFSNDirPath, "%s", m_strFSNSaveDri.empty() ? STR_SAVEFSNDEF_DIR : m_strFSNSaveDri.c_str());
        sprintf(szFSNDirPathBak, "%s_BAK", m_strFSNSaveDri.empty() ? STR_SAVEFSNDEF_DIR : m_strFSNSaveDri.c_str());
    }


    if (!IsDirExist(szFSNDirPath))
    {
        Log("GenerateFSNFile", __LINE__, "创建目录%s失败", szFSNDirPath);
        return;
    }

    if (!IsDirExist(szFSNDirPathBak))
    {
        Log("GenerateFSNFile", __LINE__, "创建目录%s失败", szFSNDirPathBak);
        return;
    }

    if (m_uFGMode == FSNGENMODE_NORMAL)
    {
        char cTimeDate[64] = {0};                                                       //30-00-00-00(FS#0019)
        CINIFileReader ReaderConfigFile;
        if(!m_FSNFileNamelist[0].empty()){                                              //30-00-00-00(FS#0001)
            for(int i = 0; i < m_FSNFileNamelist->size(); i++){                         //30-00-00-00(FS#0001)
                if(m_FSNFileNamelist[i].compare("SpecialChar") == 0){                     //30-00-00-00(FS#0001)
                    strcat(szFileName,m_SpecialChar.c_str());                             //30-00-00-00(FS#0001)
                }else if(m_FSNFileNamelist[i].compare("Currency") == 0){                  //30-00-00-00(FS#0001)
                    strcat(szFileName,m_CurrencyVal.c_str());                             //30-00-00-00(FS#0001)
                }else if(m_FSNFileNamelist[i].compare("MachineSNo") == 0){                //30-00-00-00(FS#0001)                   
                    memcpy(cMachiNo, m_strMachineNo.c_str(),
                           qMin(m_strMachineNo.size(), sizeof(cMachiNo) - 1));
                    strcat(szFileName, cMachiNo);                                        //30-00-00-00(FS#0001)
                }else if(m_FSNFileNamelist[i].compare("Date") == 0){                     //30-00-00-00(FS#0001)
                    sprintf(szFileName + strlen(szFileName), "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
                }else if(m_FSNFileNamelist[i].compare("_") == 0){                         //30-00-00-00(FS#0001)
                    strcat(szFileName,"_");                                               //30-00-00-00(FS#0001)
                }else if(m_FSNFileNamelist[i].compare("Time") == 0){                      //30-00-00-00(FS#0001)
                    sprintf(szFileName + strlen(szFileName), "%02d%02d%02d", st.wHour, st.wMinute, st.wSecond);     //30-00-00-00(FS#0001)
                }else if(m_FSNFileNamelist[i].compare("Index") == 0){                     //30-00-00-00(FS#0019)
                    //获取下标值
                    //30-00-00-00(FS#0019) start
                    if(ReaderConfigFile.LoadINIFile(m_strConfigFilePath.c_str())){
                        CINIReader cINIReader = ReaderConfigFile.GetReaderSection("DEFAULT");
                        m_sDateKeepDay = (LPCSTR)cINIReader.GetValue("FSNDateKeep", "");
                        m_FSNFileNameIdx = (int)cINIReader.GetValue("FSNDateKeepCount", 0);
                    }
                    SYSTEMTIME sysTimeTemp;
                    CQtTime::GetLocalTime(sysTimeTemp);
                    sprintf(cTimeDate, "%04d%02d%02d",sysTimeTemp.wYear, sysTimeTemp.wMonth, sysTimeTemp.wDay);
                    if(strcmp(m_sDateKeepDay.c_str(),cTimeDate) == 0){
                        m_FSNFileNameIdx++;
                    }else{
                        m_FSNFileNameIdx = 1;
                    }
                    //30-00-00-00(FS#0019) end
                    sprintf(szFileName + strlen(szFileName), "%04d",m_FSNFileNameIdx);         //30-00-00-00(FS#0019)
                } else {                                                                  //30-00-00-00(FS#0001)
                    //固定文本                                                             //30-00-00-00(FS#0001)
                    strcat(szFileName, m_FSNFileNamelist[i].c_str());                     //30-00-00-00(FS#0001)
                }                                                                         //30-00-00-00(FS#0001)
            }                                                                             //30-00-00-00(FS#0001)
            //追加文件后缀                                                                  //30-00-00-00(FS#0001)
            strcat(szFileName, ".");
            strcat(szFileName, m_strFsnFileSuffix.c_str());                               //30-00-00-00(FS#0001)
            strcpy(szFileNameBak, szFileName);                                            //30-00-00-00(FS#0001)
        }else{                                                                            //30-00-00-00(FS#0001)
            // strcpy(szFileName, "sp.fsn");
            sprintf(szFileName, "sp_%04d%02d%02d_%02d%02d%02d.%s", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, m_strFsnFileSuffix.c_str());
            sprintf(szFileNameBak, "%04d_%02d_%02d_%02d_%02d_%02d.%s", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, m_strFsnFileSuffix.c_str());
        }                                                                                   //30-00-00-00(FS#0001)
        sprintf(szSNFilePath, "%s/%s", szFSNDirPath, szFileName);
        sprintf(szSNFilePathBak, "%s/%s", szFSNDirPathBak, szFileNameBak);
        FILE *pf = fopen(szSNFilePath, "wb");
        if (pf == nullptr)
        {
            Log("GenerateFSNFile", __LINE__, "打开文件错误%s", szSNFilePath);
            return;
        }
        int iLen;
        int iCount = 0;
        if(m_bFSN18){
            iLen = fwrite(&pdata->fsnHead.stFSNHead18, sizeof(pdata->fsnHead.stFSNHead18), 1, pf);
            iCount = pdata->fsnHead.stFSNHead18.Counter;
        } else {
            iLen = fwrite(&pdata->fsnHead.stFSNHead, sizeof(pdata->fsnHead.stFSNHead), 1, pf);
            iCount = pdata->fsnHead.stFSNHead.Counter;
        }
        for (DWORD j = 0; j < iCount; j++)
        {
            if(m_bFSN18){
                iLen = fwrite(pdata->fsnBody.pstFSNBody18[j], sizeof(STFSNBody18), 1, pf);
            } else {
                iLen = fwrite(pdata->fsnBody.pstFSNBody[j], sizeof(STFSNBody), 1, pf);
            }
        }
        fclose(pf);

        //        if(!QFile::copy(szSNFilePath, szSNFilePathBak))
        //        {
        //            Log("GenerateFSNFile", -1, "Copy File Failed , %s to %s", szSNFilePath, szSNFilePathBak);
        //            return ;
        //        }

        GenerateFSNFileInfo(szSNFilePath);                      //30-00-00-00(FS#0019)
        if(m_bIsNeedDailyCumulativeFileFSN){                    //30-00-00-00(FS#0019)
            GenerateDailyCumulativeFileFSN(szSNFilePath);       //30-00-00-00(FS#0019)
        }

        //FSN文件名中下标写入配置文件
        if(ReaderConfigFile.LoadINIFile(m_strConfigFilePath.c_str())){
            CINIWriter cINIWrite = ReaderConfigFile.GetWriterSection("DEFAULT");
            cINIWrite.SetValue("FSNDateKeep", cTimeDate);
            cINIWrite.SetValue("FSNDateKeepCount", m_FSNFileNameIdx);
        }
    }
}

void CSaveCusSNInfo::InitConfig()
{
    m_strConfigFilePath = SPETCPATH;
    m_strConfigFilePath += "/";
    m_strConfigFilePath += "SNConfig.ini";

    QFileInfo info(m_strConfigFilePath.c_str());
    if (!info.exists())
    {
        Log("InitConfig", __LINE__, "SNConfig.ini not exist");
        return;
    }

    string strTemp;
    CINIFileReader ReaderConfigFile;
    CINIFileReader cAPIni;
    ReaderConfigFile.LoadINIFile(m_strConfigFilePath.c_str());
    CINIReader cINI = ReaderConfigFile.GetReaderSection("DEFAULT");
    cINITemp = cINI;                    //30-00-00-00(FS#0019)

    // FSN MODE
    m_uFGMode = (FSNGENMODE)(int)cINI.GetValue("FSNGenerateMode", 1);
    string sWSAPConfigFilePath = (LPCSTR)cINI.GetValue("WSSPConfigFilePath", "");  //30-00-00-00(FS#0019)
    if(!sWSAPConfigFilePath.empty()){
        CINIFileReader cApIni;
        if(cApIni.LoadINIFile(sWSAPConfigFilePath.c_str())){
            CINIWriter cWriter = cApIni.GetWriterSection("DEFAULT");
            cWriter.SetValue("IsSPSupportFSN", 1);
        }
    }

    m_ImgType = (IMAGETYPE)(int)cINI.GetValue("ImageType", 1);
    m_dwSaveSNDays = (DWORD)cINI.GetValue("SaveDays", 90);
    string cFSNFileNamelist = (LPCSTR)cINI.GetValue("FSNFileNameSetList", "");   //30-00-00-00(FS#0001)
    vSetFSNFileNameList(cFSNFileNamelist.c_str());                               //30-00-00-00(FS#0001)

    m_uFSNMode = (DWORD)cINI.GetValue("FSNMode", 0);

    Log("InitConfig", 1, "FSNGenerateMode :%d", m_uFGMode);
    Log("InitConfig", 1, "FSNFileSaveDir :%s", m_strFSNSaveDri.c_str());
    Log("InitConfig", 1, "ImageType :%d", (DWORD)m_ImgType);
    Log("InitConfig", 1, "SaveDays :%d", m_dwSaveSNDays);
    //获取配置是否启用生成各动作对应的FSN文件
    m_sNeedFSN.bNeedFSNWhenCashIn = ((int)cINI.GetValue("NeedFSNWhenCashIn", 0) == 1);
    m_sNeedFSN.bNeedFSNWhenCashInEnd = ((int)cINI.GetValue("NeedFSNWhenCashInEnd", 1) == 1);
    m_sNeedFSN.bNeedFSNWhenRetractSlot = ((int)cINI.GetValue("NeedFSNWhenRetractSlot", 0) == 1);
    m_sNeedFSN.bNeedFSNWhenRetractStacker = ((int)cINI.GetValue("NeedFSNWhenRetractStacker", 1) == 1);
    m_sNeedFSN.bNeedFSNWhenDispense = ((int)cINI.GetValue("NeedFSNWhenDispense", 1) == 1);

    m_stSNConfig.GenImgWhenDispense = (DWORD)cINI.GetValue("GenImgWhenDispense", 1);
    m_stSNConfig.GenImgWhenCashInEnd = (DWORD)cINI.GetValue("GenImgWhenCashInEnd", 1);
    m_stSNConfig.GenImgWhenCashIn = (DWORD)cINI.GetValue("GenImgWhenCashIn", 1);
    m_stSNConfig.GenImgWhenRetract = (DWORD)cINI.GetValue("GenImgWhenRetract", 1);
    m_stSNConfig.GenImgWhenReject = (DWORD)cINI.GetValue("GenImgWhenReject", 1);
    Log("InitConfig", 1, "GenImgWhenDispense:%d", m_stSNConfig.GenImgWhenDispense);
    Log("InitConfig", 1, "GenImgWhenCashInEnd:%d", m_stSNConfig.GenImgWhenCashInEnd);
    Log("InitConfig", 1, "GenImgWhenCashIn:%d", m_stSNConfig.GenImgWhenCashIn);
    Log("InitConfig", 1, "GenImgWhenRetract:%d", m_stSNConfig.GenImgWhenRetract);
    Log("InitConfig", 1, "GenImgWhenReject:%d", m_stSNConfig.GenImgWhenReject);
    m_bGenFSNWhenOperFail = ((int)cINI.GetValue("GenerateFSNWhenOperFail", 0) == 1);
    m_bGenJRN = ((int)cINI.GetValue("GenerateJRN", 0) == 1);
    m_bGenSNFile = ((int)cINI.GetValue("GenerateSNFile", 0) == 1);

    m_strSNInfoSaveDir = (LPCSTR)cINI.GetValue("SNInfoSaveDir", STRING_PATH_FILESNINFOR);
    m_strImageSaveDir = (LPCSTR)cINI.GetValue("ImageSaveDir", STR_SAVEIMAGE_MAINDIR);
    m_strFSNSaveDri = (LPCSTR)cINI.GetValue("FSNFileSaveDir", STR_SAVEFSNDEF_DIR);
    m_strSNFileDir2ndSup = (int)cINI.GetValue("SNFileDir2ndSup", 0);   //test#6
    m_iLevelNIdxFormat = (int)cINI.GetValue("LevelNIdxFormat", 0);              //30-00-00-00(FT#0017)
    if(m_iLevelNIdxFormat < 0 || m_iLevelNIdxFormat > 1){                       //30-00-00-00(FT#0017)
        m_iLevelNIdxFormat = 0;                                                 //30-00-00-00(FT#0017)
    }                                                                           //30-00-00-00(FT#0017)

    m_strDefImgPath = (LPCSTR)cINI.GetValue("DefImgPath", "");
    Log("InitConfig", 1, "strDefImgPath :%s", m_strDefImgPath.c_str());

    //30-00-00-00(FS#0001) add start
    //获取机器编号
    m_strMachineNo = "";
    char szAPFsnConfigFileItem[MAX_PATH] = {0};
    strcpy(szAPFsnConfigFileItem,
           (LPCSTR)cINI.GetValue("APFsnConfigFileItem", ""));
    CAutoSplitByStep autoSplitByStep(szAPFsnConfigFileItem, ",");
    if(autoSplitByStep.Count() >= 3){
        m_strVirtualApFilePath = autoSplitByStep.At(0);
        //判断文件是否存在
        QFileInfo info(m_strVirtualApFilePath.c_str());
        if (info.exists())
        {
            CINIFileReader iniFile;
            iniFile.LoadINIFile(m_strVirtualApFilePath.c_str());
            CINIReader iniReader = iniFile.GetReaderSection(autoSplitByStep.At(1));
            m_strMachineNo = (LPCSTR)iniReader.GetValue(autoSplitByStep.At(2), "");
        } else {
            Log("InitConfig", __LINE__, "%s not exist", m_strVirtualApFilePath.c_str());
        }
    }
    //30-00-00-00(FS#0001) add end

    //30-00-00-00(FS#0019) add start
    //FSN ERRORCODE
    strTemp = (LPCSTR)cINI.GetValue("Level2ErrorCode", "");
    if(!strTemp.empty()){
        SetLevelNErrorCode(strTemp.c_str(), m_uLevel2ErrorCode);
    }
    strTemp = (LPCSTR)cINI.GetValue("Level3ErrorCode", "");
    if(!strTemp.empty()){
        SetLevelNErrorCode(strTemp.c_str(), m_uLevel3ErrorCode);
    }
    strTemp = (LPCSTR)cINI.GetValue("Level4ErrorCode", "");
    if(!strTemp.empty()){
        SetLevelNErrorCode(strTemp.c_str(), m_uLevel4ErrorCode);
    }
    //30-00-00-00(FS#0019) add end

    m_strfinanIns = (LPCSTR)cINI.GetValue("FinanIns", "");                      //30-00-00-00(FS#0001)
    m_strMachineType = (LPCSTR)cINI.GetValue("MachineType", "ZB_PL_ZZJB");      //30-00-00-00(FS#0001)
    m_strMachineModel = (LPCSTR)cINI.GetValue("MachineModel", "TS-EA45-I11");   //30-00-00-00(FS#0001)
    m_strMakerName = (LPCSTR)cINI.GetValue("MakerName", "CFES");                //30-00-00-00(FS#0001)
    m_strFinanInst = (LPCSTR)cINI.GetValue("FinanInst", "");                    //30-00-00-00(FS#0019)
    m_strFinanInstOutlet = (LPCSTR)cINI.GetValue("FinanInstOutlet", "");        //30-00-00-00(FS#0019)
    m_strOperator = (LPCSTR)cINI.GetValue("Operator", "");                      //30-00-00-00(FS#0019)
    m_strEnableTime = (LPCSTR)cINI.GetValue("EnableTime", "");                  //30-00-00-00(FS#0019)
    m_SpecialChar = (LPCSTR)cINI.GetValue("SpecialChar", "FSN10");              //30-00-00-00(FS#0001)
    m_CurrencyVal = (LPCSTR)cINI.GetValue("CurrencyVal", "CNY");                //30-00-00-00(FS#0001)
    m_bFSN18 =  (int)cINI.GetValue("FSN18Sup", 0) == 1;                         //30-00-00-00(FS#0001)
    m_strFsnFileSuffix = (LPCSTR)cINI.GetValue("FSNFileSuffix", "fsn");         //30-00-00-00(FS#0001)
    m_bFSNSubDateDirSupp = (int)cINI.GetValue("FSNSubDateDirSupp", 0) == 1;     //30-00-00-00(FS#0001)

    Log("InitConfig", 1, "DeviceID :%s", m_strMachineNo.c_str());               //30-00-00-00(FS#0001)
    Log("InitConfig", 1, "FinanIns :%s", m_strfinanIns.c_str());                //30-00-00-00(FS#0001)
    Log("InitConfig", 1, "MachineType :%s", m_strMachineType.c_str());          //30-00-00-00(FS#0001)
    Log("InitConfig", 1, "MakerName :%s", m_strMakerName.c_str());              //30-00-00-00(FS#0001)
    Log("InitConfig", 1, "SpecialChar :%s", m_SpecialChar.c_str());             //30-00-00-00(FS#0001)
    Log("InitConfig", 1, "CurrencyVal :%s", m_CurrencyVal.c_str());             //30-00-00-00(FS#0001)
    Log("InitConfig", 1, "MachineModel :%s", m_strMachineModel.c_str());        //30-00-00-00(FS#0001)
    Log("InitConfig", 1, "FSN18Sup :%d", m_bFSN18);                             //30-00-00-00(FS#0001)

    m_strDailyCumulativeFileDir = (LPCSTR)cINI.GetValue("DailyCumulativeFileDir", STR_DAILY_CUMULATIVE_FILE_DIR);
    string m_strDailyCumulativeApCfgPath = (LPCSTR)cINI.GetValue("DailyCumulativeAPCfgPath", STR_DAILY_CUMULATIVE_AP_CONFIG_FILE);
    Log("InitConfig", 1, "m_strDailyCumulativeFilePath :%s", m_strDailyCumulativeFileDir.c_str());
    Log("InitConfig", 1, "m_strDailyCumulativeApCfgPath :%s", m_strDailyCumulativeApCfgPath.c_str());

    if(cAPIni.LoadINIFile(m_strDailyCumulativeApCfgPath)){
        CINIReader cApIniReader = cAPIni.GetReaderSection("CFG");
        m_bIsNeedDailyCumulativeFile = (int)cApIniReader.GetValue("IsNeedDailyRecord", 0);
        m_dwDailyCumulativeFileSaveDays = (int)cApIniReader.GetValue("SavaTime", 120);
        m_dwSaveSNDays = m_dwDailyCumulativeFileSaveDays;

        Log("InitConfig", 1, "m_bIsNeedDailyCumulativeFile :%d", (int)m_bIsNeedDailyCumulativeFile);
        Log("InitConfig", 1, "m_dwDailyCumulativeFileSaveDays :%d", m_dwDailyCumulativeFileSaveDays);
    }

    m_strSNDBFilePath = (LPCSTR)cINI.GetValue("SNDBFilePath", "/data/SPSerialNo/SNDatabase.db");
    m_iSNDBSaveDays = (int)cINI.GetValue("SNDBSaveDays", 365);

    //for 2018 fsn(PAB)//30-00-00-00(FS#0019) start    
    if(strlen(szAPFsnConfigFileItem) == 0){
        //如果ini方式未配置，以xml方式读取
        char sDeviceInfo[MAX_PATH] = {0};
        m_strDeviceNoFilePath = (LPCSTR)cINI.GetValue("DeviceNoFilePath", "/home/projects/ZJUAPClient/EAI/cfg/EAICfg.xml");
        GetDeviceNoInfo(m_strDeviceNoFilePath, sDeviceInfo);  //获取设备编号//30-00-00-00(FS#0019)
        Log("InitConfig", 0, "cDeviceNo :%s", sDeviceInfo);             //30-00-00-00(FS#0019)
        m_strMachineNo = sDeviceInfo;                                   //30-00-00-00(FS#0019)
    }

    m_strDailyCumulativeFileDirFSN = (LPCSTR)cINI.GetValue("FSNDailyCumulativeFileDir", STR_DAILY_CUMULATIVE_FILE_DIR_FSN);
    string m_strDailyCumulativeApCfgPathFSN = (LPCSTR)cINI.GetValue("FSNDailyCumulativeAPCfgPath", STR_DAILY_CUMULATIVE_AP_CONFIG_FILE_FSN);
    Log("InitConfig", 1, "m_strDailyCumulativeFileDirFSN :%s", m_strDailyCumulativeFileDirFSN.c_str());
    Log("InitConfig", 1, "m_strDailyCumulativeApCfgPathFSN :%s", m_strDailyCumulativeApCfgPathFSN.c_str());

    m_bIsNeedDailyCumulativeFileFSN = false;
    m_dwDailyCumulativeFileSaveDaysFSN = m_dwSaveSNDays;
    if(cAPIni.LoadINIFile(m_strDailyCumulativeApCfgPathFSN)){
        CINIReader cApIniReader = cAPIni.GetReaderSection("CFG");
        m_bIsNeedDailyCumulativeFileFSN = (int)cApIniReader.GetValue("IsNeedDailyRecord", 0);
        m_dwDailyCumulativeFileSaveDaysFSN = (int)cApIniReader.GetValue("SavaTime", 120);

        Log("InitConfig", 1, "m_bIsNeedDailyCumulativeFileFSN :%d", (int)m_bIsNeedDailyCumulativeFileFSN);
        Log("InitConfig", 1, "m_dwDailyCumulativeFileSaveDaysFSN :%d", m_dwDailyCumulativeFileSaveDaysFSN);
    }
    m_strFSNInfoSaveDir = (LPCSTR)cINI.GetValue("FSNInfoSaveDir", STRING_PATH_FILESNINFOR_FSN);          //30-00-00-00(FS#0019)
}

void CSaveCusSNInfo::GenerateJRNInfo(const LPSTSNINFODETAILS lpSnInfoDetails, string& strJRNInfo)
{

}

void CSaveCusSNInfo::GenerateJRNFile(const string strJRNInfo)
{
    if (strJRNInfo.empty())
    {
        return;
    }
    string strJRNFileName("");

    char szSubDir[MAX_PATH] = {0};
    sprintf(szSubDir, "%04d%02d%02d", m_sOperationTime.wYear, m_sOperationTime.wMonth, m_sOperationTime.wDay);
    string strJRNDir = strJRNFileName + STR_SAVEJRNFILE_DIR + "/" + szSubDir;
    if (!IsDirExist(strJRNDir.c_str()))
    {
        Log("GenerateJRNFile", 0, "创建目录%s失败", strJRNDir.c_str());
        return;
    }
    strJRNFileName = strJRNDir + "/OCRSP.jrn";
    FILE *pf = fopen(strJRNFileName.c_str(), "a");
    if (pf == nullptr)
    {
        Log("GenerateJRNFile", __LINE__, "打开文件(%s)失败", strJRNFileName.c_str());
        return;
    }
    fwrite(strJRNInfo.c_str(), sizeof(char), strJRNInfo.size(), pf);
    fclose(pf);
}

BOOL CSaveCusSNInfo::IsNeedFSNFileByOper(const LPSTSNINFODETAILS lp)
{
    if (lp == nullptr)
    {
        return FALSE;
    }

    switch (lp->OperCmd)
    {
    case SAVESERIALOPERAT_D:
        return m_sNeedFSN.bNeedFSNWhenDispense;
    case SAVESERIALOPERAT_S:
        return m_sNeedFSN.bNeedFSNWhenCashInEnd;
    case SAVESERIALOPERAT_C:
        return m_sNeedFSN.bNeedFSNWhenCashIn;
    case SAVESERIALOPERAT_R:
    {
        if (lp->DestCass == CASS_ROOM_ID_ESC)
            return m_sNeedFSN.bNeedFSNWhenRetractSlot;
        else if (lp->DestCass == CASS_ROOM_ID_4)
            return m_sNeedFSN.bNeedFSNWhenRetractStacker;
    }
    default:
        return FALSE;
    }
}

bool CSaveCusSNInfo::IsNeedSaveSNImg(EnumSaveSerialOperation OperCmd)
{
    switch (OperCmd)
    {
    case SAVESERIALOPERAT_D:
        return m_stSNConfig.GenImgWhenDispense & 1;
    case SAVESERIALOPERAT_S:
        return m_stSNConfig.GenImgWhenCashInEnd & 1;
    case SAVESERIALOPERAT_C:
        return m_stSNConfig.GenImgWhenCashIn & 1;
    case SAVESERIALOPERAT_R:
        return m_stSNConfig.GenImgWhenRetract & 1;
    case SAVESERIALOPERAT_J:
        return m_stSNConfig.GenImgWhenReject & 1;
    default:
        return FALSE;
    }
}

bool CSaveCusSNInfo::IsNeedSaveFullImg(EnumSaveSerialOperation OperCmd)
{
    switch (OperCmd)
    {
    case SAVESERIALOPERAT_D:
        return m_stSNConfig.GenImgWhenDispense & 2;
    case SAVESERIALOPERAT_S:
        return m_stSNConfig.GenImgWhenCashInEnd & 2;
    case SAVESERIALOPERAT_C:
        return m_stSNConfig.GenImgWhenCashIn & 2;
    case SAVESERIALOPERAT_R:
        return m_stSNConfig.GenImgWhenRetract & 2;
    case SAVESERIALOPERAT_J:
        return m_stSNConfig.GenImgWhenReject & 2;
    default:
        return FALSE;
    }
}

void CSaveCusSNInfo::GetSaveImgConfig(STIMGCONFIG &stImgConfig)
{
    strcpy(stImgConfig.szDefImgPath, m_strDefImgPath.c_str());
    stImgConfig.ImgType = m_ImgType;
    stImgConfig.uFSNMode = m_uFSNMode;
}

void CSaveCusSNInfo::CheckAndDeleteImage(QString qMainDir)
{
    char aryTmp[10] = {0};
    time_t tCurr = time(nullptr);
    time_t tCurr_ = tCurr - (m_dwSaveSNDays - 1) * 24 * 60 * 60;
    tm *pTime = localtime(&tCurr_);
    strftime(aryTmp, MAX_PATH, "%Y%m%d", pTime);
    string strEffeDays = aryTmp;

    QDir dir(qMainDir);
    if (!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);  //设置过滤
    QFileInfoList fileList = dir.entryInfoList();            // 获取所有的文件信息
    foreach (QFileInfo file, fileList)
    {
        if (file.isDir())
        {
            std::string strFileName = file.fileName().toStdString();
            if (strFileName < strEffeDays)
            {
                if (!DeleteDir(file.absoluteFilePath()))
                {
                    Log("CheckAndDeleteImage", __LINE__, "删除目录(%s)失败", file.absoluteFilePath().toStdString().c_str());
                }
            }
        }
    }
    s_LastDelTime = tCurr;
    return;  // 删除文件夹
}

bool CSaveCusSNInfo::DeleteDir(const QString &path)
{
    if (path.isEmpty())
    {
        return false;
    }
    QDir dir(path);
    if (!dir.exists())
    {
        return true;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);  //设置过滤
    QFileInfoList fileList = dir.entryInfoList();            // 获取所有的文件信息
    foreach (QFileInfo file, fileList)
    {
        //遍历文件信息
        if (file.isFile())
        {
            // 是文件，删除
            file.dir().remove(file.fileName());
        }
        else
        {
            // 递归删除
            DeleteDir(file.absoluteFilePath());
        }
    }
    return dir.rmpath(dir.absolutePath());  // 删除文件夹
}

void CSaveCusSNInfo::GenerateDailyCumulativeFile(const STSNINFO &stSNInfo)          //test#8
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    int     iOprCode = -1;
    switch(m_eLfsCmdId){
    case ADP_CASHIN:
        iOprCode = 1302;
        break;
    case ADP_CASHINEND:
        break;
    case ADP_CIM_RETRACT:
        iOprCode = 1305;
        break;
    case ADP_CIM_RESET:
        iOprCode = 1313;
        break;
    case ADP_DISPENSE:
        iOprCode = 302;
        break;
    case ADP_CDM_RETRACT:
        iOprCode = 305;
        break;
    case ADP_CDM_RESET:
        iOprCode = 321;
        break;
    default:
        break;
    }

    //无需记录的命令直接返回
    if(-1 == iOprCode){
        return;
    }

    char szOperationTime[7] = {0};
    sprintf(szOperationTime, "%02d%02d%02d", m_sOperationTime.wHour,
            m_sOperationTime.wMinute, m_sOperationTime.wSecond);

    stringstream ss;
    string strTradeInfo;
    for (DWORD i = 0; i < stSNInfo.dwCount; i++)
    {
        if(i > 0){
            ss << ";";
        }
        USHORT  usNoteLevel;
        string  szSerialNumber;
        string  szImageFilePath;
        USHORT  usValue;
        USHORT  usNoteVersion;
        string  szCurrency;

        LPSTSNINFODETAILS lpSNInfoDetails = (LPSTSNINFODETAILS)stSNInfo.lppSNInfoDetail[i];
        GetValueAndVersionByDenomId(lpSNInfoDetails->DnoCode, usValue, usNoteVersion);

        szSerialNumber = lpSNInfoDetails->szSerialNO;
        szCurrency = lpSNInfoDetails->szCurrencyID;
        szImageFilePath = lpSNInfoDetails->szSNFullPath;

        if (lpSNInfoDetails->OperCmd == SAVESERIALOPERAT_D )
        {
            if (lpSNInfoDetails->DestCass == CASS_ROOM_ID_CS)
            {
                usNoteLevel = NOTELEVEL_LEVEL4;
            }
            else
            {
                usNoteLevel = NOTELEVEL_LEVEL2;
            }
        }
        else if (lpSNInfoDetails->OperCmd == SAVESERIALOPERAT_S)
        {
            usNoteLevel = NOTELEVEL_LEVEL4;
        }
        else
        {
            switch (lpSNInfoDetails->NoteCateGory)
            {
            case NOTECATEGORY_4:
            {
                usNoteLevel = NOTELEVEL_LEVEL4;
                break;
            }
            default:
            {
                usNoteLevel = NOTELEVEL_LEVEL2;
                break;
            }
            }
        }

        ss << usNoteLevel << "," << szSerialNumber << "," << szImageFilePath << ","
                     << usValue << "," << usNoteVersion << "," << szCurrency;
    }

    ss << "|" << szOperationTime << ":" << iOprCode << "\n";
    strTradeInfo = ss.str();
    ss.str("");
 //   Log("GenerateDailyCumulativeFile", 0, "单次交易信息：％s", strTradeInfo.c_str());

    //获取文件全路径
    char szFilePath[MAX_PATH] = {0};
    strcpy(szFilePath, m_strDailyCumulativeFileDir.c_str());
    if(szFilePath[strlen(szFilePath) - 1] != '/'){
        strcat(szFilePath, "/");
    }

    //判断目录是否存在，不存在则创建
    if(!IsDirExist(szFilePath)){
        Log("GenerateDailyCumulativeFile", 0, "创建目录%s失败", szFilePath);
        return;
    }

    sprintf(szFilePath + strlen(szFilePath), "%04d%02d%02d.txt", m_sOperationTime.wYear,
                m_sOperationTime.wMonth, m_sOperationTime.wDay);

    //交易冠字号信息写入文件
    FILE *fp = fopen(szFilePath, "a+");
    if(fp == nullptr){
        Log("GenerateDailyCumulativeFile", 0, "打开文件%s失败", szFilePath);
    } else {
        if(fwrite(strTradeInfo.c_str(), strTradeInfo.size(), 1, fp) != 1){
            Log("GenerateDailyCumulativeFile", 0, "写入文件%s失败", szFilePath);
        }
        fclose(fp);
    }

    return;
}

//30-00-00-00(FS#0019) FSN单天累加文件
void CSaveCusSNInfo::GenerateDailyCumulativeFileFSN(LPCSTR lpszFsnFilePath)
{
    THISMODULE(__FUNCTION__);

    int     iOprCode = -1;
    switch(m_eLfsCmdId){
    case ADP_CASHIN:
        iOprCode = 1302;
        break;
    case ADP_CASHINEND:
        break;
    case ADP_CIM_RETRACT:
        iOprCode = 1305;
        break;
    case ADP_CIM_RESET:
        iOprCode = 1313;
        break;
    case ADP_DISPENSE:
        iOprCode = 302;
        break;
    case ADP_CDM_RETRACT:
        iOprCode = 305;
        break;
    case ADP_CDM_RESET:
        iOprCode = 321;
        break;
    default:
        break;
    }

    //无需记录的命令直接返回
    if(-1 == iOprCode){
        return;
    }

    char szOperationTime[7] = {0};
    sprintf(szOperationTime, "%02d%02d%02d", m_sOperationTime.wHour,
            m_sOperationTime.wMinute, m_sOperationTime.wSecond);

    //按照格式将信息装换为字符串
    stringstream ss;
    string strTradeInfo;
    ss << (m_bFSN18 ? "2018" : "2014");

    ss << "," << lpszFsnFilePath << "," << "CNY";
    ss << "|" << szOperationTime << ":" << iOprCode << "\n";
    strTradeInfo = ss.str();
    ss.str("");
 //   Log("GenerateDailyCumulativeFileFSN", 0, "单次交易信息：％s", strTradeInfo.c_str());

    //获取文件全路径
    char szFilePath[MAX_PATH] = {0};
    strcpy(szFilePath, m_strDailyCumulativeFileDirFSN.c_str());
    if(szFilePath[strlen(szFilePath) - 1] != '/'){
        strcat(szFilePath, "/");
    }

    //判断目录是否存在，不存在则创建
    if(!IsDirExist(szFilePath)){
        Log("GenerateDailyCumulativeFileFSN", __LINE__, "创建目录%s失败", szFilePath);
        return;
    }

    sprintf(szFilePath + strlen(szFilePath), "%04d%02d%02d.txt", m_sOperationTime.wYear,
                m_sOperationTime.wMonth, m_sOperationTime.wDay);

    //交易冠字号信息写入文件
    FILE *fp = fopen(szFilePath, "a+");
    if(fp == nullptr){
        Log("GenerateDailyCumulativeFileFSN", __LINE__, "打开文件%s失败", szFilePath);
    } else {
        if(fwrite(strTradeInfo.c_str(), strTradeInfo.size(), 1, fp) != 1){
            Log("GenerateDailyCumulativeFileFSN", __LINE__, "写入文件%s失败", szFilePath);
        }
        fclose(fp);
    }

    return;
}

void CSaveCusSNInfo::CheckAndDeleteDailyCumulativeFile()            //test#8
{
    char szTmp[MAX_PATH] = {0};
    time_t curTime = time(NULL);
    time_t deadLine = curTime - m_dwDailyCumulativeFileSaveDays * 24 * 60 * 60;
    tm *pTime = localtime(&deadLine);
    strftime(szTmp, MAX_PATH, "%Y%m%d", pTime);
    string strDeadLine = szTmp;

    QDir dir(m_strDailyCumulativeFileDir.c_str());
    if (!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);  //设置过滤
    QFileInfoList fileList = dir.entryInfoList();            // 获取所有的文件信息
    foreach (QFileInfo file, fileList)
    {
        if (file.isFile())
        {
            std::string strFileName = file.baseName().toStdString();
            if (strFileName <= strDeadLine)
            {
                if (!file.dir().remove(file.absoluteFilePath()))
                {
                    Log("CheckAndDeleteDailyCumulativeFile", __LINE__, "删除文件(%s)失败", file.absoluteFilePath().toStdString().c_str());
                }
            }
        }
    }

    return;
}

//30-00-00-00(FS#0019) 删除每日累加文件
void CSaveCusSNInfo::CheckAndDeleteDailyCumulativeFileFSN()
{
    char szTmp[MAX_PATH] = {0};
    time_t curTime = time(NULL);
    time_t deadLine = curTime - m_dwDailyCumulativeFileSaveDaysFSN * 24 * 60 * 60;
    tm *pTime = localtime(&deadLine);
    strftime(szTmp, MAX_PATH, "%Y%m%d", pTime);
    string strDeadLine = szTmp;

    QDir dir(m_strDailyCumulativeFileDirFSN.c_str());
    if (!dir.exists())
    {
        return;
    }
    dir.setFilter(QDir::AllEntries | QDir::NoDotAndDotDot);  //设置过滤
    QFileInfoList fileList = dir.entryInfoList();            // 获取所有的文件信息
    foreach (QFileInfo file, fileList)
    {
        if (file.isFile())
        {
            std::string strFileName = file.baseName().toStdString();
            if (strFileName <= strDeadLine)
            {
                if (!file.dir().remove(file.absoluteFilePath()))
                {
                    Log("CheckAndDeleteDailyCumulativeFileFSN", __LINE__, "删除文件(%s)失败", file.absoluteFilePath().toStdString().c_str());
                }
            }
        }
    }

    return;

}

bool CSaveCusSNInfo::SaveToDatabase()
{
    SNDatabase sn_db;
    SNDBInfo sn_info;
    SNDBNoteInfo note_info;
    int idx = 0;

    if ((m_strSNDBFilePath.empty()) ||
            (sn_db.init_db(m_strSNDBFilePath) == false)) {
        return false;
    }

    // sn_info.act_name
    if (m_oper == SAVESERIALOPERAT_D) {
        sn_info.act_name = "取款";
    } else if (m_oper == SAVESERIALOPERAT_S) {
        sn_info.act_name = "存款收纳";
    } else if (m_oper == SAVESERIALOPERAT_C) {
        sn_info.act_name = "存款计数";
    } else if ((m_oper == SAVESERIALOPERAT_R) ||
               (m_oper == SAVESERIALOPERAT_J)) {
        sn_info.act_name = "回收";
    } else if (m_oper == SAVESERIALOPERAT_B) {
        sn_info.act_name = "存款取消";
    }

    // sn_info.act_time
    sn_info.act_time.year = m_sOperationTime.wYear;
    sn_info.act_time.month = m_sOperationTime.wMonth;
    sn_info.act_time.day = m_sOperationTime.wDay;
    sn_info.act_time.hour = m_sOperationTime.wHour;
    sn_info.act_time.minute = m_sOperationTime.wMinute;
    sn_info.act_time.second = m_sOperationTime.wSecond;
    sn_info.act_time.milli_second = m_sOperationTime.wMilliseconds;

    // sn_info.num_of_notes
    sn_info.num_of_notes = (int)m_vSNInfoDetails.size();
    for (idx = 0; idx < sn_info.num_of_notes; idx ++) {
        note_info.serial_number = m_vSNInfoDetails[idx].szSerialNO;
        note_info.currency_id = m_vSNInfoDetails[idx].szCurrencyID;
        // note_id & value
        note_info.note_id = m_vSNInfoDetails[idx].DnoCode;
        switch (note_info.note_id) {
        case BRMDENOMINATION_CODE_100_C:
        case BRMDENOMINATION_CODE_100_D:
        case BRMDENOMINATION_CODE_100_B:
            note_info.value = 100;
            break;
        case BRMDENOMINATION_CODE_50_C:
        case BRMDENOMINATION_CODE_50_B:
        case BRMDENOMINATION_CODE_50_D:
            note_info.value = 50;
            break;
        case BRMDENOMINATION_CODE_20_C:
        case BRMDENOMINATION_CODE_20_B:
        case BRMDENOMINATION_CODE_20_D:
            note_info.value = 20;
            break;
        case BRMDENOMINATION_CODE_10_C:
        case BRMDENOMINATION_CODE_10_B:
        case BRMDENOMINATION_CODE_10_D:
            note_info.value = 10;
            break;
        case BRMDENOMINATION_CODE_5_C:
        case BRMDENOMINATION_CODE_5_B:
            note_info.value = 5;
            break;
        case BRMDENOMINATION_CODE_1_B:
        case BRMDENOMINATION_CODE_1_D:
            note_info.value = 1;
            break;
        default:
            note_info.value = 0;
            break;
        }
        // note_ver
        if (m_vSNInfoDetails[idx].NoteEdition == NoteEdition_1999) {
            note_info.note_ver = "1999";
        } else if (m_vSNInfoDetails[idx].NoteEdition == NoteEdition_2005) {
            note_info.note_ver = "2005";
        } else if (m_vSNInfoDetails[idx].NoteEdition == NoteEdition_2015) {
            note_info.note_ver = "2015";
        } else if (m_vSNInfoDetails[idx].NoteEdition == NoteEdition_2019) {
            note_info.note_ver = "2019";
        } else {
            note_info.note_ver = "9999";
        }
        // origin
        if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_CS) {
            note_info.origin = "存取款口";
        } else if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_ESC) {
            note_info.origin = "暂存区";
        } else if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_URJB) {
            note_info.origin = "URJB";
        } else if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_1) {
            note_info.origin = "1A";
        } else if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_2) {
            note_info.origin = "2A";
        } else if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_3) {
            note_info.origin = "3A";
        } else if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_4) {
            note_info.origin = "4A";
        } else if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_5) {
            note_info.origin = "5A";
        } else if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_6) {
            note_info.origin = "1B";
        } else if (m_vSNInfoDetails[idx].SourCass == CASS_ROOM_ID_7) {
            note_info.origin = "1C";
        } else {
            note_info.origin = "未知";
        }
        // destination
        if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_CS) {
            note_info.destination = "存取款口";
        } else if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_ESC) {
            note_info.destination = "暂存区";
        } else if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_URJB) {
            note_info.destination = "URJB";
        } else if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_1) {
            note_info.destination = "1A";
        } else if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_2) {
            note_info.destination = "2A";
        } else if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_3) {
            note_info.destination = "3A";
        } else if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_4) {
            note_info.destination = "4A";
        } else if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_5) {
            note_info.destination = "5A";
        } else if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_6) {
            note_info.destination = "1B";
        } else if (m_vSNInfoDetails[idx].DestCass == CASS_ROOM_ID_7) {
            note_info.destination = "1C";
        } else {
            note_info.destination = "未知";
        }
        // category
        if (m_vSNInfoDetails[idx].NoteCateGory == NOTECATEGORY_4B) {
            note_info.category = "损钞";
        } else if (m_vSNInfoDetails[idx].NoteCateGory == NOTECATEGORY_4) {
            note_info.category = "正钞";
        } else if (m_vSNInfoDetails[idx].NoteCateGory == NOTECATEGORY_3) {
            note_info.category = "疑钞";
        } else if (m_vSNInfoDetails[idx].NoteCateGory == NOTECATEGORY_2) {
            note_info.category = "伪钞";
        } else if (m_vSNInfoDetails[idx].NoteCateGory == NOTECATEGORY_1) {
            note_info.category = "拒钞";
        } else {
            note_info.category = "未知";
        }
        // reject_cause
        if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_RESERVED) {
            note_info.reject_cause = "OK";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_SHIFT) {
            note_info.reject_cause = "SHIFT";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_SKEW) {
            note_info.reject_cause = "SKEW";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_EXSKEW) {
            note_info.reject_cause = "EXSKEW";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_LONG) {
            note_info.reject_cause = "LONG";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_SPACING) {
            note_info.reject_cause = "SPACING";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_INTERVAL) {
            note_info.reject_cause = "INTERVAL";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_DOUBLE) {
            note_info.reject_cause = "DOUBLE";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_DIMENSTION_ERR) {
            note_info.reject_cause = "DIMENSTION_ERR";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_DENO_UNIDENTIFIED) {
            note_info.reject_cause = "DENO_UNIDENTIFIED";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_VERIFICATION) {
            note_info.reject_cause = "VERIFICATION";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_UNFIT) {
            note_info.reject_cause = "UNFIT";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_BV_OTHERS) {
            note_info.reject_cause = "BV_OTHERS";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_DIFF_DENO) {
            note_info.reject_cause = "DIFF_DENO";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_EXCESS) {
            note_info.reject_cause = "EXCESS";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_FACTOR1) {
            note_info.reject_cause = "FACTOR1";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_FACTOR2) {
            note_info.reject_cause = "FACTOR2";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_NO_VALIDATION) {
            note_info.reject_cause = "NO_VALIDATION";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_BV_FORMAT_ERR) {
            note_info.reject_cause = "BV_FORMAT_ERR";
        } else if (m_vSNInfoDetails[idx].RejectCause == REJECT_CAUSE_OTHER) {
            note_info.reject_cause = "OTHER";
        } else {
            note_info.reject_cause = "UNKOWN";
        }

        note_info.sn_image_path = m_vSNInfoDetails[idx].szSNFullPath;
        note_info.full_image_path1 = m_vSNInfoDetails[idx].szImgAFullPath;
        note_info.full_image_path2 = m_vSNInfoDetails[idx].szImgBFullPath;

        sn_info.notes_info.push_back(note_info);
    }

    if (m_iSNDBSaveDays > 0) {
        // delete database by time
        std::chrono::duration<int, std::ratio<60*60*24>> days(m_iSNDBSaveDays);
        std::chrono::system_clock::time_point pt = std::chrono::system_clock::now() - days;
        std::time_t tt = std::chrono::system_clock::to_time_t(pt);
        tm* ptm = localtime(&tt);
        SNDBTimeInfo timeInfo;

        memset(&timeInfo, 0x00, sizeof(timeInfo));
        timeInfo.year = ptm->tm_year + 1900;
        timeInfo.month = ptm->tm_mon + 1;
        timeInfo.day = ptm->tm_mday;
        timeInfo.hour = ptm->tm_hour;
        timeInfo.minute = ptm->tm_min;
        timeInfo.second = ptm->tm_sec;

        sn_db.delete_sndb_info(timeInfo);
    }

    if (sn_db.save_sndb_info(sn_info) == false) {
        return false;
    }

    return true;
}

//30-00-00-00(FS#0001)
void CSaveCusSNInfo::vSetFWVerInfo(char cFwVer[512])
{
    strcpy(cFwVersionInfo, cFwVer);
}
//30-00-00-00(FS#0001)
void CSaveCusSNInfo::vSetFSNFileNameList(const char *cFsnFileNameList,  BYTE byType)
{
    if ((cFsnFileNameList == NULL) || (cFsnFileNameList[0] == 0x00)) {
        return;
    }
    if (byType == 1) {
        ;
    } else {

       CAutoSplitByStep cSplit(cFsnFileNameList, ":");
        for(int i = 0; i < cSplit.Count(); i++){
            LPCSTR lpData = cSplit.At(i);
           m_FSNFileNamelist[i] = lpData;
        }

     }
}

//30-00-00-00(FS#0019)
int CSaveCusSNInfo::GetDeviceNoInfo(string sFilePath, LPSTR sDeviceInfo)
{
    const char *ThisModule = "iGetDeviceInof";
    //如果配置文件对象不为空，先释放它
    m_pRootNode = NULL;

    //创建配置文件对象
    if(m_pConfig == nullptr){
        m_pConfig = SMCreateConfig();
        if (m_pConfig == NULL)
        {
            Log(ThisModule, -1, "SMCreateConfig() return NULL");
            return -1;
        }
    }

    //先从ETCDIR目录装入配置文件
    m_sFilePath = sFilePath;
    bool bLoadSucc = m_pConfig->Load(m_sFilePath.c_str());
    if (!bLoadSucc)
    {
        int iRow, iCol;
        m_pConfig->GetErrRowCol(&iRow, &iCol);
        Log(ThisModule, -1,
            "m_pConfig->Load(%s) failed(%s) iRow=%d, iCol=%d",
            m_sFilePath.c_str(), m_pConfig->GetErrDesc(),
            iRow, iCol);
        return -2;
    }

    //得到根节点，如果根节点不存在，释放配置文件对象并返回失败  for PAB
    m_pRootNode = m_pConfig->GetRootNode();
    if (m_pRootNode == NULL ||
        strcmp(m_pRootNode->GetTagName(), "atm") != 0)
    {
        Log(ThisModule, -1, "node atm not exist");
        return -3;
    }

    //查找device节点
    ISMConfigNode *pNode;
    if (FindNode(&pNode, "device", NULL))
    {
        ISMConfigNode *pChild = pNode->FirstChildNode("deviceno");
        if(pChild != NULL){
            if(pChild->GetAttrib("value") != nullptr){
                strcpy(sDeviceInfo, pChild->GetAttrib("value"));
            }
        } else {
            Log(ThisModule, -1, "node deviceno not exist");
            return -4;
        }
    } else {
        Log(ThisModule, -1, "node device not exist");
        return -5;
    }

    return 0;
}

//30-00-00-00(FS#0019)
BOOL CSaveCusSNInfo::FindNode(ISMConfigNode **ppNode, ...)
{
    *ppNode = NULL;

    va_list vl;
    va_start(vl, ppNode);

    ISMConfigNode *pNode = NULL;

    //如果根节点不存在，直接返回FALSE
    if (m_pRootNode == NULL)
        return FALSE;

    //使用可变参数查找对应的节点，直到找不到或者参数为NULL
    pNode = m_pRootNode;
    while (pNode != NULL)
    {
        const char *pKeyName = va_arg(vl, const char *);
        if (pKeyName == NULL)
            break;
        pNode = pNode->FirstChildNode(pKeyName);
    }
    va_end(vl);

    //如果节点为NULL，键名没有找到，返回FALSE
    if (pNode == NULL)
    {
        return FALSE;
    }

    *ppNode = pNode;
    return TRUE;
}

void CSaveCusSNInfo::GetValueAndVersionByDenomId(BYTE byDenomId, USHORT &usValue, USHORT &usNoteVersion)
{
    //NoteVersion:0:1990 1:1999 2:2005 3:2015 4:2019 5:2020 9999:其他
    switch (byDenomId)
    {
    case BRMDENOMINATION_CODE_100_B:
    case BRMDENOMINATION_CODE_100_C:
    case BRMDENOMINATION_CODE_100_D:
        {
            usValue = 100;
            usNoteVersion = (byDenomId == BRMDENOMINATION_CODE_100_B) ? 1 :
                            (byDenomId == BRMDENOMINATION_CODE_100_C) ? 2 : 3;
        }
        break;
    case BRMDENOMINATION_CODE_50_B:
    case BRMDENOMINATION_CODE_50_C:
    case BRMDENOMINATION_CODE_50_D:
        {
            usValue = 50;
            usNoteVersion = (byDenomId == BRMDENOMINATION_CODE_50_B) ? 1 :
                            (byDenomId == BRMDENOMINATION_CODE_50_C) ? 2 : 4;
        }
        break;
    case BRMDENOMINATION_CODE_20_B:
    case BRMDENOMINATION_CODE_20_C:
    case BRMDENOMINATION_CODE_20_D:
    {
        usNoteVersion = (byDenomId == BRMDENOMINATION_CODE_20_B) ? 1 :
                        (byDenomId == BRMDENOMINATION_CODE_20_C) ? 2 : 4;
        usValue = 20;
    }
        break;
    case BRMDENOMINATION_CODE_10_B:
    case BRMDENOMINATION_CODE_10_C:
    case BRMDENOMINATION_CODE_10_D:
    {
        usNoteVersion = (byDenomId == BRMDENOMINATION_CODE_10_B) ? 1 :
                        (byDenomId == BRMDENOMINATION_CODE_10_C) ? 2 : 4;
        usValue = 10;
    }
        break;
    case BRMDENOMINATION_CODE_5_B:
    case BRMDENOMINATION_CODE_5_C:
    case BRMDENOMINATION_CODE_5_D:
    {
        usNoteVersion = (byDenomId == BRMDENOMINATION_CODE_5_B) ? 1 :
                        (byDenomId == BRMDENOMINATION_CODE_5_C) ? 2 : 5;
        usValue = 5;
    }
        break;
    case BRMDENOMINATION_CODE_1_B:
    case BRMDENOMINATION_CODE_1_D:
    {
        usNoteVersion = (byDenomId == BRMDENOMINATION_CODE_1_B) ? 1 : 4;
        usValue = 1;
    }
        break;
    case BRMDENOMINATION_CODE_ALL:
    case BRMDENOMINATION_CODE_00:
    default:
        usNoteVersion = 9999;
        usValue = 0;
        break;
    }

    return;
}

//30-00-00-00(FS#0019)
void CSaveCusSNInfo::SetLevelNErrorCode(LPCSTR lpszErrorCode, Uint16 uErrorCode[])
{
    if(lpszErrorCode == nullptr || strlen(lpszErrorCode) == 0){
        return;
    }
    CAutoSplitByStep autoSplitByStep(lpszErrorCode, ",");
    if(autoSplitByStep.Count() >= 3){
        for(int i = 0; i < 3; i++){
            uErrorCode[i] = atoi(autoSplitByStep.At(i));
        }
    }

    return;
}

bool CSaveCusSNInfo::ClearSNFile()
{
    bool bRet = true;
    //SNR
    if (!QFile::remove(m_strSNInfoSaveDir.c_str()))
    {
        Log("ClearSNFile", __LINE__, "Delete file(%s) Fail", m_strSNInfoSaveDir.c_str());
        bRet = false;
    }

    if (CreateFile(m_strSNInfoSaveDir.c_str())){
        CINIFileReader ConfigFile;
        ConfigFile.LoadINIFile(m_strSNInfoSaveDir.c_str());

        char szSection[128] = "Cash_Info";

        ConfigFile.SetNewSection(szSection);
        CINIWriter cINI = ConfigFile.GetWriterSection(szSection);
        cINI.AddValue("LEVEL4_COUNT", 0);
        cINI.AddValue("LEVEL3_COUNT", 0);
        cINI.AddValue("LEVEL2_COUNT", 0);

        char szTimeBuff[32];
        sprintf(szTimeBuff, "%04d-%02d-%02d %02d:%02d:%02d", m_sOperationTime.wYear, m_sOperationTime.wMonth, m_sOperationTime.wDay, m_sOperationTime.wHour,
                m_sOperationTime.wMinute, m_sOperationTime.wSecond);
        cINI.AddValue("OperationTime", szTimeBuff);

        cINI.Save();
    } else {
        bRet = false;
    }

    //FSN
    std::string strFSNFilePath = GetFSNFileFullPath();
    if(!strFSNFilePath.empty()){
        if(!QFile::remove(strFSNFilePath.c_str())){
            Log("ClearSNFile", __LINE__, "Delete file(%s) Fail", strFSNFilePath.c_str());
            bRet = false;
        }
    }

    return bRet;
}
string CSaveCusSNInfo::GetFSNFileFullPath()
{
    char szFSNFilePath[MAX_PATH] = {0};
    char szFSNDirPath[MAX_PATH] = {0};
    char szFSNFileName[MAX_PATH] = {0};

    SYSTEMTIME st;
    CQtTime::GetLocalTime(st);
    char cMachiNo[16] = {0};

    //获取FSN文件存放目录
    if(m_bFSNSubDateDirSupp){
        sprintf(szFSNDirPath, "%s/%04d%02d%02d", m_strFSNSaveDri.empty() ? STR_SAVEFSNDEF_DIR : m_strFSNSaveDri.c_str(), st.wYear, st.wMonth, st.wDay);
    } else {
        sprintf(szFSNDirPath, "%s", m_strFSNSaveDri.empty() ? STR_SAVEFSNDEF_DIR : m_strFSNSaveDri.c_str());
    }

    if(!m_FSNFileNamelist[0].empty()){
        char cTimeDate[64] = {0};                                                       //30-00-00-00(FS#0019)
        CINIFileReader ReaderConfigFile;
        for(int i = 0; i < m_FSNFileNamelist->size(); i++){                         //30-00-00-00(FS#0001)
            if(m_FSNFileNamelist[i].compare("SpecialChar") == 0){                     //30-00-00-00(FS#0001)
                strcat(szFSNFileName,m_SpecialChar.c_str());                             //30-00-00-00(FS#0001)
            }else if(m_FSNFileNamelist[i].compare("Currency") == 0){                  //30-00-00-00(FS#0001)
                strcat(szFSNFileName,m_CurrencyVal.c_str());                             //30-00-00-00(FS#0001)
            }else if(m_FSNFileNamelist[i].compare("MachineSNo") == 0){                //30-00-00-00(FS#0001)
                memcpy(cMachiNo, m_strMachineNo.c_str(),
                       qMin(m_strMachineNo.size(), sizeof(cMachiNo) - 1));
                strcat(szFSNFileName, cMachiNo);                                        //30-00-00-00(FS#0001)
            }else if(m_FSNFileNamelist[i].compare("Date") == 0){                     //30-00-00-00(FS#0001)
                sprintf(szFSNFileName + strlen(szFSNFileName), "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
            }else if(m_FSNFileNamelist[i].compare("_") == 0){                         //30-00-00-00(FS#0001)
                strcat(szFSNFileName,"_");                                               //30-00-00-00(FS#0001)
            }else if(m_FSNFileNamelist[i].compare("Time") == 0){                      //30-00-00-00(FS#0001)
                sprintf(szFSNFileName + strlen(szFSNFileName), "%02d%02d%02d", st.wHour, st.wMinute, st.wSecond);     //30-00-00-00(FS#0001)
            }else if(m_FSNFileNamelist[i].compare("Index") == 0){                     //30-00-00-00(FS#0019)
                //获取下标值
                //30-00-00-00(FS#0019) start
                int iFSNFileNameIdx = 1;
                if(ReaderConfigFile.LoadINIFile(m_strConfigFilePath.c_str())){
                    CINIReader cINIReader = ReaderConfigFile.GetReaderSection("DEFAULT");
                    iFSNFileNameIdx = (int)cINIReader.GetValue("FSNDateKeepCount", 1);
                }
                sprintf(szFSNFileName + strlen(szFSNFileName), "%04d", iFSNFileNameIdx);         //30-00-00-00(FS#0019)
            } else {                                                                  //30-00-00-00(FS#0001)
                //固定文本                                                             //30-00-00-00(FS#0001)
                strcat(szFSNFileName, m_FSNFileNamelist[i].c_str());                     //30-00-00-00(FS#0001)
            }                                                                         //30-00-00-00(FS#0001)
        }
        //追加文件后缀
        strcat(szFSNFileName, ".");
        strcat(szFSNFileName, m_strFsnFileSuffix.c_str());
    } else {
        sprintf(szFSNFileName, "sp_%04d%02d%02d_%02d%02d%02d.%s", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute, st.wSecond, m_strFsnFileSuffix.c_str());
    }
    sprintf(szFSNFilePath, "%s/%s", szFSNDirPath, szFSNFileName);

    return szFSNFilePath;
}
