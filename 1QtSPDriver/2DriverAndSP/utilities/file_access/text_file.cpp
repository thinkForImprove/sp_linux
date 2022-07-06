#include "framework.h"
#include "file_access.h"


#define LINE_UTF8_TO_ASCII(FileType, LineString) \
{\
	char* ps_line = NULL;\
	unsigned long len = 0;\
	int local_file_type = FileType;\
	if (local_file_type == FILE_TYPE_UTF8) {\
		len = DataConvertor::string_utf8_to_ascii(LineString.c_str(), NULL, 0);\
		if (len > 0) {\
			ps_line = new char[len];\
			if (ps_line) {\
				memset(ps_line, 0x00, sizeof(char) * len);\
				DataConvertor::string_utf8_to_ascii(LineString.c_str(), ps_line, len);\
				LineString.clear();\
				LineString = string(ps_line);\
				delete[] ps_line;\
				ps_line = NULL;\
			} else {\
				LineString.clear();\
			}\
		}\
	}\
}


TextFile::TextFile()
{
	m_file_type = FILE_TYPE_UNKOWN;
}

TextFile::~TextFile()
{
	m_file_type = FILE_TYPE_UNKOWN;
	m_file_info_vec.clear();
}


/**
 @功能：	载入指定文件按行存储为列表（ASCII、UNICODE、UTF-8）
 @参数：	file_path：文件路径					file_type：文件编码（FILE_TYPE_UNKOWN：自动识别）
 @返回：	文件行数
 */
unsigned long TextFile::load_text_file(const char* file_path, int file_type/* = FILE_TYPE_UNKOWN*/)
{
#ifdef WIN32
    UNREFERENCED_PARAMETER(file_type);
#else
    Q_UNUSED(file_type);
#endif

	FILE* fp = NULL;
	string str_line = "";
	int get_char = -1;
	char c = '\0';
	char cc[3] = { 0 };
	bool lf_char = false;
	bool line_ignore = false;

	if ((file_path == NULL) || (strlen(file_path) == 0)) {
		return 0;
	}

#ifdef WIN32
	if ((fp = fopen(file_path, "rb")) == NULL) {
        Sleep(50);
		if ((fp = fopen(file_path, "rb")) == NULL) {
			Sleep(50);
			if ((fp = fopen(file_path, "rb")) == NULL) {
				if (GetLastError() != ERROR_FILE_NOT_FOUND) {
					REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", fp, true, file_path);
				}
				return 0;
			}
		}
	}
#else
    if ((fp = fopen(file_path, "rb")) == NULL) {
        sleep(50);
        if ((fp = fopen(file_path, "rb")) == NULL) {
            sleep(50);
            if ((fp = fopen(file_path, "rb")) == NULL) {
                if (errno != ENOENT) {
                    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", fp, true, file_path);
                }
                return 0;
            }
        }
    }
#endif
	// UNICODE
	if ((fread(cc, 2, 1, fp) == 1) &&
		(((unsigned char)(cc[0]) == 0xFF) && ((unsigned char)(cc[1]) == 0xFE))) {
		fclose(fp);
		m_file_type = FILE_TYPE_UNICODE;
		return load_text_file_for_unicode(file_path);
	}
	// UTF-8
	fseek(fp, 0, SEEK_SET);
	if ((fread(cc, 3, 1, fp) == 1) &&
		(((unsigned char)(cc[0]) == 0xEF) && ((unsigned char)(cc[1]) == 0xBB) && ((unsigned char)(cc[2]) == 0xBF))) {
		m_file_type = FILE_TYPE_UTF8;
	} else {
		m_file_type = FILE_TYPE_ASCII;
		fseek(fp, 0, SEEK_SET);
	}

	m_file_info_vec.clear();
	while (feof(fp) == 0) {
		get_char = fgetc(fp);
		if (get_char == -1) {
			break;
		}

		c = (char)get_char;
		switch (c) {
		case '\0':
			line_ignore = true;
			lf_char = false;
			LINE_UTF8_TO_ASCII(m_file_type, str_line)
			m_file_info_vec.push_back(str_line);
			str_line.clear();
			break;
		case '\r':
			if (lf_char == false) {
				lf_char = true;
			} else {
				lf_char = false;
				if (line_ignore == false) {
					LINE_UTF8_TO_ASCII(m_file_type, str_line)
					m_file_info_vec.push_back(str_line);
					str_line.clear();
				}
				line_ignore = false;
			}
			break;
		case '\n':
			lf_char = false;
			if (line_ignore == false) {
				LINE_UTF8_TO_ASCII(m_file_type, str_line)
				m_file_info_vec.push_back(str_line);
				str_line.clear();
			}
			line_ignore = false;
			break;
		default:
			if (line_ignore == false) {
				if (lf_char) {
					lf_char = false;
					LINE_UTF8_TO_ASCII(m_file_type, str_line)
					m_file_info_vec.push_back(str_line);
					str_line.clear();
				}
				str_line.push_back(c);
			}
			break;
		}
	}
	if (str_line.empty() == false) {
		LINE_UTF8_TO_ASCII(m_file_type, str_line)
		m_file_info_vec.push_back(str_line);
	}
	fclose(fp);

	return m_file_info_vec.size();
}

/**
 @功能：	载入指定文件按行存储为列表（UNICODE）
 @参数：	file_path：文件路径
 @返回：	文件行数
 */
unsigned long TextFile::load_text_file_for_unicode(const char* file_path)
{
	FILE* fp = NULL;
	string str_line = "";
	int get_char = -1;
	char c = '\0';
	char cc[2] = { 0 };
	wchar_t wc = L'\0';
	bool lf_char = false;
	bool line_ignore = false;
	int i = 0, j = 0;
	int len = 0;

	if ((file_path == NULL) || (strlen(file_path) == 0)) {
		return 0;
	}

#ifdef WIN32
	if ((fp = fopen(file_path, "rb")) == NULL) {
		Sleep(50);
		if ((fp = fopen(file_path, "rb")) == NULL) {
			Sleep(50);
			if ((fp = fopen(file_path, "rb")) == NULL) {
				REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", fp, true, file_path);
				return 0;
			}
		}
	}
#else
    if ((fp = fopen(file_path, "rb")) == NULL) {
        sleep(50);
        if ((fp = fopen(file_path, "rb")) == NULL) {
            sleep(50);
            if ((fp = fopen(file_path, "rb")) == NULL) {
                REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", fp, true, file_path);
                return 0;
            }
        }
    }
#endif
	if ((fread(cc, 2, 1, fp) != 1) ||
		(((unsigned char)(cc[0]) != 0xFF) || ((unsigned char)(cc[1]) != 0xFE))) {
		fclose(fp);
		return 0;
	}

	m_file_info_vec.clear();
	while (feof(fp) == 0) {
		get_char = fgetc(fp);
		if (get_char == -1) {
			break;
		}

		c = (char)get_char;
		((char*)(&wc))[i] = c;
		i = (i + 1) % 2;
		if (i == 1) {
			continue;
		}
		switch (wc) {
		case L'\0':
			line_ignore = true;
			lf_char = false;
			m_file_info_vec.push_back(str_line);
			str_line.clear();
			break;
		case L'\r':
			if (lf_char == false) {
				lf_char = true;
			} else {
				lf_char = false;
				if (line_ignore == false) {
					m_file_info_vec.push_back(str_line);
					str_line.clear();
				}
				line_ignore = false;
			}
			break;
		case L'\n':
			lf_char = false;
			if (line_ignore == false) {
				m_file_info_vec.push_back(str_line);
				str_line.clear();
			}
			line_ignore = false;
			break;
		default:
			if (line_ignore == false) {
				if (lf_char) {
					lf_char = false;
					m_file_info_vec.push_back(str_line);
					str_line.clear();
				}
#ifdef WIN32
				len = WideCharToMultiByte(CP_ACP, 0, &wc, 1, cc, sizeof(cc), NULL, NULL);
#else
                len = DataConvertor::d_wcstombs(cc, &wc, sizeof(cc));
#endif
				for (j = 0; j < len; j++) {
					str_line.push_back(cc[j]);
				}
			}
			break;
		}
	}
	if (str_line.empty() == false) {
		m_file_info_vec.push_back(str_line);
	}
	fclose(fp);

	return m_file_info_vec.size();
}

/**
 @功能：	取得文件行数
 @参数：	无
 @返回：	文件行数
 */
unsigned long TextFile::get_line_count()
{
	return (unsigned long)m_file_info_vec.size();
}

/**
 @功能：	取得文件中某行内容
 @参数：	str_line：内容Buffer			line_buffer_size：Buffer大小
 @			line_index：行索引（>=0）
 @返回：	true：成功		false：失败
 */
bool TextFile::get_line_text(char* str_line, unsigned int line_buffer_size, unsigned long line_index)
{
	string local_line = "";

	if ((str_line == NULL) || (line_buffer_size == 0) ||
		(line_index >= m_file_info_vec.size())) {
		return false;
	}

	local_line = m_file_info_vec.at(line_index);
	if (line_buffer_size <= local_line.length()) {
		return false;
	}

	strncpy(str_line, local_line.c_str(), line_buffer_size - 1);

	return true;
}

/**
 @功能：	删除文件中某行内容
 @参数：	line_index：行索引（>=0）
 @返回：	true：成功		false：失败
 */
bool TextFile::del_line_text(unsigned long line_index)
{
	if (line_index >= m_file_info_vec.size()) {
		return false;
	}

	m_file_info_vec.erase(m_file_info_vec.begin() + line_index);

	return true;
}

/**
 @功能：	在指定位置插入文件中一行内容
 @参数：	str_line：插入内容（NULL：空行）			line_index：行索引（>=0，-1：最后一行）
 @返回：	无
 */
void TextFile::insert_line_text(const char* str_line/* = NULL*/, unsigned long line_index/* = (unsigned long)-1*/)
{
	string local_line = "";

	if (str_line != NULL) {
		local_line = string(str_line);
	}
	if (line_index == (unsigned long)-1) {
		m_file_info_vec.push_back(local_line);
	} else {
		m_file_info_vec.insert(m_file_info_vec.begin() + line_index, local_line);
	}
}

/**
 @功能：	在指定位置更新文件中一行内容
 @参数：	str_line：更新内容			line_index：行索引（>=0）
 @返回：	true：成功		false：失败
 */
bool TextFile::update_line_text(const char* str_line, unsigned long line_index)
{
	if (line_index >= m_file_info_vec.size()) {
		return false;
	}

	if (del_line_text(line_index) == false) {
		return false;
	}
	insert_line_text(str_line, line_index);

	return true;
}

/**
 @功能：	写入文本文件
 @参数：	file_path：文件路径			file_type：编码（FILE_TYPE_UNKOWN：当前（读文件时）编码）
 @返回：	true：成功		false：失败
 */
bool TextFile::write_text_file(const char* file_path, int file_type/* = FILE_TYPE_UNKOWN*/)
{
	FILE* fp = NULL;
	string str_line = "";
	bool first_line = true;
	vector<string>::iterator it;
	wchar_t* wchar_line = NULL;
	int len = 0;
	unsigned char head[2] = { 0xFF, 0xFE };
	char* line_ptr = NULL;
	unsigned long utf_len = 0;
	unsigned char utf_head[3] = { 0xEF, 0xBB, 0xBF };

	if ((file_path == NULL) || (strlen(file_path) == 0)) {
		return false;
	}

#ifdef WIN32
	if ((fp = fopen(file_path, "w")) == NULL) {
		Sleep(50);
		if ((fp = fopen(file_path, "w")) == NULL) {
			Sleep(50);
			//SetFileAttributes(file_path, FILE_ATTRIBUTE_ARCHIVE);
			_chmod(file_path, _S_IREAD | _S_IWRITE);
			if ((fp = fopen(file_path, "w")) == NULL) {
				REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", fp, true, file_path);
				return false;
			}
		}
	}
#else
    if ((fp = fopen(file_path, "w")) == NULL) {
        sleep(50);
        if ((fp = fopen(file_path, "w")) == NULL) {
            sleep(50);
            //SetFileAttributes(file_path, FILE_ATTRIBUTE_ARCHIVE);
            chmod(file_path, S_IRUSR | S_IWUSR);  // 设置用户(所有者)读写
            if ((fp = fopen(file_path, "w")) == NULL) {
                REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "fopen", fp, true, file_path);
                return false;
            }
        }
    }
#endif

	if (file_type == FILE_TYPE_UNKOWN) {
		file_type = m_file_type;
	}

	for (it = m_file_info_vec.begin(); it != m_file_info_vec.end(); it++) {
		if (first_line == false) {
			str_line = string("\n") + *it;
		} else {
			first_line = false;
			str_line = *it;
			if (file_type == FILE_TYPE_UNICODE) {
				fwrite(head, sizeof(head), 1, fp);
			} else if (file_type == FILE_TYPE_UTF8) {
				fwrite(utf_head, sizeof(utf_head), 1, fp);
			} else {
			}
		}
		if (file_type == FILE_TYPE_UNICODE) {
#ifdef WIN32
			len = MultiByteToWideChar(CP_ACP, 0, str_line.c_str(), -1, NULL, 0);
#else
            len = DataConvertor::d_mbstowcs(NULL, str_line.c_str(), 0);
#endif
			if (len > 0) {
				wchar_line = new wchar_t[len];
				if (wchar_line != NULL) {
					memset(wchar_line, 0x00, sizeof(wchar_t) * len);
#ifdef WIN32
					len = MultiByteToWideChar(CP_ACP, 0, str_line.c_str(), -1, wchar_line, len);
#else
                    len = DataConvertor::d_mbstowcs(wchar_line, str_line.c_str(), len);
#endif
					if (len > 0) {
						fwrite(wchar_line, (len - 1) * sizeof(wchar_t), 1, fp);
					}
					delete[] wchar_line;
				}
			}
		} else if (file_type == FILE_TYPE_UTF8) {
			utf_len = DataConvertor::string_ascii_to_utf8(str_line.c_str(), NULL, 0);
			if (utf_len > 0) {
				line_ptr = new char[utf_len];
				if (line_ptr != NULL) {
					memset(line_ptr, 0x00, sizeof(char) * utf_len);
					utf_len = DataConvertor::string_ascii_to_utf8(str_line.c_str(), line_ptr, utf_len);
					if (utf_len > 0) {
						fwrite(line_ptr, (utf_len - 1) * sizeof(char), 1, fp);
					}
					delete[] line_ptr;
				}
			}
		} else {
			fwrite(str_line.c_str(), str_line.length(), 1, fp);
		}
	}

	fclose(fp);

	return true;
}
