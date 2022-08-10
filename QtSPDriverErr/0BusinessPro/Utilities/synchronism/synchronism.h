#ifndef SYNCHRONISM_H
#define SYNCHRONISM_H


// Event等待结果
#define WAIT_OBJ_SUCCESS					((unsigned long)0)
#define WAIT_OBJ_ABNORMAL					((unsigned long)0x80)
#define WAIT_OBJ_TIMEOUT					((unsigned long)0x102)
#define WAIT_OBJ_FAIL						((unsigned long)-1)

// MultiEvent类管理Event最大个数
#define	MAX_EVENT_COUNT						(512)


#include <map>
#include <stdarg.h>
#include <fcntl.h>
#include <semaphore.h>
#include <string.h>
#include "synchronism_global.h"
#include "__common_version_def.h"
#include "__common_constant_def.h"
#include "__common_os_compatible_def.h"
#include "__common_operation_def.h"
#include "waitable_event.h"

using namespace std;


/****************************************************************************************************
 @功能：	线程间 / 进程间 同步锁
 @参数：	lock_name：锁名称必须唯一
 ****************************************************************************************************/
class SYNCHRONISM_EXPORT SyncLock
{
DEFINE_STATIC_VERSION_FUNCTIONS("synchronism", "0.0.0.0", TYPE_STATIC)

public:
    SyncLock();
    virtual ~SyncLock();

public:
    bool set_lock();
    bool release_lock();
    bool is_locked();

public:
    static void set_global_lock(const char* lock_name);
    static void release_global_lock(const char* lock_name);
    static void wait_global_lock(const char* lock_name);
    static bool is_global_locked(const char* lock_name);


protected:
    sem_t m_sem_lock;
};


/****************************************************************************************************
 @功能：	Mutex锁，构造时阻塞析构时释放
 @参数：	mutex_name：锁名称必须唯一
 @			suffix：名称后缀
 ****************************************************************************************************/
class SYNCHRONISM_EXPORT SetupMutex
{
DEFINE_STATIC_VERSION_FUNCTIONS("synchronism", "0.0.0.0", TYPE_STATIC)

public:
    SetupMutex(const char* mutex_name)
    {
        m_sem = SEM_FAILED;
        memset(m_mutex_name, 0x00, sizeof(m_mutex_name));

        if ((mutex_name == NULL) || (strlen(mutex_name) == 0)) {
            return;
        }

        if ((m_sem = sem_open(mutex_name, O_CREAT, 0644, 1)) != SEM_FAILED) {
            strcpy(m_mutex_name, mutex_name);
            sem_wait(m_sem);
        }
    }

    SetupMutex(const char* mutex_name, const char* suffix)
    {
        char local_mutex_name[CONST_VALUE_260] = { 0 };

        m_sem = SEM_FAILED;
        memset(m_mutex_name, 0x00, sizeof(m_mutex_name));
        if ((mutex_name == NULL) || (strlen(mutex_name) == 0)) {
            return;
        }
        strncpy(local_mutex_name, mutex_name, sizeof(local_mutex_name) - 1);
        if ((suffix != NULL) && (strlen(suffix) > 0)) {
            strcat(local_mutex_name, suffix);
        }

        if ((m_sem = sem_open(local_mutex_name, O_CREAT, 0644, 1)) != SEM_FAILED) {
            strcpy(m_mutex_name, local_mutex_name);
            sem_wait(m_sem);
        }
    }

    ~SetupMutex(void)
    {
        if (m_sem != SEM_FAILED)
        {
            sem_post(m_sem);
            sem_close(m_sem);
            sem_unlink(m_mutex_name);
        }
    }

private:
    sem_t* m_sem;
    char m_mutex_name[CONST_VALUE_260];
};


/****************************************************************************************************
 @功能：	单一Event锁，提供常用组合方法
 @参数：	event_name：锁名称必须唯一
 ****************************************************************************************************/
class SYNCHRONISM_EXPORT SingleEvent
{
DEFINE_STATIC_VERSION_FUNCTIONS("synchronism", "0.0.0.0", TYPE_STATIC)

public:
    SingleEvent(void);
    SingleEvent(const char* event_name, bool initial);
    virtual ~SingleEvent(void);

    int get_error() { return m_last_error; }
    bool create_event(const char* event_name, bool initial);
    bool open_event(const char* event_name);
    bool set_event();
    bool reset_event();
    unsigned long wait_single(unsigned long milli_second = INFINITE);
    void close_event();
    void close_named_sem();
    bool open_set(const char* event_name);
    unsigned long open_wait(const char* event_name, unsigned long milli_second = INFINITE);
    bool create_set(const char* event_name = NULL, bool initial = false);
    unsigned long create_wait(const char* event_name = NULL, unsigned long milli_second = INFINITE);
    unsigned long wait_close(unsigned long milli_second = INFINITE);
    void set_close();


private:
    sem_t m_sem_instance;
    sem_t* m_sem;
    char m_sem_name[CONST_VALUE_260];
    int m_last_error;
    bool m_is_named_event;
};


/****************************************************************************************************
 @功能：	多Event锁，提供常用组合方法
 @参数：	event_name：锁名称必须唯一
 ****************************************************************************************************/
class SYNCHRONISM_EXPORT MultiEvent
{
DEFINE_STATIC_VERSION_FUNCTIONS("synchronism", "0.0.0.0", TYPE_DYNAMIC)

public:
    MultiEvent();
    virtual ~MultiEvent();

    int get_error() { return m_last_error; }
    void clear_events();
    bool add_event(int event_index, bool manual = false, bool initial = false);
    bool add_event(int* event_index_array, int event_count, bool manual = false, bool initial = false);
    bool add_index_based_event(int base_index, int event_count, bool manual = false, bool initial = false);
    bool add_zero_based_event(int event_count, bool manual = false, bool initial = false);
    bool set_event(int event_index);
    bool set_all_event();
    bool reset_event(int event_index);
    bool reset_all_event();
    unsigned long wait_multi(int* waited_index, unsigned long milli_second, bool need_wait_all, int* event_index_array, int event_count);
    unsigned long wait_multi(int* waited_index, unsigned long milli_second, bool need_wait_all, int event_count, ...);
    unsigned long wait_multi(int* waited_index, unsigned long milli_second, bool need_wait_all);


private:
    map<int, neosmart_event_t> m_event_map;
    int m_last_error;
};


#endif // SYNCHRONISM_H
