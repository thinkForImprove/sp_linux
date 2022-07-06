#include "framework.h"
#include "memory_buffer.h"


HeapBuffer::HeapBuffer()
{
}

HeapBuffer::~HeapBuffer()
{
    release_buffer();
}


/**
 @功能：	释放所有内存
 @参数：	无
 @返回：	无
 */
void HeapBuffer::release_buffer()
{
    vector<void*>::iterator it;

    if (m_heap_vec.size() == 0) {
        return;
    }

    for (it = m_heap_vec.begin(); it != m_heap_vec.end(); it++) {
        if (*it != NULL) {
            delete[](*it);
        }
    }

    m_heap_vec.clear();
}

/**
 @功能：	分配指定长度内存并用指定字符填充
 @参数：	buffer_size：内存长度				fill_byte：填充字节（默认0x00）
 @返回：	分配的内存地址（NULL：失败）
 */
void* HeapBuffer::alloc_buffer(unsigned long buffer_size, unsigned char fill_byte/* = 0x00*/)
{
	void* buf_ptr = NULL;

	if (buffer_size == 0) {
		return NULL;
	}

	buf_ptr = new unsigned char[buffer_size];
	if (buf_ptr != NULL) {
		memset(buf_ptr, fill_byte, buffer_size);
		m_heap_vec.push_back(buf_ptr);
	} else {
		REPORT_APP_EVENT_API(EVENTLOG_ERROR_TYPE, get_product_name(), "new", buf_ptr, true)
	}

	return buf_ptr;
}

/**
 @功能：	分配指定长度内存并用数组数据填充
 @参数：	buffer_size：内存长度				buffer_data：数组地址（NULL：忽略）
 @			data_size：数组数据长度（0：忽略）
 @返回：	分配的内存地址（NULL：失败）
 */
void* HeapBuffer::alloc_buffer(unsigned long buffer_size, unsigned char* buffer_data, unsigned long data_size/* = (unsigned long)-1*/)
{
	void* buf_ptr = NULL;

	buf_ptr = alloc_buffer(buffer_size);
	if ((buf_ptr != NULL) && (buffer_data != NULL) && (data_size > 0)) {
		if (data_size > buffer_size) {
			data_size = buffer_size;
		}
		memcpy(buf_ptr, buffer_data, data_size);
	}

	return buf_ptr;
}

/**
 @功能：	分配内存复制字符串（ASCII编码）
 @参数：	string_data：字符串
 @返回：	分配的内存地址（NULL：失败）
 */
void* HeapBuffer::alloc_buffer(const char* string_data)
{
	void* buf_ptr = NULL;
	unsigned long data_size = 0;

	if (string_data == NULL) {
		return NULL;
	}
	data_size = strlen(string_data) + 1;
	buf_ptr = alloc_buffer(data_size, (unsigned char*)string_data, data_size);

	return buf_ptr;
}

/**
 @功能：	分配内存复制字符串（UNICODE编码）
 @参数：	wstring_data：字符串
 @返回：	分配的内存地址（NULL：失败）
 */
void* HeapBuffer::alloc_buffer(const wchar_t* wstring_data)
{
	void* buf_ptr = NULL;
	unsigned long data_size = 0;

	if (wstring_data == NULL) {
		return NULL;
	}
	data_size = (wcslen(wstring_data) + 1) * 2;
	buf_ptr = alloc_buffer(data_size, (unsigned char*)wstring_data, data_size);

	return buf_ptr;
}

/**
 @功能：	指定ASCII字符串转为UNICODE字符串，为之配内存
 @参数：	string_data：字符串
 @返回：	分配的内存地址（NULL：失败）
 */
void* HeapBuffer::alloc_string_to_buffer_for_unicode(const char* string_data)
{
	void* buf_ptr = NULL;
	int len = 0;
	wchar_t* wtring_data = NULL;

	if (string_data == NULL) {
		return NULL;
	}

#ifdef WIN32
    unsigned int code_page = 0;
	code_page = AreFileApisANSI() ? CP_ACP : CP_OEMCP;
	len = MultiByteToWideChar(code_page, 0, string_data, -1, NULL, 0);
	if (len == 0) {
		return NULL;
	}
	wtring_data = new wchar_t[len];
	if (wtring_data == NULL) {
		return 0;
	}
	len = MultiByteToWideChar(code_page, 0, string_data, -1, wtring_data, len);
	if (len == 0) {
		delete[] wtring_data;
		return NULL;
	}
#else   // LINUX
    len = DataConvertor::d_mbstowcs(NULL, string_data, 0);
    if (len == 0) {
        return NULL;
    }
    wtring_data = new wchar_t[len];
    if (wtring_data == NULL) {
        return 0;
    }
    len = DataConvertor::d_mbstowcs(wtring_data, string_data, len);
    if (len == 0) {
        delete[] wtring_data;
        return NULL;
    }
#endif // WIN32
	buf_ptr = alloc_buffer(wtring_data);
	delete[] wtring_data;

	return buf_ptr;
}


