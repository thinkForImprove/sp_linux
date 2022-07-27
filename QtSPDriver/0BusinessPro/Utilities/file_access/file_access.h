#ifndef FILE_ACCESS_H
#define FILE_ACCESS_H

#include "file_access_global.h"
#include <vector>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#include "__common_def.h"

using namespace std;

#define	PART_DRIVE					(0x01)
#define	PART_PATH					(0x02)
#define	PART_FILE_NAME				(0x04)
#define	PART_FILE_EXT				(0x08)
#define	PART_FULL_PATH				(PART_DRIVE|PART_PATH)
#define	PART_FULL_NAME				(PART_FILE_NAME|PART_FILE_EXT)

#define VER_INFO_PRODUCTNAME		"ProductName"
#define VER_INFO_PRODUCTVERSION		"ProductVersion"
#define VER_INFO_COMPANYNAME		"CompanyName"
#define VER_INFO_FILEDESC			"FileDescription"
#define VER_INFO_FILEVERSION		"FileVersion"
#define VER_INFO_INTERNALNAME		"InternalName"
#define VER_INFO_COPYRIGHT			"LegalCopyright"
#define VER_INFO_ORIGINALNAME		"OriginalName"

#define FILE_TYPE_UNKOWN			(-1)
#define FILE_TYPE_ASCII				(0)
#define FILE_TYPE_UNICODE			(1)
#define FILE_TYPE_UTF8				(2)

#define ZIP_COMPRESS				(0)
#define ZIP_DECOMPRESS				(1)

#define FILE_TIME_WRITE				(1)
#define FILE_TIME_CREATE			(2)
#define FILE_TIME_ACCESS			(4)
#define FILE_TIME_ALL				(7)


/****************************************************************************************************
 @功能：	文件路径相关操作
 @参数：
 ****************************************************************************************************/
class FILE_ACCESS_EXPORT FileDir
{
DEFINE_STATIC_VERSION_FUNCTIONS("file_access", "0.0.0.0", TYPE_DYNAMIC)

public:
    static bool trim_path(const char* file_path, char* result, unsigned int res_buf_size);
    static bool analyze_path(const char* file_path, unsigned char part, char* result, unsigned int res_buf_size);
    static bool combine_path(const char* file_path1, const char* file_path2, char* result, unsigned int res_buf_size);
    static bool combine_path(
        const char* file_path1, const char* file_path2, const char* file_path3, char* result, unsigned int res_buf_size);
    static bool combine_path(
        const char* file_path1, const char* file_path2, const char* file_path3, const char* file_path4, char* result, unsigned int res_buf_size);
    static bool reduce_path(const char* file_path, int reduce_count, char* result, unsigned int res_buf_size);
    static void _splitpath(const char *path, char *drive, char *dir, char *fname, char *ext);
    static void _split_whole_name(const char *whole_name, char *fname, char *ext);
#ifdef WIN32
    static bool get_module_name(HMODULE module_handle, char* result, unsigned int res_buf_size);
    static bool get_module_path(HMODULE module_handle, char* result, unsigned int res_buf_size);
    static bool get_file_path_name(HMODULE module_handle, const char* upper_folder, const char* file_name, char* result, unsigned int res_buf_size);
#else   // LINUX
    static bool get_module_name(char* result, unsigned int res_buf_size);
    static bool get_module_path(char* result, unsigned int res_buf_size);
    static bool get_file_path_name(const char* upper_folder, const char* file_name, char* result, unsigned int res_buf_size);
    static unsigned long _GetModuleFileName(char* sFileName, unsigned long nSize);
#endif // WIN32
};


/****************************************************************************************************
 @功能：	文件访问相关操作
 @参数：
 ****************************************************************************************************/
class FILE_ACCESS_EXPORT FileAccess :
    public FileDir
{
public:
#ifdef WIN32
    static bool get_file_ver_info(const char* file_path, const char* ver_info_type, char* result, unsigned int res_buf_size);
    static bool get_file_ver_info(HMODULE module_handle, const char* ver_info_type, char* result, unsigned int res_buf_size);

    static unsigned long long win_get_file_directory_size(const char* file_dir_path, LPSYSTEMTIME start_time = NULL, LPSYSTEMTIME end_time = NULL);
    static bool win_get_file_directory_time(
        const char* file_dir_path, LPSYSTEMTIME write_time, LPSYSTEMTIME create_time, LPSYSTEMTIME access_time);
    static bool win_set_file_local_time(
        const char* file_path, LPSYSTEMTIME write_time, LPSYSTEMTIME create_time, LPSYSTEMTIME access_time);
#endif // WIN32
    static unsigned long long posix_get_file_directory_size(const char* file_dir_path, struct tm* start_time = NULL, struct tm* end_time = NULL);
    static bool posix_get_file_directory_time(const char* file_dir_path, struct tm* write_time, struct tm* create_time, struct tm* access_time);

    static bool create_directory_by_path(const char* file_dir_path, bool is_file_path = false);
#ifdef WIN32
    static unsigned long win_find_file_names(
        const char* dir_path, const char* file_name, char name_array[][MAX_PATH], unsigned long max_count);
#endif // WIN32
    static unsigned long posix_find_file_names(
        const char* dir_path, const char* file_name, char name_array[][MAX_PATH], unsigned long max_count);
#ifdef WIN32
    static bool win_copy_file_directory(
        const char* path_src, const char* path_des,
        LPSYSTEMTIME start_time = NULL, LPSYSTEMTIME end_time = NULL,
        bool path_des_is_file = false, bool recursive = true, bool create_path_in_des = false,
        char exclude_src_array[][MAX_PATH] = NULL, unsigned long exclude_count = 0);
#endif // WIN32
    static bool posix_copy_file(const char* path_src, const char* path_des, bool fail_exist);
    static bool posix_copy_file_directory(
        const char* path_src, const char* path_des,
        tm* start_time = NULL, tm* end_time = NULL,
        bool path_des_is_file = false, bool recursive = true, bool create_path_in_des = false,
        char exclude_src_array[][MAX_PATH] = NULL, unsigned long exclude_count = 0);
#ifdef WIN32
    static bool win_delete_file_directory(const char* file_dir_path,
        unsigned long long max_file_size = 0, unsigned long long num_of_second = 0, LPSYSTEMTIME time_base = NULL,
        char exclude_array[][MAX_PATH] = NULL, unsigned long exclude_count = 0);
    static bool win_delete_older_file_directory(const char* file_dir_path, unsigned long num_of_day, LPSYSTEMTIME time_base = NULL,
        char exclude_array[][MAX_PATH] = NULL, unsigned long exclude_count = 0);
#endif // WIN32
    static bool posix_delete_file_directory(const char* file_dir_path,
        unsigned long long max_file_size = 0, unsigned long long num_of_second = 0, tm* time_base = NULL,
        char exclude_array[][MAX_PATH] = NULL, unsigned long exclude_count = 0);
    static bool posix_delete_older_file_directory(const char* file_dir_path, unsigned long num_of_day, tm* time_base = NULL,
        char exclude_array[][MAX_PATH] = NULL, unsigned long exclude_count = 0);
    static bool delete_larger_file_directory(const char* file_dir_path, unsigned long long max_file_size,
        char exclude_array[][MAX_PATH] = NULL, unsigned long exclude_count = 0);
#ifdef WIN32
    static bool win_delete_more_file_in_directory(const char* file_dir_path, unsigned long long max_file_count,
        char exclude_array[][MAX_PATH] = NULL, unsigned long exclude_count = 0);
#endif // WIN32
    static bool posix_delete_more_file_in_directory(const char* file_dir_path, unsigned long long max_file_count,
        char exclude_array[][MAX_PATH] = NULL, unsigned long exclude_count = 0);
#ifdef WIN32
    static bool win_zip_compress(
        int mode, const char* zip_file_path, const char* path,
        bool is_file_path = false, const char* password = NULL, bool recursive = true, unsigned long timeout = INFINITE);
#else   // LINUX
    static bool linux_zip_compress(
        int mode, const char* zip_file_path, const char* path,
        bool is_file_path = false, const char* password = NULL, bool recursive = true, unsigned long timeout = 0/* = INFINITE*/);
#endif // WIN32
    static bool is_file_directory_exist(const char* file_dir_path);
#ifdef WIN32
    static unsigned long win_get_file_count(const char* dir_path, bool recursive = true);
#endif // WIN32
    static unsigned long posix_get_file_count(const char* dir_path, bool recursive = true);
#ifdef WIN32
    static unsigned long win_get_write_time_of_files(
        const char* file_dir_path, bool recursive, LPSYSTEMTIME write_time_array, unsigned long max_count, bool need_sort);
#endif // WIN32
    static unsigned long posix_get_write_time_of_files(
        const char* file_dir_path, bool recursive, tm* write_time_array, unsigned long max_count, bool need_sort);
    static bool create_process(const char* expression);
    static BOOL WriteDataToFile(LPCSTR lpcFile, LPCSTR lpcData, INT nDataSize);
};
#ifdef WIN32
#define find_file_names win_find_file_names
#define get_file_directory_size win_get_file_directory_size
#define get_file_directory_time win_get_file_directory_time
#define copy_file_directory win_copy_file_directory
#define delete_file_directory win_delete_file_directory
#define delete_older_file_directory win_delete_older_file_directory
#define delete_more_file_in_directory win_delete_more_file_in_directory
#define get_file_count win_get_file_count
#define get_write_time_of_files win_get_write_time_of_files
#else
#define find_file_names posix_find_file_names
#define get_file_directory_size posix_get_file_directory_size
#define get_file_directory_time posix_get_file_directory_time
#define copy_file_directory posix_copy_file_directory
#define delete_file_directory posix_delete_file_directory
#define delete_older_file_directory posix_delete_older_file_directory
#define delete_more_file_in_directory posix_delete_more_file_in_directory
#define get_file_count posix_get_file_count
#define get_write_time_of_files posix_get_write_time_of_files
#endif


/****************************************************************************************************
 @功能：	访问INI文件的封装
 @参数：
 ****************************************************************************************************/
//#ifdef WIN32
class FILE_ACCESS_EXPORT INIFile
{
DEFINE_STATIC_VERSION_FUNCTIONS("file_access", "0.0.0.0", TYPE_STATIC)

public:
    static bool write_ini_file(const char* app_name, const char* key_name, unsigned long value, const char* file_path);
    static bool write_ini_file(const char* app_name, const char* key_name, const char* content, const char* file_path);
    static unsigned long read_ini_file(const char* app_name, const char* key_name, const unsigned long def_value, const char* file_path);
    static unsigned long read_ini_file(const char* app_name, const char* key_name, const char* def_content, char* content, unsigned long cont_size, const char* file_path);
    static unsigned long get_ini_sections(const char* file_path, char secion_array[][CONST_VALUE_260], unsigned long max_count);
    static unsigned long get_ini_keys(const char* file_path, const char* app_name, char key_array[][CONST_VALUE_260], unsigned long max_count);
};
//#endif // WIN32


/****************************************************************************************************
 @功能：	按行处理文本文件（支持ASCII、UNICODE、UTF-8）
 @参数：
 ****************************************************************************************************/
class FILE_ACCESS_EXPORT TextFile
{
DEFINE_STATIC_VERSION_FUNCTIONS("file_access", "0.0.0.0", TYPE_STATIC)

public:
    TextFile();
    ~TextFile();

    unsigned long load_text_file(const char* file_path, int file_type = FILE_TYPE_UNKOWN);
    int get_file_type() { return m_file_type; }
    unsigned long get_line_count();
    bool get_line_text(char* str_line, unsigned int line_buffer_size, unsigned long line_index);
    bool del_line_text(unsigned long line_index);
    void insert_line_text(const char* str_line = NULL, unsigned long line_index = (unsigned long)-1);
    bool update_line_text(const char* str_line, unsigned long line_index);
    bool write_text_file(const char* file_path, int file_type = FILE_TYPE_UNKOWN);

protected:
    unsigned long load_text_file_for_unicode(const char* file_path);


protected:
    int m_file_type;
    vector<string> m_file_info_vec;
};

#endif // FILE_ACCESS_H
