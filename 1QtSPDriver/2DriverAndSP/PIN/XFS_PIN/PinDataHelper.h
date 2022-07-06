#pragma once
#include "ISPBasePIN.h"
#include <string>
#include <vector>
#include <list>
#include <map>
#include <mutex>
#include <fstream>
#include <algorithm>
using namespace std;
//////////////////////////////////////////////////////////////////////////
typedef vector<WFSPINKEY>               vectorWFSPINKEY;
//////////////////////////////////////////////////////////////////////////
#define KEY_MAX_NUM             144// 120普通密钥，24国密密钥
#define KEY_MAX_NUM_XZ          128// 120普通密钥，24国密密钥
class CWFSPINKEYDETAIL
{
public:
    typedef std::map<std::string, LPWFSPINKEYDETAIL> mapLPWFSPINKEYDETAIL;
public:
    CWFSPINKEYDETAIL();
    ~CWFSPINKEYDETAIL();
public:
    void Clear();
    void Add(LPCSTR lpsKeyName, WORD fwUse, BOOL bLoaded = TRUE);
    LPWFSPINKEYDETAIL *GetData();
private:
    LPWFSPINKEYDETAIL      *m_ppKeys;
    mapLPWFSPINKEYDETAIL    m_mapKeys;
};

class CWFSPINKEYDETAILEX
{
public:
    typedef std::map<std::string, LPWFSPINKEYDETAILEX> mapLPWFSPINKEYDETAILEX;
public:
    CWFSPINKEYDETAILEX();
    ~CWFSPINKEYDETAILEX();
public:
    void Clear();
    void Add(LPCSTR lpsKeyName, DWORD fwUse, BYTE byVersion, BOOL bLoaded = TRUE);
    LPWFSPINKEYDETAILEX *GetData();
private:
    LPWFSPINKEYDETAILEX        *m_ppKeys;
    mapLPWFSPINKEYDETAILEX      m_mapKeys;
};
//////////////////////////////////////////////////////////////////////////
class CWFSXDATA
{
public:
    CWFSXDATA();
    ~CWFSXDATA();
public:
    void Clear();
    void Add(const LPBYTE lpbData, USHORT usLength);
    LPWFSXDATA GetData();
private:
    WFSXDATA  m_stData;
};
//////////////////////////////////////////////////////////////////////////
//30-00-00-00(FS#0003)  start
class CWFSENCIODATA
{
public:
    CWFSENCIODATA();
    ~CWFSENCIODATA();
public:
    void Clear();
    void Add(const LPBYTE lpbData, USHORT usLength, WORD wProtocol);
    LPWFSPINENCIO GetData();
private:
    WFSPINENCIO  m_stENCIOData;
};

class CWFSGENERATESM2KEYPAIRDATA
{
public:
    CWFSGENERATESM2KEYPAIRDATA();
    ~CWFSGENERATESM2KEYPAIRDATA();
public:
    void Clear();
    void Add(WORD wCommand, WORD wResult);
    LPPROTCHNGENERATESM2KEYPAIROUT GetData();
private:
    PROTCHNGENERATESM2KEYPAIROUT  m_stGenerateSm2KeyPairData;
};

class CWFSEXPORTSM2EPPSIGNITEMDATA
{
public:
    CWFSEXPORTSM2EPPSIGNITEMDATA();
    ~CWFSEXPORTSM2EPPSIGNITEMDATA();
public:
    void Clear();
    void Add(const LPBYTE lpbData, USHORT usLength, WORD wCmd, WORD wResult);
    LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT GetData();
private:
    PROTCHNEXPORTSM2EPPSIGNEDITEMOUT  m_stExportSm2EppSignItemData;
};

class CWFSIMPORTSM4DATA
{
public:
    CWFSIMPORTSM4DATA();
    ~CWFSIMPORTSM4DATA();
public:
    void Clear();
    void Add(const LPBYTE lpbData, USHORT usLength, WORD wCmd, WORD wResult);
    LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT GetData();
private:
    PROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT  m_stImportSm4KeyData;
};

//30-00-00-00(FS#0003)  end
//////////////////////////////////////////////////////////////////////////
#define FDK_MAX_NUM             8 // 最多8个则键
class CWFSPINFUNCKEYDETAIL
{
public:
    CWFSPINFUNCKEYDETAIL();
    ~CWFSPINFUNCKEYDETAIL();
public:
    void Clear();
    void SetMask(ULONG ulFuncMask);
    void AddFDK(const WFSPINFDK &stFDK);
    LPWFSPINFUNCKEYDETAIL GetData();
private:
    WFSPINFUNCKEYDETAIL m_stKeys;
    WFSPINFDK           m_stFDKs[FDK_MAX_NUM];
    LPWFSPINFDK         m_pszFDK[FDK_MAX_NUM];
    USHORT              m_usNumberFDKs;
};

//////////////////////////////////////////////////////////////////////////
#define KEY_MAX_INPUT               2048// 明文最大输入按键
class CWFSPINDATA
{
public:
    CWFSPINDATA();
    ~CWFSPINDATA();
public:
    void Clear();
    void Add(const WFSPINKEY &stKey);
    void Add(const vectorWFSPINKEY &vtKeys);
    LPWFSPINDATA GetData(WORD wCompletion);
private:
    WFSPINDATA          m_stPinData;
    WFSPINKEY           m_stPinKey[KEY_MAX_INPUT];
    LPWFSPINKEY         m_pszPinKey[KEY_MAX_INPUT];
    USHORT              m_usKeys;
};

//////////////////////////////////////////////////////////////////////////
class CInputDataHelper
{
public:
    CInputDataHelper(const LPWFSPINGETDATA lpPinGetData);
    CInputDataHelper(const LPWFSPINGETPIN lpGetPin);
    ~CInputDataHelper();
public:
    bool IsSupportKey(ULONG ulAllFuncKey, ULONG ulAllFDK);
    bool IsSupportGetPinKey(ULONG ulAllFuncKey);
    bool IsHasActiveKey();
    bool IsActiveKey(ULONG ulFuncKey);
    bool IsActiveFDK(ULONG ulFDK);
    bool IsTermKey(ULONG ulFuncKey);
    bool IsTermFDK(ULONG ulFDK);
    bool IsTermKeyActived();                //30-00-00-00(FT#0021)
    USHORT MinLen();
    USHORT MaxLen();
    BOOL IsAutoEnd();
    BOOL IsGetPin();
    // 保存最后结束按键
    void SetEndKey(WFSPINKEY stKey);
    WFSPINKEY GetEndKey();
    // 设置不检测侧键
    void SetNotCheckFDK(bool bNotCheck);
    // 获取支持的按键值
    ULONG GetActiveKey();
    ULONG GetActiveFDK();

    char GetEchoChar();
private:
    ULONG               m_ulActiveFDKs;
    ULONG               m_ulActiveKeys;
    ULONG               m_ulTerminateFDKs;
    ULONG               m_ulTerminateKeys;
    USHORT              m_usMinLen;
    USHORT              m_usMaxLen;
    BOOL                m_bAutoEnd;
    BOOL                m_bGetPin;
    WFSPINKEY           m_stKey;
    bool                m_bNotCheckFDK;
    char                m_cEcho;
};
//////////////////////////////////////////////////////////////////////////
//30-00-00-00(FS#0005) add start
//远程密钥下载相关命令输出数据
class CWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT
{
public:
    CWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT(){
        memset(&m_stExportRsaItem, 0, sizeof(m_stExportRsaItem));
        memset(&m_stValue, 0, sizeof(m_stValue));
        memset(&m_stSignature, 0, sizeof(m_stSignature));
    }
    ~CWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT(){
        //内存释放
        Clear();
    }

    void Clear(){
        if(m_stValue.lpbData != nullptr){
            delete m_stValue.lpbData;
        }
        if(m_stSignature.lpbData != nullptr){
            delete m_stSignature.lpbData;
        }

        memset(&m_stExportRsaItem, 0, sizeof(m_stExportRsaItem));
        memset(&m_stValue, 0, sizeof(m_stValue));
        memset(&m_stSignature, 0, sizeof(m_stSignature));
    }

    void AddBaseData(DWORD dwRSASignatureAlgorithm)
    {
        m_stExportRsaItem.dwRSASignatureAlgorithm = dwRSASignatureAlgorithm;
    }
    void AddValueData(const LPBYTE lpbData, USHORT usLength)
    {
        m_stExportRsaItem.lpxValue = &m_stValue;
        if(lpbData != nullptr && usLength != 0){
            m_stValue.lpbData = new BYTE[usLength];
            if(m_stValue.lpbData){
                m_stValue.usLength = usLength;
                memcpy(m_stValue.lpbData, lpbData, usLength);
            }
        }
    }
    void AddSignatureData(const LPBYTE lpbData, USHORT usLength)
    {
        m_stExportRsaItem.lpxSignature = &m_stSignature;
        if(lpbData != nullptr && usLength != 0){
            m_stSignature.lpbData = new BYTE[usLength];
            if(m_stSignature.lpbData){
                m_stSignature.usLength = usLength;
                memcpy(m_stSignature.lpbData, lpbData, usLength);
            }
        }
    }
    LPWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT GetData()
    {
        return &m_stExportRsaItem;
    }
private:
    WFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT m_stExportRsaItem;
    WFSXDATA m_stValue;
    WFSXDATA m_stSignature;
};

class CWFSPINIMPORTRSAPUBLICKEYOUTPUT
{
public:
    CWFSPINIMPORTRSAPUBLICKEYOUTPUT()
    {
        memset(&m_stImportPK, 0, sizeof(m_stImportPK));
        memset(&m_stCheckKeyValue, 0, sizeof(m_stCheckKeyValue));
    }
    ~CWFSPINIMPORTRSAPUBLICKEYOUTPUT()
    {
        Clear();
    }

    void Clear()
    {
        if(m_stCheckKeyValue.lpbData != nullptr){
            delete m_stCheckKeyValue.lpbData;
        }

        memset(&m_stImportPK, 0, sizeof(m_stImportPK));
        memset(&m_stCheckKeyValue, 0, sizeof(m_stCheckKeyValue));
    }
    void AddBaseData(DWORD dwRSAKeyCheckMode)
    {
        m_stImportPK.dwRSAKeyCheckMode = dwRSAKeyCheckMode;
    }
    void Add(const LPBYTE lpbData, USHORT usLength)
    {
        m_stImportPK.lpxKeyCheckValue = &m_stCheckKeyValue;
        if(lpbData != nullptr && usLength != 0){
            m_stCheckKeyValue.lpbData = new BYTE[usLength];
            if(m_stCheckKeyValue.lpbData){
                m_stCheckKeyValue.usLength = usLength;
                memcpy(m_stCheckKeyValue.lpbData, lpbData, usLength);
            }
        }
    }
    LPWFSPINIMPORTRSAPUBLICKEYOUTPUT GetData()
    {
        return &m_stImportPK;
    }
private:
    WFSPINIMPORTRSAPUBLICKEYOUTPUT m_stImportPK;
    WFSXDATA m_stCheckKeyValue;
};

class CWFSPINIMPORTRSASIGNEDDESKEYOUTPUT
{
public:
    CWFSPINIMPORTRSASIGNEDDESKEYOUTPUT()
    {
        memset(&m_stImportDesKey, 0, sizeof(m_stImportDesKey));
        memset(&m_stCheckKeyValue, 0, sizeof(m_stCheckKeyValue));
    }
    ~CWFSPINIMPORTRSASIGNEDDESKEYOUTPUT()
    {
        Clear();
    }

    void Clear()
    {
        if(m_stCheckKeyValue.lpbData != nullptr){
            delete m_stCheckKeyValue.lpbData;
        }

        memset(&m_stImportDesKey, 0, sizeof(m_stImportDesKey));
        memset(&m_stCheckKeyValue, 0, sizeof(m_stCheckKeyValue));
    }
    void AddBaseData(WORD wKeyLength, WORD wKeyCheckMode)
    {
        m_stImportDesKey.wKeyLength = wKeyLength;
        m_stImportDesKey.wKeyCheckMode = wKeyCheckMode;
    }
    void Add(const LPBYTE lpbData, USHORT usLength)
    {
        m_stImportDesKey.lpxKeyCheckValue = &m_stCheckKeyValue;
        if(lpbData != nullptr && usLength != 0){
            m_stCheckKeyValue.lpbData = new BYTE[usLength];
            if(m_stCheckKeyValue.lpbData){
                m_stCheckKeyValue.usLength = usLength;
                memcpy(m_stCheckKeyValue.lpbData, lpbData, usLength);
            }
        }
    }
    LPWFSPINIMPORTRSASIGNEDDESKEYOUTPUT GetData()
    {
        return &m_stImportDesKey;
    }
private:
    WFSPINIMPORTRSASIGNEDDESKEYOUTPUT m_stImportDesKey;
    WFSXDATA m_stCheckKeyValue;
};
//30-00-00-00(FS#0005) add end
