//--------------------------------------------------------------------
// [社外秘]
// COPYRIGHT (c) HITACHI-OMRON TERMINAL SOLUTIONS,CORP.
// 2015,2016.All rights reserved.
//--------------------------------------------------------------------
//--------------------------------------------------------------------
// filename	= FNUSB.h
// 日付		= 2015/10/16(CREATE)
// HISTORY	= FNUSB00, 00-01-00-00, 2015/10/16		Create
//          = LFNUSB,  00-00-01-00, 2015/04/05		For Linux
//          = LFNUSB,  00-00-02-00, 2015/06/07		Add _F_GetInitResult
//													ERR_FNC_LIB_INIT
//													ERR_FNC_LIB_INIT2
//													ERR_FNC_LIB_NOT_INIT
//													ERR_FNC_LIB_IMG_RENAME
//			= LFNUSB,  00-00-03-00, 2016/09/22		FUNC_SET_VERIFICATION_LEVEL_FOR_XX, FUNC_SET_BLACK_LIST
//													ERR_FNC_LIB_FILE,ERR_FNC_LIB_FNATM_PRM, ERR_FNC_LIB_FNATM_PRM1,
//													ERR_FNC_LIB_INFATM_PRM, ERR_FNC_LIB_NONSUPPORT add
//--------------------------------------------------------------------
#ifndef __FNUSB_H__
#define __FNUSB_H__

//--------------------------------------------------------------------
// Structure
//--------------------------------------------------------------------
typedef struct _STR_SETPRM {						// Structure
	unsigned short	usParam ;						// Parameter
	void *				pvDataInBuffPtr ;				// Buffer pointer of input data
	unsigned int		uiDataInBuffSz ;				// Buffer size of input data
} __attribute__((packed))STR_SETPRM, *PSTR_SETPRM ;						//

typedef struct _PSTR_GETPRM {						// Structure
	unsigned short	usParam ;						// Parameter
	void *				pvDataOutBuffPtr ;			// Buffer pointer of output data
	unsigned int		uiDataOutReqSz ;				// Request size of output data
	unsigned int		uiDataOutBuffSz ;				// output data size
} __attribute__((packed))STR_GETPRM, *PSTR_GETPRM ;						//

typedef struct _PSTR_STATUS {						// Structure
	unsigned int		uiStatus ;						// Status
	unsigned int		uiResult1 ;					// Result 1
	unsigned int		uiResult2 ;					// Result 2
	unsigned int		uiResult3 ;					// Result 3
	unsigned int		uiResult4 ;					// Result 4
	unsigned int		uiResult5 ;					// Result 5
} __attribute__((packed))STR_STATUS, *PSTR_STATUS ;						//

//--------------------------------------------------------------------
// Define
//--------------------------------------------------------------------
#define		FUNC_GET_DIRECTCASHIN_PARALLEL		1	// Parallel serial number acquisition during cash in
#define		FUNC_GET_DISPENSE_PARALLEL			2	// Parallel serial number acquisition during dispense
#define		FUNC_SET_VERIFICATION_LEVEL_FOR_CASH_COUNT		3		// 
#define		FUNC_SET_VERIFICATION_LEVEL_FOR_STOREMONEY		4		// 
#define		FUNC_SET_VERIFICATION_LEVEL_FOR_DISPENSE		5		// 
#define		FUNC_SET_BLACK_LIST								6		// 

#define		IMAGETYPE_IN_PARALLEL_ACQUISITION	1	// 

//------------------------------------------------------------------------------
// Error Code
//------------------------------------------------------------------------------
#define		ERR_FNC_LIB_MEMORY						0xF1F00000	// Program error      Failure in reserving memory (RESERVE)
#define		ERR_FNC_LIB_MEMORY2						0xF1F00001	// Program error      Failure in reserving memory (RESERVE)
#define		ERR_FNC_LIB_BUFFER						0xF1F00002	// Program error      Buffer overflow
#define		ERR_FNC_LIB_IMG_CREATE					0xF1F10000	// Program error      Save File error (create file)
#define		ERR_FNC_LIB_IMG_WRITE					0xF1F10001	// Program error      Save File error (write file)
#define		ERR_FNC_LIB_IMG_UNDEFINED				0xF1F10002	// Program error      Save File error(undefined)
#define		ERR_FNC_LIB_IMG_RENAME					0xF1F10003	// Program error      Rename File error
#define		ERR_FNC_LIB_SETFN_PROCEDURE				0xF1F20000	// Program error      Procedure error in SetFnSetting
#define		ERR_FNC_LIB_SETFN_MEMORY					0xF1F20001	// Program error      Failure in reserving memory in SetFnSetting
#define		ERR_FNC_LIB_SEQUENCE						0xF1FA0000	// Program error      Sequence error(for parallel function)
#define		ERR_FNC_LIB_BV_NOTOPEN					0xF1FA0001	// Program error      BV is not open

#define		ERR_FNC_LIB_STATUS						0xF1400000	// Abnormal response  Status error
#define		ERR_FNC_LIB_STATUS_UNDEFINED			0xF1400001	// Abnormal response  undefined status error
#define		ERR_FNC_LIB_TRCFILE_NONE					0xF1410000	// Abnormal response  Trace none
#define		ERR_FNC_LIB_TRCFILE_NONE2				0xF1410001	// Abnormal response  Trace none
#define		ERR_FNC_LIB_TRCFILE_ABNORMAL			0xF1410002	// Abnormal response  Data error
#define		ERR_FNC_LIB_TRCFILE_DATA					0xF1410003	// Abnormal response  Data error
#define		ERR_FNC_LIB_TRCFILE_SIZE					0xF1410004	// Abnormal response  Size error
#define		ERR_FNC_LIB_TRCFILE_UNDEFINED			0xF1410005	// Abnormal response  undefined response

#define		ERR_FNC_LIB_IMG_PRM						0xF1010000	// Parameter error    Save File error (parameter)
#define		ERR_FNC_LIB_SETFN_PRM					0xF1020000	// Parameter error    Parameter error in SetFnSetting
#define		ERR_FNC_LIB_SETFN_PRM1					0xF1020001	// Parameter error    folder path is not exist(SetFnSetting)
#define		ERR_FNC_LIB_SETFN_PRM2					0xF1020002	// Parameter error    path is not folder(SetFnSetting)
#define		ERR_FNC_LIB_SETFN_PRM3					0xF1020003	// Parameter error    path is read only (SetFnSetting)
#define		ERR_FNC_LIB_FILE						0xF1020004	// Parameter error    Appointed file does not exist
#define		ERR_FNC_LIB_GETFN_PRM					0xF1030000	// Parameter error    Parameter error in GetFnSetting
#define		ERR_FNC_LIB_GETSTS_PRM					0xF1040000	// Parameter error    Parameter error in GetStatus
#define		ERR_FNC_LIB_FNATMUSB_PRM					0xF1050000	// Parameter error    Parameter error in FnATMUSB (RESERVE)
#define		ERR_FNC_LIB_SETCMN_PRM					0xF1060000	// Parameter error    Parameter error in SetCommonSetting
#define		ERR_FNC_LIB_GETCMN_PRM					0xF1070000	// Parameter error    Parameter error in GetCommonSetting
#define		ERR_FNC_LIB_LACK_OF_PRM					0xF10A0000	// Parameter error    Lack of prm for parallel function
#define		ERR_FNC_LIB_FNATM_PRM					0xF10A0001	// Parameter error    Parameter error in FnATMUSB (Pointer is NULL)
#define		ERR_FNC_LIB_FNATM_PRM1					0xF10A0002	// Parameter error    Parameter error in FnATMUSB (usParam is invalid)
#define		ERR_FNC_LIB_INFATM_PRM					0xF10B0000	// Parameter error    Parameter error in InfATMUSB (Pointer is NULL)

#define		ERR_FNC_LIB_INDATA_BUFFSZ				0xF1600000	// Buufer size error  Buffer size error for input data
#define		ERR_FNC_LIB_OUTDATA_BUFFSZ				0xF1610000	// Buffer size error  Buffer size error for output data
#define		ERR_FNC_LIB_INIT						0xF1F50000	// Program error	Load error in initialize(dlopen)
#define		ERR_FNC_LIB_INIT2						0xF1F50001	// Program error	Create error in initialize(dlopen)
#define		ERR_FNC_LIB_NOT_INIT					0xF1F50002	// Program error	Procedure Error (not open library)

#define		ERR_FNC_LIB_NONSUPPORT					0xF1700000	// Non Supported      Not supported function

#ifdef __cpuluspulus
extern "C"
{
#endif
	unsigned int _F_GetInitResult(void);								// Function library Initialize
	unsigned int _F_FnATMUSB( unsigned int, PSTR_DRV );				// Function Series API
	unsigned int _F_InfATMUSB( unsigned int, PSTR_DRV );				// Informatin Series API
	unsigned int _F_SetFnSetting( unsigned int, PSTR_SETPRM );		// Function Set API
	unsigned int _F_SetCommonSetting( unsigned int, PSTR_SETPRM );	// Parameter Set API
	unsigned int _F_GetFnSetting( unsigned int, PSTR_GETPRM );		// Function Settings Acquisitoin API
	unsigned int _F_GetCommonSetting( unsigned int, PSTR_GETPRM );	// Parameter Settings Acquisition API
	unsigned int _F_GetStatus( unsigned int, PSTR_STATUS );			// Status Acquisition API
#ifdef __cpuluspulus
}
#endif

#endif
