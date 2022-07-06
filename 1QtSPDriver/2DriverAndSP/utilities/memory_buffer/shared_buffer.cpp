#include "framework.h"
#include "memory_buffer.h"


SharedBuffer::SharedBuffer(void)
{
	memset(m_mapping_name, 0x00, sizeof(m_mapping_name));
    m_mapping_handle = NULL;
    m_mapping_key = 0;
    m_mapping_id = 0;
}

SharedBuffer::~SharedBuffer(void)
{
	destroy_shared_memory();
}

/**
 @功能：	销毁共享内存
 @参数：	无
 @返回：	true：作为创建者成功销毁		false：作为使用者不能销毁
 */
bool SharedBuffer::destroy_shared_memory()
{
    if (m_mapping_handle != NULL) {     // 分离共享内存指针
        shmdt(m_mapping_handle);
        m_mapping_handle = NULL;
    }
    if (shmctl(m_mapping_key, IPC_RMID, NULL) == 0) {
        memset(m_mapping_name, 0x00, sizeof(m_mapping_name));
        m_mapping_key = -1;
        m_mapping_id = -1;
    } else {
        if (errno == EIDRM || errno == ENOENT) {    // shmctl执行前已销毁/参数key所指的共享内存不存在
            return true;
        } else
        if (errno == EACCES) {  // 没有权限
            return false;
        }
    }

	return true;
}

/**
 @功能：	创建或打开共享内存
 @参数：	mapping_name：名称		total_size：内存大小
 @		is_creator：输出（false：打开	true：创建）
 @返回：	true：成功		false：失败
 */
bool SharedBuffer::create_or_open_shared_memory(
	const char* mapping_name, unsigned long long total_size/* = 0*/, bool* is_creator/* = NULL*/)
{
    key_t   map_key = -1;
    if ((mapping_name == NULL) || (strlen(mapping_name) == 0)) {
        return false;
    }
    if ((map_key = ftok(mapping_name, 99)) == -1) {
        return false;
    }
    if(create_or_open_shared_memory(map_key, total_size, is_creator) != true) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "create_or_open_shared_memory", m_mapping_handle, true, mapping_name);
        return false;
    }
    strncpy(m_mapping_name, mapping_name, sizeof(m_mapping_name) - 1);

    return true;
}

/**
 @功能：	创建或打开共享内存
 @参数：	mapping_key：mapKey		total_size：内存大小
 @		is_creator：输出（false：打开	true：创建）
 @返回：	true：成功		false：失败
 */
bool SharedBuffer::create_or_open_shared_memory(
    const key_t mapping_key, unsigned long long total_size/* = 0*/, bool* is_creator/* = NULL*/)
{
    int mapping_id = -1;
    unsigned long long actual_size = 0;

    if (m_mapping_handle != NULL || m_mapping_id >= 0) {
        destroy_shared_memory();
    }

    // 根据内存页size修正申请共享内存size
    if ((total_size % ((unsigned long long)getpagesize())) == 0) {
        actual_size = total_size;
    } else {
        actual_size = ((total_size / ((unsigned long long)getpagesize())) + 1) * getpagesize();
    }

    // 指定key的共享内存已存在
    if (shmget(mapping_key, actual_size, IPC_CREAT | IPC_EXCL | 0666) == -1 && errno == EEXIST) {
        if((mapping_id = shmget(mapping_key, actual_size, IPC_CREAT | 0666)) == -1) {
            REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "mapping_key", m_mapping_handle, true, m_mapping_name);
            return false;
        }
        if (is_creator != NULL) {
            *is_creator = false;
        }
        m_mapping_id = mapping_id;
        m_mapping_key = mapping_key;
    } else {
        // 创建新共享内存
        if (total_size == 0) {
            return false;
        }
        if (shmget(mapping_key, actual_size, IPC_CREAT | IPC_EXCL | 0666) == -1) {
            REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "mapping_key", m_mapping_handle, true, m_mapping_name);
            return false;
        }
        if (is_creator != NULL) {
            *is_creator = true;
        }
        m_mapping_id = mapping_id;
        m_mapping_key = mapping_key;
    }
    //m_mapping_handle = shmat(m_mapping_id, NULL, 0);    // 挂载共享内存到指针
    //if (m_mapping_handle == NULL) {
    //    REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "mapping_key", m_mapping_handle, true, mapping_key);
    //    return false;
    //}

    return true;
}

/**
 @功能：	打开已存在的共享内存
 @参数：	mapping_name：名称
 @返回：	true：成功		false：失败
 */
bool SharedBuffer::open_shared_memory(const char* mapping_name)
{
    key_t   map_key = -1;

    if ((mapping_name == NULL) || (strlen(mapping_name) == 0)) {
        return false;
    }

    if ((map_key = ftok(mapping_name, 99)) == -1) {
        return false;
    }

    return open_shared_memory(map_key);
}

/**
 @功能：	打开已存在的共享内存
 @参数：	mapping_key：mapKey
 @返回：	true：成功		false：失败
 */
bool SharedBuffer::open_shared_memory(const key_t mapping_key)
{
    int     mapping_id = -1;
    // 指定key的共享内存已存在
    if (shmget(mapping_key, 0, IPC_CREAT | IPC_EXCL | 0666) == -1 && errno == EEXIST) {
        if((mapping_id = shmget(mapping_key, 0, IPC_CREAT | 0666)) == -1) {
            return false;
        }
        m_mapping_id = mapping_id;
        m_mapping_key = mapping_key;
        //if (m_mapping_handle == NULL) {
        //    m_mapping_handle = shmat(m_mapping_id, NULL, 0);    // 挂载共享内存到指针
        //    if (m_mapping_handle == NULL) {
        //        return false;
        //    }
        //}
        return true;
    } else {
        return false;
    }
    return true;
}

/**
 @功能：	是否已打开指定名称的共享内存
 @参数：	mapping_name：名称（NULL：忽略）
 @返回：	true：已打开		false：未打开
 */
bool SharedBuffer::is_opened(const char* mapping_name/* = NULL*/)
{
    //if (m_mapping_handle == NULL) {
    //	return false;
    //}

	if ((mapping_name != NULL) && (strlen(mapping_name) > 0)) {
		if (strcmp(m_mapping_name, mapping_name) != 0) {
			return false;
		}
    }

	return true;
}

/**
 @功能：	是否已打开指定key的共享内存
 @参数：	mapping_key：key
 @返回：	true：已打开		false：未打开
 */
bool SharedBuffer::is_opened(const key_t mapping_key)
{
    if (m_mapping_handle == NULL) {
        return false;
    }

    if (mapping_key < 0 || m_mapping_key != mapping_key) {
        return false;
    }

    return true;
}

/**
 @功能：	从共享内存指定位置读取数据的拷贝
 @参数：	offset：偏移量		data_buffer：数据缓冲区地址
 @			buffer_size：数据缓冲区大小
 @返回：	true：成功		false：失败
 */
bool SharedBuffer::read_buffer(unsigned long long offset, void* data_buffer, unsigned long buffer_size)
{
    if ((data_buffer == NULL) || (buffer_size == 0)) {
        return false;
    }
    m_mapping_handle = shmat(m_mapping_id, NULL, 0);    // 挂载共享内存到指针
    if (m_mapping_handle == NULL) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "shmat", m_mapping_handle, true, m_mapping_name)
        return false;
    }
    memcpy((char*)m_mapping_handle + offset, data_buffer, buffer_size);
    shmdt(m_mapping_handle);    // 分离共享内存和读写指针
    m_mapping_handle = NULL;

	return true;
}

/**
 @功能：	向共享内存指定位置写入数据
 @参数：	offset：偏移量		data_buffer：数据缓冲区地址
 @			buffer_size：数据缓冲区大小
 @返回：	true：成功		false：失败
 */
bool SharedBuffer::write_buffer(unsigned long long offset, void* data_buffer, unsigned long buffer_size)
{
    if ((data_buffer == NULL) || (buffer_size == 0)) {
        return false;
    }
    m_mapping_handle = shmat(m_mapping_id, NULL, 0);    // 挂载共享内存到指针
    if (m_mapping_handle == NULL) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "shmat", m_mapping_handle, true, m_mapping_name)
        return false;
    }
    memcpy(data_buffer, (char*)m_mapping_handle + offset, buffer_size);
    shmdt(m_mapping_handle);    // 分离共享内存和读写指针
    m_mapping_handle = NULL;

	return true;
}

/**
 @功能：	向共享内存指定位置写入相同数据
 @参数：	offset：偏移量		buffer_size：数据缓冲区大小
 @		fill_byte：单字节数据
 @返回：	true：成功		false：失败
 */
bool SharedBuffer::clear_buffer(unsigned long long offset, unsigned long buffer_size, unsigned char fill_byte/* = 0x00*/)
{
    unsigned char* start_ptr = NULL;
    if (buffer_size == 0) {
        return false;
    }
    m_mapping_handle = shmat(m_mapping_id, NULL, 0);    // 挂载共享内存到指针
    if (m_mapping_handle == NULL) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "shmat", m_mapping_handle, true, m_mapping_name)
        return false;
    }
    start_ptr = new unsigned char [buffer_size];
    memset(start_ptr, fill_byte, buffer_size);
    memcpy(start_ptr, (char*)m_mapping_handle + offset, buffer_size);
    delete[] start_ptr;
    start_ptr = NULL;
    shmdt(m_mapping_handle);    // 分离共享内存和读写指针
    m_mapping_handle = NULL;

	return true;
}
