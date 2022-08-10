#include "framework.h"
#include "waitable_event.h"


struct neosmart_wfmo_t_
{
    pthread_mutex_t mutex;
    pthread_cond_t variable;
    int ref_count;
    union {
        int fired_event; //WFSO
        int events_left; //WFMO
    } status;
    bool wait_all;
    bool still_waiting;

    void destroy() {
        pthread_mutex_destroy(&mutex);
        pthread_cond_destroy(&variable);
    }
};
typedef neosmart_wfmo_t_ *neosmart_wfmo_t;

struct neosmart_wfmo_info_t_
{
    neosmart_wfmo_t waiter;
    int wait_index;
};
typedef neosmart_wfmo_info_t_ *neosmart_wfmo_info_t;

struct neosmart_event_t_
{
    pthread_cond_t variable;
    pthread_mutex_t mutex;
    bool auto_reset;
    bool state;

    std::deque<neosmart_wfmo_info_t_> registered_waits;
};


bool remove_expired_wait_helper(neosmart_wfmo_info_t_ wait)
{
    int result = pthread_mutex_trylock(&wait.waiter->mutex);

    if (result != 0) {
        return false;
    }

    if (wait.waiter->still_waiting == false) {
        wait.waiter->ref_count--;
        if (wait.waiter->ref_count < 0) {
            return false;
        }
        if (wait.waiter->ref_count == 0) {
            wait.waiter->destroy();
            delete wait.waiter;
        } else {
            result = pthread_mutex_unlock(&wait.waiter->mutex);
            if (result != 0) {
                return false;
            }
        }

        return true;
    }

    result = pthread_mutex_unlock(&wait.waiter->mutex);
    if (result != 0) {
        return false;
    }

    return true;
}


neosmart_event_t WaitableEvent::create_event(bool manual_reset/* = false*/, bool initial_state/* = false*/)
{
    int result = 0;
    neosmart_event_t event = new neosmart_event_t_;

    result = pthread_cond_init(&event->variable, NULL);
    if (result != 0) {
        return NULL;
    }

    result = pthread_mutex_init(&event->mutex, NULL);
    if (result != 0) {
        return NULL;
    }

    event->state = false;
    event->auto_reset = !manual_reset;

    if (initial_state != false) {
        result = set_event(event);
        if (result != 0) {
            return NULL;
        }
    }

    return event;
}

unsigned long WaitableEvent::unlocked_wait_for_event(neosmart_event_t event, unsigned long milliseconds)
{
    unsigned long wait_result = WAIT_OBJECT_0;
    int result = 0;
    timespec ts;
    timeval tv;

    if (event->state == false) {
        //Zero-timeout event state check optimization
        if (milliseconds == 0) {
            return WAIT_TIMEOUT;
        }

        if (milliseconds != INFINITE) {
            gettimeofday(&tv, NULL);

            __uint64_t nanoseconds = ((__uint64_t)tv.tv_sec) * 1000 * 1000 * 1000 +
                    milliseconds * 1000 * 1000 +
                    ((__uint64_t)tv.tv_usec) * 1000;

            ts.tv_sec = nanoseconds / 1000 / 1000 / 1000;
            ts.tv_nsec = (nanoseconds - ((__uint64_t)ts.tv_sec) * 1000 * 1000 * 1000);
        }

        do {
            //Regardless of whether it's an auto-reset or manual-reset event:
            //wait to obtain the event, then lock anyone else out
            if (milliseconds != INFINITE) {
                result = pthread_cond_timedwait(&event->variable, &event->mutex, &ts);
            } else {
                result = pthread_cond_wait(&event->variable, &event->mutex);
            }
        } while ((result == 0) && (event->state == false));

        if ((result == 0) && (event->auto_reset != false)) {
            //We've only accquired the event if the wait succeeded
            event->state = false;
        }
    } else if (event->auto_reset != false) {
        //It's an auto-reset event that's currently available;
        //we need to stop anyone else from using it
        event->state = false;
    }
    //Else we're trying to obtain a manual reset event with a signaled state;
    //don't do anything

    switch (result) {
    case 0:
        wait_result = WAIT_OBJECT_0;
        break;
    case ETIMEDOUT:
        wait_result = WAIT_TIMEOUT;
        break;
    default:
        wait_result = WAIT_FAILED;
        break;
    }

    return wait_result;
}

unsigned long WaitableEvent::wait_for_event(neosmart_event_t event, unsigned long milliseconds/* = INFINITE*/)
{
    unsigned long wait_result = WAIT_OBJECT_0;
    int result = 0;

    if (milliseconds == 0) {
        result = pthread_mutex_trylock(&event->mutex);
        if (result == EBUSY) {
            return WAIT_TIMEOUT;
        }
    } else {
        result = pthread_mutex_lock(&event->mutex);
    }

    if (result != 0) {
        return WAIT_FAILED;
    }

    wait_result = unlocked_wait_for_event(event, milliseconds);

    result = pthread_mutex_unlock(&event->mutex);
    if (result != 0) {
        return WAIT_FAILED;
    }

    return wait_result;
}

unsigned long WaitableEvent::wait_for_multiple_events(neosmart_event_t *events, int count, bool wait_all, unsigned long milliseconds)
{
    unsigned long wait_result = WAIT_OBJECT_0;
    unsigned long wait_idx = (unsigned long)-1;
    int result = wait_for_multiple_events(events, count, wait_all, milliseconds, wait_idx);
    switch (result) {
    case 0:
        wait_result = WAIT_OBJECT_0;
        if (wait_all == false) {
            wait_result += wait_idx;
        }
        break;
    case ETIMEDOUT:
        wait_result = WAIT_TIMEOUT;
        break;
    default:
        wait_result = WAIT_FAILED;
        break;
    }

    return wait_result;
}

int WaitableEvent::wait_for_multiple_events(neosmart_event_t *events, int count, bool wait_all, unsigned long milliseconds, unsigned long &wait_index)
{
    neosmart_wfmo_t wfmo = new neosmart_wfmo_t_;

    bool done = false;
    int i = 0;
    int result = 0;
    timespec ts;
    timeval tv;
    __uint64_t usec = 0;
    int temp_result = pthread_mutex_init(&wfmo->mutex, NULL);
    if (temp_result != 0) {
        return temp_result;
    }

    temp_result = pthread_cond_init(&wfmo->variable, NULL);
    if (temp_result != 0) {
        return temp_result;
    }

    neosmart_wfmo_info_t_ wait_info;
    wait_info.waiter = wfmo;
    wait_info.wait_index = -1;

    wfmo->wait_all = wait_all;
    wfmo->still_waiting = true;
    wfmo->ref_count = 1;

    if (wait_all) {
        wfmo->status.events_left = count;
    } else {
        wfmo->status.fired_event = -1;
    }

    temp_result = pthread_mutex_lock(&wfmo->mutex);
    if (temp_result != 0) {
        return temp_result;
    }

    done = false;
    wait_index = -1;

    for (i = 0; i < count; ++i) {
        wait_info.wait_index = i;

        //Must not release lock until RegisteredWait is potentially added
        temp_result = pthread_mutex_lock(&events[i]->mutex);
        if (temp_result != 0) {
            return temp_result;
        }

        //Before adding this wait to the list of registered waits, let's clean up old, expired waits while we have the event lock anyway
        events[i]->registered_waits.erase(
                    std::remove_if (events[i]->registered_waits.begin(), events[i]->registered_waits.end(), remove_expired_wait_helper),
                    events[i]->registered_waits.end());

        if (unlocked_wait_for_event(events[i], 0) == WAIT_OBJECT_0) {
            temp_result = pthread_mutex_unlock(&events[i]->mutex);
            if (temp_result != 0) {
                return temp_result;
            }

            if (wait_all) {
                wfmo->status.events_left--;
                if (wfmo->status.events_left < 0) {
                    return -1;
                }
            } else {
                wfmo->status.fired_event = i;
                wait_index = i;
                done = true;
                break;
            }
        } else {
            events[i]->registered_waits.push_back(wait_info);
            wfmo->ref_count++;

            temp_result = pthread_mutex_unlock(&events[i]->mutex);
            if (temp_result != 0) {
                return temp_result;
            }
        }
    }

    if (done == false) {
        if (milliseconds == 0) {
            result = WAIT_TIMEOUT;
            done = true;
        } else if (milliseconds != INFINITE) {
            gettimeofday(&tv, NULL);
            usec = ((__uint64_t)tv.tv_sec) * 1000 * 1000 + milliseconds * 1000 + tv.tv_usec;
            ts.tv_sec = usec / 1000 / 1000;
            ts.tv_nsec = (usec - ((__uint64_t)ts.tv_sec) * 1000 * 1000) * 1000;
        }
    }

    while (done == false) {
        //One (or more) of the events we're monitoring has been triggered?

        //If we're waiting for all events, assume we're done and check if there's an event that hasn't fired
        //But if we're waiting for just one event, assume we're not done until we find a fired event
        done = ((wait_all != false) && (wfmo->status.events_left == 0)) ||
                ((wait_all == false) && (wfmo->status.fired_event != -1));

        if (done == false) {
            if (milliseconds != INFINITE) {
                result = pthread_cond_timedwait(&wfmo->variable, &wfmo->mutex, &ts);
            } else {
                result = pthread_cond_wait(&wfmo->variable, &wfmo->mutex);
            }

            if (result != 0) {
                break;
            }
        }
    }

    wait_index = wfmo->status.fired_event;
    wfmo->still_waiting = false;

    wfmo->ref_count--;
    if (wfmo->ref_count < 0) {
        return -1;
    }
    if (wfmo->ref_count == 0) {
        wfmo->destroy();
        delete wfmo;
    } else {
        temp_result = pthread_mutex_unlock(&wfmo->mutex);
        if (temp_result != 0) {
            return temp_result;
        }
    }

    return result;
}

bool WaitableEvent::destroy_event(neosmart_event_t event)
{
    int result = 0;

    result = pthread_mutex_lock(&event->mutex);
    if (result != 0) {
        return false;
    }
    event->registered_waits.erase(std::remove_if (event->registered_waits.begin(), event->registered_waits.end(), remove_expired_wait_helper), event->registered_waits.end());
    result = pthread_mutex_unlock(&event->mutex);
    if (result != 0) {
        return false;
    }

    result = pthread_cond_destroy(&event->variable);
    if (result != 0) {
        return false;
    }

    result = pthread_mutex_destroy(&event->mutex);
    if (result != 0) {
        return false;
    }

    delete event;

    return true;
}

bool WaitableEvent::set_event(neosmart_event_t event)
{
    size_t i = 0;
    int result = pthread_mutex_lock(&event->mutex);
    if (result != 0) {
        return false;
    }

    event->state = true;

    //Depending on the event type, we either trigger everyone or only one
    if (event->auto_reset != false) {
        while (event->registered_waits.empty() == false) {
            neosmart_wfmo_info_t i = &event->registered_waits.front();

            result = pthread_mutex_lock(&i->waiter->mutex);
            if (result != 0) {
                return false;
            }

            i->waiter->ref_count--;
            if (i->waiter->ref_count < 0) {
                return false;
            }
            if (i->waiter->still_waiting == false) {
                if (i->waiter->ref_count == 0) {
                    i->waiter->destroy();
                    delete i->waiter;
                } else {
                    result = pthread_mutex_unlock(&i->waiter->mutex);
                    if (result != 0) {
                        return false;
                    }
                }
                event->registered_waits.pop_front();
                continue;
            }

            event->state = false;

            if (i->waiter->wait_all != false) {
                i->waiter->status.events_left--;
                if (i->waiter->status.events_left < 0) {
                    return false;
                }
                //We technically should do i->Waiter->StillWaiting = Waiter->Status.EventsLeft != 0
                //but the only time it'll be equal to zero is if we're the last event, so no one
                //else will be checking the StillWaiting flag. We're good to go without it.
            } else {
                i->waiter->status.fired_event = i->wait_index;
                i->waiter->still_waiting = false;
            }

            result = pthread_mutex_unlock(&i->waiter->mutex);
            if (result != 0) {
                return false;
            }

            result = pthread_cond_signal(&i->waiter->variable);
            if (result != 0) {
                return false;
            }

            event->registered_waits.pop_front();

            result = pthread_mutex_unlock(&event->mutex);
            if (result != 0) {
                return false;
            }

            return true;
        }

        //event->State can be false if compiled with WFMO support
        if (event->state != false) {
            result = pthread_mutex_unlock(&event->mutex);
            if (result != 0) {
                return false;
            }

            result = pthread_cond_signal(&event->variable);
            if (result != 0) {
                return false;
            }

            return true;
        }
    } else {

        for (i = 0; i < event->registered_waits.size(); ++i)
        {
            neosmart_wfmo_info_t info = &event->registered_waits[i];

            result = pthread_mutex_lock(&info->waiter->mutex);
            if (result != 0) {
                return false;
            }

            info->waiter->ref_count--;
            if (info->waiter->ref_count < 0) {
                return false;
            }

            if (info->waiter->still_waiting == false) {
                if (info->waiter->ref_count == 0)
                {
                    info->waiter->destroy();
                    delete info->waiter;
                } else {
                    result = pthread_mutex_unlock(&info->waiter->mutex);
                    if (result != 0) {
                        return false;
                    }
                }
                continue;
            }

            if (info->waiter->wait_all != false) {
                info->waiter->status.events_left--;
                if (info->waiter->status.events_left < 0) {
                    return false;
                }
                //We technically should do i->Waiter->StillWaiting = Waiter->Status.EventsLeft != 0
                //but the only time it'll be equal to zero is if we're the last event, so no one
                //else will be checking the StillWaiting flag. We're good to go without it.
            } else {
                info->waiter->status.fired_event = info->wait_index;
                info->waiter->still_waiting = false;
            }

            result = pthread_mutex_unlock(&info->waiter->mutex);
            if (result != 0) {
                return false;
            }

            result = pthread_cond_signal(&info->waiter->variable);
            if (result != 0) {
                return false;
            }
        }
        event->registered_waits.clear();

        result = pthread_mutex_unlock(&event->mutex);
        if (result != 0) {
            return false;
        }

        result = pthread_cond_broadcast(&event->variable);
        if (result != 0) {
            return false;
        }
    }

    return true;
}

bool WaitableEvent::reset_event(neosmart_event_t event)
{
    int result = pthread_mutex_lock(&event->mutex);
    if (result != 0) {
        return false;
    }

    event->state = false;

    result = pthread_mutex_unlock(&event->mutex);
    if (result != 0) {
        return false;
    }

    return true;
}
