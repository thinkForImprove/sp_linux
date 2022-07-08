#ifndef MIXMANAGER_H
#define MIXMANAGER_H

#include "IMixManager.h"

#include <set>
#include <list>
#include <vector>
#include <algorithm>
#include <functional>
#include <assert.h>
#include <string>

using namespace std;


//得到索引错误
#define ERR_GET_INDEX       (static_cast<UINT>(-1))

//钞箱索引列表类型定义
typedef set<UINT> CASS_INDEX_LIST;
typedef CASS_INDEX_LIST::iterator CASS_INDEX_LIST_IT;
typedef CASS_INDEX_LIST::const_iterator CASS_INDEX_LIST_CONST_IT;

struct CU_COMBINATION_CONTAINER;

//候选列表类，用于保存候选列表
//保存多个ULONG[uResultLen]数组
struct CANDIDATE_LIST
{
    //构造函数
    //uResultLen，指定结果数据的长度
    CANDIDATE_LIST(UINT uResultLen)
    {
        m_uResultLen = uResultLen;
    }

    //析构函数
    virtual ~CANDIDATE_LIST()
    {
        Clear();
    }

    //清除候选列表中的数据
    void Clear()
    {
        CANDIDATES_IT it;
        for (it = m_Results.begin(); it != m_Results.end(); it++)
        {
            delete [](*it);
        }
        m_Results.clear();
    }

    //得到每个结果的长度
    UINT GetResultLen() const
    {
        return m_uResultLen;
    }

    //得到候选结果的个数
    UINT GetCount() const
    {
        return m_Results.size();
    }

    //得到第uIndex个候选结果
    ULONG *GetAt(UINT uIndex) const
    {
        assert(uIndex < m_Results.size());
        return m_Results[uIndex];
    }

    //增加一个候选结果，返回选择结果的指针
    ULONG *Add()
    {
        ULONG *pTemp = new ULONG[m_uResultLen];
        m_Results.push_back(pTemp);
        return pTemp;
    }

    //得到候选列表中最小张数的索引
    UINT GetMinNumberIndex(ULONG ulMaxDispenseCount) const
    {
        ULONG ulCount = 0xFFFFFFFF;
        UINT uIndex = ERR_GET_INDEX;
        for (UINT i = 0; i < GetCount(); i++)
        {
            ULONG ulTemp = 0;
            ULONG *pResult = GetAt(i);
            for (UINT j = 0; j < GetResultLen(); j++)
            {
                ulTemp += pResult[j];
            }
            if (ulTemp < ulCount &&
                ulTemp <= ulMaxDispenseCount)
            {
                uIndex = i;
                ulCount = ulTemp;
            }
        }
        return uIndex;
    }

    //得到候选列表中最多面值数的结果的索引
    UINT GetMaxNoteNumberIndex(const CU_COMBINATION_CONTAINER &CUContainer,
                               ULONG ulMaxDispenseCount) const;

    //得到等空结果的索引
    UINT GetEqualEmptyIndex(
    const CU_COMBINATION_CONTAINER &CUContainer, ULONG ulMaxDispenseCount) const;

private:
    typedef vector<ULONG *> CANDIDATES;
    typedef CANDIDATES::iterator CANDIDATES_IT;

    UINT m_uResultLen;      //结果数据的长度
    CANDIDATES m_Results;   //结果列表
}; //CANDIDATE_LIST

//钞箱组合结构体
//相同面值的钞箱合并到一起，张数累加，索引保存在m_itIndex中
struct CU_COMBINATION
{
    //构造函数
    //ulValue：组合钞箱的面值
    CU_COMBINATION(ULONG ulValue)
    {
        m_ulValue = ulValue;
        m_ulCount = 0;
        m_ulAllocCount = 0;
        m_ulEqualEmptySafeCount = 0;
        m_itIndex = m_lstIndex.begin();
    }

    //Value
    ULONG GetValue() const
    {
        return m_ulValue;
    }

    //Count
    ULONG GetCount() const
    {
        return m_ulCount;
    }

    void SetCount(ULONG ulCount)
    {
        m_ulCount = ulCount;
    }

    //AllocCount
    //得到分配数，用于工作中保存分配数和把最后结果反馈给MixByAlgorithm
    ULONG GetAllocCount() const
    {
        return m_ulAllocCount;
    }

    void SetAllocCount(ULONG ulAllocCount)
    {
        m_ulAllocCount = ulAllocCount;
    }

    //LeftCount : Count - AllocCount
    ULONG GetLeftCount() const
    {
        assert(m_ulCount >= m_ulAllocCount);
        return m_ulCount - m_ulAllocCount;
    }

    //EqualEmptySafeCount
    ULONG GetEqualEmptySafeCount() const
    {
        return m_ulEqualEmptySafeCount;
    }

    void SetEqualEmptySafeCount(ULONG ulEqualEmptySafeCount)
    {
        m_ulEqualEmptySafeCount = ulEqualEmptySafeCount;
    }

    //EqualEmptySafeLeftCount : Count - AllocCount - EqualEmptySafeCount
    ULONG GetEqualEmptySafeLeftCount() const
    {
        assert(m_ulCount >= m_ulAllocCount);
        if (m_ulCount - m_ulAllocCount < m_ulEqualEmptySafeCount)
            return 0;
        return m_ulCount - m_ulAllocCount - m_ulEqualEmptySafeCount;
    }

    //Index
    //增加一个指向外部钞箱的索引
    void AddIndex(UINT uIndex)
    {
        m_lstIndex.insert(uIndex);
        m_itIndex = m_lstIndex.begin();
    }

    //得到第一个索引，若无，返回ERR_GET_INDEX
    UINT GetFirstIndex()
    {
        if (m_lstIndex.size() == 0)
            return ERR_GET_INDEX;
        m_itIndex = m_lstIndex.begin();
        return *m_itIndex;
    }

    //得到下一个索引，若没有下一个，返回ERR_GET_INDEX
    UINT GetNextIndex()
    {
        if (m_itIndex == m_lstIndex.end())
            return ERR_GET_INDEX;
        m_itIndex++;
        if (m_itIndex == m_lstIndex.end())
            return ERR_GET_INDEX;
        return *m_itIndex;
    }

    //以字串形式得到钞箱信息
    string GetCUInfoString()
    {
        UINT uIndex = GetFirstIndex();
        char arycBuf[1024];
        sprintf(arycBuf, "Value=%3lu, Count=%lu, Alloc=%lu, Safe=%lu, Index=%d",
                m_ulValue, m_ulCount, m_ulAllocCount, m_ulEqualEmptySafeCount, uIndex);
        while (uIndex != ERR_GET_INDEX)
        {
            char arycTemp[20];
            sprintf(arycTemp, "%d", uIndex);
            strcat(arycBuf, arycTemp);
            uIndex = GetNextIndex();
            if (uIndex != ERR_GET_INDEX)
            {
                strcat(arycBuf, ",");
            }
        }
        return string(arycBuf);
    }

    //数据成员
private:
    ULONG m_ulValue;        //面值
    ULONG m_ulCount;        //组合钞箱的张数
    ULONG m_ulAllocCount;   //分配数
    ULONG m_ulEqualEmptySafeCount;      //等空算法下的安全张数，等于“上一面值/本面值”向上取整
    CASS_INDEX_LIST m_lstIndex;         //索引列表，对应面值的所有钞箱下标（对应与MixByAlgorithm()的aryCUInfor）
    CASS_INDEX_LIST_CONST_IT m_itIndex; //索引列表的叠代器
};//CU_COMBINATION


typedef vector<CU_COMBINATION *> CU_COMBINATION_ARRAY;          //组合钞箱数组类型
typedef CU_COMBINATION_ARRAY::iterator CU_COMBINATION_IT;       //组合钞箱数组叠代器
typedef CU_COMBINATION_ARRAY::const_iterator CU_COMBINATION_CONST_IT;//组合钞箱数组叠代器

//组合钞相容器，保存所有的组合钞箱
struct CU_COMBINATION_CONTAINER
{
    //构造函数
    CU_COMBINATION_CONTAINER()
    {
    }

    //析构函数
    virtual ~CU_COMBINATION_CONTAINER()
    {
        CU_COMBINATION_IT it;
        for (it = m_CUs.begin(); it != m_CUs.end(); it++)
        {
            delete *it;
        }
        m_CUs.clear();
    }

    //得到组合钞箱的个数
    UINT GetCUCount() const
    {
        return m_CUs.size();
    }

    //得到第uIndex个组合钞箱
    CU_COMBINATION *GetCU(UINT uIndex) const
    {
        assert(uIndex < m_CUs.size());
        return m_CUs[uIndex];
    }

    //生成组合钞箱列表，只保存张数不为0的钞箱
    //aryCUInfor：配钞钞箱信息数组
    //usSize    ：配钞钞箱信息数组的长度
    void GenerateCU(const LPMIXCUINFOR aryCUInfor, USHORT usSize)
    {
        for (UINT i = 0; i < usSize; i++)
        {
            if (aryCUInfor[i].ulCount == 0) //不算张数为0的钞箱
                continue;

            //注释正面一句后部分是为了允许面值重复，等空或最大面值张数中得到的张数更合理
            CU_COMBINATION *pCU = nullptr;//FindCUByValue(aryCUInfor[i].ulValue);
            if (pCU == nullptr)
            {
                pCU = new CU_COMBINATION(aryCUInfor[i].ulValue);
                m_CUs.push_back(pCU);
            }
            pCU->SetCount(pCU->GetCount() + aryCUInfor[i].ulCount);
            pCU->AddIndex(i);
        }
    }

    //生成等空算法的各钞箱安全数
    //调用该方法前必须按面值从大到小排序
    void GenerateEqualEmptySafeCount()
    {
        m_CUs[0]->SetEqualEmptySafeCount(1);
        ULONG ulLeftAmount = m_CUs[0]->GetValue();
        ULONG ulMaxValue = m_CUs[0]->GetValue();
        for (UINT i = 1; i < m_CUs.size(); i++)
        {
            ULONG ulThisValue = m_CUs[i]->GetValue();
            assert(ulThisValue <= ulMaxValue);
            ULONG ulSafeLeftCount = (ulMaxValue + ulThisValue - 1) / ulThisValue;
            if (ulSafeLeftCount < ulLeftAmount / ulThisValue)
                ulSafeLeftCount = ulLeftAmount / ulThisValue;
            m_CUs[i]->SetEqualEmptySafeCount(ulSafeLeftCount);

            if (ulSafeLeftCount > m_CUs[i]->GetLeftCount())
                ulSafeLeftCount = m_CUs[i]->GetLeftCount();
            if (ulLeftAmount > ulSafeLeftCount * ulThisValue)
                ulLeftAmount -= ulSafeLeftCount * ulThisValue;
            else
                ulLeftAmount = 0;
        }
    }

    //得到所有钞箱的金额的总和
    ULONG GetTotalAmounts() const
    {
        ULONG ulTotalAmounts = 0;
        CU_COMBINATION_CONST_IT it;
        for (it = m_CUs.begin(); it != m_CUs.end(); it++)
        {
            CU_COMBINATION *pCU = *it;
            ulTotalAmounts += pCU->GetCount() * pCU->GetValue();
        }
        return ulTotalAmounts;
    }

    //得到所有钞箱的分配数的总和
    ULONG GetTotalAllocCounts() const
    {
        ULONG ulTotalAllocCount = 0;
        CU_COMBINATION_CONST_IT it;
        for (it = m_CUs.begin(); it != m_CUs.end(); it++)
        {
            CU_COMBINATION *pCU = *it;
            ulTotalAllocCount += pCU->GetAllocCount();
        }
        return ulTotalAllocCount;
    }

    //得到所有钞箱的面值的总和
    ULONG GetTotalValues() const
    {
        ULONG ulTotalValues = 0;
        CU_COMBINATION_CONST_IT it;
        for (it = m_CUs.begin(); it != m_CUs.end(); it++)
        {
            CU_COMBINATION *pCU = *it;
            ulTotalValues += pCU->GetValue();
        }
        return ulTotalValues;
    }

    //得到索引列表中钞箱的面值的总和
    ULONG GetTotalValues(const CASS_INDEX_LIST &lstIndex) const
    {
        ULONG ulTotalValues = 0;
        CASS_INDEX_LIST_CONST_IT it;
        for (it = lstIndex.begin(); it != lstIndex.end(); it++)
        {
            ulTotalValues += m_CUs[*it]->GetValue();
        }

        return ulTotalValues;
    }

    //计算最小新分配张数(不包括以前分配的和预分配数)
    //调用前必须按面值降序排序
    //lstPreAlloc: 已预分配的列表
    //ulPreAllocCount：预分配的张数
    ULONG ComputeMinNewAllocCount(ULONG ulAmount, const CASS_INDEX_LIST &lstPreAlloc, ULONG ulPreAllocCount) const
    {
        ULONG ulCount = 0;
        CU_COMBINATION *pCU = nullptr;
        for (USHORT i = 0; i < m_CUs.size(); i++)
        {
            pCU = m_CUs[i];
            ULONG ulTemp = ulAmount / pCU->GetValue();
            if (lstPreAlloc.find(i) != lstPreAlloc.end())
            {
                assert(ulPreAllocCount <= pCU->GetEqualEmptySafeLeftCount());
                if (ulTemp > pCU->GetEqualEmptySafeLeftCount() - ulPreAllocCount)
                    ulTemp = pCU->GetEqualEmptySafeLeftCount() - ulPreAllocCount;
            }
            else
            {
                if (ulTemp > pCU->GetEqualEmptySafeLeftCount())
                    ulTemp = pCU->GetEqualEmptySafeLeftCount();
            }
            ulCount += ulTemp;
            ulAmount -= ulTemp * pCU->GetValue();
        }
        assert(pCU != nullptr);
        if (ulAmount > 0)
        {
            ulCount += (ulAmount + pCU->GetValue() - 1) / pCU->GetValue();
        }

        return ulCount;
    }

    //以字串形式得到各钞箱信息
    string GetCUInfoStrings() const
    {
        string s;
        for (UINT i = 0; i < m_CUs.size(); i++)
        {
            CU_COMBINATION *pCU = m_CUs[i];
            char arycBuf[20];
            sprintf(arycBuf, " [%d]: ", i + 1);
            s += string(arycBuf) + pCU->GetCUInfoString();
        }
        return s;
    }

    //清空各钞箱的分配数
    void ClearAllocCount()
    {
        CU_COMBINATION_CONST_IT it;
        for (it = m_CUs.begin(); it != m_CUs.end(); it++)
        {
            CU_COMBINATION *pCU = *it;
            pCU->SetAllocCount(0);
        }
    }

    //从候选列表的第uIndex位置中拷贝分配数
    //CandidateList：候选列表
    //uIndex：要拷贝的元素在候选列表中的索引
    void CopyAllocCountFromCandidateList(const CANDIDATE_LIST &CandidateList, UINT uIndex)
    {
        assert(CandidateList.GetResultLen() == GetCUCount());
        ULONG *pResult = CandidateList.GetAt(uIndex);
        for (UINT i = 0; i < CandidateList.GetResultLen(); i++)
        {
            CU_COMBINATION *pCU = GetCU(i);
            pCU->SetAllocCount(pResult[i]);
        }
    }

    //把钞箱的分配数作为结果增加到候选列表中
    //CandidateList: 要增加到的候选列表
    void AddAllocCountsToCandidateList(CANDIDATE_LIST &CandidateList) const
    {
        assert(CandidateList.GetResultLen() == GetCUCount());
        ULONG *pResult = CandidateList.Add();
        for (UINT i = 0; i < CandidateList.GetResultLen(); i++)
        {
            CU_COMBINATION *pCU = GetCU(i);
            pResult[i] = pCU->GetAllocCount();
        }
    }

    //拷贝分配数到结果数组中
    //pulResult：要拷贝到的结果数组，长度由uSize指定
    //aryCUInfor：配钞钞箱信息数组，长度由uSize指定
    //uSize：结果数组的长度，必须与GenerateCU传入的数组长度相同
    void CopyAllocCountsToResultArray(ULONG *pulResult, const LPMIXCUINFOR aryCUInfor, UINT uSize) const
    {
        //清结果
        for (UINT i = 0; i < uSize; i++)
        {
            pulResult[i] = 0;
        }

        CU_COMBINATION_CONST_IT it;
        for (it = m_CUs.begin(); it != m_CUs.end(); it++)
        {
            CU_COMBINATION *pCU = *it;
            ULONG ulCount = pCU->GetAllocCount();
            if (ulCount == 0)
                continue;

            UINT uIndex = pCU->GetFirstIndex();
            while (uIndex != ERR_GET_INDEX)
            {
                assert(uIndex < uSize);
                ULONG ulTemp = ulCount;
                if (ulTemp > aryCUInfor[uIndex].ulCount)
                    ulTemp = aryCUInfor[uIndex].ulCount;
                ulCount -= ulTemp;
                pulResult[uIndex] = ulTemp;
                uIndex = pCU->GetNextIndex();
            }
            assert(ulCount == 0);
        }
    }

    //按面值从大到小排序
    void SortByValueDesc()
    {
        sort(m_CUs.begin(), m_CUs.end(), ComparerByValueDesc());
    }

    //按张数从大到小排序
    void SortByCountDesc()
    {
        sort(m_CUs.begin(), m_CUs.end(), ComparerByCountDesc());
    }


    //内部成员函数
private:
    //删除ulValue指定的面值的组合钞箱（未用）
    void RemoveCUByValue(ULONG ulValue)
    {
        UINT uIndex;
        CU_COMBINATION *pCU = FindCUByValue(ulValue, &uIndex);
        if (pCU == nullptr)
            return;
        m_CUs.erase(m_CUs.begin() + uIndex);
        delete pCU;
    }

    //查找面值为ulValue的钞箱，返回它的指针和索引(通过pulIndex)
    //pulIndex: 如果不为NULL，返回对象的索引
    //返回值：NULL 失败；否则返回找到的对象
    CU_COMBINATION *FindCUByValue(ULONG ulValue, UINT *pulIndex = nullptr) const
    {
        for (UINT i = 0; i < m_CUs.size(); i++)
        {
            if (m_CUs[i]->GetValue() == ulValue)
            {
                if (pulIndex != nullptr)
                {
                    *pulIndex = i;
                }
                return m_CUs[i];
            }
        }
        return nullptr;
    }

    //删除张数为0的钞箱
    void RemoveEmptyCU()
    {
        CU_COMBINATION_IT it;
        for (it = m_CUs.begin(); it != m_CUs.end();)
        {
            if ((*it)->GetCount() == 0)
            {
                delete *it;
                m_CUs.erase(it);
            }
            else
            {
                it++;
            }
        }
    }


    //排序类
private:


    //面值从大到小排序比较类
    struct ComparerByValueDesc : binary_function<CU_COMBINATION *, CU_COMBINATION *, bool>
    {
        bool operator()(CU_COMBINATION *_X,  CU_COMBINATION *_Y) const
        {
            return (_X->GetValue() > _Y->GetValue());
        }
    };

    //张数从大到小排序比较类
    struct ComparerByCountDesc : binary_function<CU_COMBINATION *, CU_COMBINATION *, bool>
    {
        bool operator()(const CU_COMBINATION *_X, const CU_COMBINATION *_Y) const
        {
            return (_X->GetCount() > _Y->GetCount());
        }
    };

    //私有数据成员
private:
    CU_COMBINATION_ARRAY m_CUs;     //组合钞箱列表
}; //CU_COMBINATION_CONTAINER


//配钞算法主类，实现IMixManager接口
class CMixMng : public IMixManager, public CLogManage
{
public:
    //构造函数
    CMixMng();

    //析构函数
    virtual ~CMixMng();

    //实现IMixManager方法
    //释放本对象
    virtual void Release()
    {
        delete this;
    }

    //功能：配钞 (实现接口文件定义的纯虚函数)
    // aryulResult  :  [out],  配钞结果，分别对应钞箱1 ~ usSize出钞的张数
    // aryCUInfor   :  [in]   ,  钞箱信息结构数组首地址，分别对应1 ~ usSize个钞箱的信息
    // usSize       :  [in]   ,  钞箱信息结构个数（aryCUInfor 、aryulResult数组元素个数）
    // ulAmount     :  [in]   ,  配钞金额
    // alg          :  [in]   ,  配钞方式
    //ulMaxDispenseCount : [in], 最大出钞张数
    //返回值        :  成功 则返回0；其他为失败
    virtual HRESULT MixByAlgorithm(
    ULONG *aryulResult,
    const LPMIXCUINFOR aryCUInfor,
    USHORT usSize,
    ULONG ulAmount,
    MIXALGORITHM alg,
    ULONG ulMaxDispenseCount);

    //内部成员函数
private:
    //按等空方式预分配金额到组合钞箱
    //首先生成各个钞箱的等空安全张数
    //再循环进行以下操作
    //  1. 查找最大安全剩余张数的钞箱索引，存放在列表中，同时查找到第二多的钞箱的张数
    //  2. 如果所有钞箱一样多，以最大剩余数进行分配; 否则，分配比第二多张数多出的部分
    //      如果剩余金额未分配完，则保证剩余金额不小于面值总和。
    //  3. 如果本次分配数为0张或剩余金额为0，则退出循环
    //ulAmount：要分配的金额
    //CUContainer：组合钞箱容器
    //ulMaxDispenseCount : [in], 最大出钞张数
    //返回值：返回未分配完的剩余金额
    ULONG PreAllocByEqualEmpty(ULONG ulAmount,
                               CU_COMBINATION_CONTAINER &CUContainer,
                               ULONG ulMaxDispenseCount);

    //按最多面值种类配钞
    //ulAmount：要分配的金额
    //CUContainer：组合钞箱容器
    //ulMaxDispenseCount : [in], 最大出钞张数
    HRESULT MixByMaxNoteNumber(ULONG ulAmount,
                               CU_COMBINATION_CONTAINER &CUContainer,
                               ULONG ulMaxDispenseCount);

    //按等空方式进行配钞
    //ulAmount：要分配的金额
    //CUContainer：组合钞箱容器
    //ulMaxDispenseCount : [in], 最大出钞张数
    HRESULT MixByEqualEmpty(ULONG ulAmount,
                            CU_COMBINATION_CONTAINER &CUContainer,
                            ULONG ulMaxDispenseCount);

    //按最小张数进行配钞
    //ulAmount：要分配的金额
    //CUContainer：组合钞箱容器
    //ulMaxDispenseCount : [in], 最大出钞张数
    HRESULT MixByMinNumber(ULONG ulAmount,
                           CU_COMBINATION_CONTAINER &CUContainer,
                           ULONG ulMaxDispenseCount);

    //按金额ulAmount尝试分配到从uStartIndex开始的后续钞箱
    //如金额分配完成，结果保存到候选列表CandidateList中
    //有两种模式，如bDecToZero为FALSE，做两种尝试：1）前面钞箱分配最多；2）前面钞箱分配“最多-1”张
    //          否则，把本钞箱的分配张数从最大可能分配数递减到0
    //该方法内部递归调用自身进行后续钞箱的尝试
    //ulAmount：要分配的金额
    //CUContainer：组合钞箱容器
    //uStartIndex：开始分配的钞箱索引
    //CandidateList：候选列表
    //bDecToZero：是否尝试把本钞箱的分配张数减到0，如果为FALSE，只尝试最大张数和最大张数-1
    void TryToMix(ULONG ulAmount,
                  const CU_COMBINATION_CONTAINER &CUContainer,
                  UINT uStartIndex, CANDIDATE_LIST &CandidateList,
                  BOOL bDecToZero = FALSE);

    //查找最大安全剩余张数的钞箱索引，并返回它的索引列表及第二多剩余张数
    //返回值：返回最大安全剩余张数的钞箱的总面值
    ULONG FindMaxLeftNumberIndex(const CU_COMBINATION_CONTAINER &CUContainer, CASS_INDEX_LIST &lstIndex,
                                 ULONG &ulSecondMaxLeftCount, ULONG ulValueMax);        //30-00-00-00(FS#0007)

};


#endif // MIXMANAGER_H
