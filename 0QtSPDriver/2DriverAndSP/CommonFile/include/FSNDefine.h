#pragma once

typedef unsigned short Uint16;
typedef unsigned int     Uint32;
/* be aware of alignment */
#pragma pack (push, 1)

//FSN文件结构定义
//假币特征码
enum ERRCODE
{
    ERRCODE_0,//表示真币
    ERRCODE_1,
    ERRCODE_2,
    ERRCODE_3,
    ERRCODE_4,
    ERRCODE_5,
    ERRCODE_6,
    ERRCODE_7,
    ERRCODE_8,
    ERRCODE_9,
    ERRCODE_10,
    ERRCODE_11,
    ERRCODE_12,
};

typedef struct
{
    Uint16 HeadStart[4];
    Uint16 HeadString[6];
    Uint32 Counter;
    Uint16 HeadEnd[4];
} STFSNHEAD, *LPSTFSNHEAD;

typedef struct
{
    Uint32 Data[32];
} TImgSNoData;

typedef struct
{
    Uint16 Num;
    Uint16 height, width;
    Uint16 Reserve3;
    TImgSNoData SNo[12];
} TImageSNo;

typedef struct
{
    Uint16 Date;
    Uint16 Time;
    Uint16 tfFlag;
    Uint16 ErrorCode[3];
    Uint16 MoneyFlag[4];
    Uint16 Ver;
    Uint16 Valuta;
    Uint16 CharNUM;
    Uint16 SNo[12];
    Uint16 MachineSNo[24];
    Uint16 Reserve1;
    //Uint16 Reserve2; 注意此项可能和别的银行不一样
    TImageSNo ImageSNo;
} STFSNBody, *LPSTFSNBody;

typedef struct _fsnfiledata
{
    STFSNHEAD stFSNHead;
    LPSTFSNBody *pstFSNBody;
} FSNFileData, *LPFSNFileData;

/* restore alignment */
#pragma pack (pop)
