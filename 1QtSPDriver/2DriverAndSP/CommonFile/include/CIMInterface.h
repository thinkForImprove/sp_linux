#ifndef CIM_INTERFACE_H
#define CIM_INTERFACE_H

#include "XFSCIM.H"

//本接口提供返回CIM真实状态的方法，定义同xfs的CIM部分
struct ICIMStatus
{
    //设置CDM的状态
    virtual long SetDeviceSt(WORD fwDevice) = 0;

    //设置安全门的状态
    virtual long SetSafeDoorSt(WORD fwSafeDoor) = 0;

    //设置存款钞箱状态
    virtual long SetAcceptorSt(WORD fwAcceptor) = 0;

    //设置暂存区的状态
    virtual long SetIntermediateStackerSt(WORD fwIntermediateStacker) = 0;

    //通知应用暂存区中的钞票能否被用户取到
    virtual long SetStackerItemsSt(WORD fwStackerItems) = 0;

    //设置验钞机的状态
    virtual long SetBanknoteReaderSt(WORD fwBanknoteReader) = 0;

    //设置Drop Box的状态。
    //bDropBox[in]: TRUE:由于CashIn操作引起的问题导致Drop Box中有钞; FALSE: Drop Box没有钞票
    virtual long SetDropBox(BOOL bDropBox) = 0;

    //添加WFSCIMOUTPOS指针对象到WFSCIMOUTPOS数组中，数组以NULL结束
    //WFSCIMOUTPOS: 钞票送出的位置信息
    virtual long AddPositionSt(const WFSCIMINPOS arryPosition[2]) = 0;

    //是由"key=value"串组成的厂家自定义信息，构成了一个以NULL结尾的子串
    virtual long AddExtraSt(LPCSTR lpszKey, LPCSTR lpszValue) = 0;

};

//本接口提供返回CIM的真实能力集方法，wClass设置为固定值WFS_SERVICE_CLASS_CDM,定义同xfs的CIM部分
struct ICIMCaps
{
    //设置CIM的类型
    virtual long SetType(WORD fwType) = 0;

    //设置单次CashIn操作中最大接收的钞票数
    virtual long SetMaxCashInItems(WORD wMaxCashInItems) = 0;

    ////设置CIM是否是复合设备的一部分
    virtual long SetCompound(BOOL bCompound) = 0;

    //设置是否支持WFS_CMD_CIM_OPEN_SHUTTER、WFS_CMD_CIM_CLOSE_SHUTTER命令
    virtual long SetShutter(BOOL bShutter) = 0;

    //设置是否隐性支持shutter门控制
    //TRUE:SP隐性控制shutter门开关
    //FALSE:由应用程序调用WFS_CMD_CIM_OPEN_SHUTTER、WFS_CMD_CIM_CLOSE_SHUTTER控制
    virtual long SetShutterControl(BOOL bShutterControl) = 0;

    //设置是否支持WFS_CMD_CIM_OPEN_OPEN_SAFE_DOOR
    virtual long SetSafeDoor(BOOL bSafeDoor) = 0;

    //该域仅适用于CIM类型为WFS_CIM_TELLERBILL或者WFS_CIM_TELLERCOIN.
    virtual long SetCashBox(BOOL bCashBox) = 0;

    virtual long SetRefill(BOOL bRefill) = 0;

    //设定在CashIn操作中暂存区的最大钞票张数。
    //fwIntermediateStacker为0，则暂存区在CashIn操作中不能用
    virtual long SetIntermediateStacker(WORD fwIntermediateStacker) = 0;

    //设置CIM是否能检查到CS中的钞票被拿走的能力
    //TRUE: 事件WFS_SRVE_CIM_ITEM_TAKEN事件产生，FALSE:不产生该服务事件
    virtual long SetItemsTakenSensor(BOOL bItemsTakenSensor) = 0;

    //设置CIM是否能检查到用户在进钞口放钞
    //TRUE: 事件WFS_SRVE_CIM_ITEMINSERTED事件产生，FALSE:不产生该服务事件
    virtual long SetItemsInsertedSensor(BOOL bItemsInsertedSensor) = 0;

    //设置CIM 可用的OutPut Position和InPut Position
    virtual long SetPositions(WORD fwPositions) = 0;

    //设置CIM支持钞箱交换操作的类型，该值可以是WFS_CIM_EXBYHAND、WFS_CIM_EXTOCASSETTES等的组合
    virtual long SetExchangeType(WORD fwExchangeType) = 0;

    //设置钞票的回收区域
    virtual long SetRetractAreas(WORD fwRetractAreas) = 0;

    //设置通道中的钞票能执行的动作
    virtual long SetRetractTransportActions(WORD fwRetractTransportActions) = 0;

    //设置暂存区中的钞票能执行的动作
    virtual long SetRetractStackerActions(WORD fwRetractStackerActions) = 0;

    //是由"key=value"串组成的厂家自定义信息，构成了一个以NULL结尾的子串
    virtual long AddExtraCp(LPCSTR lpszKey, LPCSTR lpszValue) = 0;
};

struct ICIMCurrencyExp
{
    //设置每一种币种对应的指数
    virtual long AddCurrencyExp(const WFSCIMCURRENCYEXP &Exp) = 0;
};

struct ICIMBanknoteTypes
{
    //获取BV能检测到的币种类型
    virtual long AddBanknoteTypes(const LPWFSCIMNOTETYPE lpNoteType) = 0;
};

//本接口提供设置CashIn时的进钞信息
struct ICIMCashInStatus
{
    //设置Cash_In交易的状态
    virtual long SetStatus(WORD wStatus) = 0;

    //设置Cash_In交易期间拒绝的钞票数
    virtual long SetNumOfRefused(USHORT usNumOfRefused) = 0;

    //增加WFSCIMNOTENUMBER节点
    virtual long SetCountByID(USHORT usNoteID, ULONG ulCount) = 0;

    //增加一个形式为"Key=Name"的Sub_String,以NULL结束
    virtual long AddExtra(LPCSTR lpszKey, LPCSTR lpszValue) = 0;
};

struct ICIMCashInResult
{
    // 设置指定ID的钞数，若ID不存在则自动生成记录。
    virtual long SetCountByID(USHORT usNoteID, ULONG ulCount) = 0;
};

//本命令的返回结果数据都是指本次交易受影响的增量，即钞箱信息结构中的逻辑钞箱结构数组成员
//只包含有涉及的钞箱，且钞箱内部计数器体现的是增量计数。
struct ICIMSetUpdatesOfCU
{
    // 设置原始钞箱数据
    virtual long SetSourceInfor(const LPWFSCIMCASHINFO lpCashInfo) = 0;

    // 钞箱逻辑和物理一对一，所以只需设置一组，内部会先将所有计数器清零
    virtual long SetCount(USHORT usNumber, ULONG ulCashInCount, ULONG ulCount) = 0;

    // 内部先将原始钞箱数据中的列表计数清零，调用此方法设置相应计数，ID不存在就自动新增
    virtual long SetNoteNumber(USHORT usNumber, USHORT usNoteID, ULONG ulCount) = 0;
};

struct ICIMItemInfo
{
    //设置钞票系列号信息
    virtual BOOL SetSerialNumber(USHORT usNoteID, LPSTR lpszSerialNumber) = 0;
};
#endif //CIM_INTERFACE_H
