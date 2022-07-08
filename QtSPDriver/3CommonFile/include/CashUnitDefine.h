#pragma once
#include "XFSCIM.H"
#include "XFSCDM.H"

enum ADP_CASSETTE_TYPE
{
    ADP_CASSETTE_BILL      = 0,
    ADP_CASSETTE_CASHIN    = 1,
    ADP_CASSETTE_RECYCLING = 2,
    ADP_CASSETTE_RETRACT   = 3,
    ADP_CASSETTE_REJECT    = 4,
    ADP_CASSETTE_UNKNOWN   = 5,
};

enum CASHUNIT_STATUS
{
    ADP_CASHUNIT_OK       =  0,
    ADP_CASHUNIT_FULL     =  1,
    ADP_CASHUNIT_HIGH     =  2,
    ADP_CASHUNIT_LOW      =  3,
    ADP_CASHUNIT_EMPTY    =  4,
    ADP_CASHUNIT_INOP     =  5,
    ADP_CASHUNIT_MISSING  =  6,
    ADP_CASHUNIT_MANIP    =  9,
    ADP_CASHUNIT_UNKNOWN  = -1,
};

//转换XFS钞箱类型为本地类型
//转换失败时返回ADP_CASSETTE_UNKNOWN
inline ADP_CASSETTE_TYPE CassTypeXFS2Local(unsigned long dwType, bool bCDM)
{
    if (bCDM)
    {
        switch (dwType)
        {
        case WFS_CDM_TYPEREJECTCASSETTE:
            return ADP_CASSETTE_REJECT;

        case WFS_CDM_TYPEBILLCASSETTE:
            return ADP_CASSETTE_BILL;

        case WFS_CDM_TYPERETRACTCASSETTE:
            return ADP_CASSETTE_RETRACT;

        case WFS_CDM_TYPERECYCLING:
            return ADP_CASSETTE_RECYCLING;
        case WFS_CDM_TYPECOINCYLINDER:
        case WFS_CDM_TYPECOINDISPENSER:
        case WFS_CDM_TYPECOUPON:
        case WFS_CDM_TYPEDOCUMENT:
        case WFS_CDM_TYPEREPCONTAINER:
        default:
            return ADP_CASSETTE_UNKNOWN;
        }
    }
    else
    {
        switch (dwType)
        {
        case WFS_CIM_TYPERECYCLING:
            return ADP_CASSETTE_RECYCLING;
        case WFS_CIM_TYPECASHIN:
            return ADP_CASSETTE_CASHIN;
        case WFS_CIM_TYPERETRACTCASSETTE:
            return ADP_CASSETTE_RETRACT;

        default:
            return ADP_CASSETTE_UNKNOWN;
        }
    }
}

//转换本地钞箱类型为XFS类型，失败时返回0
inline unsigned long CassTypeLocal2XFS(ADP_CASSETTE_TYPE eType, bool bCDM)
{
    if (bCDM)
    {
        switch (eType)
        {
        case ADP_CASSETTE_REJECT:
            return WFS_CDM_TYPEREJECTCASSETTE;

        case ADP_CASSETTE_BILL:
            return WFS_CDM_TYPEBILLCASSETTE;

        case ADP_CASSETTE_RETRACT:
            return WFS_CDM_TYPERETRACTCASSETTE;

        case ADP_CASSETTE_RECYCLING:
            return WFS_CDM_TYPERECYCLING;

        default:
            return 0;
        }
    }
    else
    {
        switch (eType)
        {
        case ADP_CASSETTE_RECYCLING:
            return WFS_CIM_TYPERECYCLING;

        case ADP_CASSETTE_CASHIN:
            return WFS_CIM_TYPECASHIN;

        case ADP_CASSETTE_RETRACT:
            return WFS_CIM_TYPERETRACTCASSETTE;

        default:
            return 0;
        }
    }
}

//转换XFS钞箱状态为本地状态
inline CASHUNIT_STATUS StatusXFS2Local(unsigned short usStatus)
{
    return (CASHUNIT_STATUS)usStatus;
}

//转换本地钞箱状态为XFS钞箱状态
inline unsigned short StatusLocal2XFS(CASHUNIT_STATUS eStatus)
{
    return (unsigned short)eStatus;
}


