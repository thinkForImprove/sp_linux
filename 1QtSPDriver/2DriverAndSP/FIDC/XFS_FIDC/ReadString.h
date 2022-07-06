#ifndef READ_STRING_H
#define READ_STRING_H

#include "QtTypeDef.h"
#include "XFSIDC.H"
#include "MultiString.h"
#include <stdio.h>
#include <ctype.h>


//功能：使p指向第一个非空格的字符
//输入：p : 字串指针。
//返回：无。
inline void SkipSpace(char *&p)
{
    while (isspace((BYTE)*p))
        p++;
}

//功能：跳过指定字符。
//输入：p : 字串指针，
//      c : 指定字符。
//返回：TRUE，p指向的字符是c。
inline BOOL SkipChar(char *&p, char c)
{
    if (*p == c)
    {
        p++;
        return TRUE;
    }
    return FALSE;
}

//功能：使p指向指定字符，pFirstContinueBlank指向最后一个连续
//      空格串的第一个空格。
//输入：p : 字串指针，
//      c : 指定字符，
//      pFirstContinueBlank : 空格字串指针。
//返回：TRUE，p指向的字符是c，否则，找到字串结束。
inline BOOL ReadToChar(char *&p, char c, char *&pFirstContinueBlank)
{
    pFirstContinueBlank = NULL;
    while (*p && *p != c)
    {
        if (isspace((BYTE)*p))
        {
            if (!pFirstContinueBlank)
                pFirstContinueBlank = p;
        }
        else
        {
            pFirstContinueBlank = NULL;
        }
        p++;
    }

    if (isspace((BYTE)*p))
    {
        if (!pFirstContinueBlank)
            pFirstContinueBlank = p;
    }
    return *p;
}

//功能：使p指向空格
//输入：p : 字串指针
//返回：TRUE, p指向空格，否则，p指向字串结束。
inline BOOL ReadToSpace(char *&p)
{
    while (*p && !isspace((BYTE)*p))
        p++;
    return *p;
}

//功能：在字串p中找EndChar, 使p指向EndChar后，pStart指向实际值开始处
//输入：p : 字串指针
//      pStart  : 字串指针
//      EndChar : 结束字符
//返回：TRUE, p指向EndChar后，否则，p指向字串结束。
inline BOOL ReadValue(char *&p, char *&pStart, char EndChar)
{
    SkipSpace(p);
    pStart = p;
    char *pFirstBlank = NULL;
    if (SkipChar(p, '"'))
    {
        pStart = p;
        if (!ReadToChar(p, '"', pFirstBlank))
            return false;
        *p = 0;
    }
    else
    {
        if (!ReadToChar(p, EndChar, pFirstBlank))
            return false;
        if (pFirstBlank)
            *pFirstBlank = 0;
        *p = 0;
    }
    p++;
    return TRUE;
}

//功能：从字串读取名字和值对
//输入：lpLine : 字串
//      Pair   : 名值对
//返回：TRUE，成功；否则，失败。
inline BOOL GetNameAndValue(char *lpLine, CMultiString &Pair)
{
    char *pStart = nullptr;
    char *p = lpLine;
    if (!ReadValue(p, pStart, '='))
    {
        return FALSE;
    }
    Pair.Add(pStart);

    ReadValue(p, pStart, '\0');
    Pair.Add(pStart);
    return TRUE;
}

//功能：从字串读取多个值，以SepStr分隔
//输入：lpLine : 字串
//      SeqStr : 分隔字串
//返回：多字串。
inline CMultiString Split(char *lpLine, const char *SepStr)
{
    CMultiString ms;
    char *p = strtok(lpLine, SepStr);
    while (p)
    {
        ms.Add(p);
        p = strtok(nullptr, SepStr);
    }
    return ms;
}


#endif

