#include "framework.h"
#include "synchronism.h"


MultiEvent::MultiEvent()
{
    m_last_error = 0;
}

MultiEvent::~MultiEvent()
{
	clear_events();
}


/**
 @功能：	关闭并清空所有Event
 @参数：	无
 @返回：	无
 */
void MultiEvent::clear_events()
{
    map<int, neosmart_event_t>::iterator it;

    m_last_error = 0;
	if (m_event_map.size() == 0) {
		return;
	}
	for (it = m_event_map.begin(); it != m_event_map.end(); it++) {
		if (it->second != NULL) {
            WaitableEvent::destroy_event(it->second);
			it->second = NULL;
		}
	}
	m_event_map.clear();
}

/**
 @功能：	通过索引和名称创建Event
 @参数：	event_index：Event索引（>=0）	manual：是否手动
 @		initial：初始状态是否释放
 @返回：	true：成功		false：失败
 */
bool MultiEvent::add_event(int event_index, bool manual/* = false*/, bool initial/* = false*/)
{
    neosmart_event_t event_handle = NULL;

    m_last_error = 0;
	if ((event_index == -1) ||
		(m_event_map.count(event_index) > 0) ||
		(m_event_map.size() == MAX_EVENT_COUNT)) {
		return false;
	}

    event_handle = WaitableEvent::create_event(manual, initial);
	if (event_handle == NULL) {
        m_last_error = errno;
		return false;
	}
    m_event_map.insert(map<int, neosmart_event_t>::value_type(event_index, event_handle));

	return true;
}

/**
 @功能：	通过索引序列批量创建无名称的Event
 @参数：	event_index_array：Event索引序列	event_count：Event个数
 @		manual：是否手动					initial：初始状态是否释放
 @返回：	true：成功		false：失败
 */
bool MultiEvent::add_event(int* event_index_array, int event_count, bool manual/* = false*/, bool initial/* = false*/)
{
	int i = 0;

    m_last_error = 0;
	if ((event_index_array == NULL) || (event_count == 0)) {
		return false;
	}

	for (i = 0; i < event_count; i++) {
        if (add_event(event_index_array[i], manual, initial) == false) {
			return false;
		}
	}

	return true;
}

/**
 @功能：	通过起始索引值及个数批量创建无名称的Event
 @参数：	base_index：起始的索引         event_count：Event个数
 @		manual：是否手动               initial：初始状态是否释放
 @返回：	true：成功		false：失败
 */
bool MultiEvent::add_index_based_event(int base_index, int event_count, bool manual/* = false*/, bool initial/* = false*/)
{
	int i = 0;

    m_last_error = 0;
	if ((base_index < 0) || (event_count == 0)) {
		return false;
	}

	for (i = base_index; i < base_index + event_count; i++) {
        if (add_event(i, manual, initial) == false) {
			return false;
		}
	}

	return true;
}

/**
 @功能：	通过0起始的索引序列批量创建无名称的Event
 @参数：	event_count：Event个数			manual：是否手动
 @		initial：初始状态是否释放
 @返回：	true：成功		false：失败
 */
bool MultiEvent::add_zero_based_event(int event_count, bool manual/* = false*/, bool initial/* = false*/)
{
    return add_index_based_event(0, event_count, manual, initial);
}

/**
 @功能：	释放指定索引的Event
 @参数：	event_index：Event索引
 @返回：	true：成功		false：失败
 */
bool MultiEvent::set_event(int event_index)
{
	bool ret = false;
    map<int, neosmart_event_t>::iterator it;

    m_last_error = 0;
	if (m_event_map.size() == 0) {
		return false;
	}

	it = m_event_map.find(event_index);
	if (it == m_event_map.end()) {
		return false;
	}

    ret = WaitableEvent::set_event(it->second);
	if (ret == false) {
        m_last_error = errno;
	}

	return ret;
}

/**
 @功能：	释放所有Event
 @参数：	无
 @返回：	true：成功		false：失败
 */
bool MultiEvent::set_all_event()
{
    map<int, neosmart_event_t>::iterator it;

    m_last_error = 0;
	if (m_event_map.size() == 0) {
		return true;
	}

	for (it = m_event_map.begin(); it != m_event_map.end(); it++) {
        if (WaitableEvent::set_event(it->second) == false) {
            m_last_error = errno;
			return false;
		}
	}

	return true;
}

/**
 @功能：	阻塞指定索引的Event
 @参数：	event_index：Event索引
 @返回：	true：成功		false：失败
 */
bool MultiEvent::reset_event(int event_index)
{
    bool ret = false;
    map<int, neosmart_event_t>::iterator it;

    m_last_error = 0;
	if (m_event_map.size() == 0) {
		return false;
	}

	it = m_event_map.find(event_index);
	if (it == m_event_map.end()) {
		return false;
	}

    ret = WaitableEvent::reset_event(it->second);
	if (ret == false) {
        m_last_error = errno;
	}

	return ret;
}

/**
 @功能：	阻塞所有Event
 @参数：	无
 @返回：	true：成功		false：失败
 */
bool MultiEvent::reset_all_event()
{
    map<int, neosmart_event_t>::iterator it;

    m_last_error = 0;
    if (m_event_map.size() == 0) {
        return true;
    }

    for (it = m_event_map.begin(); it != m_event_map.end(); it++) {
        if (WaitableEvent::reset_event(it->second) == false) {
            m_last_error = errno;
                return false;
        }
    }

    return true;
}

/**
 @功能：	通过指定Event索引序列等待多个Event结果
 @参数：	waited_index：
 @          若need_wait_all = false，返回值WAIT_OBJ_SUCCESS or WAIT_OBJ_ABNORMAL，waited_index存放等待到（或异常时）的Event索引
 @          其它情况下不设置该参数
 @	milli_second：超时时间（ms）			need_wait_all：是否等待所有Event释放
 @	event_index_array：索引序列			event_count：序列中索引个数
 @返回：	等待结果
 */
unsigned long MultiEvent::wait_multi(int* waited_index, unsigned long milli_second, bool need_wait_all, int* event_index_array, int event_count)
{
    unsigned long wait = WAIT_FAILED, result = WAIT_OBJ_SUCCESS;
    neosmart_event_t event_handle[MAX_EVENT_COUNT];
    int event_index[MAX_EVENT_COUNT] = { -1 };
    map<int, neosmart_event_t>::iterator it;
    int i = 0;
    int count = 0;

    m_last_error = 0;
    memset(event_handle, 0x00, sizeof(event_handle));
    if ((event_index_array == NULL) || (event_count == 0) || (m_event_map.size() == 0)) {
        return WAIT_OBJ_FAIL;
    }

    count = 0;
    for (i = 0; i < event_count; i++) {
        it = m_event_map.find(event_index_array[i]);
        if (it != m_event_map.end()) {
            event_index[count] = event_index_array[i];
            event_handle[count] = it->second;
            count++;
        }
    }

    if (count == 0) {
        return WAIT_OBJ_FAIL;
    }
    wait = WaitableEvent::wait_for_multiple_events(event_handle, count, need_wait_all, milli_second);

    switch (wait) {
    case WAIT_FAILED:
        m_last_error = errno;
        result = WAIT_SINGLE_OBJ_RES(wait);
        break;
    case WAIT_TIMEOUT:
        result = WAIT_SINGLE_OBJ_RES(wait);
        break;
    default:
        if (wait >= WAIT_ABANDONED_0) {
            result = WAIT_OBJ_ABNORMAL;
            if (need_wait_all == false) {
                if (waited_index) {
                    *waited_index = event_index[wait - WAIT_ABANDONED_0];
                }
            }
        } else {
            result = WAIT_OBJ_SUCCESS;
            if (need_wait_all == false) {
                if (waited_index) {
                    *waited_index = event_index[wait - WAIT_OBJECT_0];
                }
            }
        }
        break;
    }

    return result;
}

/**
 @功能：	通过指定Event索引序列等待多个Event结果（不定参数版）
 @参数：	waited_index：
 @          若need_wait_all = false，返回值WAIT_OBJ_SUCCESS or WAIT_OBJ_ABNORMAL，waited_index存放等待到（或异常时）的Event索引
 @          其它情况下不设置该参数
 @	milli_second：超时时间（ms）			need_wait_all：是否等待所有Event释放
 @	event_count：等待的Event索引个数		...：等待的Event索引
 @返回：	等待结果
 */
unsigned long MultiEvent::wait_multi(int* waited_index, unsigned long milli_second, bool need_wait_all, int event_count, ...)
{
    int event_index[MAX_EVENT_COUNT];
    int i = 0;

    m_last_error = 0;
    memset(event_index, 0xff, sizeof(event_index));
    if ((event_count == 0) || (m_event_map.size() == 0)) {
        return WAIT_OBJ_FAIL;
    }

    va_list arg_ptr;
    va_start(arg_ptr, event_count);
    for (i = 0; i < event_count; i++) {
        event_index[i] = va_arg(arg_ptr, int);
    }
    va_end(arg_ptr);

    return wait_multi(waited_index, milli_second, need_wait_all, event_index, event_count);
}

/**
 @功能：	等待所有Event结果
 @参数：	waited_index：
 @          若need_wait_all = false，返回值WAIT_OBJ_SUCCESS or WAIT_OBJ_ABNORMAL，waited_index存放等待到（或异常时）的Event索引
 @          其它情况下不设置该参数
 @	milli_second：超时时间（ms）			need_wait_all：是否等待所有Event释放
 @返回：	等待结果
 */
unsigned long MultiEvent::wait_multi(int* waited_index, unsigned long milli_second, bool need_wait_all)
{
    int event_index[MAX_EVENT_COUNT];
    map<int, neosmart_event_t>::iterator it;
    int count = 0;

    m_last_error = 0;
    memset(event_index, 0xff, sizeof(event_index));
    if (m_event_map.size() == 0) {
        return WAIT_OBJ_FAIL;
    }

    count = 0;
    for (it = m_event_map.begin(); it != m_event_map.end(); it++) {
        event_index[count] = it->first;
        count++;
    }

    return wait_multi(waited_index, milli_second, need_wait_all, event_index, count);
}
