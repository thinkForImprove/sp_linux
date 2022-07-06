#ifndef CWINEVENT_H
#define CWINEVENT_H


#include "__common_os_compatible_def.h"


#define WAIT_OBJECT_0                           ((unsigned long)0)
#define WAIT_TIMEOUT                            ((unsigned long)0x102)
#define WAIT_ABANDONED                          ((unsigned long)0x80)
#define WAIT_ABANDONED_0                        ((unsigned long)0x80)
#define WAIT_FAILED                             ((unsigned long)-1)


struct neosmart_event_t_;
typedef neosmart_event_t_ * neosmart_event_t;


class WaitableEvent
{
public:
    static unsigned long unlocked_wait_for_event(neosmart_event_t event, unsigned long milliseconds);

    static neosmart_event_t create_event(bool manual_reset = false, bool initial_state = false);
    static bool destroy_event(neosmart_event_t event);
    static unsigned long wait_for_event(neosmart_event_t event, unsigned long milliseconds = INFINITE);
    static bool set_event(neosmart_event_t event);
    static bool reset_event(neosmart_event_t event);

    static unsigned long wait_for_multiple_events(neosmart_event_t *events, int count, bool wait_all, unsigned long milliseconds);
    static int wait_for_multiple_events(neosmart_event_t *events, int count, bool wait_all, unsigned long milliseconds, unsigned long &wait_index);
};

#endif // CWINEVENT_H
