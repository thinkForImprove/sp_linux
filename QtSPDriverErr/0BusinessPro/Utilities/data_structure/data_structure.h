#ifndef DATA_STRUCTURE_H
#define DATA_STRUCTURE_H

#include "__common_version_def.h"
#include <pthread.h>

using namespace std;


class DataStructure
{
    DEFINE_STATIC_VERSION_FUNCTIONS("data_structure", "0.0.0.0", TYPE_STATIC)
};


/****************************************************************************************************
 @功能：	线程同步的队列，其元素必须为指针类型（其值不可重复）
 @参数：	LPQueType：指针元素类型
 ****************************************************************************************************/
#include <list>
template<class LPQueType>
class SyncPointerQue
{
public:
    SyncPointerQue(void)
    {
        mutex_x = PTHREAD_MUTEX_INITIALIZER;
    }
    virtual ~SyncPointerQue(void)
    {
        clear_que();
        pthread_mutex_destroy(&mutex_x);
    }

    /**
     @功能：	队列中添加元素
     @参数：	element：元素
     @返回：	无
     */
    void add_que_element(LPQueType element)
    {
        pthread_mutex_lock(&mutex_x);
        m_que.push_back(element);
        pthread_mutex_unlock(&mutex_x);
    }

    /**
     @功能：	取得队列当前元素
     @参数：	delete_from_que：取得当前元素后是否从队列中删除该元素
     @返回：	当前元素
     */
    LPQueType get_cur_element(bool delete_from_que = false)
    {
        LPQueType element = NULL;

        pthread_mutex_lock(&mutex_x);
        if (m_que.empty() == false) {
            element = m_que.front();
            if (delete_from_que) {
                m_que.pop_front();
            }
        }
        pthread_mutex_unlock(&mutex_x);

        return element;
    }

    /**
     @功能：	删除队列当前元素
     @参数：	无
     @返回：	无
     */
    void del_cur_element()
    {
        LPQueType element = NULL;

        element = get_cur_element(true);

        if (element != NULL) {
            delete element;
        }
    }

    /**
     @功能：	删除队列指定元素
     @参数：	element：指定元素值
     @返回：	true：成功	false：失败
     */
    bool del_element(LPQueType element)
    {
        bool deleted = false;
        typename list<LPQueType>::iterator it;

        pthread_mutex_lock(&mutex_x);
        if (m_que.empty() == false) {
            for (it = m_que.begin(); it != m_que.end(); it++) {
                if (*it == element) {
                    m_que.erase(it);
                    deleted = true;
                    if (element != NULL) {
                        delete element;
                    }
                    break;
                }
            }
        }
        pthread_mutex_unlock(&mutex_x);

        return deleted;
    }

    /**
     @功能：	删除队列尾部元素
     @参数：	无
     @返回：	无
     */
    void del_tail_element()
    {
        LPQueType element = NULL;

        pthread_mutex_lock(&mutex_x);
        if (m_que.empty() == false) {
            element = m_que.back();
            m_que.pop_back();
            if (element != NULL) {
                delete element;
            }
        }
        pthread_mutex_unlock(&mutex_x);
    }

    /**
     @功能：	清空队列
     @参数：	无
     @返回：	无
     */
    void clear_que()
    {
        LPQueType element = NULL;

        pthread_mutex_lock(&mutex_x);
        while (m_que.empty() == false) {
            element = m_que.front();
            m_que.pop_front();
            if (element != NULL) {
                delete element;
            }
        }
        pthread_mutex_unlock(&mutex_x);
    }

    /**
     @功能：	取得队列元素个数
     @参数：	无
     @返回：	元素个数
     */
    int get_element_count()
    {
        int count = 0;

        pthread_mutex_lock(&mutex_x);
        count = m_que.size();
        pthread_mutex_unlock(&mutex_x);

        return count;
    }

    /**
     @功能：	取得队列元素列表
     @参数：	array_buf：列表数组Buffer	max_count：列表数组最大元素个数
     @返回：	元素个数（array_buf=NULL or max_count=0 时也返回）
     */
    int get_element_array(LPQueType array_buf[], int max_count)
    {
        int count = 0;
        int i = 0;
        typename list<LPQueType>::iterator it;

        if ((array_buf == NULL) || (max_count == 0)) {
            return get_element_count();
        }

        pthread_mutex_lock(&mutex_x);
        count = m_que.size();
        if (count > 0) {
            if (count > max_count) {
                count = max_count;
            }
            for (i = 0, it = m_que.begin(); i < count; it++, i++) {
                array_buf[i] = *it;
            }
        }
        pthread_mutex_unlock(&mutex_x);

        return count;
    }

protected:
    list<LPQueType> m_que;
    pthread_mutex_t mutex_x;
};


/****************************************************************************************************
 @功能：	线程同步的映射（其Key值不可重复）
 @参数：	KeyType：Key类型		ValueType：值类型
****************************************************************************************************/
#include <map>
template<class KeyType, class ValueType>
class SyncMap
{
public:
    SyncMap()
    {
        mutex_x = PTHREAD_MUTEX_INITIALIZER;
    }
    virtual ~SyncMap()
    {
        clear_map();
        pthread_mutex_destroy(&mutex_x);
    }

    /**
     @功能：	通过Key Value追加元素
     @参数：	key：Key值		val：Value值
     @返回：	无
     */
    void add_map(KeyType key, ValueType val)
    {
        bool found = false;
        typename map<KeyType, ValueType>::iterator it;

        pthread_mutex_lock(&mutex_x);
        if (m_map.size() > 0) {
            it = m_map.find(key);
            if (it != m_map.end()) {
                it->second = val;
                found = true;
            }
        }
        if (found == false) {
            m_map.insert(map<KeyType, ValueType>::value_type(key, val));
        }
        pthread_mutex_unlock(&mutex_x);
    }

    /**
     @功能：	通过Key查找Value值
     @参数：	key：Key值		val_ptr：Value值地址（NULL：忽略）
     @			delete_from_map：是否从映射中删除该值
     @返回：	true：找到		false：未找到
     */
    bool find_value(KeyType key, ValueType* val_ptr = NULL, bool delete_from_map = false)
    {
        bool found = false;
        typename map<KeyType, ValueType>::iterator it;

        pthread_mutex_lock(&mutex_x);
        if (m_map.size() > 0) {
            it = m_map.find(key);
            if (it != m_map.end()) {
                if (val_ptr != NULL) {
                    *val_ptr = it->second;
                }
                found = true;
                if (delete_from_map) {
                    m_map.erase(it);
                }
            }
        }
        pthread_mutex_unlock(&mutex_x);

        return found;
    }

    /**
     @功能：	查找相同Value值的Key值列表
     @参数：	val：Value值		key_array：Key值列表数组地址（NULL：忽略）
     @			max_count：Key值列表数组容纳Key值的最大个数（0：忽略）
     @返回：	找到的Key值个数
     */
    int find_keys_by_value(ValueType val, KeyType* key_array, int max_count)
    {
        int found_count = 0;
        typename map<KeyType, ValueType>::iterator it;

        if (m_map.size() == 0) {
            return 0;
        }

        pthread_mutex_lock(&mutex_x);

        for (it = m_map.begin(); it != m_map.end(); it++) {
            if (it->second == val) {
                if ((key_array != NULL) && (max_count > 0)) {
                    key_array[found_count] = it->first;
                }
                found_count++;
                if ((key_array != NULL) && (max_count > 0) &&
                    (found_count >= max_count)) {
                    break;
                }
            }
        }

        pthread_mutex_unlock(&mutex_x);

        return found_count;
    }

    /**
     @功能：	通过Key值删除元素
     @参数：	key：Key值
     @返回：	true：已删除		false：未删除
     */
    bool del_by_key(KeyType key)
    {
        bool deleted = false;
        typename map<KeyType, ValueType>::iterator it;

        pthread_mutex_lock(&mutex_x);
        if (m_map.size() > 0) {
            it = m_map.find(key);
            if (it != m_map.end()) {
                m_map.erase(it);
                deleted = true;
            }
        }
        pthread_mutex_unlock(&mutex_x);

        return deleted;
    }

    /**
     @功能：	通过Value值删除元素
     @参数：	val：Value值
     @返回：	已删除个数
     */
    int del_by_value(ValueType val)
    {
        int deleted_count = 0;
        KeyType* keys = NULL;
        int count = 0, idx = 0;

        count = m_map.size();
        if (count == 0) {
            return 0;
        }
        keys = new KeyType[count];
        if (keys == NULL) {
            return 0;
        }
        memset(keys, 0x00, count * sizeof(KeyType));

        deleted_count = find_keys_by_value(val, keys, count);
        if (deleted_count > 0) {
            for (idx = 0; idx < deleted_count; idx++) {
                del_by_key(keys[idx]);
            }
        }
        delete[] keys;

        return deleted_count;
    }

    /**
     @功能：	清除映射
     @参数：	无
     @返回：	无
     */
    void clear_map()
    {
        pthread_mutex_lock(&mutex_x);
        m_map.clear();
        pthread_mutex_unlock(&mutex_x);
    }

    /**
     @功能：	取得元素个数
     @参数：	无
     @返回：	元素个数
     */
    int get_element_count()
    {
        int count = 0;

        pthread_mutex_lock(&mutex_x);
        count = m_map.size();
        pthread_mutex_unlock(&mutex_x);

        return count;
    }

    /**
     @功能：	取得元素Key值列表
     @参数：	key_array：列表数组Buffer	max_count：列表数组最大元素个数
     @返回：	元素个数（key_array=NULL or max_count=0 时也返回）
     */
    int get_key_array(KeyType key_array[], int max_count)
    {
        int count = 0;
        int i = 0;
        typename map<KeyType, ValueType>::iterator it;

        if ((key_array == NULL) || (max_count == 0)) {
            return get_element_count();
        }

        pthread_mutex_lock(&mutex_x);
        count = m_map.size();
        if (count > 0) {
            if (count > max_count) {
                count = max_count;
            }
            for (i = 0, it = m_map.begin(); i < count; it++, i++) {
                key_array[i] = it->first;
            }
        }
        pthread_mutex_unlock(&mutex_x);

        return count;
    }

protected:
    map<KeyType, ValueType> m_map;
    pthread_mutex_t mutex_x;
};

#endif // DATA_STRUCTURE_H
