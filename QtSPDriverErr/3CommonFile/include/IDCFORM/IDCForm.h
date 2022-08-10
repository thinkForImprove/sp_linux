#pragma once

#include "ILogWrite.h"
#include "IDSDec.h"
#include "MultiString.h"
#include "ReadString.h"
#include "ProUtil.h"
#include "XfsHelper.h"
#include "XfsRegValue.h"

//注册表信息
#define FORMFILEVALUENAME           "IDCForm"               //Form值名
#define FORMFILEDEFAULT             "CardReaderForm.idc"    //FORM缺省值

#ifdef Q_OS_WIN32
#define FORMPATH                    "C:/CFES/FORM"
#else
#define FORMPATH                    "/usr/local/CFES/DATA/FORM/IDC"
#endif

#define ZeroMemory(a, l)    memset(a, 0, l)

#define END_SEP_INDEX               100000                  //ENDTRACK的分隔符索引

#define FORMFILEVALUENAME           "IDCForm"               //Form值名
#define FORMFILEDEFAULT             "CardReaderForm.idc"    //FORM缺省值

void LogWrite(const char *ThisModule, int Err, int fmt, ...);

//---------------------- 类型定义 ------------------------
struct SP_IDC_FORM_FIELD    //读卡器FORM中的一个字段定义
{
    string  FieldName;          //字段名
    long    FieldSepIndices[2]; //字段的开始和结束位置的分隔符索引，0为开始，1为结束
    long    nOffsets[2];        //字段的开始和结束位置的偏移量

    SP_IDC_FORM_FIELD()
    {
        FieldSepIndices[0]  = FieldSepIndices[1]    = -1;   //小于0没有初始化
        nOffsets[0]         = nOffsets[1]           = 0;
    }

    bool Load(const char *pName, const char *pValue);

    ~SP_IDC_FORM_FIELD()
    {
    }
};

typedef vector<SP_IDC_FORM_FIELD *> FFL;        //字段列表
typedef FFL::iterator FFLIT;

struct SP_IDC_FORM          //读卡器FORM定义
{
    BOOL            bLoadSucc;                  //指示是否装入成功
    string          FormName;                   //FORM名
    char            cFieldSeparatorTracks[3];   //三个磁道的分隔符
    WORD            fwAction;                   //动作：READ或WRITE
    string          sTracks;                    //磁道算法
    char            szTracks[100];              //压缩了的磁道算法
    BOOL            bSecures[3];                //是否进行安全校验
    string          sDefault;                   //保留值
    CMultiString    szTrackFields[3];           //每个磁道的字段字串
    FFL             TracksFields[3];            //每个磁道的字段列表
    FFL             FieldList;                  //所有字段组成的列表

    SP_IDC_FORM()
    {
        bLoadSucc               = false;
        ZeroMemory(cFieldSeparatorTracks, sizeof(cFieldSeparatorTracks));
        fwAction                = 0;
        ZeroMemory(bSecures, sizeof(bSecures));
        ZeroMemory(szTracks, sizeof(szTracks));
    }

    ~SP_IDC_FORM()
    {
        FFLIT itField;
        for (itField = FieldList.begin(); itField != FieldList.end(); itField++)
        {
            delete (*itField);
        }
        FieldList.clear();
    }

    bool Load(const char *pFormName, const char *pBuf);
};

typedef vector<SP_IDC_FORM *>   FL;         //读卡器FORM列表
typedef FL::iterator            FLIT;

class CIDCForm
{
public:
    CIDCForm();
    ~CIDCForm();
public:
    HRESULT CheackFormInvalid(FL &FormList, const char *pFormName, DWORD dwAction);
    HRESULT SPGetIDCForm(FL &FormList, const char *pFormName, SP_IDC_FORM *&pForm, DWORD dwAction);
    //HRESULT SPParseData(SP_IDC_DATA *pData, SP_IDC_FORM *pForm);
    //void SPFireInvalidTrackData(SP_IDC_DATA *pData, WORD wStatus, LPSTR pTrackName, LPSTR pTrackData);
    DWORD TracksToDataSourceOption(const char *szTracks);
    void GenSepList(const char *pTrack, int nLen, char cSep, vector<int> &SepList);
    bool ComputeFieldInfo(LPBYTE lpByte, int nLen, long *Indices, long *nOffsets, vector<int> &SepList, long &nFieldOffset, long &nFieldLen);
    bool LoadFormFile(LPCSTR SPName, FL &FormList);
    void ClearFormList(FL &FormList);
    BOOL ComputeTracks(const char **pp, int *pValue, BOOL &bValue);
    BOOL IsDoSecureCheck(SP_IDC_FORM *pForm, DWORD dwOption);
    string GetFormFileName(LPCSTR SPName);
    void AddFieldALL(SP_IDC_FORM *pForm);
    BOOL FillTracksFields(SP_IDC_FORM *pForm);
    //void LogWrite(const char *ThisModule, int Err, int fmt, ...);
    SP_IDC_FORM *FLFind(FL &FormList, LPCSTR szFormName);
    DWORD TrackIndexToDataSourceOption(int iTrack);
    bool NeedTrack(const char *szTracks, int iTrack);
    bool WriteFieldData(LPWFSIDCCARDDATA pCardData, SP_IDC_FORM_FIELD *pField, vector<int> &SepList,
                        BOOL bDefault, char cFieldSeparator, CMultiString &InputData);
    LPCSTR SPFindFieldName(const CMultiString &FieldData, LPCSTR FieldName);
protected:
    CXfsRegValue        m_cXfsReg;
    std::string         m_strLogicalName;
    std::string         m_strSPName;
    time_t              m_FormFileLastChangeTime;   //FORM文件的最后修改时间
    CMultiString        m_FormNames;                //所有FORM名组成的多字串
    WFSIDCFORM          m_LastForm;                 //最后返回的FORM
    FL                  m_FormList;
};
