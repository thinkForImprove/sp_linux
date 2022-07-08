#include "GenerateSN.h"
#include <time.h>
#include <QFile>
#include <QDir>
#include <string.h>
#define ZeroMemory(a, l)    memset(a, 0, l)

static const char *ThisFile = "GenerateSNInfo";
ISaveCusSNInfo *CThreadGenerateSN::m_pSaveCusSNInfo = nullptr;

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

CThreadGenerateSN::CThreadGenerateSN(EnumSaveSerialOperation nOper, ADP_LFS_CMD_ID eLfsCmdId,int iResult)
    : m_OperType(nOper), m_iResult(iResult)
{
    CQtTime::GetLocalTime(m_OperTime);
    SetLogFile(LOGFILE, ThisFile, "UR2");
    if (m_pSaveCusSNInfo == nullptr)
    {
        InitDllSaveFileSNInfo();
    }

    m_pSaveCusSNInfo->GetSaveImgConfig(m_stIMGConfig);  //test#29
    Log("InitDllSaveFileSNInfo", 1, "DefaultImgPath:%s", m_stIMGConfig.szDefImgPath);//test#29

    m_eLfsCmdId = eLfsCmdId;                        //test#8
}

CThreadGenerateSN::~CThreadGenerateSN()
{
    GenerateData();
    ClearData();
}

inline void ReplaceCharInBuff(char *aryBuff, char cOld, char cNew)
{
    assert(aryBuff != nullptr);
    int iLen = strlen(aryBuff);
    for (int i = 0; i < iLen; i++)
    {
        if (aryBuff[i] == cOld)
        {
            aryBuff[i] = cNew;
        }
    }
}

int CThreadGenerateSN::SaveBMPFile(const SNOTESERIALINFOFULLIMAGE *psnotefullimginfo, const char lpFileName[MAX_PATH])
{
    const char *ThisModule =  "SaveBMPFile";
    FILE *pfile;
    pfile = fopen(lpFileName, "wb");
    if (pfile == nullptr)
    {
        return FALSE;
    }

    try
    {
        int len = fwrite(psnotefullimginfo->cFullImgData, sizeof(char), psnotefullimginfo->dwImgDataLen, pfile);
    }
    catch (...)
    {
        Log(ThisModule, -1, "保存图片%s失败:%s(%d)", lpFileName, strerror(errno), errno);
        fclose(pfile);
        return FALSE;
    }
    fclose(pfile);
    return TRUE;
}

BOOL CThreadGenerateSN::SaveBMPFile(const SNOTESERIALINFO *psnoteserialinfo, const char lpFileName[MAX_PATH])
{
    const char *ThisModule =  "SaveBMPFile";
    FILE *pfile;
    pfile = fopen(lpFileName, "wb");
    if (pfile == nullptr)
    {
        Log(ThisModule, -1, "Open File %s失败", lpFileName);
        return FALSE;
    }

    try
    {
        int len = fwrite(psnoteserialinfo->cSerialImgData, sizeof(char), psnoteserialinfo->dwImgDataLen, pfile);
    }
    catch (...)
    {
        Log(ThisModule, -1, "保存图片%s失败:%s(%d)", lpFileName, strerror(errno), errno);
        fclose(pfile);
        return FALSE;
    }
    fclose(pfile);
    //Log(ThisModule, 1, "保存图片%s success", lpFileName);
    return TRUE;
}

void CThreadGenerateSN::ClearData()
{
    MAPSNINFO::iterator its = m_mapNotesSNInfo.begin();
    for (; its != m_mapNotesSNInfo.end(); its++)
    {
        delete its->second;
        its->second = nullptr;
    }
    m_mapNotesSNInfo.clear();

    MAPTRACEINFO::iterator itt = m_mapNotesTraceInfo.begin();
    for (; itt != m_mapNotesTraceInfo.end(); itt++)
    {
        delete itt->second;
        itt->second = nullptr;
    }
    m_mapNotesTraceInfo.clear();

    MAPFULLIMGINFO::iterator itfull_A = m_mapNoteFullImgInfo.begin();
    for (; itfull_A != m_mapNoteFullImgInfo.end(); itfull_A++)
    {
        delete itfull_A->second;
        itfull_A->second = nullptr;
    }
    m_mapNoteFullImgInfo.clear();

    MAPFULLIMGINFO::iterator itfull_B = m_mapNoteFullImgInfo_B.begin();
    for (; itfull_B != m_mapNoteFullImgInfo_B.end(); itfull_B++)
    {
        delete itfull_B->second;
        itfull_B->second = nullptr;
    }

    m_mapNoteFullImgInfo_B.clear();
}

bool CThreadGenerateSN::IsNeedSaveSNIMG()
{
    return m_pSaveCusSNInfo->IsNeedSaveSNImg(m_OperType);
}

bool CThreadGenerateSN::IsNeedSaveFullImg()
{
    return m_pSaveCusSNInfo->IsNeedSaveFullImg(m_OperType);
}

inline void ReplaceChar(char *lpData, char cOld, char cNew)
{
    assert(lpData != nullptr);
    int iLen = strlen(lpData);
    for (int i = 0; i < iLen; i++)
    {
        if (lpData[i] == cOld)
        {
            lpData[i] = cNew;
        }
    }
}

bool CThreadGenerateSN::GenerateData()
{
    LPCSTR ThisModule = "GenerateData";
    Log(ThisModule, 1, "GenerateData start");
    if (!IsNeedSaveSNIMG() && !IsNeedSaveFullImg())
    {
        Log(ThisModule, 0, "not need Save Img");
        return false;
    }

    if (m_pSaveCusSNInfo == nullptr)
    {
        Log(ThisModule, -1, "m_pSaveCusSNInfo == nullptr");
        return false;
    }

    m_pSaveCusSNInfo->SetLfsCmdId(m_eLfsCmdId);
    char szSaveImgPath[MAX_PATH] = {0};
    m_pSaveCusSNInfo->GetSaveImgDir(m_OperTime, m_OperType, szSaveImgPath);

    if (!IsDirExist((QString)szSaveImgPath))
    {
        Log(ThisModule, -1, "创建目录%s失败", szSaveImgPath);
        return false;
    }
    Log(ThisModule, 1, "创建目录%s Success", szSaveImgPath);
    //自动保存图片信息文件
    CAutoSaveFileSNInfo AutoSaveFileSNInfo(m_pSaveCusSNInfo, m_iResult, m_stIMGConfig.uFSNMode);

    //QFile file;
    char szFileName[MAX_PATH] = {0};
    char szFullPath[MAX_PATH] = {0};

    char szFullImgFileName[MAX_PATH] = {0};

    MAPTRACEINFO::iterator ittrace = m_mapNotesTraceInfo.begin();

    for (int i = 0; ittrace != m_mapNotesTraceInfo.end(); ittrace++, i++)
    {
        string strImgSN, strFullImgA, strFullImgB;
        SNOTESERIALINFO NoteSNInfo;
        SNOTESERIALINFO *pSNInfo = &NoteSNInfo;

        if (IsNeedSaveSNIMG())
        {
            MAPSNINFO::iterator it = m_mapNotesSNInfo.find(ittrace->first);
            if (it != m_mapNotesSNInfo.end())
            {

                pSNInfo = it->second;
            }

//test#29            ReplaceChar(pSNInfo->cSerialNumber, '*', '$');
            ReplaceChar(pSNInfo->cSerialNumber, '*', m_stIMGConfig.cSerialNoChar);      //test#29
            FormNoteSerialFileName(i + 1, m_OperTime, pSNInfo, ittrace->second, szFileName);
            sprintf(szFullPath, "%s/%s", szSaveImgPath, szFileName);

            if (pSNInfo->dwImgDataLen <= 0)
            {
                bool bRet = QFile::copy((QString)m_stIMGConfig.szDefImgPath, (QString)szFullPath);
                if (!bRet)
                {
                    Log(ThisModule, -1, "%s复制到%s文件失败", m_stIMGConfig.szDefImgPath, szFullPath);
                }
                //Log(ThisModule, 1, "%s复制到%s文件 success", m_stIMGConfig.szDefImgPath, szFullPath);
            }
            else
            {
                SaveBMPFile(pSNInfo, szFullPath);
            }
            strImgSN = szFullPath;

            if (m_stIMGConfig.ImgType == IMAGETYPE_JPG) //处理JPG格式
            {
                BMPToJPG(strImgSN);
                //Log(ThisModule, 1, "%s  BMPToJPG finish", strImgSN.c_str());
            }
            //Log(ThisModule, 1, "Img%d finish", i+1);
        }

        if (IsNeedSaveFullImg())
        {
            //取全副图像正面
            MAPFULLIMGINFO::iterator itfullimg = m_mapNoteFullImgInfo.find(ittrace->first);
            SNOTESERIALINFOFULLIMAGE *pFullImgInfo = nullptr;
            if (itfullimg != m_mapNoteFullImgInfo.end())
            {
                pFullImgInfo = itfullimg->second;
                if (pFullImgInfo->dwImgDataLen > 0)
                {
                    FormFullImgFileName(i + 1, m_OperTime, ittrace->second, szFullImgFileName, TRUE);
                    sprintf(szFullPath, "%s/%s", szSaveImgPath, szFullImgFileName);
                    SaveBMPFile(pFullImgInfo, szFullPath);

                    strFullImgA = szFullPath;
                    if (m_stIMGConfig.ImgType == IMAGETYPE_JPG) //处理JPG格式
                    {
                        BMPToJPG(strFullImgA);
                    }
                }
            }

            //取全副图像反面
            MAPFULLIMGINFO::iterator itfullimg_B = m_mapNoteFullImgInfo_B.find(ittrace->first);
            pFullImgInfo = nullptr;
            if (itfullimg_B != m_mapNoteFullImgInfo_B.end())
            {
                pFullImgInfo = itfullimg_B->second;
                if (pFullImgInfo->dwImgDataLen >= 0)
                {
                    FormFullImgFileName(i + 1, m_OperTime, ittrace->second, szFullImgFileName, FALSE);
                    sprintf(szFullPath, "%s/%s", szSaveImgPath, szFullImgFileName);
                    SaveBMPFile(pFullImgInfo, szFullPath);
                    strFullImgB = szFullPath;
                    if (m_stIMGConfig.ImgType == IMAGETYPE_JPG) //处理JPG格式
                    {
                        BMPToJPG(strFullImgB);
                    }
                }
            }
        }

        AutoSaveFileSNInfo.SetSNInfoEveryNote(i, m_OperTime, m_OperType,  strImgSN.c_str(),
                                              ittrace->second, pSNInfo, strFullImgA.c_str(), strFullImgB.c_str());
    }

    Log(ThisModule, 1, "冠子码处理完成");
    return true;
}

bool CThreadGenerateSN::BMPToJPG(string &strImgName)
{
    string strNameJpg = strImgName;
    strNameJpg = strNameJpg.substr(0, strNameJpg.rfind('.'));
    strNameJpg += ".JPG";

    if (!SaveImageFile(strImgName.c_str(), strNameJpg.c_str()))
    {
        Log("BMPToJPG", -1, "将图片bmp(%s)转化为JPG(%s)时失败",
            strImgName.c_str(), strNameJpg.c_str());
        return FALSE;
    }
    QFile::remove(strImgName.c_str());
    strImgName = strNameJpg;
    return TRUE;
}

void CThreadGenerateSN::InsertSNInfoDetail(const SNOTESERIALINFO &NoteSNInfo)
{

    LPSNOTESERIALINFO pdata = new SNOTESERIALINFO;
    pdata->dwBVInternalIndex = NoteSNInfo.dwBVInternalIndex;
    pdata->dwImgDataLen = NoteSNInfo.dwImgDataLen;
    pdata->NotesEdition = NoteSNInfo.NotesEdition;
    memcpy(pdata->cSerialNumber, NoteSNInfo.cSerialNumber, MAX_SERIAL_LENGTH);
    memcpy(pdata->cSerialImgData, NoteSNInfo.cSerialImgData, MAX_SNIMAGE_DATA_LENGTH);
    m_mapNotesSNInfo[pdata->dwBVInternalIndex] = pdata;

}

bool CThreadGenerateSN::IsFullImgDataExist(bool bFront, ULONG dwInternCount)
{
    if (bFront)
    {
        return m_mapNoteFullImgInfo.find(dwInternCount) != m_mapNoteFullImgInfo.end();
    }
    else
    {
        return m_mapNoteFullImgInfo_B.find(dwInternCount) != m_mapNoteFullImgInfo_B.end();
    }
}

void CThreadGenerateSN::InsertFullImageInfoDetail(const SNOTESERIALINFOFULLIMAGE &pNoteFullImgInfo, bool bFront)
{
    LPSNOTESERIALINFOFULLIMAGE pdata = new SNOTESERIALINFOFULLIMAGE;
    pdata->dwImgDataLen = pNoteFullImgInfo.dwImgDataLen;
    pdata->dwBVInternalIndex = pNoteFullImgInfo.dwBVInternalIndex;
    memcpy(pdata->cFullImgData, pNoteFullImgInfo.cFullImgData, pdata->dwImgDataLen);

    if (bFront)
    {
        m_mapNoteFullImgInfo[pdata->dwBVInternalIndex] = pdata;
    }
    else
    {
        m_mapNoteFullImgInfo_B[pdata->dwBVInternalIndex] = pdata;
    }
}

void CThreadGenerateSN::InsertTraceInfoDetail(const ST_MEDIA_INFORMATION_INFO &NoteTraceInfo)
{
    ST_MEDIA_INFORMATION_INFO *pdata = new ST_MEDIA_INFORMATION_INFO;
    pdata->ulBVInternalCounter = NoteTraceInfo.ulBVInternalCounter;
    pdata->iMediaInfoOrigin = NoteTraceInfo.iMediaInfoOrigin;
    pdata->iMediaInfoDest = NoteTraceInfo.iMediaInfoDest;
    pdata->iMediaInfoDnoCode = NoteTraceInfo.iMediaInfoDnoCode;
    pdata->iNoteCategory = NoteTraceInfo.iNoteCategory;
    pdata->iBVImage = NoteTraceInfo.iBVImage;
    pdata->iMediaInfoRejectCause = NoteTraceInfo.iMediaInfoRejectCause;
    m_mapNotesTraceInfo[pdata->ulBVInternalCounter] = pdata;
}

bool CThreadGenerateSN::FormFullImgFileName(UINT uIndex, const SYSTEMTIME &stCurr,
                                            const ST_MEDIA_INFORMATION_INFO *pTraceInfo, char lpImageName[MAX_PATH], bool bFront)
{
    ZeroMemory(lpImageName, MAX_PATH * sizeof(char));
    //时间
    sprintf(&lpImageName[0], "%04d%02d%02d%02d%02d%02d", stCurr.wYear, stCurr.wMonth, stCurr.wDay,
            stCurr.wHour, stCurr.wHour, stCurr.wMinute);
    //索引
    sprintf(&lpImageName[14], "%03d", uIndex);
    //正反面
    sprintf(&lpImageName[17], "_%s.bmp", bFront ? "A" : "B");

    return TRUE;
}

//功能：获取保存的序列号图片的名字（默认名）
//输入：dwIndex：单次动作中保存的第几张图片序列号
//      dwNum: 一天中产生保存序列号动作的次数
//输出：lpFileName：获取的序列号图片名
//返回：-1：失败，0： 成功
//说明：内部方法 图片名格式是yyyymmddssD/C/R/S_Index_xxxxxxxxxx.bmp，
// 例如:  20121015093245D001_xxxxxxxxxx.bmp、20121015093245C001_xxxxxxxxxx.bmp
bool CThreadGenerateSN::FormNoteSerialFileName(UINT uIndex,  const SYSTEMTIME &stCurr,  const SNOTESERIALINFO *pSNInfo,
                                               const ST_MEDIA_INFORMATION_INFO *pTraceInfo, char lpImageName[MAX_PATH])
{
    const char *ThisModule = "FormNoteSerialFileName";
    assert(pTraceInfo != nullptr);
    assert(pSNInfo != nullptr);
    assert(lpImageName != nullptr);

    //时间
    sprintf(&lpImageName[0], "%04d%02d%02d%02d%02d%02d", stCurr.wYear, stCurr.wMonth, stCurr.wDay,
            stCurr.wHour, stCurr.wMinute, stCurr.wSecond);

    //获取操作
    switch (m_OperType)
    {
    case SAVESERIALOPERAT_D:  lpImageName[14] = 'D'; break;
    case SAVESERIALOPERAT_S:  lpImageName[14] = 'S';  break;
    case SAVESERIALOPERAT_C:  lpImageName[14] = 'C';  break;
    case SAVESERIALOPERAT_R:  lpImageName[14] = 'R';  break;
    case SAVESERIALOPERAT_J:  lpImageName[14] = 'J';  break;
    default:                  lpImageName[14] = 'U';  break;
    }

    //获取起始位置
    switch (pTraceInfo->iMediaInfoOrigin)
    {
    case ID_ROOM_1:    memcpy(&lpImageName[15], "R1", 2); break;
    case ID_ROOM_2:    memcpy(&lpImageName[15], "R2", 2); break;
    case ID_ROOM_3:    memcpy(&lpImageName[15], "R3", 2); break;
    case ID_ROOM_4:    memcpy(&lpImageName[15], "A1", 2); break;
    case ID_ROOM_5:    memcpy(&lpImageName[15], "R4", 2); break;
    case ID_CS:        memcpy(&lpImageName[15], "CS", 2); break;
    case ID_ESC:       memcpy(&lpImageName[15], "TS", 2); break;
    default:           memcpy(&lpImageName[15], "UN", 2); break;
    }

    //获取目的位置
    switch (pTraceInfo->iMediaInfoDest)
    {
    case ID_ROOM_1:    memcpy(&lpImageName[17], "R1", 2); break;
    case ID_ROOM_2:    memcpy(&lpImageName[17], "R2", 2); break;
    case ID_ROOM_3:    memcpy(&lpImageName[17], "R3", 2); break;
    case ID_ROOM_4:    memcpy(&lpImageName[17], "A1", 2); break;
    case ID_ROOM_5:    memcpy(&lpImageName[17], "R4", 2); break;
    case ID_CS:        memcpy(&lpImageName[17], "CS", 2); break;
    case ID_ESC:       memcpy(&lpImageName[17], "TS", 2); break;
    case ID_URJB:      memcpy(&lpImageName[17], "A2", 2); break;
    default:           memcpy(&lpImageName[17], "UN", 2); break;
    }

    //索引
    sprintf(&lpImageName[19], "%03d", uIndex);

    //币种
    memcpy(&lpImageName[22], "CNY", 3);

    //面额
    switch (pTraceInfo->iMediaInfoDnoCode)
    {
    case DENOMINATION_CODE_100_B:     memcpy(&lpImageName[25], "1001999", 7); break;
    case DENOMINATION_CODE_100_C:     memcpy(&lpImageName[25], "1002005", 7); break;
    case DENOMINATION_CODE_100_D:     memcpy(&lpImageName[25], "1002015", 7); break;
    case DENOMINATION_CODE_20_B:      memcpy(&lpImageName[25], "0201999", 7); break;
    case DENOMINATION_CODE_20_C:      memcpy(&lpImageName[25], "0202005", 7); break;
    case DENOMINATION_CODE_20_D:      memcpy(&lpImageName[25], "0202015", 7); break;
    case DENOMINATION_CODE_10_B:      memcpy(&lpImageName[25], "0101999", 7); break;
    case DENOMINATION_CODE_10_C:      memcpy(&lpImageName[25], "0102005", 7); break;
    case DENOMINATION_CODE_10_D:      memcpy(&lpImageName[25], "0102015", 7); break;
    case DENOMINATION_CODE_50_B:      memcpy(&lpImageName[25], "0502001", 7); break;
    case DENOMINATION_CODE_50_C:      memcpy(&lpImageName[25], "0502005", 7); break;
    case DENOMINATION_CODE_50_D:      memcpy(&lpImageName[25], "0502015", 7); break;
    case DENOMINATION_CODE_ALL:
    case DENOMINATION_CODE_00:
    default:                                memcpy(&lpImageName[25], "0000000", 7); break;
    }

    //获取验钞结果
    switch (pTraceInfo->iNoteCategory)
    {
    case NOTE_CATEGORY_4:
        {
            if (pTraceInfo->iMediaInfoRejectCause == MEDIA_INFORMATION_REJECT_CAUSE_RESERVED)
            {
                memcpy(&lpImageName[32], "4F", 2);
            }
            else
            {
                memcpy(&lpImageName[32], "1V", 2);
            }
            break;
        }
    case NOTE_CATEGORY_4B:                  memcpy(&lpImageName[32], "4U", 2); break;
    case NOTE_CATEGORY_1:
    case NOTE_CATEGORY_2:
    case NOTE_CATEGORY_3:                   memcpy(&lpImageName[32], "1V", 2); break;
    case NOTE_CATEGORY_UNKNOWN:
    default:                                memcpy(&lpImageName[32], "0U", 2); break;
    }

    //获取冠字码
    if (pSNInfo->cSerialNumber[0] != 0)
    {
        sprintf(&lpImageName[34], "_%10s.bmp", pSNInfo->cSerialNumber);

    }
    else
    {
        sprintf(&lpImageName[34], "_%10s.bmp", "$$$$$$$$$$");
    }

    return 0;
}

long CThreadGenerateSN::InitDllSaveFileSNInfo()
{
    const char *ThisModule = "InitDllSaveFileSNInfo";

    if (m_pSaveCusSNInfo != nullptr)
    {
        return 0;
    }

    int iRet = CreateSaveSNInfoInstance(m_pSaveCusSNInfo);
    if (iRet < 0)
    {
        m_pSaveCusSNInfo = nullptr;
        return WFS_ERR_INTERNAL_ERROR;
    }

//test#29    m_pSaveCusSNInfo->GetSaveImgConfig(m_stIMGConfig);
//test#29    Log("InitDllSaveFileSNInfo", 1, "DefaultImgPath:%s", m_stIMGConfig.szDefImgPath);
    return WFS_SUCCESS;
}

CAutoSaveFileSNInfo::CAutoSaveFileSNInfo(ISaveCusSNInfo *pData, int iOperResult, unsigned int uMode)
{
    m_pData = pData;
    m_iOperResult = iOperResult;
    m_uMode = uMode;
}

CAutoSaveFileSNInfo::~CAutoSaveFileSNInfo()
{
    if (m_pData != nullptr && m_vSNInfo.size() > 0)
    {
        STSNINFO stSNInfo;
        stSNInfo.iOperResult = m_iOperResult;
        stSNInfo.dwCount = m_vSNInfo.size();
        stSNInfo.lppSNInfoDetail = new LPSTSNINFODETAILS[stSNInfo.dwCount];
        vector<STSNINFODETAILS>::const_iterator it;
        int i = 0;
        for (it = m_vSNInfo.begin(); it != m_vSNInfo.end() && i < stSNInfo.dwCount ; it++)
        {
            LPSTSNINFODETAILS pDetail = new STSNINFODETAILS;
            *pDetail = *it;
            stSNInfo.lppSNInfoDetail[i++] = pDetail;
        }

        m_pData->OnSNInfoGeneration(stSNInfo);
        if (stSNInfo.dwCount > 0)
        {
            for (i = 0;  i < stSNInfo.dwCount; i++)
            {
                delete stSNInfo.lppSNInfoDetail[i];
                stSNInfo.lppSNInfoDetail[i] = nullptr;
            }
            delete stSNInfo.lppSNInfoDetail;
            stSNInfo.lppSNInfoDetail = nullptr;
        }
    }
    m_pData = nullptr;
}

void CAutoSaveFileSNInfo::SetSNInfoEveryNote(ULONG uIndex, const SYSTEMTIME &stSysTime, EnumSaveSerialOperation OperCmd, LPCSTR szFullPath,
                                             ST_MEDIA_INFORMATION_INFO *pMediaInfo, SNOTESERIALINFO *pSNInfo, LPCSTR lpFullImgA, LPCSTR lpFullImgB)
{
    if (m_pData != nullptr)
    {
        STSNINFODETAILS sSNDetails;
        sSNDetails.uIndex = uIndex;
        sSNDetails.sSTime = stSysTime;
        sSNDetails.OperCmd = OperCmd;
        sSNDetails.RejectCause = (EnumREJECTCAUSE)pMediaInfo->iMediaInfoRejectCause;
        ConvertCassID(pMediaInfo->iMediaInfoDest, sSNDetails.DestCass);
        ConvertCassID(pMediaInfo->iMediaInfoOrigin, sSNDetails.SourCass);
        sSNDetails.DnoCode = (EnumBRMDENOMINATION_CODE)pMediaInfo->iMediaInfoDnoCode;
        ConvertCATEGORY(pMediaInfo->iNoteCategory, pMediaInfo->iMediaInfoRejectCause, sSNDetails.NoteCateGory);
        ConverNoteEdition(pSNInfo->NotesEdition, sSNDetails.NoteEdition);
        memcpy(sSNDetails.szCurrencyID, "CNY", 3);
        memcpy(sSNDetails.szSerialNO, pSNInfo->cSerialNumber, MAX_SERIAL_LENGTH);
        strcpy(sSNDetails.szSNFullPath, szFullPath);
        strcpy(sSNDetails.szImgAFullPath, lpFullImgA);
        strcpy(sSNDetails.szImgBFullPath, lpFullImgB);
        UINT cashvalue = 100;
        if (pMediaInfo->iMediaInfoDnoCode = DENOMINATION_CODE_100_C)
        {
            cashvalue = 10;
        }
        else if (pMediaInfo->iMediaInfoDnoCode == DENOMINATION_CODE_50_C)
        {
            cashvalue = 50;
        }
        SNIP_Parser(szFullPath, cashvalue, m_uMode, sSNDetails.ImageSNo);
        m_vSNInfo.push_back(sSNDetails);
    }
}

//test#28
void CThreadGenerateSN::DeleteSnrinfomation()
{
    m_pSaveCusSNInfo->vDeleteSnrinfoForOpen();
}

