#ifndef DATA_CONVERTOR_H
#define DATA_CONVERTOR_H

#include "data_convertor_global.h"
#include "__common_def.h"

#include <QTextCodec>


#define TIME_UNIT_MS				(10000ULL)
#define TIME_UNIT_SEC				(1000 * 10000ULL)
#define TIME_UNIT_MIN				(60 * 1000 * 10000ULL)
#define TIME_UNIT_HOUR				(60 * 60 * 1000 * 10000ULL)
#define TIME_UNIT_DAY				(24 * 60 * 60 * 1000 * 10000ULL)

#define STRING_ALIGN_LEFT			(0)
#define STRING_ALIGN_CENTER			(1)
#define STRING_ALIGN_RIGHT			(2)

// 编码格式
#define CODE_UTF8                   1
#define CODE_GBK                    2

/****************************************************************************************************
 @功能：	常用数据转换功能，字符串解析、格式转换、时间比较等
 @参数：
 ****************************************************************************************************/
class DATA_CONVERTOR_EXPORT DataConvertor
{
DEFINE_STATIC_VERSION_FUNCTIONS("data_convertor", "0.0.0.0", TYPE_DYNAMIC)

public:
    static bool is_same_char_string(const char* str, char fill_char, unsigned long fill_begin_pos = 0, unsigned long fill_length = (unsigned long)-1);
    static unsigned long string_to_ulong(const char* str, int convert_length = 0, int radix = 10);
    static unsigned long string_to_hex(const char* str, unsigned long convert_length, unsigned char* hex_data, unsigned long data_buf_size);
    static unsigned long hex_to_string(unsigned char* hex_data, unsigned long data_length, char* result, unsigned long res_buf_size);
    static unsigned long find_string(const char* whole_str, const char* sub_str, bool must_in_end = false);
    static unsigned long string_ascii_to_utf8(const char* ascii_str, char* utf8_str, unsigned long buf_size);
    static unsigned long string_utf8_to_ascii(const char* utf8_str, char* ascii_str, unsigned long buf_size);
    static unsigned long binary_to_base64(unsigned char* binary_data, unsigned long binary_length, unsigned char* base64_data, unsigned long buf_size);
    static unsigned long base64_to_binary(unsigned char* base64_data, unsigned long base64_length, unsigned char* binary_data, unsigned long buf_size);

    static unsigned long append_string(const char* str, int count, char append_char, char* result, unsigned long res_buf_size);
    static unsigned long align_string(const char* str, unsigned short align_mode, unsigned long align_length, char* result, unsigned long res_buf_size);
    static unsigned long trim_string(const char* str, char* result, unsigned long res_buf_size);
    static unsigned long trim_string(const char* str, char trim_char, char* result, unsigned long res_buf_size);
    static unsigned long trim_string(const char* str, const char* trim_char_set, char* result, unsigned long res_buf_size);
    static unsigned long trim_string_by_end_char(const char* str, char end_char, char* result, unsigned long res_buf_size);

    static unsigned long split_string(const char* str, char split_char, char string_array[][CONST_VALUE_260], unsigned long max_string_count);
    static unsigned long split_string_to_numbers(
        const char* str, char split_char, int radix, unsigned long number_array[], unsigned long max_number_count);
    static unsigned long split_and_find_string(
        const char* str, char split_char, const char* sub_str,
        char string_array[][CONST_VALUE_260] = NULL, unsigned long max_string_count = 0, unsigned long* string_count = NULL);

    static unsigned long two_null_end_unicode_string_to_ascii(const wchar_t* unicode_string, char* ascii_string, unsigned long buf_size);
    static unsigned long analyse_two_null_end_ascii_string(const char* two_null_end_str, char* str_array[], unsigned long max_string_count);
    static unsigned long analyse_two_null_end_unicode_string(const wchar_t* two_null_end_str, wchar_t* str_array[], unsigned long max_string_count);
    static unsigned long get_keys_from_two_null_end_ascii_string(const char* two_null_end_str, char key_array[][CONST_VALUE_260], unsigned long max_count, char val_array[][CONST_VALUE_260] = NULL);
    static unsigned long get_keys_from_two_null_end_unicode_string(const wchar_t* two_null_end_str, char key_array[][CONST_VALUE_260], unsigned long max_count, char val_array[][CONST_VALUE_260] = NULL);
    static bool get_key_value_from_key_value_ascii_string(
        const char* key_value_str, char* key, unsigned int key_buf_size, char* value, unsigned int value_buf_size, const char split_char = '=');
    static bool get_key_value_from_key_value_unicode_string(
        const wchar_t* key_value_str, char* key, unsigned int key_buf_size, char* value, unsigned int value_buf_size, const char split_char = '=');
    static bool get_value_from_two_null_end_ascii_string_by_key(const char* two_null_end_str, const char* key, char* value, unsigned int value_buf_size);
    static bool get_value_from_two_null_end_unicode_string_by_key(const wchar_t* two_null_end_str, const char* key, char* value, unsigned int value_buf_size);
    static bool systime_to_string(LPSYSTEMTIME sys_time, const char* format, char* result, unsigned int res_buf_size);

#ifdef WIN32
    static unsigned long long systime_to_quadpart(LPSYSTEMTIME sys_time = NULL);
    static unsigned long long filetime_to_quadpart(FILETIME file_time);
    static long long get_time_diff(SYSTEMTIME sys_time, SYSTEMTIME base_sys_time, unsigned long long unit);
    static long long get_time_diff(FILETIME file_time, FILETIME base_file_time, unsigned long long unit);
    static long long get_time_diff(SYSTEMTIME sys_time, FILETIME base_file_time, unsigned long long unit);
    static long long get_time_diff(FILETIME file_time, SYSTEMTIME base_sys_time, unsigned long long unit);
#else
    static bool local_time_to_system_time(LPSYSTEMTIME system_time, struct tm* local_time = NULL);
#endif // WIN32

    static bool local_time_to_string(struct tm* local_time, const char* format, char* result, unsigned int res_buf_size);
    static bool ctime_to_string(time_t* c_time, const char* format, char* result, unsigned int res_buf_size);
    static long long get_time_diff(time_t c_time, time_t base_c_time, unsigned long long unit);
    static long long get_time_diff(struct tm local_time, struct tm base_local_time, unsigned long long unit);
    static long long get_time_diff(struct tm local_time, time_t base_c_time, unsigned long long unit);
    static long long get_time_diff(time_t c_time, struct tm base_local_time, unsigned long long unit);

    static size_t d_mbstowcs(wchar_t *dest, const char *src, size_t n);
    static size_t d_wcstombs(char *dest, const wchar_t *src, size_t n);
    static size_t code_convert(char *from_charset, char *to_charset, const char *inbuf, int inlen, char *outbuf, int outlen);
    static double str2number(char *str);                                                        // 数字字串转换为整形浮点型(第一位为“-”转为负数)
    static DWORD Ascii2Hex(LPCSTR lpcAscii, DWORD dwAsciiSize, LPSTR lpHex, DWORD dwHexSize);   // ASCII字串转换为16进制字串
    static DWORD Hex2Ascii(LPCSTR lpcHex, DWORD dwHexSize, LPSTR lpAscii, DWORD dwAsciiSize);   // 16进制字串转换为ASCII字串
    static DWORD Int_To_HexStr(LONG lSource, LPSTR lpDest, DWORD dwDestSize);                   // 整型转换为16进制字符串
    static DWORD str_to_toupper(LPCSTR lpcSource, DWORD dwSourceSize, LPSTR lpDest, DWORD dwDestSize);// 字串转换为大写
    static BOOL ChkDataIsUTF8(LPCSTR lpcData, DWORD dwDataSize);                                // 检查数据编码格式
};

#endif // DATA_CONVERTOR_H
