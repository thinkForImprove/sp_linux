#include "framework.h"
#include "file_access.h"


/**
 @功能：	数值写入INI
 @参数：	app_name：block名称					key_name：Key名称
 @			value：写入值						file_path：INI文件路径
 @返回：	true：成功		false：失败
 */
bool INIFile::write_ini_file(const char* app_name, const char* key_name, unsigned long value, const char* file_path)
{
    bool ret = false;
    char local_value[CONST_VALUE_16] = { 0 };

    snprintf(local_value, sizeof(local_value), "%lu", value);
#ifdef WIN32
    ret = (WritePrivateProfileString(app_name, key_name, local_value, file_path) != FALSE);
    if (ret == false) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "WritePrivateProfileString", ret, true, file_path);
    }
#else
    QSettings ini_file(file_path, QSettings::IniFormat);
    ini_file.setIniCodec("UTF8");             // 设置编码方式
    ini_file.beginGroup(app_name);            // 设置节名
    ini_file.setValue(key_name, local_value); // 设置键名及写入
    ini_file.endGroup();                      // 结束当前节的操作
    ret = true;
#endif

    return ret;
}

/**
 @功能：	字符串写入INI
 @参数：	app_name：block名称					key_name：Key名称
 @			value：写入字符串					file_path：INI文件路径
 @返回：	true：成功		false：失败
 */
bool INIFile::write_ini_file(const char* app_name, const char* key_name, const char* content, const char* file_path)
{
	bool ret = false;
	const char* local_value = content;

	if (local_value == NULL) {
		local_value = "";
	}
#ifdef WIN32
	ret = (WritePrivateProfileString(app_name, key_name, local_value, file_path) != FALSE);
	if (ret == false) {
		REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "WritePrivateProfileString", ret, true, file_path);
	}
#else
    QSettings m_IniFile(file_path, QSettings::IniFormat);
    m_IniFile.setIniCodec("UTF8");             // 设置编码方式
    m_IniFile.beginGroup(app_name);            // 设置节名
    m_IniFile.setValue(key_name, local_value); // 设置键名及写入
    m_IniFile.endGroup();                      // 结束当前节的操作
    ret = true;
#endif

    return ret;
}

/**
 @功能：	INI中读数值
 @参数：	app_name：block名称					key_name：Key名称
 @		def_value：未找到键名时缺省键值			file_path：INI文件路径
 @返回：	键值
 */
unsigned long INIFile::read_ini_file(const char* app_name, const char* key_name, const unsigned long def_value, const char* file_path)
{
    unsigned int value = 0;
    QSettings m_IniFile(file_path, QSettings::IniFormat);
    m_IniFile.setIniCodec("UTF8");                  // 设置编码方式
    m_IniFile.beginGroup(app_name);                 // 设置节名
    if (m_IniFile.contains(key_name) != true) {     // 键名不存在
        return def_value;
    }
    value = (unsigned long)m_IniFile.value(key_name).toInt(); // 读键值
    m_IniFile.endGroup();                           // 结束当前节的操作
    return value;
}

/**
 @功能：	INI中读字符串
 @参数：	app_name：block名称					key_name：Key名称
 @		content：返回字串buff					cont_size：返回字串buff长度
 @      file_path：INI文件路径
 @返回：	键值字符串长度
 */
unsigned long INIFile::read_ini_file(const char* app_name, const char* key_name, const char* def_content, char* content, unsigned long cont_size, const char* file_path)
{
    unsigned long ulSize = 0;
    if (content == NULL || cont_size == 0) {
        return 0;
    }

    QSettings m_IniFile(file_path, QSettings::IniFormat);
    m_IniFile.setIniCodec("UTF8");             // 设置编码方式
    m_IniFile.beginGroup(app_name);            // 设置节名
    if (m_IniFile.contains(key_name) != true) {     // 键名不存在
        if (def_content == NULL) {
            content = (char*)def_content;
            return 0;
        }
        strcpy(content, def_content);
        return strlen(def_content);
    }
    QString m_str = m_IniFile.value(key_name).toString(); // 读键值
    ulSize = ((unsigned long)m_str.length() < cont_size ? (unsigned long)m_str.length() : cont_size);
    memset(content, 0x00, cont_size);
    memcpy(content, m_str.toStdString().c_str(), ulSize);
    m_IniFile.endGroup();                      // 结束当前节的操作
    return ulSize;
}

/**
 @功能：	取得INI文件中Block名称列表
 @参数：	file_path：INI文件路径					secion_array：存放结果的列表
 @			max_count：存放结果最大个数
 @返回：	Block名称个数（secion_array=NULL or max_count=0 返回实际个数）
 */
unsigned long INIFile::get_ini_sections(const char* file_path, char secion_array[][CONST_VALUE_260], unsigned long max_count)
{
	unsigned long count = 0, idx = 0;
#ifdef WIN32
	char* sec = NULL;
	unsigned long sec_size = CONST_VALUE_512 * (CONST_VALUE_260 + 1) + 2;
	char* str_ptr[CONST_VALUE_512] = { 0 };

	sec = new char[sec_size];
	if (sec == NULL) {
		return 0;
	}
	memset(sec, 0x00, sec_size);

	count = GetPrivateProfileString(NULL, NULL, "", sec, sec_size, file_path);
    if (count == 0) {
        delete[] sec;
        return 0;
    }
    count = DataConvertor::analyse_two_null_end_ascii_string(sec, str_ptr, CONST_VALUE_512);
    if (count == 0) {
        delete[] sec;
        return 0;
    }
    if ((secion_array == NULL) || (max_count == 0)) {
        delete[] sec;
        return count;
    }
    for (idx = 0; idx < count; idx++) {
        if (idx >= max_count) {
            break;
        }
        strncpy(secion_array[idx], str_ptr[idx], CONST_VALUE_260 - 1);
    }

    delete[] sec;
#else
    QSettings ini_file(file_path, QSettings::IniFormat);
    ini_file.setIniCodec("UTF8");               // 设置编码方式
    QStringList block_list = ini_file.childGroups();
    count = block_list.size();
    if ((count == 0) ||
            (secion_array == NULL) || (max_count == 0)) {
        ini_file.endGroup();
        return count;
    }
    for (idx = 0; idx < count; idx++) {
        if (idx >= max_count) {
            break;
        }
        strncpy(secion_array[idx], block_list.at(idx).toStdString().c_str(), CONST_VALUE_260 - 1);
    }
    ini_file.endGroup();
#endif

	return min(count, max_count);
}

/**
 @功能：	取得INI文件中指定Block下Key名称列表
 @参数：	file_path：INI文件路径					app_name：Block名称
 @			key_array：存放结果的列表				max_count：存放结果最大个数
 @返回：	Key名称个数（key_array=NULL or max_count=0 返回实际个数）
 */
unsigned long INIFile::get_ini_keys(const char* file_path, const char* app_name, char key_array[][CONST_VALUE_260], unsigned long max_count)
{
	unsigned long count = 0, idx = 0;
#ifdef WIN32
	char* key = NULL;
	unsigned long key_size = CONST_VALUE_512 * (CONST_VALUE_260 + 1) + 2;
	char* str_ptr[CONST_VALUE_512] = { 0 };

	if ((app_name == NULL) || (strlen(app_name) == 0)) {
		return 0;
	}

	key = new char[key_size];
	if (key == NULL) {
		return 0;
	}
	memset(key, 0x00, key_size);

    count = GetPrivateProfileString(app_name, NULL, "", key, key_size, file_path);
    if (count == 0) {
        delete[] key;
        return 0;
    }
    count = DataConvertor::analyse_two_null_end_ascii_string(key, str_ptr, CONST_VALUE_512);
    if (count == 0) {
        delete[] key;
        return 0;
    }
    if ((key_array == NULL) || (max_count == 0)) {
        delete[] key;
        return count;
    }
    for (idx = 0; idx < count; idx++) {
        if (idx >= max_count) {
            break;
        }
        strncpy(key_array[idx], str_ptr[idx], CONST_VALUE_260 - 1);
    }

    delete[] key;
#else
    QSettings ini_file(file_path, QSettings::IniFormat);
    ini_file.setIniCodec("UTF8");               // 设置编码方式
    ini_file.beginGroup(app_name);
    QStringList key_list = ini_file.childKeys();
    count = key_list.size();
    if ((count == 0) ||
            (key_array == NULL) || (max_count == 0)) {
        ini_file.endGroup();
        return count;
    }
    for (idx = 0; idx < count; idx++) {
        if (idx >= max_count) {
            break;
        }
        strncpy(key_array[idx], key_list.at(idx).toStdString().c_str(), CONST_VALUE_260 - 1);
    }
    ini_file.endGroup();
#endif

	return min(count, max_count);
}
