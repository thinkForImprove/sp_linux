#ifndef CDM_INTERFACE_H
#define CDM_INTERFACE_H

#include "XFSCDM.H"

struct ICDMDenomination;

//本接口提供返回出钞机真实状态的方法，定义同xfs的CDM部分
struct  ICDMStatus
{
    //设置CDM的状态，其值为WFS_CDM_DEVONLINE时，并不意味着能出钞，
    //还需考虑fwDispenser甚至fwSafeDoor;
    virtual long SetDeviceSt(WORD fwDevice) = 0;
    //设置安全门的状态
    virtual long SetSafeDoorSt(WORD fwSafeDoor) = 0;
    //设置出钞的逻辑钞箱状态
    virtual long SetDispenserSt(WORD fwDispenser) = 0;
    //设置TS的状态
    virtual long SetIntermediateStackerSt(WORD fwIntermediateStacker) = 0;
    //添加WFSCDMOUTPOS指针对象到WFSCDMOUTPOS数组中，数组以NULL结束
    //WFSCDMOUTPOS 钞票送出的位置信息
    virtual long AddPositionSt(const WFSCDMOUTPOS &Position) = 0;
    //是由"key=value"串组成的厂家自定义信息，构成了一个以NULL结尾的子串
    virtual long AddExtraSt(LPCSTR lpszKey, LPCSTR lpszValue) = 0;
};

//本接口提供返回出钞机的真实能力集方法，wClass设置为固定值WFS_SERVICE_CLASS_CDM,定义同xfs的CDM部分
struct ICDMCaps
{
    //设置CDM的类型
    virtual long SetType(WORD fwType) = 0;
    //设置单次出钞的最大张数
    virtual long SetMaxDispenseItems(WORD wMaxDispenseItems) = 0;
    //设置CDM是否是复合设备的一部分
    virtual long SetCompound(BOOL bCompound) = 0;
    //设置是否支持WFS_CMD_CDM_OPEN_SHUTTER、WFS_CMD_CDM_CLOSE_SHUTTER命令
    virtual long SetShutter(BOOL bShutter) = 0;
    //设置是否隐性支持shutter门控制
    //TRUE:SP隐性控制shutter门开关
    //FALSE:由应用程序调用WFS_CMD_CDM_OPEN_SHUTTER、WFS_CMD_CDM_CLOSE_SHUTTER控制
    virtual long SetShutterControl(BOOL bShutterControl) = 0;
    //指定钞票的回收区域，该值可以是WFS_CDM_RA_RETRACT、WFS_CDM_RA_TRANSPORT、
    //WFS_CDM_RA_STACKER、WFS_CDM_RA_REJECT的组合或者是WFS_CDM_RA_NOTSUPP
    virtual long SetRetractAreas(WORD fwRetractAreas) = 0;
    //设置已回收到通道中的钞票能执行的动作，该值可以是WFS_CDM_PRESENT、WFS_CDM_RETRACT、
    //WFS_CDM_REJECT的组合或者是WFS_CDM_NOTSUPP
    virtual long SetRetractTransportActions(WORD fwRetractTransportActions) = 0;
    //设置已回收到TS中的钞票能执行的动作,该值取值和SetRetractAreas一样
    virtual long SetRetractStackerActions(WORD fwRetractStackerActions) = 0;
    //设置是否支持WFS_CMD_CDM_OPEN_SAFE_DOOR命令
    virtual long SetSafeDoor(BOOL bSafeDoor) = 0;
    //该域仅适用于CDM类型为WFS_CDM_TELLERBILL或者WFS_CDM_TELLERCOIN.
    virtual long SetCashBox(BOOL bCashBox) = 0;
    //设置TS是否启用。
    //TRUE:启用TS，则DISPENSE命令中的bPresent可以设置为FALSE
    virtual long SetIntermediateStacker(BOOL bIntermediateStacker) = 0;
    //设置CDM是否能检查道CS中的钞票被拿走的能力
    //TRUE: 事件WFS_SRVE_CDM_ITEM_TAKEN事件产生，FALSE:不产生该服务事件
    virtual long SetItemsTakenSensor(BOOL bItemsTakenSensor) = 0;
    //设置CDM 可用的OutPut Position
    virtual long SetPositions(WORD fwPositions) = 0;
    //设置CDM移动钞票的能力，该值可以是WFS_CDM_FROMCU、WFS_CDM_TOCU、WFS_CDM_TOTRANSPORT的组合
    virtual long SetMoveItems(WORD fwMoveItems) = 0;
    //设置CDM支持钞箱交换操作的类型，该值可以是WFS_CDM_EXBYHAND、WFS_CDM_EXTOCASSETTES的组合
    virtual long SetExchangeType(WORD fwExchangeType) = 0;
    //是由"key=value"串组成的厂家自定义信息，构成了一个以NULL结尾的子串
    virtual long AddExtraCp(LPCSTR lpszKey, LPCSTR lpszValue) = 0;
};

//本接口用来设置最近一次送钞给用户的配钞算法，该值直到下一次尝试送钞给用户时才会改变
struct ICDMPresentStatus
{
    //设置每一个钞箱在本次交易中出了多少张钞票
    virtual long SetDenomination(const LPWFSCDMDENOMINATION pDeno) = 0;
    //设置是否已经将钞票送给用户
    virtual long SetPresentState(WORD wPresentState) = 0;
    virtual long AddExtra(LPCSTR lpszKey, LPCSTR lpszValue) = 0;
};

//本接口用来设置配钞或者出钞时，每个钞箱需要出多少张钞票来满足此次出钞的金额
struct ICDMDenomination
{
    //SetCurrency、GetCurrency分别为设置和获取币种的ID，例如"CNY",如果是多币种，则为3个0x20
    virtual void   SetCurrency(char cCurrencyID[3]) = 0;
    virtual const char *GetCurrency() const = 0;
    //SetAmount、GetAmount分别为设置和获取出钞或配钞时的总额，如果是多币种，该值为0
    virtual void   SetAmount(ULONG Amount) = 0;
    virtual ULONG  GetAmount() const = 0;
    //只适用于Teller CDM设备
    virtual void   SetCashBox(ULONG CashBox) = 0;
    virtual ULONG  GetCashBox() const = 0;
    //设置各个钞箱的出钞张数
    //Count为数组Values的实际大小，Values是各个钞箱的出钞张数
    virtual long   SetValues(USHORT Count, LPULONG Values) = 0;
    //功能：获取所有钞箱出钞数数组
    //Count[out]: 返回数组大小
    //返回值: 所有钞箱出钞数数组
    virtual const LPULONG GetValues(USHORT &Count) const = 0;
};

//本接口用来设置配钞或者出钞时，每个钞箱需要出多少张钞票来满足此次出钞的金额(取自CSCB分支)
struct ICDMDenominationTemp   //test#13 start
{
    //SetCurrency、GetCurrency分别为设置和获取币种的ID，例如"CNY",如果是多币种，则为3个0x20
    virtual void   SetCurrency(char cCurrencyID[3]) = 0;
    virtual const char *GetCurrency() const = 0;
    //SetAmount、GetAmount分别为设置和获取出钞或配钞时的总额，如果是多币种，该值为0
    virtual void   SetAmount(ULONG Amount) = 0;
    virtual ULONG  GetAmount() const = 0;
    //只适用于Teller CDM设备
    virtual void   SetCashBox(ULONG CashBox) = 0;
    virtual ULONG  GetCashBox() const = 0;
    //设置各个钞箱的出钞张数
    //Count为数组Values的实际大小，Values是各个钞箱的出钞张数
    virtual long   SetValues(USHORT Count, LPULONG Values) = 0;
    //功能：获取所有钞箱出钞数数组
    //Count[out]: 返回数组大小
    //返回值: 所有钞箱出钞数数组
    virtual const LPULONG GetValues(USHORT &Count) const = 0;
};
//test#13 end

#endif //CDM_INTERFACE_H
