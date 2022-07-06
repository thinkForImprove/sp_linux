/*
 * This file is part of the EasyLogger Library.
 *
 * Copyright (c) 2015, Armink, <armink.ztl@gmail.com>
 *
 * Permission is hereby granted, free of charge, to any person obtaining
 * a copy of this software and associated documentation files (the
 * 'Software'), to deal in the Software without restriction, including
 * without limitation the rights to use, copy, modify, merge, publish,
 * distribute, sublicense, and/or sell copies of the Software, and to
 * permit persons to whom the Software is furnished to do so, subject to
 * the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED 'AS IS', WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 * IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 * CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 * TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 * SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * Function: Portable interface for each platform.
 * Created on: 2015-04-28
 */
 
#include <elog.h>

/**
 * EasyLogger port initialize
 *
 * @return result
 */
ElogErrCode elog_port_init() {
    ElogErrCode result = ELOG_NO_ERR;

    /* add your code here */
    
    return result;
}

/**
 * output log port interface
 *
 * @param log output of log
 * @param size log size
 */
void elog_port_output(const char *log, size_t size, FILE* fp) {
    
    /* add your code here */
	if (fp != NULL) {
        printf("%.*s", (int)size, log);
        setbuf(fp, NULL);
        fprintf(fp, "%.*s", (int)size, log);
	}
}

/**
 * output lock
 */
void elog_port_output_lock(HANDLE mutex_handle) {
    
    /* add your code here */
	if (mutex_handle) {
#ifdef  WIN32
            WaitForSingleObject(mutex_handle, INFINITE);
#else
            pthread_mutex_lock((pthread_mutex_t*)mutex_handle);
#endif
	}
}

/**
 * output unlock
 */
void elog_port_output_unlock(HANDLE mutex_handle) {
    
    /* add your code here */
	if (mutex_handle) {
#ifdef  WIN32
            ReleaseMutex(mutex_handle);
#else
            pthread_mutex_unlock((pthread_mutex_t*)mutex_handle);
#endif
	}
}

/**
 * get current time interface
 *
 * @return current time
 */
const char *elog_port_get_time(void) {
    
    /* add your code here */
	static char cur_system_time[128] = { 0 };
	static SYSTEMTIME sys_time;

#ifdef  WIN32
	GetLocalTime(&sys_time);
#else   // LINUX
    struct timeval tv;
    struct tm local_time;
    memset(&tv, 0, sizeof(tv));
    gettimeofday(&tv, NULL);
    local_time = *(localtime(&(tv.tv_sec)));
    sys_time.wYear = (WORD)(local_time.tm_year + 1900);
    sys_time.wMonth = (WORD)(local_time.tm_mon + 1);
    sys_time.wDay = (WORD)local_time.tm_mday;
    sys_time.wHour = (WORD)local_time.tm_hour;
    sys_time.wMinute = (WORD)local_time.tm_min;
    sys_time.wSecond = (WORD)local_time.tm_sec;
    sys_time.wMilliseconds = (WORD)(tv.tv_usec / 1000);
#endif
    snprintf(cur_system_time, sizeof(cur_system_time), "%04d-%02d-%02d %02d:%02d:%02d.%03d",
            sys_time.wYear, sys_time.wMonth, sys_time.wDay, sys_time.wHour, sys_time.wMinute, sys_time.wSecond, sys_time.wMilliseconds);

    return cur_system_time;
}

/**
 * get current process name interface
 *
 * @return current process name
 */
const char *elog_port_get_p_info(void) {
    
    /* add your code here */
	static char cur_process_info[16] = { 0 };
#ifdef WIN32
        snprintf(cur_process_info, sizeof(cur_process_info), "pid:%04ld", GetCurrentProcessId());
#else
        snprintf(cur_process_info, sizeof(cur_process_info), "pid:%04d", getpid());
#endif // WIN32

	return cur_process_info;
}

/**
 * get current thread name interface
 *
 * @return current thread name
 */
const char *elog_port_get_t_info(void) {
    
    /* add your code here */
	static char cur_thread_info[16] = { 0 };
#ifdef WIN32
	snprintf(cur_thread_info, sizeof(cur_thread_info), "tid:%04ld", GetCurrentThreadId());
#else
        snprintf(cur_thread_info, sizeof(cur_thread_info), "tid:%04lu", pthread_self());
#endif // WIN32

	return cur_thread_info;
}
