// data_convertor.cpp : 定义静态库的函数。
//

#include "framework.h"
#include "data_convertor.h"


/**
 @功能：	判断字符串是否由连续相同字符组成
 @参数：	str：字符串				fill_char：字符
 @			fill_begin_pos：连续相同字符的起始位置（>=0）
 @			fill_length：连续相同字符的长度（-1：判断到字符串结尾）
 @返回：	true：是		false：否
 */
bool DataConvertor::is_same_char_string(
    const char* str, char fill_char, unsigned long fill_begin_pos/* = 0*/, unsigned long fill_length/* = (unsigned long)-1*/)
{
    unsigned long idx = 0;
    unsigned long end_idx = 0;

    if ((str == NULL) || (strlen(str) < fill_begin_pos + 1) ||
        (fill_length == 0)) {
        return false;
    }

    end_idx = strlen(str) - 1;
    if (end_idx > fill_begin_pos + fill_length - 1) {
        end_idx = fill_begin_pos + fill_length - 1;
    }

    for (idx = fill_begin_pos; idx <= end_idx; idx++) {
        if (str[idx] != fill_char) {
            break;
        }
    }

    return (idx > end_idx);
}

/**
 @功能：	字符串 => ULONG数值（"1234" => 1234 , "345F" => 0x345F）
 @参数：	str：字符串				convert_length：要转换的字符串长度（0/-1：全字符串转换）
 @			radix：进制（10<默认>、16、8）
 @返回：	转换结果（-1：失败）
 */
unsigned long DataConvertor::string_to_ulong(const char* str, int convert_length/* = 0*/, int radix/* = 10*/)
{
    unsigned long ulong_data = 0;
    char str_data[64] = { 0 };
    int len = convert_length;
    const char* fmt = "%lu";
    int ret = 0;

    if (str == NULL) {
        return (unsigned long)-1;
    }

    if ((convert_length == 0) || (convert_length == -1)) {
        len = strlen(str);
    }
    memset(str_data, 0x00, sizeof(str_data));
    memcpy(str_data, str, len);

    switch (radix) {
    case 10:
        fmt = "%lu";
        break;
    case 16:
        if (strncmp(str_data, "0x", strlen("0x")) == 0) {
            fmt = "0x%02X";
        } else {
            fmt = "%02X";
        }
        break;
    case 8:
        fmt = "%o";
        break;
    default:
        fmt = "%lu";
        break;
    }

    ret = sscanf(str_data, fmt, &ulong_data);

    return ((ret > 0) ? ulong_data : (unsigned long)-1);
}

/**
 @功能：	用指定个数的相同字符扩展字符串
 @参数：	str：字符串					count：扩展字符个数
 @			append_char：扩展字符
 @			result：结果字符串Buffer	res_buf_size：结果Buffer大小
 @返回：	结果字符串长度
 */
unsigned long DataConvertor::append_string(const char* str, int count, char append_char, char* result, unsigned long res_buf_size)
{
    string str_res = "";

    if ((result == NULL) || (res_buf_size == 0)) {
        if ((str != NULL) && (strlen(str) > 0)) {
            return strlen(str);
        }
        return 0;
    }

    if ((str != NULL) && (strlen(str) > 0)) {
        str_res = string(str);
    }
    str_res.append(count, append_char);
    strncpy(result, str_res.c_str(), res_buf_size - 1);

    return strlen(result);
}

/**
 @功能：	将字符串扩展为新（更长的）字符串（原字符串按指定的对齐方式放入新字符串）
 @参数：	str：字符串						align_mode：对齐方式
 @			align_length：扩展字符长度
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	结果字符串长度
 */
unsigned long DataConvertor::align_string(
    const char* str, unsigned short align_mode, unsigned long align_length, char* result, unsigned long res_buf_size)
{
    string str_res = "";
    unsigned long left = 0, str_len = 0, right = 0;
    unsigned long diff = 0;

    if ((result == NULL) || (res_buf_size == 0)) {
        return 0;
    }
    if (align_length == 0) {
        result[0] = '\0';
        return 0;
    }

    if ((str == NULL) || (strlen(str) == 0)) {
        str_len = 0;
        str_res.append(align_length, ' ');
    } else {
        str_len = strlen(str);
        if (align_length <= str_len) {
            str_res.append(str, align_length);
        }
        else {
            diff = align_length - str_len;
            switch (align_mode) {
            case STRING_ALIGN_LEFT:
                left = 0;
                right = diff;
                break;
            case STRING_ALIGN_CENTER:
                left = diff / 2;
                right = align_length - left - str_len;
                break;
            case STRING_ALIGN_RIGHT:
                left = diff;
                right = 0;
                break;
            default:		// STRING_ALIGN_LEFT
                left = 0;
                right = diff;
                break;
            }
            str_res.append((int)left, ' ');
            str_res.append(str);
            str_res.append((int)right, ' ');
        }
    }
    strncpy(result, str_res.c_str(), res_buf_size - 1);

    return strlen(result);
}

/**
 @功能：	剪切字符串首尾的空白
 @参数：	str：字符串
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	结果字符串长度
 */
unsigned long DataConvertor::trim_string(const char* str, char* result, unsigned long res_buf_size)
{
    unsigned long len = 0;
    unsigned long pre_len = 0;

    if ((str == NULL) || (strlen(str) == 0)) {
        return 0;
    }

    do {
        pre_len = len;
        len = trim_string(str, " \t", result, res_buf_size);
    } while (len != pre_len);

    return len;
}

/**
 @功能：	剪切字符串首尾连续的指定字符
 @参数：	str：字符串						trim_char：指定字符
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	结果字符串长度
 */
unsigned long DataConvertor::trim_string(const char* str, char trim_char, char* result, unsigned long res_buf_size)
{
    string str_res = "";
    int len = 0;

    if ((result == NULL) || (res_buf_size == 0)) {
        return 0;
    }

    if ((str == NULL) || (strlen(str) == 0)) {
        result[0] = '\0';
        return 0;
    }

    str_res = string(str);
    len = str_res.length();
    while ((len > 0) && (str_res.find_first_of(trim_char) == 0)) {
        str_res.erase(0, 1);
        len--;
    }
    if (len == 0) {
        result[0] = '\0';
        return 0;
    }

    while ((len > 0) && (str_res.find_last_of(trim_char) == (unsigned int)(len - 1))) {
        str_res.erase(len - 1, 1);
        len--;
    }
    if (len == 0) {
        result[0] = '\0';
        return 0;
    }

    if (res_buf_size <= str_res.length()) {
        result[0] = '\0';
        return 0;
    }
    strncpy(result, str_res.c_str(), res_buf_size - 1);

    return strlen(result);
}

/**
 @功能：	剪切字符串首尾连续的指定字符集中的字符
 @参数：	str：字符串						trim_char_set：指定字符集
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	结果字符串长度
 */
unsigned long DataConvertor::trim_string(const char* str, const char* trim_char_set, char* result, unsigned long res_buf_size)
{
    string str_res = "";
    int len = 0;
    size_t idx = 0;

    if ((result == NULL) || (res_buf_size == 0)) {
        return 0;
    }

    if ((str == NULL) || (strlen(str) == 0)) {
        result[0] = '\0';
        return 0;
    }

    if ((trim_char_set == NULL) || (strlen(trim_char_set) == 0)) {
        strncpy(result, str, res_buf_size - 1);
        return strlen(result);
    }

    str_res = string(str);
    len = str_res.length();
    while (len > 0) {
        for (idx = 0; idx < strlen(trim_char_set); idx++) {
            if (str_res.find_first_of(trim_char_set[idx]) == 0) {
                break;
            }
        }
        if (idx < strlen(trim_char_set)) {
            str_res.erase(0, 1);
            len--;
        } else {
            break;
        }
    }
    if (len == 0) {
        result[0] = '\0';
        return 0;
    }

    while (len > 0) {
        for (idx = 0; idx < strlen(trim_char_set); idx++) {
            if (str_res.find_last_of(trim_char_set[idx]) == (unsigned int)(len - 1)) {
                break;
            }
        }
        if (idx < strlen(trim_char_set)) {
            str_res.erase(len - 1, 1);
            len--;
        } else {
            break;
        }
    }
    if (len == 0) {
        result[0] = '\0';
        return 0;
    }

    if (res_buf_size <= str_res.length()) {
        result[0] = '\0';
        return 0;
    }
    strncpy(result, str_res.c_str(), res_buf_size - 1);

    return strlen(result);
}

/**
 @功能：	剪切字符串指定字符右边的部分（指定字符不保留）
 @参数：	str：字符串						end_char：指定字符
 @			result：结果字符串Buffer		res_buf_size：结果Buffer大小
 @返回：	结果字符串长度
 */
unsigned long DataConvertor::trim_string_by_end_char(const char* str, char end_char, char* result, unsigned long res_buf_size)
{
    string str_res = "";
    unsigned int pos = (unsigned int)string::npos;

    if ((result == NULL) || (res_buf_size == 0)) {
        return 0;
    }

    if ((str == NULL) || (strlen(str) == 0)) {
        result[0] = '\0';
        return 0;
    }

    str_res = string(str);
    pos = str_res.find_first_of(end_char);
    if (pos != (unsigned int)string::npos) {
        str_res[pos] = '\0';
    }

    if (res_buf_size <= str_res.length()) {
        result[0] = '\0';
        return 0;
    }
    strncpy(result, str_res.c_str(), res_buf_size - 1);

    return strlen(result);
}

/**
 @功能：	UNICODE版双NULL结尾字符串 => ASCII版双NULL结尾字符串
 @参数：	unicode_string：UNICODE版双NULL结尾字符串			ascii_string：ASCII版双NULL结尾字符串Buffer
 @			buf_size：结果Buffer大小
 @返回：	转换后所需的结果Buffer大小（ascii_string=NULL or buf_size=0 时也返回结果）
 */
unsigned long DataConvertor::two_null_end_unicode_string_to_ascii(const wchar_t* unicode_string, char* ascii_string, unsigned long buf_size)
{
    unsigned long count = 0;

    if (unicode_string == NULL) {
        return 0;
    }

    while ((unicode_string[count] != L'\0') ||
        (unicode_string[count + 1] != L'\0')) {
        count++;
    }
    count += 2;

#ifdef WIN32
    return WideCharToMultiByte(
        CP_ACP, 0, unicode_string, count, ascii_string, buf_size, NULL, NULL);
#else
    return d_wcstombs(ascii_string, unicode_string, buf_size);
#endif
}

/**
 @功能：	解析ASCII版的单NULL分割、双NULL结尾字符串
 @参数：	two_null_end_str：待分析的字符串			str_array：指针数组Buffer（解析后数组每个成员指向原字符串中的每个子字符串）
 @			max_string_count：指针数组Buffer成员个数
 @返回：	解析完成的子字符串个数（允许str_array=NULL or max_string_count=0）
 */
unsigned long DataConvertor::analyse_two_null_end_ascii_string(const char* two_null_end_str, char* str_array[], unsigned long max_string_count)
{
    unsigned long count = 0;
    char* str_ptr = (char*)two_null_end_str;
    bool const_true = true;

    if (two_null_end_str == NULL) {
        return 0;
    }

    if ((str_array != NULL) && (max_string_count > 0)) {
        while (count < max_string_count) {
            str_array[count++] = str_ptr;
            str_ptr += strlen(str_ptr);
            if ((str_ptr[0] == '\0') && (str_ptr[1] == '\0')) {
                break;
            }
            str_ptr++;
        }
    } else {
        while (const_true) {
            count++;
            str_ptr += strlen(str_ptr);
            if ((str_ptr[0] == '\0') && (str_ptr[1] == '\0')) {
                break;
            }
            str_ptr++;
        }
    }

    return count;
}

/**
 @功能：	解析UNICODE版的单NULL分割、双NULL结尾字符串
 @参数：	two_null_end_str：待分析的字符串			str_array：指针数组Buffer（解析后数组每个成员指向原字符串中的每个子字符串）
 @			max_string_count：指针数组Buffer成员个数
 @返回：	解析完成的子字符串个数（允许str_array=NULL or max_string_count=0）
 */
unsigned long DataConvertor::analyse_two_null_end_unicode_string(const wchar_t* two_null_end_str, wchar_t* str_array[], unsigned long max_string_count)
{
    unsigned long count = 0;
    wchar_t* str_ptr = (wchar_t*)two_null_end_str;
    bool const_true = true;

    if (two_null_end_str == NULL) {
        return 0;
    }

    if ((str_array != NULL) && (max_string_count > 0)) {
        while (count < max_string_count) {
            str_array[count++] = str_ptr;
            str_ptr += wcslen(str_ptr);
            if ((str_ptr[0] == L'\0') && (str_ptr[1] == L'\0')) {
                break;
            }
            str_ptr++;
        }
    } else {
        while (const_true) {
            count++;
            str_ptr += wcslen(str_ptr);
            if ((str_ptr[0] == L'\0') && (str_ptr[1] == L'\0')) {
                break;
            }
            str_ptr++;
        }
    }

    return count;
}

/**
 @功能：	从ASCII版的单NULL分割、双NULL结尾字符串中取得所有KEY名称
 @参数：	two_null_end_str：双NULL结尾字符串			key_array：存放Key值的字符串数组Buffer
 @			max_count：结果最大个数						val_array：存放Val值的字符串数组Buffer
 @返回：	取得的KEY个数（key_array=NULL or max_count=0 时返回KEY个数）
 */
unsigned long DataConvertor::get_keys_from_two_null_end_ascii_string(const char* two_null_end_str, char key_array[][CONST_VALUE_260], unsigned long max_count, char val_array[][CONST_VALUE_260]/* = NULL*/)
{
    unsigned long count = 0;
    unsigned long key_cnt = 0;
    char* string_ptr[CONST_VALUE_512] = { 0 };
    unsigned long idx = 0;
    char key[CONST_VALUE_260] = { 0 };
    char val[CONST_VALUE_260] = { 0 };
    bool ret = false;

    if (two_null_end_str == NULL) {
        return 0;
    }

    count = analyse_two_null_end_ascii_string(two_null_end_str, string_ptr, CONST_VALUE_512);
    if (count == 0) {
        return 0;
    }
    for (idx = 0; idx < count; idx++) {
        ret = get_key_value_from_key_value_ascii_string(string_ptr[idx], key, sizeof(key), val, sizeof(val));
        if (ret) {
            if ((key_array != NULL) && (max_count > 0) && (key_cnt < max_count)) {
                strncpy(key_array[key_cnt], key, CONST_VALUE_260 - 1);
            }
            if ((val_array != NULL) && (max_count > 0) && (key_cnt < max_count)) {
                strncpy(val_array[key_cnt], val, CONST_VALUE_260 - 1);
            }
            key_cnt++;
        }
    }

    return key_cnt;
}

/**
 @功能：	从UNICODE版的单NULL分割、双NULL结尾字符串中取得所有KEY名称（ASCII版）
 @参数：	two_null_end_str：双NULL结尾字符串			key_array：存放Key值的字符串数组Buffer（ASCII版）
 @			max_count：结果最大个数						val_array：存放Val值的字符串数组Buffer（ASCII版）
 @返回：	取得的KEY个数（key_array=NULL or max_count=0 时返回KEY个数）
 */
unsigned long DataConvertor::get_keys_from_two_null_end_unicode_string(const wchar_t* two_null_end_str, char key_array[][CONST_VALUE_260], unsigned long max_count, char val_array[][CONST_VALUE_260]/* = NULL*/)
{
    unsigned long count = 0;
    unsigned long key_cnt = 0;
    char* key_value_str = NULL;
    wchar_t* wstring_ptr[CONST_VALUE_512] = { 0 };
    unsigned long char_cnt = 0;
    unsigned long idx = 0;
    char key[CONST_VALUE_260] = { 0 };
    char val[CONST_VALUE_260] = { 0 };
    bool ret = false;

    if (two_null_end_str == NULL) {
        return 0;
    }

    count = analyse_two_null_end_unicode_string(two_null_end_str, wstring_ptr, CONST_VALUE_512);
    if (count == 0) {
        return 0;
    }

    for (idx = 0; idx < count; idx++) {
        if (wstring_ptr[idx] == NULL) {
            return 0;
        }
        char_cnt = wcslen(wstring_ptr[idx]) + 1;
        key_value_str = new char[char_cnt];
        if (key_value_str == NULL) {
            return 0;
        }
        memset(key_value_str, 0x00, sizeof(char) * char_cnt);
#ifdef WIN32
        char_cnt = WideCharToMultiByte(
            CP_ACP, 0, wstring_ptr[idx], -1, key_value_str, char_cnt, NULL, NULL);
#else   // LINUX
        char_cnt = d_wcstombs(key_value_str, wstring_ptr[idx], char_cnt);
#endif
        ret = get_key_value_from_key_value_ascii_string(key_value_str, key, sizeof(key), val, sizeof(val));
        delete[] key_value_str;
        if (ret) {
            if ((key_array != NULL) && (max_count > 0) && (key_cnt < max_count)) {
                strncpy(key_array[key_cnt], key, CONST_VALUE_260 - 1);
            }
            if ((val_array != NULL) && (max_count > 0) && (key_cnt < max_count)) {
                strncpy(val_array[key_cnt], val, CONST_VALUE_260 - 1);
            }
            key_cnt++;
        }
    }

    return key_cnt;
}

/**
 @功能：	从ASCII版"Key=Value"字符串中提取Key名称和Value
 @参数：	key_value_str："Key=Value"字符串			key：存放Key的Buffer
 @			key_buf_size：Key的Buffer大小				value：存放Value的Buffer
 @			value_buf_size：Value的Buffer大小			split_char：Key与Value之间的分隔符（默认'='）
 @返回：	true：成功		false：失败		（允许只提取key或value）
 */
bool DataConvertor::get_key_value_from_key_value_ascii_string(
    const char* key_value_str, char* key, unsigned int key_buf_size, char* value, unsigned int value_buf_size, const char split_char/* = '='*/)
{
    size_t pos = string::npos;
    string str = "";

    if ((key_value_str == NULL) || (strlen(key_value_str) == 0)) {
        return false;
    }

    if (((key == NULL) || (key_buf_size == 0)) &&
        ((value == NULL) || (value_buf_size == 0))) {
        return false;
    }

    str = string(key_value_str);
    pos = str.find(split_char);
    if (pos == string::npos) {
        if ((key != NULL) && (key_buf_size > 0)) {
            strncpy(key, key_value_str, key_buf_size - 1);
            trim_string(key, key, key_buf_size);
        }
        if ((value != NULL) && (value_buf_size > 0)) {
            memset(value, 0x00, value_buf_size);
        }
        return true;
    }

    if ((key != NULL) && (key_buf_size > 0)) {
        strncpy(key, key_value_str, pos);
        trim_string(key, key, key_buf_size);
    }
    if ((value != NULL) && (value_buf_size > 0)) {
        strncpy(value, &key_value_str[pos + 1], value_buf_size - 1);
        trim_string(value, value, value_buf_size);
    }

    return true;
}

/**
 @功能：	从UNICODE版"Key=Value"字符串中提取Key名称和Value（ASCII版）
 @参数：	key_value_str："Key=Value"字符串			key：存放Key的Buffer
 @			key_buf_size：Key的Buffer大小				value：存放Value的Buffer
 @			value_buf_size：Value的Buffer大小			split_char：Key与Value之间的分隔符（默认'='）
 @返回：	true：成功		false：失败		（允许只提取key或value）
 */
bool DataConvertor::get_key_value_from_key_value_unicode_string(
    const wchar_t* key_value_str, char* key, unsigned int key_buf_size, char* value, unsigned int value_buf_size, const char split_char/* = '='*/)
{
    bool ret = false;
    char* ascii_key_value_str = NULL;
    unsigned long char_cnt = 0;

    if ((key_value_str == NULL) || (wcslen(key_value_str) == 0)) {
        return false;
    }

    char_cnt = wcslen(key_value_str) + 1;
    ascii_key_value_str = new char[char_cnt];
    if (key_value_str == NULL) {
        return false;
    }
    memset(ascii_key_value_str, 0x00, sizeof(char) * char_cnt);
#ifdef WIN32
    char_cnt = WideCharToMultiByte(
        CP_ACP, 0, key_value_str, -1, ascii_key_value_str, char_cnt, NULL, NULL);
#else
    char_cnt = d_wcstombs(ascii_key_value_str, key_value_str, char_cnt);
#endif
    if (char_cnt == 0) {
        delete[] ascii_key_value_str;
        return false;
    }
    ret = get_key_value_from_key_value_ascii_string(ascii_key_value_str, key, key_buf_size, value, value_buf_size, split_char);
    delete[] ascii_key_value_str;

    return ret;
}

/**
 @功能：	从ASCII版"Key1=Value1\0Key2=Value2\0\0"字符串中取得指定Key对应的Value
 @参数：	two_null_end_str：双NULL结尾字符串			key：指定Key
 @			value：存放Value的Buffer					value_buf_size：Value的Buffer大小
 @返回：	true：成功		false：失败
 */
bool DataConvertor::get_value_from_two_null_end_ascii_string_by_key(
    const char* two_null_end_str, const char* key, char* value, unsigned int value_buf_size)
{
    bool ret = false;
    unsigned long count = 0;
    char* string_ptr[CONST_VALUE_512] = { 0 };
    char local_key[CONST_VALUE_260] = { 0 };
    char local_value[CONST_VALUE_512] = { 0 };
    unsigned long idx = 0;

    if ((two_null_end_str == NULL) ||
        (key == NULL) || (strlen(key) == 0) ||
        (value == NULL) || (value_buf_size == 0)) {
        return false;
    }

    count = analyse_two_null_end_ascii_string(two_null_end_str, string_ptr, CONST_VALUE_512);
    if (count == 0) {
        return false;
    }
    for (idx = 0; idx < count; idx++) {
        ret = get_key_value_from_key_value_ascii_string(
            string_ptr[idx], local_key, sizeof(local_key), local_value, sizeof(local_value));
        if (ret != false) {
            if (strcmp(local_key, key) == 0) {
                strncpy(value, local_value, value_buf_size - 1);
                break;
            }
        }
    }

    return (idx < count);
}

/**
 @功能：	从UNICODE版L"Key1=Value1\0Key2=Value2\0\0"字符串中取得指定Key对应的Value（ASCII版）
 @参数：	two_null_end_str：双NULL结尾字符串			key：指定Key
 @			value：存放Value的Buffer					value_buf_size：Value的Buffer大小
 @返回：	true：成功		false：失败
 */
bool DataConvertor::get_value_from_two_null_end_unicode_string_by_key(
    const wchar_t* two_null_end_str, const char* key, char* value, unsigned int value_buf_size)
{
    bool ret = false;
    unsigned long count = 0;
    wchar_t* string_ptr[CONST_VALUE_512] = { 0 };
    char local_key[CONST_VALUE_260] = { 0 };
    char local_value[CONST_VALUE_512] = { 0 };
    unsigned long idx = 0;

    if ((two_null_end_str == NULL) ||
        (key == NULL) || (strlen(key) == 0) ||
        (value == NULL) || (value_buf_size == 0)) {
        return false;
    }

    count = analyse_two_null_end_unicode_string(two_null_end_str, string_ptr, CONST_VALUE_512);
    if (count == 0) {
        return false;
    }
    for (idx = 0; idx < count; idx++) {
        ret = get_key_value_from_key_value_unicode_string(
            string_ptr[idx], local_key, sizeof(local_key), local_value, sizeof(local_value));
        if (ret != false) {
            if (strcmp(local_key, key) == 0) {
                strncpy(value, local_value, value_buf_size - 1);
                break;
            }
        }
    }

    return (idx < count);
}

/**
 @功能：	字符串 => HEX数组（"345F" => {0x34, 0x5F}）
 @参数：	str：字符串						convert_length：要转换的字符串长度（0/-1：全字符串转换）
 @			hex_data：十六进制数组地址		data_buf_size：数组最大长度
 @返回：	转换后十六进制数据个数（0：失败）
 */
unsigned long DataConvertor::string_to_hex(const char* str, unsigned long convert_length, unsigned char* hex_data, unsigned long data_buf_size)
{
    unsigned long hex_len = 0;
    unsigned long str_len = 0;
    unsigned long idx = 0;
    const char* str_ptr = NULL;
    int tmp_data = 0;

    if ((str == NULL) || (hex_data == NULL)) {
        return 0;
    }

    str_len = convert_length;
    if ((convert_length == 0) || (convert_length == (unsigned long)-1)) {
        str_len = strlen(str);
    }
    if ((str_len == 0) || (str_len % 2 != 0)) {
        return 0;
    }
    hex_len = str_len / 2;
    if (data_buf_size < hex_len) {
        return 0;
    }

    str_ptr = str;
    for (idx = 0; idx < hex_len; idx++) {
        if (sscanf(str_ptr, "%02X", &tmp_data) <= 0) {
            break;
        }
        hex_data[idx] = (BYTE)tmp_data;
        str_ptr += 2;
    }

    return idx;
}

/**
 @功能：	HEX数组 => 字符串（{0x34, 0x5F} => "345F"）
 @参数：	hex_data：十六进制数组地址		data_length：数组中数据长度
 @			result：结果字符串				res_buf_size：结果Buffer长度
 @返回：	结果字符串的字符个数（0：失败）
 */
unsigned long DataConvertor::hex_to_string(unsigned char* hex_data, unsigned long data_length, char* result, unsigned long res_buf_size)
{
    unsigned long idx = 0;
    char str_data[16] = { 0 };

    if ((hex_data == NULL) || (data_length == 0) ||
        (result == NULL) || (res_buf_size == 0)) {
        return 0;
    }
    if (res_buf_size <= data_length * 2) {
        return 0;
    }

    memset(result, 0x00, res_buf_size);
    for (idx = 0; idx < data_length; idx++) {
        if (snprintf(str_data, sizeof(str_data), "%02X", hex_data[idx]) != 2) {
            break;
        }
        strcat(result, str_data);
    }

    return (idx * 2);
}

/**
 @功能：	以指定字符分割字符串
 @参数：	str：字符串								split_char：分隔符
 @			string_array：结果字符串数组Buffer		max_string_count：结果Buffer最大字符串个数
 @返回：	字符串个数（若string_array=NULL or max_string_count=0，返回字符串个数）
 */
unsigned long DataConvertor::split_string(const char* str, char split_char, char string_array[][CONST_VALUE_260], unsigned long max_string_count)
{
    unsigned long cnt = 0;
    string str_local = "", s = "";
    size_t pos = string::npos, pre = 0;

    if (str == NULL) {
        return 0;
    }
    if ((strlen(str) == 0) || (split_char == '\0')) {
        if ((string_array != NULL) && (max_string_count > 0)) {
            strncpy(string_array[0], str, CONST_VALUE_260 - 1);
        }
        return 0;
    }

    str_local = string(str);
    pos = str_local.find(split_char, pre);
    while (pos != string::npos) {
        s = str_local.substr(pre, pos - pre);
        pre = pos + 1;
        pos = str_local.find(split_char, pre);
        if ((string_array != NULL) && (cnt < max_string_count)) {
            strncpy(string_array[cnt], s.c_str(), CONST_VALUE_260 - 1);
        }
        cnt++;
    }
    s = str_local.substr(pre);
    if ((string_array != NULL) && (cnt < max_string_count)) {
        strncpy(string_array[cnt], s.c_str(), CONST_VALUE_260 - 1);
    }
    cnt++;

    if ((string_array != NULL) && (max_string_count > 0)) {
        cnt = min(cnt, max_string_count);
    }

    return cnt;
}

/**
 @功能：	以指定字符分割字符串并转换为长整数
 @参数：	str：字符串								split_char：分隔符
 @			radix：进制
 @			number_array：结果数组Buffer			max_number_count：结果Buffer最大数据个数
 @返回：	数据个数（若number_array=NULL or max_number_count=0，返回数据个数）
 */
unsigned long DataConvertor::split_string_to_numbers(
    const char* str, char split_char, int radix, unsigned long number_array[], unsigned long max_number_count)
{
    unsigned long count = 0;
    unsigned long idx = 0;
    char* buffer = NULL;
    char (*str_array)[CONST_VALUE_260];

    if ((str == NULL) || (strlen(str) == 0)) {
        return 0;
    }

    if (split_char == '\0') {
        if ((number_array != NULL) && (max_number_count > 0)) {
            number_array[0] = string_to_ulong(str, 0, radix);
        }
        return 1;
    }

    count = split_string(str, split_char, NULL, 0);
    if (count > 0) {
        buffer = new char[count * CONST_VALUE_260];
        if (buffer == NULL) {
            return 0;
        }
        memset(buffer, 0x00, count * CONST_VALUE_260);
        str_array = (char(*)[CONST_VALUE_260])buffer;
        count = split_string(str, split_char, str_array, count);
        if (count == 0) {
            delete[] buffer;
            return 0;
        }
        if ((number_array != NULL) && (max_number_count > 0)) {
            count = min(count, max_number_count);
            for (idx = 0; idx < count; idx++) {
                number_array[idx] = string_to_ulong(str_array[idx], 0, radix);
            }
        }
        delete[] buffer;
    }

    return count;
}

/**
 @功能：	以指定字符分割字符串并在其中查找指定字符串
 @参数：	str：要查找的字符串											split_char：分隔符
 @			sub_str：进制												string_array：容纳分割结果的字符串列表Buffer（NULL：不使用）
 @			max_string_count：分割结果的最大字符串个数（0：不使用）		string_count：分割字符串总个数（NULL：不使用）
 @返回：	查找的字符串在字符串列表中的索引值（-1：未找到）
 */
unsigned long DataConvertor::split_and_find_string(
    const char* str, char split_char, const char* sub_str,
    char string_array[][CONST_VALUE_260]/* = NULL*/, unsigned long max_string_count/* = 0*/, unsigned long* string_count/* = NULL*/)
{
    unsigned long count = 0;
    unsigned long total_cnt = 0;
    unsigned long idx = 0;
    char* buf = NULL;
    char(*str_array)[CONST_VALUE_260];

    if ((str == NULL) || (sub_str == NULL)) {
        return (unsigned long)-1;
    }

    count = split_string(str, split_char, NULL, 0);
    if (count == 0) {
        return (unsigned long)-1;
    }
    buf = new char[count * CONST_VALUE_260];
    if (buf == NULL) {
        return 0;
    }
    memset(buf, 0x00, count * CONST_VALUE_260);
    str_array = (char(*)[CONST_VALUE_260])buf;
    count = split_string(str, split_char, str_array, count);
    if (count == 0) {
        delete[] buf;
        return (unsigned long)-1;
    }
    if ((string_array != NULL) && (max_string_count > 0)) {
        total_cnt = min(count, max_string_count);
        memcpy(string_array, str_array, total_cnt * CONST_VALUE_260);
    } else {
        total_cnt = count;
    }
    if (string_count != NULL) {
        *string_count = total_cnt;
    }
    for (idx = 0; idx < count; idx++) {
        if (strcmp(str_array[idx], sub_str) == 0) {
            break;
        }
    }
    delete[] buf;
    if (idx >= count) {
        return (unsigned long)-1;
    }

    return idx;
}

/**
 @功能：	在全体字符串中查找子字符串
 @参数：	whole_str：全体字符串			sub_str：子字符串
 @			must_in_end：子字符串是否必须位于全体字符串的末尾
 @返回：	子字符串在全体字符串的位置（-1：未查到）
 */
unsigned long DataConvertor::find_string(const char* whole_str, const char* sub_str, bool must_in_end/* = false*/)
{
    bool found = false;
    string str = "";
    size_t pos = string::npos;

    if (whole_str == NULL) {
        return (unsigned long)-1;
    }
    if ((sub_str == NULL) || (strlen(sub_str) == 0)) {
        return (unsigned long)strlen(whole_str);
    }

    str = string(whole_str);
    pos = str.find(sub_str);
    if (must_in_end == false) {
        found = (pos != string::npos);
    } else {
        found = (pos == (str.length() - strlen(sub_str)));
    }

    return (found ? (unsigned long)pos : (unsigned long)-1);
}

/**
 @功能：	字符串转换：ASCII => UTF-8
 @参数：	ascii_str：ASCII字符串			utf8_str：UTF-8字符串Buffer
 @			buf_size：结果Buffer长度
 @返回：	结果字符串Buffer长度
 @			若(utf8_str=NULL or buf_size=0)返回必须的结果Buffer长度
 */
unsigned long DataConvertor::string_ascii_to_utf8(const char* ascii_str, char* utf8_str, unsigned long buf_size)
{
    int len = 0;
#ifdef WIN32
    unsigned int code_page = 0;
    wchar_t* unicode_str = NULL;
#endif // WIN32

    if ((utf8_str != NULL) && (buf_size > 0)) {
        memset(utf8_str, 0x00, buf_size);
    }
    if ((ascii_str == NULL) || (strlen(ascii_str) == 0)) {
        return 0;
    }

#ifdef WIN32
    code_page = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
    len = MultiByteToWideChar(code_page, 0, ascii_str, -1, NULL, 0);
    if (len == 0) {
        return 0;
    }
    unicode_str = new wchar_t[len];
    if (unicode_str == NULL) {
        return 0;
    }
    len = MultiByteToWideChar(code_page, 0, ascii_str, -1, unicode_str, len);
    if (len == 0) {
        delete[] unicode_str;
        return 0;
    }

    len = WideCharToMultiByte(CP_UTF8, 0, unicode_str, -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        delete[] unicode_str;
        return 0;
    }
    if ((utf8_str != NULL) && (buf_size > 0)) {
        if (len > (int)buf_size) {
            len = (int)buf_size;
        }
        len = WideCharToMultiByte(CP_UTF8, 0, unicode_str, -1, utf8_str, len, NULL, NULL);
    }
    delete[] unicode_str;
#else
    char cInCode[] = "GBK";
    char cOutCode[] = "UTF-8";
    len = code_convert(cInCode, cOutCode, ascii_str, strlen(ascii_str), utf8_str, buf_size);
#endif  // WIN32

    return len;

}

/**
 @功能：	字符串转换：UTF-8 => ASCII
 @参数：	utf8_str：UTF-8字符串			ascii_str：ASCII字符串Buffer
 @			buf_size：结果Buffer长度
 @返回：	结果字符串Buffer长度（含'\0'）
 @			若(ascii_str=NULL or buf_size=0)返回必须的结果Buffer长度
 */
unsigned long DataConvertor::string_utf8_to_ascii(const char* utf8_str, char* ascii_str, unsigned long buf_size)
{
    int len = 0;
#ifdef WIN32
    unsigned int code_page = 0;
    wchar_t* unicode_str = NULL;
#endif

    if ((ascii_str != NULL) && (buf_size > 0)) {
        memset(ascii_str, 0x00, buf_size);
    }
    if (utf8_str == NULL) {
        return 0;
    }
#ifdef WIN32
    code_page = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
    len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, NULL, 0);
    if (len == 0) {
        return 0;
    }
    unicode_str = new wchar_t[len];
    if (unicode_str == NULL) {
        return 0;
    }
    len = MultiByteToWideChar(CP_UTF8, 0, utf8_str, -1, unicode_str, len);
    if (len == 0) {
        delete[] unicode_str;
        return 0;
    }

    len = WideCharToMultiByte(code_page, 0, unicode_str, -1, NULL, 0, NULL, NULL);
    if (len == 0) {
        delete[] unicode_str;
        return 0;
    }
    if ((ascii_str != NULL) && (buf_size > 0)) {
        if (len > (int)buf_size) {
            len = (int)buf_size;
        }
        len = WideCharToMultiByte(code_page, 0, unicode_str, -1, ascii_str, len, NULL, NULL);
    }
    delete[] unicode_str;
#else
    char cInCode[] = "UTF-8";
    char cOutCode[] = "GB2312";
    len = code_convert(cInCode, cOutCode, utf8_str, strlen(utf8_str), ascii_str, buf_size);
#endif // WIN32
    return len;
}

/**
 @功能：	数据转换：BIN => BASE64
 @参数：	binary_data：二进制数据				binary_length：二进制数据长度
 @			base64_data：BASE64数据Buffer		buf_size：结果Buffer长度
 @返回：	结果数据长度
 @			若(base64_data=NULL or buf_size=0)返回必须的结果Buffer长度
 */
unsigned long DataConvertor::binary_to_base64(unsigned char* binary_data, unsigned long binary_length, unsigned char* base64_data, unsigned long buf_size)
{
    char* base64 = NULL;
    unsigned long base64_len = 0;
#ifdef WIN32
    bool ret = false;
#endif

    if ((binary_data == NULL) || (binary_length == 0)) {
        return 0;
    }

    if ((base64_data != NULL) && (buf_size > 0)) {
        base64 = (char*)base64_data;
        base64_len = (unsigned long)buf_size;
    }

#ifdef WIN32
    ret = CryptBinaryToString(binary_data, binary_length, CRYPT_STRING_BASE64|CRYPT_STRING_NOCRLF, base64, &base64_len);
    if (ret == false) {
        return 0;
    }
#else
    QByteArray qByteArray_bin((char*)binary_data, binary_length);
    base64 = qByteArray_bin.toBase64().data();
    base64_len = qByteArray_bin.toBase64().length();
    if (base64_len > buf_size) {
        base64_len = buf_size;
    }
    memcpy(base64_data, base64, base64_len);
#endif

    return base64_len;
}

/**
 @功能：	数据转换：BASE64 => BIN
 @参数：	base64_data：BASE64数据				base64_length：BASE64数据长度
 @			binary_data：二进制数据Buffer		buf_size：结果Buffer长度
 @返回：	结果数据长度
 @			若(binary_data=NULL or buf_size=0)返回必须的结果Buffer长度
 */
unsigned long DataConvertor::base64_to_binary(unsigned char* base64_data, unsigned long base64_length, unsigned char* binary_data, unsigned long buf_size)
{
    char* base64 = NULL;
    unsigned long binary_len = 0;
    unsigned char* data = NULL;
#ifdef WIN32
    bool ret = false;
    unsigned long skip = 0, out_flags = 0;
#endif

    if ((base64_data == NULL) || (base64_length == 0)) {
        return 0;
    }

    base64 = (char*)base64_data;
    if ((binary_data != NULL) && (buf_size > 0)) {
        data = binary_data;
        binary_len = buf_size;
    }
#ifdef WIN32
    ret = CryptStringToBinary(base64, base64_length, CRYPT_STRING_BASE64, data, &binary_len, &skip, &out_flags);
    if (ret == false) {
        return 0;
    }
#else
    QByteArray qByteArray_base64(base64, base64_length);
    QByteArray qByteArray_bin = QByteArray::fromBase64(qByteArray_base64);
    data = (unsigned char*)qByteArray_bin.data();
    binary_len = qByteArray_bin.length();
    if (binary_len > buf_size) {
        binary_len = buf_size;
    }
    memcpy(binary_data, data, binary_len);
#endif

    return binary_len;
}

/**
 @功能：	Windows/Linux系统时间转字符串
 @参数：	sys_time：Windows系统时间		format：字符串格式
 @		result：转换结果Buffer			res_buf_size：结果Buffer长度
 @返回：	true：成功		false：失败
 */
bool DataConvertor::systime_to_string(LPSYSTEMTIME sys_time, const char* format, char* result, unsigned int res_buf_size)
{
    SYSTEMTIME local_sys_time;
    char local_format[CONST_VALUE_64] = { "%04d/%02d/%02d %02d:%02d:%02d.%03d" };
    int count = 0;
    int i = 0;

    if ((result == NULL) || (res_buf_size < 18)) {
        return false;
    }

    if ((format != NULL) && (strlen(format) > 0)) {
        for (i = 0; i < (int)strlen(format); i++) {
            if (format[i] == '%') {
                count++;
            }
        }
        if (count <= 7) {
            strncpy(local_format, format, sizeof(local_format) - 1);
        }
    }

#ifdef WIN32
    if (sys_time == NULL) {
        GetLocalTime(&local_sys_time);
    } else {
        local_sys_time = *sys_time;
    }
#else   // LINUX
    if (sys_time == NULL) {
        local_time_to_system_time(&local_sys_time);
    } else {
        local_sys_time = *sys_time;
    }
#endif
    snprintf(result, res_buf_size, local_format,
        local_sys_time.wYear, local_sys_time.wMonth, local_sys_time.wDay,
        local_sys_time.wHour, local_sys_time.wMinute, local_sys_time.wSecond, local_sys_time.wMilliseconds);

    return true;
}

/**
 @功能：	Windows系统时间转绝对时间值
 @参数：	sys_time：Windows系统时间（NULL：当前系统时间）
 @返回：	绝对时间值
 */
#ifdef WIN32
unsigned long long DataConvertor::systime_to_quadpart(LPSYSTEMTIME sys_time/* = NULL*/)
{
    SYSTEMTIME local_sys_time;
    FILETIME file_time;
    ULARGE_INTEGER now_time;

    if (sys_time == NULL) {
        GetLocalTime(&local_sys_time);
    } else {
        local_sys_time = *sys_time;
    }
    SystemTimeToFileTime(&local_sys_time, &file_time);
    now_time.HighPart = file_time.dwHighDateTime;
    now_time.LowPart = file_time.dwLowDateTime;

    return now_time.QuadPart;
}
#endif // WIN32

/**
 @功能：	Windows文件时间转绝对时间值
 @参数：	file_time：Windows文件时间
 @返回：	绝对时间值
 */
#ifdef WIN32
unsigned long long DataConvertor::filetime_to_quadpart(FILETIME file_time)
{
    ULARGE_INTEGER now_time;

    now_time.HighPart = file_time.dwHighDateTime;
    now_time.LowPart = file_time.dwLowDateTime;

    return now_time.QuadPart;
}
#endif // WIN32

/**
 @功能：	取得2个Windows系统时间值的差
 @参数：	sys_time：Windows系统时间（被减数）		base_sys_time：Windows系统时间（减数）
 @			unit：时间单位
 @返回：	时间差值（sys_time - base_sys_time）
 */
#ifdef WIN32
long long DataConvertor::get_time_diff(SYSTEMTIME sys_time, SYSTEMTIME base_sys_time, unsigned long long unit)
{
    unsigned long long local_time = 0;
    unsigned long long local_base = 0;

    if (unit == 0) {
        unit = TIME_UNIT_MS;
    }

    local_time = systime_to_quadpart(&sys_time);
    local_base = systime_to_quadpart(&base_sys_time);

    return ((long long)(local_time - local_base) / (long long)unit);
}
#endif // WIN32

/**
 @功能：	取得2个Windows文件时间值的差
 @参数：	file_time：Windows文件时间（被减数）		base_file_time：Windows文件时间（减数）
 @			unit：时间单位
 @返回：	时间差值（file_time - base_file_time）
 */
#ifdef WIN32
long long DataConvertor::get_time_diff(FILETIME file_time, FILETIME base_file_time, unsigned long long unit)
{
    unsigned long long local_time = 0;
    unsigned long long local_base = 0;

    if (unit == 0) {
        unit = TIME_UNIT_MS;
    }

    local_time = filetime_to_quadpart(file_time);
    local_base = filetime_to_quadpart(base_file_time);

    return ((long long)(local_time - local_base) / (long long)unit);
}
#endif // WIN32

/**
 @功能：	取得Windows系统时间与文件时间值的差
 @参数：	sys_time：Windows系统时间（被减数）		base_file_time：Windows文件时间（减数）
 @			unit：时间单位
 @返回：	时间差值（sys_time - base_file_time）
 */
#ifdef WIN32
long long DataConvertor::get_time_diff(SYSTEMTIME sys_time, FILETIME base_file_time, unsigned long long unit)
{
    unsigned long long local_time = 0;
    unsigned long long local_base = 0;

    if (unit == 0) {
        unit = TIME_UNIT_MS;
    }

    local_time = systime_to_quadpart(&sys_time);
    local_base = filetime_to_quadpart(base_file_time);

    return ((long long)(local_time - local_base) / (long long)unit);
}
#endif // WIN32

/**
 @功能：	取得Windows文件时间与系统时间值的差
 @参数：	file_time：Windows文件时间（被减数）		base_sys_time：Windows系统时间（减数）
 @			unit：时间单位
 @返回：	时间差值（file_time - base_sys_time）
 */
#ifdef WIN32
long long DataConvertor::get_time_diff(FILETIME file_time, SYSTEMTIME base_sys_time, unsigned long long unit)
{
    unsigned long long local_time = 0;
    unsigned long long local_base = 0;

    if (unit == 0) {
        unit = TIME_UNIT_MS;
    }

    local_time = filetime_to_quadpart(file_time);
    local_base = systime_to_quadpart(&base_sys_time);

    return ((long long)(local_time - local_base) / (long long)unit);
}
#endif // WIN32

/**
 @功能：	本地时间转字符串（精确到秒）
 @参数：	system_time：系统时间                local_time：本地时间（NULL:取当前时间）
 @返回：	true：成功		false：失败
 */
#ifndef WIN32
bool DataConvertor::local_time_to_system_time(LPSYSTEMTIME system_time, struct tm* local_time /*= NULL*/)
{
    struct tm l_time;
    struct timeval tv;

    if (system_time == NULL) {
        return false;
    }

    if (local_time == NULL) {
        memset(&tv, 0, sizeof(tv));
        gettimeofday(&tv, NULL);
        l_time = *(localtime(&(tv.tv_sec)));
    } else {
        l_time = *local_time;
    }
    system_time->wYear = (WORD)(l_time.tm_year + 1900);
    system_time->wMonth = (WORD)(l_time.tm_mon + 1);
    system_time->wDay = (WORD)l_time.tm_mday;
    system_time->wHour = (WORD)l_time.tm_hour;
    system_time->wMinute = (WORD)l_time.tm_min;
    system_time->wSecond = (WORD)l_time.tm_sec;
    system_time->wMilliseconds = (WORD)(tv.tv_usec / 1000);

    return true;
}
#endif

/**
 @功能：	本地时间转字符串（精确到秒）
 @参数：	local_time：本地时间			format：字符串格式
 @			result：转换结果Buffer			res_buf_size：结果Buffer长度
 @返回：	true：成功		false：失败
 */
bool DataConvertor::local_time_to_string(struct tm* local_time, const char* format, char* result, unsigned int res_buf_size)
{
    time_t local_c_time;
    struct tm* tm_time = NULL;
    char local_format[CONST_VALUE_64] = { "%04d/%02d/%02d %02d:%02d:%02d" };
    int count = 0;
    int i = 0;

    if ((result == NULL) || (res_buf_size < 15)) {
        return false;
    }

    if (local_time == NULL) {
        local_c_time = time(NULL);
        tm_time = localtime(&local_c_time);
    } else {
        tm_time = local_time;
    }

    if ((format != NULL) && (strlen(format) > 0)) {
        for (i = 0; i < (int)strlen(format); i++) {
            if (format[i] == '%') {
                count++;
            }
        }
        if (count <= 6) {
            strncpy(local_format, format, sizeof(local_format) - 1);
        }
    }

    snprintf(result, res_buf_size, local_format,
        tm_time->tm_year + 1900, tm_time->tm_mon + 1, tm_time->tm_mday,
        tm_time->tm_hour, tm_time->tm_min, tm_time->tm_sec);

    return true;
}

/**
 @功能：	系统时间绝对值转字符串（精确到秒）
 @参数：	c_time：系统时间绝对值			format：字符串格式
 @			result：转换结果Buffer			res_buf_size：结果Buffer长度
 @返回：	true：成功		false：失败
 */
bool DataConvertor::ctime_to_string(time_t* c_time, const char* format, char* result, unsigned int res_buf_size)
{
    return local_time_to_string((c_time == NULL) ? NULL : localtime(c_time), format, result, res_buf_size);
}

/**
 @功能：	取得时间值的差
 @参数：	c_time：时间（被减数）		base_c_time：基准时间（减数）
 @			unit：时间单位（精度支持到秒）
 @返回：	时间差值（c_time - base_c_time）
 */
long long DataConvertor::get_time_diff(time_t c_time, time_t base_c_time, unsigned long long unit)
{
    if (unit == 0) {
        unit = TIME_UNIT_SEC;
    }

    return ((long long)(c_time - base_c_time) / (long long)(unit / TIME_UNIT_SEC));
}

/**
 @功能：	取得本地时间的差
 @参数：	local_time：本地时间（被减数）		base_local_time：基准本地时间（减数）
 @			unit：时间单位（精度支持到秒）
 @返回：	时间差值（c_time - base_c_time）
 */
long long DataConvertor::get_time_diff(struct tm local_time, struct tm base_local_time, unsigned long long unit)
{
    time_t local_c_time = 0;
    time_t local_base_c_time = 0;

    local_c_time = mktime(&local_time);
    local_base_c_time = mktime(&base_local_time);

    return get_time_diff(local_c_time, local_base_c_time, unit);
}

/**
 @功能：	取得时间的差
 @参数：	local_time：本地时间（被减数）		base_c_time：基准时间（减数）
 @			unit：时间单位（精度支持到秒）
 @返回：	时间差值（c_time - base_c_time）
 */
long long DataConvertor::get_time_diff(struct tm local_time, time_t base_c_time, unsigned long long unit)
{
    time_t local_c_time = 0;

    local_c_time = mktime(&local_time);

    return get_time_diff(local_c_time, base_c_time, unit);
}

/**
 @功能：	取得时间的差
 @参数：	c_time：时间（被减数）		base_local_time：基准本地时间（减数）
 @			unit：时间单位（精度支持到秒）
 @返回：	时间差值（c_time - base_c_time）
 */
long long DataConvertor::get_time_diff(time_t c_time, struct tm base_local_time, unsigned long long unit)
{
    time_t local_base_c_time = 0;

    local_base_c_time = mktime(&base_local_time);

    return get_time_diff(c_time, local_base_c_time, unit);
}

/**
 @功能：	多字符 => 宽字符
 @参数：	dest: 转换后字串    src: 被转换源字串    n: 被转换的字符长度（含\0，-1：作为字符串转换）
 @返回：	转换成功，返回转换的字节数（n=-1：不含\0，n!=-1：含\0），不成功返回(size_t)(-1)
 */
size_t DataConvertor::d_mbstowcs(wchar_t *dest, const char *src, size_t n)
{
    char *old_locale = NULL;
    size_t char_cnt = 0, ret_cnt = 0, rest_cnt = n, cov_cnt = 0;
    wchar_t* dst_ptr = dest;
    const char* src_ptr = src;

    if (src == NULL) {
        return (size_t)-1;
    }
    if (n == 0) {
        return 0;
    }
    if (n == (size_t)-1) {
        old_locale = strdup(setlocale(LC_CTYPE, NULL));  // 保存原来的语系 & 设置新的语系
        setlocale(LC_CTYPE, setlocale(LC_ALL, ""));
        ret_cnt = mbstowcs(dest, src, strlen(src));
        setlocale(LC_CTYPE, old_locale);// 还原语系
        free(old_locale);
    } else {
        while (rest_cnt > 0) {
            cov_cnt = strlen(src_ptr);
            if (cov_cnt > 0) {
                if (rest_cnt < strlen(src_ptr)) {
                    cov_cnt = rest_cnt;
                }
            } else {
                src_ptr++;
                *(dst_ptr++) = L'\0';
                rest_cnt--;
                ret_cnt++;
                continue;
            }

            old_locale = strdup(setlocale(LC_CTYPE, NULL));  // 保存原来的语系 & 设置新的语系
            setlocale(LC_CTYPE, setlocale(LC_ALL, ""));
            char_cnt = mbstowcs(dst_ptr, src_ptr, cov_cnt);
            setlocale(LC_CTYPE, old_locale);// 还原语系
            free(old_locale);

            ret_cnt += (char_cnt + 1);
            if (rest_cnt >= char_cnt + 1) {
                rest_cnt -= (char_cnt + 1);
            } else {
                rest_cnt -= char_cnt;
            }
            if (rest_cnt > 0) {
                src_ptr += (char_cnt + 1);
                dst_ptr += (char_cnt + 1);
            }
        }
    }

    return ret_cnt;
}

/**
 @功能：	宽字符 => 多字符
 @参数：	dest: 转换后字串    src: 被转换源字串    n: 被转换的字符长度（含\0，-1：作为字符串转换）
 @返回：	转换成功，返回转换的字节数（n=-1：不含\0，n!=-1：含\0），不成功返回(size_t)(-1)
 */
size_t DataConvertor::d_wcstombs(char *dest, const wchar_t *src, size_t n)
{
    char *old_locale = NULL;
    size_t char_cnt = 0, ret_cnt = 0, rest_cnt = n, cov_cnt = 0;
    char* dst_ptr = dest;
    const wchar_t* src_ptr = src;

    if (src == NULL) {
        return (size_t)-1;
    }
    if (n == 0) {
        return 0;
    }

    if (n == (size_t)-1) {
        old_locale = strdup(setlocale(LC_CTYPE, NULL));  // 保存原来的语系 & 设置新的语系
        setlocale(LC_CTYPE, setlocale(LC_ALL, ""));
        ret_cnt = wcstombs(dest, src, wcslen(src));
        setlocale(LC_CTYPE, old_locale);// 还原语系
        free(old_locale);
    } else {
        while (rest_cnt > 0) {
            cov_cnt = wcslen(src_ptr);
            if (cov_cnt > 0) {
                if (rest_cnt < wcslen(src_ptr)) {
                    cov_cnt = rest_cnt;
                }
            } else {
                src_ptr++;
                *(dst_ptr++) = '\0';
                rest_cnt--;
                ret_cnt++;
                continue;
            }

            old_locale = strdup(setlocale(LC_CTYPE, NULL));  // 保存原来的语系 & 设置新的语系
            setlocale(LC_CTYPE, setlocale(LC_ALL, ""));
            char_cnt = wcstombs(dst_ptr, src_ptr, cov_cnt);
            setlocale(LC_CTYPE, old_locale);// 还原语系
            free(old_locale);

            ret_cnt += (char_cnt + 1);
            if (rest_cnt >= char_cnt + 1) {
                rest_cnt -= (char_cnt + 1);
            } else {
                rest_cnt -= char_cnt;
            }
            if (rest_cnt > 0) {
                src_ptr += (char_cnt + 1);
                dst_ptr += (char_cnt + 1);
            }
        }
    }

    return ret_cnt;
}

/**
 @功能：	代码转换:从一种编码转为另一种编码
 @参数：	from_charset: 转换源字串编码    to_charset: 转换目标字串编码
 @      inbuf: 转换源字串     inlen: 转换源字串长度
 @      outbuf: 转换目标字串     outlen: 转换目标字串长度
 @返回：	转换成功，返回转换的字节数（不包括0字符），不成功返回(size_t)(-1)
 */
size_t DataConvertor::code_convert(char *from_charset, char *to_charset, const char *inbuf, int inlen, char *outbuf, int outlen)
{
    iconv_t cd;
    char **pin = (char**)(&inbuf);
    char **pout = &outbuf;
    size_t inlen_tmp = inlen;
    size_t outlen_tmp = outlen;

    cd = iconv_open(to_charset,from_charset);
    if (cd == 0) {
        return -1;
    }
    memset(outbuf,0,outlen);
    if (iconv(cd, pin, &inlen_tmp, pout, &outlen_tmp) == (size_t)-1) {
        return -1;
    }
    iconv_close(cd);

    return outlen_tmp;
}

