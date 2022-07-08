#include "GenerateCustomSNInfo.h"
#include "INIFileReader.h"
#include <sstream>
#include <string>
#include <QDir>
#include <iomanip>
#include <chrono>
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
#endif

static time_t s_LastDelTime = 0;
const char *ThisFile = "GenCusSNInfo";

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
    InitConfig();
}

CSaveCusSNInfo::~CSaveCusSNInfo()
{
    m_vPABNoteSNInfoLV2.clear();
    m_vPABNoteSNInfoLV3.clear();
    m_vPABNoteSNInfoLV4.clear();
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
            //更新最后一次删除时间
            s_LastDelTime = tCurr;                                  //test#8
        }
    } else {
        //启动后第一次动作时删除
        CheckAndDeleteImage(m_strImageSaveDir.c_str());         //test#8
        CheckAndDeleteDailyCumulativeFile();                    //test#8
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
            pData->bIsFSN18 = true;
            pData->fsnHead.stFSNHead18.Counter = stSNInfo.dwCount;
        } else {
            pData->bIsFSN18 = false;
            pData->fsnHead.stFSNHead.Counter = stSNInfo.dwCount;
        }

        pData->pstFSNBody = new LPSTFSNBody[stSNInfo.dwCount];
        memset(pData->pstFSNBody, 0, sizeof(LPSTFSNBody) * stSNInfo.dwCount);

        for (DWORD i = 0; i < stSNInfo.dwCount; i++)
        {
            pData->pstFSNBody[i] = new STFSNBody;
            memset(pData->pstFSNBody[i], 0, sizeof(STFSNBody));
            LPSTFSNBody pFSNBody = pData->pstFSNBody[i];
            LPSTSNINFODETAILS lpSNInfoDetails = (LPSTSNINFODETAILS)stSNInfo.lppSNInfoDetail[i];
            GenerateFSNInfo(lpSNInfoDetails, pFSNBody);
        }

        if(m_bFSN18){
            GenerateFSNFileHeader18(pData);
        }else{
            GenerateFSNFileHeader(pData);
        }

        GenerateFSNFile(pData);

        if (pData != nullptr)
        {
            int iCount = pData->bIsFSN18 ? pData->fsnHead.stFSNHead18.Counter : pData->fsnHead.stFSNHead.Counter;
            for (Uint32 i = 0; i < iCount; i++)
            {
                if (pData->pstFSNBody[i] != nullptr)
                {
                    delete pData->pstFSNBody[i];
                    pData->pstFSNBody[i] = nullptr;
                }
            }
        }
        delete pData->pstFSNBody;
        pData->pstFSNBody = nullptr;
        delete pData;
        pData = nullptr;
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

    switch (lpSnInfoDetails->DnoCode)
    {
    case BRMDENOMINATION_CODE_50_B:
        {
            PABNoteInfo.usNoteVersion = 1;
            PABNoteInfo.usValue = 50;
        }
        break;
    case BRMDENOMINATION_CODE_100_B:                    //30-00-00-00(FT#0016)
        {                                               //30-00-00-00(FT#0016)
            PABNoteInfo.usNoteVersion = 1;              //30-00-00-00(FT#0016)
            PABNoteInfo.usValue = 100;                  //30-00-00-00(FT#0016)
        }                                               //30-00-00-00(FT#0016)
        break;                                          //30-00-00-00(FT#0016)
    case BRMDENOMINATION_CODE_100_C:
        {
            PABNoteInfo.usNoteVersion = 2;
            PABNoteInfo.usValue = 100;
        }
        break;
    case BRMDENOMINATION_CODE_100_D:
        {
            PABNoteInfo.usNoteVersion = 3;
            PABNoteInfo.usValue = 100;
        }
        break;
    case BRMDENOMINATION_CODE_50_C:
        {
            PABNoteInfo.usNoteVersion = 2;
            PABNoteInfo.usValue = 50;
        }
        break;
    case BRMDENOMINATION_CODE_50_D:
        {
            PABNoteInfo.usNoteVersion = 4;                  //30-00-00-00(FS#0018)
            PABNoteInfo.usValue = 50;
        }
        break;
    //30-00-00-00(FT#0016) add start
    case BRMDENOMINATION_CODE_20_B:
    case BRMDENOMINATION_CODE_20_C:
    case BRMDENOMINATION_CODE_20_D:
    {
        PABNoteInfo.usNoteVersion = (lpSnInfoDetails->DnoCode == BRMDENOMINATION_CODE_20_B) ? 1 :
                                    (lpSnInfoDetails->DnoCode == BRMDENOMINATION_CODE_20_C) ? 2 : 4;    //30-00-00-00(FS#0018)                                                                                                       ;
        PABNoteInfo.usValue = 20;
    }
        break;
    case BRMDENOMINATION_CODE_10_B:
    case BRMDENOMINATION_CODE_10_C:
    case BRMDENOMINATION_CODE_10_D:
    {
        PABNoteInfo.usNoteVersion = (lpSnInfoDetails->DnoCode == BRMDENOMINATION_CODE_10_B) ? 1 :
                                    (lpSnInfoDetails->DnoCode == BRMDENOMINATION_CODE_10_C) ? 2 : 4;    //30-00-00-00(FS#0018)
        PABNoteInfo.usValue = 10;
    }
        break;
    case BRMDENOMINATION_CODE_5_B:
    case BRMDENOMINATION_CODE_5_C:
    case BRMDENOMINATION_CODE_5_D:                      //30-00-00-00(FS#0018)
    {
        PABNoteInfo.usNoteVersion = (lpSnInfoDetails->DnoCode == BRMDENOMINATION_CODE_5_B) ? 1 :        //30-00-00-00(FS#0018)
                                    (lpSnInfoDetails->DnoCode == BRMDENOMINATION_CODE_5_C) ? 2 : 5;     //30-00-00-00(FS#0018)
        PABNoteInfo.usValue = 5;
    }
        break;
    case BRMDENOMINATION_CODE_1_B:
    case BRMDENOMINATION_CODE_1_D:
    {
        PABNoteInfo.usNoteVersion = (lpSnInfoDetails->DnoCode == BRMDENOMINATION_CODE_1_B) ? 1 : 4;     //30-00-00-00(FS#0018)
        PABNoteInfo.usValue = 1;
    }
        break;
    //30-00-00-00(FT#0016) add end
    case BRMDENOMINATION_CODE_ALL:
    case BRMDENOMINATION_CODE_00:
    default:
        PABNoteInfo.usNoteVersion = 9999;
        PABNoteInfo.usValue = 0;
        break;
    }

    PABNoteInfo.szSerialNumber = lpSnInfoDetails->szSerialNO;
    PABNoteInfo.szCurrency = lpSnInfoDetails->szCurrencyID;

    if (lpSnInfoDetails->OperCmd == SAVESERIALOPERAT_D )
    {
        if (lpSnInfoDetails->DestCass == CASS_ROOM_ID_CS)
        {
            PABNoteInfo.unotelevel = NOTELEVEL_LEVEL4;
            m_vPABNoteSNInfoLV4.push_back(PABNoteInfo);
        }
        else
        {
            PABNoteInfo.unotelevel = NOTELEVEL_LEVEL2;
            m_vPABNoteSNInfoLV2.push_back(PABNoteInfo);
        }
    }
    else if (lpSnInfoDetails->OperCmd == SAVESERIALOPERAT_S)
    {
        PABNoteInfo.unotelevel = NOTELEVEL_LEVEL4;
        m_vPABNoteSNInfoLV4.push_back(PABNoteInfo);
    }
    else
    {
        switch (lpSnInfoDetails->NoteCateGory)
        {
        case NOTECATEGORY_4:
        {
            PABNoteInfo.unotelevel = NOTELEVEL_LEVEL4;
            m_vPABNoteSNInfoLV4.push_back(PABNoteInfo);
            break;
        }
        default:
        {
            PABNoteInfo.unotelevel = NOTELEVEL_LEVEL2;
            m_vPABNoteSNInfoLV2.push_back(PABNoteInfo);
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

    if (!m_bSNAddingMode)   // T.B.D
    {
        if (!QFile::remove(m_strSNInfoSaveDir.c_str()))
        {
            Log("GenerateFSNFileHeader", __LINE__, "DeleteDir(%s) Failed", m_strSNInfoSaveDir.c_str());
        }

        if (!CreateFile(m_strSNInfoSaveDir.c_str()))
            return;
    }

    CINIFileReader ConfigFile;
    ConfigFile.LoadINIFile(m_strSNInfoSaveDir.c_str());

    char szSection[128] = "Cash_Info";

    ConfigFile.SetNewSection(szSection);

    UINT oL4_Cnt = 0, oL3_Cnt = 0, oL2_Cnt = 0, oL_Total = 0; // old value
    if (m_bSNAddingMode)
    {
        CINIReader cINI_R = ConfigFile.GetReaderSection(szSection);
        oL4_Cnt = (UINT)cINI_R.GetValue("LEVEL4_COUNT", 0);
        oL3_Cnt = (UINT)cINI_R.GetValue("LEVEL3_COUNT", 0);
        oL2_Cnt = (UINT)cINI_R.GetValue("LEVEL2_COUNT", 0);
        oL_Total = oL4_Cnt + oL3_Cnt + oL2_Cnt;
    }

    CINIWriter cINI = ConfigFile.GetWriterSection(szSection);
    cINI.AddValue("LEVEL4_COUNT", m_vPABNoteSNInfoLV4.size() + oL4_Cnt);
    cINI.AddValue("LEVEL3_COUNT", m_vPABNoteSNInfoLV3.size() + oL3_Cnt);
    cINI.AddValue("LEVEL2_COUNT", m_vPABNoteSNInfoLV2.size() + oL2_Cnt);

    if (!m_bSNAddingMode)
    {
        char szTimeBuff[32];
        sprintf(szTimeBuff, "%04d-%02d-%02d %02d:%02d:%02d", m_sOperationTime.wYear, m_sOperationTime.wMonth, m_sOperationTime.wDay, m_sOperationTime.wHour,
                m_sOperationTime.wMinute, m_sOperationTime.wSecond);
        cINI.AddValue("OperationTime", szTimeBuff);
    }

    cINI.Save();
    vector<NOTESNINFO>::const_iterator it;
    int i = 1 + oL4_Cnt;
    for (it = m_vPABNoteSNInfoLV4.begin(); it != m_vPABNoteSNInfoLV4.end(); it++, i++)
    {
        if(m_iLevelNIdxFormat == 0){                    //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL4_%03d", i);       //30-00-00-00(FT#0017)
        } else {                                        //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL4_%d", i);         //30-00-00-00(FT#0017)
        }                                               //30-00-00-00(FT#0017)
        ConfigFile.SetNewSection(szSection);
        cINI = ConfigFile.GetWriterSection(szSection);
        cINI.AddValue("Index", (*it).usIndex + oL_Total);
        cINI.AddValue("Currency", (*it).szCurrency);
        cINI.AddValue("Value", (*it).usValue);
        cINI.AddValue("NoteVersion", (*it).usNoteVersion);
        cINI.AddValue("SerialNumber", (*it).szSerialNumber);
        cINI.AddValue("ImageFile", (*it).szImageFilePath);
        cINI.Save();
    }

    for (i = 1 + oL3_Cnt, it = m_vPABNoteSNInfoLV3.begin(); it != m_vPABNoteSNInfoLV3.end(); it++, i++)
    {
        if(m_iLevelNIdxFormat == 0){                                //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL3_%03d", i);                   //30-00-00-00(FT#0017)
        } else {                                                    //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL3_%d", i);                     //30-00-00-00(FT#0017)
        }                                                           //30-00-00-00(FT#0017)
        ConfigFile.SetNewSection(szSection);
        cINI = ConfigFile.GetWriterSection(szSection);
        cINI.AddValue("Index", (*it).usIndex + oL_Total);
        cINI.AddValue("Currency", (*it).szCurrency);
        cINI.AddValue("Value", (*it).usValue);
        cINI.AddValue("NoteVersion", (*it).usNoteVersion);
        cINI.AddValue("SerialNumber", (*it).szSerialNumber);
        cINI.AddValue("ImageFile", (*it).szImageFilePath);
        cINI.Save();
    }

    for (i = 1 + oL2_Cnt, it = m_vPABNoteSNInfoLV2.begin(); it != m_vPABNoteSNInfoLV2.end(); it++, i++)
    {
        if(m_iLevelNIdxFormat == 0){                    //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL2_%03d", i);       //30-00-00-00(FT#0017)
        } else {                                        //30-00-00-00(FT#0017)
            sprintf(szSection, "LEVEL2_%d", i);         //30-00-00-00(FT#0017)
        }                                               //30-00-00-00(FT#0017)
        ConfigFile.SetNewSection(szSection);
        cINI = ConfigFile.GetWriterSection(szSection);
        cINI.AddValue("Index", (*it).usIndex + oL_Total);
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

void CSaveCusSNInfo::GenerateFSNInfo(const LPSTSNINFODETAILS lpSNInfoDetails, LPSTFSNBody lpFSNBody)
{
    PRODUCTIONINFO stProductionInfo;                //test#26
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

    lpFSNBody->Ver = 2;  //人民币年版标识，0：1990，1：1999，2:2005
    switch (lpSNInfoDetails->DnoCode)
    {
    case	BRMDENOMINATION_CODE_100_B:    lpFSNBody->Valuta = 100; lpFSNBody->Ver = 1; break;
    case	BRMDENOMINATION_CODE_100_C:    lpFSNBody->Valuta = 100; lpFSNBody->Ver = 2; break;
    case	BRMDENOMINATION_CODE_100_D:    lpFSNBody->Valuta = 100; lpFSNBody->Ver = 3; break;
    case	BRMDENOMINATION_CODE_50_B:     lpFSNBody->Valuta = 50;  lpFSNBody->Ver = 1; break;
    case	BRMDENOMINATION_CODE_50_C:     lpFSNBody->Valuta = 50;  lpFSNBody->Ver = 2; break;
    case    BRMDENOMINATION_CODE_50_D:     lpFSNBody->Valuta = 50;  lpFSNBody->Ver = 3;  break;
    case    BRMDENOMINATION_CODE_00:          // 未指定
    default:                                 lpFSNBody->Valuta = 0;  lpFSNBody->Ver = 9999; break;
    }

    memset(lpFSNBody->ErrorCode, 0, 3 * sizeof(Uint16));  //假币特征码(1-12)，真币时为0

    // 0为假币或可疑币， 1 为真币， 2为残币，3为旧币
    switch (lpSNInfoDetails->NoteCateGory)
    {
    case NOTECATEGORY_4:
    {
        lpFSNBody->tfFlag = 1;
        lpFSNBody->ErrorCode[0] = (Uint16)ERRCODE_0;
        break;
    }
    default:
    {
        lpFSNBody->tfFlag = 0;
        lpFSNBody->ErrorCode[0] = (Uint16)ERRCODE_2;
        break;
    }
    }

    m_stProductionInfo.Money_Type = uiGetMoneyTypeByLevelForAct(lpSNInfoDetails->NoteCateGory);   //test#26

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
        char szYear[32] = {0};
        sprintf(szYear, "%d", lpSNInfoDetails->sSTime.wYear);
        char szMachineSNo[25];
//test#26        sprintf(szMachineSNo, "%s/CFES/UR2", &szYear[2]);
        sprintf(szMachineSNo, "%s", m_cMachineNo);                 //test#26

        Uint16 *p = lpFSNBody->MachineSNo;
        int i = 0;
        char *pSN = szMachineSNo;
        while (*pSN != 0 && i < 24)
        {
            *p++ = *pSN++;
            i++;
        }
    }

    {                                                                                   //test#26
        memcpy(&lpFSNBody->Reserve1, &m_stProductionInfo, sizeof(PRODUCTIONINFO));      //test#26
     }                                                                                   //test#26
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
    pdata->fsnHead.stFSNHead18.HeadString[1] = 2;
    pdata->fsnHead.stFSNHead18.HeadString[2] = 0x2E;
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
     Uint16 uFinanInsLen = m_strfinanIns.size();               
     if(uFinanInsLen > sizeof(pdata->fsnHead.stFSNHead18.FinanIns)){
        uFinanInsLen = sizeof(pdata->fsnHead.stFSNHead18.FinanIns);
     }                                                         
     strncpy((char *)pdata->fsnHead.stFSNHead18.FinanIns, m_strfinanIns.c_str(), uFinanInsLen);

     //设备启动时间 0000 Enabletime
      pdata->fsnHead.stFSNHead18.Enabletime = 0x30;

    int i = 0;
    //机具编号  公司缩写/机具编号
     for(i = 0; i < m_strMakerName.size(); i++){          
        pdata->fsnHead.stFSNHead18.MachineSNo[i] =  m_strMakerName[i];
     }                                               
     for(i = 0; i < m_strMachineNo.size(); i++){       
        pdata->fsnHead.stFSNHead18.MachineSNo[i+5] = m_strMachineNo[i];
     }                                                   
     //机具类型
    for(i = 0; i < m_strMachineType.size(); i++){
     pdata->fsnHead.stFSNHead18.MachineType[i] = m_strMachineType[i];

    }
    //机具型号
    for(i = 0; i < m_strMachineModel.size(); i++){
     pdata->fsnHead.stFSNHead18.MachineModel[i] = m_strMachineModel[i];

    }
    //硬件版本号
    for(i = 0; i < 8; i++){
     pdata->fsnHead.stFSNHead18.HardVerNo[i] = (cFwVersionInfo+87)[i];

    }
    //软件版本号
    for(i = 0; i < 8; i++){
     pdata->fsnHead.stFSNHead18.AuthSoftVerNo[i] = (cFwVersionInfo+106)[i];

    }
    //适用卷别
    for(int m = 0; m < 6; m++){
       pdata->fsnHead.stFSNHead18.Applidenom[m] = 1;
    }
    memset(pdata->fsnHead.stFSNHead18.FinanInst, 0x30, sizeof(pdata->fsnHead.stFSNHead18.FinanInst));
    memset(pdata->fsnHead.stFSNHead18.FinanInstOutlet, 0x30, sizeof(pdata->fsnHead.stFSNHead18.FinanInstOutlet));
    memset(pdata->fsnHead.stFSNHead18.Operator, 0x30, sizeof(pdata->fsnHead.stFSNHead18.Operator));
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
    char cMachiNo[16] = {0};                    //30-00-00-00(FS#0001)

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
        if(!m_FSNFileNamelist[0].empty()){                                              //30-00-00-00(FS#0001)
            for(int i = 0; i < m_FSNFileNamelist->size(); i++){                         //30-00-00-00(FS#0001)
                if(m_FSNFileNamelist[i].compare("SpecialChar") == 0){                     //30-00-00-00(FS#0001)
                    strcat(szFileName,m_SpecialChar.c_str());                             //30-00-00-00(FS#0001)
                }else if(m_FSNFileNamelist[i].compare("Currency") == 0){                  //30-00-00-00(FS#0001)
                    strcat(szFileName,m_CurrencyVal.c_str());                             //30-00-00-00(FS#0001)
                }else if(m_FSNFileNamelist[i].compare("MachineSNo") == 0){                //30-00-00-00(FS#0001)
                    if(pdata->bIsFSN18){
                        for(int i = 0; i < sizeof(cMachiNo)-5; i++)                           //30-00-00-00(FS#0001)
                        {
                            cMachiNo[i] = pdata->fsnHead.stFSNHead18.MachineSNo[i+5];
                        }                                                                     //30-00-00-00(FS#0001)
                    }
                    strcat(szFileName,cMachiNo);                                          //30-00-00-00(FS#0001)
                }else if(m_FSNFileNamelist[i].compare("Date") == 0){                     //30-00-00-00(FS#0001)
                    sprintf(szFileName + strlen(szFileName), "%04d%02d%02d", st.wYear, st.wMonth, st.wDay);
                }else if(m_FSNFileNamelist[i].compare("DeviceNo") == 0){                  //30-00-00-00(FS#0001)
                    if(pdata->bIsFSN18){
                        strcat(szFileName,(char *)pdata->fsnHead.stFSNHead18.MachineSNo[5]);            //30-00-00-00(FS#0001)
                    }
                }else if(m_FSNFileNamelist[i].compare("_") == 0){                         //30-00-00-00(FS#0001)
                    strcat(szFileName,"_");                                               //30-00-00-00(FS#0001)
                }else if(m_FSNFileNamelist[i].compare("Time") == 0){                      //30-00-00-00(FS#0001)
                    sprintf(szFileName + strlen(szFileName), "%02d%02d%02d", st.wHour, st.wMinute, st.wSecond);     //30-00-00-00(FS#0001)
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
        if(pdata->bIsFSN18){
            iLen = fwrite(&pdata->fsnHead.stFSNHead18, sizeof(pdata->fsnHead.stFSNHead18), 1, pf);
            iCount = pdata->fsnHead.stFSNHead18.Counter;
        } else {
            iLen = fwrite(&pdata->fsnHead.stFSNHead, sizeof(pdata->fsnHead.stFSNHead), 1, pf);
            iCount = pdata->fsnHead.stFSNHead.Counter;
        }
        for (DWORD j = 0; j < iCount; j++)
        {
            iLen = fwrite(pdata->pstFSNBody[j], sizeof(STFSNBody), 1, pf);
        }
        fclose(pf);

        //        if(!QFile::copy(szSNFilePath, szSNFilePathBak))
        //        {
        //            Log("GenerateFSNFile", -1, "Copy File Failed , %s to %s", szSNFilePath, szSNFilePathBak);
        //            return ;
        //        }
    }
}

void CSaveCusSNInfo::InitConfig()
{
    m_strConfigFilePath = SPETCPATH;
    m_strConfigFilePath += "/";
    m_strConfigFilePath += "SNConfig.ini";
    CHAR cTempvalue[512] = {0};

    QFileInfo info(m_strConfigFilePath.c_str());
    if (!info.exists())
    {
        Log("InitConfig", __LINE__, "SNConfig.ini not exist");
        return;
    }

    CINIFileReader ReaderConfigFile;
    ReaderConfigFile.LoadINIFile(m_strConfigFilePath.c_str());
    CINIReader cINI = ReaderConfigFile.GetReaderSection("DEFAULT");

    // FSN MODE
    m_uFGMode = (FSNGENMODE)(int)cINI.GetValue("FSNGenerateMode", 1);
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
    Log("InitConfig", 1, "bNeedFSNWhenCashIn:%d", m_sNeedFSN.bNeedFSNWhenCashIn);
    Log("InitConfig", 1, "bNeedFSNWhenCashInEnd:%d", m_sNeedFSN.bNeedFSNWhenCashInEnd);
    Log("InitConfig", 1, "bNeedFSNWhenRetractSlot:%d", m_sNeedFSN.bNeedFSNWhenRetractSlot);
    Log("InitConfig", 1, "bNeedFSNWhenRetractStacker:%d", m_sNeedFSN.bNeedFSNWhenRetractStacker);
    Log("InitConfig", 1, "bNeedFSNWhenDispense:%d", m_sNeedFSN.bNeedFSNWhenDispense);

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

    m_strMachineNo = "00000000";
    char szAPFsnConfigFileItem[MAX_PATH] = {0};
    strcpy(szAPFsnConfigFileItem,
           (LPCSTR)cINI.GetValue("APFsnConfigFileItem", "/home/cfes/CFESAgent/ini/virtual_ap.ini,ATM,DeviceID"));
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
            m_strMachineNo = basic_string<char>(iniReader.GetValue(autoSplitByStep.At(2), "00000000"));


        } else {
            Log("InitConfig", __LINE__, "%s not exist", m_strVirtualApFilePath.c_str());
        }
    }
    //30-00-00-00(FS#0001) add end
    //test#26 start
    LPCSTR lpMachineNoFormatList = static_cast<LPCSTR>(cINI.GetValue("MachineNoFormatList", ""));
    memcpy(cTempvalue, lpMachineNoFormatList, strlen(lpMachineNoFormatList));
    vGetMachineNO2(const_cast<char *>(m_strMachineNo.c_str()),cTempvalue);
    vSetFSNProductionInfo(m_uMachineSN0Length,m_uMachineLength);
    //test#26 end

    m_strfinanIns = (LPCSTR)cINI.GetValue("FinanIns", "");                      //30-00-00-00(FS#0001)
    m_strMachineType = (LPCSTR)cINI.GetValue("MachineType", "ZB_PL_ZZJB");      //30-00-00-00(FS#0001)
    m_strMachineModel = (LPCSTR)cINI.GetValue("MachineModel", "TS-EA45-I11");   //30-00-00-00(FS#0001)
    m_strMakerName = (LPCSTR)cINI.GetValue("MakerName", "CFES");                //30-00-00-00(FS#0001)
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

    if(ReaderConfigFile.LoadINIFile(m_strDailyCumulativeApCfgPath)){
        cINI = ReaderConfigFile.GetReaderSection("CFG");
        m_bIsNeedDailyCumulativeFile = (int)cINI.GetValue("IsNeedDailyRecord", 0) == 1 ? true : false;
        m_dwDailyCumulativeFileSaveDays = (int)cINI.GetValue("SaveTime", 120);

        Log("InitConfig", 1, "m_bIsNeedDailyCumulativeFile :%d", (int)m_bIsNeedDailyCumulativeFile);
        Log("InitConfig", 1, "m_dwDailyCumulativeFileSaveDays :%d", m_dwDailyCumulativeFileSaveDays);
    }
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
        switch (lpSNInfoDetails->DnoCode)
        {
        case BRMDENOMINATION_CODE_100_B:
        {
            usNoteVersion = 1;
            usValue = 100;
        }
        case BRMDENOMINATION_CODE_100_C:
        {
            usNoteVersion = 2;
            usValue = 100;
        }
            break;
        case BRMDENOMINATION_CODE_100_D:
        {
            usNoteVersion = 3;
            usValue = 100;
        }
            break;
        case BRMDENOMINATION_CODE_50_B:
        {
            usNoteVersion = 1;
            usValue = 50;
        }
            break;
        case BRMDENOMINATION_CODE_50_C:
        {
            usNoteVersion = 2;
            usValue = 50;
        }
            break;
        case BRMDENOMINATION_CODE_50_D:
        {
            usNoteVersion = 3;
            usValue = 50;
        }
            break;
        case BRMDENOMINATION_CODE_ALL:
        case BRMDENOMINATION_CODE_00:
        default:
            usNoteVersion = 9999;
            usValue = 0;
            break;
        }

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
        return;
    }

    if(fwrite(strTradeInfo.c_str(), strTradeInfo.size(), 1, fp) != 1){
        Log("GenerateDailyCumulativeFile", 0, "写入文件%s失败", szFilePath);
    }

    fclose(fp);
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

//test#26 start
void CSaveCusSNInfo::vGetMachineNO2(CHAR* cInDeviceNo, CHAR* cMachineNoFormatList)
{
    string cTempFarmatList[MACHINENO_KEYWORDS_LIST_MAX_NUM];
    CHAR cTmpBuffer[MACHINE_NO_MAX_LEN] = {0};
    UINT i = 0;
    UINT nCnt = 0;
    unsigned long uLen = 0;
    string MachineInfo = "";
    CQtTime::GetLocalTime(m_OperTime);

    if (cInDeviceNo != nullptr) {
        memset(m_cDeviceNo, 0x00, sizeof(m_cDeviceNo));
        uLen = strlen(cInDeviceNo);
        if (uLen > MACHINE_NO_MAX_LEN-1) {
            uLen = MACHINE_NO_MAX_LEN - 1;
        }
        memcpy(m_cDeviceNo, cInDeviceNo, uLen);
    }
    if ((cMachineNoFormatList == nullptr) || (cMachineNoFormatList[0] == 0x00)) {
        return;
    }
    for (i = 0; i < MACHINENO_KEYWORDS_LIST_MAX_NUM; i++) {
        cTempFarmatList[i] = "";
    }

    CAutoSplitByStep SplitByStep(cMachineNoFormatList, ",");
    memset(m_cMachineNo, 0x00, sizeof(m_cMachineNo));
    nCnt = SplitByStep.Count();
    for (i = 0; i < nCnt; i++) {
        memset(cTmpBuffer, 0x00, sizeof(cTmpBuffer));
        if(strcmp(SplitByStep.At(i),"Year2") == 0){
            sprintf(cTmpBuffer, "%04d",	m_OperTime.wYear);
            strcat(m_cMachineNo, &cTmpBuffer[2]);
        } else if(strcmp(SplitByStep.At(i),"DeviceNo") == 0) {
            uLen = strlen(m_cDeviceNo);
            if (uLen + strlen(m_cMachineNo) > MACHINE_NO_MAX_LEN-1) {
                uLen = MACHINE_NO_MAX_LEN - strlen(m_cMachineNo) - 1;
            }
            m_uMachineSN0Length += uLen;
            memcpy(cTmpBuffer, m_cDeviceNo, uLen);
            strcat(m_cMachineNo, cTmpBuffer);
        } else {
            uLen = strlen(SplitByStep.At(i));
            if(i == 4){
                m_uMachineLength = uLen-2;
                m_uMachineSN0Length = m_uMachineLength;
            }
            if(i == 5){
                m_uMachineSN0Length += uLen;
            }
            if (uLen + strlen(m_cMachineNo) > MACHINE_NO_MAX_LEN-1) {
                uLen = MACHINE_NO_MAX_LEN - strlen(m_cMachineNo) - 1;
            }
            memcpy(cTmpBuffer, SplitByStep.At(i), uLen);
            strcat(m_cMachineNo, cTmpBuffer);
        }
    }
}

void CSaveCusSNInfo::vSetFSNProductionInfo(Uint16 uMachineSN0Length, Uint16 uMachineLength)
{
    memset(&m_stProductionInfo, 0x00, sizeof(m_stProductionInfo));
    m_stProductionInfo.MachineSNo_Length = uMachineSN0Length;
    m_stProductionInfo.Machine_Length = uMachineLength;
    m_stProductionInfo.Machine_Type = MACHINE_TYPE_CASHIN_AND_DISPENSE;
}

UINT CSaveCusSNInfo::uiGetMoneyTypeByLevelForAct(USHORT usLevel)
{
  return ((usLevel != NOTELEVEL_LEVEL4) ? MONEY_TYPE_DOUBTFUL_BILL : MONEY_TYPE_COMMON_INTACT_BILL);
}
//test#26 end
