#pragma once

#include "stdlib.h"
#include "FSNDefine.h"
#include "XFSAPI.H"
#include "QtTypeDef.h"
#include "IBRMAdapter.h"
#include <QtCore/qglobal.h>
#include <string.h>
//保存序列号的操作
enum EnumSaveSerialOperation
{
    //dispense()
    SAVESERIALOPERAT_D,
    //StoreCash()
    SAVESERIALOPERAT_S,
    //验钞
    SAVESERIALOPERAT_C,
    //回收
    SAVESERIALOPERAT_R,
    //ROLLBACK
    SAVESERIALOPERAT_B,
    //REJECT
    SAVESERIALOPERAT_J,
    //自动精查
    SAVESERIALOPERAT_SELFCOUNT          //30-00-00-00(FS#0022)
};

enum IMAGETYPE
{
    IMAGETYPE_BMP = 0,
    IMAGETYPE_JPG = 1,
};

enum EnumBRMDENOMINATION_CODE
{
    BRMDENOMINATION_CODE_00,            // 未指定
    BRMDENOMINATION_CODE_100_C = 0x04,
    BRMDENOMINATION_CODE_20_C = 0x05,
    BRMDENOMINATION_CODE_10_C  = 0x06,
    BRMDENOMINATION_CODE_50_C  = 0x07,
    BRMDENOMINATION_CODE_1_B   = 0x08,
    BRMDENOMINATION_CODE_5_C   = 0x09,
    BRMDENOMINATION_CODE_10_B  = 0x0A,
    BRMDENOMINATION_CODE_20_B  = 0x0B,
    BRMDENOMINATION_CODE_50_B    = 0x0C,
    BRMDENOMINATION_CODE_100_D  = 0x0D,
    BRMDENOMINATION_CODE_100_B   = 0x0E,
    BRMDENOMINATION_CODE_5_B     = 0x0F,
    BRMDENOMINATION_CODE_10_D   = 0x10,
    BRMDENOMINATION_CODE_20_D   = 0x11,
    BRMDENOMINATION_CODE_50_D   = 0x12,
    BRMDENOMINATION_CODE_1_D    = 0x13,
    BRMDENOMINATION_CODE_5_D    = 0x14,             //30-00-00-00(FS#0018)
    BRMDENOMINATION_CODE_ALL       = 0xFF,
};   

enum EnumNoteEdition
{
    NoteEdition_1999  = 0,
    NoteEdition_2005  = 1,
    NoteEdition_2015  = 2,
    NoteEdition_2019  = 2,
    NoteEdition_unknown = 4
};

enum EnumNOTECATEGORY
{
    NOTECATEGORY_UNKNOWN = 0,
    NOTECATEGORY_1,                 //Cat.1
    NOTECATEGORY_2,                 //Cat.2
    NOTECATEGORY_3,                 //Cat.3
    NOTECATEGORY_4,                 //Cat.4
    NOTECATEGORY_4B,                //Cat.4B
    NOTECATEGORY_ALL
};

enum EnumREJECTCAUSE
{
    REJECT_CAUSE_RESERVED          = 0x00,
    REJECT_CAUSE_SHIFT             = 0x01,
    REJECT_CAUSE_SKEW              = 0x02,
    REJECT_CAUSE_EXSKEW            = 0x04,
    REJECT_CAUSE_LONG              = 0x05,
    REJECT_CAUSE_SPACING           = 0x07,
    REJECT_CAUSE_INTERVAL          = 0x08,
    REJECT_CAUSE_DOUBLE            = 0x09,
    REJECT_CAUSE_DIMENSTION_ERR    = 0x0A,
    REJECT_CAUSE_DENO_UNIDENTIFIED = 0x0B,
    REJECT_CAUSE_VERIFICATION      = 0x0C,
    REJECT_CAUSE_UNFIT             = 0x0D,
    REJECT_CAUSE_BV_OTHERS         = 0x10,
    REJECT_CAUSE_DIFF_DENO         = 0x16,
    REJECT_CAUSE_EXCESS            = 0x18,
    REJECT_CAUSE_FACTOR1           = 0x19,
    REJECT_CAUSE_FACTOR2           = 0x1A,
    REJECT_CAUSE_NO_VALIDATION     = 0x1B,
    REJECT_CAUSE_BV_FORMAT_ERR     = 0x1C,
    REJECT_CAUSE_OTHER             = 0xFF,
};

enum EnumCASSETTEROOMID
{

    CASS_ROOM_ID_1   = 1,
    CASS_ROOM_ID_2   = 2,
    CASS_ROOM_ID_3   = 3,
    CASS_ROOM_ID_4   = 4,
    CASS_ROOM_ID_5   = 5,
    CASS_ROOM_ID_6   = 6,
    CASS_ROOM_ID_7   = 7,
    CASS_ROOM_ID_8   = 8,
    CASS_ROOM_ID_9   = 9,
    CASS_ROOM_ID_10  = 10,
    CASS_ROOM_ID_11  = 11,
    CASS_ROOM_ID_12  = 12,
    CASS_ROOM_ID_13  = 13,
    CASS_ROOM_ID_14  = 14,
    CASS_ROOM_ID_15  = 15,
    CASS_ROOM_ID_16  = 16,
    CASS_ROOM_ID_CS       = 20,
    CASS_ROOM_ID_ESC      = 21,
    CASS_ROOM_ID_URJB     = 22,
    CASS_ROOM_ID_RESERVED  = 0xFF,
};

typedef struct _sn_info_details
{
    SYSTEMTIME sSTime;                               //当前时间
    EnumSaveSerialOperation OperCmd;                 //操作类型
    DWORD uIndex;                                    //图片序号
    char         szCurrencyID[64];                   //币种
    EnumBRMDENOMINATION_CODE     DnoCode;               //版本面额
    EnumNoteEdition          NoteEdition;            //出版年别
    EnumNOTECATEGORY         NoteCateGory;           //钞票类别，判断真假钞
    EnumREJECTCAUSE             RejectCause;         //拒钞原因
    EnumCASSETTEROOMID            SourCass;          //纸币起始位置
    EnumCASSETTEROOMID            DestCass;          //操作纸币目的位置
    char       szSerialNO[128];                      //冠字码
    char       szSNFullPath[MAX_PATH];              //冠子码图像保存全路径
    char       szImgAFullPath[MAX_PATH];            //全副图像正面保存全路径
    char       szImgBFullPath[MAX_PATH];            //全副图像反面保存全路径
    TImageSNo ImageSNo;
    _sn_info_details()
    {
        memset(szCurrencyID, 0, sizeof(szCurrencyID));
        memset(szSerialNO, 0, sizeof(szSerialNO));
        memset(szSNFullPath, 0, sizeof(szSNFullPath));
        memset(szImgAFullPath, 0, sizeof(szImgAFullPath));
        memset(szImgBFullPath, 0, sizeof(szImgBFullPath));
    }

} STSNINFODETAILS, *LPSTSNINFODETAILS;

typedef struct _sn_info
{
    int iOperResult;
    DWORD dwCount;
    LPSTSNINFODETAILS *lppSNInfoDetail;
} STSNINFO, *LPSTSNINFO;

typedef struct _img_config
{
    IMAGETYPE     ImgType;
    unsigned int  uFSNMode;
    char          szDefImgPath[MAX_PATH];
    char          cSerialNoChar;   //test#29
    _img_config()
    {
        memset(szDefImgPath, 0, sizeof(szDefImgPath));
    }
} STIMGCONFIG, *LPSTIMGCONFIG;

class ISaveCusSNInfo
{
public:
    //得到客户化图片保存目录，保存在lpSaveImgPath中
    virtual void  GetSaveImgDir(const SYSTEMTIME &stSysTime,
                                EnumSaveSerialOperation OperCmd, char lpSaveImgPath[MAX_PATH]) = 0;
    virtual bool IsNeedSaveSNImg(EnumSaveSerialOperation OperCmd) = 0;
    virtual bool IsNeedSaveFullImg(EnumSaveSerialOperation OperCmd) = 0;
    virtual void GetSaveImgConfig(STIMGCONFIG &stImgConfig) = 0;
    //传入所有图片的相关信息，根据需要对冠字码信息包括图片进行处理
    virtual void OnSNInfoGeneration(const STSNINFO &stSNInfo) = 0;

    virtual void SetLfsCmdId(ADP_LFS_CMD_ID eLfsCmdId) = 0;
#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB)
    virtual void SetSNAdingMode(bool bSNAddingMode) = 0;
#elif defined(SET_BANK_SXXH)
    virtual void vDeleteSnrinfoForOpen() = 0;  //test#28
#else
    virtual bool SaveToDatabase() = 0;
    virtual void vSetFWVerInfo(char cFwVersionInof[512]) = 0;               //30-00-00-00(FS#00001)
#endif
};

extern "C" int CreateSaveSNInfoInstance(ISaveCusSNInfo *&pInst);
