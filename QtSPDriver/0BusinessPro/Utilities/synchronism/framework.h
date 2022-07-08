// header.h: 标准系统包含文件的包含文件，
// 或特定于项目的包含文件
//
#ifndef FRAMEWORK_H
#define FRAMEWORK_H


#include <errno.h>
#include <pthread.h>
#include <deque>
#include <time.h>
#include <sys/time.h>
#include <algorithm>
#include "__common_operation_def.h"


#define WAIT_SINGLE_OBJ_RES(WaitResult) \
    (((WaitResult) == WAIT_OBJECT_0) ? WAIT_OBJ_SUCCESS : \
        (((WaitResult) == WAIT_TIMEOUT) ? WAIT_OBJ_TIMEOUT : \
            (((WaitResult) == WAIT_ABANDONED) ? WAIT_OBJ_ABNORMAL : WAIT_OBJ_FAIL)))


#endif // FRAMEWORK_H


/***********************************************************************
动态库：synchronism 变更履历
0.0.0.0			2019/10/24	Diaoyue		0.0.0.0-000			初版
************************************************************************/
