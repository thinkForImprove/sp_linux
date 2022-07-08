#pragma once
#include "CIMInterface.h"
#include "ILogWrite.h"
#include "MultiString.h"
#include <assert.h>

//在钞箱列表中查找钞箱号为usNumber的下标索引,如果不存在，则iCUIndex==usCount
inline int FindCUIndex(const LPWFSCIMCASHIN *lppCashIn, USHORT usCount, USHORT usNumber)
{
    int i = 0;
    assert(lppCashIn != NULL);
    for (; i < usCount; i++)
    {
        assert((lppCashIn[i] != NULL));
        if ((lppCashIn[i] != NULL) && (usNumber == lppCashIn[i]->usNumber))
        {
            break;
        }
    }
    return i;
}

//查找usNoteID是否已存在，如果存在，返回值小于usNumOfNoteNumbers，否则等于usNumOfNoteNumbers
inline int FindIDIndex(const LPWFSCIMNOTENUMBER *lppNoteNumber, USHORT usNumOfNoteNumbers, USHORT usNoteID)
{
    int i = 0;
    if (lppNoteNumber == NULL)
    {
        return 0;
    }

    for (; i < usNumOfNoteNumbers; i++)
    {
        assert((lppNoteNumber[i] != NULL));
        if (usNoteID == lppNoteNumber[i]->usNoteID)
        {
            break;
        }
    }
    return i;
}

//类CCIMStatus
//功能：实现ICIMStatus接口，同时包装WFSCIMSTATUS数据
struct CCIMStatus : public ICIMStatus, public WFSCIMSTATUS
{
    CCIMStatus()
    {
        InitData();
    }

    virtual ~CCIMStatus() {}

    //清空本地数据并初始化类成员
    void InitData()
    {
        memset((WFSCIMSTATUS *)this, 0, sizeof(WFSCIMSTATUS));
        memset(&m_Pos, 0, sizeof(WFSCIMINPOS) * 3);

        m_lparryPos[0] = &m_Pos[0];  //第一个指向输入位置
        m_lparryPos[1] = &m_Pos[1];  //第二个指向输出位置
        m_lparryPos[2] = NULL;    //第三个值总是NULL

        this->lppPositions = m_lparryPos;

        m_Extra = NULL;    //清空CMultiString数据
    }

    virtual long SetDeviceSt(WORD fwDevice)
    {
        this->fwDevice = fwDevice;
        return 0;
    }

    virtual long SetSafeDoorSt(WORD fwSafeDoor)
    {
        this->fwSafeDoor = fwSafeDoor;
        return 0;
    }

    virtual long SetAcceptorSt(WORD fwAcceptor)
    {
        this->fwAcceptor = fwAcceptor;
        return 0;
    }

    virtual long SetIntermediateStackerSt(WORD fwIntermediateStacker)
    {
        this->fwIntermediateStacker = fwIntermediateStacker;
        return 0;
    }

    virtual long SetStackerItemsSt(WORD fwStackerItems)
    {
        this->fwStackerItems = fwStackerItems;
        return 0;
    }

    virtual long SetBanknoteReaderSt(WORD fwBanknoteReader)
    {
        this->fwBanknoteReader = fwBanknoteReader;
        return 0;
    }

    virtual long SetDropBox(BOOL bDropBox)
    {
        this->bDropBox = bDropBox;
        return 0;
    }

    virtual long AddPositionSt(const WFSCIMINPOS arryPosition[2])
    {
        memcpy(m_Pos, arryPosition, sizeof(WFSCIMINPOS) * 2);
        return 0;
    }

    virtual long AddExtraSt(LPCSTR lpszKey, LPCSTR lpszValue)
    {
        char szBuf[1024] = { 0 };
        sprintf(szBuf, "%s=%s", lpszKey, lpszValue);
        m_Extra.Add(szBuf);
        this->lpszExtra = (LPSTR)(LPCSTR)m_Extra;
        return 0;
    }

private:
    WFSCIMINPOS m_Pos[2];             //有效的位置元素
    LPWFSCIMINPOS m_lparryPos[3];  //NULL结束的指针列表，只包含一个元素
    CMultiString m_Extra;          //保存以'\0'分割"\0\0"结束的厂家自定义信息
};

//类CCIMCaps
//功能：实现ICIMCaps接口，同时包装WFSCIMCAPS数据
struct CCIMCaps : public ICIMCaps, public WFSCIMCAPS
{
    CCIMCaps()
    {
        InitData();
    }

    virtual ~CCIMCaps()
    {

    }

    //清空本地数据并初始化类成员
    void InitData()
    {
        memset((WFSCIMCAPS *)this, 0, sizeof(WFSCIMCAPS));
        this->wClass = WFS_SERVICE_CLASS_CIM;
        m_ExtraCp = NULL;
    }

    virtual long SetType(WORD fwType)
    {
        this->fwType = fwType;
        return 0;
    }

    virtual long SetMaxCashInItems(WORD wMaxCashInItems)
    {
        this->wMaxCashInItems = wMaxCashInItems;
        return 0;
    }

    virtual long SetCompound(BOOL bCompound)
    {
        this->bCompound = bCompound;
        return 0;
    }

    virtual long SetShutter(BOOL bShutter)
    {
        this->bShutter = bShutter;
        return 0;
    }

    virtual long SetShutterControl(BOOL bShutterControl)
    {
        this->bShutterControl = bShutterControl;
        return 0;
    }

    virtual long SetSafeDoor(BOOL bSafeDoor)
    {
        this->bSafeDoor = bSafeDoor;
        return 0;
    }

    virtual long SetCashBox(BOOL bCashBox)
    {
        this->bCashBox = bCashBox;
        return 0;
    }

    virtual long SetRefill(BOOL bRefill)
    {
        this->bRefill = bRefill;
        return 0;
    }

    virtual long SetIntermediateStacker(WORD fwIntermediateStacker)
    {
        this->fwIntermediateStacker = fwIntermediateStacker;
        return 0;
    }

    virtual long SetItemsTakenSensor(BOOL bItemsTakenSensor)
    {
        this->bItemsTakenSensor = bItemsTakenSensor;
        return 0;
    }

    virtual long SetItemsInsertedSensor(BOOL bItemsInsertedSensor)
    {
        this->bItemsInsertedSensor = bItemsInsertedSensor;
        return 0;
    }

    virtual long SetPositions(WORD fwPositions)
    {
        this->fwPositions = fwPositions;
        return 0;
    }

    virtual long SetExchangeType(WORD fwExchangeType)
    {
        this->fwExchangeType = fwExchangeType;
        return 0;
    }

    virtual long SetRetractAreas(WORD fwRetractAreas)
    {
        this->fwRetractAreas = fwRetractAreas;
        return 0;
    }

    virtual long SetRetractTransportActions(WORD fwRetractTransportActions)
    {
        this->fwRetractTransportActions = fwRetractTransportActions;
        return 0;
    }

    virtual long SetRetractStackerActions(WORD fwRetractStackerActions)
    {
        this->fwRetractStackerActions = fwRetractStackerActions;
        return 0;
    }

    //获取厂家自定义的信息，形式为(lpszKey=lpszValue),以NULL为间隔，以俩个NULL为结束标记
    virtual long AddExtraCp(LPCSTR lpszKey, LPCSTR lpszValue)
    {
        char szBuf[1024];
        sprintf(szBuf, "%s=%s", lpszKey, lpszValue);
        m_ExtraCp.Add(szBuf);
        this->lpszExtra = (LPSTR)(LPCSTR)m_ExtraCp;
        return 0;
    }

private:
    CMultiString m_ExtraCp;      //保存以'\0'分割"\0\0"结束的厂家自定义信息
};

//CCIMCurrencyExp类:
//功能：实现ICIMCurrencyExp接口，包含设备支持的所有钞票的信息
struct CCIMCurrencyExp : public ICIMCurrencyExp//,public CNPArray<WFSCIMCURRENCYEXP, LPWFSCIMCURRENCYEXP>
{
    CCIMCurrencyExp() {}
    virtual ~CCIMCurrencyExp()
    {
        ClearData();
    }

    void ClearData()
    {
        //this->Clear();
    }

    virtual long AddCurrencyExp(const WFSCIMCURRENCYEXP &Exp)
    {
        //DWORD dwIndex = this->Insert(-1);
        //this->GetAt(dwIndex) = Exp;
        return 0;
    }
};


//CCIMBanknoteTypes类
//功能：实现ICIMBanknoteTypes接口，同时封装WFSCIMNOTETYPELIST数据,包含BV能检测到的所有币种信息
struct CCIMBanknoteTypes : public ICIMBanknoteTypes, public WFSCIMNOTETYPELIST
{
    CCIMBanknoteTypes()
    {
        memset((WFSCIMNOTETYPELIST *)this, 0, sizeof(WFSCIMNOTETYPELIST));
    }

    virtual ~CCIMBanknoteTypes()
    {
        ClearData();
    }

    //释放掉分配的内存并清空本地数据
    void ClearData()
    {
        if (this->lppNoteTypes != NULL)
        {
            for (int i = 0; i < this->usNumOfNoteTypes; i++)
            {
                delete this->lppNoteTypes[i];
            }
            delete[] this->lppNoteTypes;
            this->lppNoteTypes = NULL;
        }
        memset((WFSCIMNOTETYPELIST *)this, 0, sizeof(WFSCIMNOTETYPELIST));
    }

    //增加BV能检测的钞票信息
    virtual long AddBanknoteTypes(const LPWFSCIMNOTETYPE lpNoteType)
    {
        //临时变量，每调用该方法一次，重新分配内存
        LPWFSCIMNOTETYPE *lppNoteTypes = new LPWFSCIMNOTETYPE[this->usNumOfNoteTypes + 1];
        memset(lppNoteTypes, 0, sizeof(LPWFSCIMNOTETYPE) * (this->usNumOfNoteTypes + 1));

        //如果已存在币种信息，将其内容拷贝到新分配的内存中并释放先前分配的内存
        if (this->usNumOfNoteTypes != 0)
        {
            memcpy(lppNoteTypes, this->lppNoteTypes,
                   sizeof(LPWFSCIMNOTETYPE) * this->usNumOfNoteTypes);
            delete[] this->lppNoteTypes;
            this->lppNoteTypes = NULL;
        }

        //将新增的币种信息添加到币种列表中
        this->lppNoteTypes = lppNoteTypes;
        LPWFSCIMNOTETYPE pNoteType = new WFSCIMNOTETYPE;
        memcpy(pNoteType, lpNoteType, sizeof(WFSCIMNOTETYPE));
        this->lppNoteTypes[this->usNumOfNoteTypes] = pNoteType;

        //每调用该方法一次,币种列表大小增加1个
        this->usNumOfNoteTypes++;

        return 0;
    }
};

//CCIMNoteNumberList类
//功能：封装WFSCIMNOTENUMBERLIST数据，被CCIMCashInResult继承
struct CWFSCIMNoteNumberList : private WFSCIMNOTENUMBERLIST
{
    CWFSCIMNoteNumberList()
    {
        memset((WFSCIMNOTENUMBERLIST *)this, 0, sizeof(WFSCIMNOTENUMBERLIST));
    }

    virtual ~CWFSCIMNoteNumberList()
    {
        ClearData();
    }
    //释放掉分配的内存并清空本地数据
    void ClearData()
    {
        if (lppNoteNumber != NULL)
        {
            for (int i = 0; i < usNumOfNoteNumbers; i++)
            {
                if (lppNoteNumber[i] != NULL)
                {
                    delete lppNoteNumber[i];
                }

            }
            delete[] lppNoteNumber;
        }
        memset((WFSCIMNOTENUMBERLIST *)this, 0, sizeof(WFSCIMNOTENUMBERLIST));
    }

    WFSCIMNOTENUMBERLIST *GetNoteNumberList()
    {
        return (WFSCIMNOTENUMBERLIST *)this;
    }


    // 设置币种列表中的ID为usNoteID的钞票数目
    long SetCountByIDEx(USHORT usNoteID, ULONG ulCount)
    {
        //查找钞票列表中的ID时的位置
        int iIDListIndex = FindIDIndex(this->lppNoteNumber, this->usNumOfNoteNumbers, usNoteID);

        //如果在钞票的列表中找不到币种ID为usNoteID的钞票，
        //新增1个节点存储新增加的WFSCIMNOTENUMBER数据，否则，直接赋值。
        if (iIDListIndex == this->usNumOfNoteNumbers)
        {
            LPWFSCIMNOTENUMBER  *lppNoteNumber =
            new LPWFSCIMNOTENUMBER[this->usNumOfNoteNumbers + 1];  //在原来的基础上增加1个节点
            memset(lppNoteNumber, 0, sizeof(LPWFSCIMNOTENUMBER) *
                   (this->usNumOfNoteNumbers + 1));

            //将先前的钞票列表保存到新分配的列表中并释放先前分配的内存空间，
            //然后再将新分配的列表保存到本地成员变量中
            if (this->lppNoteNumber != NULL)
            {
                memcpy(lppNoteNumber, this->lppNoteNumber,
                       sizeof(LPWFSCIMNOTENUMBER) * this->usNumOfNoteNumbers);
                delete[] this->lppNoteNumber;
                this->lppNoteNumber = NULL;
            }
            this->lppNoteNumber = lppNoteNumber;

            //动态生成一个新节点存储新增加的WFSCIMNOTENUMBER数据,
            //并将NoteNumberList中的实际有效个数增加1个
            this->lppNoteNumber[iIDListIndex] = new WFSCIMNOTENUMBER;
            this->lppNoteNumber[iIDListIndex]->ulCount = ulCount;
            this->lppNoteNumber[iIDListIndex]->usNoteID = usNoteID;
            this->usNumOfNoteNumbers++;
        }
        else
        {
            this->lppNoteNumber[iIDListIndex]->ulCount = ulCount;
        }
        return 0;
    }

  //test#13  start
    // 追加币种列表中的ID为usNoteID的钞票数目
    long AddCountByIDEx(USHORT usNoteID, ULONG ulCount)
    {
        //查找钞票列表中的ID时的位置
        int iIDListIndex = FindIDIndex(this->lppNoteNumber, this->usNumOfNoteNumbers, usNoteID);

        //如果在钞票的列表中找不到币种ID为usNoteID的钞票，
        //新增1个节点存储新增加的WFSCIMNOTENUMBER数据，否则，直接赋值。
        if (iIDListIndex == this->usNumOfNoteNumbers)
        {
            LPWFSCIMNOTENUMBER  *lppNoteNumber =
            new LPWFSCIMNOTENUMBER[this->usNumOfNoteNumbers + 1];  //在原来的基础上增加1个节点
            memset(lppNoteNumber, 0, sizeof(LPWFSCIMNOTENUMBER) *
                   (this->usNumOfNoteNumbers + 1));

            //将先前的钞票列表保存到新分配的列表中并释放先前分配的内存空间，
            //然后再将新分配的列表保存到本地成员变量中
            if (this->lppNoteNumber != NULL)
            {
                memcpy(lppNoteNumber, this->lppNoteNumber,
                       sizeof(LPWFSCIMNOTENUMBER) * this->usNumOfNoteNumbers);
                delete[] this->lppNoteNumber;
                this->lppNoteNumber = NULL;
            }
            this->lppNoteNumber = lppNoteNumber;

            //动态生成一个新节点存储新增加的WFSCIMNOTENUMBER数据,
            //并将NoteNumberList中的实际有效个数增加1个
            this->lppNoteNumber[iIDListIndex] = new WFSCIMNOTENUMBER;
            this->lppNoteNumber[iIDListIndex]->ulCount = ulCount;
            this->lppNoteNumber[iIDListIndex]->usNoteID = usNoteID;
            this->usNumOfNoteNumbers++;
        }
        else
        {
            this->lppNoteNumber[iIDListIndex]->ulCount += ulCount;
        }
        return 0;
    }
 //test#13  end

    friend struct CWFSCCIMCashInStatus;
    friend struct CWFSCIMCashInfo;

};

//CCIMCashInResult类
//功能：实现ICIMCashInResult接口，包含CashIn交易中识别和接收的钞票列表
struct CCIMCashInResult : public ICIMCashInResult, public CWFSCIMNoteNumberList
{
    CCIMCashInResult() {}

    virtual ~CCIMCashInResult() {}

    // 设置指定ID的钞数，若ID不存在则自动生成记录。
    virtual long SetCountByID(USHORT usNoteID, ULONG ulCount)
    {
        SetCountByIDEx(usNoteID, ulCount);
        return 0;
    }

    // 累加指定ID的钞数，若ID不存在则自动生成记录。
    virtual long AddCountByID(USHORT usNoteID, ULONG ulCount)   //test#13
    {                                                           //test#13
        AddCountByIDEx(usNoteID, ulCount);                      //test#13
        return 0;                                               //test#13
    }                                                           //test#13

    //如果全部钞票都被拒绝，输出参数为NULL。否则，为存进的钞票列表
    operator LPVOID()
    {
        if (GetNoteNumberList()->usNumOfNoteNumbers == 0)
        {
            return NULL;
        }

        return GetNoteNumberList();
    }
};


struct CWFSCIMCashInfo : public WFSCIMCASHINFO
{
    CWFSCIMCashInfo()
    {
        memset((WFSCIMCASHINFO *)this, 0, sizeof(WFSCIMCASHINFO));
    }
    virtual ~CWFSCIMCashInfo()
    {
        ClearData();
    }

    //释放掉分配的内存并清空本地数据
    void ClearData()
    {
        if (this->lppCashIn != NULL && this->usCount != 0)
        {
            for (int i = 0; i < this->usCount; i++)
            {
                if (this->lppCashIn[i] != NULL)
                {
                    ClearNoteNumberList(i);
                    ClearPhysicalCU(i);
                    if (this->lppCashIn[i]->lpszExtra != NULL)
                    {
                        delete[] this->lppCashIn[i]->lpszExtra;
                        this->lppCashIn[i]->lpszExtra = NULL;
                    }
                    delete this->lppCashIn[i];
                }
            }
            delete[] this->lppCashIn;
            this->lppCashIn = NULL;
        }

        memset((WFSCIMCASHINFO *)this, 0, sizeof(WFSCIMCASHINFO));
    }

protected:
    CWFSCIMNoteNumberList *temp;
    //调用基类CWFSCIMNoteNumberList::SetCountByIDEx
    //设置指定逻辑钞箱的LPWFSCIMNOTENUMBER数据
    //usIndex: 逻辑钞箱列表的下标  usNoteID: 币种ID  ulCount: 该ID的钞票数
    long SetNoteNumberEx(USHORT usIndex, USHORT usNoteID, ULONG ulCount)
    {
        assert((this->lppCashIn[usIndex] != NULL) && (usIndex < this->usCount));
        if (this->lppCashIn[usIndex]->lpNoteNumberList == NULL)
        {
            CWFSCIMNoteNumberList *temp = new CWFSCIMNoteNumberList;
            this->lppCashIn[usIndex]->lpNoteNumberList = temp->GetNoteNumberList();
        }
        ((CWFSCIMNoteNumberList *)this->lppCashIn[usIndex]->lpNoteNumberList)->SetCountByIDEx(usNoteID, ulCount);
        return 0;
    }

    //清除逻辑钞箱num的WFSCIMNOTENUMBERLIST结构
    //num:逻辑钞箱序号
    void ClearNoteNumberList(int num);

    //清除逻辑钞箱num的LPWFSCIMPHCU结构数组
    //num:逻辑钞箱序号
    void ClearPhysicalCU(int num);
};

//CCIMSetUpdatesOfCU类
//功能：实现ICIMSetUpdatesOfCU接口，同时封装WFSCIMCASHINFO数据
//返回结果数据都是指本次交易受影响的增量，即钞箱信息结构中的逻辑钞箱结构数组成员
//只包含有涉及的钞箱，且钞箱内部计数器体现的是增量计数。
struct CCIMSetUpdatesOfCU : public ICIMSetUpdatesOfCU, public CWFSCIMCashInfo, public CLogManage
{
    CCIMSetUpdatesOfCU()
    {
        SetLogFile(LOGFILE, "CCIMSetUpdatesOfCU", "BRM");
    }

    virtual ~CCIMSetUpdatesOfCU() {}

    // 设置原始钞箱数据，初始设置所有钞箱为不引用并将相关计数清零
    //返回值：WFS_ERR_INTERNAL_ERROR 加载原始钞箱数据失败
    //        WFS_SUCCESS            加载原始钞箱数据成功
    virtual long SetSourceInfor(const LPWFSCIMCASHINFO lpCashInfo);

    // 功能：钞箱逻辑和物理一对一，如果多次调用该命令设置同一个钞箱的计数，
    //       每次都重新设置钞箱计数
    // usNumber[in]: 钞箱号(从1开始，以后每个钞箱递增1)
    // ulCashInCount[in]: 进钞数  ulCount[in]:当前张数
    //返回值:WFS_SUCCESS                      成功
    //       WFS_ERR_INTERNAL_ERROR 内部数据错误
    virtual long SetCount(USHORT usNumber, ULONG ulCashInCount, ULONG ulCount);


    // 调用此方法设置相应计数，ID不存在就自动新增,如果多次调用该命令设置同一个钞箱的币种计数，
    //       每次都重新设置钞箱的币种计数
    // usNumber[in]: 钞箱号(从1开始，以后每个钞箱递增1)
    // ulCashInCount[in]: 进钞数  ulCount[in]:当前张数
    //返回值:WFS_SUCCESS                        成功
    //       WFS_ERR_INTERNAL_ERROR 内部数据错误
    virtual long SetNoteNumber(USHORT usNumber, USHORT usNoteID, ULONG ulCount);


    //LPWFSCIMCASHINFO操作符，用于OnExecute中调用SetResultData
    operator LPWFSCIMCASHINFO()
    {
        if (this->lppCashIn == NULL)
        {
            return NULL;
        }

        int iCUIndex = 0;  //钞箱的位置
        while (iCUIndex < this->usCount)
        {
            //钞箱不被引用，则将其删除
            if (this->lppCashIn[iCUIndex] != NULL && this->lppCashIn[iCUIndex]->ulCount == -1)
            {
                delete this->lppCashIn[iCUIndex];
                this->lppCashIn[iCUIndex] = NULL;
                //如果当前钞箱不是最后一个，则后面的钞箱往前移动
                if (iCUIndex != this->usCount - 1)
                {
                    memmove(this->lppCashIn + iCUIndex, this->lppCashIn + iCUIndex + 1, (this->usCount - iCUIndex - 1) * sizeof(LPWFSCIMCASHIN));
                }
                this->usCount--;
            }
            else
            {
                iCUIndex++;  //如果钞箱被引用，则判断下一个钞箱是否被引用
            }
        }

        if (this->usCount == 0)
        {
            return NULL;
        }
        else
        {
            return (LPWFSCIMCASHINFO)this;
        }
    }

private:
    //功能:设置一次CashIn操作中钞票列表(usNoteID--ulCount)
    //lpNoteNumberList:逻辑钞箱iCUNum对应的币种列表; iCUNum:逻辑钞箱号码
    //返回值：WFS_ERR_INTERNAL_ERROR 加载原始钞箱数据失败
    //        WFS_SUCCESS            加载原始钞箱数据成功
    long SetNoteNumberList(const LPWFSCIMNOTENUMBERLIST lpNoteNumberList, const int iCUNum);

    //功能: 设置逻辑钞箱对应的物理钞箱组
    //lppPhysical:物理钞箱数组;  count:物理钞箱的个数; iCUNum:逻辑钞箱的序号
    //返回值：WFS_ERR_INTERNAL_ERROR 加载原始钞箱数据失败
    //        WFS_SUCCESS            加载原始钞箱数据成功
    long SetPhysicalCU(const LPWFSCIMPHCU *lppPhysical, const int count, const int iCUNum);
};

//币种列表辅助类
struct CNoteTypeList : public WFSCIMNOTETYPELIST
{
    CNoteTypeList();
    void Copy(const WFSCIMNOTETYPELIST &src);
    ~CNoteTypeList();
};


//功能:封装WFSCIMCASHINSTATUS数据,该类被CCIMCashInStatus类继承
struct CWFSCCIMCashInStatus : public WFSCIMCASHINSTATUS
{
    CWFSCCIMCashInStatus()
    {
        memset((WFSCIMCASHINSTATUS *)this, 0, sizeof(WFSCIMCASHINSTATUS));
    }
    virtual ~CWFSCCIMCashInStatus()
    {
        ClearData();
    }

    //释放掉分配的内存并清空本地数据
    void ClearData()
    {
        if (this->lpNoteNumberList != NULL)
        {
            delete (CWFSCIMNoteNumberList *)this->lpNoteNumberList;
        }

        memset((WFSCIMCASHINSTATUS *)this, 0, sizeof(WFSCIMCASHINSTATUS));
        m_Extra = NULL;
    }

    long SetCountByIDExtra(USHORT usNoteID, ULONG ulCount)
    {
        if (this->lpNoteNumberList == NULL)
        {
            CWFSCIMNoteNumberList *temp = new CWFSCIMNoteNumberList;
            this->lpNoteNumberList = temp->GetNoteNumberList();
        }
        ((CWFSCIMNoteNumberList *)this->lpNoteNumberList)->SetCountByIDEx(usNoteID, ulCount);
        return 0;
    }

protected:
    CMultiString        m_Extra;  //保存以'\0'分割"\0\0"结束的厂家自定义信息
};

//CCIMCashInStatus类
//功能：实现ICIMCashInStatus接口,包含最后一次CashIn交易的状态信息
struct CCIMCashInStatus : public ICIMCashInStatus, public CWFSCCIMCashInStatus
{
    CCIMCashInStatus() {}

    virtual ~CCIMCashInStatus() {}

    //设置Cash_In交易的状态
    virtual long SetStatus(WORD wStatus)
    {
        this->wStatus = wStatus;
        return 0;
    }

    //设置Cash_In交易期间拒绝的钞票数
    virtual long SetNumOfRefused(USHORT usNumOfRefused)
    {
        this->usNumOfRefused = usNumOfRefused;
        return 0;
    }

    //增加WFSCIMNOTENUMBER节点
    virtual long SetCountByID(USHORT usNoteID, ULONG ulCount)
    {
        SetCountByIDExtra(usNoteID, ulCount);
        return 0;
    }

    //增加一个形式为"Key=Name"的Sub_String,以两个NULL结束
    virtual long AddExtra(LPCSTR lpszKey, LPCSTR lpszValue)
    {
        char szBuf[1024] = { 0 };
        sprintf(szBuf, "%s=%s", lpszKey, lpszValue);
        m_Extra.Add(szBuf);
        this->lpszExtra = (LPSTR)(LPCSTR)m_Extra;
        return 0;
    }
};

