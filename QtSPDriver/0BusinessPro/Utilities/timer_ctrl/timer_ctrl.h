#ifndef TIMERCTRL_H
#define TIMERCTRL_H

#include <map>
#include <signal.h>
#include <time.h>
#include <semaphore.h>

#include "timer_ctrl_global.h"


typedef void(*TmerFuncType)(sigval_t);


class TIMER_CTRL_EXPORT TimerCtrl
{
public:
    TimerCtrl();
    TimerCtrl(TmerFuncType timer_func);
    virtual ~TimerCtrl(void);

    void set_timer_function(TmerFuncType timer_func);
    void clear_timers();
    bool start_timer(unsigned int id, unsigned int elapse, bool exec_one_time = false);
    bool start_timer_with_new_id(unsigned int id, unsigned int elapse, bool exec_one_time = false);
    unsigned int start_timer_by_random_id(
            unsigned int elapse, unsigned int start_id, unsigned int end_id, bool exec_one_time = false);
    unsigned int start_timer_by_random_id(unsigned int elapse, bool exec_one_time = false);
    bool end_timer(unsigned int id);


protected:
    itimerspec elapse_to_itimerspec(unsigned int elapse, bool exec_one_time = false);
    timer_t set_posix_timer(unsigned int id, unsigned int elapse, bool exec_one_time = false);
    bool stop_posix_timer(timer_t tt);


protected:
    TmerFuncType m_timer_func;
    std::map<unsigned int, timer_t> m_timer_map;    // <timer_id, timer_t>
    sem_t m_sem;
};

#endif // TIMERCTRL_H
