#include <stdarg.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cctype>

#include "QtTypeDef.h"
typedef const char   *LPCTSTR;
//字串缓冲区类
class CStringBuffer
{
public:
    CStringBuffer(DWORD dwPreallocLen = 4096, const char *pTabStr = "\t", const char *pLFStr = "\n");
    virtual ~CStringBuffer();

    //清除内容
    CStringBuffer &ClearContent();

    //加一个字串到缓冲区中
    CStringBuffer &Add(const char *pStr);

    //格式化数据追加到缓冲区中
    CStringBuffer &AddF(const char *pFormat, ...);

    //加一个TAB字串到缓冲区
    CStringBuffer &AddTab(int iTabNum);

    //加一个换行字串到缓冲区
    CStringBuffer &EndLine();

    //得到缓冲区首地址
    const char *GetBuffer() const
    {
        return m_pData;
    }

    //得到缓冲区的长度
    DWORD GetBufLen() const
    {
        return m_dwBufLen;
    }

    //得到数据长度
    DWORD GetDataLen() const
    {
        return m_dwDataLen;
    }

    //得到缓冲区首地址
    operator LPCTSTR() const
    {
        return m_pData;
    }
private:
    void Clear();
    void EnsureLeftLen(DWORD dwLen);

    char *m_pData;      //缓冲区
    DWORD m_dwBufLen;   //缓冲区长度
    DWORD m_dwDataLen;  //数据长度

    char m_szTabStr[20];//TAB字串
    char m_szLFStr[20]; //换行字串
};

