#ifndef CORCONFIGPARAM_H
#define CORCONFIGPARAM_H

#include "XFSCIM.H"
#include "XFSCDM.H"
#include <string>
using namespace std;
#include "INIFileReader.h"
#include "MultiString.h"
#include "ILogWrite.h"
#include "IDevCOR.h"

#define SEP_STR_COMMA           ","
#define SEP_STR_SEMICOLON       ";"
#define SEP_STR_COLON           ":"

#define SAVE_MODE_CIM_COUNT 0
#define SAVE_MODE_CDM_COUNT 1
#define SAVE_MODE_CIM_EXCHANGE 2
#define SAVE_MODE_CDM_EXCHANGE 3
#define SAVE_MODE_CIM_CASHIN 4
#define SAVE_MODE_CDM_CASHOUT 5

#define TIMING_BEFORE           "before"
#define TIMING_AFTER            "after"

#define ACTION_CASHIN           "CashIn"
#define ACTION_CASHOUT          "CashOut"

typedef struct _tag_cor_config_param : public CLogManage
{
public:

    //构造函数，清空数据
    _tag_cor_config_param()
    {
        lpNoteTypeList = NULL;
        lppCurrencyExp = NULL;
        lppMixType = NULL;
        usCoinValuesCount = 3;
        usCoinValues[0] = 100;
        usCoinValues[1] = 50;
        usCoinValues[2] = 10;

        iCoinInTimeOut = 60;
        iStartExchangeIgnoreCUNumberList = 1;
        iDiscardCheck_ulCashBox = 1;

        memset(cEmptyString, 0, sizeof(cEmptyString));
        memset(lpNoteNumberListArray, 0, sizeof(lpNoteNumberListArray));

        SetLogFile(LOGFILE, "_tag_cor_config_param", "COR");
    }

    //析构函数，释放使用过程的分配的内存
    virtual ~_tag_cor_config_param()
    {
        Clear();
    }

    LPWFSCIMNOTETYPELIST GetNoteTypes() const
    {
        return lpNoteTypeList;
    }

    void ConfigureNoteTypes(LPUSHORT lpusNoteIDs);
    LPUSHORT getNoteIDs()
    {
        return usNoteIDs;
    }

    LPWFSCDMCURRENCYEXP *GetCurrencyExp() const
    {
        return lppCurrencyExp;
    }

    LPWFSCDMMIXTYPE *GetMixTypes() const
    {
        return lppMixType;
    }

    USHORT getCoinValuesCount()
    {
        return usCoinValuesCount;
    }

    LPUSHORT getCoinValues()
    {
        return usCoinValues;
    }

    ULONG getCylinderNO(int index)
    {
        return ulCylinderNO[index];
    }

    void setInitialCount(int index, ULONG count)
    {
        ulInitialCount[index] = count;
    }

    void addInitialCount(int index, ULONG count)
    {
        ulInitialCount[index] += count;
    }

    ULONG getInitialCount(int index)
    {
        return ulInitialCount[index];
    }

    void setCashInCount(int index, ULONG count)
    {
        ulCashInCount[index] = count;
    }

    void addCashInCount(int index, ULONG count)
    {
        ulCashInCount[index] += count;
    }

    ULONG getCashInCount(int index)
    {
        return ulCashInCount[index];
    }

    void setCount(int index, ULONG count)
    {
        ulCount[index] = count;
    }

    void addCount(int index, ULONG count)
    {
        ulCount[index] += count;
    }

    void subtractCount(int index, ULONG count)
    {
        if (count < ulCount[index])
        {
            ulCount[index] -= count;
        }
        else
        {
            ulCount[index] = 0;
        }
    }

    ULONG getCount(int index)
    {
        return ulCount[index];
    }

    void setRejectCount(int index, ULONG count)
    {
        ulRejectCount[index] = count;
    }

    void addRejectCount(int index, ULONG count)
    {
        ulRejectCount[index] += count;
    }

    ULONG getRejectCount(int index)
    {
        return ulRejectCount[index];
    }

    void setMinimum(int index, ULONG count)
    {
        ulMinimum[index] = count;
    }

    ULONG getMinimum(int index)
    {
        return ulMinimum[index];
    }

    void setMaximum(int index, ULONG count)
    {
        ulMaximum[index] = count;
    }

    ULONG getMaximum(int index)
    {
        return ulMaximum[index];
    }

    void setPHMaximum(int index, ULONG count)
    {
        ulPHMaximum[index] = count;
    }

    ULONG getPHMaximum(int index)
    {
        return ulPHMaximum[index];
    }

    int32_t getCashValue(int index)
    {
        return iCashValue[index];
    }

    int32_t getCoinCode(int index)
    {
        return iCoinCode[index];
    }

    int GetCoinInTimeOut()
    {
        return iCoinInTimeOut;
    }

    int GetStartExchangeIgnoreCUNumberList()
    {
        return iStartExchangeIgnoreCUNumberList;
    }

    int GetDiscardCheck_ulCashBox()
    {
        return iDiscardCheck_ulCashBox;
    }

    LPWFSCIMNOTENUMBERLIST getNoteNumberList(int index)
    {
        return lpNoteNumberListArray[index];
    }

    string getUnitID(int index) const
    {
        return multiStrUnitID.GetAt(index);
    }

    inline void MakeNoteNumberList(char* buf, int size);

    //从配置文件装入参数
    //lpszFileName: 带路径的文件名
    int LoadParam(LPCSTR lpszFileName);

    //释放使用过程的分配的内存
    void Clear();

    void SaveCashUnitInfo(int mode);

    string getDevDllName() const
    {
        return sDevDllName;
    }

    //得到适配器DLL名
    string getPortMode() const
    {
        return sPortMode;
    }

    void LogCashUnitCount(LPCSTR action, LPCSTR timing);

private:

    //读由多个项目组成的配置项，如“1=USD,2”
    //返回值：读取的项数
    int SplitMultipleItems(LPCSTR pValue, CMultiString &ms, std::string separator = SEP_STR_COMMA);
    int ConvertMultipleNumber(LPCSTR pValue, ULONG* ulValues, USHORT usCount = 0);
    void MakeMultipleNumberValue(char* pValue, ULONG* ulValues, USHORT usCount);
    USHORT ConvertNoteRelease(LPCSTR pValue);

    //内部数据
private:

    CINIFileReader          m_configfile;
    bool                    m_bLoad;

    // Configuration items.
    std::string             sDevDllName;    // Device specific dll name.
    std::string             sPortMode;      // Open mode.
    USHORT                  usCoinValuesCount;
    USHORT                  usCoinValues[MAX_COINCYLINDER_NUM + 1];
    int32_t                 iCashValue[MAX_COINCYLINDER_NUM + 1];
    int32_t                 iCoinCode[MAX_COINCYLINDER_NUM + 1];
    CMultiString            multiStrUnitID;

    // User settings.
    int                     iCoinInTimeOut; // CoinIn wait time.

    // Customized settings.
    int                     iStartExchangeIgnoreCUNumberList;
    int                     iDiscardCheck_ulCashBox;

    // cash unit count relative info.
    ULONG                   ulCylinderNO[MAX_COINCYLINDER_NUM];
    ULONG                   ulInitialCount[MAX_COINCYLINDER_NUM + 1];
    ULONG                   ulCount[MAX_COINCYLINDER_NUM + 1];
    ULONG                   ulCashInCount[MAX_COINCYLINDER_NUM + 1];
    ULONG                   ulRejectCount[MAX_COINCYLINDER_NUM + 1];
    ULONG                   ulMinimum[MAX_COINCYLINDER_NUM + 1];        // minimum capacity for threshold.
    ULONG                   ulMaximum[MAX_COINCYLINDER_NUM + 1];        // maximum capacity for threshold.
    ULONG                   ulPHMaximum[MAX_COINCYLINDER_NUM + 1];      // Physical maximum capacity.
    LPWFSCIMNOTENUMBERLIST  lpNoteNumberListArray[MAX_COINCYLINDER_NUM + 1];    // Cash unit specific note number list.

    // BankNoteTypes
    USHORT                  usNoteIDs[MAX_SUPP_COIN_TYPE_NUM];
    LPWFSCIMNOTETYPELIST    lpNoteTypeList;

    // 设备支持的所有钞票的信息["币种-指数"]
    LPWFSCDMCURRENCYEXP     *lppCurrencyExp;
    // Mix algorithms.
    LPWFSCDMMIXTYPE         *lppMixType;

    // common variables.
    char                    cEmptyString[2];

} CORCONFIGPARAM, *LPCORCONFIGPARAM;

#endif // CORCONFIGPARAM_H
