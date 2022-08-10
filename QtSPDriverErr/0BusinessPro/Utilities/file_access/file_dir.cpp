// file_dir.cpp : 定义静态库的函数。
//

#include "framework.h"
#include "file_access.h"


/**
 @功能：	删除路径首尾的'\'以及空白
 @参数：	file_path：路径
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
bool FileDir::trim_path(const char* file_path, char* result, unsigned int res_buf_size)
{
    string local_result = "";
    char local_path[MAX_PATH] = { 0 };
    int len = 0;

    if ((result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    if ((file_path == NULL) || (strlen(file_path) == 0) ||
        (DataConvertor::trim_string(file_path, local_path, sizeof(local_path)) == 0)) {
        result[0] = '\0';
        return true;
    }
#ifndef WIN32
    if (strcmp(local_path, PATH_SPT_STR) == 0) {
        strncpy(result, local_result.c_str(), res_buf_size - 1);
        return true;
    }
#endif

    local_result = string(local_path);
    len = local_result.length();
#ifdef WIN32
    while ((len > 0) &&
        (local_result.find_first_of(PATH_SPT_CHR) == 0)) {
        local_result.erase(0, 1);
        len--;
    }
    if (len == 0) {
        result[0] = '\0';
        return true;
    }
#endif

    while ((len > 0) &&
        (local_result.find_last_of(PATH_SPT_CHR) == (unsigned int)(len - 1))) {
        local_result.erase(len - 1, 1);
        len--;
    }
    if (len == 0) {
        result[0] = '\0';
        return true;
    }

    if (res_buf_size <= local_result.length()) {
        return false;
    }
    strncpy(result, local_result.c_str(), res_buf_size - 1);

    return true;
}

/**
 @功能：	从全路径中取得指定的路径成分组成字符串
 @参数：	file_path：全路径				part：路径组成
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
bool FileDir::analyze_path(const char* file_path, unsigned char part, char* result, unsigned int res_buf_size)
{
    bool ret = false;
    string local_result = "";
    string file_name = "";
    string file_ext = "";
    char local_path[MAX_PATH] = { 0 };

    char drive[MAX_PATH] = { 0 };
    char dir[MAX_PATH] = { 0 };
    char name[MAX_PATH] = { 0 };
    char ext[MAX_PATH] = { 0 };

    if ((file_path == NULL) || (strlen(file_path) == 0) ||
        (DataConvertor::trim_string(file_path, local_path, sizeof(local_path)) == 0) ||
        ((part & (PART_FILE_EXT | PART_FILE_NAME | PART_PATH | PART_DRIVE)) == 0) ||
        (result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    _splitpath(local_path, drive, dir, name, ext);
#ifndef WIN32
    strcpy(drive, PATH_SPT_STR);
#endif
    ret = trim_path(dir, dir, sizeof(dir));
    if (ret == false) {
        return false;
    }

    if ((part & PART_DRIVE) != 0x00) {
        local_result += string(drive);
    }
    if ((part & PART_PATH) != 0x00) {
        if ((local_result.length() > 0) && (strlen(dir) > 0) &&
            (local_result.find_last_of(PATH_SPT_CHR) != local_result.length() - 1)) {
            local_result += string(PATH_SPT_STR);
        }
        if ((strlen(dir) > 0) && (dir[0] == PATH_SPT_CHR)) {
            local_result += string(&dir[1]);
        } else {
            local_result += string(dir);
        }
    }
    if ((part & (PART_FILE_NAME | PART_FILE_EXT)) == (PART_FILE_NAME | PART_FILE_EXT)) {
        file_name = string(name) + string(ext);
        if ((local_result.length() > 0) && (file_name.length() > 0)) {
            local_result += string(PATH_SPT_STR);
        }
        local_result += file_name;
    } else {
        if ((part & PART_FILE_NAME) != 0x00) {
            if ((local_result.length() > 0) && (strlen(name) > 0)) {
                local_result += string(PATH_SPT_STR);
            }
            local_result += string(name);
        }
        if ((part & PART_FILE_EXT) != 0x00) {
            file_ext = string(ext);
            if ((file_ext.length() > 0) && (file_ext.at(0) == '.')) {
                file_ext.erase(0, 1);
            }
            if ((local_result.length() > 0) && (file_ext.length() > 0)) {
                local_result += string(PATH_SPT_STR);
            }
            local_result += file_ext;
        }
    }

    if (res_buf_size <= local_result.length()) {
        return false;
    }
    strncpy(result, local_result.c_str(), res_buf_size - 1);

    return true;
}

/**
 @功能：	链接2个路径组成新路径
 @参数：	file_path1：路径1				file_path2：路径2（若为绝对路径则将其作为结果）
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
bool FileDir::combine_path(const char* file_path1, const char* file_path2, char* result, unsigned int res_buf_size)
{
    string local_result = "";
    string str_tmp = "";
    int len1 = 0, len2 = 0;
    int pos = (int)string::npos;

    if ((result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    if ((file_path1 != NULL) && (strlen(file_path1) > 0)) {
        local_result += string(file_path1);
    }

    len1 = local_result.length();
    while ((len1 > 0) && ((pos = local_result.find_last_of(PATH_SPT_CHR)) == len1 - 1)) {
        local_result.erase(len1 - 1, 1);
        len1--;
    }

    if ((file_path2 != NULL) && (strlen(file_path2) > 0)) {
        str_tmp = string(file_path2);
        len2 = str_tmp.length();
        while ((len2 > 0) && ((pos = str_tmp.find_first_of(PATH_SPT_CHR)) == 0)) {
            str_tmp.erase(0, 1);
            len2--;
        }

        if ((len1 == 0) && (len2 == 0)) {
            return false;
        }

        if (len1 == 0) {
            local_result += str_tmp;
        } else {
            if (len2 > 0) {
                if (IS_FULL_PATH(str_tmp.c_str())) {
                    local_result = str_tmp;
                } else {
                    local_result += string(PATH_SPT_STR);
                    local_result += str_tmp;
                }
            }
        }
    }
    if (res_buf_size <= local_result.length()) {
        return false;
    }
    strncpy(result, local_result.c_str(), res_buf_size - 1);

    return true;
}

/**
 @功能：	链接3个路径组成新路径
 @参数：	file_path1：路径1				file_path2：路径2
 @			file_path3：路径3
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
bool FileDir::combine_path(
    const char* file_path1, const char* file_path2, const char* file_path3, char* result, unsigned int res_buf_size)
{
    bool ret = true;

    ret = combine_path(file_path1, file_path2, result, res_buf_size);
    if (ret) {
        ret = combine_path(result, file_path3, result, res_buf_size);
    }

    return ret;
}

/**
 @功能：	链接4个路径组成新路径
 @参数：	file_path1：路径1				file_path2：路径2
 @			file_path3：路径3				file_path4：路径4
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
bool FileDir::combine_path(
    const char* file_path1, const char* file_path2, const char* file_path3, const char* file_path4, char* result, unsigned int res_buf_size)
{
    bool ret = true;

    ret = combine_path(file_path1, file_path2, result, res_buf_size);
    if (ret) {
        ret = combine_path(result, file_path3, result, res_buf_size);
        if (ret) {
            ret = combine_path(result, file_path4, result, res_buf_size);
        }
    }

    return ret;
}

/**
 @功能：	取得指定路径的上级路径
 @参数：	file_path：指定路径				reduce_count：指定上几级路径（>0）
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
bool FileDir::reduce_path(const char* file_path, int reduce_count, char* result, unsigned int res_buf_size)
{
    string local_file_path = "";
    int len = 0;
    int pos = (int)string::npos;

    if ((file_path == NULL) || (strlen(file_path) == 0) ||
        (result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    if (reduce_count == 0) {
        strncpy(result, file_path, res_buf_size - 1);
        return true;
    }

    local_file_path = string(file_path);
    len = local_file_path.length();
    while ((len > 0) && ((pos = local_file_path.find_last_of(PATH_SPT_CHR)) == len - 1)) {
        local_file_path.erase(len - 1, 1);
        len--;
    }
    if (len == 0) {
        return false;
    }
    if (pos == (int)string::npos) {
        return false;
    }
    local_file_path.erase(pos, len - pos);
    if ((--reduce_count) == 0) {
        if (res_buf_size <= local_file_path.length()) {
            return false;
        }
        strncpy(result, local_file_path.c_str(), res_buf_size - 1);
#ifndef WIN32
        if ((strlen(result) == 0) && (file_path[0] == PATH_SPT_CHR)) {
            strncpy(result, PATH_SPT_STR, res_buf_size - 1);
        }
#endif
        return true;
    }

    return reduce_path(local_file_path.c_str(), reduce_count, result, res_buf_size);
}

/**
 @功能：	分解路径，把完整路径给分割开来(同Windows _splitpath函数)
 @参数：	path: 入参路径   drive: 空值	dir: 中间路径
 @		fname: 文件名    ext: 文件扩展名
 @返回：	无返回值
 */
void FileDir::_splitpath(const char *path, char *drive, char *dir, char *fname, char *ext)
{
    char *p_whole_name;

    drive[0] = '\0';
    if (path == NULL) {
        dir[0] = '\0';
        fname[0] = '\0';
        ext[0] = '\0';
        return;
    }

    if (path[strlen(path)] == '/') {
        strcpy(dir, path);
        fname[0] = '\0';
        ext[0] = '\0';
        return;
    }

    p_whole_name = rindex((char*)path, '/');
    if (p_whole_name != NULL) {
        p_whole_name++;
        _split_whole_name(p_whole_name, fname, ext);
        snprintf(dir, p_whole_name - path, "%s", path);
    } else {
        _split_whole_name(path, fname, ext);
        dir[0] = '\0';
    }
}

/**
 @功能：	分解文件名和文件扩展名
 @参数：	whole_name: 入参文件名   fname: 文件名    ext: 文件扩展名
 @返回：	无返回值
 */
void FileDir::_split_whole_name(const char *whole_name, char *fname, char *ext)
{
    char *p_ext;

    p_ext = rindex((char*)whole_name, '.');
    if (p_ext != NULL) {
        strcpy(ext, p_ext);
        snprintf(fname, p_ext - whole_name + 1, "%s", whole_name);
    } else {
        ext[0] = '\0';
        strcpy(fname, whole_name);
    }
}

/**
 @功能：	由模块句柄取得模块文件名
 @参数：	module_handle：模块句柄
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileDir::get_module_name(HMODULE module_handle, char* result, unsigned int res_buf_size)
{
    unsigned long ret = 0;
    char module_path[MAX_PATH] = { 0 };

    if ((result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    ret = GetModuleFileName(module_handle, module_path, sizeof(module_path));
    if (ret == 0) {
        return false;
    }

    return analyze_path(module_path, PART_FULL_NAME, result, res_buf_size);
}
#else   // LINUX
bool FileDir::get_module_name(char* result, unsigned int res_buf_size)
{
    unsigned long ret = 0;
    char module_path[MAX_PATH] = { 0 };

    if ((result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    ret = _GetModuleFileName(module_path, sizeof(module_path));
    if (ret == 0) {
        return false;
    }

    return analyze_path(module_path, PART_FULL_NAME, result, res_buf_size);
}
#endif // WIN32

/**
 @功能：	由模块句柄取得模块路径名（不包含文件名）
 @参数：	module_handle：模块句柄
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 */
#ifdef WIN32
bool FileDir::get_module_path(HMODULE module_handle, char* result, unsigned int res_buf_size)
{
    unsigned long ret = 0;
    char module_path[MAX_PATH] = { 0 };

    if ((result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    ret = GetModuleFileName(module_handle, module_path, sizeof(module_path));
    if (ret == 0) {
        return false;
    }

    return analyze_path(module_path, PART_FULL_PATH, result, res_buf_size);
}
#else   // LINUX
bool FileDir::get_module_path(char* result, unsigned int res_buf_size)
{
    unsigned long ret = 0;
    char module_path[MAX_PATH] = { 0 };

    if ((result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    ret = _GetModuleFileName(module_path, sizeof(module_path));
    if (ret == 0) {
        return false;
    }

    return analyze_path(module_path, PART_FULL_PATH, result, res_buf_size);
}
#endif // WIN32

/**
 @功能：	由指定模块所在路径组合新的全路径
 @参数：	module_handle：模块句柄			upper_folder：上级文件夹名称（可选）
 @			file_name：文件名（可选）
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	true：成功		false：失败
 @示例：
 @			若当前路径为D:\f1\f2\xxx.dll
 @			（1）get_file_path_name(module_handle, "upper_folder", "file_name")
 @				result = "D:\f1\upper_folder\file_name"
 @			（2）get_file_path_name(module_handle, NULL, "file_name")
 @				result = "D:\f1\f2\file_name"
 */
#ifdef WIN32
bool FileDir::get_file_path_name(HMODULE module_handle, const char* upper_folder, const char* file_name, char* result, unsigned int res_buf_size)
{
    bool ret = false;
    char module_path[MAX_PATH] = { 0 };

    if ((result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    ret = get_module_path(module_handle, module_path, sizeof(module_path));
    if (ret == false) {
        return false;
    }

    if ((upper_folder != NULL) && (strlen(upper_folder) > 0)) {
        ret = reduce_path(module_path, 1, module_path, sizeof(module_path));
        if (ret == false) {
            return false;
        }
        ret = combine_path(module_path, upper_folder, module_path, sizeof(module_path));
        if (ret == false) {
            return false;
        }
    }

    if ((file_name != NULL) && (strlen(file_name) > 0)) {
        ret = combine_path(module_path, file_name, module_path, sizeof(module_path));
        if (ret == false) {
            return false;
        }
    }

    if (res_buf_size <= strlen(module_path)) {
        return false;
    }

    strncpy(result, module_path, res_buf_size - 1);

    return true;
}
#else   // LINUX
bool FileDir::get_file_path_name(const char* upper_folder, const char* file_name, char* result, unsigned int res_buf_size)
{
    bool ret = false;
    char module_path[MAX_PATH] = { 0 };

    if ((result == NULL) || (res_buf_size == 0)) {
        return false;
    }

    ret = get_module_path(module_path, sizeof(module_path));
    if (ret == false) {
        return false;
    }

    if ((upper_folder != NULL) && (strlen(upper_folder) > 0)) {
        ret = reduce_path(module_path, 1, module_path, sizeof(module_path));
        if (ret == false) {
            return false;
        }
        ret = combine_path(module_path, upper_folder, module_path, sizeof(module_path));
        if (ret == false) {
            return false;
        }
    }

    if ((file_name != NULL) && (strlen(file_name) > 0)) {
        ret = combine_path(module_path, file_name, module_path, sizeof(module_path));
        if (ret == false) {
            return false;
        }
    }

    if (res_buf_size <= strlen(module_path)) {
        return false;
    }

    strncpy(result, module_path, res_buf_size - 1);

    return true;
}
#endif // WIN32

/**
 @功能：	实现Windows GetModuleFileName()
 @参数：	sFileName：结果字符串Buffer		nSize：结果Buffer大小
 @返回：	成功,fanhui 复制到sFileName的实际字符数量；0表示失败
 */
unsigned long FileDir::_GetModuleFileName(char* sFileName, unsigned long nSize)
{
    unsigned long len = 0;
    char sLine[1024] = { 0 };
    void* pSymbol = (void*)"";
    FILE *fp; char *pPath;

    fp = fopen ("/proc/self/maps", "r");
    if (fp != NULL) {
        while (!feof(fp)) {
            unsigned long start, end;
            if (!fgets (sLine, sizeof (sLine), fp)) {
                continue;
            }
            if (!strstr (sLine, " r-xp ") || !strchr (sLine, '/')) {
                continue;
            }
            sscanf (sLine, "%lx-%lx ", &start, &end);
            if (pSymbol >= (void *) start && pSymbol < (void *) end) {
                char *tmp;
                pPath = strchr(sLine, '/');
                tmp = strrchr(pPath, '\n');
                if (tmp) {
                    *tmp = 0;
                }
                len = (strlen(pPath) > nSize ? nSize : strlen(pPath));
                // Delete的情况
                if (len > 10 && strcmp (pPath + len - 10, " (deleted)") == 0) {
                    tmp = pPath + len - 10;
                    *tmp = 0;
                    len -= 10;
                }
                strncpy(sFileName, pPath, len);
            }
        }
        fclose (fp);
    }
    return len;
}

