// file_access.cpp : 定义静态库的函数。
//

#include "framework.h"
#include "file_access.h"


#ifdef WIN32
struct _last_file_write_time
{
    FILETIME file_time;

    bool operator<(const struct _last_file_write_time& write_time) const
    {
        if (file_time.dwHighDateTime < write_time.file_time.dwHighDateTime)
        {
            return true;
        }
        if (file_time.dwHighDateTime == write_time.file_time.dwHighDateTime)
        {
            return (file_time.dwLowDateTime < write_time.file_time.dwLowDateTime);
        }
        return false;
    }
};
#endif // WIN32


/**
 @功能：	通过文件全路径取得该文件属性中的指定版本信息
 @参数：	file_path：文件全路径			ver_info_type：版本信息种类
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileAccess::get_file_ver_info(const char* file_path, const char* ver_info_type, char* result, unsigned int res_buf_size)
{
    unsigned long info_size = 0;
    unsigned long rtn = 0;
    char* ver_info = NULL;
    void* info = NULL;
    unsigned int info_len = 0;
    char info_type[CONST_VALUE_260] = { 0 };

    if ((result == NULL) || (res_buf_size == 0) ||
        (file_path == NULL) || (strlen(file_path) == 0) ||
        (ver_info_type == NULL) || (strlen(ver_info_type) == 0)) {
        return false;
    }

    info_size = GetFileVersionInfoSize(file_path, NULL);
    if (info_size == 0) {
        return false;
    }
    ver_info = new CHAR[info_size + 1];
    if (ver_info == NULL) {
        return false;
    }
    memset(ver_info, 0x00, info_size + 1);

    rtn = GetFileVersionInfo(file_path, NULL, info_size, ver_info);
    if (rtn == 0) {
        delete[] ver_info;
        return false;
    }
    snprintf(info_type, sizeof(info_type), "\\StringFileInfo\\080404b0\\%s", ver_info_type);
    rtn = VerQueryValue(ver_info, info_type, &info, &info_len);
    if (rtn == 0) {
        memset(info_type, 0x00, sizeof(info_type));
        snprintf(info_type, sizeof(info_type), "\\StringFileInfo\\000004b0\\%s", ver_info_type);
        rtn = VerQueryValue(ver_info, info_type, &info, &info_len);
    }
    if ((rtn == 0) || (info_len == 0)) {
        delete[] ver_info;
        return false;
    }
    if (info_len > res_buf_size) {
        info_len = res_buf_size;
    }
    memset(result, 0x00, res_buf_size);
    memcpy(result, info, info_len);
    delete[] ver_info;

    return true;
}
#endif // WIN32

/**
 @功能：	通过模块句柄取得该模块文件属性中的指定版本信息
 @参数：	module_handle：模块句柄			ver_info_type：版本信息种类
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileAccess::get_file_ver_info(HMODULE module_handle, const char* ver_info_type, char* result, unsigned int res_buf_size)
{
    char module_path[MAX_PATH] = { 0 };
    unsigned long ret = 0;

    if ((result == NULL) || (res_buf_size == 0) ||
        (ver_info_type == NULL) || (strlen(ver_info_type) == 0)) {
        return false;
    }

    ret = GetModuleFileName(module_handle, module_path, sizeof(module_path));
    if (ret == 0) {
        return false;
    }

    return get_file_ver_info(module_path, ver_info_type, result, res_buf_size);
}
#endif // WIN32

/**
 @功能：	计算指定路径（文件/文件夹）中在指定修改时间范围内的文件总大小
 @参数：	file_dir_path：全路径			start_time：最早修改时间（NULL：忽略）
 @			end_time：最晚修改时间（NULL：忽略）
 @返回：	总大小（-1：失败）
 */
#ifdef WIN32
unsigned long long FileAccess::win_get_file_directory_size(const char* file_dir_path, LPSYSTEMTIME start_time/* = NULL*/, LPSYSTEMTIME end_time/* = NULL*/)
{
    unsigned long long total_size = 0, size = 0;
    WIN32_FIND_DATA file_info;
    HANDLE find_handle = NULL;
    char find_path[MAX_PATH] = { 0 };
    bool ret = false;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0)) {
        return (unsigned long long)-1;
    }
    ret = trim_path(file_dir_path, find_path, sizeof(find_path));
    if (ret == false) {
        return (unsigned long long)-1;
    }

    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return (unsigned long long)-1;
    }
    FindClose(find_handle);
    if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        ret = combine_path(find_path, "*", find_path, sizeof(find_path));
        if (ret == false) {
            return (unsigned long long)-1;
        }
    }

    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return (unsigned long long)-1;
    }
    do {
        if ((strcmp(file_info.cFileName, ".") == 0) ||
            (strcmp(file_info.cFileName, "..") == 0)) {
            continue;
        }
        if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            ret = combine_path(file_dir_path, file_info.cFileName, find_path, sizeof(find_path));
            if (ret == false) {
                FindClose(find_handle);
                return (unsigned long long)-1;
            }
            size = win_get_file_directory_size(find_path, start_time, end_time);
            if (size != (unsigned long long)-1) {
                total_size += size;
            }
        } else {
            if (start_time != NULL && end_time != NULL) {
                if ((DataConvertor::get_time_diff(file_info.ftLastWriteTime, *start_time, TIME_UNIT_SEC) >= 0) &&
                    (DataConvertor::get_time_diff(file_info.ftLastWriteTime, *end_time, TIME_UNIT_SEC) <= 0)) {
                    total_size += (((unsigned long long)file_info.nFileSizeHigh) << 32) + file_info.nFileSizeLow;
                }
            } else if (start_time != NULL) {
                if (DataConvertor::get_time_diff(file_info.ftLastWriteTime, *start_time, TIME_UNIT_SEC) >= 0) {
                    total_size += (((unsigned long long)file_info.nFileSizeHigh) << 32) + file_info.nFileSizeLow;
                }
            } else if (end_time != NULL) {
                if (DataConvertor::get_time_diff(file_info.ftLastWriteTime, *end_time, TIME_UNIT_SEC) <= 0) {
                    total_size += (((unsigned long long)file_info.nFileSizeHigh) << 32) + file_info.nFileSizeLow;
                }
            } else {
                total_size += (((unsigned long long)file_info.nFileSizeHigh) << 32) + file_info.nFileSizeLow;
            }
        }
    } while (FindNextFile(find_handle, &file_info) != FALSE);

    FindClose(find_handle);

    return total_size;
}
#endif

/**
 @功能：	取得（文件/文件夹）时间（修改时间、创建时间、访问时间）
 @参数：	file_dir_path：全路径						write_time：存放修改时间（NULL：忽略）
 @			create_time：存放创建时间（NULL：忽略）		access_time：存放访问时间（NULL：忽略）
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileAccess::win_get_file_directory_time(
    const char* file_dir_path, LPSYSTEMTIME write_time, LPSYSTEMTIME create_time, LPSYSTEMTIME access_time)
{
    WIN32_FILE_ATTRIBUTE_DATA file_attr;
    char file_path[MAX_PATH] = { 0 };
    bool ret = false;
    bool all_ok = true;
    FILETIME local_write_time, local_create_time, local_access_time;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0) ||
        ((write_time == NULL) && (create_time == NULL) && (access_time == NULL))) {
        return false;
    }

    ret = trim_path(file_dir_path, file_path, sizeof(file_path));
    if (ret == false) {
        return false;
    }

    memset(&file_attr, 0x00, sizeof(file_attr));
    ret = GetFileAttributesEx(file_path, GetFileExInfoStandard, &file_attr);
    if (ret == false) {
        return false;
    }

    if (write_time != NULL) {
        ret = FileTimeToLocalFileTime(&file_attr.ftLastWriteTime, &local_write_time);
        if (ret) {
            ret = FileTimeToSystemTime(&local_write_time, write_time);
        }
        if (ret == false) {
            all_ok = false;
        }
    }
    if (create_time != NULL) {
        ret = FileTimeToLocalFileTime(&file_attr.ftCreationTime, &local_create_time);
        if (ret) {
            ret = FileTimeToSystemTime(&local_create_time, create_time);
        }
        if (ret == false) {
            all_ok = false;
        }
    }
    if (access_time != NULL) {
        ret = FileTimeToLocalFileTime(&file_attr.ftLastAccessTime, &local_access_time);
        if (ret) {
            ret = FileTimeToSystemTime(&local_access_time, access_time);
        }
        if (ret == false) {
            all_ok = false;
        }
    }

    return all_ok;
}
#endif

/**
 @功能：	设置文件时间（修改时间、创建时间、访问时间）
 @参数：	file_path：全路径							write_time：存放修改时间（NULL：忽略）
 @			create_time：存放创建时间（NULL：忽略）		access_time：存放访问时间（NULL：忽略）
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileAccess::win_set_file_local_time(
    const char* file_path, LPSYSTEMTIME write_time, LPSYSTEMTIME create_time, LPSYSTEMTIME access_time)
{
    HANDLE file_handle = NULL;
    char local_file_path[MAX_PATH] = { 0 };
    bool ret = false;
    bool all_ok = true;
    FILETIME file_write_time, local_write_time;
    FILETIME file_create_time, local_create_time;
    FILETIME file_access_time, local_access_time;
    LPFILETIME file_write_time_ptr = NULL;
    LPFILETIME file_create_time_ptr = NULL;
    LPFILETIME file_access_time_ptr = NULL;

    if ((file_path == NULL) || (strlen(file_path) == 0) ||
        ((write_time == NULL) && (create_time == NULL) && (access_time == NULL))) {
        return false;
    }

    ret = trim_path(file_path, local_file_path, sizeof(local_file_path));
    if (ret == false) {
        return false;
    }

    file_handle = CreateFile(local_file_path, GENERIC_WRITE, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file_handle == INVALID_HANDLE_VALUE) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "CreateFile", file_handle, true, local_file_path);
        return FALSE;
    }

    if (write_time != NULL) {
        ret = SystemTimeToFileTime(write_time, &local_write_time);
        if (ret) {
            ret = LocalFileTimeToFileTime(&local_write_time, &file_write_time);
        }
        if (ret == false) {
            all_ok = false;
        } else {
            file_write_time_ptr = &file_write_time;
        }
    }
    if (create_time != NULL) {
        ret = SystemTimeToFileTime(create_time, &local_create_time);
        if (ret) {
            ret = LocalFileTimeToFileTime(&local_create_time, &file_create_time);
        }
        if (ret == false) {
            all_ok = false;
        } else {
            file_create_time_ptr = &file_create_time;
        }
    }
    if (access_time != NULL) {
        ret = SystemTimeToFileTime(access_time, &local_access_time);
        if (ret) {
            ret = LocalFileTimeToFileTime(&local_access_time, &file_access_time);
        }
        if (ret == false) {
            all_ok = false;
        } else {
            file_access_time_ptr = &file_access_time;
        }
    }
    ret = SetFileTime(file_handle, file_create_time_ptr, file_access_time_ptr, file_write_time_ptr);
    if (ret == false) {
        all_ok = false;
    }

    CloseHandle(file_handle);

    return all_ok;
}
#endif

/**
 @功能：	计算指定路径（文件/文件夹）中在指定修改时间范围内的文件总大小
 @参数：	file_dir_path：全路径			start_time：最早修改时间（NULL：忽略）
 @			end_time：最晚修改时间（NULL：忽略）
 @返回：	总大小（-1：失败）
 */
unsigned long long FileAccess::posix_get_file_directory_size(const char* file_dir_path, struct tm* start_time/* = NULL*/, struct tm* end_time/* = NULL*/)
{
    unsigned long long total_size = 0, size = 0;
#ifdef WIN32
    struct _finddata_t file_info;
    long find_handle = 0;
#else // LINUX
    struct stat file_stat;
    struct dirent *dir_ent;
    DIR *dir = NULL;
    int find_handle = 0;
#endif
    char find_path[MAX_PATH] = { 0 };
    bool ret = false;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0)) {
        return (unsigned long long)-1;
    }
    ret = trim_path(file_dir_path, find_path, sizeof(find_path));
    if (ret == false) {
        return (unsigned long long)-1;
    }

#ifdef WIN32
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return (unsigned long long)-1;
    }
    _findclose(find_handle);
    if ((file_info.attrib & _A_SUBDIR) != 0) {
        ret = combine_path(find_path, "*", find_path, sizeof(find_path));
        if (ret == false) {
            return (unsigned long long)-1;
        }
    }

    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return (unsigned long long)-1;
    }
    do {
        if ((strcmp(file_info.name, ".") == 0) ||
            (strcmp(file_info.name, "..") == 0)) {
            continue;
        }
        if ((file_info.attrib & _A_SUBDIR) != 0) {
            ret = combine_path(file_dir_path, file_info.name, find_path, sizeof(find_path));
            if (ret == false) {
                _findclose(find_handle);
                return (unsigned long long)-1;
            }
            size = posix_get_file_directory_size(find_path, start_time, end_time);
            if (size != (unsigned long long)-1) {
                total_size += size;
            }
        } else {
            if (start_time != NULL && end_time != NULL) {
                if ((DataConvertor::get_time_diff(file_info.time_write, *start_time, TIME_UNIT_SEC) >= 0) &&
                    (DataConvertor::get_time_diff(file_info.time_write, *end_time, TIME_UNIT_SEC) <= 0)) {
                    total_size += file_info.size;
                }
            } else if (start_time != NULL) {
                if (DataConvertor::get_time_diff(file_info.time_write, *start_time, TIME_UNIT_SEC) >= 0) {
                    total_size += file_info.size;
                }
            } else if (end_time != NULL) {
                if (DataConvertor::get_time_diff(file_info.time_write, *end_time, TIME_UNIT_SEC) <= 0) {
                    total_size += file_info.size;
                }
            } else {
                total_size += file_info.size;
            }
        }
    } while (_findnext(find_handle, &file_info) == 0);

    _findclose(find_handle);
#else   // Linux
    memset(&file_stat, 0x00, sizeof(file_stat));
    find_handle = stat(find_path, &file_stat);
    if (find_handle == -1) {
        return (unsigned long long)-1;
    }
    if (S_ISREG(file_stat.st_mode ) == true) {   // 一般文件
        if (start_time != NULL && end_time != NULL) {
            if ((DataConvertor::get_time_diff(file_stat.st_mtime, *start_time, TIME_UNIT_SEC) >= 0) &&
                (DataConvertor::get_time_diff(file_stat.st_mtime, *end_time, TIME_UNIT_SEC) <= 0)) {
                total_size += file_stat.st_size;
            }
        } else if (start_time != NULL) {
            if (DataConvertor::get_time_diff(file_stat.st_mtime, *start_time, TIME_UNIT_SEC) >= 0) {
                total_size += file_stat.st_size;
            }
        } else if (end_time != NULL) {
            if (DataConvertor::get_time_diff(file_stat.st_mtime, *end_time, TIME_UNIT_SEC) <= 0) {
                total_size += file_stat.st_size;
            }
        } else {
            total_size += file_stat.st_size;
        }
        return total_size;
    }
    if (S_ISDIR(file_stat.st_mode) == true) {   // 目录
        if ((dir = opendir(find_path)) == NULL) {
            return (unsigned long long)-1;
        }
        while ((dir_ent = readdir(dir)) != NULL) {
            if ((strcmp(dir_ent->d_name, ".") == 0) ||
                (strcmp(dir_ent->d_name, "..") == 0)) {
                continue;
            }
            ret = combine_path(file_dir_path, dir_ent->d_name, find_path, sizeof(find_path));
            if (ret == false) {
                closedir(dir);
                return (unsigned long long)-1;
            }
            if (dir_ent->d_type == DT_DIR) {    // 目录
                size = posix_get_file_directory_size(find_path, start_time, end_time);
                if (size != (unsigned long long)-1) {
                    total_size += size;
                }
            } else {
                memset(&file_stat, 0x00, sizeof(file_stat));
                find_handle = stat(find_path, &file_stat);
                if (find_handle == -1) {
                    closedir(dir);
                    return (unsigned long long)-1;
                }
                if (S_ISREG(file_stat.st_mode ) == true) {   // 一般文件
                    if (start_time != NULL && end_time != NULL) {
                        if ((DataConvertor::get_time_diff(file_stat.st_mtime, *start_time, TIME_UNIT_SEC) >= 0) &&
                            (DataConvertor::get_time_diff(file_stat.st_mtime, *end_time, TIME_UNIT_SEC) <= 0)) {
                            total_size += file_stat.st_size;
                        }
                    } else if (start_time != NULL) {
                        if (DataConvertor::get_time_diff(file_stat.st_mtime, *start_time, TIME_UNIT_SEC) >= 0) {
                            total_size += file_stat.st_size;
                        }
                    } else if (end_time != NULL) {
                        if (DataConvertor::get_time_diff(file_stat.st_mtime, *end_time, TIME_UNIT_SEC) <= 0) {
                            total_size += file_stat.st_size;
                        }
                    } else {
                        total_size += file_stat.st_size;
                    }
                }
            }
        }
    }
    closedir(dir);
#endif
    return total_size;
}

/**
 @功能：	取得（文件/文件夹）时间（修改时间、创建时间、访问时间）
 @参数：	file_dir_path：全路径						write_time：存放修改时间（NULL：忽略）
 @			create_time：存放创建时间（NULL：忽略）		access_time：存放访问时间（NULL：忽略）
 @返回：	true：成功		false：失败
 */
bool FileAccess::posix_get_file_directory_time(const char* file_dir_path, struct tm* write_time, struct tm* create_time, struct tm* access_time)
{
#ifdef WIN32
    struct _finddata_t file_attr;
    long file_handle = 0;
#else // LINUX
    struct stat file_stat;
    int find_handle = 0;
#endif
    char file_path[MAX_PATH] = { 0 };
    bool ret = false;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0) ||
        ((write_time == NULL) && (create_time == NULL) && (access_time == NULL))) {
        return false;
    }

    ret = trim_path(file_dir_path, file_path, sizeof(file_path));
    if (ret == false) {
        return false;
    }

#ifdef WIN32
    memset(&file_attr, 0x00, sizeof(file_attr));
    file_handle = _findfirst(file_path, &file_attr);
    if (file_handle == 0) {
        return false;
    }
    _findclose(file_handle);

    if (write_time != NULL) {
        *write_time = *(localtime(&file_attr.time_write));
    }
    if (create_time != NULL) {
        *create_time = *(localtime(&file_attr.time_create));
    }
    if (access_time != NULL) {
        *access_time = *(localtime(&file_attr.time_access));
    }
#else // LINUX
    memset(&file_stat, 0x00, sizeof(file_stat));
    find_handle = stat(file_path, &file_stat);
    if (find_handle == -1) {
        return (unsigned long long)-1;
    }
    if (write_time != NULL) {
        *write_time = *(localtime((const time_t*)file_stat.st_mtim.tv_sec));
    }
    if (create_time != NULL) {
        create_time = NULL;    // LINUX没有文件建立时间，返回NULL
    }
    if (access_time != NULL) {
        *access_time = *(localtime((const time_t*)&file_stat.st_ctim.tv_sec));
    }
#endif

    return true;
}

/**
 @功能：	通过全路径创建文件夹
 @参数：	file_dir_path：全路径			is_file_path：全路径是否带有文件名
 @返回：	true：成功		false：失败
 */
bool FileAccess::create_directory_by_path(const char* file_dir_path, bool is_file_path/* = false*/)
{
    bool ret = false;
    char new_dir[MAX_PATH] = { 0 };
    char buf_dir[MAX_PATH] = { 0 };
    size_t len = 0;

    if (!IS_FULL_PATH(file_dir_path)) {
        return false;
    }

    strncpy(new_dir, file_dir_path, sizeof(new_dir) - 1);
    len = strlen(new_dir);
    if (new_dir[len - 1] == PATH_SPT_CHR) {
        trim_path(new_dir, new_dir, sizeof(new_dir));
        len = strlen(new_dir);
    }
    else {
        if (is_file_path) {
            ret = analyze_path(file_dir_path, PART_FULL_PATH, new_dir, sizeof(new_dir));
            if (ret == false) {
                return false;
            }
        }
    }

#ifdef WIN32
    if (strlen(new_dir) == 2) {
        strcat(new_dir, PATH_SPT_STR);
        return (GetVolumeInformation(new_dir, NULL, 0, NULL, NULL, NULL, NULL, 0) != FALSE);
    }
#else
    if (strcmp(new_dir, PATH_SPT_STR) == 0) {
        return true;
    }
#endif

#ifdef WIN32
    ret = ((CreateDirectory(new_dir, NULL) == FALSE) && (GetLastError() != ERROR_ALREADY_EXISTS));
#else
    //ret = ((_access(new_dir, 0) != 0) && (_mkdir(new_dir) != 0));
    ret = ((access(new_dir, F_OK) != 0) && (mkdir(new_dir, S_IRWXU | S_IRGRP | S_IROTH) != 0));
#endif
    if (ret) {
        ret = reduce_path(new_dir, 1, buf_dir, sizeof(buf_dir));
        if (ret == false) {
            return false;
        }
        ret = create_directory_by_path(buf_dir, false);
        if (ret == false) {
            return false;
        }
        return create_directory_by_path(new_dir, false);
    }

    return true;
}

/**
 @功能：	在指定文件夹查找指定文件名（支持通配符）的文件
 @参数：	dir_path：指定文件夹					file_name：指定文件名（支持通配符）
 @			name_array：存放找到的文件名Buffer		max_count：存放文件名最大个数
 @返回：	找到的文件名个数（支持 name_array=NULL or max_count=0）
 */
#ifdef WIN32
unsigned long FileAccess::win_find_file_names(
    const char* dir_path, const char* file_name, char name_array[][MAX_PATH], unsigned long max_count)
{
    unsigned long file_count = 0;
    bool ret = false;
    char find_path[MAX_PATH] = { 0 };
    WIN32_FIND_DATA file_info;
    HANDLE find_handle = NULL;

    if ((dir_path == NULL) || (strlen(dir_path) == 0)) {
        return 0;
    }

    ret = combine_path(dir_path, file_name, find_path, sizeof(find_path));
    if (ret == false) {
        return 0;
    }

    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    file_count = 0;
    do {
        if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            continue;
        }
        if ((name_array != NULL) && (file_count < max_count)) {
            strncpy(name_array[file_count], file_info.cFileName, MAX_PATH - 1);
        }
        file_count++;
    } while (FindNextFile(find_handle, &file_info) != FALSE);
    FindClose(find_handle);

    if ((name_array != NULL) && (max_count > 0) && (file_count > max_count)) {
        file_count = max_count;
    }

    return file_count;
}
#endif // WIN32

/**
 @功能：	在指定文件夹查找指定文件名（支持通配符）的文件
 @参数：	dir_path：指定文件夹					file_name：指定文件名（支持通配符）
 @			name_array：存放找到的文件名Buffer		max_count：存放文件名最大个数
 @返回：	找到的文件名个数（支持 name_array=NULL or max_count=0）
 */
unsigned long FileAccess::posix_find_file_names(
    const char* dir_path, const char* file_name, char name_array[][MAX_PATH], unsigned long max_count)
{
    unsigned long file_count = 0;
#ifdef WIN32
    bool ret = false;
    char find_path[MAX_PATH] = { 0 };
    struct _finddata_t file_info;
    long find_handle = NULL;
#else // LINUX
    struct dirent *dir_ent;
    DIR *dir;
    int find_ret = 0;
#endif

    if ((dir_path == NULL) || (strlen(dir_path) == 0)) {
        return 0;
    }

#ifdef WIN32
    ret = combine_path(dir_path, file_name, find_path, sizeof(find_path));
    if (ret == false) {
        return 0;
    }
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return 0;
    }

    file_count = 0;
    do {
        if ((file_info.attrib & _A_SUBDIR) != 0) {
            continue;
        }
        if ((name_array != NULL) && (file_count < max_count)) {
            strncpy(name_array[file_count], file_info.name, MAX_PATH - 1);
        }
        file_count++;
    } while (_findnext(find_handle, &file_info) == 0);
    _findclose(find_handle);
#else  // LINUX
    if ((dir = opendir(dir_path)) == NULL) {
       return 0;
    }
    while ((dir_ent = readdir(dir)) != NULL) {
        if (dir_ent->d_type == DT_DIR) {    // 目录
            continue;
        }
        find_ret = fnmatch(file_name, dir_ent->d_name, FNM_PATHNAME|FNM_PERIOD); // 模糊匹配
        if (find_ret == 0)	{
            if ((name_array != NULL) && (file_count < max_count)) {
                strncpy(name_array[file_count], dir_ent->d_name, MAX_PATH - 1);
            }
            file_count++;
        } else
        if(find_ret == FNM_NOMATCH) {
            continue ;
        }
    }
    closedir(dir);
#endif

    if ((name_array != NULL) && (max_count > 0) && (file_count > max_count)) {
        file_count = max_count;
    }

    return file_count;
}

/**
 @功能：	拷贝文件夹或文件
 @参数：	path_src：拷贝元路径							path_des：拷贝先路径
 @			start_time：文件修改时间最早（NULL：忽略）		end_time：文件修改时间最晚（NULL：忽略）
 @			path_des_is_file：拷贝先路径是否是文件			recursive：是否拷贝子文件夹
 @			create_path_in_des：是否创建拷贝先文件夹		exclude_src_array：拷贝时排除的文件列表（NULL：忽略）
 @			exclude_count：列表中文件个数（0：忽略）
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileAccess::win_copy_file_directory(
    const char* path_src, const char* path_des,
    LPSYSTEMTIME start_time/* = NULL*/, LPSYSTEMTIME end_time/* = NULL*/,
    bool path_des_is_file/* = false*/, bool recursive/* = true*/, bool create_path_in_des/* = false*/,
    char exclude_src_array[][MAX_PATH]/* = NULL*/, unsigned long exclude_count/* = 0*/)
{
    bool all_copied = true;
    bool ret = false;
    WIN32_FIND_DATA file_info;
    HANDLE find_handle = NULL;
    char find_path[MAX_PATH] = { 0 };
    char des_file_path[MAX_PATH] = { 0 };
    char des_file_path_temp[MAX_PATH] = { 0 };
    char src_file_path[MAX_PATH] = { 0 };
    char src_folder[MAX_PATH] = { 0 };
    unsigned long idx = 0;

    if ((path_src == NULL) || (strlen(path_src) == 0) ||
        (path_des == NULL) || (strlen(path_des) == 0)) {
        return false;
    }
    if (strcmp(path_src, path_des) == 0) {
        return true;
    }

    ret = trim_path(path_src, find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }
    if ((exclude_src_array != NULL) && (exclude_count > 0)) {
        for (idx = 0; idx < exclude_count; idx++) {
            if (strcmp(find_path, exclude_src_array[idx]) == 0) {
                break;
            }
        }
        if (idx < exclude_count) {
            return true;
        }
    }
    ret = trim_path(path_des, des_file_path, sizeof(des_file_path));
    if (ret == false) {
        return false;
    }
    ret = create_directory_by_path(des_file_path, path_des_is_file);
    if (ret == false) {
        return false;
    }

    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return true;
    }
    FindClose(find_handle);

    if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        if (path_des_is_file == false) {
            ret = combine_path(
                path_des, file_info.cFileName, des_file_path, sizeof(des_file_path));
            if (ret == false) {
                return false;
            }
        } else {
            ret = trim_path(path_des, des_file_path, sizeof(des_file_path));
            if (ret == false) {
                return false;
            }
        }
        SetFileAttributes(des_file_path, FILE_ATTRIBUTE_ARCHIVE);
        if (start_time != NULL && end_time != NULL) {
            if ((DataConvertor::get_time_diff(file_info.ftLastWriteTime, *start_time, TIME_UNIT_SEC) >= 0) &&
                (DataConvertor::get_time_diff(file_info.ftLastWriteTime, *end_time, TIME_UNIT_SEC) <= 0)) {
                return (CopyFile(find_path, des_file_path, FALSE) != FALSE);
            } else {
                return true;
            }
        } else if (start_time != NULL) {
            if (DataConvertor::get_time_diff(file_info.ftLastWriteTime, *start_time, TIME_UNIT_SEC) >= 0) {
                return (CopyFile(find_path, des_file_path, FALSE) != FALSE);
            } else {
                return true;
            }
        } else if (end_time != NULL) {
            if (DataConvertor::get_time_diff(file_info.ftLastWriteTime, *end_time, TIME_UNIT_SEC) <= 0) {
                ret = (CopyFile(find_path, des_file_path, FALSE) != FALSE);
                if (ret == false) {
                    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "CopyFile", ret, true, des_file_path);
                }
                return ret;
            }
        } else {
            ret = (CopyFile(find_path, des_file_path, FALSE) != FALSE);
            if (ret == false) {
                REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "CopyFile", ret, true, des_file_path);
            }
            return ret;
        }
    }

    if ((path_des_is_file == false) && (create_path_in_des != false)) {
        ret = analyze_path(
            find_path, PART_FILE_NAME | PART_FILE_EXT, src_folder, sizeof(src_folder));
        if (ret == false) {
            return false;
        }
        ret = combine_path(des_file_path, src_folder, des_file_path, sizeof(des_file_path));
        if (ret == false) {
            return false;
        }
        CreateDirectory(des_file_path, NULL);
        return win_copy_file_directory(
            find_path, des_file_path, start_time, end_time, false, recursive, false, exclude_src_array, exclude_count);
    }

    ret = combine_path(find_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    all_copied = true;
    do {
        if ((strcmp(file_info.cFileName, ".") == 0) ||
            (strcmp(file_info.cFileName, "..") == 0)) {
            continue;
        }
        ret = combine_path(path_src, file_info.cFileName, src_file_path, sizeof(src_file_path));
        if (ret == false) {
            all_copied = false;
            continue;
        }
        if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            if (recursive == false) {
                continue;
            }
            ret = combine_path(path_des, file_info.cFileName, des_file_path_temp, sizeof(des_file_path_temp));
            if (ret == false) {
                all_copied = false;
                continue;
            }
            ret = win_copy_file_directory(
                src_file_path, des_file_path_temp, start_time, end_time, false, true, false, exclude_src_array, exclude_count);
        } else {
            ret = win_copy_file_directory(
                src_file_path, des_file_path, start_time, end_time, false, true, false, exclude_src_array, exclude_count);
        }
        if (ret == false) {
            all_copied = false;
        }
    } while (FindNextFile(find_handle, &file_info) != FALSE);
    FindClose(find_handle);

    return all_copied;
}
#endif // WIN32

/**
 @功能：	拷贝文件
 @参数：	path_src：拷贝元路径							path_des：拷贝先路径
 @			fail_exist：拷贝先存在时报错
 @返回：	true：成功		false：失败
 */
bool FileAccess::posix_copy_file(const char* path_src, const char* path_des, bool fail_exist)
{
    FILE* in = NULL, * out = NULL;
    unsigned char buff[CONST_VALUE_1024] = { 0 };
    size_t len = 0;

    if ((path_src == NULL) || (strlen(path_src) == 0) ||
        (path_des == NULL) || (strlen(path_des) == 0)) {
        return false;
    }
    if (strcmp(path_src, path_des) == 0) {
        return true;
    }

    if ((fail_exist) &&
#ifdef WIN32
        (_access(path_des, 0) == 0)) {
#else // LINUX
        (access(path_des, F_OK) == 0)) {
#endif
        return false;
    }

    if ((in = fopen(path_src, "rb+")) == NULL) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", in, true, path_src);
        return false;
    }
    if ((out = fopen(path_des, "wb+")) == NULL) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", out, true, path_des);
        fclose(in);
        return false;
    }

    while ((len = fread(buff, 1, sizeof(buff), in)) != 0) {
        if ((len < sizeof(buff)) && (ferror(in))) {
            REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fread", len, true, path_src);
            fclose(in);
            fclose(out);
            return false;
        }
        if (fwrite(buff, 1, len, out) != len) {
            REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fwrite", len, true, path_des);
            fclose(in);
            fclose(out);
            return false;
        }
    }

    fclose(in);
    fclose(out);

    return true;
}

/**
 @功能：	拷贝文件夹或文件
 @参数：	path_src：拷贝元路径							path_des：拷贝先路径
 @			start_time：文件修改时间最早（NULL：忽略）		end_time：文件修改时间最晚（NULL：忽略）
 @			path_des_is_file：拷贝先路径是否是文件			recursive：是否拷贝子文件夹
 @			create_path_in_des：是否创建拷贝先文件夹		exclude_src_array：拷贝时排除的文件列表（NULL：忽略）
 @			exclude_count：列表中文件个数（0：忽略）
 @返回：	true：成功		false：失败
 */
bool FileAccess::posix_copy_file_directory(
    const char* path_src, const char* path_des,
    tm* start_time/* = NULL*/, tm* end_time/* = NULL*/,
    bool path_des_is_file/* = false*/, bool recursive/* = true*/, bool create_path_in_des/* = false*/,
    char exclude_src_array[][MAX_PATH]/* = NULL*/, unsigned long exclude_count/* = 0*/)
{
    bool all_copied = true;
    bool ret = false;
#ifdef WIN32
    struct _finddata_t file_info;
    long find_handle = NULL;
#else // LINUX
    struct stat file_stat;
    struct dirent *dir_ent;
    DIR *dir;
    int find_handle = 0;
    char file_name[MAX_PATH] = { 0 };
#endif
    char find_path[MAX_PATH] = { 0 };
    char des_file_path[MAX_PATH] = { 0 };
    char des_file_path_temp[MAX_PATH] = { 0 };
    char src_file_path[MAX_PATH] = { 0 };
    char src_folder[MAX_PATH] = { 0 };
    unsigned long idx = 0;

    if ((path_src == NULL) || (strlen(path_src) == 0) ||
        (path_des == NULL) || (strlen(path_des) == 0)) {
        return false;
    }
    if (strcmp(path_src, path_des) == 0) {
        return true;
    }

    ret = trim_path(path_src, find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }
    if ((exclude_src_array != NULL) && (exclude_count > 0)) {
        for (idx = 0; idx < exclude_count; idx++) {
            if (strcmp(find_path, exclude_src_array[idx]) == 0) {
                break;
            }
        }
        if (idx < exclude_count) {
            return true;
        }
    }
    ret = trim_path(path_des, des_file_path, sizeof(des_file_path));
    if (ret == false) {
        return false;
    }
    ret = create_directory_by_path(des_file_path, path_des_is_file);
    if (ret == false) {
        return false;
    }

#ifdef WIN32
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return true;
    }
    _findclose(find_handle);

    if ((file_info.attrib & _A_SUBDIR) == 0) {
        if (path_des_is_file == false) {
            ret = combine_path(
                path_des, file_info.name, des_file_path, sizeof(des_file_path));
            if (ret == false) {
                return false;
            }
        } else {
            ret = trim_path(path_des, des_file_path, sizeof(des_file_path));
            if (ret == false) {
                return false;
            }
        }
        _chmod(des_file_path, _S_IREAD | _S_IWRITE);
        if (start_time != NULL && end_time != NULL) {
            if ((DataConvertor::get_time_diff(file_info.time_write, *start_time, TIME_UNIT_SEC) >= 0) &&
                (DataConvertor::get_time_diff(file_info.time_write, *end_time, TIME_UNIT_SEC) <= 0)) {
                return (CopyFile(find_path, des_file_path, FALSE) != FALSE);
            } else {
                return true;
            }
        } else if (start_time != NULL) {
            if (DataConvertor::get_time_diff(file_info.time_write, *start_time, TIME_UNIT_SEC) >= 0) {
                return (CopyFile(find_path, des_file_path, FALSE) != FALSE);
            } else {
                return true;
            }
        } else if (end_time != NULL) {
            if (DataConvertor::get_time_diff(file_info.time_write, *end_time, TIME_UNIT_SEC) <= 0) {
                ret = posix_copy_file(find_path, des_file_path, false);
                if (ret == false) {
                    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "copy_file", ret, true, des_file_path);
                }
                return ret;
            }
        } else {
            ret = posix_copy_file(find_path, des_file_path, false);
            if (ret == false) {
                REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "copy_file", ret, true, des_file_path);
            }
            return ret;
        }
    }

    if ((path_des_is_file == false) && (create_path_in_des != false)) {
        ret = analyze_path(
            find_path, PART_FILE_NAME | PART_FILE_EXT, src_folder, sizeof(src_folder));
        if (ret == false) {
            return false;
        }
        ret = combine_path(des_file_path, src_folder, des_file_path, sizeof(des_file_path));
        if (ret == false) {
            return false;
        }
        _mkdir(des_file_path);
        return posix_copy_file_directory(
            find_path, des_file_path, start_time, end_time, false, recursive, false, exclude_src_array, exclude_count);
    }

    ret = combine_path(find_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return false;
    }

    all_copied = true;
    do {
        if ((strcmp(file_info.name, ".") == 0) ||
            (strcmp(file_info.name, "..") == 0)) {
            continue;
        }
        ret = combine_path(path_src, file_info.name, src_file_path, sizeof(src_file_path));
        if (ret == false) {
            all_copied = false;
            continue;
        }
        if ((file_info.attrib & _A_SUBDIR) != 0) {
            if (recursive == false) {
                continue;
            }
            ret = combine_path(path_des, file_info.name, des_file_path_temp, sizeof(des_file_path_temp));
            if (ret == false) {
                all_copied = false;
                continue;
            }
            ret = posix_copy_file_directory(
                src_file_path, des_file_path_temp, start_time, end_time, false, true, false, exclude_src_array, exclude_count);
        }
        else {
            ret = posix_copy_file_directory(
                src_file_path, des_file_path, start_time, end_time, false, true, false, exclude_src_array, exclude_count);
        }
        if (ret == false) {
            all_copied = false;
        }
    } while (_findnext(find_handle, &file_info) == 0);
    _findclose(find_handle);
#else   // LINUX
    memset(&file_stat, 0x00, sizeof(file_stat));
    find_handle = stat(find_path, &file_stat);
    if (find_handle == -1) {
        return false;
    }
    if (S_ISREG(file_stat.st_mode ) == true) {   // 一般文件
        if (path_des_is_file == false) {
            if (analyze_path(find_path, PART_FILE_NAME | PART_FILE_EXT, file_name, MAX_PATH) != true) {
                return false;
            }
            ret = combine_path(path_des, file_name, des_file_path, sizeof(des_file_path));
            if (ret == false) {
                return false;
            }
        } else {
            ret = trim_path(path_des, des_file_path, sizeof(des_file_path));
            if (ret == false) {
                return false;
            }
        }
        chmod(des_file_path, S_IRUSR | S_IWUSR);// 设置目标件夹 用户(所有者)读写执行权限
        if (start_time != NULL && end_time != NULL) {
            if ((DataConvertor::get_time_diff(file_stat.st_mtime, *start_time, TIME_UNIT_SEC) >= 0) &&
                (DataConvertor::get_time_diff(file_stat.st_mtime, *end_time, TIME_UNIT_SEC) <= 0)) {
                ret = posix_copy_file(find_path, des_file_path, false);
                if (ret == false) {
                    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "copy_file", ret, true, des_file_path);
                }
                return ret;
            } else {
                return true;
            }
        } else if (start_time != NULL) {
            if (DataConvertor::get_time_diff(file_stat.st_mtime, *start_time, TIME_UNIT_SEC) >= 0) {
                ret = posix_copy_file(find_path, des_file_path, false);
                if (ret == false) {
                    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "copy_file", ret, true, des_file_path);
                }
                return ret;
            } else {
                return true;
            }
        } else if (end_time != NULL) {
            if (DataConvertor::get_time_diff(file_stat.st_mtime, *end_time, TIME_UNIT_SEC) <= 0) {
                ret = posix_copy_file(find_path, des_file_path, false);
                if (ret == false) {
                    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "copy_file", ret, true, des_file_path);
                }
                return ret;
            }else {
                return true;
            }
        } else {
            ret = posix_copy_file(find_path, des_file_path, false);
            if (ret == false) {
                REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "copy_file", ret, true, des_file_path);
            }
            return ret;
        }
    }

    if ((path_des_is_file == false) && (create_path_in_des != false)) {
        ret = analyze_path(
            find_path, PART_FILE_NAME | PART_FILE_EXT, src_folder, sizeof(src_folder));
        if (ret == false) {
            return false;
        }
        ret = combine_path(des_file_path, src_folder, des_file_path, sizeof(des_file_path));
        if (ret == false) {
            return false;
        }
        mkdir(des_file_path, S_IRUSR| S_IWUSR | S_IXUSR);
        return posix_copy_file_directory(
            find_path, des_file_path, start_time, end_time, false, recursive, false, exclude_src_array, exclude_count);
    }

    if ((dir = opendir(find_path)) == NULL) {
        return false;
    }

    all_copied = true;
    while ((dir_ent = readdir(dir)) != NULL) {
        if ((strcmp(dir_ent->d_name, ".") == 0) ||
            (strcmp(dir_ent->d_name, "..") == 0)) {
            continue;
        }
        ret = combine_path(path_src, dir_ent->d_name, src_file_path, sizeof(src_file_path));
        if (ret == false) {
            all_copied = false;
            continue;
        }
        if (dir_ent->d_type == DT_DIR) {    // 目录
            if (recursive == false) {
                continue;
            }
            ret = combine_path(path_des, dir_ent->d_name, des_file_path_temp, sizeof(des_file_path_temp));
            if (ret == false) {
                all_copied = false;
                continue;
            }
            ret = posix_copy_file_directory(
                src_file_path, des_file_path_temp, start_time, end_time, false, true, false, exclude_src_array, exclude_count);
        }
        else {
            ret = posix_copy_file_directory(
                src_file_path, des_file_path, start_time, end_time, false, true, false, exclude_src_array, exclude_count);
        }
        if (ret == false) {
            all_copied = false;
        }
    }
    closedir(dir);
#endif

    return all_copied;
}

/**
 @功能：	删除文件夹或文件
 @参数：	file_dir_path：文件夹或文件路径											max_file_size：只删除大于该大小的文件（0：忽略）
 @			num_of_second：早于time_base时刻指定秒数的文件将被删除（NULL：忽略）	time_base：见num_of_second（NULL：当前时刻）
 @			exclude_array：删除时排除的文件列表（NULL：忽略）						exclude_count：列表中文件个数（0：忽略）
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileAccess::win_delete_file_directory(const char* file_dir_path,
    unsigned long long max_file_size/* = 0*/, unsigned long long num_of_second/* = 0*/, LPSYSTEMTIME time_base/* = NULL*/,
    char exclude_array[][MAX_PATH]/* = NULL*/, unsigned long exclude_count/* = 0*/)
{
    bool ret = false, del_all = true;
    WIN32_FIND_DATA file_info;
    HANDLE find_handle = NULL;
    char find_path[MAX_PATH] = { 0 };
    char folder_path[MAX_PATH] = { 0 };
    unsigned long last_error = 0;
    SYSTEMTIME sys_time;
    long long diff_time = 0;
    unsigned long long file_size = 0;
    bool need_del = true;
    unsigned long idx = 0;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0)) {
        return true;
    }

    ret = trim_path(file_dir_path, find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }

    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return true;
    }
    FindClose(find_handle);

    if ((exclude_array != NULL) && (exclude_count > 0)) {
        for (idx = 0; idx < exclude_count; idx++) {
            if (strcmp(find_path, exclude_array[idx]) == 0) {
                break;
            }
        }
        if (idx < exclude_count) {
            return true;
        }
    }

    if (time_base == NULL) {
        GetLocalTime(&sys_time);
    } else {
        sys_time = *time_base;
    }

    ret = true;
    if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        need_del = true;
        if (max_file_size > 0) {
            file_size = (((unsigned long long)file_info.nFileSizeHigh) << 32) + file_info.nFileSizeLow;
            need_del = (file_size > max_file_size);
        }
        if ((need_del) &&
            ((num_of_second > 0) || (time_base != NULL))) {
            diff_time = DataConvertor::get_time_diff(sys_time, file_info.ftLastWriteTime, TIME_UNIT_SEC);
            need_del = (diff_time > (long long)num_of_second);
        }
        if (need_del) {
            SetFileAttributes(find_path, FILE_ATTRIBUTE_ARCHIVE);
            ret = (DeleteFile(find_path) != FALSE);
            if (ret == false) {
                REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "DeleteFile", ret, true, find_path);
                if (GetLastError() == ERROR_SHARING_VIOLATION) {
                    ret = (MoveFileEx(find_path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) != FALSE);
                }
            }
        }
        return ret;
    } else {
        strncpy(folder_path, find_path, sizeof(folder_path) - 1);
    }

    ret = combine_path(find_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    del_all = true;
    do {
        if ((strcmp(file_info.cFileName, ".") == 0) ||
            (strcmp(file_info.cFileName, "..") == 0)) {
            continue;
        }
        ret = combine_path(file_dir_path, file_info.cFileName, find_path, sizeof(find_path));
        if (ret == false) {
            del_all = false;
            continue;
        }
        if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            ret = win_delete_file_directory(find_path, max_file_size, num_of_second, time_base);
        } else {
            need_del = true;
            if (max_file_size > 0) {
                file_size = (((unsigned long long)file_info.nFileSizeHigh) << 32) + file_info.nFileSizeLow;
                need_del = (file_size > max_file_size);
            }
            if ((need_del) &&
                ((num_of_second > 0) || (time_base != NULL))) {
                diff_time = DataConvertor::get_time_diff(sys_time, file_info.ftLastWriteTime, TIME_UNIT_SEC);
                need_del = (diff_time > (long long)num_of_second);
            }
            if (need_del) {
                SetFileAttributes(find_path, FILE_ATTRIBUTE_ARCHIVE);
                ret = (DeleteFile(find_path) != FALSE);
                if (ret == false) {
                    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "DeleteFile", ret, true, find_path);
                    if (GetLastError() == ERROR_SHARING_VIOLATION) {
                        ret = (MoveFileEx(find_path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) != FALSE);
                    }
                }
            }
        }
        if (ret == false) {
            del_all = false;
        }
    } while (FindNextFile(find_handle, &file_info) != FALSE);
    FindClose(find_handle);

    if (win_get_file_directory_size(folder_path) == 0) {
        ret = (RemoveDirectory(folder_path) != FALSE);
        if (ret == false) {
            last_error = GetLastError();
            if ((last_error == ERROR_SHARING_VIOLATION) ||
                (last_error == ERROR_DIR_NOT_EMPTY)) {
                ret = (MoveFileEx(folder_path, NULL, MOVEFILE_DELAY_UNTIL_REBOOT) != FALSE);
            }
        }
        if (ret == false) {
            del_all = false;
        }
    }

    return del_all;
}
#endif // WIN32

/**
 @功能：	删除文件夹或文件中的过时文件
 @参数：	file_dir_path：文件夹或文件路径							num_of_day：早于time_base时刻指定天数的文件将被删除（NULL：忽略）
 @			time_base：见num_of_day（NULL：当前时刻）				exclude_array：删除时排除的文件列表（NULL：忽略）
 @			exclude_count：列表中文件个数（0：忽略）
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileAccess::win_delete_older_file_directory(const char* file_dir_path, unsigned long num_of_day, LPSYSTEMTIME time_base/* = NULL*/,
    char exclude_array[][MAX_PATH]/* = NULL*/, unsigned long exclude_count/* = 0*/)
{
    return win_delete_file_directory(file_dir_path, 0, (unsigned long long)num_of_day * 24 * 60 * 60, time_base, exclude_array, exclude_count);
}
#endif // WIN32

/**
 @功能：	删除文件夹或文件
 @参数：	file_dir_path：文件夹或文件路径											max_file_size：只删除大于该大小的文件（0：忽略）
 @			num_of_second：早于time_base时刻指定秒数的文件将被删除（NULL：忽略）	time_base：见num_of_second（NULL：当前时刻）
 @			exclude_array：删除时排除的文件列表（NULL：忽略）						exclude_count：列表中文件个数（0：忽略）
 @返回：	true：成功		false：失败
 */
bool FileAccess::posix_delete_file_directory(const char* file_dir_path,
    unsigned long long max_file_size/* = 0*/, unsigned long long num_of_second/* = 0*/, tm* time_base/* = NULL*/,
    char exclude_array[][MAX_PATH]/* = NULL*/, unsigned long exclude_count/* = 0*/)
{
    bool ret = false, del_all = true;
    int ret2 = 0;
#ifdef WIN32
    struct _finddata_t file_info;
    long find_handle = NULL;
#else // LINUX
    struct stat file_stat;
    struct dirent *dir_ent;
    DIR *dir;
    int find_handle = 0;
#endif
    char find_path[MAX_PATH] = { 0 };
    char folder_path[MAX_PATH] = { 0 };
    time_t now_time = 0;
    tm* local_time = NULL;
    long long diff_time = 0;
    bool need_del = true;
    unsigned long idx = 0;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0)) {
        return true;
    }

    ret = trim_path(file_dir_path, find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }

#ifdef WIN32
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return true;
    }
    _findclose(find_handle);

    if ((exclude_array != NULL) && (exclude_count > 0)) {
        for (idx = 0; idx < exclude_count; idx++) {
            if (strcmp(find_path, exclude_array[idx]) == 0) {
                break;
            }
        }
        if (idx < exclude_count) {
            return true;
        }
    }

    if (time_base == NULL) {
        now_time = time(NULL);
        local_time = localtime(&now_time);
    } else {
        local_time = time_base;
    }

    ret = true;
    if ((file_info.attrib & _A_SUBDIR) == 0) {
        need_del = true;
        if (max_file_size > 0) {
            need_del = (file_info.size > max_file_size);
        }
        if ((need_del) &&
            ((num_of_second > 0) || (time_base != NULL))) {
            diff_time = DataConvertor::get_time_diff(*local_time, file_info.time_write, TIME_UNIT_SEC);
            need_del = (diff_time > (long long)num_of_second);
        }
        if (need_del) {
            _chmod(find_path, _S_IREAD | _S_IWRITE);
            ret = ((ret2 = remove(find_path)) == 0);
            if (ret == false) {
                REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "remove", ret2, true, find_path);
            }
        }
        return ret;
    } else {
        strncpy(folder_path, find_path, sizeof(folder_path) - 1);
    }

    ret = combine_path(find_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return false;
    }

    del_all = true;
    do {
        if ((strcmp(file_info.name, ".") == 0) ||
            (strcmp(file_info.name, "..") == 0)) {
            continue;
        }
        ret = combine_path(file_dir_path, file_info.name, find_path, sizeof(find_path));
        if (ret == false) {
            del_all = false;
            continue;
        }
        if ((file_info.attrib & _A_SUBDIR) != 0) {
            ret = posix_delete_file_directory(find_path, max_file_size, num_of_second, time_base);
        } else {
            need_del = true;
            if (max_file_size > 0) {
                need_del = (file_info.size > max_file_size);
            }
            if ((need_del) &&
                ((num_of_second > 0) || (time_base != NULL))) {
                diff_time = DataConvertor::get_time_diff(*local_time, file_info.time_write, TIME_UNIT_SEC);
                need_del = (diff_time > (long long)num_of_second);
            }
            if (need_del) {
                _chmod(find_path, _S_IREAD | _S_IWRITE);
                ret = ((ret2 = remove(find_path)) == 0);
                if (ret == false) {
                    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "remove", ret2, true, find_path);
                }
            }
        }
        if (ret == false) {
            del_all = false;
        }
    } while (_findnext(find_handle, &file_info) == 0);
    _findclose(find_handle);

    if (posix_get_file_directory_size(folder_path) == 0) {
        ret = (_rmdir(folder_path) != FALSE);
        if (ret == false) {
            del_all = false;
        }
    }
#else // LINUX
    memset(&file_stat, 0x00, sizeof(file_stat));
    find_handle = stat(find_path, &file_stat);
    if (find_handle == -1) {
        return true;
    }

    if ((exclude_array != NULL) && (exclude_count > 0)) {
        for (idx = 0; idx < exclude_count; idx++) {
            if (strcmp(find_path, exclude_array[idx]) == 0) {
                break;
            }
        }
        if (idx < exclude_count) {
            return true;
        }
    }

    if (time_base == NULL) {
        now_time = time(NULL);
        local_time = localtime(&now_time);
    } else {
        local_time = time_base;
    }

    ret = true;
    if (S_ISREG(file_stat.st_mode) == true) {   // 一般文件
        need_del = true;
        if (max_file_size > 0) {
            need_del = ((unsigned long long)file_stat.st_size > max_file_size);
        }
        if ((need_del) &&
            ((num_of_second > 0) || (time_base != NULL))) {
            diff_time = DataConvertor::get_time_diff(*local_time, file_stat.st_mtime, TIME_UNIT_SEC);
            need_del = (diff_time > (long long)num_of_second);
        }
        if (need_del) {
            chmod(find_path, S_IRUSR | S_IWUSR | S_IXUSR);
            ret = ((ret2 = remove(find_path)) == 0);
            if (ret == false) {
                REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "remove", ret2, true, find_path);
            }
        }
        return ret;
    } else {
        strncpy(folder_path, find_path, sizeof(folder_path) - 1);
    }

    if ((dir = opendir(find_path)) == NULL) {
        return false;
    }
    del_all = true;
    while ((dir_ent = readdir(dir)) != NULL) {
        if ((strcmp(dir_ent->d_name, ".") == 0) ||
            (strcmp(dir_ent->d_name, "..") == 0)) {
            continue;
        }
        ret = combine_path(file_dir_path, dir_ent->d_name, find_path, sizeof(find_path));
        if (ret == false) {
            del_all = false;
            continue;
        }
        if (dir_ent->d_type == DT_DIR) {    // 目录
            ret = posix_delete_file_directory(find_path, max_file_size, num_of_second, time_base);
        } else {
            need_del = true;
            memset(&file_stat, 0x00, sizeof(file_stat));
            find_handle = stat(find_path, &file_stat);
            if (find_handle == -1) {
                return false;
            }
            if (max_file_size > 0) {
                need_del = ((unsigned long long)file_stat.st_size > max_file_size);
            }
            if ((need_del) &&
                ((num_of_second > 0) || (time_base != NULL))) {
                diff_time = DataConvertor::get_time_diff(*local_time, file_stat.st_mtime, TIME_UNIT_SEC);
                need_del = (diff_time > (long long)num_of_second);
            }
            if (need_del) {
                chmod(find_path, S_IRUSR | S_IWUSR | S_IXUSR);
                ret = ((ret2 = remove(find_path)) == 0);
                if (ret == false) {
                    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "remove", ret2, true, find_path);
                }
            }
        }
        if (ret == false) {
            del_all = false;
        }
    }
    closedir(dir);

    if (posix_get_file_directory_size(folder_path) == 0) {
        //ret = (rmdir(folder_path) != 0);
        ret = (rmdir(folder_path) == 0);    // rmdir()成功返回0,失败-1;返回成功设置ret为true
        if (ret == false) {
            del_all = false;
        }
    }
#endif

    return del_all;
}

/**
 @功能：	删除文件夹或文件中的过时文件
 @参数：	file_dir_path：文件夹或文件路径							num_of_day：早于time_base时刻指定天数的文件将被删除（NULL：忽略）
 @			time_base：见num_of_day（NULL：当前时刻）				exclude_array：删除时排除的文件列表（NULL：忽略）
 @			exclude_count：列表中文件个数（0：忽略）
 @返回：	true：成功		false：失败
 */
bool FileAccess::posix_delete_older_file_directory(const char* file_dir_path, unsigned long num_of_day, tm* time_base/* = NULL*/,
    char exclude_array[][MAX_PATH]/* = NULL*/, unsigned long exclude_count/* = 0*/)
{
    return posix_delete_file_directory(file_dir_path, 0, (unsigned long long)num_of_day * 24 * 60 * 60, time_base, exclude_array, exclude_count);
}

/**
 @功能：	删除文件夹或文件中的大文件
 @参数：	file_dir_path：文件夹或文件路径							max_file_size：只删除大于该大小的文件（0：忽略）
 @			exclude_array：删除时排除的文件列表（NULL：忽略）		exclude_count：列表中文件个数（0：忽略）
 @返回：	true：成功		false：失败
 */
bool FileAccess::delete_larger_file_directory(const char* file_dir_path, unsigned long long max_file_size,
    char exclude_array[][MAX_PATH]/* = NULL*/, unsigned long exclude_count/* = 0*/)
{
#ifdef WIN32
    return win_delete_file_directory(file_dir_path, max_file_size, 0, NULL, exclude_array, exclude_count);
#else
    return posix_delete_file_directory(file_dir_path, max_file_size, 0, NULL, exclude_array, exclude_count);
#endif // WIN32
}

/**
 @功能：	删除文件夹中超过指定个数的旧文件
 @参数：	file_dir_path：文件夹或文件路径							max_file_count：指定个数（0：全删除）
 @			exclude_array：删除时排除的文件列表（NULL：忽略）		exclude_count：列表中文件个数（0：忽略）
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileAccess::win_delete_more_file_in_directory(const char* file_dir_path, unsigned long long max_file_count,
    char exclude_array[][MAX_PATH]/* = NULL*/, unsigned long exclude_count/* = 0*/)
{
    bool del_all = true;
    bool ret = false;
    WIN32_FIND_DATA file_info;
    HANDLE find_handle = NULL;
    char find_path[MAX_PATH] = { 0 };
    struct _last_file_write_time write_time;
    map<struct _last_file_write_time, string> file_info_map;
    map<struct _last_file_write_time, string>::iterator it;
    unsigned long long del_count = 0;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0)) {
        return true;
    }

    if (max_file_count == 0) {
        return win_delete_file_directory(file_dir_path);
    }

    ret = trim_path(file_dir_path, find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }

    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return true;
    }
    FindClose(find_handle);
    if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        return true;
    }

    ret = FileDir::combine_path(find_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return false;
    }

    do {
        if (((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) ||
            (strcmp(file_info.cFileName, ".") == 0) ||
            (strcmp(file_info.cFileName, "..") == 0)) {
            continue;
        }
        ret = combine_path(file_dir_path, file_info.cFileName, find_path, sizeof(find_path));
        if (ret == false) {
            FindClose(find_handle);
            return false;
        }
        write_time.file_time = file_info.ftLastWriteTime;
        file_info_map.insert(map<struct _last_file_write_time, string>::value_type(write_time, find_path));
    } while (FindNextFile(find_handle, &file_info) != FALSE);
    FindClose(find_handle);

    if (file_info_map.size() <= max_file_count) {
        return true;
    }
    del_count = file_info_map.size() - max_file_count;
    it = file_info_map.begin();
    del_all = true;
    do {
        if (win_delete_file_directory(it->second.c_str(), 0, 0, NULL, exclude_array, exclude_count) == false) {
            del_all = false;
        }
        del_count--;
        it++;
    } while ((del_count > 0) && (it != file_info_map.end()));

    return del_all;
}
#endif // WIN32

/**
 @功能：	删除文件夹中超过指定个数的旧文件
 @参数：	file_dir_path：文件夹或文件路径							max_file_count：指定个数（0：全删除）
 @			exclude_array：删除时排除的文件列表（NULL：忽略）		exclude_count：列表中文件个数（0：忽略）
 @返回：	true：成功		false：失败
 */
bool FileAccess::posix_delete_more_file_in_directory(const char* file_dir_path, unsigned long long max_file_count,
    char exclude_array[][MAX_PATH]/* = NULL*/, unsigned long exclude_count/* = 0*/)
{
    bool del_all = true;
    bool ret = false;
#ifdef WIN32
    struct _finddata_t file_info;
    long find_handle = NULL;
#else // LINUX
    struct stat file_stat;
    struct dirent *dir_ent;
    DIR *dir;
    int find_handle = 0;
#endif
    char find_path[MAX_PATH] = { 0 };
    map<time_t, string> file_info_map;
    map<time_t, string>::iterator it;
    unsigned long long del_count = 0;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0)) {
        return true;
    }

    if (max_file_count == 0) {
        return posix_delete_file_directory(file_dir_path);
    }

    ret = trim_path(file_dir_path, find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }

#ifdef WIN32
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return true;
    }
    _findclose(find_handle);
    if ((file_info.attrib & _A_SUBDIR) == 0) {
        return true;
    }

    ret = FileDir::combine_path(find_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        return false;
    }
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return false;
    }

    do {
        if (((file_info.attrib & _A_SUBDIR) != 0) ||
            (strcmp(file_info.name, ".") == 0) ||
            (strcmp(file_info.name, "..") == 0)) {
            continue;
        }
        ret = combine_path(file_dir_path, file_info.name, find_path, sizeof(find_path));
        if (ret == false) {
            _findclose(find_handle);
            return false;
        }
        file_info_map.insert(map<time_t, string>::value_type(file_info.time_write, find_path));
    } while (_findnext(find_handle, &file_info) != FALSE);
    _findclose(find_handle);

    if (file_info_map.size() <= max_file_count) {
        return true;
    }
    del_count = file_info_map.size() - max_file_count;
    it = file_info_map.begin();
    del_all = true;
    do {
        if (posix_delete_file_directory(it->second.c_str(), 0, 0, NULL, exclude_array, exclude_count) == false) {
            del_all = false;
        }
        del_count--;
        it++;
    } while ((del_count > 0) && (it != file_info_map.end()));
#else
    memset(&file_stat, 0x00, sizeof(file_stat));
    find_handle = stat(find_path, &file_stat);
    if (find_handle == -1) {
        return true;
    }
    if (S_ISDIR(file_stat.st_mode) == false) {   // 非目录
        return true;
    }

    if ((dir = opendir(find_path)) == NULL) {
        return false;
    }
    while ((dir_ent = readdir(dir)) != NULL) {
        if ((strcmp(dir_ent->d_name, ".") == 0) ||
            (strcmp(dir_ent->d_name, "..") == 0)) {
            continue;
        }
        ret = combine_path(file_dir_path, dir_ent->d_name, find_path, sizeof(find_path));
        if (ret == false) {
            closedir(dir);
            return false;
        }
        memset(&file_stat, 0x00, sizeof(file_stat));
        find_handle = stat(find_path, &file_stat);
        if (find_handle == -1) {
            closedir(dir);
            return false;
        }
        file_info_map.insert(map<time_t, string>::value_type(file_stat.st_mtime, find_path));
    }
    closedir(dir);

    if (file_info_map.size() <= max_file_count) {
        return true;
    }
    del_count = file_info_map.size() - max_file_count;
    it = file_info_map.begin();
    del_all = true;
    do {
        if (posix_delete_file_directory(it->second.c_str(), 0, 0, NULL, exclude_array, exclude_count) == false) {
            del_all = false;
        }
        del_count--;
        it++;
    } while ((del_count > 0) && (it != file_info_map.end()));

#endif

    return del_all;
}

#ifdef WIN32
/**
 @功能：	用7z压缩或解压缩
 @参数：	zip_file_path：压缩文件路径			path：未压缩/解压缩路径
 @			is_file_path：path是否是文件		password：压缩/解压密码（NULL：忽略）
 @			recursive：是否包含子文件夹			timeout：等待超时时间（ms）
 @返回：	true：成功		false：失败
 */
bool FileAccess::win_zip_compress(
    int mode, const char* zip_file_path, const char* path,
    bool is_file_path/* = false*/, const char* password/* = NULL*/, bool recursive/* = true*/, unsigned long timeout/* = INFINITE*/)
{
    unsigned long wait = WAIT_OBJECT_0;
    bool ret = false;
    char cmd_line[MAX_PATH + CONST_VALUE_260] = { 0 };
    char local_path[MAX_PATH] = { 0 };
    STARTUPINFO si = { sizeof(si) };
    PROCESS_INFORMATION pi = { 0 };

    if ((zip_file_path == NULL) || (strlen(zip_file_path) == 0) ||
        (path == NULL) || (strlen(path) == 0)) {
        return false;
    }

    create_directory_by_path(zip_file_path, true);

    strncpy(cmd_line, "7z.exe ", sizeof(cmd_line) - 1);
    if (mode == ZIP_DECOMPRESS) {
        strcat(cmd_line, "x ");
    } else {
        strcat(cmd_line, "a ");
    }
    strcat(cmd_line, zip_file_path);

    if (mode == ZIP_DECOMPRESS) {
        strcat(cmd_line, " -o");
        strcat(cmd_line, path);
        strcat(cmd_line, " -tzip -y");
    } else {
        if (is_file_path == false) {
            ret = combine_path(path, "*", local_path, sizeof(local_path));
            if (ret == false) {
                return false;
            }
        } else {
            strncpy(local_path, path, sizeof(local_path) - 1);
        }
        strcat(cmd_line, " ");
        strcat(cmd_line, local_path);
        strcat(cmd_line, " -tzip");
    }
    if (recursive) {
        strcat(cmd_line, " -r");
    }
    if ((password != NULL) && (strlen(password) > 0)) {
        strcat(cmd_line, " -p");
        strcat(cmd_line, password);
    }

    memset(&pi, 0x00, sizeof(pi));
    memset(&si, 0x00, sizeof(si));
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESHOWWINDOW;
    si.wShowWindow = SW_HIDE;

    ret = (CreateProcess(
        NULL, cmd_line, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi) != FALSE);
    if (ret == false) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "CreateProcess", ret, true, cmd_line);
        if (pi.hProcess != NULL) {
            TerminateProcess(pi.hProcess, 0);
        }
        return false;
    }
    if (timeout != 0) {
        wait = WaitForSingleObject(pi.hProcess, timeout);
        if (wait != WAIT_OBJECT_0) {
            REPORT_APP_EVENT_API(EVENTLOG_ERROR_TYPE, get_product_name(), "WaitForSingleObject", wait, true);
        }
    }

    return (wait == WAIT_OBJECT_0);
}
#else   // LINUX
/**
 @功能：	用zip/unzip压缩或解压缩
 @参数：	mode: 压缩(0)/解压(1)标识
 @      zip_file_path：生成压缩文件路径			path：未压缩/解压缩路径
 @      is_file_path：path是否是文件		password：压缩/解压密码（NULL：忽略）
 @      recursive：是否包含子文件夹			timeout：等待超时时间（ms）
 @返回：	true：成功		false：失败
 */
bool FileAccess::linux_zip_compress(
    int mode, const char* zip_file_path, const char* path,
    bool is_file_path/* = false*/, const char* password/* = NULL*/, bool recursive/* = true*/, unsigned long timeout/* = INFINITE*/)
{
    Q_UNUSED(is_file_path);
    Q_UNUSED(recursive);
    Q_UNUSED(timeout);

    int ret = 0;
    char cmd_line[3000] = { 0 };
    char szUpperPath[MAX_PATH] = { 0x00 };      // 压缩路径的上一级目录
    char szSourceFile[MAX_PATH] = { 0x00 };     // 压缩路径包含的压缩文件/目录名

    // Check 入参
    if ((zip_file_path == NULL) || (strlen(zip_file_path) == 0) ||
        (path == NULL) || (strlen(path) == 0)) {
        return false;
    }

    // 压缩前处理
    if (mode == 0)
    {
        // 通过全路径创建文件夹(压缩文件路径)
        create_directory_by_path(zip_file_path, true);

        // 获取上一级目录和压缩文件/目录名
        if (FileDir::reduce_path(path, 1, szUpperPath, sizeof(szUpperPath)) == true)
        {
            memcpy(szSourceFile, path + strlen(szUpperPath) + 1, strlen(path) - strlen(szUpperPath) - 1);

            // 先cd 到 压缩路径的上一级目录
            strncpy(cmd_line, "cd ", sizeof(cmd_line) - 1);
            strcat(cmd_line, szUpperPath);
            strcat(cmd_line, " && ");

        } else
        {
            memcpy(szSourceFile, path, strlen(path));   // 直接使用压缩路径
        }
    }

    // 确认压缩/解压 命令+参数
    if (mode == 1)
    {
        //strncpy(cmd_line, "unzip -qo ", sizeof(cmd_line) - 1);  // 解压缩(q不显示执行过程/o不询问直接覆盖)
        strcat(cmd_line, "unzip -qo ");
    } else
    {
        //strncpy(cmd_line, "zip -rq ", sizeof(cmd_line) - 1);    // 压缩(r递归压缩/q不显示执行过程)
        strcat(cmd_line, "zip -rq ");
    }

    // -P 写入密码
    if ((password != NULL) && (strlen(password) > 0))
    {
        strcat(cmd_line, " -P ");
        strcat(cmd_line, password);
        strcat(cmd_line, " ");
    }

    // 连接 压缩目的文件/待解压文件
    strcat(cmd_line, zip_file_path);

    // 连接 要压缩的文件目录/解压到目录
    if (mode == 1) {
        strcat(cmd_line, " -d ");   // 解压缩
        strcat(cmd_line, path);     // 解压路径
    } else {
        strcat(cmd_line, " ");      // 压缩
        strcat(cmd_line, szSourceFile);// 待压缩目录
    }

    // system执行命令
    ret = system(cmd_line);
    if (ret == -1) {    // system() error
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "system", ret, true, cmd_line);
        return false;
    } else {
        if (WIFEXITED(ret)) {
            if (WEXITSTATUS(ret) != 0) {    // system() - run shell script fail
                REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(),
                                         "system()-run shell script fail", ret, true, cmd_line);
                return false;
            }
        } else {    // system() - exit status fail
            REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(),
                                     "system()-exit status fail", ret, true, cmd_line);
            return false;
        }
    }

    return true;
}
#endif // WIN32

/**
 @功能：	验证文件/目录是否存在
 @参数：	zip_file_path：待验证文件/目录
 @返回：	true：存在		false：不存在
 */
bool FileAccess::is_file_directory_exist(const char* file_dir_path)
{
    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0))
    {
        return false;
    }

#ifdef WIN32
    return (_access(file_dir_path, 0) == 0);
#else // LINUX
    return (access(file_dir_path, 0/*F_OK*/) == 0);
#endif
}

/**
 @功能：	取得指定文件夹下文件个数
 @参数：	dir_path：文件夹路径			recursive：是否包含子文件夹
 @返回：	文件个数（-1：失败）
 */
#ifdef WIN32
unsigned long FileAccess::win_get_file_count(const char* dir_path, bool recursive/* = true*/)
{
    unsigned long count = 0;
    WIN32_FIND_DATA file_info;
    HANDLE find_handle = NULL;
    bool ret = false;
    char find_path[MAX_PATH] = { 0 };

    if ((dir_path == NULL) || (strlen(dir_path) == 0)) {
        return 0;
    }

    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = FindFirstFile(dir_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        FindClose(find_handle);
        return 1;
    }
    FindClose(find_handle);

    ret = combine_path(dir_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        return (unsigned long)-1;
    }
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    count = 0;
    do {
        if ((strcmp(file_info.cFileName, ".") == 0) ||
            (strcmp(file_info.cFileName, "..") == 0)) {
            continue;
        }
        if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            if (recursive == false) {
                continue;
            }
            ret = combine_path(dir_path, file_info.cFileName, find_path, sizeof(find_path));
            if (ret == false) {
                FindClose(find_handle);
                return (unsigned long)-1;
            }
            count += win_get_file_count(find_path, recursive);
        } else {
            count++;
        }
    } while (FindNextFile(find_handle, &file_info) != FALSE);

    FindClose(find_handle);

    return count;
}
#endif // WIN32

/**
 @功能：	取得指定文件夹下文件个数
 @参数：	dir_path：文件夹路径			recursive：是否包含子文件夹
 @返回：	文件个数（-1：失败）
 */
unsigned long FileAccess::posix_get_file_count(const char* dir_path, bool recursive/* = true*/)
{
    unsigned long count = 0;
#ifdef WIN32
    struct _finddata_t file_info;
    long find_handle = NULL;
#else // LINUX
    struct stat file_stat;
    struct dirent *dir_ent;
    DIR *dir;
    int find_handle = 0;
#endif
    bool ret = false;
    char find_path[MAX_PATH] = { 0 };

    if ((dir_path == NULL) || (strlen(dir_path) == 0)) {
        return 0;
    }

    sprintf(find_path, "%s", dir_path);

#ifdef WIN32
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = _findfirst(dir_path, &file_info);
    if (find_handle <= 0) {
        return 0;
    }
    if ((file_info.attrib & _A_SUBDIR) == 0) {
        _findclose(find_handle);
        return 1;
    }
    _findclose(find_handle);

    ret = combine_path(dir_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        return (unsigned long)-1;
    }
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return 0;
    }

    count = 0;
    do {
        if ((strcmp(file_info.name, ".") == 0) ||
            (strcmp(file_info.name, "..") == 0)) {
            continue;
        }
        if ((file_info.attrib & _A_SUBDIR) != 0) {
            if (recursive == false) {
                continue;
            }
            ret = combine_path(dir_path, file_info.name, find_path, sizeof(find_path));
            if (ret == false) {
                _findclose(find_handle);
                return (unsigned long)-1;
            }
            count += posix_get_file_count(find_path, recursive);
        } else {
            count++;
        }
    } while (_findnext(find_handle, &file_info) == 0);

    _findclose(find_handle);
#else // LINUX
    memset(&file_stat, 0x00, sizeof(file_stat));
    find_handle = stat(find_path, &file_stat);
    if (find_handle == -1) {
        return 0;
    }
    if (S_ISDIR(file_stat.st_mode) == false) {   // 非目录
        return 1;
    }

    if ((dir = opendir(find_path)) == NULL) {
        return 0;
    }
    count = 0;
    while ((dir_ent = readdir(dir)) != NULL) {
        if ((strcmp(dir_ent->d_name, ".") == 0) ||
            (strcmp(dir_ent->d_name, "..") == 0)) {
            continue;
        }
        ret = combine_path(dir_path, dir_ent->d_name, find_path, sizeof(find_path));
        if (ret == false) {
            closedir(dir);
            return 0;
        }
        memset(&file_stat, 0x00, sizeof(file_stat));
        find_handle = stat(find_path, &file_stat);
        if (find_handle == -1) {
            closedir(dir);
            return 0;
        }
        if (S_ISDIR(file_stat.st_mode) == true) {   // 目录
            if (recursive == false) {
                continue;
            }
            count += posix_get_file_count(find_path, recursive);
        } else {
            count ++;
        }
    }
    closedir(dir);
#endif

    return count;
}

/**
 @功能：	取得指定文件夹下的文件修改时间
 @参数：	file_dir_path：文件夹路径							recursive：是否包含子文件夹
 @			write_time_array：存放修改时间的数组（NULL：忽略）	max_count：存放修改时间的最大个数（0：忽略）
 @			need_sort：是否对修改时间排序
 @返回：	修改时间个数（-1：失败）
 */
#ifdef WIN32
unsigned long FileAccess::win_get_write_time_of_files(
    const char* file_dir_path, bool recursive, LPSYSTEMTIME write_time_array, unsigned long max_count, bool need_sort)
{
    unsigned long count = 0;
    WIN32_FIND_DATA file_info;
    HANDLE find_handle = NULL;
    bool ret = false;
    char find_path[MAX_PATH] = { 0 };
    FILETIME file_local_time;
    multiset<struct _last_file_write_time> write_time_set;
    multiset<struct _last_file_write_time>::iterator it;
    struct _last_file_write_time file_write_time;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0)) {
        return 0;
    }
    if ((write_time_array == NULL) || (max_count == 0)) {
        return get_file_count(file_dir_path, recursive);
    }

    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = FindFirstFile(file_dir_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }
    if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0) {
        ret = (FileTimeToLocalFileTime(&file_info.ftLastWriteTime, &file_local_time) != FALSE);
        if (ret == false) {
            FindClose(find_handle);
            return (unsigned long)-1;
        }
        ret = (FileTimeToSystemTime(&file_local_time, &write_time_array[0]) != FALSE);
        if (ret == false) {
            FindClose(find_handle);
            return (unsigned long)-1;
        }
        FindClose(find_handle);
        return 1;
    }
    FindClose(find_handle);

    ret = combine_path(file_dir_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        FindClose(find_handle);
        return (unsigned long)-1;
    }
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = FindFirstFile(find_path, &file_info);
    if (find_handle == INVALID_HANDLE_VALUE) {
        return 0;
    }

    count = 0;
    do {
        if (count >= max_count) {
            FindClose(find_handle);
            return count;
        }
        if ((strcmp(file_info.cFileName, ".") == 0) ||
            (strcmp(file_info.cFileName, "..") == 0)) {
            continue;
        }
        ret = combine_path(file_dir_path, file_info.cFileName, find_path, sizeof(find_path));
        if (ret == false) {
            FindClose(find_handle);
            return (unsigned long)-1;
        }
        if ((file_info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
            if (recursive == false) {
                continue;
            }
            count += win_get_write_time_of_files(find_path, recursive, write_time_array + count, max_count - count, false);
        } else {
            ret = (FileTimeToLocalFileTime(&file_info.ftLastWriteTime, &file_local_time) != FALSE);
            if (ret == false) {
                FindClose(find_handle);
                return (unsigned long)-1;
            }
            ret = (FileTimeToSystemTime(&file_local_time, &write_time_array[count]) != FALSE);
            if (ret == false) {
                FindClose(find_handle);
                return (unsigned long)-1;
            }
            count++;
            if (need_sort) {
                file_write_time.file_time = file_info.ftLastWriteTime;
                write_time_set.insert(multiset<struct _last_file_write_time>::value_type(file_write_time));
            }
        }
    } while ((count < max_count) && (FindNextFile(find_handle, &file_info) != FALSE));

    FindClose(find_handle);

    if (!write_time_set.empty()) {
        count = 0;
        memset(write_time_array, 0x00, sizeof(SYSTEMTIME) * max_count);
        for (it = write_time_set.begin(); it != write_time_set.end(); it++) {
            ret = (FileTimeToLocalFileTime(&it->file_time, &file_local_time) != FALSE);
            if (ret == false) {
                return (unsigned long)-1;
            }
            ret = (FileTimeToSystemTime(&file_local_time, &write_time_array[count++]) != FALSE);
            if (ret == false) {
                return (unsigned long)-1;
            }
        }
    }

    return count;
}
#endif // WIN32

/**
 @功能：	取得指定文件夹下的文件修改时间
 @参数：	file_dir_path：文件夹路径							recursive：是否包含子文件夹
 @			write_time_array：存放修改时间的数组（NULL：忽略）	max_count：存放修改时间的最大个数（0：忽略）
 @			need_sort：是否对修改时间排序
 @返回：	修改时间个数（-1：失败）
 */
unsigned long FileAccess::posix_get_write_time_of_files(
    const char* file_dir_path, bool recursive, tm* write_time_array, unsigned long max_count, bool need_sort)
{
    unsigned long count = 0;
#ifdef WIN32
    struct _finddata_t file_info;
    long find_handle = NULL;
#else // LINUX
    struct stat file_stat;
    struct dirent *dir_ent;
    DIR *dir;
    int find_handle = 0;
#endif
    bool ret = false;
    char find_path[MAX_PATH] = { 0 };
    tm* file_local_time;
    multiset<time_t> write_time_set;
    multiset<time_t>::iterator it;

    if ((file_dir_path == NULL) || (strlen(file_dir_path) == 0)) {
        return 0;
    }
    if ((write_time_array == NULL) || (max_count == 0)) {
        return get_file_count(file_dir_path, recursive);
    }

#ifdef WIN32
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = _findfirst(file_dir_path, &file_info);
    if (find_handle <= 0) {
        return 0;
    }
    if ((file_info.attrib & _A_SUBDIR) == 0) {
        file_local_time = localtime(&file_info.time_write);
        memcpy(&write_time_array[0], file_local_time, sizeof(tm));
        return 1;
    }
    _findclose(find_handle);

    ret = combine_path(file_dir_path, "*", find_path, sizeof(find_path));
    if (ret == false) {
        _findclose(find_handle);
        return (unsigned long)-1;
    }
    memset(&file_info, 0x00, sizeof(file_info));
    find_handle = _findfirst(find_path, &file_info);
    if (find_handle <= 0) {
        return 0;
    }

    count = 0;
    do {
        if (count >= max_count) {
            _findclose(find_handle);
            return count;
        }
        if ((strcmp(file_info.name, ".") == 0) ||
            (strcmp(file_info.name, "..") == 0)) {
            continue;
        }
        ret = combine_path(file_dir_path, file_info.name, find_path, sizeof(find_path));
        if (ret == false) {
            _findclose(find_handle);
            return (unsigned long)-1;
        }
        if ((file_info.attrib & _A_SUBDIR) != 0) {
            if (recursive == false) {
                continue;
            }
            count += posix_get_write_time_of_files(find_path, recursive, write_time_array + count, max_count - count, false);
        } else {
            file_local_time = localtime(&file_info.time_write);
            memcpy(&write_time_array[count], file_local_time, sizeof(tm));
            count++;
            if (need_sort) {
                write_time_set.insert(multiset<time_t>::value_type(file_info.time_write));
            }
        }
    } while ((count < max_count) && (_findnext(find_handle, &file_info) == 0));

    _findclose(find_handle);

    if (!write_time_set.empty()) {
        count = 0;
        memset(write_time_array, 0x00, sizeof(tm) * max_count);
        for (it = write_time_set.begin(); it != write_time_set.end(); it++) {
            file_local_time = localtime(&(*it));
            memcpy(&write_time_array[count++], file_local_time, sizeof(tm));
        }
    }
#else // LINUX
    memset(&file_stat, 0x00, sizeof(file_stat));
    find_handle = stat(find_path, &file_stat);
    if (find_handle == -1) {
        return 0;
    }
    if (S_ISDIR(file_stat.st_mode) == false) {   // 非目录
        file_local_time = localtime(&file_stat.st_mtime);
        memcpy(&write_time_array[0], file_local_time, sizeof(tm));
        return 1;
    }

    if ((dir = opendir(find_path)) == NULL) {
        return 0;
    }
    count = 0;
    while ((dir_ent = readdir(dir)) != NULL) {
        if (count >= max_count) {
            closedir(dir);
            return count;
        }
        if ((strcmp(dir_ent->d_name, ".") == 0) ||
            (strcmp(dir_ent->d_name, "..") == 0)) {
            continue;
        }
        ret = combine_path(file_dir_path, dir_ent->d_name, find_path, sizeof(find_path));
        if (ret == false) {
            closedir(dir);
            return (unsigned long)-1;
        }
        memset(&file_stat, 0x00, sizeof(file_stat));
        find_handle = stat(find_path, &file_stat);
        if (find_handle == -1) {
            closedir(dir);
            return 0;
        }
        if (S_ISDIR(file_stat.st_mode) == true) {   // 目录
            if (recursive == false) {
                continue;
            }
            count += posix_get_write_time_of_files(find_path, recursive, write_time_array + count, max_count - count, false);
        } else {
            file_local_time = localtime(&file_stat.st_mtime);
            memcpy(&write_time_array[count], file_local_time, sizeof(tm));
            count++;
            if (need_sort) {
                write_time_set.insert(multiset<time_t>::value_type(file_stat.st_mtime));
            }
        }
    }
    closedir(dir);

    if (!write_time_set.empty()) {
        count = 0;
        memset(write_time_array, 0x00, sizeof(tm) * max_count);
        for (it = write_time_set.begin(); it != write_time_set.end(); it++) {
            file_local_time = localtime(&(*it));
            memcpy(&write_time_array[count++], file_local_time, sizeof(tm));
        }
    }
#endif

    return count;
}

/**
 @功能：	启动进程
 @参数：	expression：进程启动表达式，如："chrome -kiosk http://47.94.237.188:5001/"
 @返回：	false：失败    true：成功
 */
bool FileAccess::create_process(const char* expression)
{
    if ((expression == NULL) || (strlen(expression) == 0)) {
        return false;
    }

#ifdef WIN32
    BOOL ret = FALSE;
    STARTUPINFO si;
    PROCESS_INFORMATION pi;

    memset(&si, 0x00, sizeof(si));
    si.cb = sizeof(si);
    memset(&pi, 0x00, sizeof(pi));

    ret = CreateProcess(NULL, (LPSTR)(expression), NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi);

    return (ret != FALSE);

#else
    char param_array[9][CONST_VALUE_260];
    char *arg_list[9];
    unsigned long count = 0;
    unsigned long idx = 0;
    int ret = 0;
    pid_t fpid = -1;

    memset(param_array, 0x00, sizeof(param_array));
    count = DataConvertor::split_string(expression, ' ', param_array, 9);
    if ((count == 0) || (count == (unsigned long )-1)) {
        return false;
    }
    memset(arg_list, 0x00, sizeof(arg_list));
    for (idx = 0; idx < count; idx++) {
        arg_list[idx] = param_array[idx];
    }

    if ((fpid = fork()) < 0) {
        return false;
    }
    if (fpid == 0) {
        ret = execlp(arg_list[0], arg_list[0], arg_list[1], arg_list[2],
                arg_list[3], arg_list[4], arg_list[5],
                arg_list[6], arg_list[7], arg_list[8], (char*)NULL);
        if (ret < 0) {
            exit(errno);
        }
    }
#endif

    return true;
}
