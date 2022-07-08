#pragma once

#include "QtTypeDef.h"

#define     CIPHER                      0
#define     COMPOUND                    1
#define     PIN_ANSIX98                 2
//#define     PIN_IBM3624                 3

class CDes
{
public:
    int EntryDes(LPBYTE pbyText, WORD wInSize, LPBYTE pbyKey, WORD wKeySize,
        WORD wFlag, LPBYTE pbyOutCode, WORD wOutSize);
private:
    BYTE char2bin( BYTE str );
    void text_sub( LPBYTE pbyInCode );
    void key_sub( LPBYTE pbyKeyCode );
    void des();
    void ip_tbl_sub();
    void e_tbl_sub();
    void pc1_tbl_sub();
    void ls_tbl_sub();
    void pc2_tbl_sub();
    void s_tbl_sub();
    void p_tbl_sub();
private:
    int loop,id;
    BYTE by1,bb1,by2,bb2,s_bit;
    BYTE text[16],key_dt[16];
    BYTE   ip_data[8],
        l0_data[17][4],
        r0_data[17][4],
        r0_Convert[16][6],
        pc1_data[7],
        c0_Convert[17][4],
        d0_Convert[17][4],
        ls,
        k1_Convert[16][6],
        s_box[6],
        s_dt,
        g_no,
        r_no,
        p_data[4],
        p_Convert[16][4],
        output_data[8],
        p;

private:
    static BYTE bit[4];
    static BYTE parity_tbl[256];
    static BYTE h_bit[8];
    static BYTE ip_tbl[64];
    static BYTE e_tbl[48];
    static BYTE pc1_tbl[56];
    static BYTE ls_tbl[16];
    static BYTE pc2_tbl[48];
    static BYTE s_tbl[8][64];
    static BYTE p_tbl[32];
    static BYTE ip1_tbl[64];
};
class CEncrypt
{
public:
    //DES加密
    void DataEncrypt(BYTE *data, BYTE *Key, BYTE *ResultCode);
    //DES解密
    void DataDecrypt(BYTE *data, BYTE *Key, BYTE *ResultCode);
    //3DES加密
    void DataEncrypt3DES(BYTE *data, BYTE *Key, BYTE *ResultCode);
    //3DES解密
    void DataDecrypt3DES(BYTE *data, BYTE *Key, BYTE *ResultCode);
    //DES密钥更新
    BOOL KeyCheck(BYTE *data, BYTE *Key, BYTE *NewKey, BYTE *CheckCode=NULL);
    //3DES密钥更新
    BOOL KeyCheck3DES(BYTE *data, BYTE *Key, BYTE *NewKey, BYTE *CheckCode=NULL);
    //3DES密钥8字节模式更新
    BOOL KeyCheck3DESEx(BYTE *data, BYTE *Key, BYTE *NewKey, BYTE *CheckCode=NULL);
    //DES密码生成
    BOOL PinCheck(BYTE *Pin, int PinSize, BYTE *Pan, int PanSize, BYTE byFill, int PinType, BYTE *Key, BYTE *CheckCode);
    //3DES密码生成
    BOOL PinCheck3DES(BYTE *Pin, int PinSize, BYTE *Pan, int PanSize, BYTE byFill, int PinType, BYTE *Key, BYTE *CheckCode);
    //DES密钥MAC码生成
    BOOL MacCheck(BYTE *MacData, int DataSize, BYTE byFill, BYTE *Key, BYTE *CheckCode);
    //3DES密钥MAC码生成
    BOOL MacCheck3DES(BYTE *MacData, int DataSize, BYTE byFill, BYTE *Key, BYTE *CheckCode);
	//FW3.0, Mac code generation
	BOOL MacCreat3DES(BYTE *MacData, int DataSize, BYTE byFill, BYTE *Key, BYTE *MacCode);				//40-23-00-03(DS1#0003)//40-23-00-03(PG#00007)
	void UID_KBPKEncrypt3DES(BYTE *data, BYTE *Key, BYTE *ResultCode);									//40-23-00-06(PG#00037)
	int sm4_enc_ecb(unsigned char *pInput, int nInSize, unsigned char *pOutput, int nOutSize, unsigned char *pKey);						//40-28-00-06(DS1#0006)
	int sm4_dec_ecb(unsigned char *pInput, int nInSize, unsigned char *pOutput, int nOutSize, unsigned char *pKey);						//40-28-00-06(DS1#0006)
	int sm4_enc_cbc(unsigned char *pInput, int nInSize, unsigned char *pOutput, int nOutSize, unsigned char *pKey, unsigned char *pIV);	//40-28-00-06(DS1#0006)
	int sm4_dec_cbc(unsigned char *pInput, int nInSize, unsigned char *pOutput, int nOutSize, unsigned char *pKey, unsigned char *pIV);	//40-28-00-06(DS1#0006)
public:
    //辅助方法
    static void Compress(BYTE *Source, BYTE *Dest, int Size);
    static void Expand(BYTE *Source, BYTE *Dest, int Size);
private:
    //PIN加密数据生成
    void PinEncryptData(BYTE *Pin, int PinSize, BYTE *Pan, int PanSize, BYTE byFill, int PinType, BYTE *EncryptData);
    CDes des;
};
