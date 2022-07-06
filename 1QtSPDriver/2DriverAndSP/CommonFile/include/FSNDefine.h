#pragma once

typedef unsigned short Uint16;
typedef unsigned int   Uint32;
typedef unsigned long  Uint64;
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

//---------------------------FSN 头部----------------------
typedef struct
{
    Uint16 HeadStart[4];
    Uint16 HeadString[6];
    Uint32 Counter;
    Uint16 HeadEnd[4];
} STFSNHEAD, *LPSTFSNHEAD;
//----------------------------2018 FSN 头部--------------------------------
//30-00-00-00(FS#0001) start
typedef struct
{
    Uint16 HeadStart[4];         //文件头开始标识
    Uint16 HeadString[6];        //文件类型标识
    Uint16 HeadDataTypeFlag;     //机具信息记录格式标识
    Uint32 Counter;              //冠字号码记录数量
    Uint16 FinanIns[6];          //金融机构缩写
    Uint32 Enabletime;           //设备启动时间
    Uint16 MachineSNo[24];       //机具编号
    Uint16 MachineType[15];      //机具类型
    Uint16 MachineModel[24];     //机具型号
    Uint16 HardVerNo[24];        //硬件版本号
    Uint16 AuthSoftVerNo[24];    //软件版本号
    Uint16 Applidenom[6];        //适用卷别
    Uint16 FinanInst[14];        //报送银行
    Uint16 FinanInstOutlet[14];  //报送网点
    Uint16 Operator[4];          //操作人员
    Uint16 Reserve1;             //保留字
    Uint16 HeadEnd[4];           //文件头结束标志
} STFSNHEAD18, *LPSTFSNHEAD18;
//30-00-00-00(FS#0001) end

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
    Uint16 Date;                    //验钞启动日期
    Uint16 Time;                    //验钞启动时间
    Uint16 tfFlag;                  //钞币标识(0:假，1:真，2:残，3:旧)
    Uint16 ErrorCode[3];            //错误码
    Uint16 MoneyFlag[4];            //币种标识
    Uint16 Ver;                     //版本号
    Uint16 Valuta;                  //币值
    Uint16 CharNUM;                 //冠字号码字符数
    Uint16 SNo[12];                 //冠字号码
    Uint16 MachineSNo[24];
    Uint16 Reserve1;
    //Uint16 Reserve2; 注意此项可能和别的银行不一样
    TImageSNo ImageSNo;
} STFSNBody, *LPSTFSNBody;

typedef struct
{
    Uint16 Date;                    //验钞启动日期
    Uint16 Time;                    //验钞启动时间
    Uint16 tfFlag;                  //钞币标识(0:假，1:真，2:残，3:旧)
    Uint16 ErrorCode[3];            //错误码
    Uint16 MoneyFlag[4];            //币种标识
    Uint16 Ver;                     //版本号
    Uint64 Valuta;                  //币值
    Uint16 CharNUM;                 //冠字号码字符数
    Uint16 SNo[12];                 //冠字号码
    Uint16 Reserve2[16];
    TImageSNo ImageSNo;
} STFSNBody18, *LPSTFSNBody18;

typedef struct _fsnfiledata
{
    union {                             //30-00-00-00(FS#0001)
        STFSNHEAD   stFSNHead;          //30-00-00-00(FS#0001)
        STFSNHEAD18 stFSNHead18;        //30-00-00-00(FS#0001)
    } fsnHead;                          //30-00-00-00(FS#0001)
    union {
        LPSTFSNBody *pstFSNBody;
        LPSTFSNBody18 *pstFSNBody18;
    } fsnBody;
} FSNFileData, *LPFSNFileData;
/* restore alignment */
#pragma pack (pop)
