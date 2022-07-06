/****************************************************************************
*                                                                           *
*xfspin.h XFS - Personal Identification Number Keypad (PIN) definitions     *
*                                                                           *
*            Version 3.03  (24/09/04)                                       *
*                                                                           *
****************************************************************************/
#ifndef __INC_XFSPINCHN__H
#define __INC_XFSPINCHN__H

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "XFSAPI.H"

/*   be aware of alignment   */
#pragma pack(push,1)


/* values of WFSPINCAPS.wClass */
/* values of WFSPINCAPS.wClass */

/* values of PROTCHNIN.wCommand */
#define WFS_CMD_ENC_IO_CHN_DIGEST							(0x0001)
#define WFS_CMD_ENC_IO_CHN_SET_SM2_PARAM					(0x0002)
#define WFS_CMD_ENC_IO_CHN_IMPORT_SM2_PUBLIC_KEY			(0x0003)
#define WFS_CMD_ENC_IO_CHN_SIGN								(0x0004)
#define WFS_CMD_ENC_IO_CHN_VERIFY							(0x0005)
#define WFS_CMD_ENC_IO_CHN_EXPORT_SM2_ISSUER_SIGNED_ITEM	(0x0006)
#define WFS_CMD_ENC_IO_CHN_GENERATE_SM2_KEY_PAIR			(0x0007)
#define WFS_CMD_ENC_IO_CHN_EXPORT_SM2_EPP_SIGNED_ITEM		(0x0008)
#define WFS_CMD_ENC_IO_CHN_IMPORT_SM2_SIGNED_SM4_KEY		(0x0009)

#define PROT_CHN_RESULT_OFFSET (0)

/* values of PROTCHNOUT.wResult */
#define PROT_CHN_SUCCESS						(0)
#define PROT_CHN_ERR_INVALID_CMD				(-(PROT_CHN_RESULT_OFFSET + 1))
#define PROT_CHN_ERR_INVALID_DATA				(-(PROT_CHN_RESULT_OFFSET + 2))
#define PROT_CHN_ERR_INVALID_KEY				(-(PROT_CHN_RESULT_OFFSET + 3))

/* values of PROTCHNDIGESTOUTPUT.wResult, PROTCHNIMPORTSM2PUBLICKEYOUT.wResult, PROTCHNEXPORTSM2ISSUERSIGNEDITEMOUT.wResult, PROTCHNEXPORTSM2EPPSIGNEDITEMOUT.wResult and PROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT.wResult */
#define PROT_CHN_ERR_PIN_ACCESSDENIED			(-(PROT_CHN_RESULT_OFFSET + 4))

/* values of PROTCHNIMPORTSM2PUBLICKEYOUT.wResult, PROTCHNDIGESTOUT.wResult, PROTCHNEXPORTSM2ISSUERSIGNEDITEMOUT.wResult, PROTCHNEXPORTSM2EPPSIGNEDITEMOUT.wResult and PROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT.wResult */
#define PROT_CHN_ERR_PIN_KEYNOTFOUND			(-(PROT_CHN_RESULT_OFFSET + 5))

/* values of PROTCHNIMPORTSM2PUBLICKEYOUT.wResult, PROTCHNDIGESTOUT.wResult and PROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT.wResult */
#define PROT_CHN_ERR_PIN_USEVIOLATION			(-(PROT_CHN_RESULT_OFFSET + 6))
#define PROT_CHN_ERR_PIN_INVALIDKEYLENGTH		(-(PROT_CHN_RESULT_OFFSET + 7))

/* additional values of PROTCHNIMPORTSM2PUBLICKEYOUT.wResult and PROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT.wResult */
#define PROT_CHN_ERR_PIN_DUPLICATEKEY			(-(PROT_CHN_RESULT_OFFSET + 8))
#define PROT_CHN_ERR_PIN_SIG_NOT_SUPP			(-(PROT_CHN_RESULT_OFFSET + 9))
#define PROT_CHN_ERR_PIN_SIGNATUREINVALID		(-(PROT_CHN_RESULT_OFFSET + 10))

/* additional values of PROTCHNSIGNOUT.wResult and PROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT.wResult */
#define PROT_CHN_ERR_PIN_MODENOTSUPPORTED		(-(PROT_CHN_RESULT_OFFSET + 11))
#define PROT_CHN_ERR_PIN_KEYNOVALUE				(-(PROT_CHN_RESULT_OFFSET + 12))
#define PROT_CHN_ERR_PIN_NOCHIPTRANSACTIVE		(-(PROT_CHN_RESULT_OFFSET + 13))
#define PROT_CHN_ERR_PIN_ALGORITHMNOTSUPP		(-(PROT_CHN_RESULT_OFFSET + 14))

/* values of PROTCHNVERIFYOUT.wResult */
#define PROT_CHN_ERR_PIN_SIGNATUREERROR			(-(PROT_CHN_RESULT_OFFSET + 15))

/* values of PROTCHNEXPORTSM2ISSUERSIGNEDITEMOUT.wResult */
#define PROT_CHN_ERR_PIN_NOPRIVATEKEY			(-(PROT_CHN_RESULT_OFFSET + 16))

/* values of PROTCHNGENERATESM2KEYOUT.wResult */
#define PROT_CHN_ERR_PIN_INVALID_MOD_LEN		(-(PROT_CHN_RESULT_OFFSET + 17))
#define PROT_CHN_ERR_PIN_KEY_GENERATION_ERROR	(-(PROT_CHN_RESULT_OFFSET + 18))

/* values of PROTCHNEXPORTSM2EPPSIGNEDITEMOUT.wResult */
#define PROT_CHN_ERR_PIN_NOSM2KEYPAIR			(-(PROT_CHN_RESULT_OFFSET + 19))

/* values of PROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT.wResult */
#define PROT_CHN_ERR_PIN_NOKEYRAM				(-(PROT_CHN_RESULT_OFFSET + 20))
#define PROT_CHN_ERR_PIN_RANDOMINVALID			(-(PROT_CHN_RESULT_OFFSET + 21))

/* values of PROTCHNDIGESTIN.wHashAlgorithm */
#define PROT_CHN_HASH_SM3_DIGEST				(0x0001)

/* values for PROTCHNIMPORTSM2PUBLICKEYIN.dwUse */
#define PROT_CHN_USESM2PUBLIC					(0x00000001)
#define PROT_CHN_USESM2PUBLICVERIFY				(0x00000002)

/* values of PROTCHNIMPORTSM2PUBLICKEYIN.dwSM2SignatureAlgorithm, PROTCHNEXPORTSM2ISSUERSIGNEDITEMOUT.dwSM2SignatureAlgorithm, PROTCHNEXPORTSM2EPPSIGNEDITEMIN.dwSignatureAlgorithm and PROTCHNIMPORTSM2SIGNEDSM4KEY.dwSM2SignatureAlgorithm */
#define PROT_CHN_PIN_SIGN_NA					(0)
#define PROT_CHN_SIGN_SM2_GM_T_2012				(0x00000001)

/* values for PROTCHNIMPORTSM2PUBLICKEYOUT.dwSM2KeyCheckMode */
#define PROT_CHN_SM2_KCV_NONE					(0x00000001)
#define PROT_CHN_SM2_KCV_SM3					(0x00000002)

/* values for PROTCHNEXPORTSM2ISSUERSIGNEDITEMIN.wExportItemType, PROTCHNEXPORTSM2EPPSIGNEDITEMIN.wExportItemType */
#define PROT_CHN_PIN_EXPORT_EPP_ID				(0x0001)
#define PROT_CHN_PIN_EXPORT_PUBLIC_KEY			(0x0002)

/* values for PROTCHNGENERATESM2KEYOUT.dwUse */
#define PROT_CHN_USESM2PRIVATE					(0x00000001)
#define PROT_CHN_USESM2PRIVATESIGN				(0x00000002)

/*typedef struct _wfs_hex_data
{
    USHORT              usLength;
    LPBYTE              lpbData;
} WFSXDATA, * LPWFSXDATA;*/

// Used to type-cast specific command to access common fields
typedef struct _prot_chn_in
{
	WORD		wCommand;
} PROTCHNIN, *LPPROTCHNIN;

// Used to type-cast specific response to access common fields
typedef struct _prot_chn_out
{
	WORD		wCommand;
	WORD		wResult;
} PROTCHNOUT, *LPPROTCHNOUT;

typedef struct _prot_chn_digest_in
{
	WORD		wCommand;
	WORD		wHashAlgorithm;
        LPWFSXDATA	lpxDigestInput;
} PROTCHNDIGESTIN, *LPPROTCHNDIGESTIN;

typedef struct _prot_chn_digest_out
{
	WORD		wCommand;
	WORD		wResult;
	LPWFSXDATA	lpxDigestOutput;
} PROTCHNDIGESTOUT, *LPPROTCHNDIGESTOUT;

typedef struct _prot_chn_sm2_algorithm_param_in
{
	WORD wCommand;
	LPWFSXDATA	lpxP;
	LPWFSXDATA	lpxA;
	LPWFSXDATA	lpxB;
	LPWFSXDATA	lpxN;
	LPWFSXDATA	lpxXg;
	LPWFSXDATA	lpxYg;
} PROTCHNSM2ALGORITHMPARAMIN, *LPPROTCHNSM2ALGORITHMPARAMIN;

typedef struct _prot_chn_sm2_algorithm_param_out
{
	WORD		wCommand;
	WORD		wResult;
} PROTCHNSM2ALGORITHMPARAMOUT, *LPPROTCHNSM2ALGORITHMPARAMOUT;

typedef struct _prot_chn_import_sm2_public_key_in
{
	WORD		wCommand;
	LPSTR		lpsKey;
	LPWFSXDATA	lpxValue;
	DWORD		dwUse;
	LPSTR		lpsSigKey;
	DWORD		dwSM2SignatureAlgorithm;
	LPWFSXDATA	lpxSignature;
} PROTCHNIMPORTSM2PUBLICKEYIN, *LPPROTCHNIMPORTSM2PUBLICKEYIN;

typedef struct _prot_chn_import_sm2_public_key_out
{
	WORD		wCommand;
	WORD		wResult;
	DWORD		dwSM2KeyCheckMode;
	LPWFSXDATA	lpxKeyCheckValue;
} PROTCHNIMPORTSM2PUBLICKEYOUT, *LPPROTCHNIMPORTSM2PUBLICKEYOUT;

typedef struct _prot_chn_sign_in
{
	WORD		wCommand;
	LPSTR		lpsKey;
	LPSTR		lpSignerID;
	LPWFSXDATA	lpxPlaintextData;
} PROTCHNSIGNIN, *LPPROTCHNSIGNIN;

typedef struct _prot_chn_sign_out
{
	WORD		wCommand;
	WORD		wResult;
	LPWFSXDATA	lpxSignData;
} PROTCHNSIGNOUT, *LPPROTCHNSIGNOUT;

typedef struct _prot_chn_verify_in
{
	WORD		wCommand;
	LPSTR		lpsKey;
	LPWFSXDATA	lpxPlaintextData;
	LPWFSXDATA	lpxSignData;
} PROTCHNVERIFYIN, *LPPROTCHNVERIFYIN;

typedef struct _prot_chn_verify_out
{
	WORD		wCommand;
	WORD		wResult;
} PROTCHNVERIFYOUT, *LPPROTCHNVERIFYOUT;

typedef struct _prot_chn_export_sm2_issuer_signed_item_in
{
	WORD		wCommand;
	WORD		wExportItemType;
	LPSTR		lpsName;
} PROTCHNEXPORTSM2ISSUERSIGNEDITEMIN, *LPPROTCHNEXPORTSM2ISSUERSIGNEDITEMIN;

typedef struct _prot_chn_export_sm2_issuer_signed_item_out
{
	WORD		wCommand;
	WORD		wResult;
	LPWFSXDATA	lpxValue;
	WORD		dwSM2SignatureAlgorithm;
	LPWFSXDATA	lpxSignature;
} PROTCHNEXPORTSM2ISSUERSIGNEDITEMOUT, *LPPROTCHNEXPORTSM2ISSUERSIGNEDITEMOUT;

typedef struct _prot_chn_generate_sm2_keypair_in
{
	WORD		wCommand;
	LPSTR		lpsKey;
	DWORD		dwUse;
} PROTCHNGENERATESM2KEYPAIRIN, *LPPROTCHNGENERATESM2KEYPAIRIN;

typedef struct _prot_chn_generate_sm2_keypair_out
{
	WORD		wCommand;
	WORD		wResult;
} PROTCHNGENERATESM2KEYPAIROUT, *LPPROTCHNGENERATESM2KEYPAIROUT;

typedef struct _prot_chn_export_sm2_epp_signed_item_in
{
	WORD		wCommand;
	WORD		wExportItemType;
	LPSTR		lpsName;
	LPSTR		lpsSigKey; 
	DWORD		dwSignatureAlgorithm;
} PROTCHNEXPORTSM2EPPSIGNEDITEMIN, *LPPROTCHNEXPORTSM2EPPSIGNEDITEMIN;

typedef struct _prot_chn_export_sm2_epp_signed_item_output
{
	WORD		wCommand;
	WORD		wResult;
        LPWFSXDATA	lpxValue;
        LPWFSXDATA      lpxSelfSignature;
        LPWFSXDATA	lpxSignature;
} PROTCHNEXPORTSM2EPPSIGNEDITEMOUT, *LPPROTCHNEXPORTSM2EPPSIGNEDITEMOUT;


typedef struct _prot_chn_import_sm2_signed_sm4_key
{
	WORD		wCommand;
	LPSTR		lpsKey;
	LPSTR		lpsDecryptKey;
	DWORD		dwSM2EncipherAlgorithm;
	LPWFSXDATA	lpxValue;
	DWORD		dwUse;
	LPSTR		lpsSigKey;
	DWORD		dwSM2SignatureAlgorithm;
	LPWFSXDATA	lpxSignature;
} PROTCHNIMPORTSM2SIGNEDSM4KEY, *LPPROTCHNIMPORTSM2SIGNEDSM4KEY;

typedef struct _prot_chn_import_sm2_signed_sm4_key_output
{
	WORD		wCommand;
	WORD		wResult;
	WORD		wKeyCheckMode;
	LPWFSXDATA	lpxKeyCheckValue;
} PROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT, *LPPROTCHNIMPORTSM2SIGNEDSM4KEYOUTPUT;

/* restore alignment */
#pragma pack(pop)

// #ifdef __cplusplus
// }       /*extern "C"*/
 //#endif

#endif /* __INC_XFSPINCHN__H */


