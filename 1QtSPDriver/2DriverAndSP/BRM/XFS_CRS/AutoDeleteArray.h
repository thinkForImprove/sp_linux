//AutoDeleteArray.h

#include <assert.h>
#include "QtTypeDef.h"

//自动的析构时删除数组的辅助类
template <class TYPE>
class CAutoDeleteArrayEx
{
public:
    CAutoDeleteArrayEx(USHORT usLen)
    {
        assert(usLen > 0);
        m_usLen = usLen;
        m_pArray = new TYPE[usLen];
    }

    virtual ~CAutoDeleteArrayEx()
    {
        assert(m_pArray != NULL);
        delete [] m_pArray;
        m_pArray = NULL;
    }

    operator TYPE *()
    {
        assert(m_pArray != NULL);
        return m_pArray;
    }

    TYPE &operator[](USHORT usIndex)
    {
        assert(m_pArray != NULL);
        assert(usIndex < m_usLen);
        return m_pArray[usIndex];
    }
private:
    TYPE *m_pArray;
    ULONG m_usLen;
};
