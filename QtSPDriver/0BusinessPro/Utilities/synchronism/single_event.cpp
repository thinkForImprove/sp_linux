#include "framework.h"
#include "synchronism.h"


SingleEvent::SingleEvent(void)
{
    m_sem = SEM_FAILED;
    memset(m_sem_name, 0x00, sizeof(m_sem_name));
    m_last_error = 0;
    m_is_named_event = false;
}

SingleEvent::SingleEvent(const char* event_name, bool initial)
{
    create_event(event_name, initial);
}

SingleEvent::~SingleEvent(void)
{
    close_event();
}


/**
 @功能：	创建Event
 @参数：	event_name：Event名称		initial：初始状态是否释放
 @返回：	true：成功		false：失败
 */
bool SingleEvent::create_event(const char* event_name, bool initial)
{
    int ret = -1;
    int init_state = -1;

    m_last_error = 0;
    close_event();
    close_named_sem();

    init_state = 0;
    if (initial) {
        init_state = 1;
    }
    if ((event_name != NULL) && (strlen(event_name) > 0)) {
        m_sem = sem_open(event_name, O_CREAT, 0644, init_state);
        if (m_sem == SEM_FAILED) {
            m_last_error = errno;
            return false;
        }
        strcpy(m_sem_name, event_name);
    } else {
        ret = sem_init(&m_sem_instance, 0, init_state);
        if (ret != 0) {
            m_last_error = errno;
            return false;
        }
        m_sem = &m_sem_instance;
    }

    return (m_sem != SEM_FAILED);
}

/**
 @功能：	打开既存Event
 @参数：	event_name：Event名称
 @返回：	true：成功		false：失败
 */
bool SingleEvent::open_event(const char* event_name)
{
    m_last_error = 0;
    if ((event_name == NULL) || (strlen(event_name) == 0)) {
        return false;
    }

    m_sem = sem_open(event_name, O_EXCL);
    if (m_sem == SEM_FAILED) {
        m_last_error = errno;
        return false;
    }
    strcpy(m_sem_name, event_name);

    return (m_sem != SEM_FAILED);
}

/**
 @功能：	释放Event
 @参数：	无
 @返回：	true：成功		false：失败
 */
bool SingleEvent::set_event()
{
    int ret = 0;

    m_last_error = 0;
    if (m_sem == SEM_FAILED) {
        return false;
    }
    do {
        ret = sem_trywait(m_sem);
    } while (ret == 0);
    ret = sem_post(m_sem);
    if (ret != 0) {
        m_last_error = errno;
    }

    return (ret == 0);
}

/**
 @功能：	阻塞Event
 @参数：	无
 @返回：	true：成功		false：失败
 */
bool SingleEvent::reset_event()
{
    int ret = 0;

    m_last_error = 0;
    do {
        ret = sem_trywait(m_sem);
    } while (ret == 0);

    return true;
}

/**
 @功能：	等待Event
 @参数：	milli_second：超时时间（ms）
 @返回：	等待结果
 */
unsigned long SingleEvent::wait_single(unsigned long milli_second/* = INFINITE*/)
{
    unsigned long wait_result = WAIT_OBJ_SUCCESS;
    int ret = 0;
    timeval tv;
    timespec ts;
    __uint64_t usec = 0;

    m_last_error = 0;

    if (milli_second != INFINITE) {
        gettimeofday(&tv, NULL);
        usec = ((__uint64_t)tv.tv_sec) * 1000 * 1000 + ((__uint64_t)milli_second) * 1000 + tv.tv_usec;
        ts.tv_sec = usec / 1000 / 1000;
        ts.tv_nsec = (usec - ((__uint64_t)ts.tv_sec) * 1000 * 1000) * 1000;
        ret = sem_timedwait(m_sem, &ts);
    } else {
        ret = sem_wait(m_sem);
    }
    if (ret == 0) {
        wait_result = WAIT_OBJ_SUCCESS;
    } else {
        m_last_error = errno;
        switch (m_last_error) {
        case 0:
            wait_result = WAIT_OBJ_SUCCESS;
            break;
        case ETIMEDOUT:
            wait_result = WAIT_OBJ_TIMEOUT;
            break;
        default:
            wait_result = WAIT_OBJ_FAIL;
            break;
        }
    }

    return wait_result;
}

/**
 @功能：	关闭无名Event
 @参数：	无
 @返回：	无
 */
void SingleEvent::close_event()
{
    m_last_error = 0;
    if ((strlen(m_sem_name) > 0) || (m_sem == SEM_FAILED)) {
        return;
    }
    sem_destroy(m_sem);
    m_sem = SEM_FAILED;
}

/**
 @功能：	关闭命名Event
 @参数：	无
 @返回：	无
 */
void SingleEvent::close_named_sem()
{
    m_last_error = 0;
    if ((strlen(m_sem_name) == 0) || (m_sem == SEM_FAILED)) {
        return;
    }
    sem_close(m_sem);
    sem_unlink(m_sem_name);
    memset(m_sem_name, 0x00, sizeof(m_sem_name));
    m_sem = SEM_FAILED;
}

/**
 @功能：	打开既存Event并释放
 @参数：	event_name：Event名称
 @返回：	true：成功		false：失败
 */
bool SingleEvent::open_set(const char* event_name)
{
    if (open_event(event_name) == false) {
		return false;
	}
	return set_event();
}

/**
 @功能：	打开既存Event并等待
 @参数：	event_name：Event名称			milli_second：超时时间（ms）
 @返回：	等待结果
 */
unsigned long SingleEvent::open_wait(const char* event_name, unsigned long milli_second/* = INFINITE*/)
{
    if (open_event(event_name) == false) {
		return WAIT_OBJ_FAIL;
	}
	return wait_single(milli_second);
}

/**
 @功能：	创建Event并释放
 @参数：	event_name：Event名称			initial：初始状态是否释放
 @返回：	true：成功		false：失败
 */
bool SingleEvent::create_set(const char* event_name/* = NULL*/, bool initial/* = false*/)
{
    if (create_event(event_name, initial) == false) {
		return false;
	}
	return set_event();
}

/**
 @功能：	创建Event并等待
 @参数：	event_name：Event名称			milli_second：超时时间（ms）
 @返回：	等待结果
 */
unsigned long SingleEvent::create_wait(const char* event_name/* = NULL*/, unsigned long milli_second/* = INFINITE*/)
{
    if (create_event(event_name, false) == false) {
		return WAIT_OBJ_FAIL;
	}
	return wait_single(milli_second);
}

/**
 @功能：	等待Event并关闭
 @参数：	milli_second：超时时间（ms）
 @返回：	等待结果
 */
unsigned long SingleEvent::wait_close(unsigned long milli_second/* = INFINITE*/)
{
	unsigned long wait = 0;

	wait = wait_single(milli_second);
	close_event();

	return wait;
}

/**
 @功能：	释放Event并关闭
 @参数：	无
 @返回：	无
 */
void SingleEvent::set_close()
{
	set_event();
	close_event();
}
