#ifndef LOG_CTRL_H
#define LOG_CTRL_H

#include "log_ctrl_global.h"
#include "__common_def.h"
#include "synchronism.h"


#define MAX_LOG_TAG_LEN					(30)

/* output log's level */
#define LOG_LVL_ASSERT					(0)
#define LOG_LVL_ERROR					(1)
#define LOG_LVL_WARN					(2)
#define LOG_LVL_INFO					(3)
#define LOG_LVL_DEBUG					(4)
#define LOG_LVL_VERBOSE					(5)

#define HEX_LOG_NORMAL					(0)
#define HEX_LOG_DUMP					(1)


/****************************************************************************************************
 @功能：	日志出力接口
 @参数：	file_name：日志文件名		log_tag：调用者
 ****************************************************************************************************/
class LOG_CTRL_EXPORT LogCtrl
{
DEFINE_STATIC_VERSION_FUNCTIONS("log_ctrl", "0.0.0.0", TYPE_DYNAMIC)

public:
    LogCtrl();
    virtual ~LogCtrl();

    bool is_registed()
    {
        return ((strlen(m_log_file) > 0) && (strlen(m_log_tag) > 0));
    }
    bool log_register(const char* file_name, const char* log_tag);
    void close_logger();
    void log_output(unsigned char level,
        const char* file, const char* func, const long line, const char* format, ...);
    void hex_output(int data_type, const char* msg, unsigned char* data_buf, unsigned short data_len);
    void log_raw(const char* format, ...);
    void log_msg_raw(const char* msg, const char* format, ...);

public:
    static bool load_log_path_and_setting();
    static bool change_current_path();
    static bool log_backup(bool need_encrypt, const char* backup_reason);
    static bool need_encrypt() { return m_encrypt_compress; }
    static const char* check_backup_reason(bool force_backup = false);
    static bool backup_file_by_size(const char* file_path);

protected:
    bool initialize_logger(const char* log_file_path, const char* log_file_name);
    bool open_file();
    void close_file();
    void close_mutex();

protected:
    static bool backup_extra_files(unsigned short max_path_count, const char* target_path);


protected:
    char m_log_file[MAX_PATH];
    char m_log_file_path[MAX_PATH];
    char m_log_tag[MAX_LOG_TAG_LEN + 1];
    FILE* m_fp;
    SyncLock m_sync_lock;
#ifdef WIN32
    HANDLE m_mutex_handle;
#else
    pthread_mutex_t m_mutex_handle;
#endif

protected:
    static char m_log_ini_path[MAX_PATH];
    static char m_log_config_path[MAX_PATH];
    static char m_log_main_path[MAX_PATH];
    static char m_log_cur_path[MAX_PATH];
    static char m_cur_path_name[CONST_VALUE_16];
    static char m_backup_path[MAX_PATH];
    static char m_last_backup_time_str[CONST_VALUE_32];
    static int m_file_max_size;
    static int m_backup_save_days;
    static int m_backup_save_count;
    static int m_total_max_size;
#ifdef WIN32
    static SYSTEMTIME m_backup_time_every_day;
#else
    static struct tm m_backup_time_every_day;
#endif // WIN32
    static bool m_encrypt_compress;
};

#define LOG_OUTPUT_BY_LEVEL(Logger, Level, ...) \
    Logger.log_output((Level), __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_OUTPUT_ASSERT(Logger, ...) \
    Logger.log_output(LOG_LVL_ASSERT, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_OUTPUT_ERROR(Logger, ...) \
    Logger.log_output(LOG_LVL_ERROR, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_OUTPUT_WARN(Logger, ...) \
    Logger.log_output(LOG_LVL_WARN, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_OUTPUT_INFO(Logger, ...) \
    Logger.log_output(LOG_LVL_INFO, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_OUTPUT_DEBUG(Logger, ...) \
    Logger.log_output(LOG_LVL_DEBUG, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)
#define LOG_OUTPUT_VERBOSE(Logger,  ...) \
    Logger.log_output(LOG_LVL_VERBOSE, __FILE__, __FUNCTION__, __LINE__, __VA_ARGS__)

#endif // LOG_CTRL_H
