#ifndef MEMORY_BUFFER_H
#define MEMORY_BUFFER_H

#include <vector>
#include "memory_buffer_global.h"
#include "__common_version_def.h"
#include "__common_os_compatible_def.h"

using namespace std;


#define MAX_MAPPING_NAME_SIZE					(260)


/****************************************************************************************************
 @功能：	管理堆内存（new出来的内存）的申请和释放等
 @参数：
 ****************************************************************************************************/
class MEMORY_BUFFER_EXPORT HeapBuffer
{
DEFINE_STATIC_VERSION_FUNCTIONS("memory_buffer", "0.0.0.0", TYPE_DYNAMIC)

public:
    HeapBuffer();
    virtual ~HeapBuffer();

    void release_buffer();
    void* alloc_buffer(unsigned long buffer_size, unsigned char fill_byte = 0x00);
    void* alloc_buffer(unsigned long buffer_size, unsigned char* buffer_data, unsigned long data_size = (unsigned long)-1);
    void* alloc_buffer(const char* string_data);
    void* alloc_buffer(const wchar_t* wstring_data);
//#ifdef WIN32
    void* alloc_string_to_buffer_for_unicode(const char* string_data);
//#endif // WIN32


protected:
    vector<void*> m_heap_vec;
};


/****************************************************************************************************
 @功能：	管理共享内存的申请和释放等
 @参数：
 ****************************************************************************************************/
class MEMORY_BUFFER_EXPORT SharedBuffer
{
DEFINE_STATIC_VERSION_FUNCTIONS("memory_buffer", "0.0.0.0", TYPE_STATIC)

public:
    SharedBuffer(void);
    virtual ~SharedBuffer(void);

    bool destroy_shared_memory();
    bool create_or_open_shared_memory(
        const char* mapping_name, unsigned long long total_size = 0, bool* is_creator = NULL);
    bool create_or_open_shared_memory(
        const key_t mapping_key, unsigned long long total_size = 0, bool* is_creator = NULL);
    bool open_shared_memory(const char* mapping_name);
    bool open_shared_memory(const key_t mapping_key);
    bool is_opened(const char* mapping_name = NULL);
    bool is_opened(const key_t mapping_key);
    bool read_buffer(unsigned long long offset, void* data_buffer, unsigned long buffer_size);
    bool write_buffer(unsigned long long offset, void* data_buffer, unsigned long buffer_size);
    bool clear_buffer(unsigned long long offset, unsigned long buffer_size, unsigned char fill_byte = 0x00);


protected:
    char m_mapping_name[MAX_MAPPING_NAME_SIZE];
    void* m_mapping_handle;
    key_t   m_mapping_key;
    int     m_mapping_id;
};

#endif // MEMORY_BUFFER_H
