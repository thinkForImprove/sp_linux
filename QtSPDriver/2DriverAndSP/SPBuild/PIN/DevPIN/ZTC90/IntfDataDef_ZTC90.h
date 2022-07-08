#pragma once

//常量定义
#define DEF_TIMEOUT                 (3000 ) // 默认超时
#define CMD_START                   (0x02)  // 命令开始符
#define CMD_END                     (0x03)  // 命令结束符

#define MARK_LEN                    (1)     // 命令开始/结束标志字节长度
#define LEN_LEN                     (2)     // 命令数据长度２字节
#define CMD_OR_RSP_CODE_LEN         (2)     // 命令码/响应码２字节
#define FORMAT_DATA_LEN             (4)     // 开始标志位+LRC+结束标志位 长度

//错误码
#define CMD_EXEC_OK                     0x04
#define CMD_LEN_ERR                     0x15
#define KEY_VERIFICATION_ERR            0x16
#define INVALID_PARAM_OR_MODE           0x17
#define KCV_VERIFICATION_ERR            0x18
#define PIN_INPUT_LEN_INVALID           0x19
#define PRODUCTION_SN_SETTED            0x1A
#define NO_CARD_IN_PSAM_SLOT            0x21
#define PSAM_CARD_OPT_ERR               0x22
#define PSAM_CARD_NO_POWERON            0x23
#define PSAM_CARD_TYPE_NOT_SUPP         0x24
#define KEY_PRESS_HOLD                  0x80
#define TIMEOUT_NO_KEY_PRESS            0x81
#define SM_CHIP_COMM_TIMEOUT            0x8E
#define MKEY_NOT_EXIST_OR_INVALID       0xA4
#define WKEY_NOT_EXIST_OR_INVALID       0xA5
#define SM2_RSAKEY_NOT_EXIST_OR_INVALID 0xA6
#define EXC_SM4KEY_NOT_EXIST_OR_INVALID 0xA7
#define ENCRYPT_OR_DECRYPT_FAIL         0xA8
#define SIGN_OR_SIGN_VERF_FAIL          0xA9
#define KEY_EXCHANGE_FAIL               0xAA
#define INVALID_RSA_MOLD_HEIGHT         0xB0
#define INVALID_RSA_MOLD_DATA           0xB1
#define INVALID_RSA_EXPONENT_LEN        0xB2
#define INVALID_RSA_EXPONENT            0xB3
#define INVALID_RSA_DATA_STRUCTURE      0xB4
#define INVALID_PADDING_DATA            0xB5
#define INVALID_RSA_SIGN_DATA           0xB6
#define INVALID_DATA_LEN                0xB7
#define BATTERY_LOW                     0xC4
#define BATTERY_BAD                     0xC5
#define INVALID_FLASH_PARAM             0xDA
#define WRITE_FLASH_PARAM_FAIL          0xDB
#define SELF_DEST_REGISTER_CONFIGURED   0xE0
#define SELF_DEST_REGISTER_LOCKED       0xE1
#define CLEAR_SELF_DEST_VERI_FAIL       0xE2
#define NO_EXEC_CLEAR_SELE_DEST_VERI    0xE3
#define CHIP_ABNORMAL                   0XEF

#define STR(X)  #X
