#ifndef DEVPIN_H
#define DEVPIN_H
#include "IDevPIN.h"
#include "QtTypeInclude.h"

typedef list<EPP_KEYVAL>                listEPP_KEYVAL;
typedef map<BYTE, std::string>          mapByteString;
#define MACROTOSTR(X)   #X

#define EXWORDHL(X)  (WORD)((LOBYTE(X) << 8) | HIBYTE(X))
#define EXDWORDHL(X) (DWORD)((DWORD)(LOWORD(X) << 16) | (HIWORD(X)))
#define RVSDWORD(X)  (DWORD)((EXWORDHL(X & 0xffff) << 16) | (EXWORDHL((X & 0xffff0000) >> 16)))

//对称密钥模式
#define SYMMETRIC_KEY_MODE_DES              0x00
#define SYMMETRIC_KEY_MODE_AES              0x01
#define SYMMETRIC_KEY_MODE_SM4              0x02
#define SYMMETRIC_KEY_MODE_UNKNOWN          0xFF

//ANS1
#define ASN1_INTEGER		0x02
#define ASN1_SEQUENCE		0x30

//长数据加解密时，分块数据长度
#define BLOCK_DATA_MAX_SIZE                 (1024)

#endif // DEVPIN_H
