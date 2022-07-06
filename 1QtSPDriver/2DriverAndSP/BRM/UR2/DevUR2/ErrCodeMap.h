#ifndef ERRCODEMAP_H
#define ERRCODEMAP_H
#include "QtTypeDef.h"
#include <vector>
#include <string>
#include <map>
#include <mutex>
using namespace std;
class CErrCodeMap
{

public:
    static CErrCodeMap  *GetInstance();

    /*
    功能：获取错误代号（保存在数组的错误字符串）对应的错误描述
    参数：aryLastErr   输入参数，保存着错误代号的数组
    返回值：成功返回对应的错误描述；未找到或参数错误返回空串：""
    */
    const char *GetErrDescrStr(const char *pszErrorCode);

private:
    CErrCodeMap();
    virtual ~CErrCodeMap();
    typedef const char *MY_TYPE;

    struct MY_COMPARE  :  binary_function<MY_TYPE, MY_TYPE, bool>
    {
        bool operator()(const MY_TYPE &_X, const MY_TYPE &_Y) const
        {
            MY_TYPE p1 = _X, p2 = _Y;
            while (*p1 != '\0' && *p2 != '\0')
            {
                if (*p1 == '*' || *p2 == '*' || *p1 == *p2)
                {
                    p1++;
                    p2++;
                }
                else
                {
                    return *p1 < *p2;
                }
            }
            if (*p1 == '\0' && *p2 == '\0')
            {
                return false;
            }
            else if (*p1 == '\0')
            {
                return true;
            }

            return false;
        }//end operator()
    };

    typedef multimap<MY_TYPE, MY_TYPE, MY_COMPARE> ERRMAP;
    ERRMAP m_errmap; //映射错误代号到错误描述的元素，所在对象被构造时将所有映射对初始化

    /*
    功能：向m_errmap插入一个错误代号和错误描述对
    参数：szErr ：错误代号；szDescr：与szErr对应的错误描述
    返回值：无
    */
    void InsertToMap(const char *szErr, const char *szDescr);
    static CErrCodeMap *m_instance;
};//end class CErrCodeMap
#endif // ERRCODEMAP_H
