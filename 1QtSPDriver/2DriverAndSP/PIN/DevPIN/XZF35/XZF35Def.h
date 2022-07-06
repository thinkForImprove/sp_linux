#ifndef XZF35_H
#define XZF35_H

#include "QtTypeDef.h"

//包标志
#define CMD_STX          0x1B            //command packet start tag
#define CMD_ETX          0x0A0D          //command packet end tag
#define RSP_STX          0x02            //response packet start tag

//应答包结果码
#define RSP_OK           0x53            //command execute success
#define RSP_NG           0x45            //command execute fail

//最大长度
#define CMD_PKG_DATA_MAX_LEN    3272
#define RSP_PKG_DATA_MAX_LEN    3072
#define CMD_PKG_MAX_LEN         CMD_PKG_DATA_MAX_LEN + 8
#define RSP_PKG_MAX_LEN         RSP_PKG_DATA_MAX_LEN + 4
#define VER_STR_MAX_LEN                     128
#define DEV_SN_MAX_LEN                      32

//支持密钥个数最大值
#define DES_OR_SM4_KEY_MAX_NUM              64
#define RSA_OR_SM2_KEY_MAX_NUM              17

//KEY ATTRIBUTE
#define KEY_ATTR_DATA                       0x00000001
#define KEY_ATTR_PIN                        0x00000002
#define KEY_ATTR_MAC                        0x00000004
#define KEY_ATTR_ENC                        0x00000020
#define KEY_ATTR_VECTOR_DECRYPT             0x00000080
#define KEY_ATTR_REMOVE                     0x10000000
#define KEY_ATTR_IMPORT_COMPONENT           0x00000100
#define KEY_ATTR_INPUT_COMPONENT            0x00000200

//KCV MODE
#define KCV_MODE_NONE                       0x00
#define KCV_MODE_SELF                       0x01
#define KCV_MODE_ZERO                       0x02

#define ENCRYPT_MODE                        0x01
#define DECRYPT_MODE                        0x02

//MAC ALGORITHM
#define MAC_ALGO_X99                        0x0020          //DES MAC
#define MAC_ALGO_X99T                       0x0021
#define MAC_ALGO_BPI                        0x0031
#define MAC_ALGO_UBC                        0x0032
#define MAC_ALGO_X919                       0x0200          //2KEY TDES MAC
#define MAC_ALGO_PBCO                       0x0034

//DATA ALGORITHM
#define DATA_ALGO_ECB                       0x0001
#define DATA_ALGO_CBC                       0x0002
#define DATA_ALGO_TDES_ECB                  0x0040
#define DATA_ALGO_TDES_CBC                  0x0080

//PIN FORMAT
#define PIN_FORMAT_IBM3624                  0x0001
#define PIN_FORMAT_ANSIX98                  0x0002
#define PIN_FORMAT_ISO9564_F0               0x0004
#define PIN_FORMAT_ISO9564_F1               0x0008
#define PIN_FORMAT_FORMECT2                 0x0010
#define PIN_FORMAT_FORMECT3                 0x0020
#define PIN_FORMAT_FORMVISA                 0x0040
#define PIN_FORMAT_FORMDIEBOLD              0x0080
#define PIN_FORMAT_FORMDIEBOLDCO            0x0100
#define PIN_FORMAT_FORMVISA3                0x0200
#define PIN_FORMAT_ISO9564_F3               0x2000


//命令码
/* --------------TWO WAY VERIFICATION------------ */
#define GET_TWO_SIDES_VERIFICATION_KEY      0x4B47 //GK
#define GET_RANDOM_NUMBER                   0x4152 //RA
#define GET_TWO_SIDES_VERIFICATION_RESULT   0x4342 //BC

#define SET_SUPER_PASSWORD                  0x5053 //SP
#define CHANGE_SUPER_PASSWORD               0x5058 //XP
#define VERIFICTION_SUPER_PASSWORD          0x5043 //CP
#define INPUT_INITIAL_SYMMETRIC_KEY_COMPONENT  0x4B49 //IK
#define SET_SHIFT_KEY_POSTION               0x4853 //SH

#define SET_SYMMETRIC_ENC_ALGORITHM         0x5753 //SW
#define DES_KEY_LOAD                        0x4B4C //LK
#define SM4_KEY_LOAD                        0x474C //LG
#define QUERY_SYMMETRIC_ENC_KEY_INFO        0x5345 //ES
#define READ_SYMMETRIC_ENC_KEY_KCV          0x4352 //RC
#define INPUT_PIN                           0x5052 //RP
#define GET_PINBLOCK                        0x4250 //PB

/*------------------DATA OR MAC OPERATION---------*/
#define SET_INITIAL_PLAINTEXT_VECTOR        0x5644 //DV
#define SET_INITIAL_CIPERTEXT_VECTOR        0x5645 //EV
#define DATA_ECB_OR_CBC_ARITHMETIC          0x4F44 //DO
#define MAC_ARITHMETIC                      0x434D //MC
#define SET_PAD_VALUE_MAC_ARITHMETIC        0x504D //MP

#define GENERATE_SM2_KEY                    0x4D47 //GM
#define SM2_KEY_LOAD                        0x534C //LS
#define EXPORT_SM2_KEY                      0x4D45 //EM
#define ACTIVE_SM2_KEY                      0x4947 //GI
#define SM2_DATA_ENC_OR_DENC                0x4C44 //DL
#define SM2_SIGNATURE_OPERATION             0x5147 //GQ
#define SM2_CHECK_SIGNATURE_OPERATION       0x5647 //GV
#define SM4_KEY_ENC_BY_SM2_LOAD             0x415A //ZA
#define MD5_ARITHMETIC                      0x4D44 //DM
#define SHA_ARITHMETIC                      0x4844 //DH
#define SM3_ARITHMETIC                      0x4644 //DF

/*-----------------DUKPT ONE COMMAND ONE KEY-------------*/
#define DUKPT_IK_LOAD                       0x495A //ZI
#define DUKPT_GENE_NEW_DUKPT_KEY_GROUP      0x475A //ZG
#define DUKPT_READ_IKSN_VALUE               0x525A //ZR
#define DUKPT_READ_KEY_KCV                  0x4352 //ZC

/*-----------------RSA RKL INSTRUCTION--------------------*/
#define RSA_GENERATE_KEY_PAIR               0x4147 //GA
#define RSA_IMPORT_KEY_PAIR_CS_MODE         0x524C //LR
#define RSA_IMPORT_PRIVATE_KEY_CS_MODE      0x5249 //IR
#define RSA_EXPORT_PULIC_KEY_AND_SIGNATURE  0x5245 //ER
#define RSA_EXPORT_EPPID_SIGNATURE          0x4545 //EE
#define EXPORT_RSAPK_SIGNED_EPPID_AND_PK_SIGNATURE 0x4549 //IE
#define RSA_ENC_OR_DENC_ARITHMETIC          0x5344 //DS
#define IMPORT_DES_KEY_ENC_BY_RSA_CS_MODE   0x4452 //RD

/*-----------------DELETE KEY----------------------------*/
#define DELETE_ALL_KEY                      0x4144 //DA
#define DELETE_MARKED_SYMMETRIC_KEY         0x4444 //DD
#define DELETE_DUKPT_KEY                    0x445A //ZD
#define DELETE_SM2_KEY                      0x4744 //DG
#define DELETE_RSA_KEY                      0x5244 //DR

/*-----------------PLAINTTEXT INPUT----------------------*/
#define SET_ACTIVE_AND_TERMINATE_KEY        0x4B53 //SK
#define INPUT_PLAINTEXT                     0x4B4F //OK
#define GET_KEY_VALUE_PASSIVE               0x4247 //GB
#define ABORT_INPUT                         0x4B43 //CK
#define SET_LONG_PRESS_TIME                 0x4A53 //SJ
#define SET_KEY_VALUE_TABLE                 0x4453 //SD

/*-----------------DEVICE REMOVE-------------------------*/
#define DEV_REMOVE_AUTHENTICATION           0x4145 //EA
#define REMOVE_OPERATION                    0x5845 //EX

#define RESET                               0x4653 //SF
#define MODIFY_COMMU_BPS                    0x4253 //SB
#define SET_REMOVE_FUNCTION                 0x4553 //SE
#define GET_DEVICE_STATUS                   0x5745 //EW
#define GET_FIRMWARE_VERSION                0x4552 //RE
#define GET_FIRMWARE_INNER_VERSION          0x4852 //RH
#define SET_SERIAL_NUMBER                   0x5347 //GS
#define READ_SERIAL_NUMBER                  0x5352 //RS
#define SET_ENABLE_BUZZER                   0x5A53 //SZ
#define SWITCH_DUAL_CHANNEL                 0x454B //KE
#define WRITE_CUSTOMER_INFO                 0x4957 //WI
#define READ_CUSTOMER_INFO                  0x4952 //RI
#define WRITE_CUSTOMER_MASS_DATA            0x4B57 //WK
#define READ_CUSTOMER_MASS_DATA             0x4B44 //DK
#define DELETE_CUSTOMER_DATA                0x5344 //DR
#define READ_KB_INNER_PARAM                 0x5444 //GT
#define SET_SYSTEM_TIME                     0x5453 //ST
#define READ_SYSTEM_TIME                    0x5453 //RT

//错误码
#define KUHN_DEFENSE_ACTIVE                 0x01
#define WRONG_ALGORITHM                     0x02
#define INSTRUCTION_FORMAT_ERROR            0x03
#define SN_ALREADY_EXIST                    0x07
#define SN_NO_EXIST                         0x08
#define SAVE_SIGNATURE_DATA_ERROR           0x09
#define NO_VERSION                          0x0A
#define PASSWORD_NO_EXIST                   0x0B
#define PASSWORD_ALREADY_EXIST              0x0C
#define PASSWORD_ERROR                      0x0D
#define KEY_NO_EXIST                        0x0E
#define KEY_ALREADY_EXIST                   0x0F
#define KEY_ATTRIBUTE_ERROR                 0x10
#define INPUT_EXIT_ABNORMAL                 0x11
#define KEY_LOAD_FAIL                       0x12
#define SAME_VALUE_KEY_EXIST                0x14
#define AUTHENTICATION_DATA_CHECK_ERROR     0x16
#define KEY_INDEX_OUT_OF_RANGE              0x17
#define PAD_DATA_ERROR                      0x18
#define NO_INPUT_PIN_PASSWORD               0x19
#define PIN_PASSWORD_INVALID                0x1A
#define PIN_INPUT_INTERVAL_TOO_SHORT        0x1B
#define PIN_LENGTH_ERROR                    0x1C
#define CARDNUM_OR_TRANSCODE_LENGTH_ERROR   0x1D
#define INSTRUCTION_SEQUENCE_ERROR          0x1F
#define NO_MEMORY                           0x20
#define RSA_DENCRYPT_FAIL                   0x57
#define PUBLIC_KEY_DER_ANALYZE_FAIL         0x50
#define RSA_KEY_LENGTH_ERROR                0x59
#define EPPID_NO_IMPORT                     0x60
#define RSA_KEY_NO_IMPORT                   0x72
#define KEYBOARD_LOCKED                     0x24
#define PAD_DATA_LENGTH_ERROR               0x25
#define OPERATION_TIMEOUT                   0x26
#define NO_VERFICATION_PASSWORD             0x29
#define PARAM_ERROR_GET_PINBLOCK_DATA       0x30
#define ESSENTIAL_DATA_LENGTH_ERROR         0x31
#define NEED_REMOVE_OPERATION               0x34
#define DEVICE_NO_INITIALIZATION            0x35
#define NEED_TWO_WAY_VERFICATION            0x36
#define WRITED_REMOVE_INFO_FORMAT_ERROR     0x39
#define SET_PLAINT_OR_CIPHER_TEXT_SPE_FUN_FAIL 0x40
#define WRITE_CUSTOMER_INFO_ERROR           0x41
#define KEY_LENGTH_ERROR                    0x42
#define CUSTOMER_INFO_NO_EXIST              0x43
#define FIRMWARE_SELF_CHECK_FAIL            0x44
#define RSA_DATA_GREATER_THAN_RSA_MOLD      0x45
#define SM2_OR_RSA_KEY_NO_ACTIVATED           0x46
#define SM_CHIP_ACCESS_TIMEOUT              0x47
#define ARITHMETIC_ERROR                    0x48
#define SIGNATURE_CHECK_ERROR               0x49
#define NO_SUPPORTED_HASH_ALGORITHM         0x4A
#define PW_INSTRUCTION_SEQUENCE_ERROR       0x51
#define TWICE_PASSWORD_NOT_SAME             0x52
#define OLD_PASSWORD_VERFICATION_FAIL       0x53
#define KEY_SET_SHIFT_NO_PERMIT             0x54
#define ACTIVE_KEY_FAIL_AS_PARAM_ERROR      0x55
#define RSA_SIGNATURE_INVALID               0x56
#define DER_DATA_ANALYSIS_ERROR              0x58
#define RSA_E_VALUE_ERROR                   0x5a
#define RSA_KEY_PAIR_NO_EXIST               0x71
#define SIGNATURE_ERROR                     0x73
#define SIGNATURE_NO_IMPORT                 0x74
//#define UNKNOWN_ERROR                     0xFX        //范围值0xF0~0xFF
#define KEY_HOLD                            0xA5

//命令参数结构体定义
#pragma pack(push, 1)
//REQ PACKET
typedef struct _REQINFO{
    BYTE bySTX;                             //开始标志
    WORD wCmd;                              //命令码
    WORD wLen;                              //命令数据长度
    BYTE byData[CMD_PKG_DATA_MAX_LEN + 3];  //命令数据
}REQINFO, *LPREQINFO;

//RSP PACKET
typedef struct _RSPINFO{
    BYTE bySTX;                             //开始标志
    BYTE byResCode;                         //命令结果码
    WORD wLen;                              //返回数据长度
    BYTE byData[RSP_PKG_DATA_MAX_LEN];      //返回数据
}RSPINFO, *LPRSPINFO;

//CMD PARAM
typedef struct _RANDOM{
    BYTE byUseType;                         //0x30:remove verification 0x31:two-sides verification
    BYTE byRandomBytes;                     //0x01~0x10[0x30(0x01~0x10) 0x31(0x10)]
}RANDOM, *LPRANDOM;

typedef struct _TWOSIDESVERI{
    BYTE byVeriData[16];
}TWOSIDESVERI, *LPTWOSIDESVERI;

typedef struct _SUPERPW{
    BYTE byMode;                            //0x30:Super password A  0x31:Super password B
    BYTE byFixData;                         //0xAA
}SUPERPW, *LPSUPERPW;

typedef struct _INPUTINITKEYCMPT{
    BYTE  byMode;                           //Input mode[0x30(pw A, Key component A);0x31(pw B, Key component B);0x32(no pw, Key componet A);0x33(no pw, Key component B)]]
    WORD  wKeyId;                           //Key index
    DWORD dwKeyAttr;                        //Key attribute
    WORD  wInputMaxLen;                     //Input max length
    BYTE  byAutoEnd;                        //0x00(no automatic end) 0x01(automatic end)
}INPUTINITKEYCMPT, *LPINPUTINITKEYCMPT;

typedef struct _SETINNERFUNC{
    BYTE byFunc;                    //0x00 : disable remove func  other : enable
    BYTE byResv;
}SETINNERFUNC, *LPSETINNERFUNC;

typedef struct _SETSHIFTPOS{
    DWORD dwShiftPos;               //default position is blank key
}SETSHIFTPOS, *LPSETSHIFTPOS;

typedef struct _SETSYMMKEYHDLMODE{
    BYTE byAlgo;                    //0x00:DES/3DES  0x02:SM4
}SETSYMMKEYHDLMODE, *LPSETSYMMKEYHDLMODE;

typedef struct _IMPORTKEY{
     WORD wKeyId;
     WORD wMKeyId;
     DWORD dwKeyAttr;
     BYTE byKeyValue[24];           //DES:0/8/16/24 SM4:16
}IMPORTKEY, *LPIMPORTKEY;

typedef struct _GETSYMMKEYINFO{
    WORD wKeyId;
}GETSYMMKEYINFO, *LPGETSYMMKEYINFO;

typedef struct _READSYMMKEYKCV{
    WORD wKeyIdAndMode;             //one and a half:keyId+1 half:mode(0:none/1:self/2:zero)
}READSYMMKEYKCV, *LPREADSYMMKEYKCV;

typedef struct _INPUTPIN{
    BYTE byMinLen;
    BYTE byMaxLen;
    BYTE byGetKeyModeAndEndMode;  //first half byte : key send mode  | second half byte : autoend
    BYTE byEchoCode;               //return code when vaild PIN pressed
}INPUTPIN, *LPINPUTPIN;

typedef struct _PINBLOCK{
    WORD wKeyId;
    WORD wFormat;
    BYTE byPadChar;
    BYTE byCustomerDataLen;
    BYTE byCustomerData[16];
}PINBLOCK;

typedef struct _SETINITVEC{
    BYTE byVector[16];
}SETINITVEC, *LPSETINITVEC;

typedef struct _SETCIPHEREDINITVEC{
    WORD wKeyId;
    BYTE byVector[16];
}SETCIPHEREDINITVEC, *LPSETCIPHEREDINITVEC;

typedef struct _DATAOP{
    WORD wKeyId;
    BYTE byOperate;
    WORD wAlgorithm;
    BYTE byPadChar;
    WORD wDataLen;
    BYTE byData[1024];
}DATAOP, *LPDATAOP;

typedef struct _MACOP{
    WORD wKeyId;
    WORD wAlgorithm;
    WORD wDataLen;
    BYTE byData[2048];
}MACOP;

typedef struct _MACOPPADCHAR{
    WORD wKeyId;
    BYTE byPadChar;
    WORD wAlgorithm;
    WORD wDataLen;
    BYTE byData[2048];
}MACOPPADCHAR, *LPMACOPPADCHAR;

typedef  struct _DELSPECKEY{
    WORD wKeyId;
} DELSPECKEY, *LPDELSPECKEY;

typedef struct _ACTIVATEKEY{
    DWORD dwActiveKeys;
    DWORD dwActiveFDKs;
    DWORD dwTerminateKeys;
    DWORD dwTerminateFDKs;
}ACTIVATEKEY;

typedef struct _INPUTPLAINTEXT{
    BYTE byMaxLen;
    BYTE byEndMode;                   //first half byte : key send mode  | second half byte : autoend
}INPUTPLAINTEXT, *LPINPUTPLAINTEXT;

typedef struct _GETINPUTKEYVALUE{

}GETINPUTKEYVALUE, *LPGETINPUTKEYVALUE;

typedef struct _BEEP{
    BYTE bySwitch;                   //0x30:open 0x31:close
}BEEP;

typedef struct _IMPORTSM4KEY{
    WORD wKeyId;
    WORD wMKeyId;
    UINT ulKeyAttr;
    BYTE byKeyValue[16];
}IMPORTSM4KEY;

typedef struct _IMPORTSM4KEYENCBYSM2{				//30-00-00-00(FS#0003)
    WORD wSM4KeyId;									//30-00-00-00(FS#0003)
    WORD wSM2KeyId;									//30-00-00-00(FS#0003)
    DWORD dwKeyAttr;                                //30-00-00-00(FS#0003)
    BYTE byKeyValue[128];							//30-00-00-00(FS#0003)
}IMPORTSM4KEYENCBYSM2;								//30-00-00-00(FS#0003)

typedef struct _SM2DATAOPT{							//30-00-00-00(FS#0003)
    WORD wKeyId;									//30-00-00-00(FS#0003)
    BYTE byKeyType;         //0x50 公钥 0x53 私钥    //30-00-00-00(FS#0003)
    BYTE byData[352];								//30-00-00-00(FS#0003)
}SM2DATAOPT;										//30-00-00-00(FS#0003)

typedef struct _IMPORTSM2KEYPAIR{                   //30-00-00-00(FS#0003)
    WORD wKeyId;                                    //30-00-00-00(FS#0003)
    BYTE byKeyAttr;                                 //30-00-00-00(FS#0003)
}IMPORTSM2KEYPAIR;                                  //30-00-00-00(FS#0003)

typedef struct _EXPORTSM2EPPSIGNKEY{                 //30-00-00-00(FS#0003)
    WORD wKeyId;                                     //30-00-00-00(FS#0003)
    BYTE byKeyAttr;                                  //30-00-00-00(FS#0003)
}EXPORTSM2EPPSIGNKEY;                                //30-00-00-00(FS#0003)

//Res data
typedef struct _DEVSTATUS{
    BYTE byBatteryStatus;           //0x01:Normal 0x00:low voltage
    BYTE byTemperActive;            //0x01:Active 0x00:inacitve
    BYTE byFactoryInit;             //0x01:yes    0x00:no
    BYTE byMasterKey;               //0x01:exist  0x00:no
    BYTE byWorkKey;                 //0x01:exist  0x00:no
    BYTE byRemoveLegal;             //0x01:legal  0x00:illegal
    BYTE byTwoSidesVerifOk;         //0x01:yes    0x00:no
    BYTE byReserved;
}DEVSTATUS, LPDEVSTATUS;

#pragma pack(pop)


#endif // XZF35_H
