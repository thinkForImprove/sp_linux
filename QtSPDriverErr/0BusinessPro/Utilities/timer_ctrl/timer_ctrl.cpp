#include "framework.h"
#include "timer_ctrl.h"


TimerCtrl::TimerCtrl()
{
    m_timer_func = nullptr;
    sem_init(&m_sem, 0, 1);
}
TimerCtrl::TimerCtrl(TmerFuncType timer_func)
{
    m_timer_func = timer_func;
    sem_init(&m_sem, 0, 1);
}
TimerCtrl::~TimerCtrl(void)
{
    clear_timers();
    sem_destroy(&m_sem);
}


/**
 @功能：	设置计时器函数（线程）
 @参数：	timer_func：计时器函数
 @返回：	无
 */
void TimerCtrl::set_timer_function(TmerFuncType timer_func)
{
    m_timer_func = timer_func;
}

/**
 @功能：	清空全部计时器
 @参数：	无
 @返回：	无
 */
void TimerCtrl::clear_timers()
{
    typename std::map<unsigned int, timer_t>::iterator it;

    sem_wait(&m_sem);
    for (it = m_timer_map.begin(); it != m_timer_map.end(); it++) {
        timer_delete(it->second);
    }
    m_timer_map.clear();
    sem_post(&m_sem);
}

/**
 @功能：	启动一个指定ID的计时器
 @参数：	id：计时器ID            elapse：超时时间（ms）
 @      exec_one_time：计时器超时后是否只执行一次
 @返回：	true：成功        false：失败
 */
bool TimerCtrl::start_timer(unsigned int id, unsigned int elapse, bool exec_one_time/* = false*/)
{
    timer_t tt = 0;
    typename std::map<unsigned int, timer_t>::iterator it;

    if ((id == 0) || (elapse == 0)) {
        return false;
    }

    sem_wait(&m_sem);
    if (m_timer_map.empty() == false) {
        for (it = m_timer_map.begin(); it != m_timer_map.end(); it++) {
            if (it->first == id) {
                timer_delete(it->second);
                m_timer_map.erase(it);
                break;
            }
        }
    }

    tt = set_posix_timer(id, elapse, exec_one_time);
    if (tt == 0) {
        sem_post(&m_sem);
        return false;
    }
    m_timer_map.insert(std::map<unsigned int, timer_t>::value_type(id, tt));
    sem_post(&m_sem);

    return true;
}

/**
 @功能：	启动一个新计时器
 @参数：	id：计时器ID            elapse：超时时间（ms）
 @      exec_one_time：计时器超时后是否只执行一次
 @返回：	true：成功        false：失败
 */
bool TimerCtrl::start_timer_with_new_id(unsigned int id, unsigned int elapse, bool exec_one_time/* = false*/)
{
    timer_t tt = 0;
    typename std::map<unsigned int, timer_t>::iterator it;

    if ((id == 0) || (elapse == 0)) {
        return false;
    }

    sem_wait(&m_sem);
    if (m_timer_map.empty() == false) {
        for (it = m_timer_map.begin(); it != m_timer_map.end(); it++) {
            if (it->first == id) {
                sem_post(&m_sem);
                return false;
            }
        }
    }

    tt = set_posix_timer(id, elapse, exec_one_time);
    if (tt == 0) {
        sem_post(&m_sem);
        return false;
    }
    m_timer_map.insert(std::map<unsigned int, timer_t>::value_type(id, tt));
    sem_post(&m_sem);

    return true;
}

/**
 @功能：	启动一个指定范围内随机ID的新计时器
 @参数：	elapse：超时时间（ms）         start_id：最小ID
 @      end_id：最大ID                exec_one_time：计时器超时后是否只执行一次
 @返回：	0：失败        >0：成功
 */
unsigned int TimerCtrl::start_timer_by_random_id(
        unsigned int elapse, unsigned int start_id, unsigned int end_id, bool exec_one_time/* = false*/)
{
    unsigned int timer_id = 0;

    if (elapse == 0) {
        return 0;
    }
    if (start_id == 0) {
        start_id = 1;
    }
    if (end_id == 0) {
        end_id = 0x7FFFFFFE;
    }

    for (timer_id = start_id; timer_id <= end_id; timer_id++) {
        if (start_timer_with_new_id(timer_id, elapse, exec_one_time)) {
            break;
        }
    }
    if (timer_id > end_id) {
        return 0;
    }

    return timer_id;
}

/**
 @功能：	启动一个固定范围内随机ID的新计时器
 @参数：	elapse：超时时间（ms）         exec_one_time：计时器超时后是否只执行一次
 @返回：	0：失败        >0：成功
 */
unsigned int TimerCtrl::start_timer_by_random_id(unsigned int elapse, bool exec_one_time/* = FALSE*/)
{
    return start_timer_by_random_id(elapse, 1, 0x7FFFFFFE, exec_one_time);
}

/**
 @功能：	停止一个计时器
 @参数：	id：计时器ID
 @返回：	true：成功        false：失败
 */
bool TimerCtrl::end_timer(unsigned int id)
{
    typename std::map<unsigned int, timer_t>::iterator it;

    if (id == 0) {
        return false;
    }

    sem_wait(&m_sem);
    it = m_timer_map.find(id);
    if (it == m_timer_map.end()) {
        sem_post(&m_sem);
        return false;
    }

    if (stop_posix_timer(it->second) == false) {
        sem_post(&m_sem);
        return false;
    }
    m_timer_map.erase(it);
    sem_post(&m_sem);

    return true;
}


/**
 @功能：	超时时间（ms）=> itimerspec
 @参数：	elapse：超时时间（ms）         exec_one_time：计时器超时后是否只执行一次
 @返回：	itimerspec
 */
itimerspec TimerCtrl::elapse_to_itimerspec(unsigned int elapse, bool exec_one_time/* = false*/)
{
    itimerspec its;
    time_t sec = 0;
    long msec = 0;

    sec = (time_t)(elapse / 1000);
    msec = (long)(elapse % 1000);

    if (exec_one_time == false) {
        its.it_interval.tv_sec = sec;
        its.it_interval.tv_nsec = msec * 1000 * 1000;
        its.it_value = its.it_interval;
    } else {
        its.it_interval.tv_sec = 0;
        its.it_interval.tv_nsec = 0;
        its.it_value.tv_sec = sec;
        its.it_value.tv_nsec = msec * 1000 * 1000;
    }

    return its;
}

/**
 @功能：	启动计时器
 @参数：	id：计时器ID            elapse：超时时间（ms）
 @      exec_one_time：计时器超时后是否只执行一次
 @返回：	0：失败        >0：成功
 */
timer_t TimerCtrl::set_posix_timer(unsigned int id, unsigned int elapse, bool exec_one_time/* = false*/)
{
    int ret = 0;
    timer_t tt;
    sigevent sev;
    itimerspec its;

    if ((m_timer_func == nullptr) || (id == 0)) {
        return 0;
    }

    sev.sigev_notify = SIGEV_THREAD;
    sev.sigev_value.sival_int = (int)id;
    sev._sigev_un._sigev_thread._function = m_timer_func;
    sev._sigev_un._sigev_thread._attribute = nullptr;
    ret = timer_create(CLOCK_REALTIME, &sev, &tt);
    if (ret != 0) {
        return 0;
    }

    its = elapse_to_itimerspec(elapse, exec_one_time);
    ret = timer_settime(tt, 0, &its, NULL);
    if (ret != 0) {
        timer_delete(tt);
        return 0;
    }

    return tt;
}

/**
 @功能：	停止计时器
 @参数：	tt：计时器timer_t
 @返回：	true：成功        false：失败
 */
bool TimerCtrl::stop_posix_timer(timer_t tt)
{
    int ret = 0;
    itimerspec its;

    its = elapse_to_itimerspec(0);
    ret = timer_settime(tt, 0, &its, NULL);
    if (ret != 0) {
        return false;
    }
    if (timer_delete(tt) != 0) {
        return false;
    }

    return true;
}
