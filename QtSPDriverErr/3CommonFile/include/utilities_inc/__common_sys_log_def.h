#ifndef _COMMON_SYS_LOG_DEF_H_
#define _COMMON_SYS_LOG_DEF_H_


#ifdef WIN32
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif // _CRT_SECURE_NO_WARNINGS
#include <Windows.h>
#endif // WIN32

#include <stdio.h>

#define MAX_EVT_DATA_SIZE						(260)


#ifdef WIN32
#define REPORT_APP_EVENT_MSG(EvtType, AppName, Message)\
{\
	char event_data[2][MAX_EVT_DATA_SIZE + 100];\
	const char* event_data_ptr[2] = { NULL };\
	int _i_ = 0;\
	unsigned short data_line = 0;\
	HANDLE  event_log_handle = NULL;\
	memset(event_data, 0x00, sizeof(event_data));\
	for (_i_ = 0; _i_ < 2; _i_++) {\
		event_data_ptr[_i_] = event_data[_i_];\
	}\
	sprintf(event_data[0], "%s(line:%d)-<function:%s>", __FILE__, __LINE__, __FUNCTION__);\
	data_line++;\
	strncpy(cEventData[1], Message, MAX_EVT_DATA_SIZE + 100 - 1);\
	data_line++;\
	event_log_handle = RegisterEventSource(NULL, AppName);\
	ReportEvent(event_log_handle, EvtType, 0, (DWORD)-1, NULL, data_line, 0, event_data_ptr, NULL);\
	DeregisterEventSource(event_log_handle);\
}
#else
#define REPORT_APP_EVENT_MSG(EvtType, AppName, Message)\
{\
        openlog(AppName, LOG_CONS | LOG_PID, LOG_USER);\
        syslog(EvtType | LOG_USER, "%s", Message);\
        closelog();\
}
#endif // WIN32

#ifdef WIN32
#define REPORT_APP_EVENT_API(EvtType, AppName, APIFunc, Return, OutputLastError)\
{\
	char event_data[4][MAX_EVT_DATA_SIZE + 100];\
	const char* event_data_ptr[4] = { NULL };\
	int _i_ = 0;\
	unsigned short data_line = 0;\
	HANDLE  event_log_handle = NULL;\
	long ret_val = (long)Return;\
	bool output_last_error = OutputLastError;\
	memset(event_data, 0x00, sizeof(event_data));\
	for (_i_ = 0; _i_ < 4; _i_++) {\
		event_data_ptr[_i_] = event_data[_i_];\
	}\
	sprintf(event_data[0], "%s(line:%d)-<function:%s>", __FILE__, __LINE__, __FUNCTION__);\
	data_line++;\
	strncpy(event_data[1], APIFunc, MAX_EVT_DATA_SIZE + 100 - 1);\
	data_line++;\
	sprintf(event_data[2], "Return = %ld", ret_val);\
	data_line++;\
	if (output_last_error) {\
		sprintf(event_data[3], "GetLastError = %d", GetLastError());\
		data_line++;\
	}\
	event_log_handle = RegisterEventSource(NULL, AppName);\
	ReportEvent(event_log_handle, EvtType, 0, (DWORD)-1, NULL, data_line, 0, event_data_ptr, NULL);\
	DeregisterEventSource(event_log_handle);\
}
#else
#define REPORT_APP_EVENT_API(EvtType, AppName, APIFunc, Return, OutputLastError)\
{\
        char event_data[4][MAX_EVT_DATA_SIZE + 100];\
        long ret_val = (long)Return;\
        bool output_last_error = OutputLastError;\
        memset(event_data, 0x00, sizeof(event_data));\
        sprintf(event_data[0], "%s(line:%d)-<function:%s>", __FILE__, __LINE__, __FUNCTION__);\
        strncpy(event_data[1], APIFunc, MAX_EVT_DATA_SIZE + 100 - 1);\
        sprintf(event_data[2], "Return = %ld", ret_val);\
        if (output_last_error) {\
                sprintf(event_data[3], "errno = %d", errno);\
        }\
        openlog(AppName, LOG_CONS | LOG_PID, LOG_USER);\
        syslog(EvtType | LOG_USER, "%s; %s; %s; %s",\
                event_data[0], event_data[1], event_data[2], event_data[3]);\
        closelog();\
}
#endif // WIN32

#ifdef WIN32
#define REPORT_APP_EVENT_API_MSG(EvtType, AppName, APIFunc, Return, OutputLastError, Msg)\
{\
	char event_data[5][MAX_EVT_DATA_SIZE + 100];\
	const char* event_data_ptr[5] = { NULL };\
	int _i_ = 0;\
	unsigned short data_line = 0;\
	HANDLE  event_log_handle = NULL;\
	long ret_val = (long)Return;\
	bool output_last_error = OutputLastError;\
	const char* _msg_ = (const char*)Msg;\
	memset(event_data, 0x00, sizeof(event_data));\
	for (_i_ = 0; _i_ < 5; _i_++) {\
		event_data_ptr[_i_] = event_data[_i_];\
	}\
	sprintf(event_data[0], "%s(line:%d)-<function:%s>", __FILE__, __LINE__, __FUNCTION__);\
	data_line++;\
	strncpy(event_data[1], APIFunc, MAX_EVT_DATA_SIZE + 100 - 1);\
	data_line++;\
	sprintf(event_data[2], "Return = %ld", ret_val);\
	data_line++;\
	if (output_last_error) {\
		sprintf(event_data[3], "GetLastError = %d", GetLastError());\
		data_line++;\
	}\
	if ((_msg_ != NULL) && (strlen(_msg_) > 0)) {\
		strncpy(event_data[4], _msg_, MAX_EVT_DATA_SIZE + 100 - 1);\
		data_line++;\
	}\
	event_log_handle = RegisterEventSource(NULL, AppName);\
	ReportEvent(event_log_handle, EvtType, 0, (DWORD)-1, NULL, data_line, 0, event_data_ptr, NULL);\
	DeregisterEventSource(event_log_handle);\
}
#else
#define REPORT_APP_EVENT_API_MSG(EvtType, AppName, APIFunc, Return, OutputLastError, Msg)\
{\
        char event_data[5][MAX_EVT_DATA_SIZE + 100];\
        long ret_val = (long)Return;\
        bool output_last_error = OutputLastError;\
        const char* _msg_ = (const char*)Msg;\
        memset(event_data, 0x00, sizeof(event_data));\
        sprintf(event_data[0], "%s(line:%d)-<function:%s>", __FILE__, __LINE__, __FUNCTION__);\
        strncpy(event_data[1], APIFunc, MAX_EVT_DATA_SIZE + 100 - 1);\
        sprintf(event_data[2], "Return = %ld", ret_val);\
        if (output_last_error) {\
                sprintf(event_data[3], "Error = %d; ", errno);\
        }\
        if ((_msg_ != NULL) && (strlen(_msg_) > 0)) {\
                strncpy(event_data[4], _msg_, MAX_EVT_DATA_SIZE + 100 - 1);\
        }\
        openlog(AppName, LOG_CONS | LOG_PID, LOG_USER);\
        syslog(EvtType | LOG_USER, "%s; %s; %s; %s; %s",\
                event_data[0], event_data[1], event_data[2], event_data[3], event_data[4]);\
        closelog();\
}
#endif // WIN32

#ifdef WIN32
#define REPORT_APP_EVENT_API_DATA(EvtType, AppName, APIFunc, Return, OutputLastError, BinaryData, DataSize)\
{\
	char event_data[4][MAX_EVT_DATA_SIZE + 100];\
	const char* event_data_ptr[4] = { NULL };\
	int _i_ = 0;\
	unsigned short data_line = 0;\
	HANDLE  event_log_handle = NULL;\
	long ret_val = (long)Return;\
	bool output_last_error = OutputLastError;\
	memset(event_data, 0x00, sizeof(event_data));\
	for (_i_ = 0; _i_ < 4; _i_++) {\
		event_data_ptr[_i_] = event_data[_i_];\
	}\
	sprintf(event_data[0], "%s(line:%d)-<function:%s>", __FILE__, __LINE__, __FUNCTION__);\
	data_line++;\
	strncpy(event_data[1], APIFunc, MAX_EVT_DATA_SIZE + 100 - 1);\
	data_line++;\
	sprintf(event_data[2], "Return = %ld", ret_val);\
	data_line++;\
	if (output_last_error)\
	{\
		sprintf(event_data[3], "GetLastError = %d", GetLastError());\
		data_line++;\
	}\
	hEventLog = RegisterEventSource(NULL, AppName);\
	ReportEvent(hEventLog, EvtType, 0, (DWORD)-1, NULL, data_line, DataSize, event_data_ptr, BinaryData);\
	DeregisterEventSource(hEventLog);\
}
#else
#define REPORT_APP_EVENT_API_DATA(EvtType, AppName, APIFunc, Return, OutputLastError, BinaryData, DataSize)\
{\
        char event_data[4][MAX_EVT_DATA_SIZE + 100];\
        const char* event_data_ptr[4] = { NULL };\
        long ret_val = (long)Return;\
        bool output_last_error = OutputLastError;\
        memset(event_data, 0x00, sizeof(event_data));\
        sprintf(event_data[0], "%s(line:%d)-<function:%s>", __FILE__, __LINE__, __FUNCTION__);\
        strncpy(event_data[1], APIFunc, MAX_EVT_DATA_SIZE + 100 - 1);\
        sprintf(event_data[2], "Return = %ld", ret_val);\
        if (output_last_error)\
        {\
                sprintf(event_data[3], "GetLastError = %d", GetLastError());\
        }\
        openlog(AppName, LOG_CONS | LOG_PID, LOG_USER);\
        syslog(EvtType | LOG_USER, "%s; %s; %s; %s",\
                event_data[0], event_data[1], event_data[2], event_data[3]);\
        closelog();\
}
#endif // WIN32


#endif // _COMMON_SYS_LOG_DEF_H_
