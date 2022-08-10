#ifndef _COMMON_OPERATION_DEF_H_
#define _COMMON_OPERATION_DEF_H_


#include <string.h>

// BOOL <-> bool
#define	TO_bool(BVar)									((BVar) ? true : false)
#define	TO_BOOL(BVar)									((BVar) ? TRUE : FALSE)
// Reverse bytes
#define	EXWORDHL(X)										(unsigned short)((((unsigned short)(X & 0xFF))<<8) | ((X >> 8) & 0xFF))
#define	EXDWORDHL(X)									(unsigned int)((((unsigned int)EXWORDHL((X & 0xFFFF)))<<16) | (EXWORDHL((X >> 16) & 0xFFFF)))
#define	EXLONGLONGHL(X)									(unsigned long long)((((unsigned long long)EXDWORDHL((X & 0xFFFFFFFF)))<<32) | (EXDWORDHL((X >> 32) & 0xFFFFFFFF)))
// Array length
#define	ARRAY_LENGTH(Array)								(sizeof(Array) / sizeof(Array[0]))
// Clear Array
#define CLEAR_ARRAY(Array)								memset(Array, 0x00, sizeof(Array))
// Free buffer to NULL
#define FREE_BUFFER(Pointer)							{if (Pointer) {delete[] Pointer; Pointer = NULL;}}
// Free Pointer to NULL
#define FREE_PTR(Pointer)							{if (Pointer) {delete Pointer; Pointer = NULL;}}
// Same byte buffer
#define IS_BUFFER_SAME_BYTE(Pointer, By, BufSize, BoolRes) \
{\
	BoolRes = false;\
	if ((Pointer != NULL) && (BufSize > 0)) {\
		unsigned long _i_ = 0;\
		BoolRes = true;\
		unsigned char* _tmp_ptr_ = Pointer;\
		for (_i_ = 0; _i_ < (unsigned long)BufSize; _i_++) {\
			if (_tmp_ptr_[_i_] != (unsigned char)By) {\
				BoolRes = false;\
				break;\
			}\
		}\
	}\
}
// Empty string
#define IS_STRING_EMPTY(Str)							(((Str) == NULL) || (strlen(Str) == 0))
#define IS_STRING_NOT_EMPTY(Str)						(!IS_STRING_EMPTY(Str))
// Same char string
#define IS_STRING_SAME_CHAR(Str, Ch, BoolRes)			IS_BUFFER_SAME_BYTE(Str, Ch, strlen(Str), BoolRes)
#define IS_SPACE_STRING(Str, BoolRes)					IS_STRING_SAME_CHAR(Str, ' ', BoolRes)
// Swap two data 
#define SWAP_DATA(DataType, D1, D2) \
{\
	DataType tmpData;\
	memcpy(&tmpData, &D1, sizeof(DataType));\
	memcpy(&D1, &D2, sizeof(DataType));\
	memcpy(&D2, &tmpData, sizeof(DataType));\
}
// Table traversal
#define TEST_TABLE_VALUE(Tab, TestCondition, TabEndCondition, DoInit, TestOKtoDo, TestNGtoDo) \
{\
	DoInit;\
	if (Tab != NULL) {\
		unsigned long _Idx_ = 0;\
		for (_Idx_ = 0; _Idx_ < ARRAY_LENGTH(Tab); _Idx_++) {\
			if (TabEndCondition) {\
				break;\
			}\
			if (TestCondition) {\
				TestOKtoDo;\
			} else {\
				TestNGtoDo;\
			}\
		}\
	}\
}
// Set value to pointer variable
#define SET_VALUE_TO_PTR(Ptr, Var, VarType)				if (Ptr) *(Ptr) = (VarType)Var

#ifdef WIN32
// Repeat Startup Avoidance
#define AVOID_PROCESS_REPEAT_STARTUP(ProcessName, RetVal) \
{\
	char _process_event_name_[MAX_PATH] = {0};\
	sprintf_s(_process_event_name_, sizeof(_process_event_name_), "%s%s%s", "Global\\", ProcessName, "_STARTUP_EVENT");\
	if (OpenEvent(EVENT_ALL_ACCESS, TRUE, _process_event_name_))\
	{\
		return RetVal;\
	}\
	CreateEvent(NULL, FALSE, TRUE, _process_event_name_);\
}

// Simply Startup Process
#define SIMPLE_START_PROCESS(StartProcess) \
{\
	STARTUPINFO si = {sizeof(si)};\
	PROCESS_INFORMATION pi;\
	memset(&pi, 0x00, sizeof(pi));\
	CreateProcess(NULL, (LPSTR)(StartProcess), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);\
}
#endif // WIN32

// Add loop counter
#define ADD_LOOP_COUNTER(Counter, MaxCounter) \
	(Counter) = (((Counter) < MaxCounter) ? ((Counter) + 1) : 0)

#define strncpy(DestBuf, SourceStr, CopyCount) \
{\
    size_t _count = ((strlen(SourceStr) < CopyCount) ? strlen(SourceStr) : CopyCount);\
    memcpy(DestBuf, SourceStr, _count);\
    DestBuf[_count] = 0x00;\
}


#endif // _COMMON_OPERATION_DEF_H_
