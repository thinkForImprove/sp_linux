//#include <log_lib.h>
#include "MixMng.h"

//日志相关的宏定义
#define THISFILE    "MixManager"
#define DEFMOD(f)   const char *ThisModule=#f

static const char *ThisFile  = "MixMng";
//------------------ CANDIDATE_LIST -----------------------

//得到最优等穿结果的索引
UINT CANDIDATE_LIST::GetEqualEmptyIndex(
const CU_COMBINATION_CONTAINER &CUContainer,
ULONG ulMaxDispenseCount) const
{
    double fMinVariance = 10e10;        //最小方差
    UINT uIndex = static_cast<UINT>(-1);                    //最小偏移量的候选结果索引

    for (UINT i = 0; i < GetCount(); i++)
    {
        //计算最大最小值
        ULONG *pResult = GetAt(i);
        ULONG ulCount = 0;

        //计算平均值
        double fAverage = 0.0;
        for (UINT j = 0; j < GetResultLen(); j++)
        {
            CU_COMBINATION *pCU = CUContainer.GetCU(j);
            ULONG ulTemp = pCU->GetCount() - pResult[j];
            fAverage += ulTemp;
        }
        fAverage /= (double)GetResultLen();

        //计算方差
        double fVariance = 0.0;
        for (UINT j = 0; j < GetResultLen(); j++)
        {
            CU_COMBINATION *pCU = CUContainer.GetCU(j);
            ULONG ulTemp = pCU->GetCount() - pResult[j];
            fVariance += ((double)ulTemp - fAverage) * ((double)ulTemp - fAverage);
            ulCount += pResult[j];
        }

        //比较偏移量，并保存最小偏移量和索引
        if (fVariance < fMinVariance &&
            ulCount <= ulMaxDispenseCount)
        {
            fMinVariance = fVariance;
            uIndex = i;
        }
    }

    return uIndex;
}

//得到候选列表中最多面值数的结果的索引
UINT CANDIDATE_LIST::GetMaxNoteNumberIndex(const CU_COMBINATION_CONTAINER &CUContainer,
                                           ULONG ulMaxDispenseCount) const
{
    ULONG ulNoteNumber = 0;     //面值数
    ULONG ulNoteTypeNumber = 0; //不同面值的类型数
    UINT uIndex = ERR_GET_INDEX;
    for (UINT i = 0; i < GetCount(); i++)
    {
        ULONG ulLastValue = static_cast<ULONG>(-1);
        ULONG ulTemp = 0;
        ULONG ulTempNoteTypeNumber = 0;
        ULONG *pResult = GetAt(i);
        ULONG ulCount = 0;
        for (UINT j = 0; j < GetResultLen(); j++)
        {
            if (pResult[j] > 0)
            {
                ulTemp++;
                CU_COMBINATION *pCU = CUContainer.GetCU(j);
                if (ulLastValue != pCU->GetValue())
                {
                    ulLastValue = pCU->GetValue();
                    ulTempNoteTypeNumber++;
                }
            }
            ulCount += pResult[j];
        }
        if (ulTemp > ulNoteNumber &&
            ulTempNoteTypeNumber > ulNoteTypeNumber &&
            ulCount <= ulMaxDispenseCount)
        {
            uIndex = i;
            ulNoteNumber = ulTemp;
            ulNoteTypeNumber = ulTempNoteTypeNumber;
        }
    }
    return uIndex;
}

//-------------- CMixMng --------------------
CMixMng::CMixMng()
{
    SetLogFile(LOGFILE, ThisFile, "BRM");
}

CMixMng::~CMixMng()
{
}

//按最小张数进行配钞
//ulAmount：要分配的金额
//CUContainer：组合钞箱容器
HRESULT CMixMng::MixByMinNumber(ULONG ulAmount,
                                CU_COMBINATION_CONTAINER &CUContainer,
                                ULONG ulMaxDispenseCount)
{
    CUContainer.SortByValueDesc();  //按面值降序排序组合钞箱

    CANDIDATE_LIST CandidateList(CUContainer.GetCUCount());
    TryToMix(ulAmount, CUContainer, 0, CandidateList);

    if (CandidateList.GetCount() == 0)
        return ERR_MIX_ALGORITHM;

    //查找最优数据的索引
    UINT uIndex = CandidateList.GetMinNumberIndex(ulMaxDispenseCount);
    if (uIndex == ERR_GET_INDEX)
        return ERR_MIX_MORE_ITEM;

    //把最优数据拷贝回组合钞箱中
    CUContainer.CopyAllocCountFromCandidateList(CandidateList, uIndex);

    return 0;
}

//按最多面值种类配钞
HRESULT CMixMng::MixByMaxNoteNumber(ULONG ulAmount,
                                    CU_COMBINATION_CONTAINER &CUContainer,
                                    ULONG ulMaxDispenseCount)
{
    CUContainer.SortByValueDesc();  //按面值降序排序组合钞箱

    CANDIDATE_LIST CandidateList(CUContainer.GetCUCount());
    TryToMix(ulAmount, CUContainer, 0, CandidateList);

    if (CandidateList.GetCount() == 0)
        return ERR_MIX_ALGORITHM;

    //查找最优数据的索引
    UINT uIndex = CandidateList.GetMaxNoteNumberIndex(CUContainer, ulMaxDispenseCount);
    if (uIndex == ERR_GET_INDEX)
        return ERR_MIX_MORE_ITEM;

    //把最优数据拷贝回组合钞箱中
    CUContainer.CopyAllocCountFromCandidateList(CandidateList, uIndex);

    return 0;
}

//查找最大剩余张数，返回它的张数及钞箱索引
ULONG CMixMng::FindMaxLeftNumberIndex(const CU_COMBINATION_CONTAINER &CUContainer,
                                      CASS_INDEX_LIST &lstIndex,
                                      ULONG &ulSecondMaxLeftCount,
                                      ULONG ulValueMax)         //30-00-00-00(FS#0007)
{
    ULONG ulTotalValues = 0;
    ULONG ulMaxLeftCount = 0;   //最大剩余张数
    for (UINT i = 0; i < CUContainer.GetCUCount(); i++)
    {
        CU_COMBINATION *pCU = CUContainer.GetCU(i);

        if(pCU->GetValue() > ulValueMax){               //30-00-00-00(FS#0007)
            continue;                                   //30-00-00-00(FS#0007)
        }                                               //30-00-00-00(FS#0007)

        //计算剩余张数
        ULONG ulLeftCount = pCU->GetEqualEmptySafeLeftCount();

        //如果最大张数小于本钞箱的剩余张数，则保存本钞箱剩余张数和索引
        if (ulMaxLeftCount < ulLeftCount)
        {
            lstIndex.clear();
            lstIndex.insert(i);
            ulTotalValues = pCU->GetValue();
            ulSecondMaxLeftCount = ulMaxLeftCount;
            ulMaxLeftCount = ulLeftCount;
        }
        else if (ulMaxLeftCount == ulLeftCount)
        {
            lstIndex.insert(i);
            ulTotalValues += pCU->GetValue();
        }
        else if (ulSecondMaxLeftCount < ulLeftCount)
        {
            ulSecondMaxLeftCount = ulLeftCount;
        }
    }
    assert(ulTotalValues > 0);
    return ulTotalValues;
}

//按等空方式预分配金额到组合钞箱
//首先生成各个钞箱的等空安全张数
//再循环进行以下操作
//  1. 查找最大安全剩余张数的钞箱索引，存放在列表中，同时查找到第二多的钞箱的张数
//  2. 如果所有钞箱一样多，以最大剩余数进行分配; 否则，分配比第二多张数多出的部分
//      如果剩余金额未分配完，则保证剩余金额不小于面值总和。
//  3. 如果本次分配数为0张或剩余金额为0，则退出循环
//返回值：返回未分配完的剩余金额
ULONG CMixMng::PreAllocByEqualEmpty(ULONG ulAmount,
                                    CU_COMBINATION_CONTAINER &CUContainer,
                                    ULONG ulMaxDispenseCount)
{
    assert(CUContainer.GetCUCount() > 0);

//30-00-00-00(FT#0007)    CUContainer.GenerateEqualEmptySafeCount();

    ULONG ulLeftAmount = ulAmount;      //保存剩余金额
    ULONG ulTotalAllocCount = 0;        //保存总的分配张数

    //计算钞箱总面值
    const ULONG ulTotalValues = CUContainer.GetTotalValues();

    //循环给张数最多的分配，以接近平均数
    while (true)
    {
//30-00-00-00(FT#0007) add start
        //获取可用于分配的钞箱最大面额
        ULONG ulCstValueMax = 0;
        for(int i = 0; i < CUContainer.GetCUCount(); i++){
            ULONG ulCurrentCstValue = CUContainer.GetCU(i)->GetValue();
            if(ulCurrentCstValue > ulCstValueMax && ulCurrentCstValue <= ulLeftAmount){
                ulCstValueMax = CUContainer.GetCU(i)->GetValue();
            }
        }

        if(ulCstValueMax == 0){
            break;
        }
//30-00-00-00(FT#0007) add end
        //找到张数最多的
        ULONG ulSecondMaxLeftCount = 0;
        CASS_INDEX_LIST lstIndex; //最大剩余张数的钞箱索引列表
        ULONG ulTotalValuesOfMaxLeftCount = FindMaxLeftNumberIndex(CUContainer, lstIndex, ulSecondMaxLeftCount, ulCstValueMax);    //30-00-00-00(FS#0007)
//30-00-00-00(FT#0007)        assert(lstIndex.size() > 0);
        if(lstIndex.size() == 0){               //30-00-00-00(FT#0007)
            break;                              //30-00-00-00(FT#0007)
        }                                       //30-00-00-00(FT#0007)
        ULONG ulMaxLeftCount = CUContainer.GetCU(*lstIndex.begin())->GetEqualEmptySafeLeftCount();  //最大剩余张数
        assert(ulMaxLeftCount >= ulSecondMaxLeftCount);

//30-00-00-00(FT#0007) add start
        //最大剩余张数为0，跳出循环
        if(ulMaxLeftCount == 0){
            break;
        }

        int iIndex = 0;
        if(lstIndex.size() > 0){
            //张数相同，面额大的先出
            ULONG ulMaxValue = 0;
            CASS_INDEX_LIST::const_iterator it = lstIndex.begin();
            for(it; it != lstIndex.end(); it++){
                if(CUContainer.GetCU(*it)->GetValue() > ulMaxValue){
                    ulMaxValue = CUContainer.GetCU(*it)->GetValue();
                    iIndex = *it;
                }
            }
        } else {
            iIndex = *lstIndex.begin();
        }

        CU_COMBINATION *pCU = CUContainer.GetCU(iIndex);
        pCU->SetAllocCount(pCU->GetAllocCount() + 1);
        ULONG ulValue = pCU->GetValue();

        //更新分配总张数和剩余金额
        ulTotalAllocCount++;
        ulLeftAmount -= ulValue;

//30-00-00-00(FT#0007) add end
/*30-00-00-00(FT#0007) del start
        //如果剩余金额小于总面值加最大安全剩余数钞箱的总面值，不再继续预分配
         if (ulLeftAmount < ulTotalValues + ulTotalValuesOfMaxLeftCount)
             break;

        //如果所有钞箱一样多，以最大剩余数进行分配
        //否则，分配比第二多张数多出的部分
        ULONG ulAllocCount;
        if (lstIndex.size() == CUContainer.GetCUCount())    //全部钞箱一样多
        {
            ulAllocCount = ulMaxLeftCount;                  //各钞箱以最大剩余数分配
        }
        else
        {
            ulAllocCount = ulMaxLeftCount - ulSecondMaxLeftCount;   //分配比第二大张数多出的部分
        }

        //最少保留总面值以上的金额
        if ((ulAllocCount > (ulLeftAmount - (ulTotalValues + ulTotalValuesOfMaxLeftCount)) / ulTotalValuesOfMaxLeftCount))
        {
            ulAllocCount = (ulLeftAmount - ulTotalValues) / ulTotalValuesOfMaxLeftCount;
        }

        //出钞张数不可超过最大出钞张数
        ULONG ulMinNewAllocCount = CUContainer.ComputeMinNewAllocCount(
                                   ulLeftAmount - ulAllocCount * ulTotalValuesOfMaxLeftCount, lstIndex, ulAllocCount);
        if (ulTotalAllocCount + ulMinNewAllocCount >= ulMaxDispenseCount)
        {
            ulAllocCount = 0;
        }
        else if (ulTotalAllocCount + ulMinNewAllocCount + ulAllocCount * lstIndex.size() >= ulMaxDispenseCount)
        {
            ulAllocCount = (ulMaxDispenseCount - (ulTotalAllocCount + ulMinNewAllocCount)) / lstIndex.size();
        }

        if (ulAllocCount == 0)  //计算得到的张数为0，退出循环
        {
            break;              //不再继续分配
        }

        //分配到钞箱
        CASS_INDEX_LIST_CONST_IT it;
        for (it = lstIndex.begin(); it != lstIndex.end(); it++)
        {
            CU_COMBINATION *pCU = CUContainer.GetCU(*it);
            pCU->SetAllocCount(pCU->GetAllocCount() + ulAllocCount);
            assert(pCU->GetAllocCount() <= pCU->GetCount());
        }
        ulLeftAmount -= ulAllocCount * ulTotalValuesOfMaxLeftCount;
        ulTotalAllocCount += ulAllocCount * lstIndex.size();
30-00-00-00(FT#0007) del end*/
    }

    return ulLeftAmount;
}

//按等空方式进行配钞
HRESULT CMixMng::MixByEqualEmpty(ULONG ulAmount,
                                 CU_COMBINATION_CONTAINER &CUContainer,
                                 ULONG ulMaxDispenseCount)
{
    CUContainer.SortByValueDesc();  //按面值降序排序组合钞箱

    //首先按最少张数进行分配，如果能分配，保存其分配结果
    CANDIDATE_LIST CandidateList(CUContainer.GetCUCount());
    TryToMix(ulAmount, CUContainer, 0, CandidateList);
    if (CandidateList.GetCount() == 0)
        return ERR_MIX_ALGORITHM;

    //预分配
    ULONG ulLeftAmount = PreAllocByEqualEmpty(ulAmount, CUContainer, ulMaxDispenseCount);
    if (ulLeftAmount == 0) //如果金额分配完，直接返回
        return 0;

    //如果预分配未完成，在预分配的基础上递归计算
    TryToMix(ulLeftAmount, CUContainer, 0, CandidateList, TRUE);
    assert(CandidateList.GetCount() != 0);
    if (CandidateList.GetCount() == 0)
        return ERR_MIX_ALGORITHM;

    //查找最优数据的索引
    UINT uIndex = CandidateList.GetEqualEmptyIndex(CUContainer, ulMaxDispenseCount);
    if (uIndex == ERR_GET_INDEX)
        return ERR_MIX_MORE_ITEM;

    //把最优数据拷贝回组合钞箱中
    CUContainer.CopyAllocCountFromCandidateList(CandidateList, uIndex);

    return 0;
}

//按金额ulAmount尝试分配到从uStartIndex开始的后续钞箱
//如金额分配完成，结果保存到候选列表CandidateList中
//做两种尝试：1）前面钞箱分配最多；2）前面钞箱分配“最多-1”张
//该方法内部递归调用自身进行后续钞箱的尝试
void CMixMng::TryToMix(ULONG ulAmount,
                       const CU_COMBINATION_CONTAINER &CUContainer, UINT uStartIndex,
                       CANDIDATE_LIST &CandidateList,
                       BOOL bDecToZero)
{
    CU_COMBINATION *pCU = CUContainer.GetCU(uStartIndex);   //得到当前钞箱
    ULONG ulOldAllocCount = pCU->GetAllocCount();           //原先已经分配的张数
    ULONG ulNewAllocCount = ulAmount / pCU->GetValue();     //新分配的张数，初始为最大张数

    //处理最后一个钞箱的情况
    if (uStartIndex + 1 == CUContainer.GetCUCount())
    {
        if (ulAmount % pCU->GetValue() != 0)                //金额不能被整除，不加入到候选列表中
            return;

        if (ulNewAllocCount > pCU->GetLeftCount())          //新分配的张数大于剩余张数，不加入到候选列表中
            return;

        //保存到候选列表中
        pCU->SetAllocCount(ulOldAllocCount + ulNewAllocCount);
        CUContainer.AddAllocCountsToCandidateList(CandidateList);
        pCU->SetAllocCount(ulOldAllocCount);            //恢复当前钞箱分配数为老值，以便于下次再做递归调用
        return;
    }

    //处理新分配数大于剩余数的情况
    if (ulNewAllocCount > pCU->GetLeftCount())      //新分配数大于剩余数
    {
        ulNewAllocCount = pCU->GetLeftCount();
    }

    //循环进行分配，直到新分配的张数为
    //  bDecToZero为TRUE的情况:  0
    //  bDecToZero为FALSE的情况: 最大张数-1()
    ULONG ulLoopCount = 0;
    while (true)
    {
        pCU->SetAllocCount(ulNewAllocCount + ulOldAllocCount);  //设置为总的分配张数
        ULONG ulAmountLeft = ulAmount - ulNewAllocCount * pCU->GetValue();  //计算剩余金额
        if (ulAmountLeft == 0) //如果剩余金额正好为0，记录到候选列表中
        {
            CUContainer.AddAllocCountsToCandidateList(CandidateList);
        }

        TryToMix(ulAmountLeft, CUContainer, uStartIndex + 1, CandidateList, bDecToZero); //对剩余金额进行递归调用

        if (ulNewAllocCount == 0)               //如果新分配的张数为0，退出循环
            break;

        ulLoopCount++;
        if (!bDecToZero && ulLoopCount == 2)    //如果不递减到0，并且循环了2次，退出循环
            break;

        ulNewAllocCount--;                      //分配张数减一
    }

    pCU->SetAllocCount(ulOldAllocCount);        //恢复当前钞箱分配数为老值，以便于下次再做递归调用
}

//功能：配钞 (实现接口文件定义的纯虚函数)
HRESULT CMixMng::MixByAlgorithm(
ULONG *aryulResult,
const LPMIXCUINFOR aryCUInfor,
USHORT usSize,
ULONG  ulAmount,
MIXALGORITHM alg,
ULONG ulMaxDispenseCount)
{
    DEFMOD(MixByAlgorithm);

    if (nullptr == aryCUInfor ||
        usSize == 0 ||
        ulAmount == 0 ||
        aryulResult == nullptr)
    {
        Log(ThisModule, ERR_MIX_INPUT_PARM,
            "Parameter error: aryCUInfor(0x%08X) = NULL or usSize(%hd) = 0 or ulAmount(%d) = 0 or aryulResult(0x%08X) = NULL",
            aryCUInfor, usSize, ulAmount, aryulResult);
        return ERR_MIX_INPUT_PARM;
    }

    //形成组合钞箱容器，并校验钞箱数据有效性
    CU_COMBINATION_CONTAINER CUCombinationContainer;
    CUCombinationContainer.GenerateCU(aryCUInfor, usSize);
    if (CUCombinationContainer.GetCUCount() == 0 ||
        CUCombinationContainer.GetTotalAmounts() < ulAmount)
    {
        Log(ThisModule, ERR_MIX_ALGORITHM,
            "No usable cassette or Total Amounts(%d) < ulAmount(%d), CUCombinationContainer.GetCUCount() == %d",
            CUCombinationContainer.GetTotalAmounts(), ulAmount, CUCombinationContainer.GetCUCount());
        return ERR_MIX_ALGORITHM;
    }

    //按算法配钞
    HRESULT hRes = 0;
    switch (alg)
    {
    case MIX_MINIMUM_NUMBER:
        hRes = MixByMinNumber(ulAmount, CUCombinationContainer, ulMaxDispenseCount);
        break;

    case MIX_MAXIMUM_NUMBER:
        hRes = MixByMaxNoteNumber(ulAmount, CUCombinationContainer, ulMaxDispenseCount);
        break;

    case MIX_EQUAL_EMPTYING:
        hRes = MixByEqualEmpty(ulAmount, CUCombinationContainer, ulMaxDispenseCount);
        break;

    default:
        assert(false);
        hRes = ERR_MIX_INPUT_PARM;
        break;
    }

    //处理结果，记录日志
    if (hRes < 0)
    {
        Log(ThisModule, static_cast<int>(hRes),
            "Mix failed: alg = %d, ulAmount = %d, usSize = %hd, CassetteInfo: %s",
            alg, ulAmount, usSize, CUCombinationContainer.GetCUInfoStrings().c_str());
        return hRes;
    }

    //拷贝数据回结果数组
    CUCombinationContainer.CopyAllocCountsToResultArray(aryulResult, aryCUInfor, usSize);

    return 0;
}

extern "C" Q_DECL_EXPORT IMixManager *CreateMixManager()
{
    return new CMixMng;
}
