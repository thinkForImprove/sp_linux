#include "CIMResult.h"
#define THISFILE                    "SPBaseCIM"
#define EVENT_LOG                   "Event.log"

// CCIMSetUpdatesOfCU

// 设置原始钞箱数据，初始设置所有钞箱为不引用并将相关计数清零
//返回值：WFS_ERR_INTERNAL_ERROR 加载原始钞箱数据失败
//        WFS_SUCCESS            加载原始钞箱数据成功
long CCIMSetUpdatesOfCU::SetSourceInfor(const LPWFSCIMCASHINFO lpCashInfo)
{
    ClearData();

    //如果逻辑钞箱列表为空或者usCount为0，记录日志并返回内部错误，否则，加载钞箱原始数据
    if (lpCashInfo->lppCashIn != NULL && lpCashInfo->usCount != 0)
    {
        this->lppCashIn = new LPWFSCIMCASHIN[lpCashInfo->usCount];
        this->usCount = lpCashInfo->usCount;
        memset(this->lppCashIn, 0, sizeof(LPWFSCIMCASHIN) * lpCashInfo->usCount);

        //加载usCount个逻辑钞箱数据，逻辑钞箱的列表大小为usCount
        for (int i = 0; i < lpCashInfo->usCount; i++)
        {
            //如果逻辑钞箱存在，则加载钞箱数据，否则，记录日志并返回内部错误
            if (lpCashInfo->lppCashIn[i] != NULL)
            {
                this->lppCashIn[i] = new WFSCIMCASHIN;
                memcpy(this->lppCashIn[i], lpCashInfo->lppCashIn[i], sizeof(WFSCIMCASHIN));
                this->lppCashIn[i]->ulCashInCount = 0;         //清空计数
                this->lppCashIn[i]->ulCount = -1;          //初始设置钞箱为不引用
                this->lppCashIn[i]->lpNoteNumberList = NULL;
                this->lppCashIn[i]->lppPhysical = NULL;
                this->lppCashIn[i]->lpszExtra = NULL;

                //加载逻辑钞箱的钞票列表数据，如果是Retract钞箱，该列表允许为NULL
                if (lpCashInfo->lppCashIn[i]->lpNoteNumberList != NULL)
                {
                    //加载逻辑钞箱对应的物理钞箱原始数据，失败则返回内部错误
                    if (SetNoteNumberList(lpCashInfo->lppCashIn[i]->lpNoteNumberList, i) ==
                        WFS_ERR_INTERNAL_ERROR)
                    {
                        return WFS_ERR_INTERNAL_ERROR;
                    }

                }

                //如果物理钞箱不存在，记录日志并返回内部错误，否则加载原始物理钞箱数据
                if (lpCashInfo->lppCashIn[i]->lppPhysical != NULL && lpCashInfo->lppCashIn[i]->usNumPhysicalCUs != 0)
                {
                    //加载逻辑钞箱对应的物理钞箱原始数据，失败则返回内部错误
                    if (SetPhysicalCU(lpCashInfo->lppCashIn[i]->lppPhysical, lpCashInfo->lppCashIn[i]->usNumPhysicalCUs, i) ==
                        WFS_ERR_INTERNAL_ERROR)
                    {
                        return WFS_ERR_INTERNAL_ERROR;
                    }
                }
                else
                {
                    Log("CCIMSetUpdatesOfCU::SetSourceInfor()",
                        WFS_ERR_INTERNAL_ERROR, "物理钞箱列表为空或者物理钞箱个数为0");
                    return WFS_ERR_INTERNAL_ERROR;
                }

                //设置逻辑钞箱的厂家自定义信息
                if (this->lppCashIn[i]->lpszExtra != NULL)
                {
                    /*
                    CMultiString Extra(this->lppCashIn[i]->lpszExtra);
                    this->lppCashIn[i]->lpszExtra = new char[Extra.GetTotalLen()];
                    memcpy(this->lppCashIn[i]->lpszExtra, lpCashInfo->lppCashIn[i]->lpszExtra, Extra.GetTotalLen());
                    */
                }
            }
            else
            {
                Log("CCIMSetUpdatesOfCU::SetSourceInfor()",
                    WFS_ERR_INTERNAL_ERROR, "第%d个逻辑钞箱没有配置", i + 1);
                return WFS_ERR_INTERNAL_ERROR;
            }
        }
    }
    else
    {
        Log("CCIMSetUpdatesOfCU::SetSourceInfor()", WFS_ERR_INTERNAL_ERROR,
            "逻辑钞箱列表为空或者逻辑钞箱个数为0");
        return WFS_ERR_INTERNAL_ERROR;
    }
    return WFS_SUCCESS;
}

// 功能：钞箱逻辑和物理一对一，如果多次调用该命令设置同一个钞箱的计数，
//       每次都重新设置钞箱计数
// usNumber[in]: 钞箱号(从1开始，以后每个钞箱递增1)
// ulCashInCount[in]: 进钞数  ulCount[in]:当前张数
//返回值:WFS_SUCCESS                      成功
//       WFS_ERR_INTERNAL_ERROR 内部数据错误
long CCIMSetUpdatesOfCU::SetCount(USHORT usNumber, ULONG ulCashInCount, ULONG ulCount)
{
    //调用此方法前需确保SetSourceInfor被调用，否则，记录日志并返回错误
    if (this->lppCashIn == NULL)
    {
        Log("CCIMSetUpdatesOfCU::SetCount",
            WFS_ERR_INTERNAL_ERROR, "CCIMSetUpdatesOfCU::SetSourceInfor未被调用");
        return WFS_ERR_INTERNAL_ERROR;
    }

    //在钞箱列表中找钞箱号为usNumber的钞箱
    int iCUIndex = FindCUIndex(this->lppCashIn, this->usCount, usNumber); //钞箱列表中的逻辑钞箱当前索引

    //如果找到钞箱号为usNumber的钞箱，则设置其逻辑和物理钞箱的ulCashInCount、ulCount,
    //否则记录日志并返回IDS_ERR_CANNOT_FIND_CASHUNIT
    if (iCUIndex != this->usCount)
    {
        assert(this->lppCashIn[iCUIndex] != NULL);
        this->lppCashIn[iCUIndex]->ulCashInCount = ulCashInCount;
        this->lppCashIn[iCUIndex]->ulCount = ulCount;

        assert((this->lppCashIn[iCUIndex]->lppPhysical != NULL) &&
               (this->lppCashIn[iCUIndex]->usNumPhysicalCUs != 0));

        //设置逻辑钞箱对应的各个物理钞箱的计数
        for (int j = 0; j < this->lppCashIn[iCUIndex]->usNumPhysicalCUs; j++)
        {
            LPWFSCIMPHCU &pPhysical = this->lppCashIn[iCUIndex]->lppPhysical[j];
            assert(pPhysical != NULL);
            pPhysical->ulCount = ulCount;
            pPhysical->ulCashInCount = ulCashInCount;
        }

        return WFS_SUCCESS;
    }
    else
    {
        Log("CCIMSetUpdatesOfCU::SetCount",
            WFS_ERR_INTERNAL_ERROR, "逻辑钞箱[usNumber=%hd]不存在", usNumber);
        return WFS_ERR_INTERNAL_ERROR;
    }
}

// 调用此方法设置相应计数，ID不存在就自动新增,如果多次调用该命令设置同一个钞箱的币种计数，
//       每次都重新设置钞箱的币种计数
// usNumber[in]: 钞箱号(从1开始，以后每个钞箱递增1)
// ulCashInCount[in]: 进钞数  ulCount[in]:当前张数
//返回值:WFS_SUCCESS                        成功
//       WFS_ERR_INTERNAL_ERROR             内部数据错误
long CCIMSetUpdatesOfCU::SetNoteNumber(USHORT usNumber, USHORT usNoteID, ULONG ulCount)
{
    //调用此方法前需确保SetSourceInfor被调用，否则，记录日志并返回错误
    if (this->lppCashIn == NULL)
    {
        Log("CCIMSetUpdatesOfCU::SetCount",
            WFS_ERR_INTERNAL_ERROR, "CCIMSetUpdatesOfCU::SetSourceInfor未被调用");
        return WFS_ERR_INTERNAL_ERROR;
    }

    //在钞箱列表中找钞箱号为usNumber的钞箱
    int iCUIndex = FindCUIndex(this->lppCashIn, this->usCount, usNumber);   //逻辑钞箱列表的当前索引

    //如果找到钞箱号为usNumber的钞箱，则设置其币种列表中ID为usNoteID的钞票数,
    //并返回WFS_SUCCESS; 否则,记录日志并返回IDS_ERR_CANNOT_FIND_CASHUNIT
    if (iCUIndex != this->usCount)
    {
        //调用基类CWFSCIMCashInfo::SetNoteNumberEx设置指定逻辑钞箱的LPWFSCIMNOTENUMBER数据.
        SetNoteNumberEx(iCUIndex, usNoteID, ulCount);
        return WFS_SUCCESS;
    }
    else
    {
        Log("CCIMSetUpdatesOfCU::SetCount",
            WFS_ERR_INTERNAL_ERROR, "钞箱[usNumber=%hd]不存在", usNumber);
        return WFS_ERR_INTERNAL_ERROR;
    }
}

//功能:辅助函数,设置一次CashIn操作中钞票列表(usNoteID--ulCount)
//lpNoteNumberList:逻辑钞箱iCUNum对应的币种列表; iCUNum:逻辑钞箱号码
//返回值：WFS_ERR_INTERNAL_ERROR 加载逻辑钞箱的钞票列表数据失败
//        WFS_SUCCESS            加载原始钞箱的钞票列表数据成功
long CCIMSetUpdatesOfCU::SetNoteNumberList(const LPWFSCIMNOTENUMBERLIST lpNoteNumberList, int iCUNum)
{
    assert((lpNoteNumberList != NULL) && (iCUNum >= 0));
    assert(this->lppCashIn[iCUNum]->lpNoteNumberList == NULL);

    LPWFSCIMNOTENUMBERLIST &lpTempList = this->lppCashIn[iCUNum]->lpNoteNumberList;
    CWFSCIMNoteNumberList *temp = new CWFSCIMNoteNumberList;
    lpTempList = temp->GetNoteNumberList();

    //设置钞票列表数据,钞票列表不为NULL且列表个数不为0，则为本地数据赋值,
    //如果两者之一有为NULL或0，则记录日志并返回错误
    //如过钞票列表为NULL且列表个数为0,则返回成功
    if ((lpNoteNumberList->lppNoteNumber != NULL) && (lpNoteNumberList->usNumOfNoteNumbers != 0))
    {
        //设置钞箱包含的BankNote Number列表
        for (int j = 0; j < lpNoteNumberList->usNumOfNoteNumbers; j++)
        {
            if (lpNoteNumberList->lppNoteNumber[j] != NULL)
            {
                temp->SetCountByIDEx(lpNoteNumberList->lppNoteNumber[j]->usNoteID, 0);
            }
            else
            {
                Log("CCIMSetUpdatesOfCU::SetSourceInfor, ERROR:SetNoteNumberList",
                    WFS_ERR_INTERNAL_ERROR, "逻辑钞箱[usNumber=%hd]的钞票列表的第%d个为NULL",
                    this->lppCashIn[iCUNum]->usNumber, j + 1);
                return WFS_ERR_INTERNAL_ERROR;
            }
        }
        return WFS_SUCCESS;
    }
    else if ((lpNoteNumberList->lppNoteNumber != NULL))
    {
        Log("CCIMSetUpdatesOfCU::SetSourceInfor, ERROR:SetNoteNumberList",
            WFS_ERR_INTERNAL_ERROR, "逻辑钞箱[usNumber=%hd]的钞票列表lppNoteNumber==NULL",
            this->lppCashIn[iCUNum]->usNumber);
        return WFS_ERR_INTERNAL_ERROR;
    }
    else if (lpNoteNumberList->usNumOfNoteNumbers != 0)
    {
        Log("CCIMSetUpdatesOfCU::SetSourceInfor, ERROR:SetNoteNumberList",
            WFS_ERR_INTERNAL_ERROR, "逻辑钞箱[usNumber=%hd]的钞票列表大小usNumOfNoteNumbers==0",
            this->lppCashIn[iCUNum]->usNumber);
        return WFS_ERR_INTERNAL_ERROR;
    }
    return WFS_SUCCESS;
}

//功能: 辅助函数,设置逻辑钞箱对应的物理钞箱组
//lppPhysical:物理钞箱数组;  count:物理钞箱的个数; iCUNum:逻辑钞箱的序号
//返回值：WFS_ERR_INTERNAL_ERROR 加载原始钞箱数据失败
//        WFS_SUCCESS            加载原始钞箱数据成功
long CCIMSetUpdatesOfCU::SetPhysicalCU(const LPWFSCIMPHCU *lppPhysical, int count, int iCUNum)
{
    assert((lppPhysical != NULL) && (count >= 0) && (iCUNum >= 0));
    assert(this->lppCashIn[iCUNum]->lppPhysical == NULL);

    this->lppCashIn[iCUNum]->lppPhysical = new LPWFSCIMPHCU[count];
    memset(this->lppCashIn[iCUNum]->lppPhysical, 0, sizeof(LPWFSCIMPHCU) * count);

    //设置逻辑钞箱列表中下标为iCUNum的物理钞箱列表数据
    for (int j = 0; j < count; j++)
    {
        LPWFSCIMPHCU &pPhysical = this->lppCashIn[iCUNum]->lppPhysical[j];

        //如果物理钞箱存在，则拷贝其内容到新分配的控件，否则记录日志并返回WFS_ERR_INTERNAL_ERROR
        if (lppPhysical[j] != NULL)
        {
            pPhysical = new WFSCIMPHCU;
            memcpy(pPhysical, lppPhysical[j], sizeof(WFSCIMPHCU));
            pPhysical->ulCashInCount = 0;   //清空钞箱的计数
            pPhysical->ulCount = 0;
            pPhysical->lpPhysicalPositionName = NULL;
            pPhysical->lpszExtra = NULL;

            //设置CIM中物理钞箱的位置标识信息
            if (lppPhysical[j]->lpPhysicalPositionName != NULL)
            {
                pPhysical->lpPhysicalPositionName =
                new char[strlen(lppPhysical[j]->lpPhysicalPositionName) + 1];
                strcpy(pPhysical->lpPhysicalPositionName, lppPhysical[j]->lpPhysicalPositionName);
            }

            //设置物理钞箱的厂家自定义信息
            if (lppPhysical[j]->lpszExtra != NULL)
            {
                /*
                CMultiString Extra(lppPhysical[j]->lpszExtra);
                pPhysical->lpszExtra = new char[Extra.GetTotalLen()];
                memcpy(pPhysical->lpszExtra, lppPhysical[j]->lpszExtra, Extra.GetTotalLen());
                */
            }

        }
        else
        {
            Log("CCIMSetUpdatesOfCU::SetSourceInfor, ERROR:SetPhysicalCU",
                WFS_ERR_INTERNAL_ERROR, "逻辑钞箱[usNumber=%hd]的第%d个物理钞箱没有配置",
                this->lppCashIn[iCUNum]->usNumber, j + 1);
            return WFS_ERR_INTERNAL_ERROR;
        }
    }
    return WFS_SUCCESS;
}

//功能:辅助函数,清除逻辑钞箱num的WFSCIMNOTENUMBERLIST结构
//num:逻辑钞箱列表中的下标
void CWFSCIMCashInfo::ClearNoteNumberList(int num)
{
    LPWFSCIMNOTENUMBERLIST &lpTempList = this->lppCashIn[num]->lpNoteNumberList;

    //如果是Retract钞箱，lpTempList为NULL
    if (lpTempList != NULL)
    {
        delete (CWFSCIMNoteNumberList *)lpTempList;
        lpTempList = NULL;
    }

}

//功能:辅助函数,清除逻辑钞箱num的LPWFSCIMPHCU结构数组
//num:逻辑钞箱序号
void CWFSCIMCashInfo::ClearPhysicalCU(int num)
{
    if (this->lppCashIn[num]->lppPhysical != NULL)
    {
        for (int i = 0; i < this->lppCashIn[num]->usNumPhysicalCUs; i++)
        {
            LPWFSCIMPHCU &pPhysical = this->lppCashIn[num]->lppPhysical[i];

            if (pPhysical != NULL)
            {
                if (pPhysical->lpPhysicalPositionName != NULL)
                {
                    delete[] pPhysical->lpPhysicalPositionName;
                    pPhysical->lpPhysicalPositionName = NULL;
                }

                if (pPhysical->lpszExtra != NULL)
                {
                    delete[] pPhysical->lpszExtra;
                    pPhysical->lpszExtra = NULL;
                }

                delete pPhysical;
            }

        }
        delete[] this->lppCashIn[num]->lppPhysical;
        this->lppCashIn[num]->lppPhysical = NULL;
    }
}

CNoteTypeList::CNoteTypeList()
{
    usNumOfNoteTypes = 0;
    lppNoteTypes = NULL;
}

void CNoteTypeList::Copy(const WFSCIMNOTETYPELIST &src)
{
    assert(usNumOfNoteTypes == 0 && lppNoteTypes == NULL);
    if (src.usNumOfNoteTypes == 0)
        return;
    lppNoteTypes = new LPWFSCIMNOTETYPE[src.usNumOfNoteTypes];
    for (USHORT i = 0; i < src.usNumOfNoteTypes; i++)
    {
        assert(src.lppNoteTypes[i] != NULL);
        lppNoteTypes[i] = new WFSCIMNOTETYPE;
        *lppNoteTypes[i] = *src.lppNoteTypes[i];
    }
    usNumOfNoteTypes = src.usNumOfNoteTypes;
}

CNoteTypeList::~CNoteTypeList()
{
    if (usNumOfNoteTypes == 0)
        return;
    for (USHORT i = 0; i < usNumOfNoteTypes; i++)
    {
        assert(lppNoteTypes[i] != NULL);
        delete lppNoteTypes[i];
    }
    delete[] lppNoteTypes;
    lppNoteTypes = NULL;
    usNumOfNoteTypes = 0;
}
