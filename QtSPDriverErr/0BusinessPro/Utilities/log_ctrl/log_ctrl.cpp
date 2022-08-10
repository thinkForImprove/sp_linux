// log_ctrl.cpp : 定义静态库的函数。
//

#include "framework.h"
#include "elog.h"
#include "elog_cfg.h"
#include "log_ctrl.h"


#define BACKUP_TIME_STR								("YYYYMMDD_hhmmss_nnn")
#define BACKUP_TIME_STR_LEN							(strlen(BACKUP_TIME_STR))


char LogCtrl::m_log_ini_path[] = { 0 };
char LogCtrl::m_log_config_path[] = { 0 };
char LogCtrl::m_log_main_path[] = { 0 };
char LogCtrl::m_log_cur_path[] = { 0 };
char LogCtrl::m_cur_path_name[] = { 0 };
char LogCtrl::m_backup_path[] = { 0 };
char LogCtrl::m_last_backup_time_str[] = { 0 };
int LogCtrl::m_file_max_size = -1;
int LogCtrl::m_backup_save_days = -1;
int LogCtrl::m_backup_save_count = -1;
int LogCtrl::m_total_max_size = -1;
#ifdef WIN32
SYSTEMTIME LogCtrl::m_backup_time_every_day = { 0, 0, 0, 0, 0, 0, 0, 0 };
#else
struct tm LogCtrl::m_backup_time_every_day = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
#endif // WIN32
bool LogCtrl::m_encrypt_compress = false;


LogCtrl::LogCtrl()
{
    memset(m_log_file, 0x00, sizeof(m_log_file));
    memset(m_log_file_path, 0x00, sizeof(m_log_file_path));
    memset(m_log_tag, 0x00, sizeof(m_log_tag));
    m_fp = NULL;
#ifdef WIN32
    m_mutex_handle = NULL;
#endif
}

LogCtrl::~LogCtrl()
{
    memset(m_log_file, 0x00, sizeof(m_log_file));
    memset(m_log_file_path, 0x00, sizeof(m_log_file_path));
    memset(m_log_tag, 0x00, sizeof(m_log_tag));
    close_logger();
}


/**
 @功能：	打开已注册文件
 @参数：	无
 @返回：	无
 */
bool LogCtrl::open_file()
{
    if ((m_fp == NULL) && (strlen(m_log_file_path) > 0)) {
#ifdef WIN32
        m_fp = _fsopen(m_log_file_path, "a+", _SH_DENYNO);
        if (m_fp == NULL) {
            REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "_fsopen", m_fp, true, m_log_file_path);
            return false;
        }
#else  // LINUX
        m_fp = fopen(m_log_file_path, "a+");
        if (m_fp == NULL) {
            REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", m_fp, true, m_log_file_path);
            return false;
        }
#endif
    }

    return (m_fp != NULL);
}

/**
 @功能：	关闭文件
 @参数：	无
 @返回：	无
 */
void LogCtrl::close_file()
{
    if (m_fp) {
        fclose(m_fp);
        m_fp = NULL;
    }
}

/**
 @功能：	关闭Mutex
 @参数：	无
 @返回：	无
 */
void LogCtrl::close_mutex()
{
#ifdef WIN32
    if (m_mutex_handle) {
        ReleaseMutex(m_mutex_handle);
        CloseHandle(m_mutex_handle);
        m_mutex_handle = NULL;
    }
#else // LINUX
    pthread_mutex_destroy(&m_mutex_handle);
#endif
}

/**
 @功能：	设置日志文件名称并启动
 @参数：	log_file_path：日志文件全路径	log_file_name：日志文件名称
 @返回：	true：成功		false：失败
 */
bool LogCtrl::initialize_logger(const char* log_file_path, const char* log_file_name)
{
    bool ret = false;
    int nRet = 0;

    if ((log_file_path == NULL) || (strlen(log_file_path) == 0) ||
        (log_file_name == NULL) || (strlen(log_file_name) == 0)) {
        return false;
    }

#ifdef WIN32
    if ((strcmp(log_file_path, m_log_file_path) == 0) &&
        (m_mutex_handle != NULL)) {
        return true;
    }
#else
    if (strcmp(log_file_path, m_log_file_path) == 0) {
        return true;
    }
#endif

    close_logger();

    CREATE_FOLDERS_BY_PATH(get_product_name(), log_file_path, ret);
    if (ret == false) {
        return false;
    }

#ifdef WIN32
    m_fp = _fsopen(log_file_path, "a+", _SH_DENYNO);
    if (m_fp == NULL) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "_fsopen", m_fp, true, log_file_path);
        return ELOG_ERR;
    }
#else   // LINUX
    m_fp = fopen(log_file_path, "a+");
    if (m_fp == NULL) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", m_fp, true, log_file_path);
        return ELOG_ERR;
    }
#endif
    close_file();
    strcpy(m_log_file_path, log_file_path);

#ifdef  WIN32
    m_mutex_handle = CreateMutex(NULL, FALSE, log_file_name);
    if (m_mutex_handle == NULL) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "CreateMutex", m_mutex_handle, true, log_file_name);
#else   // LINUX
    if ((nRet = pthread_mutex_init(&m_mutex_handle, NULL)) != 0) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "pthread_mutex_init", nRet, true, log_file_name);
#endif
        if (m_fp) {
            fclose(m_fp);
            memset(m_log_file_path, 0x00, sizeof(m_log_file_path));
            m_fp = NULL;
        }
        return false;
    }


    if (elog_init(&m_mutex_handle) != ELOG_NO_ERR) {
        return false;
    }

    elog_set_fmt(ELOG_LVL_ASSERT, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_ERROR, ELOG_FMT_ALL & (~(ELOG_FMT_DIR)));
    elog_set_fmt(ELOG_LVL_WARN, ELOG_FMT_ALL & (~(ELOG_FMT_DIR)));
    elog_set_fmt(ELOG_LVL_INFO, ELOG_FMT_ALL & (~(ELOG_FMT_DIR)));
    elog_set_fmt(ELOG_LVL_DEBUG, ELOG_FMT_ALL);
    elog_set_fmt(ELOG_LVL_VERBOSE, ELOG_FMT_ALL);

#ifdef _DEBUG
    elog_set_filter_lvl(ELOG_LVL_VERBOSE);
#else
    elog_set_filter_lvl(ELOG_LVL_INFO);
#endif // _DEBUG

    backup_file_by_size(log_file_path);

    elog_start();

    return true;
}

/**
 @功能：	注册日志文件路径
 @参数：	file_name：日志文件名（不包含路径）		log_tag：调用者名称
 @返回：	true：成功		false：失败
 */
bool LogCtrl::log_register(const char* file_name, const char* log_tag)
{
    bool ret = false;
    char log_file_path[MAX_PATH] = { 0 };

    if ((file_name == NULL) || (strlen(file_name) == 0) ||
        (log_tag == NULL) || (strlen(log_tag) == 0)) {
        return false;
    }

    if (load_log_path_and_setting() == false) {
        return false;
    }

    ret = FileDir::combine_path(m_log_cur_path, file_name, log_file_path, sizeof(log_file_path));
    if (ret == false) {
        return false;
    }

#ifdef  WIN32
    if ((strlen(log_file_path) < 4) ||
        (_stricmp(&log_file_path[strlen(log_file_path) - 4], ".log") != 0)) {
        strcat(log_file_path, ".log");
    }
#else   // LINUX
    if ((strlen(log_file_path) < 4) ||
        (strcasecmp(&log_file_path[strlen(log_file_path) - 4], ".log") != 0)) {
        strcat(log_file_path, ".log");
    }
#endif

    ret = initialize_logger(log_file_path, file_name);
    if (ret == false) {
        return false;
    }

    if (file_name) {
        strncpy(m_log_file, file_name, sizeof(m_log_file) - 1);
    }
    if (log_tag) {
        strncpy(m_log_tag, log_tag, sizeof(m_log_tag) - 1);
    }

    return true;
}

/**
 @功能：	关闭日志
 @参数：	无
 @返回：	无
 */
void LogCtrl::close_logger()
{
    close_file();
    close_mutex();
}

/**
 @功能：	格式化日志信息出力
 @参数：	level：日志级别				file：调用文件名
 @			func：调用函数名			line：调用行号
 @			format, ...：日志格式与内容
 @返回：	无
 */
void LogCtrl::log_output(unsigned char level,
    const char* file, const char* func, const long line, const char* format, ...)
{
    SetupMutex(LOG_ACCESS_LOCK);
    m_sync_lock.set_lock();

    static char fmt_log[ELOG_LINE_BUF_SIZE] = { 0 };
    va_list args;// = NULL;

    log_register(m_log_file, m_log_tag);

    va_start(args, format);
    vsnprintf(fmt_log, sizeof(fmt_log), format, args);
    va_end(args);

    if (open_file()) {
        elog_output(m_fp, &m_mutex_handle, level, m_log_tag, file, func, line, fmt_log);
    }
    close_file();

    m_sync_lock.release_lock();
}

/**
 @功能：	十六进制格式日志信息出力
 @参数：	data_type：日志种类				msg：提示信息
 @			data_buf：数据缓冲区			data_len：数据长度
 @返回：	无
 */
void LogCtrl::hex_output(int data_type, const char* msg, unsigned char* data_buf, unsigned short data_len)
{
    SetupMutex(LOG_ACCESS_LOCK);
    m_sync_lock.set_lock();

    log_register(m_log_file, m_log_tag);

    if (open_file()) {
        elog_info(m_fp, &m_mutex_handle, m_log_tag, msg);
        if (data_type == HEX_LOG_NORMAL) {
            elog_hex(m_fp, &m_mutex_handle, 16, data_buf, data_len);
        } else {
            elog_hexdump(m_fp, &m_mutex_handle, "", 16, data_buf, data_len);
        }
    }
    close_file();

    m_sync_lock.release_lock();
}

/**
 @功能：	平文日志信息出力
 @参数：	format, ...：日志格式与内容
 @返回：	无
 */
void LogCtrl::log_raw(const char* format, ...)
{
    SetupMutex(LOG_ACCESS_LOCK);
    m_sync_lock.set_lock();

    static char fmt_log[ELOG_LINE_BUF_SIZE] = { 0 };
    va_list args;// = NULL;

    log_register(m_log_file, m_log_tag);

    va_start(args, format);
    vsnprintf(fmt_log, sizeof(fmt_log), format, args);
    va_end(args);

    if (open_file()) {
        elog_raw(m_fp, &m_mutex_handle, fmt_log);
    }
    close_file();

    m_sync_lock.release_lock();
}

/**
 @功能：	平文日志信息出力
 @参数：	msg：提示信息				format, ...：日志格式与内容
 @返回：	无
 */
void LogCtrl::log_msg_raw(const char* msg, const char* format, ...)
{
    SetupMutex(LOG_ACCESS_LOCK);
    m_sync_lock.set_lock();

    static char fmt_log[ELOG_LINE_BUF_SIZE] = { 0 };
    va_list args;// = NULL;

    log_register(m_log_file, m_log_tag);

    va_start(args, format);
    vsnprintf(fmt_log, sizeof(fmt_log), format, args);
    va_end(args);

    if (open_file()) {
        elog_info(m_fp, &m_mutex_handle, m_log_tag, msg);
        elog_raw(m_fp, &m_mutex_handle, fmt_log);
    }
    close_file();

    m_sync_lock.release_lock();
}


/**
 @功能：	读取日志路径及相关配置
 @参数：	无
 @返回：	true：成功		false：失败
 */
bool LogCtrl::load_log_path_and_setting()
{
    bool ret = false;
    char log_ini[MAX_PATH] = { 0 };
    char log_config[MAX_PATH] = { 0 };
    char cur_path_name[CONST_VALUE_16] = { 0 };
    char cur_path[MAX_PATH] = { 0 };
    char main_path[MAX_PATH] = { 0 };
    char tmp_path[MAX_PATH] = { 0 };
    char backup_time[CONST_VALUE_16] = { 0 };
    unsigned long bkhhmm[2] = { 0, 0 };
#ifdef WIN32
    SYSTEMTIME sys_time_now;
#else
    struct tm sys_time_now;
    time_t t;
#endif

#ifdef  WIN32
    // log ini path: c:\\cfes\\ini\\log_manager.ini
    if (strlen(m_log_ini_path) == 0) {
        ret = FileDir::get_file_path_name(
            NULL, SYS_INI_PATH, LOG_INI_FILE, log_ini, sizeof(log_ini));
        if (ret == false) {
            return false;
        }
        strncpy(m_log_ini_path, log_ini, sizeof(m_log_ini_path) - 1);
    }

    // log config path: c:\\cfes\\data\\log_config
    if (strlen(m_log_config_path) == 0) {
        ret = FileDir::get_file_path_name(
            NULL, SYS_DATA_PATH, LOG_CONFIG_FILE, log_config, sizeof(log_config));
        if (ret == false) {
            return false;
        }
        strncpy(m_log_config_path, log_config, sizeof(m_log_config_path) - 1);
    }

    // log main path: c:\\cfes\\log_data
    if (strlen(m_log_main_path) == 0) {
        // main path folder: "log_data"
        GetPrivateProfileString("LOG", "MainPath", "log_data", tmp_path, sizeof(tmp_path), m_log_ini_path);
        if (strlen(tmp_path) == 0) {
            return false;
        }
        // c:\\cfes\\log_data
        ret = FileDir::get_file_path_name(NULL, tmp_path, NULL, main_path, sizeof(main_path));
        if (ret == false) {
            return false;
        }
        strncpy(m_log_main_path, main_path, sizeof(m_log_main_path) - 1);
    }

    // current path name: "LogAPath" / "LogBPath"
    GetPrivateProfileString("CURRENT", "CurLogPath", "", cur_path_name, sizeof(cur_path_name), m_log_config_path);
    if (strlen(cur_path_name) == 0) {
        return false;
    }
    // current path: c:\\cfes\\log_data\\log_a / log_b
    if ((strlen(m_cur_path_name) == 0) || (strcmp(m_cur_path_name, cur_path_name) != 0)) {
        // cur path name: "LogAPath" / "LogBPath"
        strncpy(m_cur_path_name, cur_path_name, sizeof(m_cur_path_name) - 1);
        // current path: "log_a" / "log_b"
        GetPrivateProfileString("LOG", cur_path_name, "", tmp_path, sizeof(tmp_path), m_log_ini_path);
        if (strlen(tmp_path) > 0) {
            // c:\\cfes\\log_data\\log_a / log_b
            ret = FileDir::combine_path(m_log_main_path, tmp_path, cur_path, sizeof(cur_path));
            if (ret == false) {
                return false;
            }
        }
        strncpy(m_log_cur_path, cur_path, sizeof(m_log_cur_path) - 1);
    }

    // file max size
    if (m_file_max_size == -1) {
        m_file_max_size = (int)GetPrivateProfileInt("BACKUP", "FileMaxSize", 0, m_log_ini_path);
    }

    // backup path: c:\\cfes\\log_backup
    if (strlen(m_backup_path) == 0) {
        GetPrivateProfileString("BACKUP", "BackupPath", "log_backup", tmp_path, sizeof(tmp_path), m_log_ini_path);
        if (strlen(tmp_path) > 0) {
            ret = FileDir::get_file_path_name(NULL, tmp_path, NULL, m_backup_path, sizeof(m_backup_path));
            if (ret == false) {
                return false;
            }
        }
    }

    // encrypt
    m_encrypt_compress = (GetPrivateProfileInt("BACKUP", "Encrypt", 0, m_log_ini_path) != 0);

    // total max size
    if (m_total_max_size == -1) {
        m_total_max_size = (int)GetPrivateProfileInt("BACKUP", "TotalMaxSize", 0, m_log_ini_path);
    }

    // backup time every day
    if (m_backup_time_every_day.wYear == 0) {
        GetPrivateProfileString("BACKUP", "BackupTime", "", backup_time, sizeof(backup_time), m_log_ini_path);
        if (strlen(backup_time) > 0) {
            if (DataConvertor::split_string_to_numbers(backup_time, ':', 10, bkhhmm, 2) == 2) {
                GetLocalTime(&m_backup_time_every_day);
                m_backup_time_every_day.wHour = (unsigned short)bkhhmm[0];
                m_backup_time_every_day.wMinute = (unsigned short)bkhhmm[1];
                m_backup_time_every_day.wSecond = 0;
                m_backup_time_every_day.wMilliseconds = 0;
            }
        }
    }

    //  last backup time
    GetPrivateProfileString("BACKUP", "LastBackupTime", "", m_last_backup_time_str, sizeof(m_last_backup_time_str), m_log_config_path);
    if (strlen(m_last_backup_time_str) < BACKUP_TIME_STR_LEN) {
        GetLocalTime(&sys_time_now);
        DataConvertor::systime_to_string(&sys_time_now, "%04d%02d%02d", m_last_backup_time_str, sizeof(m_last_backup_time_str));
        strcat(m_last_backup_time_str, "_000000_000");
        WritePrivateProfileString("BACKUP", "LastBackupTime", m_last_backup_time_str, m_log_config_path);
    } else {
        m_last_backup_time_str[BACKUP_TIME_STR_LEN] = '\0';
    }

    // backup save days and count
    if ((m_backup_save_days == -1) || (m_backup_save_count == -1)) {
        m_backup_save_days = (int)GetPrivateProfileInt("BACKUP", "SaveDays", 0, m_log_ini_path);
        m_backup_save_count = (int)GetPrivateProfileInt("BACKUP", "MaxBackupNum", 0, m_log_ini_path);
        if ((m_backup_save_days <= 0) && (m_backup_save_count <= 0)) {
            m_backup_save_days = 0;
            m_backup_save_count = 100;
        }
    }
#else   // LINUX
    // log ini path: c:\\cfes\\ini\\log_manager.ini
    if (strlen(m_log_ini_path) == 0) {
        ret = FileDir::get_file_path_name(SYS_INI_PATH, LOG_INI_FILE, log_ini, sizeof(log_ini));
        if (ret == false) {
            return false;
        }
        strncpy(m_log_ini_path, log_ini, sizeof(m_log_ini_path) - 1);
    }

    // log config path: c:\\cfes\\data\\log_config
    if (strlen(m_log_config_path) == 0) {
        ret = FileDir::get_file_path_name(SYS_DATA_PATH, LOG_CONFIG_FILE, log_config, sizeof(log_config));
        if (ret == false) {
            return false;
        }
        strncpy(m_log_config_path, log_config, sizeof(m_log_config_path) - 1);
    }

    // log main path: c:\\cfes\\log_data
    if (strlen(m_log_main_path) == 0) {
        // main path folder: "log_data"
        INIFile::read_ini_file("LOG", "MainPath", "log_data", tmp_path, sizeof(tmp_path), m_log_ini_path);
        if (strlen(tmp_path) == 0) {
            return false;
        }
        // c:\\cfes\\log_data
        ret = FileDir::get_file_path_name(tmp_path, NULL, main_path, sizeof(main_path));
        if (ret == false) {
            return false;
        }
        strncpy(m_log_main_path, main_path, sizeof(m_log_main_path) - 1);
    }

    // current path name: "LogAPath" / "LogBPath"
    INIFile::read_ini_file("CURRENT", "CurLogPath", "", cur_path_name, sizeof(cur_path_name), m_log_config_path);
    if (strlen(cur_path_name) == 0) {
        return false;
    }
    // current path: c:\\cfes\\log_data\\log_a / log_b
    if ((strlen(m_cur_path_name) == 0) || (strcmp(m_cur_path_name, cur_path_name) != 0)) {
        // cur path name: "LogAPath" / "LogBPath"
        strncpy(m_cur_path_name, cur_path_name, sizeof(m_cur_path_name) - 1);
        // current path: "log_a" / "log_b"
        INIFile::read_ini_file("LOG", cur_path_name, "", tmp_path, sizeof(tmp_path), m_log_ini_path);
        if (strlen(tmp_path) > 0) {
            // c:\\cfes\\log_data\\log_a / log_b
            ret = FileDir::combine_path(m_log_main_path, tmp_path, cur_path, sizeof(cur_path));
            if (ret == false) {
                return false;
            }
        }
        strncpy(m_log_cur_path, cur_path, sizeof(m_log_cur_path) - 1);
    }

    // file max size
    if (m_file_max_size == -1) {
        m_file_max_size = (int)INIFile::read_ini_file("BACKUP", "FileMaxSize", 0, m_log_ini_path);
    }

    // backup path: c:\\cfes\\log_backup
    if (strlen(m_backup_path) == 0) {
        INIFile::read_ini_file("BACKUP", "BackupPath", "log_backup", tmp_path, sizeof(tmp_path), m_log_ini_path);
        if (strlen(tmp_path) > 0) {
            ret = FileDir::get_file_path_name(tmp_path, NULL, m_backup_path, sizeof(m_backup_path));
            if (ret == false) {
                return false;
            }
        }
    }

    // encrypt
    m_encrypt_compress = (INIFile::read_ini_file("BACKUP", "Encrypt", 0, m_log_ini_path) != 0);

    // total max size
    if (m_total_max_size == -1) {
        m_total_max_size = (int)INIFile::read_ini_file("BACKUP", "TotalMaxSize", 0, m_log_ini_path);
    }

    // backup time every day
    if (m_backup_time_every_day.tm_year == 0) {
        INIFile::read_ini_file("BACKUP", "BackupTime", "", backup_time, sizeof(backup_time), m_log_ini_path);
        if (strlen(backup_time) > 0) {
            if (DataConvertor::split_string_to_numbers(backup_time, ':', 10, bkhhmm, 2) == 2) {
                m_backup_time_every_day = *(localtime((const time_t*)(&(t = time(NULL)))));
                m_backup_time_every_day.tm_hour = (unsigned short)bkhhmm[0];
                m_backup_time_every_day.tm_min = (unsigned short)bkhhmm[1];
                m_backup_time_every_day.tm_sec = 0;
            }
        }
    }

    //  last backup time
    INIFile::read_ini_file("BACKUP", "LastBackupTime", "", m_last_backup_time_str, sizeof(m_last_backup_time_str), m_log_config_path);
    if (strlen(m_last_backup_time_str) < BACKUP_TIME_STR_LEN) {
        m_backup_time_every_day = *(localtime((const time_t*)(&(t = time(NULL)))));
#ifdef WIN32
        DataConvertor::systime_to_string(&sys_time_now, "%04d%02d%02d", m_last_backup_time_str, sizeof(m_last_backup_time_str));
#else
        DataConvertor::local_time_to_string(&sys_time_now, "%04d%02d%02d", m_last_backup_time_str, sizeof(m_last_backup_time_str));
#endif // WIN32
        strcat(m_last_backup_time_str, "_000000_000");
        INIFile::write_ini_file("BACKUP", "LastBackupTime", m_last_backup_time_str, m_log_config_path);
    } else {
        m_last_backup_time_str[BACKUP_TIME_STR_LEN] = '\0';
    }

    // backup save days and count
    if ((m_backup_save_days == -1) || (m_backup_save_count == -1)) {
        m_backup_save_days = (int)INIFile::read_ini_file("BACKUP", "SaveDays", 0, m_log_ini_path);
        m_backup_save_count = (int)INIFile::read_ini_file("BACKUP", "MaxBackupNum", 0, m_log_ini_path);
        if ((m_backup_save_days <= 0) && (m_backup_save_count <= 0)) {
            m_backup_save_days = 0;
            m_backup_save_count = 100;
        }
    }
#endif

    return true;
}

/**
 @功能：	切换日志路径（LogAPath <---> LogBPath）
 @参数：	无
 @返回：	true：成功		false：失败
 */
bool LogCtrl::change_current_path()
{
    SetupMutex(LOG_ACCESS_LOCK);

    char cur_path_name[CONST_VALUE_16] = { 0 };

    load_log_path_and_setting();

    if (strcmp(m_cur_path_name, "LogAPath") == 0) {
        strcpy(cur_path_name, "LogBPath");
    } else if (strcmp(m_cur_path_name, "LogBPath") == 0) {
        strcpy(cur_path_name, "LogAPath");
    } else {
        return false;
    }
#ifdef  WIN32
    WritePrivateProfileString("CURRENT", "CurLogPath", cur_path_name, m_log_config_path);
#else   // LINUX
    INIFile::write_ini_file("CURRENT", "CurLogPath", cur_path_name, m_log_config_path);
#endif
    load_log_path_and_setting();

    SyncLock::release_global_lock(LOG_ACCESS_LOCK);

    return true;
}

/**
 @功能：	压缩备份日志
 @参数：	need_encrypt：是否设置解压密码（"cfes"）		backup_reason：备份原因
 @返回：	true：成功		false：失败
 */
bool LogCtrl::log_backup(bool need_encrypt, const char* backup_reason)
{
    bool ret = false;
    char old_cur_path[MAX_PATH] = { 0 };
#ifdef  WIN32
    int idx = 0;
    HANDLE event_log_handle = NULL;
    char event_log_output[MAX_PATH] = { 0 };
#endif
    char stamp_file[64] = { 0 };
    char stamp_time_str[CONST_VALUE_32] = { 0 };
    char stamp_file_path[MAX_PATH] = { 0 };
    const char* zip_password = NULL;

    if (strlen(m_backup_path) == 0) {
        return true;
    }
    if (strlen(m_log_cur_path) == 0) {
        return false;
    }

#ifdef  WIN32
    struct {
        char event_log_name[20];
        char event_log_file[20];
    } event_logs[] = {
        {"APPLICATION", "Application.evt"},
        {"SECURITY", "Security.evt"},
        {"SYSTEM", "System.evt"}
    };
#endif

    strcpy(old_cur_path, m_log_cur_path);

    if (change_current_path() == false) {
        return false;
    }

    FileAccess::copy_file_directory(m_log_main_path, old_cur_path, NULL, NULL, false, false);

    // copy event log
#ifdef  WIN32
    for (idx = 0; idx < 3; idx++) {
        event_log_handle = OpenEventLog(NULL, event_logs[idx].event_log_name);
        if (event_log_handle) {
            if (FileDir::combine_path(old_cur_path,
                event_logs[idx].event_log_file, event_log_output, sizeof(event_log_output))) {
                BackupEventLog(event_log_handle, event_log_output);
            }
            CloseEventLog(event_log_handle);
        }
    }
#endif

    // copy extra files
    backup_extra_files(512, old_cur_path);

    // compress to zip
    ret = DataConvertor::systime_to_string(NULL,	// "stamp_YYYYMMDD_hhmmss_nnn.zip"
        "stamp_%04d%02d%02d_%02d%02d%02d_%03d.zip", stamp_file, sizeof(stamp_file));
    if (ret == false) {
        return false;
    }

    // c:\\cfes\\log_backup\\stamp_YYYYMMDD_hhmmss_nnn.zip
    strncpy(stamp_time_str, &stamp_file[strlen("stamp_")], BACKUP_TIME_STR_LEN);
    ret = FileDir::combine_path(m_backup_path, stamp_file, stamp_file_path, sizeof(stamp_file_path));
    if (ret == false) {
        return false;
    }
    zip_password = (need_encrypt ? "cfes" : NULL);
#ifdef  WIN32
    ret = FileAccess::win_zip_compress(ZIP_COMPRESS, stamp_file_path, old_cur_path, false, zip_password);
#else   // LINUX
    ret = FileAccess::linux_zip_compress(ZIP_COMPRESS, stamp_file_path, old_cur_path, false, zip_password);
#endif
    if (ret == false) {
        return false;
    }
    if (FileAccess::get_file_directory_size(stamp_file_path) == (unsigned long long) - 1) {
        return false;
    }

#ifdef  WIN32
    WritePrivateProfileString("BACKUP", "LastBackupTime", stamp_time_str, m_log_config_path);
    WritePrivateProfileString("BACKUP", "LastBackupReason", (backup_reason ? backup_reason  : ""), m_log_config_path);
#else   // LINUX
    INIFile::write_ini_file("BACKUP", "LastBackupTime", stamp_time_str, m_log_config_path);
    INIFile::write_ini_file("BACKUP", "LastBackupReason", (backup_reason ? backup_reason  : ""), m_log_config_path);
#endif
    strncpy(m_last_backup_time_str, stamp_time_str, sizeof(m_last_backup_time_str) - 1);

    // delete backup files
    if (m_backup_save_days > 0) {
        FileAccess::delete_older_file_directory(m_backup_path, m_backup_save_days);
    }
    if (m_backup_save_count > 0) {
        FileAccess::delete_more_file_in_directory(m_backup_path, m_backup_save_count);
    }

    return FileAccess::delete_file_directory(old_cur_path);
}

/**
 @功能：	检查是否达到日志备份的条件并返回备份原因（字符串）
 @参数：	force_backup：是否强制备份
 @返回：	NULL：未达到日志备份的条件		!= NULL：备份原因（字符串）
 */
const char* LogCtrl::check_backup_reason(bool force_backup/* = false*/)
{
    const char* reason_force = "forcibly backup";
    const char* reason_size = "file max size";
    const char* reason_time = "backup time every day";

    bool ret = false;
    unsigned long long total_size = 0;
#ifdef WIN32
    SYSTEMTIME sys_time_now;
#else
    struct tm sys_time_now;
    time_t t;
#endif // WIN32
    char today_time_str[CONST_VALUE_32] = { 0 };

    total_size = FileAccess::get_file_directory_size(m_log_cur_path);
    if ((total_size == 0) || (total_size == (unsigned long long)-1)) {
        return NULL;
    }

    // force backup
    if (force_backup) {
        return reason_force;
    }

    // check file max size
    if (m_total_max_size > 0) {
        if (total_size > ((unsigned long long)m_total_max_size) * 1024 * 1024) {
            return reason_size;
        }
    }

    // check backup time every day
    if (m_backup_time_every_day.tm_year > 0) {
        sys_time_now = *(localtime((const time_t*)(&(t = time(NULL)))));
        m_backup_time_every_day.tm_year = sys_time_now.tm_year;
        m_backup_time_every_day.tm_mon = sys_time_now.tm_mon;
        m_backup_time_every_day.tm_mday = sys_time_now.tm_mday;
        m_backup_time_every_day.tm_wday = sys_time_now.tm_wday;
#ifdef WIN32
        ret = DataConvertor::systime_to_string(&sys_time_now, "%04d%02d%02d", today_time_str, sizeof(today_time_str));
#else
        ret = DataConvertor::local_time_to_string(&sys_time_now, "%04d%02d%02d", today_time_str, sizeof(today_time_str));
#endif // WIN32
        if (ret) {
            if ((strncmp(today_time_str, m_last_backup_time_str, strlen(today_time_str)) > 0) &&
                (DataConvertor::get_time_diff(sys_time_now, m_backup_time_every_day, TIME_UNIT_SEC) > 0)) {
                return reason_time;
            }
        }
    }

    return NULL;
}

/**
 @功能：	拷贝额外日志文件
 @参数：	max_path_count：最大路径个数		target_path：拷贝目的路径
 @返回：	true：成功		false：失败
 */
bool LogCtrl::backup_extra_files(unsigned short max_path_count, const char* target_path)
{
    bool all_ok = true;
    bool ret = false;
    unsigned short idx = 0;
    char item_key[MAX_PATH] = { 0 };
    char path_str[MAX_PATH] = { 0 };
    char path_src[MAX_PATH] = { 0 };

    if ((max_path_count == 0) ||
        (target_path == NULL) || (strlen(target_path) == 0)) {
        return false;
    }

    all_ok = true;
    for (idx = 0; idx < max_path_count; idx++) {
        snprintf(item_key, sizeof(item_key), "Path%u", idx + 1);
#ifdef  WIN32
        GetPrivateProfileString("EXTRA_BACKUP", item_key, "", path_str, sizeof(path_str), m_log_ini_path);
#else   // LINUX
        INIFile::read_ini_file("EXTRA_BACKUP", item_key, "", path_str, sizeof(path_str), m_log_ini_path);
#endif
        if (strlen(path_str) == 0) {
            break;
        }
        ret = FileDir::get_file_path_name(NULL, path_str, path_src, sizeof(path_src));
        if (ret == false) {
            all_ok = false;
            continue;
        }
        ret = FileAccess::copy_file_directory(path_src, target_path, NULL, NULL, false, true, true);
        if (ret == false) {
            all_ok = false;
            continue;
        }
    }

    return all_ok;
}

/**
 @功能：	当文件大小超过配置值时备份单个文件（改名，加编号后缀）
 @参数：	file_path：文件路径
 @返回：	true：成功		false：失败
 */
bool LogCtrl::backup_file_by_size(const char* file_path)
{
    unsigned long long max_size = 0, file_size = 0;
    unsigned long count = 0;
    char file_name[MAX_PATH] = { 0 };
    char new_file_name[MAX_PATH] = { 0 };

    if ((file_path == NULL) || (strlen(file_path) == 0)) {
        return false;
    }

    if (m_file_max_size <= 0) {
        return true;
    }

    max_size = ((unsigned long long)m_file_max_size) * 1024 * 1024;
    file_size = FileAccess::get_file_directory_size(file_path);
    if (file_size <= max_size) {
        return true;
    }

    snprintf(file_name, sizeof(file_name), "%s%s", file_path, ".*");
    count = FileAccess::find_file_names(file_name, NULL, NULL, 0);
    if (count == 0) {
        return true;
    }
    snprintf(new_file_name, sizeof(new_file_name), "%s.%03lu", file_path, count);

#ifdef  WIN32
    return (MoveFile(file_path, new_file_name) != FALSE);
#else   // LINUX
    return (rename(file_path, new_file_name) == 0);
#endif
}
