#include "framework.h"
#include "synchronism.h"


SyncLock::SyncLock()
{
    sem_init(&m_sem_lock, 0, 1);
}

SyncLock::~SyncLock()
{
    sem_destroy(&m_sem_lock);
}


/**
 @功能：	设置锁为阻塞状态（线程）
 @参数：	无
 @返回：	true：成功		false：失败
 */
bool SyncLock::set_lock()
{
    int ret = 0;
    int nVal = -1;

    do {
        ret = sem_getvalue(&m_sem_lock, &nVal);
        if (ret != 0) {
            return false;
        }
        if (nVal > 1) {
            ret = sem_trywait(&m_sem_lock);
            if (ret != 0) {
                return false;
            }
        }
    } while (nVal > 1);

    sem_wait(&m_sem_lock);

    return true;
}

/**
 @功能：	释放锁（线程）
 @参数：	无
 @返回：	true：成功		false：失败
 */
bool SyncLock::release_lock()
{
    int ret = 0;
    int nVal = -1;

    ret = sem_getvalue(&m_sem_lock, &nVal);
    if (ret != 0) {
        return false;
    }

    if (nVal == 0) {
        sem_post(&m_sem_lock);
    }

    return (ret == 0);
}

/**
 @功能：	取得锁状态（线程）
 @参数：	无
 @返回：	true：阻塞		false：释放
 */
bool SyncLock::is_locked()
{
    int ret = 0;
    int nVal = -1;

    ret = sem_getvalue(&m_sem_lock, &nVal);
    if (ret != 0) {
        return false;
    }

    return (nVal == 0);
}


/**
 @功能：	设置锁为阻塞状态（进程）
 @参数：	lock_name：锁名称
 @返回：	无
 */
void SyncLock::set_global_lock(const char* lock_name)
{
    sem_t* sem = SEM_FAILED;

    if ((lock_name == NULL) || (strlen(lock_name) == 0)) {
        return;
    }

    if ((sem = sem_open(lock_name, O_EXCL)) == SEM_FAILED) {
        sem = sem_open(lock_name, O_CREAT, 0644, 1);
    } else {
        sem_wait(sem);
    }
}

/**
 @功能：	释放锁（进程）
 @参数：	lock_name：锁名称
 @返回：	无
 */
void SyncLock::release_global_lock(const char* lock_name)
{
    sem_t* sem = SEM_FAILED;

    if ((lock_name == NULL) || (strlen(lock_name) == 0)) {
        return;
    }

    sem = sem_open(lock_name, O_EXCL);
    if (sem != SEM_FAILED) {
        sem_post(sem);
        sem_close(sem);
        sem_unlink(lock_name);
    }
}

/**
 @功能：	等待锁（进程）
 @参数：	lock_name：锁名称
 @返回：	无
 */
void SyncLock::wait_global_lock(const char* lock_name)
{
    sem_t* sem = SEM_FAILED;

    if ((lock_name == NULL) || (strlen(lock_name) == 0)) {
        return;
    }

    sem = sem_open(lock_name, O_EXCL);
    if (sem != SEM_FAILED) {
        sem_wait(sem);
    }
}

/**
 @功能：	取得锁状态（进程）
 @参数：	lock_name：锁名称
 @返回：	true：阻塞		false：释放
 */
bool SyncLock::is_global_locked(const char* lock_name)
{
    sem_t* sem = SEM_FAILED;
    int ret = 0;
    int nVal = -1;

    if ((lock_name == NULL) || (strlen(lock_name) == 0)) {
        return false;
    }

    sem = sem_open(lock_name, O_EXCL);
    if (sem == SEM_FAILED) {
        return false;
    }

    ret = sem_getvalue(sem, &nVal);
    if (ret != 0) {
        return false;
    }

    return (nVal == 0);
}
