#include "Encrypt.h"

//40-28-00-06(DS1#0006) Start
static const unsigned char	_sm4_tbl_Sbox[256] =  {
		0xd6, 0x90, 0xe9, 0xfe, 0xcc, 0xe1, 0x3d, 0xb7, 0x16, 0xb6, 0x14, 0xc2, 0x28, 0xfb, 0x2c, 0x05,
		0x2b, 0x67, 0x9a, 0x76, 0x2a, 0xbe, 0x04, 0xc3, 0xaa, 0x44, 0x13, 0x26, 0x49, 0x86, 0x06, 0x99,
		0x9c, 0x42, 0x50, 0xf4, 0x91, 0xef, 0x98, 0x7a, 0x33, 0x54, 0x0b, 0x43, 0xed, 0xcf, 0xac, 0x62,
		0xe4, 0xb3, 0x1c, 0xa9, 0xc9, 0x08, 0xe8, 0x95, 0x80, 0xdf, 0x94, 0xfa, 0x75, 0x8f, 0x3f, 0xa6,
		0x47, 0x07, 0xa7, 0xfc, 0xf3, 0x73, 0x17, 0xba, 0x83, 0x59, 0x3c, 0x19, 0xe6, 0x85, 0x4f, 0xa8,
		0x68, 0x6b, 0x81, 0xb2, 0x71, 0x64, 0xda, 0x8b, 0xf8, 0xeb, 0x0f, 0x4b, 0x70, 0x56, 0x9d, 0x35,
		0x1e, 0x24, 0x0e, 0x5e, 0x63, 0x58, 0xd1, 0xa2, 0x25, 0x22, 0x7c, 0x3b, 0x01, 0x21, 0x78, 0x87,
		0xd4, 0x00, 0x46, 0x57, 0x9f, 0xd3, 0x27, 0x52, 0x4c, 0x36, 0x02, 0xe7, 0xa0, 0xc4, 0xc8, 0x9e,
		0xea, 0xbf, 0x8a, 0xd2, 0x40, 0xc7, 0x38, 0xb5, 0xa3, 0xf7, 0xf2, 0xce, 0xf9, 0x61, 0x15, 0xa1,
		0xe0, 0xae, 0x5d, 0xa4, 0x9b, 0x34, 0x1a, 0x55, 0xad, 0x93, 0x32, 0x30, 0xf5, 0x8c, 0xb1, 0xe3,
		0x1d, 0xf6, 0xe2, 0x2e, 0x82, 0x66, 0xca, 0x60, 0xc0, 0x29, 0x23, 0xab, 0x0d, 0x53, 0x4e, 0x6f,
		0xd5, 0xdb, 0x37, 0x45, 0xde, 0xfd, 0x8e, 0x2f, 0x03, 0xff, 0x6a, 0x72, 0x6d, 0x6c, 0x5b, 0x51,
		0x8d, 0x1b, 0xaf, 0x92, 0xbb, 0xdd, 0xbc, 0x7f, 0x11, 0xd9, 0x5c, 0x41, 0x1f, 0x10, 0x5a, 0xd8,
		0x0a, 0xc1, 0x31, 0x88, 0xa5, 0xcd, 0x7b, 0xbd, 0x2d, 0x74, 0xd0, 0x12, 0xb8, 0xe5, 0xb4, 0xb0,
		0x89, 0x69, 0x97, 0x4a, 0x0c, 0x96, 0x77, 0x7e, 0x65, 0xb9, 0xf1, 0x09, 0xc5, 0x6e, 0xc6, 0x84,
		0x18, 0xf0, 0x7d, 0xec, 0x3a, 0xdc, 0x4d, 0x20, 0x79, 0xee, 0x5f, 0x3e, 0xd7, 0xcb, 0x39, 0x48
	};

static const unsigned int	_sm4_tbl_FK[4] = {
		0xa3b1bac6, 0x56aa3350, 0x677d9197, 0xb27022dc
	};

static const unsigned int	_sm4_tbl_CK[32] = {
		0x00070e15, 0x1c232a31, 0x383f464d, 0x545b6269,
		0x70777e85, 0x8c939aa1, 0xa8afb6bd, 0xc4cbd2d9,
		0xe0e7eef5, 0xfc030a11, 0x181f262d, 0x343b4249,
		0x50575e65, 0x6c737a81, 0x888f969d, 0xa4abb2b9,
		0xc0c7ced5, 0xdce3eaf1, 0xf8ff060d, 0x141b2229,
		0x30373e45, 0x4c535a61, 0x686f767d, 0x848b9299,
		0xa0a7aeb5, 0xbcc3cad1, 0xd8dfe6ed, 0xf4fb0209,
		0x10171e25, 0x2c333a41, 0x484f565d, 0x646b7279
	};


#define	SM4_BLOCK_BYTES			16

typedef union _sm4_block_data_ {
	unsigned char	d_ui8[16];
    unsigned int	d_ui32[4];
} SM4_BLOCK_DATA;

typedef enum _sm4_crypt_oprt_{
	SM4_OPRT_ENCRYPT,		// encrypt
	SM4_OPRT_DECRYPT		// decrypt
} E_SM4_CRYPT_OPRT;

typedef enum _sm4_crypt_mode_{
	SM4_MODE_ECB,			// ECB mode
	SM4_MODE_CBC			// CBC mode
} E_SM4_CRYPT_MODE;

typedef enum _sm4_crypt_result_{
	SM4_RESULT_OK = 0,		// success
	SM4_RESULT_NG = 1		// error
} E_SM4_CRYPT_RESULT;

// internal functions
int _sm4_crypt(E_SM4_CRYPT_OPRT eOprt, E_SM4_CRYPT_MODE eMode,
				unsigned char *pInData, int nInSize,
				unsigned char *pOutData, int nOutSize,
				unsigned char *pKey, unsigned char *pIV = 0);
void _sm4_setup_key(void *pKey);
void _sm4_crypt_blk(void *pData, E_SM4_CRYPT_OPRT eOprt);
void _sm4_func_T(unsigned int *pWord32);
void _sm4_func_T2(unsigned int *pWord32);
void _sm4_trans_t(unsigned int* pWord32);
void _sm4_trans_L(unsigned int* pWord32);
void _sm4_trans_L2(unsigned int* pWord32);
void __swap_words(SM4_BLOCK_DATA *pBlk);

// key expansion data
unsigned int	rk[32];			// (TLS)

/////////////////////////////////////////////////////////////////////////////
int _sm4_crypt(E_SM4_CRYPT_OPRT eOprt, E_SM4_CRYPT_MODE eMode,
				unsigned char *pInData, int nInSize,
				unsigned char *pOutData, int nOutSize,
				unsigned char *pKey, unsigned char *pIV)
{
	int		nRet = SM4_RESULT_NG;

	SM4_BLOCK_DATA	ChainBlk, ChainBlk_t;
	SM4_BLOCK_DATA	CryptKey, CipherBlk;
    memset(&ChainBlk, 0, sizeof(ChainBlk));
    memset(&ChainBlk_t, 0, sizeof(ChainBlk_t));
    memset(&CryptKey, 0, sizeof(CryptKey));
    memset(&CipherBlk, 0, sizeof(CipherBlk));
	int		nBlocks;
	int		i;

	for (;;) {
		// check param
		if ( (pInData == 0) || (pOutData == 0) || (pKey == 0) || (nInSize <= 0) ) {
			break;
		}
		if ( (eMode == SM4_MODE_CBC) && (pIV == 0) ) {
			break;
		}
		nBlocks = ((nInSize - 1) / SM4_BLOCK_BYTES) + 1;
		if ( nOutSize < (nBlocks * SM4_BLOCK_BYTES) ) {
			break;
		}

		// seup key
		::memcpy(&CryptKey, pKey, SM4_BLOCK_BYTES);
		__swap_words(&CryptKey);	// LE to BE
		_sm4_setup_key( &CryptKey );
		
		// cryption
		// - if CBC
		if (eMode == SM4_MODE_CBC) {
			::memcpy(&ChainBlk, pIV, SM4_BLOCK_BYTES);
		}

		while ( nInSize > 0 ) {

			// copy one Input Data block.
			::memset(&CipherBlk, 0x00, SM4_BLOCK_BYTES);
			::memcpy(&CipherBlk, pInData, (nInSize >= SM4_BLOCK_BYTES) ? SM4_BLOCK_BYTES : nInSize);

			// - if CBC
			if (eMode == SM4_MODE_CBC) {
				if (eOprt == SM4_OPRT_ENCRYPT) {
					for ( i = 0; i < 4; ++i ) {
						CipherBlk.d_ui32[i] ^= ChainBlk.d_ui32[i];
					}
				} else {
					::memcpy(&ChainBlk_t, &CipherBlk, SM4_BLOCK_BYTES);
				}
			}

			// crypt one block
			__swap_words( &CipherBlk );		// LE to BE
			_sm4_crypt_blk( &CipherBlk, eOprt );
			__swap_words( &CipherBlk );		// BE to LE

			// - if CBC
			if (eMode == SM4_MODE_CBC) {
				if (eOprt == SM4_OPRT_ENCRYPT) {
					::memcpy(&ChainBlk, &CipherBlk, SM4_BLOCK_BYTES);
				} else {
					for ( i = 0; i < 4; ++i ) {
						CipherBlk.d_ui32[i] ^= ChainBlk.d_ui32[i];
					}
					::memcpy(&ChainBlk, &ChainBlk_t, SM4_BLOCK_BYTES);
				}
			}
			
			// copy one Output Data block.
			::memcpy(pOutData, &CipherBlk, SM4_BLOCK_BYTES);

			pInData += SM4_BLOCK_BYTES;
			pOutData += SM4_BLOCK_BYTES;
			nInSize -= SM4_BLOCK_BYTES;
		}

		::memset(pOutData, 0x00, nOutSize - (nBlocks * SM4_BLOCK_BYTES));

		nRet = SM4_RESULT_OK;
		break;	// exit dummy loop 
	}

	return	nRet;
}
// setup key
void _sm4_setup_key(void *pKey)
{
	int i;
    unsigned int	ulWord;
    unsigned int	K[36];

	// clear !
	::memset(rk, 0x00, sizeof(rk));
	
	// Key Expansion
	::memset(K, 0x00, sizeof(K));
	for (i = 0 ; i < 4; ++i) {
        K[i] =  *((unsigned int*)pKey + i) ^ _sm4_tbl_FK[i];
	}
	for (i = 0 ; i < 32; ++i) {
		ulWord = K[i + 1] ^ K[i + 2] ^ K[i + 3] ^ _sm4_tbl_CK[i];
		_sm4_func_T2(&ulWord);
		rk[i] = K[i + 4] = K[i] ^ ulWord;
	}
}

// enc/dec one block
void _sm4_crypt_blk(void *pData, E_SM4_CRYPT_OPRT eOprt)
{
	int i;
    unsigned int	ulWord;
    unsigned int	X[36];

    ::memcpy(&X[0], pData, sizeof(unsigned int) * 4);
    ::memset(&X[4], 0x00, sizeof(X) - sizeof(unsigned int) * 4);

	for (i = 0; i < 32; ++i) {
		if (eOprt == SM4_OPRT_ENCRYPT) {	// encrypt
			ulWord = X[i + 1] ^ X[i + 2] ^ X[i + 3] ^ rk[i];
		} else {							// decrypt
			ulWord = X[i + 1] ^ X[i + 2] ^ X[i + 3] ^ rk[31 - i];
		}
		_sm4_func_T(&ulWord);
		X[i + 4] = X[i] ^ ulWord;
	}

	for (i = 0; i < 4; ++i) {
        *((unsigned int*)pData + i) = X[35 - i];
	}
}

//  T = L(t(x))
void _sm4_func_T(unsigned int *pWord32)
{
	_sm4_trans_t(pWord32);
	_sm4_trans_L(pWord32);
}

//  T' = L'(t(x))
void _sm4_func_T2(unsigned int *pWord32)
{
	_sm4_trans_t(pWord32);
	_sm4_trans_L2(pWord32);
}

// transration t()
void _sm4_trans_t(unsigned int* pWord32)
{
	unsigned char	*pByte;

	pByte = (unsigned char*)pWord32;
	pByte[0] = _sm4_tbl_Sbox[ pByte[0] ];
	pByte[1] = _sm4_tbl_Sbox[ pByte[1] ];
	pByte[2] = _sm4_tbl_Sbox[ pByte[2] ];
	pByte[3] = _sm4_tbl_Sbox[ pByte[3] ];
}

// transration L()
void _sm4_trans_L(unsigned int *pWord32)
{
    unsigned int	ulWord;

	ulWord = *pWord32;
	ulWord ^= (*pWord32 <<  2) | (*pWord32 >> 30);
	ulWord ^= (*pWord32 << 10) | (*pWord32 >> 22);
	ulWord ^= (*pWord32 << 18) | (*pWord32 >> 14);
	ulWord ^= (*pWord32 << 24) | (*pWord32 >>  8);

	*pWord32 = ulWord;
}

// transration L'()
void _sm4_trans_L2(unsigned int *pWord32)
{
    unsigned int	ulWord;

	ulWord = *pWord32;
	ulWord ^= (*pWord32 << 13) | (*pWord32 >> 19);
	ulWord ^= (*pWord32 << 23) | (*pWord32 >>  9);

	*pWord32 = ulWord;
}

// swap UI32 BE <-> LE
void __swap_words(SM4_BLOCK_DATA *pBlk)
{
	int		i, j;
    unsigned int	ulWord;

	for ( i = 0 ; i < 4 ; ++i ) {
		ulWord = pBlk->d_ui32[i];
		for ( j = 0 ; j < 4 ; ++j ) {
			pBlk->d_ui8[ i * 4 + j] = *((unsigned char*)&ulWord + 3 - j);
		}
	}
}
//40-28-00-06(DS1#0006) Start

/////////////////////////////////////////////////////////////////////
//静态变量初期化
////////////////////////////////////////////////////////////////////
BYTE CDes::parity_tbl[256] = 
{
    0x01,0x01,0x02,0x02,0x04,0x04,0x07,0x07,
    0x08,0x08,0x0b,0x0b,0x0d,0x0d,0x0e,0x0e,
    0x10,0x10,0x13,0x13,0x15,0x15,0x16,0x16,
    0x19,0x19,0x1a,0x1a,0x1c,0x1c,0x1f,0x1f,
    0x20,0x20,0x23,0x23,0x25,0x25,0x26,0x26,
    0x29,0x29,0x2a,0x2a,0x2c,0x2c,0x2f,0x2f,
    0x31,0x31,0x32,0x32,0x34,0x34,0x37,0x37,
    0x38,0x38,0x3b,0x3b,0x3d,0x3d,0x3e,0x3e,
    0x40,0x40,0x43,0x43,0x45,0x45,0x46,0x46,
    0x49,0x49,0x4a,0x4a,0x4c,0x4c,0x4f,0x4f,
    0x51,0x51,0x52,0x52,0x54,0x54,0x57,0x57,
    0x58,0x58,0x5b,0x5b,0x5d,0x5d,0x5e,0x5e,
    0x61,0x61,0x62,0x62,0x64,0x64,0x67,0x67,
    0x68,0x68,0x6b,0x6b,0x6d,0x6d,0x6e,0x6e,
    0x70,0x70,0x73,0x73,0x75,0x75,0x76,0x76,
    0x79,0x79,0x7a,0x7a,0x7c,0x7c,0x7f,0x7f,
    0x80,0x80,0x83,0x83,0x85,0x85,0x86,0x86,
    0x89,0x89,0x8a,0x8a,0x8c,0x8c,0x8f,0x8f,
    0x91,0x91,0x92,0x92,0x94,0x94,0x97,0x97,
    0x98,0x98,0x9b,0x9b,0x9d,0x9d,0x9e,0x9e,
    0xa1,0xa1,0xa2,0xa2,0xa4,0xa4,0xa7,0xa7,
    0xa8,0xa8,0xab,0xab,0xad,0xad,0xae,0xae,
    0xb0,0xb0,0xb3,0xb3,0xb5,0xb5,0xb6,0xb6,
    0xb9,0xb9,0xba,0xba,0xbc,0xbc,0xbf,0xbf,
    0xc1,0xc1,0xc2,0xc2,0xc4,0xc4,0xc7,0xc7,
    0xc8,0xc8,0xcb,0xcb,0xcd,0xcd,0xce,0xce,
    0xd0,0xd0,0xd3,0xd3,0xd5,0xd5,0xd6,0xd6,
    0xd9,0xd9,0xda,0xda,0xdc,0xdc,0xdf,0xdf,
    0xe0,0xe0,0xe3,0xe3,0xe5,0xe5,0xe6,0xe6,
    0xe9,0xe9,0xea,0xea,0xec,0xec,0xef,0xef,
    0xf1,0xf1,0xf2,0xf2,0xf4,0xf4,0xf7,0xf7,
    0xf8,0xf8,0xfb,0xfb,0xfd,0xfd,0xfe,0xfe 
};
BYTE CDes::bit[4] = { 0x08,0x04,0x02,0x01 };
BYTE CDes::h_bit[8] = {0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01 };
BYTE CDes::ip_tbl[64] = 
{
    58,50,42,34,26,18,10, 2,
    60,52,44,36,28,20,12, 4,
    62,54,46,38,30,22,14, 6,
    64,56,48,40,32,24,16, 8,
    57,49,41,33,25,17, 9, 1,
    59,51,43,35,27,19,11, 3,
    61,53,45,37,29,21,13, 5,
    63,55,47,39,31,23,15, 7
};
BYTE CDes::e_tbl[48] = 
{
    32, 1, 2, 3, 4, 5,   4, 5, 6, 7, 8, 9,
     8, 9,10,11,12,13,  12,13,14,15,16,17,
    16,17,18,19,20,21,  20,21,22,23,24,25,
    24,25,26,27,28,29,  28,29,30,31,32, 1 
};
BYTE CDes::pc1_tbl[56] = 
{
    57,49,41,33,25,17, 9,    1,58,50,42,34,26,18,
    10, 2,59,51,43,35,27,   19,11, 3,60,52,44,36,
    63,55,47,39,31,23,15,    7,62,54,46,38,30,22,
    14, 6,61,53,45,37,29,   21,13, 5,28,20,12, 4 
};
BYTE CDes::ls_tbl[16] = {1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1};
BYTE CDes::pc2_tbl[48] = 
{
    14,17,11,24, 1, 5,   3,28,15, 6,21,10,
    23,19,12, 4,26, 8,  16, 7,27,20,13, 2,
    41,52,31,37,47,55,  30,40,51,45,33,48,
    44,49,39,56,34,53,  46,42,50,36,29,32 
};
BYTE CDes::s_tbl[8][64] = {
    14, 4,13, 1, 2,15,11, 8, 3,10, 6,12, 5, 9, 0, 7,
     0,15, 7, 4,14, 2,13, 1,10, 6,12,11, 9, 5, 3, 8,
     4, 1,14, 8,13, 6, 2,11,15,12, 9, 7, 3,10, 5, 0,
    15,12, 8, 2, 4, 9, 1, 7, 5,11, 3,14,10, 0, 6,13,

    15, 1, 8,14, 6,11, 3, 4, 9, 7, 2,13,12, 0, 5,10,
     3,13, 4, 7,15, 2, 8,14,12, 0, 1,10, 6, 9,11, 5,
     0,14, 7,11,10, 4,13, 1, 5, 8,12, 6, 9, 3, 2,15,
     13, 8,10, 1, 3,15, 4, 2,11, 6, 7,12, 0, 5,14, 9,

     10, 0, 9,14, 6, 3,15, 5, 1,13,12, 7,11, 4, 2, 8,
     13, 7, 0, 9, 3, 4, 6,10, 2, 8, 5,14,12,11,15, 1,
     13, 6, 4, 9, 8,15, 3, 0,11, 1, 2,12, 5,10,14, 7,
     1,10,13, 0, 6, 9, 8, 7, 4,15,14, 3,11, 5, 2,12,

     7,13,14, 3, 0, 6, 9,10, 1, 2, 8, 5,11,12, 4,15,
     13, 8,11, 5, 6,15, 0, 3, 4, 7, 2,12, 1,10,14, 9,
     10, 6, 9, 0,12,11, 7,13,15, 1, 3,14, 5, 2, 8, 4,
     3,15, 0, 6,10, 1,13, 8, 9, 4, 5,11,12, 7, 2,14,

     2,12, 4, 1, 7,10,11, 6, 8, 5, 3,15,13, 0,14, 9,
     14,11, 2,12, 4, 7,13, 1, 5, 0,15,10, 3, 9, 8, 6,
     4, 2, 1,11,10,13, 7, 8,15, 9,12, 5, 6, 3, 0,14,
     11, 8,12, 7, 1,14, 2,13, 6,15, 0, 9,10, 4, 5, 3,

     12, 1,10,15, 9, 2, 6, 8, 0,13, 3, 4,14, 7, 5,11,
     10,15, 4, 2, 7,12, 9, 5, 6, 1,13,14, 0,11, 3, 8,
     9,14,15, 5, 2, 8,12, 3, 7, 0, 4,10, 1,13,11, 6,
     4, 3, 2,12, 9, 5,15,10,11,14, 1, 7, 6, 0, 8,13,

     4,11, 2,14,15, 0, 8,13, 3,12, 9, 7, 5,10, 6, 1,
     13, 0,11, 7, 4, 9, 1,10,14, 3, 5,12, 2,15, 8, 6,
     1, 4,11,13,12, 3, 7,14,10,15, 6, 8, 0, 5, 9, 2,
     6,11,13, 8, 1, 4,10, 7, 9, 5, 0,15,14, 2, 3,12,

     13, 2, 8, 4, 6,15,11, 1,10, 9, 3,14, 5, 0,12, 7,
     1,15,13, 8,10, 3, 7, 4,12, 5, 6,11, 0,14, 9, 2,
     7,11, 4, 1, 9,12,14, 2, 0, 6,10,13,15, 3, 5, 8,
     2, 1,14, 7, 4,10, 8,13,15,12, 9, 0, 3, 5, 6,11
};
BYTE CDes::p_tbl[32] = 
{
    16, 7,20,21,    29,12,28,17,
        1,15,23,26,     5,18,31,10,
        2, 8,24,14,    32,27, 3, 9,
        19,13,30, 6,    22,11, 4,25 
};
BYTE CDes::ip1_tbl[64] = {
    40, 8,48,16,56,24,64,32,
        39, 7,47,15,55,23,63,31,
        38, 6,46,14,54,22,62,30,
        37, 5,45,13,53,21,61,29,
        36, 4,44,12,52,20,60,28,
        35, 3,43,11,51,19,59,27,
        34, 2,42,10,50,18,58,26,
        33, 1,41, 9,49,17,57,25 
};

void CDes::p_tbl_sub()
{
    int i;
    for(i=0;i<4;i++)
        p_Convert[loop][i] = 0;
    for(i=0;i<32;i++)
    {
        id = p_tbl[i];
        by1 = BYTE((id-1) / 8);
        bb1 = BYTE((id-1) % 8);
        s_bit = p_data[by1] & h_bit[bb1];

        if (s_bit != 0)
        {
            by2 = BYTE( i / 8 );
            bb2 = BYTE( i % 8 );
            p_Convert[loop][by2] |= h_bit[bb2];
        }
    }
    for(i=0;i<4;i++)
        r0_data[loop+1][i] = 0;
    for(i=0;i<4;i++)
        r0_data[loop+1][i] = l0_data[loop][i] ^ p_Convert[loop][i];
    for(i=0;i<4;i++)
        l0_data[loop+1][i] = r0_data[loop][i];
}

void CDes::s_tbl_sub()
{
    int i;
    int pnt;

    for(i=0;i<6;i++)
        s_box[i] = 0;
    if (p == 0)
    {
        pnt = loop;
    }
    else
    {
        pnt = 15 - loop;
    }
    for(i=0;i<6;i++)
        s_box[i] = r0_Convert[loop][i] ^ k1_Convert[pnt][i];
    for(i=0;i<4;i++)
        p_data[i] = 0;

    /**** 0 ****/
    if ((s_box[0] & 0x80) == 0)
    {
        if ((s_box[0] & 0x04) == 0)
        { g_no = 0; } else { g_no = 1; }
    }
    else
    {
        if ((s_box[0] & 0x04) == 0)
        { g_no = 2; } else { g_no = 3; }
    }
    r_no = (s_box[0] & 0x78) >> 3;
    s_dt = (s_tbl[0][g_no * 16 + r_no]) << 4;
    p_data[0] |= s_dt & 0xf0;
    /**** 1 ****/
    if ((s_box[0] & 0x02) == 0)
    {
        if ((s_box[1] & 0x10) == 0)
        { g_no = 0; } else { g_no = 1; }
    }
    else
    {
        if ((s_box[1] & 0x10) == 0)
        { g_no = 2; } else { g_no = 3; }
    }
    r_no  = (s_box[0] & 0x01) << 3;
    r_no |= (s_box[1] & 0xe0) >> 5;
    s_dt = (s_tbl[1][g_no * 16 + r_no]) & 0x0f;
    p_data[0] |= s_dt;
    /**** 2 ****/
    if ((s_box[1] & 0x08) == 0)
    {
        if ((s_box[2] & 0x40) == 0)
        { g_no = 0; } else { g_no = 1; }
    }
    else
    {
        if ((s_box[2] & 0x40) == 0)
        { g_no = 2; } else { g_no = 3; }
    }
    r_no  = (s_box[1] & 0x07) << 1;
    r_no |= (s_box[2] & 0x80) >> 7;
    s_dt = (s_tbl[2][g_no * 16 + r_no]) << 4;
    p_data[1] |= s_dt & 0xf0;
    /**** 3 ****/
    if ((s_box[2] & 0x20) == 0)
    {
        if ((s_box[2] & 0x01) == 0)
        { g_no = 0; } else { g_no = 1; }
    }
    else
    {
        if ((s_box[2] & 0x01) == 0)
        { g_no = 2; } else { g_no = 3; }
    }
    r_no = (s_box[2] & 0x1e) >> 1;
    s_dt = (s_tbl[3][g_no * 16 + r_no]) & 0x0f;
    p_data[1] |= s_dt;

    /**** 4 ****/
    if ((s_box[3] & 0x80) == 0)
    {
        if ((s_box[3] & 0x04) == 0)
        { g_no = 0; } else { g_no = 1; }
    }
    else
    {
        if ((s_box[3] & 0x04) == 0)
        { g_no = 2; } else { g_no = 3; }
    }
    r_no = (s_box[3] & 0x78) >> 3;
    s_dt = (s_tbl[4][g_no * 16 + r_no]) << 4;
    p_data[2] |= s_dt & 0xf0;
    /**** 5 ****/
    if ((s_box[3] & 0x02) == 0)
    {
        if ((s_box[4] & 0x10) == 0)
        { g_no = 0; } else { g_no = 1; }
    }
    else
    {
        if ((s_box[4] & 0x10) == 0)
        { g_no = 2; } else { g_no = 3; }
    }
    r_no  = (s_box[3] & 0x01) << 3;
    r_no |= (s_box[4] & 0xe0) >> 5;
    s_dt = (s_tbl[5][g_no * 16 + r_no]) & 0x0f;
    p_data[2] |= s_dt;
    /**** 6 ****/
    if ((s_box[4] & 0x08) == 0)
    {
        if ((s_box[5] & 0x40) == 0)
        { g_no = 0; } else { g_no = 1; }
    }
    else
    {
        if ((s_box[5] & 0x40) == 0)
        { g_no = 2; } else { g_no = 3; }
    }
    r_no  = (s_box[4] & 0x07) << 1;
    r_no |= (s_box[5] & 0x80) >> 7;
    s_dt = (s_tbl[6][g_no * 16 + r_no]) << 4;
    p_data[3] |= s_dt & 0xf0;
    /**** 7 ****/
    if ((s_box[5] & 0x20) == 0)
    {
        if ((s_box[5] & 0x01) == 0)
        { g_no = 0; } else { g_no = 1; }
    }
    else
    {
        if ((s_box[5] & 0x01) == 0)
        { g_no = 2; } else { g_no = 3; }
    }
    r_no = (s_box[5] & 0x1e) >> 1;
    s_dt = (s_tbl[7][g_no * 16 + r_no]) & 0x0f;
    p_data[3] |= s_dt;
}

void CDes::pc2_tbl_sub()
{
    int i;
    for(i=0;i<6;i++)
        k1_Convert[loop][i] = 0;
    for(i=0;i<48;i++)
    {
        id = pc2_tbl[i];
        if (id < 29)
        {
            by1 = BYTE((id-1) / 8);
            bb1 = BYTE((id-1) % 8);
            s_bit = c0_Convert[loop+1][by1] & h_bit[bb1];
            if (s_bit != 0)
            {
                by2 = BYTE( i / 8 );
                bb2 = BYTE( i % 8 );
                k1_Convert[loop][by2] |= h_bit[bb2];
            }
        }
        else
        {
            by1 = BYTE((id-29) / 8);
            bb1 = BYTE((id-29) % 8);
            s_bit = d0_Convert[loop+1][by1] & h_bit[bb1];
            if (s_bit != 0)
            {
                by2 = BYTE( i / 8 );
                bb2 = BYTE( i % 8 );
                k1_Convert[loop][by2] |= h_bit[bb2];
            }
        }
    }
}

void CDes::ls_tbl_sub()
{
    int i;
    ls = ls_tbl[loop];
    for(i=0;i<4;i++)
    {
        c0_Convert[loop+1][i]  = c0_Convert[loop][i] << ls;
        d0_Convert[loop+1][i]  = d0_Convert[loop][i] << ls;
    }
    for(i=0;i<3;i++)
    {
        c0_Convert[loop+1][i] |= c0_Convert[loop][i+1] >> (8-ls);
        d0_Convert[loop+1][i] |= d0_Convert[loop][i+1] >> (8-ls);
    }
    c0_Convert[loop+1][3] |= (c0_Convert[loop][0] >> (4-ls)) & 0xf0;
    d0_Convert[loop+1][3] |= (d0_Convert[loop][0] >> (4-ls)) & 0xf0;
}

void CDes::pc1_tbl_sub()
{
    int i;
    for(i=0;i<7;i++)
        pc1_data[i] = 0;
    for(i=0;i<56;i++)
    {
        id  = pc1_tbl[i];
        by1 = BYTE((id-1) / 8);
        bb1 = BYTE((id-1) % 8);
        s_bit = key_dt[by1] & h_bit[bb1];

        if (s_bit != 0)
        {
            by2 = BYTE( i / 8 );
            bb2 = BYTE( i % 8 );
            pc1_data[by2] |= h_bit[bb2];
        }
    }
    for(i=0;i<4;i++)
        c0_Convert[0][i] = pc1_data[i];
    c0_Convert[0][3] &= 0xf0;

    for (i=0;i<3;i++)
    {
        d0_Convert[0][i]  = pc1_data[i+3] << 4;
        d0_Convert[0][i] |= pc1_data[i+4] >> 4;
    }
    d0_Convert[0][3]  = pc1_data[3+3] << 4;
}

void CDes::e_tbl_sub()
{
    int i;
    for(i=0;i<6;i++)
        r0_Convert[loop][i] = 0;
    for(i=0;i<48;i++)
    {
        id  = e_tbl[i];
        by1 = BYTE((id-1) / 8);
        bb1 = BYTE((id-1) % 8);
        s_bit = r0_data[loop][by1] & h_bit[bb1];

        if (s_bit != 0)
        {
            by2 = BYTE( i / 8 );
            bb2 = BYTE( i % 8 );
            r0_Convert[loop][by2] |= h_bit[bb2];
        }
    }
}

void CDes::ip_tbl_sub()
{
    int i;
    for(i=0;i<8;i++)
        ip_data[i] = 0;
    for(i=0;i<64;i++)
    {
        id  = ip_tbl[i];
        by1 = BYTE((id-1) / 8);
        bb1 = BYTE((id-1) % 8);
        s_bit = text[by1] & h_bit[bb1];

        if (s_bit != 0)
        {
            by2 = BYTE( i / 8 );
            bb2 = BYTE( i % 8 );
            ip_data[by2] |= h_bit[bb2];
        }
    }
    for(i=0;i<4;i++)
    {
        l0_data[0][i] = ip_data[i];
        r0_data[0][i] = ip_data[i+4];
    }
}

void CDes::des()
{
    int i;
    ip_tbl_sub();
    pc1_tbl_sub();
    for(i=0;i<4;i++)
        p_data[i] = 0;
    for(loop=0;loop<16;loop++)
    {
        ls_tbl_sub();
        pc2_tbl_sub();
    }
    for(loop=0;loop<16;loop++)
    {
        e_tbl_sub();
        s_tbl_sub();
        p_tbl_sub();
    }
    for(i=0;i<8;i++)
        output_data[i] = 0;
    for(i=0;i<64;i++)
    {
        id = ip1_tbl[i];
        if (id < 33)
        {
            by1 = BYTE((id-1) / 8);
            bb1 = BYTE((id-1) % 8);
            s_bit = r0_data[16][by1] & h_bit[bb1];
            by2 = BYTE( i / 8 );
            bb2 = BYTE( i % 8 );
            if (s_bit != 0)
                output_data[by2] |= h_bit[bb2];
        }
        else
        {
            by1 = BYTE((id-33) / 8);
            bb1 = BYTE((id-33) % 8);
            s_bit = l0_data[16][by1] & h_bit[bb1];
            by2 = BYTE( i / 8 );
            bb2 = BYTE( i % 8 );
            if (s_bit != 0)
                output_data[by2] |= h_bit[bb2];
        }
    }
}

void CDes::key_sub(LPBYTE pbyKeyCode)
{
    if (pbyKeyCode[0] != NULL)
    {
        for(int i=0;i<8;i++)
        {
            key_dt[i] = parity_tbl[(char2bin(pbyKeyCode[i*2])<<4) + char2bin(pbyKeyCode[i*2+1])];
        }
    }
}

void CDes::text_sub(LPBYTE pbyInCode)
{
    if (pbyInCode[0] != NULL)
    {
        for(int i=0;i<8;i++)
        {
            text[i]  = (char2bin(pbyInCode[i*2]) << 4) + char2bin(pbyInCode[i*2+1]);
        }
    }
}
//byChar={0,1,2,3,4,5,6,7,8,9, A, B, C, D, E, F}
//nRet  ={0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15}
BYTE CDes::char2bin(BYTE byChar)
{
    BYTE nRet = byChar & 0x0f;
    if(byChar > '9' || byChar < '0')
        return nRet + 9;
    return nRet;
}

int CDes::EntryDes(LPBYTE pbyText, WORD wInSize, LPBYTE pbyKey, WORD wKeySize, WORD wFlag, LPBYTE pbyOutCode, WORD wOutSize)
{
    int i;
    BYTE    byTextCode[16] ;
    BYTE    byKeyCode[16] ;
    BYTE    byWork ;

    memset( byTextCode,NULL,sizeof(byTextCode));
    memset( byKeyCode, NULL,sizeof(byKeyCode) );
    for( i = 0; i < wInSize; i++ ) {
        byWork = pbyText[i] >> 4 ;
        if ( (byWork >= 0x00) && (byWork <= 0x09) ) {
            byTextCode[i*2] = byWork + 0x30 ;
        }else{
            byTextCode[i*2] = byWork + 0x57 ;
        }

        byWork = pbyText[i] & 0x0f ;
        if ( (byWork >= 0x00) && (byWork <= 0x09) ) {
            byTextCode[i*2+1] = byWork + 0x30 ;
        }else{
            byTextCode[i*2+1] = byWork + 0x57 ;
        }
    }

    for( i = 0; i < wKeySize; i++ ) {
        byWork = pbyKey[i] >> 4 ;
        if ( (byWork >= 0x00) && (byWork <= 0x09) ) {
            byKeyCode[i*2] = byWork + 0x30 ;
        }else{
            byKeyCode[i*2] = byWork + 0x57 ;
        }

        byWork = pbyKey[i] & 0x0f ;
        if ( (byWork >= 0x00) && (byWork <= 0x09) ) {
            byKeyCode[i*2+1] = byWork + 0x30 ;
        }else{
            byKeyCode[i*2+1] = byWork + 0x57 ;
        }
    }


    for( i = 0;i < 16; i++ )
    {
        text[i] = 0 ;
    }
    text_sub( byTextCode );//压缩后的cdk又经解压缩16
    key_sub( byKeyCode );//初始cdk又经解压缩16

    if(wFlag==CIPHER){
        p = 0;    // Flag 加密
        des();
    }
    if(wFlag==COMPOUND){
        p = 1;   // Flag 解密
        des();
    }
    memcpy(pbyOutCode,output_data,wOutSize) ;
    return( 1 );
}

void CEncrypt::Compress(BYTE *Source, BYTE *Dest, int Size)
{
    int High,Low;
    for(int i = 0; i<Size; i+=2)
    {
        if(Source[i]>='0' && Source[i]<='9')
            High = Source[i] - '0'; 
        else
            High = Source[i] - 'A' + 10;	
        if(Source[i+1]>='0' && Source[i+1]<='9')
            Low = Source[i+1]-'0'; 
        else
            Low = Source[i+1]-'A'+10;	
        *Dest++ = BYTE(High*16+Low);
    }
}

void CEncrypt::Expand(BYTE *Source, BYTE *Dest, int Size)
{
    static char Number[]="0123456789ABCDEF";
    for(int i=0; i<Size; i++)
    {
        *Dest++ = Number[Source[i]>>4];
        *Dest++ = Number[Source[i]&0x0f];
    }
}

void CEncrypt::DataEncrypt(BYTE *data, BYTE *Key, BYTE *ResultCode)
{
    des.EntryDes(data, 8, Key, 8, CIPHER, ResultCode, 8);
}

void CEncrypt::DataDecrypt(BYTE *data, BYTE *Key, BYTE *ResultCode)
{
    des.EntryDes(data, 8, Key, 8, COMPOUND, ResultCode, 8);
}

void CEncrypt::DataEncrypt3DES(BYTE *data, BYTE *Key, BYTE *ResultCode)
{
    BYTE WorkingBuffer[8];
    des.EntryDes(data, 8, Key, 8, CIPHER, ResultCode, 8);               //左密钥加密
    des.EntryDes(ResultCode, 8, Key + 8, 8, COMPOUND, WorkingBuffer, 8);//右密钥解密
    des.EntryDes(WorkingBuffer, 8, Key, 8, CIPHER, ResultCode, 8);      //左密钥加密
}

void CEncrypt::DataDecrypt3DES(BYTE *data, BYTE *Key, BYTE *ResultCode)
{
    BYTE WorkingBuffer[8];
    des.EntryDes(data, 8, Key, 8, COMPOUND, ResultCode, 8);             //左密钥解密
    des.EntryDes(ResultCode, 8, Key + 8, 8, CIPHER, WorkingBuffer, 8);  //右密钥加密
    des.EntryDes(WorkingBuffer, 8, Key, 8, COMPOUND, ResultCode, 8);    //左密钥解密
}

/////////////////////////////////////////////////////////////////////////////
// 关数种别= 内部关数
// 关数名称= 3.0C场合 UID_KBPK 解密加密
// 呼出形式=
// 机能详细=												
// V-R-T   = 40-23-00-00													
// History = 2014/05/30	 40-23-00-00 BHH(zhagnz) 40-23-00-03(DS1#0003) 	
// History = 2014/08/06,SP ,40-23-00-00,BHH(guojy):		40-23-00-06(PG#00037)
//////////////////////////////////////////////////////////////////////////////
void CEncrypt::UID_KBPKEncrypt3DES(BYTE *data, BYTE *Key, BYTE *ResultCode)			//40-23-00-06(PG#00037)				
{
    BYTE WorkingBuffer[8];
    des.EntryDes(data, 8, Key, 8, CIPHER, ResultCode, 8);						//左密钥加密//40-23-00-06(PG#00037)	
    des.EntryDes(ResultCode, 8, Key + 8, 8, COMPOUND, WorkingBuffer, 8);		//中密钥解密//40-23-00-06(PG#00037)	
   des.EntryDes(WorkingBuffer, 8, Key + 16, 8, CIPHER, ResultCode, 8);			//右密钥加密//40-23-00-06(PG#00037)	
}

BOOL CEncrypt::KeyCheck(BYTE *data, BYTE *Key, BYTE *NewKey, BYTE *CheckCode)
{
    BOOL bOK = TRUE;
    DataDecrypt(data, Key, NewKey);
    if(CheckCode)
    {
        BYTE EncryptData[8],ResultCode[8];
        memset(EncryptData, 0, sizeof(EncryptData));
        DataEncrypt(EncryptData, NewKey, ResultCode);
        bOK = memcmp (ResultCode, CheckCode, 8) == 0;
        memcpy(CheckCode, ResultCode, 8);
    }
    return bOK;
}

BOOL CEncrypt::KeyCheck3DES(BYTE *data, BYTE *Key, BYTE *NewKey, BYTE *CheckCode)
{
    BOOL bOK = TRUE;
    DataDecrypt3DES(data, Key, NewKey);
    DataDecrypt3DES(data + 8, Key, NewKey + 8);
    if(CheckCode)
    {
        BYTE EncryptData[8],ResultCode[8];
        memset(EncryptData, 0, sizeof(EncryptData));
        DataEncrypt3DES(EncryptData, NewKey, ResultCode);
        bOK = memcmp (ResultCode, CheckCode, 8) == 0;
        memcpy(CheckCode, ResultCode, 8);
    }
    return bOK;
}

BOOL CEncrypt::KeyCheck3DESEx(BYTE *data, BYTE *Key, BYTE *NewKey, BYTE *CheckCode)
{
    BOOL bOK = TRUE;
    DataDecrypt3DES(data, Key, NewKey);
    if(CheckCode)
    {
        BYTE EncryptData[8],ResultCode[8];
        memset(EncryptData, 0, sizeof(EncryptData));
        DataEncrypt(EncryptData, NewKey, ResultCode);
        bOK = memcmp (ResultCode, CheckCode, 8) == 0;
        memcpy(CheckCode, ResultCode, 8);
    }
    return bOK;
}

void CEncrypt::PinEncryptData(BYTE *Pin, int PinSize, BYTE *Pan, int PanSize, BYTE byFill, int PinType, BYTE *EncryptData)
{
    BYTE WorkingBuffer[16], PinData[8],PanData[8];
    //填充字符校正(0x00~0x0F)->(0x30~0x39,0x41~0x46)
    byFill += byFill > 0x09 ? ('A' - 10) : '0';     
    //PinData生成
    memset(WorkingBuffer, byFill, sizeof(WorkingBuffer));
    if(PinType == PIN_ANSIX98)
    {
        memcpy(WorkingBuffer + 2, Pin, PinSize);
        Compress(WorkingBuffer, PinData, 16);
        PinData[0] = BYTE(PinSize);

        //PanData生成
        memset(WorkingBuffer, '0', sizeof(WorkingBuffer));
        if(PanSize >= 13)
        {
            memcpy(WorkingBuffer+4, Pan+PanSize-13, 12);
        }
        else if(PanSize > 1)
        {
            memcpy(WorkingBuffer+17-PanSize, Pan, PanSize-1);
        }
        Compress(WorkingBuffer, PanData, 16);
        for(int i=0; i<8; i++)
            EncryptData[i] = PanData[i]^PinData[i];
    }
    else
    {
        memcpy(WorkingBuffer, Pin, PinSize);
        Compress(WorkingBuffer, PinData, 16);
        memcpy(EncryptData, PinData, 8);
    }
}

BOOL CEncrypt::PinCheck(BYTE *Pin, int PinSize, BYTE *Pan, int PanSize, BYTE byFill, int PinType, BYTE *Key, BYTE *CheckCode)
{
    BOOL bOK = TRUE;
    BYTE ResultCode[8], EncryptData[8];
    PinEncryptData(Pin, PinSize, Pan, PanSize, byFill, PinType, EncryptData);
    DataEncrypt(EncryptData, Key, ResultCode);
    bOK = memcmp (ResultCode, CheckCode, 8) == 0;
    memcpy(CheckCode, ResultCode, 8);
    return bOK;
}

BOOL CEncrypt::PinCheck3DES(BYTE *Pin, int PinSize, BYTE *Pan, int PanSize, BYTE byFill, int PinType, BYTE *Key, BYTE *CheckCode)
{
    BOOL bOK = TRUE;
    BYTE ResultCode[8], EncryptData[8];
    PinEncryptData(Pin, PinSize, Pan, PanSize, byFill, PinType, EncryptData);
    DataEncrypt3DES(EncryptData, Key, ResultCode);
    bOK = memcmp (ResultCode, CheckCode, 8) == 0;
    memcpy(CheckCode, ResultCode, 8);
    return bOK;
}

BOOL CEncrypt::MacCheck(BYTE *MacData, int DataSize, BYTE byFill, BYTE *Key, BYTE *CheckCode)
{
    BOOL bOK = TRUE;
    BYTE WorkingBuffer[8], ResultCode[8], TableData[8];
    memset(ResultCode, 0, sizeof(ResultCode));
    byFill = byFill << 4 | byFill;
    int LoopCount = (DataSize+7)/8;
    int LastCount = DataSize%8;
    for(int i=0; i<LoopCount; i++)
    {
        if(i<LoopCount-1 || LastCount == 0)
        {
            memcpy(TableData, MacData + i*8, 8);
        }
        else
        {
            memset(TableData, byFill, sizeof(TableData));
            memcpy(TableData, MacData + i*8, LastCount);
//            TableData[LastCount] = 0x80;
        }
        for(int j=0; j<8; j++)
            WorkingBuffer[j] = ResultCode[j]^TableData[j];
        DataEncrypt(WorkingBuffer, Key, ResultCode);
    }
    bOK = memcmp (ResultCode, CheckCode, 8) == 0;
    memcpy(CheckCode, ResultCode, 8);
    return bOK;
}

BOOL CEncrypt::MacCheck3DES(BYTE *MacData, int DataSize, BYTE byFill, BYTE *Key, BYTE *CheckCode)
{
    BOOL bOK = TRUE;
    BYTE WorkingBuffer[8], ResultCode[8];
    MacCheck(MacData, DataSize, byFill, Key, ResultCode);   //左密钥CBC加密
    DataDecrypt(ResultCode, Key + 8, WorkingBuffer);        //右密钥解密
    DataEncrypt(WorkingBuffer, Key, ResultCode);            //左密钥加密
    bOK = memcmp (ResultCode, CheckCode, 8) == 0;
    memcpy(CheckCode, ResultCode, 8);
    return bOK;
}

/////////////////////////////////////////////////////////////////////////////
// Func Category = Internal
// Func Name = PCI3.0 Situation Use of Edit MacData												
// V-R-T   = 40-23-00-00													
// History = 2014/05/30	 40-23-00-00 BHH(zhagnz) 40-23-00-03(DS1#0003) 
// History = 2014/07/01  40-23-00-00,BHH(zhangz) 40-23-00-03(PG#00007)
//////////////////////////////////////////////////////////////////////////////
BOOL CEncrypt::MacCreat3DES(BYTE *MacData, int DataSize, BYTE byFill, BYTE *Key, BYTE *MacCode)
{
	BYTE ResultCode[8];
	BYTE WorkingBuffer[8];
    MacCheck(MacData, DataSize, byFill, Key, ResultCode);   //Lefe Secret key CBC Encrypt
    DataDecrypt(ResultCode, Key+8, WorkingBuffer);          //Middle Secret key Decrypt
    DataEncrypt(WorkingBuffer, Key+16, ResultCode);         //Rigth Secret key Encrtpt
	memcpy(MacCode, ResultCode, 8);							//40-23-00-03(PG#00007)					
    return TRUE;											//40-23-00-03(PG#00007)
}

/////////////////////////////////////////////////////////////////////////////
// Func Category = Internal
// Func Name = SM4 Encryption (ECB mode)												
// V-R-T   = 40-28-00-00													
// History = 2015/02/26,SP ,40-28-00-00,BHH(lipeng) 40-28-00-06(DS1#0006)
//////////////////////////////////////////////////////////////////////////////
int CEncrypt::sm4_enc_ecb(unsigned char *pInput, int nInSize, unsigned char *pOutput, int nOutSize, unsigned char *pKey)
{
	return	_sm4_crypt(SM4_OPRT_ENCRYPT, SM4_MODE_ECB,
						pInput, nInSize, pOutput, nOutSize, pKey);
}

/////////////////////////////////////////////////////////////////////////////
// Func Category = Internal
// Func Name = SM4 Decryption (ECB mode)												
// V-R-T   = 40-28-00-00													
// History = 2015/02/26,SP ,40-28-00-00,BHH(lipeng) 40-28-00-06(DS1#0006)
//////////////////////////////////////////////////////////////////////////////
int CEncrypt::sm4_dec_ecb(unsigned char *pInput, int nInSize, unsigned char *pOutput, int nOutSize, unsigned char *pKey)
{
	return	_sm4_crypt(SM4_OPRT_DECRYPT, SM4_MODE_ECB,
						pInput, nInSize, pOutput, nOutSize, pKey);
}

/////////////////////////////////////////////////////////////////////////////
// Func Category = Internal
// Func Name = SM4 Encryption (CBC mode)												
// V-R-T   = 40-28-00-00													
// History = 2015/02/26,SP ,40-28-00-00,BHH(lipeng) 40-28-00-06(DS1#0006)
//////////////////////////////////////////////////////////////////////////////
int CEncrypt::sm4_enc_cbc(unsigned char *pInput, int nInSize, unsigned char *pOutput, int nOutSize, unsigned char *pKey, unsigned char *pIV)
{
	return	_sm4_crypt(SM4_OPRT_ENCRYPT, SM4_MODE_CBC,
						pInput, nInSize, pOutput, nOutSize, pKey, pIV);
}

/////////////////////////////////////////////////////////////////////////////
// Func Category = Internal
// Func Name = SM4 Decryption (CBC mode)												
// V-R-T   = 40-28-00-00													
// History = 2015/02/26,SP ,40-28-00-00,BHH(lipeng) 40-28-00-06(DS1#0006)
//////////////////////////////////////////////////////////////////////////////
int CEncrypt::sm4_dec_cbc(unsigned char *pInput, int nInSize, unsigned char *pOutput, int nOutSize, unsigned char *pKey, unsigned char *pIV)
{
	return	_sm4_crypt(SM4_OPRT_DECRYPT, SM4_MODE_CBC,
						pInput, nInSize, pOutput, nOutSize, pKey, pIV);
}
