#ifndef CFESM01DEF_H
#define CFESM01DEF_H
#include "QtTypeInclude.h"

//超时时间定义
#define M01_WRITE_TIMEOUT    1000           //发送数据超时时间
#define M01_READ_TIMEOUT     1000           //接收数据超时时间
#define M01_CMD_EXEC_TIMEOUT 3000           //命令超时时间

#define M01_SEND_PKG_STX     0xB0B1         //发送包包头标志
#define M01_SEND_PKG_ETX     0xB2           //发送包包尾标志
#define M01_ACK_PKG_STX      0xB3B4         //应答包包头标志
#define M01_ACK_PKG_ETX      0xB5           //应答包包尾标志

#define M01_USB_PACKET_SIZE  64             //USB通讯方式包数据大小

//加解密
#define DECRYPT_MODE_CFES                   0x00
#define ENCRYPT_MODE_CFES                   0x01

//RSA密钥属性
#define RSA_ATTR_PK_ENCRYPT                 0x00020000
#define RSA_ATTR_SK_DENCRYPT                0x00040000
#define RSA_ATTR_PK_VERIFY_SIGNATURE        0x08000000
#define RSA_ATTR_SK_SIGN                    0x10000000

//RSA签名算法
#define RSA_SIGN_ALGO_NO                    0xff
#define RSA_SIGN_ALGO_PKCS_1_V1_5           0x01
#define RSA_SIGN_ALGO_PKCS_1_PSS            0x03

//HASH算法
#define RSA_HASH_ALGO_MD5                   0x00
#define RSA_HASH_ALGO_SHA_1                 0x01
#define RSA_HASH_ALGO_SHA_224               0x02
#define RSA_HASH_ALGO_SHA_256               0x03

//硬件返回错误码
#define M01_ERR_SUCCESS                     0x00
#define M01_ERR_NO_PACKET_EXT               0x01
#define M01_ERR_VERIFY_CRC                  0x02
#define M01_ERR_NO_CMD                      0x03
#define M01_ERR_UNKNOWN                     0x10
#define M01_ERR_TAMPER_TRIGGER              0x11
#define M01_ERR_REMOVE_SWITCH_NO_PRESSED    0x12
#define M01_ERR_KEY_HOLD                    0x13
#define M01_ERR_CMD_UNSUPPORT               0x20
#define M01_ERR_CMD_NOT_COMPLETE            0x21
#define M01_ERR_CMD_LEN                     0x22
#define M01_ERR_CMD_PARAMETER               0x23
#define M01_ERR_CMD_EXEC_TOO_FREQUENTLY     0x24
#define M01_ERR_CMD_SEQ                     0x25
#define M01_ERR_DEVICE_LOCKED               0x26
#define M01_ERR_CMD_LOCKED                  0x27
#define M01_ERR_CMD_FUNC_NOT_EMPOWER        0x28
#define M01_ERR_UNKNOWN_FIELD_DATA          0x30
#define M01_ERR_FIELD1_VALUE_ILLEGAL        0x31
#define M01_ERR_FIELD2_VALUE_ILLEGAL        0x32
#define M01_ERR_FIELD3_VALUE_ILLEGAL        0x33
#define M01_ERR_FIELD4_VALUE_ILLEGAL        0x34
#define M01_ERR_FIELD5_VALUE_ILLEGAL        0x35
#define M01_ERR_FIELD6_VALUE_ILLEGAL        0x36
#define M01_ERR_FIELD7_VALUE_ILLEGAL        0x37
#define M01_ERR_FIELD8_VALUE_ILLEGAL        0x38
#define M01_ERR_FIELD9_VALUE_ILLEGAL        0x39
#define M01_ERR_FIELD10_VALUE_ILLEGAL       0x3A
#define M01_ERR_FIELD11_VALUE_ILLEGAL       0x3B
#define M01_ERR_FIELD12_VALUE_ILLEGAL       0x3C
#define M01_ERR_FIELD13_VALUE_ILLEGAL       0x3D
#define M01_ERR_FIELD14_VALUE_ILLEGAL       0x3E
#define M01_ERR_OTHER_FIELD_VALUE_ILLEGAL   0x3F
#define M01_ERR_SHIFT_POSITION              0x40
#define M01_ERR_SERIAL_NUMBER_EXIST         0x41
#define M01_ERR_SERIAL_NUMBER_NOT_EXIST     0x42
#define M01_ERR_PASSWORD_EXIST              0x43
#define M01_ERR_PASSWORD_NOT_EXIST          0x44
#define M01_ERR_NOT_VERIFY_PASSWORD         0x45
#define M01_ERR_PASSWORD_ERROR              0x46
#define M01_ERR_TWO_PASSWORD_SAME           0x47
#define M01_ERR_INPUT_CANCEL                0x48
#define M01_ERR_INPUT_TIMEOUT               0x49
#define M01_ERR_KEY_NOT_EXIST               0x4A
#define M01_ERR_KEY_VECTOR_NOT_EXIST        0x4B
#define M01_ERR_KEY_EXIST                   0x4C
#define M01_ERR_KEY_ATTR                    0x4D
#define M01_ERR_KEY_LEN                     0x4E
#define M01_ERR_KEY_ID_OUT_OF_RANGE         0x4F
#define M01_ERR_KEY_DATA_VERIFY             0x50
#define M01_ERR_INPUT_PIN_LEN               0x51
#define M01_ERR_CARDNO_OR_TRANSCODE_LEN     0x52
#define M01_ERR_CARDNO_OR_TRANSCODE_FORMAT  0x53
#define M01_ERR_NO_VALID_PIN_CODE           0x54
#define M01_ERR_NOT_EXEC_REMOVE_AUTH_VERIFY 0x55
#define M01_ERR_PRE_OPER_INFO_NOT_EXIST     0x56
#define M01_ERR_ALGO_FUNC_CALC              0x57
#define M01_ERR_VERIFY_SIGNATURE_FAIL       0x58
#define M01_ERR_PARAM_DIFF                  0x59
#define M01_ERR_NOT_EXEC_REMOVE_AUTH        0x5A
//#define M01_ERR_ALGO_LIB_CALC               0x5X
#define M01_ERR_FIRMWARE_PROG_BUG           0xE0
#define M01_ERR_FIRMWARE_UPDATE_VERIFY      0xF0
#define M01_ERR_FIRMWARE_UPDATE_PKG_SIZE    0xF1

#define DECRYPT_MODE_CFES                   0x00
#define ENCRYPT_MODE_CFES                   0x01
#pragma pack(push, 1)
typedef struct _READSYMMKEYKCV_CFES{
    WORD wKeyId;                    //Key id
    BYTE byKcvMode;                 //Kcv Mode(0:none/1:self/2:zero)
}READSYMMKEYKCV_CFES, *LPREADSYMMKEYKCV_CFES;

typedef struct _INPUTPIN_CFES{
    BYTE byMinLen;
    BYTE byMaxLen;
    BYTE byEndMode;                //0:end by terminate key 1:auto end
    BYTE byEchoCode;               //return code when vaild PIN pressed
    WORD wTimeout;                 //0x00:no time out other:time(seconds)
}INPUTPIN_CFES, *LPINPUTPIN_CFES;

typedef struct _SETINITVEC_CFES{
    WORD wKeyId;                   //Key id
    BYTE byVector[16];
}SETINITVEC_CFES, *LPSETINITVEC_CFES;

typedef struct _MACOP_CFES{
    WORD wKeyId;
    WORD wAlgorithm;
    BYTE byPad;
    WORD wDataLen;
    BYTE byData[2048];
}MACOP_CFES;

typedef struct _RANDOM_CFES{
    BYTE byUseType;                         //0x00:random data 0x01:remove verification 0x02:RKL random data
    WORD wRandomDataLen;                    //0x01~0x400
}RANDOM_CFES, *LPRANDOM_CFES;

typedef struct _IMPORTSM4KEYENCBYSM2_VERSIGN_CFES{
    WORD wSM4KeyId;
    WORD wSM2SKeyId;
    WORD wSignKeyId;
    BYTE byDecryptFillAlgo;
    BYTE bySignFillAlgo;
    DWORD dwSM4KeyAttr;
    BYTE bySymmKeyMode;
    BYTE byRandomDataMode;
    BYTE byAddMode;
    BYTE bySignValue[64];
    WORD wUserIdLen;
    BYTE byUserId[64];
    WORD wEncryptedKeyValueLen;
    BYTE byEncryptedKeyValue[512];
}IMPORTSM4KEYENCBYSM2_VERSIGN_CFES;

typedef struct _IMPORTSM4KEYENCBYSM2_CFES{
    WORD wSM4KeyId;
    WORD wSM2SKeyId;
    BYTE bySymmKeyMode;
    DWORD dwSM4KeyAttr;
    WORD wEncryptedKeyValueLen;
    BYTE byEncryptedKeyValue[128];
}IMPORTSM4KEYENCBYSM2_CFES;

typedef struct _GENERATERSAKEYPAIR_CFES{
    BYTE byKeyId;
    DWORD dwKeyLen;
    DWORD dwExponent;
    DWORD dwKeyAttr;
}GENERATERASKEYPAIR_CFES, *LPGENERATERASKEYPAIR_CFES;

typedef struct _IMPORTHOSTPKEY_CFES{
    BYTE byKeyId;
    BYTE byVerifySignatureKeyId;
    WORD wPKeyDataDERLen;                   //DER编码格式的公钥数据长度
    BYTE byPKeyDataDER[600];                //DER编码格式的公钥数据
    DWORD dwKeyAttr;
    DWORD dwSignFillAlgo;
    DWORD dwSignHashAlgo;
    WORD wSignValueLen;
    BYTE bySignValue[256];
}IMPORTHOSTPKEY_CFES, *LPIMPORTHOSTPKEY_CFES;

typedef struct _IMPORTSYMMKEYENCBYRSA_CFES{
    WORD wSymmKeyId;
    BYTE byDecryptKeyId;
    BYTE byVerifySigntureKeyId;
    DWORD dwDecryptFillAlgo;
    DWORD dwSignFillAlgo;
    DWORD dwDecryptHashAlgo;
    DWORD dwSignHashAlgo;
    DWORD dwSymmKeyAttr;
    BYTE bySymmKeyMode;
    BYTE byRandomDataMode;
    BYTE byAddMode;
    WORD wEncryptedSymmKeyValueLen;
    BYTE byEncryptedSymmKeyValue[512];
    WORD wSignatureLen;
    BYTE bySignature[256];
}IMPORTSYMMKEYENCBYRSA_CFES, *LPIMPORTSYMMKEYENCBYRSA_CFES;

#pragma pack(pop)
#endif // CFESM01DEF_H
