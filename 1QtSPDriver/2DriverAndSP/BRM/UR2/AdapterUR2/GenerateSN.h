#pragma once
#include "IURDevice.h"
#include "ISaveBRMCustomSNInfo.h"
#include "XFSAPI.H"
#include "ISNImageParser.h"
#include "ILogWrite.h"
#include "AutoQtHelpClass.h"
#include <assert.h>
#include "IBRMAdapter.h"                //test#8

typedef struct _gsn_serial_info
{
    ST_MEDIA_INFORMATION_INFO   mediainfo;
    SNOTESERIALINFO            noteserialinfo;
    USHORT usNoteID;
} SGSNSERIALINFO, *LPSGSNSERIALINFO;

inline void ConvertCassID(const CASSETTE_ROOM_ID &URBoxID, EnumCASSETTEROOMID &CassRoomID)
{
    switch (URBoxID)
    {
    case ID_CS:      CassRoomID = CASS_ROOM_ID_CS;   break;
    case ID_URJB:    CassRoomID = CASS_ROOM_ID_URJB; break;
    case ID_ESC:     CassRoomID = CASS_ROOM_ID_ESC;  break;
    case ID_ROOM_1:  CassRoomID = CASS_ROOM_ID_1; break;
    case ID_ROOM_2:  CassRoomID = CASS_ROOM_ID_2; break;
    case ID_ROOM_3:  CassRoomID = CASS_ROOM_ID_3; break;
    case ID_ROOM_4:  CassRoomID = CASS_ROOM_ID_4; break;
    case ID_ROOM_5:  CassRoomID = CASS_ROOM_ID_5; break;
    case ID_ROOM_1B: CassRoomID = CASS_ROOM_ID_6; break;
    case ID_ROOM_1C: CassRoomID = CASS_ROOM_ID_7; break;
    case ID_ROOM_RESERVED:
    default:         CassRoomID = CASS_ROOM_ID_RESERVED; break;
    }
}

inline void ConvertCATEGORY(const NOTE_CATEGORY &URCategory,
                            const MEDIA_INFORMATION_REJECT_CAUSE &URRejCaus,
                            EnumNOTECATEGORY &NoteCateGory)
{
    switch (URCategory)
    {
    case NOTE_CATEGORY_4:
    case NOTE_CATEGORY_4B:
        {
            if (URRejCaus == MEDIA_INFORMATION_REJECT_CAUSE_RESERVED)
            {
                NoteCateGory = NOTECATEGORY_4;
            }
            else
            {
                NoteCateGory = NOTECATEGORY_1;
            }
            break;
        }
    case NOTE_CATEGORY_1:
    case NOTE_CATEGORY_2:
    case NOTE_CATEGORY_3:                   NoteCateGory = NOTECATEGORY_1; break;
    case NOTE_CATEGORY_UNKNOWN:
    default:                                                NoteCateGory = NOTECATEGORY_UNKNOWN; break;
    }
}

inline void ConverNoteEdition(eNoteEdition HCMNoteVer, EnumNoteEdition &NoteEdition)
{
    switch (HCMNoteVer)
    {
    case eNoteEdition_1999_100:  NoteEdition = NoteEdition_1999; break;
    case eNoteEdition_2005_010:
    case eNoteEdition_2005_020:
    case eNoteEdition_2005_050:
    case eNoteEdition_2005_100:  NoteEdition = NoteEdition_2005; break;
    case eNoteEdition_2015_100:  NoteEdition = NoteEdition_2015; break;
    default:                     NoteEdition = NoteEdition_unknown; break;
    }
}

typedef int (*PFSAVEFILESNINFO)(ISaveCusSNInfo *&);
//保存冠字号信息文件
class CAutoSaveFileSNInfo
{
public:
    CAutoSaveFileSNInfo(ISaveCusSNInfo *pData, int iOperResult, unsigned int uMode);
    ~CAutoSaveFileSNInfo();
    void SetSNInfoEveryNote(ULONG uIndex, const SYSTEMTIME &stSysTime, EnumSaveSerialOperation OperCmd, LPCSTR szFullPath,
                            ST_MEDIA_INFORMATION_INFO *pMediaInfo, SNOTESERIALINFO *pSNInfo, LPCSTR lpFullImgA, LPCSTR lpFullImgB);
    void GetSaveImgDir(const SYSTEMTIME &stSysTime, EnumSaveSerialOperation OperCmd,
                       char lpSaveImgPath[MAX_PATH]);

private:
    ISaveCusSNInfo *m_pData;
    int m_iOperResult;
    unsigned int m_uMode;
    vector<STSNINFODETAILS> m_vSNInfo;
};

typedef map<ULONG, ST_MEDIA_INFORMATION_INFO *>  MAPTRACEINFO;
typedef map<ULONG, SNOTESERIALINFO *>  MAPSNINFO;
typedef map<ULONG, SNOTESERIALINFOFULLIMAGE *>  MAPFULLIMGINFO;

class CThreadGenerateSN : public CLogManage
{
public:
    CThreadGenerateSN(EnumSaveSerialOperation nOper, ADP_LFS_CMD_ID eLfsCmdId, int iResult);            //test#8
    virtual ~CThreadGenerateSN();
    void InsertSNInfoDetail(const SNOTESERIALINFO &pNoteSNInfo);
    void InsertFullImageInfoDetail(const SNOTESERIALINFOFULLIMAGE &pNoteFullImgInfo, bool bFront);
    void InsertTraceInfoDetail(const ST_MEDIA_INFORMATION_INFO &NoteTraceInfo);
    long InitDllSaveFileSNInfo();
    void setFwVerInfo(char cFwVer[512]);            //30-00-00-00(FS#0001)

private:
    bool GenerateData();
    int SaveBMPFile(const SNOTESERIALINFO *psnoteserialinfo, const char lpFileName[MAX_PATH]);
    int SaveBMPFile(const SNOTESERIALINFOFULLIMAGE *psnotefullimginfo, const char lpFileName[MAX_PATH]);

protected:
    EnumSaveSerialOperation m_OperType;
    SYSTEMTIME              m_OperTime;
    int                     m_iResult;

private:
    MAPSNINFO  m_mapNotesSNInfo;
    MAPTRACEINFO m_mapNotesTraceInfo;
    MAPFULLIMGINFO m_mapNoteFullImgInfo;
    MAPFULLIMGINFO m_mapNoteFullImgInfo_B;

    bool FormNoteSerialFileName(UINT uIndex, const SYSTEMTIME &stCurr, const  SNOTESERIALINFO *pSNInfo,
                                const ST_MEDIA_INFORMATION_INFO *pTraceInfo, char lpImageName[MAX_PATH]);
    bool FormFullImgFileName(UINT uIndex, const SYSTEMTIME &stCurr,
                             const ST_MEDIA_INFORMATION_INFO *pTraceInfo, char lpImageName[MAX_PATH], bool bFront);
    void ClearData();
    bool BMPToJPG(string &strImgName);
    void ReadConfig();          //30-00-00-00(FT#0016)
public:
    bool IsNeedSaveSNIMG();
    bool IsNeedSaveFullImg();
    bool IsFullImgDataExist(bool bFront, ULONG dwInternCount);

private:
    vector<LPSGSNSERIALINFO>  m_lGSNSerialInfo;
    //配置项
    STIMGCONFIG            m_stIMGConfig;

    ADP_LFS_CMD_ID         m_eLfsCmdId;
    char                   m_cUnrecognizeDisplayChar;           //30-00-00-00(FT#0016)
private:
    static ISaveCusSNInfo *m_pSaveCusSNInfo;
};



