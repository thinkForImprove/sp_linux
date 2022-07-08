#pragma once
#include <assert.h>
#include "XFSCDM.H"
#include "XFSCIM.H"
#include "ILogWrite.h"
#include "QtTypeDef.h"


#define THISFILE                "CashUnitManager"
#define DEFMODULE(FuncName)     const char *ThisModule = #FuncName


//设置数据改变并且数据脏了
#define SET_CHANGED_DIRTY()     SetItemChanged(); SetDirty()

//-----------  以下类型、函数和宏协助设置字段的值 -------------------------------------------
enum SETITEMTYPE
{
    SIT_IN_EXCHANGE = 0,    //设置必须在交换状态设置的变量
    SIT_NOT_IN_EXCHANGE = 2,//设置可以在非交换设置的变量，如Count
    SIT_NOT_IN_XFS = 3      //设置不允许在SetByXFSFormat函数中设置的变量，如STATUS
};

//测试是否允许设置ITEM
//允许设置返回TRUE，否则返回FALSE
//pThis：钞箱对象
//sit：设置类型:SIT_IN_EXCHANGE、SIT_NOT_IN_EXCHANGE、SIT_NOT_IN_XFS
template <class ClassType>
BOOL SetItemIsAllowed(ClassType *pThis, SETITEMTYPE sit)
{
    if (pThis->IsInSetByXFSFormat())    //如果目前正在SetByXFSFormat函数调用过程中
    {
        if (sit == SIT_NOT_IN_XFS)      //该变量不能在SetByXFSFormat函数中调用
        {
            return FALSE;
        }
        else if (sit == SIT_IN_EXCHANGE)//如果该变量仅能在交换状态设置
        {
            if (!pThis->GetExchangeState()) //如果不在交换状态，忽略它
                return FALSE;
        }
        else    //如果该变量可以在非交换状态设置
        {
            assert(sit == SIT_NOT_IN_EXCHANGE);
        }
    }
    else    //如果不在SetByXFSFormat函数调用过程中，可以设置任何变量
    {
    }
    return TRUE;
}

//设置字段，如果不允许设置，直接返回0
//pThis：钞箱对象
//arg：参数
//ThisValue：钞箱对象对应的值
//sit：设置类型:SIT_IN_EXCHANGE、SIT_NOT_IN_EXCHANGE、SIT_NOT_IN_XFS
template <class ClassType, class DataType>
long SetItem(ClassType *pThis, DataType arg, DataType &ThisValue, SETITEMTYPE sit)
{
    if (!SetItemIsAllowed(pThis, sit))
        return 0;

    if (arg != ThisValue)
    {
        pThis->SetItemChanged();
        pThis->SetDirty();
        ThisValue = arg;
    }
    return 0;
}

//设置必须在交换状态设置的变量，如Value、Type等
#define SETITEM_IN_EXCHANGE(value)      return SetItem(this, value, this->m_##value, SIT_IN_EXCHANGE)
//设置可以在非交换设置的变量，如Count
#define SETITEM_NOT_IN_EXCHANGE(value)  return SetItem(this, value, this->m_##value, SIT_NOT_IN_EXCHANGE)
//设置不允许在SetByXFSFormat函数中设置的变量，如STATUS、Sensor
#define SETITEM_NOT_IN_XFS(value)       return SetItem(this, value, this->m_##value, SIT_NOT_IN_XFS)


// ----------------- 以下宏辅助校验数据 ------------------------------------------
#define VERIFY_SET_ITEM_ALLOWED()   if (!SetItemIsAllowed(this, SIT_IN_EXCHANGE)) return 0

//协助校验函数调用结果
#define VERIFY_FUNC_CALL(v) \
    {\
        long lRet = (v); \
        if (lRet < 0) \
        {\
            Log(ThisModule, -1, #v " failed(%d)", lRet);\
            return lRet; \
        }\
    }

//校验数据是否需要重写入文件
#define VERIFY_DIRTY() if (!IsDirty()) return 0

//校验数据是否在tMin和tMax之前
template <class TYPE>
long VerifyDataMinMax(TYPE tValue, long tMin, long tMax)
{
    DEFMODULE(VerifyDataMinMax);
    long lRet = (tValue < (TYPE)tMin || tValue > (TYPE)tMax) ? WFS_ERR_INVALID_DATA : 0;
    if (lRet < 0)
    {
        const char *pDecFormat;
        if (sizeof(TYPE) == 2)
        {
            pDecFormat = "%hd";
        }
        else
        {
            assert(sizeof(TYPE) == 4);
            pDecFormat = "%ld";
        }
        char szTemp[20];
        sprintf(szTemp, pDecFormat, tValue);
        //log_write(LOGFILE, THISFILE, ThisModule, -1, "%s not in [%ld-%ld]", szTemp, tMin, tMax);

    }
    return lRet;
}

//检验数据是否在后续的可变列表中，可变列表以-1结束
template <class TYPE>
long VerifyData(TYPE tValue, ...)
{
    DEFMODULE(VerifyData);

    va_list vl;
    va_start(vl, tValue);

    const char *pDecFormat;
    if (sizeof(TYPE) == 2)
    {
        pDecFormat = "%hd";
    }
    else
    {
        //assert(sizeof(TYPE) == 4);
        pDecFormat = "%ld";
    }
    char szBuf[1024];
    sprintf(szBuf, pDecFormat, tValue);
    strcat(szBuf, " not in (");

    TYPE tArg;
    while (true)
    {
        tArg = va_arg(vl, TYPE);
        if (tArg == -1)
            break;
        if (tArg == tValue)
            return 0;
        char szTemp[20];
        sprintf(szTemp, pDecFormat, tArg);
        strcat(szBuf, szTemp);
        strcat(szBuf, ",");
    }

    strcat(szBuf, ")");
    //log_write(LOGFILE, THISFILE, ThisModule, -1, szBuf);

    return WFS_ERR_INVALID_DATA;
}

// --------------------- 以下函数在XFS类型和配置文件类型之间转换 --------------
//全局辅助函数
//转换CMD的XFS钞箱类型为配置文件类型
inline USHORT CDM_CassType_XFS2Config(USHORT usXFSCassType)
{
    switch (usXFSCassType)
    {
    case WFS_CDM_TYPEREJECTCASSETTE: return 4;
    case WFS_CDM_TYPEBILLCASSETTE: return 0;
    case WFS_CDM_TYPERETRACTCASSETTE: return 3;
    case WFS_CDM_TYPERECYCLING: return 2;
    case WFS_CDM_TYPEREPCONTAINER:
    case WFS_CDM_TYPECOUPON:
    case WFS_CDM_TYPEDOCUMENT:
    case WFS_CDM_TYPECOINDISPENSER:
    case WFS_CDM_TYPECOINCYLINDER:
    case WFS_CDM_TYPENA:
    default:
        return -1;
    }
}

//转换CMD的配置文件类型为XFS钞箱类型
inline USHORT CDM_CassType_Config2XFS(USHORT usConfigCassType)
{
    switch (usConfigCassType)
    {
    case 4: return WFS_CDM_TYPEREJECTCASSETTE;
    case 0: return WFS_CDM_TYPEBILLCASSETTE;
    case 3: return WFS_CDM_TYPERETRACTCASSETTE;
    case 2: return WFS_CDM_TYPERECYCLING;
    default:
        return -1;
    }
}

//转换CMD的XFS钞箱类型为配置文件类型
inline DWORD CIM_CassType_XFS2Config(DWORD dwXFSType)
{
    switch (dwXFSType)
    {
    case WFS_CIM_TYPERECYCLING: return 2;
    case WFS_CIM_TYPECASHIN: return 1;
    case WFS_CIM_TYPERETRACTCASSETTE: return 3;
    case WFS_CIM_TYPEREPCONTAINER:
    default:
        return -1;
    }
}

//转换CMD的配置文件类型为XFS钞箱类型
inline DWORD CIM_CassType_Config2XFS(DWORD dwConfigCassType)
{
    switch (dwConfigCassType)
    {
    case 1: return WFS_CIM_TYPECASHIN;
    case 2: return WFS_CIM_TYPERECYCLING;
    case 3: return WFS_CIM_TYPERETRACTCASSETTE;
    default:
        return -1;
    }
}

inline USHORT CassState_Config2XFS(USHORT usState)
{
    return usState;
}

inline USHORT CassState_XFS2Config(USHORT usState)
{
    if (usState == WFS_CIM_STATCUNOVAL ||
        usState == WFS_CIM_STATCUNOREF)
        return WFS_CIM_STATCUINOP;
    return usState;
}

