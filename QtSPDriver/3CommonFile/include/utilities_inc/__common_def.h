#ifndef _COMMON_DEF_H_
#define _COMMON_DEF_H_

#include <unistd.h>
#include <sys/stat.h>

#include "__common_constant_def.h"
#include "__common_version_def.h"
#include "__common_os_compatible_def.h"
#include "__common_operation_def.h"
#include "__common_sys_log_def.h"


// 文件相关
#define SYS_INI_PATH						"ini"
#define SYS_DATA_PATH						"data"
#define LOG_INI_FILE						"log_manager.ini"
#define LOG_CONFIG_FILE						"log_config"


// 全局名称（进程同步、共享内存等）
#define LOG_ACCESS_LOCK                                         "cfes.log.access.lock"

// 日志文件
#define LOG_XFS_ADMIN						"xfs_admin.log"


// Create folders by path
#ifdef WIN32
#define CREATE_FOLDERS_BY_PATH(ModuleName, FullPath, BoolResult) \
{\
	unsigned long _idx_ = 0;\
	char _folder_path_[MAX_PATH] = { 0 };\
	int _ret_ = 0;\
	BoolResult = true;\
	if ((FullPath != NULL) && (strlen(FullPath) > 0)) {\
		for (_idx_ = 0; _idx_ < strlen(FullPath); _idx_++) {\
			if (FullPath[_idx_] == PATH_SPT_CHR) {\
				if (_idx_ > 0) {\
					memset(_folder_path_, 0x00, sizeof(_folder_path_));\
                                        memcpy(_folder_path_, FullPath, _idx_);\
					if ((_ret_ =_access(_folder_path_, 0)) != 0) {\
                                                if ((_ret_ = _mkdir(_folder_path_)) != 0) {\
							REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, ModuleName, "_mkdir", _ret_, true, _folder_path_);\
							BoolResult = false;\
						}\
					}\
				}\
			}\
		}\
	}\
}
#else   // LINUX
#define CREATE_FOLDERS_BY_PATH(ModuleName, FullPath, BoolResult) \
{\
        unsigned long _idx_ = 0;\
        char _folder_path_[MAX_PATH] = { 0 };\
        int _ret_ = 0;\
        BoolResult = true;\
        if ((FullPath != NULL) && (strlen(FullPath) > 0)) {\
                for (_idx_ = 0; _idx_ < strlen(FullPath); _idx_++) {\
                        if (FullPath[_idx_] == PATH_SPT_CHR) {\
                                if (_idx_ > 0) {\
                                        memset(_folder_path_, 0x00, sizeof(_folder_path_));\
                                        memcpy(_folder_path_, FullPath, _idx_);\
                                        if ((_ret_ = access(_folder_path_, 0)) != 0) {\
                                                if ((_ret_ = mkdir(_folder_path_, S_IRWXU | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)) != 0) {\
                                                        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, ModuleName, "mkdir", _ret_, true, _folder_path_);\
                                                        BoolResult = false;\
                                                }\
                                        }\
                                }\
                        }\
                }\
        }\
}
#endif  // WIN32


#endif // _COMMON_DEF_H_
