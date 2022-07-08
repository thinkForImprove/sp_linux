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
