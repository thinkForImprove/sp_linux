#pragma once
#include <assert.h>
#include "QtTypeDef.h"
#include <map>
using namespace std;
enum SC_TYPE
{
    SCT_DEVICE = 0,     //以下值必须连续加1
    SCT_SAFEDOOR,
    SCT_OUTSHUTTER,
    SCT_INSHUTTER,
    SCT_OUTPOS,
    SCT_INPOS,
    SCT_STACKER,
    SCT_TRANSPORT,
    SCT_NOTE_READER,
    SCT_CASS_STATUS,    //钞箱状态

    SCT_SIZE,           //用来确定枚举值的个数
};

//状态转换了辅助类
//主要转换CDM或CIM的设备状态、安全门状态、SHUTTER、钞口、STACKER、通道、BV、钞箱状态
//从适配层的状态转换为XFS状态
class CStatusConvertor
{
public:
    CStatusConvertor();
    virtual ~CStatusConvertor();

    //模版函数
    //转换适配层的状态为XFS状态
    //TYPE：要转换的状态的数据类型
    //eValue：要转换的状态
    //type：要转换的状态的类型
    //bCDM：是否为CDM
    template <class TYPE>
    WORD ADP2XFS(TYPE eValue, SC_TYPE type, BOOL bCDM) const
    {
        assert(type >= 0 && type < SCT_SIZE);
        const STATUS_MAP &sm = m_mapData[MIndex(bCDM)][type];
        STATUS_MAP_CONST_IT it = sm.find(eValue);
        if (it == sm.end())
        {
            it = sm.find(-1);
            assert(it != sm.end());
        }
        return (WORD)it->second;
    }

private:
    //根据是否CDM读取在内部数据的索引
    static inline int MIndex(BOOL bCDM)
    {
        return bCDM ? 0 : 1;
    }

    typedef map<int, int> STATUS_MAP;   //从ADP状态到XFS状态的映射
    typedef STATUS_MAP::iterator STATUS_MAP_IT;
    typedef STATUS_MAP::const_iterator STATUS_MAP_CONST_IT;
    STATUS_MAP m_mapData[2][SCT_SIZE];  //2 for CDM or CIM, [0]CDM, [1]CIM
};

