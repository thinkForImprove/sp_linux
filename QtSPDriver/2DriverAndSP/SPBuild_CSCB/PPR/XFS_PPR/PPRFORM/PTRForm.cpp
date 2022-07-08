#include <errno.h>
#include <asm/errno.h>
#include "PTRForm.h"
#include "ILogWrite.h"
/*********************宏定义***×********************************************/
// 事件日志
#define ThisFile                    "SPBasePrinter"
#define EVENT_LOG                   "Event.log"

#define LONG_MAX                    2147483647L
#define XDIGIT(x)                   (x >= '0' && x <= '9' ? x - '0' : (x >= 'A' && x <= 'F' ? x - 'A' + 10 : x - 'a' + 10))
#define OT(v)                       ((long)&v - (long)(CSPPrinterItem *)this)  // 指针为8个字节
#define AAL(v)                      AddAttr(OT(v), sizeof(v))
#define AALV(v, ve)                 AddAttr(OT(v), sizeof(v), ve)
#define AALVR(v, ve, r)             AddAttr(OT(v), sizeof(v), ve, r)
#define AAS(v)                      AddAttr(OT(v), 0)
#define AASR(v, r)                  AddAttr(OT(v), 0, NULL, r)

// 错误描述
#define IDS_ERR_REQUIRED_ATTR       "[%s][%s]的关键字[%s]第%d个属性没有定义[%d行]"
#define IDS_ERR_ATTR_INVALID_VALUE  "属性值错误，原始数据：[%s], 值类型长度：[%d], 错误码：[%d]"
#define IDS_ERR_READ_NAME           "读[%s]名失败[%d行]"
#define IDS_ERR_REBEGIN             "[%s][%s]多个BEGIN[%d行]"
#define IDS_ERR_NOBEGIN             "[%s][%s]期待BEGIN[%d行]"
#define IDS_ERR_ERR_KEY             "[%s][%s]没有定义关键字[%s][%d行]"
#define IDS_ERR_REQUIRED_KEY        "[%s][%s]没有定义[%s][%d行]"



static CLogManage s_gclog;
void log_write(const char *ThisModule, int Err, LPCSTR fmt, ...)
{
    s_gclog.SetLogFile(LOGFILE, ThisFile, "PTR");
    char buf[4096] = {0};

    va_list vl;
    va_start(vl, fmt);
    vsprintf(buf, fmt, vl);
    va_end(vl);
    s_gclog.Log(ThisModule, Err, buf);
}

/*********************全局变量***********************************************/
SIZE g_sizeTwipsPerRowCol = {1, 1};
/*********************全局函数声明********************************************/
static const char *ItemTypeToDesc(ITEMTYPE type);
int ReadAItem(char *&p, char *&pBegin, BOOL bKey, BOOL bSpanLines, int &nLine);
int ReadALine(char *&p, char *pResults[], int nMaxCount, int &nLine);
void ReplaceEsc(char *p);
/*********************全局函数实现********************************************/
// 功能：以SepStr为分隔符，分字串s为多个字串
// 输入：s, 要分的字串
//      SepStr, 分隔字串
// 返回：字串列表
STRLIST Split(LPCSTR s, LPCSTR SepStr)
{
    STRLIST sl;
    const char *p = s;
    const char *pStart = NULL;
    while (*p)
    {
        BOOL bFound;
        if (NULL == strchr(SepStr, *p))
        {
            bFound = FALSE;
        }
        else
        {
            bFound = TRUE;
        }

        if (!pStart)
        {
            if (!bFound)
            {
                pStart = p;
            }
        }
        else
        {
            if (bFound)
            {
                sl.push_back(string(pStart, p - pStart));
                pStart = NULL;
            }
        }
        p++;
    }

    if (pStart)
    {
        sl.push_back(pStart);
    }

    return sl;
}
// 功能：转换ITEM类型为ITEM描述
static const char *ItemTypeToDesc(ITEMTYPE type)
{
    switch (type)
    {
    case ITEM_FORM: return "FORM";
    case ITEM_SUBFORM: return "SUBFORM";
    case ITEM_FRAME: return "FRAME";
    case ITEM_FIELD: return "FIELD";
    case ITEM_MEDIA: return "MEDIA";
    default: return "NONE";
    }
}
// 功能：读一个ITEM，pBegin指向这个ITEM，p指向ITEM结束
// 输入：p, 指向要读的字串
//      pBegin, 返回时为读到的ITEM的开始
//      bKey, 是否是关键字
//      bSpanLines, 是否跨行读
//      nLine, 当前行数，读到\n时加1
// 返回：1，成功；0，失败
int ReadAItem(char *&p, char *&pBegin, BOOL bKey, BOOL bSpanLines, int &nLine)
{
    //Q_UNUSED(bKey);
    pBegin = NULL;
    bool bQuot = false;
    while (*p)
    {
        if ('\n' == (*p))
        {
            if (!bSpanLines || pBegin)
            {
                break;
            }
            nLine++;
            p++;
            continue;
        }
        bool bSpace = (' ' == *p || '\t' == *p  || '\r' == *p);
        bool bComma = (',' == *p);
        bool bComment = ('/' == p[0] && '/' == p[1]);
        if (!pBegin)
        {
            if (bComment)
            {
                while ('\0' != *p  && '\r' != *p  && '\n' != *p)
                {
                    p++;
                }
                break;
            }
            if (!bSpace && !bComma)
            {
                if ('"' ==  p[0])
                {
                    bQuot = true;
                    p++;
                }
                pBegin = p;
            }
        }
        else if (bQuot)
        {
            if ('"' ==  p[0])
            {
                break;
            }
        }
        else if (bComment)
        {
            while ('\0' != *p && '\r' != *p && '\n' != *p)
            {
                p++;
            }
            break;
        }
        else if (bSpace || bComma)
        {
            break;
        }
        p++;
    }

    if (pBegin)
    {
        return 1;
    }

    return 0;
}
// 功能：读一行字串，p指向新的一行
// 输入：p, 指向要读的字串
//      pResults，保存读到的ITEM
//      nMaxCount, pResults能保存的最大个数
//      nLine, 当前行数，读到\n时加1
// 返回：读到的ITEM个数
int ReadALine(char *&p, char *pResults[], int nMaxCount, int &nLine)
{
    //读第一个元素
    int nRet = ReadAItem(p, pResults[0], true, true, nLine);
    if (0 >= nRet)
    {
        return nRet;
    }
    if (0 == p[0] || '\n' == p[0])
    {
        p[0] = 0;
        p++;
        return 1;
    }
    p[0] = 0;
    p++;
    int nCount = 1;
    while (nCount < nMaxCount)
    {
        nRet = ReadAItem(p, pResults[nCount], false, false, nLine);
        if (0 > nRet)
        {
            return nRet;
        }
        if (0 == nRet)
        {
            return nCount;
        }
        nCount++;
        if (0 == p[0] || '\n' == p[0])
        {
            p[0] = 0;
            p++;
            ReplaceEsc(pResults[nCount - 1]);
            return nCount;
        }
        p[0] = 0;
        ReplaceEsc(pResults[nCount - 1]);
        p++;
    }

    return nCount;
}
//功能：代替C语言中的转义字符
//输入：p，指向要替换的字串
void ReplaceEsc(char *p)
{
    static char a[] = {'a', 'b', 'f', 'n', 'r', 't', 'v', '\'', '"', '\\', '?'};
    static char b[] = {'\a', '\b', '\f', '\n', '\r', '\t', '\v', '\'', '"', '\\', '?'};
    char *pd = p;
    int i ;
    while (*p)
    {
        if ('\\' == *p)
        {
            for (i = 0; i < (int)(sizeof(a) / sizeof(a[0])); i++)
            {
                if (p[1] == a[i])
                {
                    *pd++ = b[i];
                    p += 2;
                    break;
                }
            }
            if (i != sizeof(a) / sizeof(a[0]))
            {
                break;
            }
            if ((p[1] == 'x' || p[1] == 'X') &&
                isxdigit(p[2]) && isxdigit(p[3]))
            {
                *pd++ = XDIGIT(p[2]) * 16 + XDIGIT(p[3]);
                p += 4;
                break;
            }
        }
        pd++;
        p++;
    }

    while (*p)
    {
        if ('\\' == *p)
        {
            for (i = 0; i < (int)(sizeof(a) / sizeof(a[0])); i++)
            {
                if (p[1] == a[i])
                {
                    *pd++ = b[i];
                    p += 2;
                    break;
                }
            }
            if (i != sizeof(a) / sizeof(a[0]))
            {
                continue;
            }
            else if ((p[1] == 'x' || p[1] == 'X') &&
                     isxdigit(p[2]) && isxdigit(p[3]))
            {
                *pd++ = XDIGIT(p[2]) * 16 + XDIGIT(p[3]);
                p += 4;
            }
            else
            {
                *pd++ = *p++;
            }
        }
        else
        {
            *pd++ = *p++;
        }
    }
    *pd = 0;
}

KeyRule::KeyRule(LPCSTR szName, BOOL bRequired, ITEMTYPE type)
{
    m_Name      = szName;
    m_bRequired = bRequired;
    m_ItemType  = type;
}
KeyRule::~KeyRule()
{}
int KeyRule::GetAttrCount() const
{
    return m_Attrs.size();
}
KeyRule  &KeyRule::AddAttr(int nOffset, int nLen, LPCSTR szVerify, BOOL bRequired)
{
    m_Attrs.push_back(ATTR());
    ATTR &a     = m_Attrs.back();
    a.bString   = (nLen == 0);
    a.nOffset   = nOffset;
    a.nLen      = nLen;
    a.bVerifyOR = false;
    a.bRequired = bRequired;
    BOOL b_str;
    if (szVerify)
    {
        a.Verify    = Split(szVerify, "|, \t\r\n");
        if (NULL == strchr(szVerify, '|'))
        {
            b_str = FALSE;
        }
        else
        {
            b_str = TRUE;
        }
        a.bVerifyOR = b_str;
    }
    return *this;
}

KeyRules::KeyRules()
{
}
KeyRules::~KeyRules()
{
    RLIT it;
    for (it = m_Rules.begin(); it != m_Rules.end(); it++)
    {
        delete (*it);
    }
    m_Rules.clear();
}
KeyRule &KeyRules::AddRule(LPCSTR szName, BOOL bRequired, ITEMTYPE type)
{
    m_Rules.push_back(new KeyRule(szName, bRequired, type));
    return *m_Rules.back();
}
DWORD KeyRules::size() const
{
    return m_Rules.size();
}

//------------------ CSPPrinterItem实现 ------------------
// 功能：从字串p中装入ITEM的属性
// 输入：p, 输入字串
//      nLine, 行数
//      pRule, 关键字的定义
//      pResults, 读到数据
//      ResultCount, 读到数据的条数
// 返回：TRUE，成功；否FALSE，失败
template <class T>
BOOL CSPPrinterItem<T>::LoadAttrs(char *&p, int &nLine, KeyRule *pRule, char *pResults[], int nResultCount)
{
    //Q_UNUSED(p);
    const char *const ThisModule = "ITEM.LoadAttrs";

    CSPPrinterItem<T> *pThis = this;
    KeyRule::ATTRLIST &al = pRule->m_Attrs;
    KeyRule::ALIT itAttr;
    int nAttr;
    SLIT itString;
    for (nAttr = 1, itAttr = al.begin(); itAttr != al.end(); nAttr++, itAttr++)
    {
        if (nAttr >= nResultCount)  // 有的属性没有设置
        {
            if (itAttr->bRequired)  // 该属性要求设置
            {
                log_write(ThisModule, -1, IDS_ERR_REQUIRED_ATTR,
                          ItemTypeToDesc(GetItemType()), GetName(),
                          pRule->m_Name.c_str(), nAttr + 1, nLine);
                return FALSE;
            }
            continue;
        }
        if (itAttr->bString)
        {
            *(string *)(((long)pThis) + itAttr->nOffset) = pResults[nAttr];
        }
        else
        {
            DWORD dwValue = 0;
            if (0 < itAttr->Verify.size())
            {
                STRLIST sl = Split(pResults[nAttr], " \t|,");
                for (SLIT itSrc = sl.begin(); itSrc != sl.end(); itSrc++)
                {
                    for (itString = itAttr->Verify.begin(); itString != itAttr->Verify.end(); itString++)
                    {
                        if ((*itString) == (*itSrc))
                        {
                            break;
                        }
                    }
                    if (itString == itAttr->Verify.end())
                    {
                        return FALSE;
                    }
                    if (itAttr->bVerifyOR)
                    {
                        dwValue |= 1 << (itString - itAttr->Verify.begin());
                    }
                    else
                    {
                        dwValue = itString - itAttr->Verify.begin();
                    }
                }
            }
            else
            {
                //              char *endptr = NULL;
                //              dwValue = strtoul(pResults[nAttr], &endptr, 0);
                //              if (dwValue == ULONG_MAX)
                //              {
                //                  if (errno == ERANGE)
                //                  {
                //                      Log(ThisModule, errno, IDS_ERR_ATTR_INVALID_VALUE, pResults[nAttr], itAttr->nLen, errno);
                //                      return FALSE;
                //                  }
                //              }
                //配置media中UNIT关键字base的值为负数，执行WFS_INF_PTR_QUERY_MEDIA返回结果错误
                errno = 0;

                char *endptr = NULL;
                dwValue = strtoul(pResults[nAttr], &endptr, 0);
                if (ERANGE == errno)
                {
                    log_write(ThisModule, errno, IDS_ERR_ATTR_INVALID_VALUE,
                              pResults[nAttr], itAttr->nLen, errno);
                    return FALSE;
                }
                if (LONG_MAX < dwValue)
                {
                    log_write(ThisModule, errno, IDS_ERR_ATTR_INVALID_VALUE, \
                              pResults[nAttr], itAttr->nLen, errno);
                    return FALSE;
                }
                //  modify end
                else if (0 == dwValue)
                {
                    if ('\0' != (*endptr))
                    {
                        log_write(ThisModule, -1, IDS_ERR_ATTR_INVALID_VALUE, \
                                  pResults[nAttr], itAttr->nLen, -1);
                        return FALSE;
                    }
                }
            }
            switch (itAttr->nLen)
            {
            case 1:
                {
                    if (255 < dwValue)
                    {
                        log_write(ThisModule, -1, IDS_ERR_ATTR_INVALID_VALUE, \
                                  pResults[nAttr], itAttr->nLen, -1);
                        return FALSE;
                    }
                    *(BYTE *)((long)(pThis) + itAttr->nOffset) = (BYTE)dwValue;
                }
                break;
            case 2:
                {
                    if (65535 < dwValue)
                    {
                        log_write(ThisModule, -1, IDS_ERR_ATTR_INVALID_VALUE, \
                                  pResults[nAttr], itAttr->nLen, -1);
                        return FALSE;
                    }
                    *(WORD *)((long)(pThis) + itAttr->nOffset) = (WORD)dwValue;
                }
                break;
            case 4:
                {
                    *(DWORD *)((long)(pThis) + itAttr->nOffset) = (DWORD)dwValue;
                }
                break;
            case 8:
                {
                    *(long *)((long)(pThis) + itAttr->nOffset) = (long)dwValue;
                }
                break;
            default:
                {
                    assert(false);
                }

            }
        }
    }

    return TRUE;
}
// 功能：从字串p中装入ITEM
// 输入：p, 输入字串
//      lpszName, ITEM名，如果为NULL，从p中读出
//      nLine, 行数
// 返回：TRUE，成功；否FALSE，失败
template <class T>
BOOL CSPPrinterItem<T>::Load(char *&p, const char *lpszName, int &nLine)
{
    const char *const ThisModule = "ITEM.Load";
    // CSPPrinterItem *pThis = this;
    KeyRules::RULELIST &rl = GetKeyRules()->m_Rules;
    char *pResults[5];
    int nRet;
    // 读名字
    if (lpszName)
    {
        m_Name = lpszName;
    }
    else
    {
        if (2 > (nRet = ReadALine(p, pResults, 5, nLine)))
        {
            log_write(ThisModule, -1, IDS_ERR_READ_NAME, \
                      ItemTypeToDesc(GetItemType()), nLine);
            return FALSE;
        }
        m_Name = pResults[1];
    }

    bool bBegin = false;
    string s(rl.size(), '0');
    while (1)
    {
        // 如果一个Item已经开始，但还没到结束就发现已经没内容
        // 此时加载必须以失败结束，否则可能进入死循环。
        if (bBegin && NULL == *p)
        {
            return FALSE;
        }

        if (0 >= (nRet = ReadALine(p, pResults, 5, nLine)))
        {
            continue;
        }
        if (0 == strcmp(pResults[0], "END"))
        {
            break;
        }
        if (0 == strcmp(pResults[0], "BEGIN"))
        {
            if (!bBegin)
            {
                bBegin = true;
            }
            else
            {
                log_write(ThisModule, -1, IDS_ERR_REBEGIN, \
                          ItemTypeToDesc(GetItemType()), GetName(), nLine);
            }
            continue;
        }

        if (!bBegin)
        {
            log_write(ThisModule, -1, IDS_ERR_NOBEGIN, \
                      ItemTypeToDesc(GetItemType()), GetName(), nLine);
            return FALSE;
        }

        KeyRules::RLIT it;
        for (it = rl.begin(); it != rl.end(); it++)
        {
            if ((*it)->m_Name == pResults[0])
            {
                break;
            }
        }
        if (it == rl.end())
        {
            log_write(ThisModule, -1, IDS_ERR_ERR_KEY, \
                      ItemTypeToDesc(GetItemType()), GetName(), pResults[0], nLine);
            return FALSE;
        }
        s[it - rl.begin()] = '1';
        if (ITEM_NONE != (*it)->m_ItemType) // 子ITEM
        {
            assert(GetItemType() != ITEM_FIELD);
            assert((*it)->m_Attrs.size() > 0);
            if (2 > nRet)
            {
                return FALSE;
            }
            switch ((*it)->m_ItemType)
            {
            case ITEM_FORM:
                {
                    assert(false);
                }
                break;
            case ITEM_SUBFORM:
                {
                    CSPPrinterItem<ISPPrinterSubform> *pNew = new CSPPrinterSubform(m_pUnitItem);
                    ((IL *)((long)(this) + (*it)->m_Attrs[0].nOffset))->push_back(pNew);
                    if (!pNew->Load(p, pResults[1], nLine))
                    {
                        return FALSE;
                    }
                }
                break;
            case ITEM_FRAME:
                {
                    CSPPrinterItem<ISPPrinterFrame> *pNew = new CSPPrinterFrame(m_pUnitItem);
                    ((IL *)((long)(this) + (*it)->m_Attrs[0].nOffset))->push_back(pNew);
                    if (!pNew->Load(p, pResults[1], nLine))
                    {
                        return FALSE;
                    }
                }
                break;
            case ITEM_FIELD:
                {
                    CSPPrinterItem<ISPPrinterField> *pNew = new CSPPrinterField(m_pUnitItem);

                    ((IL *)((long)(this) + (*it)->m_Attrs[0].nOffset))->push_back(pNew);

                    if (!pNew->Load(p, pResults[1], nLine))
                    {
                        return FALSE;
                    }
                }
                break;
            default:
                {}
            }
            continue;
        }

        if (!LoadAttrs(p, nLine, (*it), pResults, nRet))
        {
            return FALSE;
        }
    }
    // 查看s中是否有必须设置而用户没有设置的值
    for (int i = 0; i < s.size(); i++)
    {
        if (s[i] == '0' && rl[i]->m_bRequired)
        {
            log_write(ThisModule, -1, IDS_ERR_REQUIRED_KEY, \
                      ItemTypeToDesc(GetItemType()), GetName(), rl[i]->m_Name.c_str(), nLine);
            return FALSE;
        }
    }

    return TRUE;
}

BOOL CSPPrinterForm::Load(char *&p, const char *lpszName, int &nLine)
{
    m_bLoadSucc = false;
    if (CSPPrinterItem<ISPPrinterForm>::Load(p, lpszName, nLine))
    {
        m_bLoadSucc = true;
        return TRUE;
    }
    return FALSE;
}

void CSPPrinterMedia::UpdatePrintAreaFromSize()
{
    if (0 >=  m_PrintArea.right - m_PrintArea.left)
    {
        m_PrintArea.right = m_Size.cx;
    }
    if (0 >= m_PrintArea.bottom - m_PrintArea.top)
    {
        m_PrintArea.bottom = m_Size.cy;
    }
}
BOOL CSPPrinterMedia::Load(char *&p, const char *lpszName, int &nLine)
{
    m_bLoadSucc = false;
    if (CSPPrinterItem<ISPPrinterMedia>::Load(p, lpszName, nLine))
    {
        m_bLoadSucc = true;
        return TRUE;
    }
    return FALSE;
}

static KeyRules SubformKeyRules;
static KeyRules FormKeyRules;
static KeyRules FieldKeyRules;
static KeyRules FrameKeyRules;
static KeyRules MediaKeyRules;

KeyRules *CSPPrinterSubform::GetKeyRules()
{
    if (0 == SubformKeyRules.size())
    {
        SubformKeyRules.AddRule("POSITION").AAL(m_Position.cx).AAL(m_Position.cy).AALVR(m_Position.cz, NULL, false);
        SubformKeyRules.AddRule("SIZE").AAL(m_Size.cx).AAL(m_Size.cy);
        SubformKeyRules.AddRule("XFSFIELD", FALSE, ITEM_FIELD).AAL(m_Children);
        SubformKeyRules.AddRule("XFSFRAME", FALSE, ITEM_FIELD).AAL(m_Children);
    }
    return &SubformKeyRules;
}
KeyRules *CSPPrinterForm::GetKeyRules()
{
    if (0 == FormKeyRules.size())
    {
        FormKeyRules.AddRule("UNIT").AALV(m_Base, "INCH,MM,ROWCOLUMN").AAL(m_BaseSize.cx).AAL(m_BaseSize.cy);
        FormKeyRules.AddRule("SIZE").AAL(m_Size.cx).AAL(m_Size.cy);
        FormKeyRules.AddRule("ALIGNMENT", FALSE).AALV(m_Align, "TOPLEFT,TOPRIGHT,BOTTOMLEFT,BOTTOMRIGHT").AAL(m_Position.cx).AAL(m_Position.cy);
        FormKeyRules.AddRule("ORIENTATION", FALSE).AALV(m_Orientation, "PORTRAIT,LANDSCAPE");
        FormKeyRules.AddRule("SKEW", FALSE).AAL(m_Skew);
        FormKeyRules.AddRule("VERSION", FALSE).AAL(m_VerMajor).AAL(m_VerMinor).AASR(m_Date, false).AASR(m_Author, false);
        FormKeyRules.AddRule("LANGUAGE").AAL(m_LangID);
        FormKeyRules.AddRule("COPYRIGHT", FALSE).AAS(m_Copyright);
        FormKeyRules.AddRule("TITLE", FALSE).AAS(m_Title);
        FormKeyRules.AddRule("COMMENT", FALSE).AAS(m_Comment);
        FormKeyRules.AddRule("USERPROMPT", FALSE).AAS(m_Prompt);
        FormKeyRules.AddRule("XFSFIELD", FALSE, ITEM_FIELD).AAL(m_Children);
        FormKeyRules.AddRule("XFSFRAME", FALSE, ITEM_FRAME).AAL(m_Children);
        FormKeyRules.AddRule("XFSSUBFORM", FALSE, ITEM_SUBFORM).AAL(m_Children);
    }
    return &FormKeyRules;
}
KeyRules *CSPPrinterField::GetKeyRules()
{
    if (FieldKeyRules.size() == 0)
    {
        // 装载Printable
        FieldKeyRules.AddRule("HEADER", FALSE).AAS(m_Header);
        FieldKeyRules.AddRule("FOOTER", FALSE).AAS(m_Footer);
        FieldKeyRules.AddRule("SIDE", FALSE).AALV(m_bBack, "FRONT,BACK");
        FieldKeyRules.AddRule("CLASS", FALSE).AALV(m_Class, "STATIC,OPTIONAL,REQUIRED");
        FieldKeyRules.AddRule("OVERFLOW", FALSE).AALV(m_Overflow, "TERMINATE,TRUNCATE,BESTFIT,OVERWRITE,WORDWRAP");
        FieldKeyRules.AddRule("HORIZONTAL", FALSE).AALV(m_HAlign, "LEFT,RIGHT,CENTER,JUSTIFY");
        FieldKeyRules.AddRule("VERTICAL", FALSE).AALV(m_VAlign, "BOTTOM,CENTER,TOP");
        FieldKeyRules.AddRule("COLOR", FALSE).AALV(m_SolidColor, "BLACK,WHITE,GRAY,RED,BLUE,GREEN,YELLOW");
        FieldKeyRules.AddRule("RGBCOLOR", FALSE).AAL(m_ColorR).AAL(m_ColorG).AAL(m_ColorB);
        FieldKeyRules.AddRule("LANGUAGE", FALSE).AAL(m_LangID);

        FieldKeyRules.AddRule("POSITION").AAL(m_Position.cx).AAL(m_Position.cy).AALVR(m_Position.cz, NULL, false);
        FieldKeyRules.AddRule("FOLLOWS", FALSE).AAS(m_Follows);
        FieldKeyRules.AddRule("SIZE").AAL(m_Size.cx).AAL(m_Size.cy);
        FieldKeyRules.AddRule("INDEX", FALSE).AAL(m_RepeatCount).AAL(m_RepeatOffset.cx).AAL(m_RepeatOffset.cy);
        FieldKeyRules.AddRule("TYPE", FALSE).AALV(m_FieldType, "TEXT,MICR,OCR,MSF,BARCODE,GRAPHIC,PAGEMARK");
        FieldKeyRules.AddRule("SCALING", FALSE).AALV(m_Scaling, "BESTFIT,ASIS,MAINTAINASPECT");
        FieldKeyRules.AddRule("BARCODE", FALSE).AALV(m_BarcodePos, "NONE,ABOVE,BELOW,BOTH");
        FieldKeyRules.AddRule("ACCESS", FALSE).AALV(m_Access, "NONE,READ,WRITE,READWRITE");
        FieldKeyRules.AddRule("STYLE", FALSE).AALV(m_Style, "NORMAL|BOLD|ITALIC|UNDER|DOUBLEUNDER|DOUBLE|TRIPLE|QUADRUPLE|STRIKETHROUGH|ROTATE90|ROTATE270|UPSIDEDOWN|PROPORTIONAL|DOUBLEHIGH|TRIPLEHIGH|QUADRUPLEHIGH|CONDENSED|SUPERSCRIPT|SUBSCRIPT|OVERSCORE|LETTERQUALITY|NEARLETTERQUALITY|DOUBLESTRIKE|OPAQUE");
        FieldKeyRules.AddRule("CASE", FALSE).AALV(m_Case, "NOCHANGE,UPPER,LOWER");
        FieldKeyRules.AddRule("FONT", FALSE).AAS(m_FontName);
        FieldKeyRules.AddRule("POINTSIZE", FALSE).AAL(m_FontSize);
        FieldKeyRules.AddRule("CPI", FALSE).AAL(m_CPI);
        FieldKeyRules.AddRule("LPI", FALSE).AAL(m_LPI);
        FieldKeyRules.AddRule("FORMAT", FALSE).AAS(m_Format);
        FieldKeyRules.AddRule("INITIALVALUE", FALSE).AAS(m_InitValue);
    }
    return &FieldKeyRules;
}

KeyRules *CSPPrinterFrame::GetKeyRules()
{
    if (0 == FrameKeyRules.size())
    {
        FrameKeyRules.AddRule("HEADER", FALSE).AAS(m_Header);
        FrameKeyRules.AddRule("FOOTER", FALSE).AAS(m_Footer);
        FrameKeyRules.AddRule("SIDE", FALSE).AALV(m_bBack, "FRONT,BACK");
        FrameKeyRules.AddRule("CLASS", FALSE).AALV(m_Class, "STATIC,OPTIONAL,REQUIRED");
        FrameKeyRules.AddRule("OVERFLOW", FALSE).AALV(m_Overflow, "TERMINATE,TRUNCATE,BESTFIT,OVERWRITE,WORDWRAP");
        FrameKeyRules.AddRule("HORIZONTAL", FALSE).AALV(m_HAlign, "LEFT,RIGHT,CENTER,JUSTIFY");
        FrameKeyRules.AddRule("VERTICAL", FALSE).AALV(m_VAlign, "BOTTOM,CENTER,TOP");
        FrameKeyRules.AddRule("COLOR", FALSE).AALV(m_SolidColor, "BLACK,WHITE,GRAY,RED,BLUE,GREEN,YELLOW");
        FrameKeyRules.AddRule("RGBCOLOR", FALSE).AAL(m_ColorR).AAL(m_ColorG).AAL(m_ColorB);
        FrameKeyRules.AddRule("LANGUAGE", FALSE).AAL(m_LangID);

        FrameKeyRules.AddRule("POSITION").AAL(m_Position.cx).AAL(m_Position.cy).AALVR(m_Position.cz, NULL, false);
        FrameKeyRules.AddRule("SIZE").AAL(m_Size.cx).AAL(m_Size.cy);
        FrameKeyRules.AddRule("FRAMES", FALSE).AAS(m_Frames);
        FrameKeyRules.AddRule("REPEATONX", FALSE).AAL(m_RepeatCount.cx).AAL(m_RepeatOffset.cx);
        FrameKeyRules.AddRule("REPEATONY", FALSE).AAL(m_RepeatCount.cy).AAL(m_RepeatOffset.cy);
        FrameKeyRules.AddRule("TYPE", FALSE).AALV(m_FrameType, "RECTANGLE,ROUNDED_CORNER,ELLIPSE");
        FrameKeyRules.AddRule("STYLE", FALSE).AALV(m_Style, "SINGLE_THIN,DOUBLE_THIN,SINGLE_THICK,DOUBLE_THICK,DOTTED");
        FrameKeyRules.AddRule("FILLCOLOR", FALSE).AALV(m_FSolidColor, "BLACK,WHITE,GRAY,RED,BLUE,GREEN,YELLOW");
        FrameKeyRules.AddRule("RGBFILLCOLOR", FALSE).AAL(m_FColorR).AAL(m_FColorG).AAL(m_FColorB);
        FrameKeyRules.AddRule("FILLSTYLE", FALSE).AALV(m_FillStyle, "NONE,SOLID,BDIAGONAL,CROSS,DIAGCROSS,FDIAGONAL,HORIZONTAL,VERTICAL");
        FrameKeyRules.AddRule("SUBSTSIGN", FALSE).AAL(m_SubstSign);
        FrameKeyRules.AddRule("TITLE", FALSE).AAS(m_Title);
    }
    return &FrameKeyRules;
}

KeyRules *CSPPrinterMedia::GetKeyRules()
{
    if (0 == MediaKeyRules.size())
    {
        MediaKeyRules.AddRule("TYPE", FALSE).AALV(m_MediaType, "GENERIC,PASSBOOK,MULTIPART");
        MediaKeyRules.AddRule("SOURCE", FALSE).AALV(m_PaperSource, "NONE,ANY,UPPER,LOWER,EXTERNAL,AUX,AUX2,PARK");
        MediaKeyRules.AddRule("UNIT").AALV(m_Base, "INCH,MM,ROWCOLUMN").AAL(m_BaseSize.cx).AAL(m_BaseSize.cy);
        MediaKeyRules.AddRule("SIZE").AAL(m_Size.cx).AAL(m_Size.cy);
        MediaKeyRules.AddRule("PRINTAREA", FALSE).AAL(m_PrintArea.left).AAL(m_PrintArea.top).AAL(m_PrintArea.right).AAL(m_PrintArea.bottom);
        MediaKeyRules.AddRule("RESTRICTED", FALSE).AAL(m_RestrictedArea.left).AAL(m_RestrictedArea.top).AAL(m_RestrictedArea.right).AAL(m_RestrictedArea.bottom);
        MediaKeyRules.AddRule("FOLD", FALSE).AALV(m_PassbookFold, "NONE,HORIZONTAL,VERTICAL");
        MediaKeyRules.AddRule("STAGGERING", FALSE).AAL(m_Staggering);
        MediaKeyRules.AddRule("PAGE", FALSE).AAL(m_PageCount);
        MediaKeyRules.AddRule("LINES", FALSE).AAL(m_LineCount);
        MediaKeyRules.AddRule("LINESPACE", FALSE).AAL(m_LineSpace);
        MediaKeyRules.AddRule("PAGE2START", FALSE).AAL(m_Page2Start);

    }

    return &MediaKeyRules;
}

