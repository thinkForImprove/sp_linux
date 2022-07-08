#pragma once
#include "ILogWrite.h"
#include "XFSPTR.H"
#include "PTRDec.h"
#include "MultiString.h"
#include "PTRData.h"
#include <string.h>
#include "cjson_object.h"
// 接口类
#define NOVTABLE    __declspec(novtable)
typedef enum OVERFLOW tagOVERFLOW;

//字段风格
#define     FS_NORMAL            0x00000001
#define     FS_BOLD              0x00000002
#define     FS_ITALIC            0x00000004
#define     FS_UNDER             0x00000008 // (single underline)
#define     FS_DOUBLEUNDER       0x00000010 // (double underline)
#define     FS_DOUBLE            0x00000020 // (double width)
#define     FS_TRIPLE            0x00000040 // (triple width)
#define     FS_QUADRUPLE         0x00000080 // (quadruple width)
#define     FS_STRIKETHROUGH     0x00000100
#define     FS_ROTATE90          0x00000200 // (rotate +90 degrees clockwise)
#define     FS_ROTATE270         0x00000400 // (rotate +270 degrees clockwise)
#define     FS_UPSIDEDOWN        0x00000800 // (upside down)
#define     FS_PROPORTIONAL      0x00001000 // (proportional spacing)
#define     FS_DOUBLEHIGH        0x00002000
#define     FS_TRIPLEHIGH        0x00004000
#define     FS_QUADRUPLEHIGH     0x00008000
#define     FS_CONDENSED         0x00010000
#define     FS_SUPERSCRIPT       0x00020000
#define     FS_SUBSCRIPT         0x00040000
#define     FS_OVERSCORE         0x00080000
#define     FS_LETTERQUALITY     0x00100000
#define     FS_NEARLETTERQUALITY 0x00200000
#define     FS_DOUBLESTRIKE      0x00400000
#define     FS_OPAQUE            0x00800000 // (If omitted then default attribute is transparent)

#define MAX_ITEM        (1024)
#define MAX_FIELD_SIZE  (500)

#define MAX_PRINTDATA_LEN (65535)

#pragma pack(push, 4)
typedef struct tagPrintContext
{
    ISPPrinterMedia     *pMedia;
    ISPPrinterForm      *pForm;
    ISPPrinterSubform   *pSubform;
    ISPPrinterFrame     *pFrame;
    ISPPrinterField     *pField;
    LPWFSPTRPRINTFORM   pPrintData;     // PrintForm命令入参
    BOOL                bCancel;    // 是否取消，EndForm可以用来决定是否继续处理数据
    LPVOID              pUserData;  // 用户数据，由SP设备类维护，可在StartForm、EndForm和Print中使用。
} PrintContext;

typedef struct tagReadContext
{
    ISPPrinterMedia *pMedia;
    ISPPrinterForm *pForm;
    ISPPrinterSubform *pSubform;
    ISPPrinterFrame *pFrame;
    ISPPrinterField *pField;
    LPWFSPTRREADFORM pReadData;
    BOOL bCancel;      //是否取消，EndReadForm可以用来决定是否继续处理数据
    LPVOID pUserData;  //用户数据，可在StartReadForm、EndReadForm中使用。
} ReadContext;
#pragma pack(pop)

/*struct PRINT_ITEM   // 打印的一项内容
{
    int x, y;       // 打印的开始坐标，从0开始
    int nWidth;     // 打印的宽度
    int nHeight;     // 打印的高度
    int nTextLen;   // 文本长度，对文本字段，如果无格式内容，与nWidth相同
    FIELDTYPE nFieldType;   // 域类型
    char      strImagePath[1024]; // 图像路径
    int       nDstImgWidth; // 期望的图片的宽度  0-原宽度
    int       nDstImgHeight;// 期望的图片的高度  0-原高度
    char Text[1];   //文本内容
};*/

struct PRINT_ITEM  // 打印的一项内容
{
    int x, y;                 // 打印的开始坐标，从0开始
    DWORD dwCPI, dwLPI;       // 行间距和字间距
    DWORD dwColor;            // 字体颜色
    DWORD dwStyle;            // 字体风格
    int nWidth;               // 打印的宽度
    int nHeight;              // 打印的高度
    int nTextLen;             // 文本长度，对文本字段，如果无格式内容，与nWidth相同
    FIELDTYPE nFieldType;     // 域类型
    char strImagePath[1024];  // 图像路径
    int nDstImgWidth;         // 期望的图片的宽度  0-原宽度
    int nDstImgHeight;        // 期望的图片的高度  0-原高度
    char strFieldName[MAX_FIELD_SIZE];    // 域名
    DWORD dwFontSize;         // 字体大小
    char strFontName[MAX_FIELD_SIZE];     // 字库名
    char Text[1];             // 文本内容
    int nSizeX;
    int nSizeY;
};

struct PRINT_ITEMS  // 所有打印内容
{
    PRINT_ITEMS()
    {
        nItemNum = 0;
        memset(pItems, 0, sizeof(pItems));
    }

    ~PRINT_ITEMS()
    {
        for (int i = 0; i < nItemNum; i++)
        {
            delete [](char *)pItems[i];
        }
    }

    int nItemNum;                   // 打印项数
    PRINT_ITEM *pItems[MAX_ITEM];   //所有打印项
};

struct PRINT_STRING // 管理打印字串的类
{
    PRINT_STRING()
    {
        m_nCurRow = m_nCurCol = 0;
        m_nAllocLen = 8192;
        m_pData = new char[m_nAllocLen];
        m_pData[0] = 0;
        m_nDataLen = 0;
    }

    ~PRINT_STRING()
    {
        delete [] m_pData;
        m_pData = NULL;
    }

    PRINT_STRING &Append(char c)
    {
        char buf[2] = {c, 0};
        return Append(buf);
    }

    PRINT_STRING &Append(int i)
    {
        char buf[5];
        *((int *)buf) = i;
        return Append(buf, 4);
    }

    PRINT_STRING &Append(const char *pStr)
    {
        return Append(pStr, strlen(pStr));
    }

    PRINT_STRING &Append(const char *pStr, int nLen)
    {
        if (0 == nLen)
        {
            return *this;
        }
        if (m_nDataLen + nLen >= m_nAllocLen)
        {
            int nNewLen = m_nAllocLen;
            if (nNewLen <= m_nDataLen + nLen)
            {
                nNewLen *= 2;
            }
            char *pData = new char[nNewLen];
            if (m_nDataLen > 0)
            {
                memcpy(pData, m_pData, m_nDataLen);
            }
            delete [] m_pData;
            m_nAllocLen = nNewLen;
        }
        memcpy(m_pData + m_nDataLen, pStr, nLen);
        m_nDataLen += nLen;
        m_pData[m_nDataLen] = 0;
        return *this;
    }

    inline const char *GetData() const { return m_pData; }
    inline int GetLen() const { return m_nDataLen; }

    int m_nCurRow;  //当前行
    int m_nCurCol;  //当前列
protected:

    int m_nAllocLen;    //分配长度
    int m_nDataLen; //数据长度
    char *m_pData;  //数据
};


#define GetRValue(rgb)  (LOBYTE(rgb))
#define GetGValue(rgb)  (LOBYTE(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)  (LOBYTE((rgb)>>16))


// 功能：从pStr中前nLen个字符的有效子串长度。
//      如果最后一个字符（第nLen-1字符）是ANSI字符，返回nLen；
//      如果最后一个字符是一个汉字的第一个字节，返回nLen-1（不包括最后半个汉字）
//      如果最后一个字符是汉字的第二个字节，返回nLen。
// 返回：nLen或nLen-1
static int SubStrLen(const char *pStr, int nLen)
{
    const BYTE *p = (const BYTE *)pStr;
    if (0 == nLen)
    {
        return 0;
    }
    int nFlag = 0;  // 汉字标字，0 ANSI字符；1 汉字第一个字节；2 汉字第2个字节
    for (int i = 0; i < nLen; i++)
    {
        if (0x80 > p[i])    // 当前字符是ANSI字符
        {
            nFlag = 0;
        }
        else                // 当前字符是汉字
        {
            switch (nFlag)
            {
            case 0:         // 前面一个是ANSI字符
            case 1:         // 前面一个是汉字第一个字节，当前是第二个字节
                nFlag++;
                break;
            default:        // 以前已经是汉字的第2个字节
                nFlag = 1;
                break;
            }
        }
    }
    switch (nFlag)
    {
    case 1:     // 最后一个是汉字第一个字节
        return nLen - 1;
    case 0:     // 最后一个是ANSI字符
    case 2:     // 最后一个是汉字第一个字节
    default:
        return nLen;
    }
}

static char *strupr(char *str)
{
    char *ptr = str;
    while ('\0' != *ptr)
    {
        if (islower(*ptr))
        {
            *ptr = toupper(*ptr);
        }
        ptr++;
    }
    return str;
}

static char *strlwr(char *str)
{
    char *ptr = str;
    while ('\0' != *ptr)
    {
        if (isupper(*ptr))
        {
            *ptr = tolower(*ptr);
        }
        ptr++;
    }
    return str;
}

enum PTR_TYPE
{
    PTR_TYPE_RECEIPT = 1,
    PTR_TYPE_JOURNAL = 2,
};

struct stPTRConfig
{
    PTR_TYPE             type;
    char                 szDevDllName[256];
    int                  nDriverType;
    bool                 bDetectBlackStripe;
    int                  nVerifyField;
    int                  nFeed;
    int                  nPageSize;
    int                  nPageLine;
    DWORD                dwMarkHeader;
    bool                 bEnableSplit;
    int                  nTakeSleep;
    int                  nLineSize;         // 30-00-00-00(FT#0008)
    char                 cTrackName1[128];
    char                 cTrackName2[128];
    char                 cTrackName3[128];
};

// 指定打印模式细节(调用EndForm函数前时需要使用)
typedef struct st_PrtAlignMode
{
    WORD wPrtTextMode;      // 文字打印方式(与硬件设备关联: 0文字底边与Y坐标齐平/1文字上边与行Y坐标齐平,缺省0)
    WORD wPrtImageMode;     // 图片打印方式(与硬件设备关联: 0图片底边与行Y坐标齐平/1图片上边与行Y坐标齐平,缺省0)
    WORD wImagePosAppoint;  // 图片打印占位方式(0XY坐标指定/1打印空格占位)

    st_PrtAlignMode()
    {
        Clear();
    }
    void Clear()
    {
        // 以新北洋NH80/BKC310为标准初始化
        wPrtTextMode = 0;
        wPrtImageMode = 0;
        wImagePosAppoint = 0;
    }
    void Check()
    {
        if (wPrtTextMode < 0 || wPrtTextMode > 1)
        {
            wPrtTextMode = 0;
        }

        if (wPrtImageMode < 0 || wPrtImageMode > 1)
        {
            wPrtImageMode = 0;
        }

        if (wImagePosAppoint < 0 || wImagePosAppoint > 1)
        {
            wImagePosAppoint = 1;
        }
    }
} STPRTALIGNMODE, *LPSTPRTALIGNMODE;

class CTextPrinter : public CLogManage
{
public:
    CTextPrinter();
    virtual~CTextPrinter();


protected:
    void InitPTRData(LPCSTR lpLogicalName);

    // 子类要实现的函数
    virtual HRESULT PrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint = FALSE) = 0;
    /* 图片打印
        *  nDstWidth  期望宽度
        *  nDstHeight 期望高度
    */
    virtual HRESULT PrintImage(LPCSTR szImagePath, int nDstWidth, int nDstHeight) = 0;
    virtual HRESULT PrintImageOrg(LPCSTR szImagePath, ULONG ulOrgX, ULONG ulOrgY) = 0;
    virtual BOOL NeedFormatString() const  = 0; // 子类继承重写:形成的打印字串是否加入格式化内容(HOST:TRUE/其他:FALSe)
    virtual void FireFieldError(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure) = 0;
    virtual void FireFieldWarning(LPCSTR szFormName, LPCSTR szFieldName, WORD wFailure) = 0;
    virtual HRESULT ControlMedia(DWORD dwControl) = 0;
    virtual long PrintData(const char *pBuffer, DWORD dwSize) = 0;
    virtual HRESULT AddPrintString(const char *pBuffer, DWORD dwSize, BOOL bIsFromPrint, char *pBuffOut, DWORD &dwSizeOut) = 0;
    virtual long PrintMICR(const char *pBuffer, DWORD dwSize);  // 打印磁码:需要则子类继承重写
    virtual HRESULT ReadItem(PRINT_ITEM *pItem);// ReadForm相关:需要则子类继承重写
    virtual HRESULT ReadItem(CMultiString cFieldInfo);// ReadForm相关:需要则子类继承重写
    virtual long PrintData2(const char *pBuffer, unsigned long ulDataLen,LONG ulOrgX, ULONG ulOrgY);


    // 以下为打印支持函数(被PrintForm所调用)
    virtual SIZE GetTwipsPerRowCol();
    virtual SIZE GetPerRowColTwips2MM();
    virtual FLOAT GetPerTwips2MM();
    virtual HRESULT StartForm(PrintContext *pContext);
    virtual HRESULT EndForm(PrintContext *pContext);
    virtual HRESULT EndFormToJSON(PrintContext *pContext, CJsonObject &cJsonData);// PrintForm相关:子类继承重写:根据需要组织JSON串
    virtual HRESULT EndFormToString(PrintContext *pContext, LPSTR lpDataOut);// PrintForm相关:子类继承重写:根据需要组织字符串
    virtual HRESULT StartReadForm(ReadContext *pContext);
    virtual HRESULT EndReadForm(ReadContext *pContext);     // ReadForm相关:需要则子类继承重写
    virtual HRESULT EndReadFormToString(PrintContext *pContext, LPSTR lpDataOut);// ReadForm相关:子类继承重写:根据需要组织字符串
    virtual HRESULT DrawText(PrintContext *pContext, LPCSTR szText, LPRECT pRect);
    virtual HRESULT DrawGraph(PrintContext *pContext, LPCSTR szImagePath, LPRECT pRect, SCALING ScaleMode = SCALING_BESTFIT);
    virtual HRESULT DrawBarcode(PrintContext *pContext, LPCSTR szCode, LPRECT pRect, BARCODEPOS BarcodeMode = BCP_NONE);    
    virtual HRESULT DrawMICR(PrintContext *pContext, LPCSTR szMicr, LPRECT pRect);
    virtual HRESULT DrawFrame(PrintContext *pContext, LPRECT pRect, FRAMETYPE Type);
    virtual HRESULT EndReadForm2(CMultiString lFieldInfo, DWORD dwMediaControl);

    static int ComparePrintItem(const void *elem1, const void *elem2);
    HRESULT PrintFieldOrFrame(PrintContext &pc, ISPPrinterItem *pItem, const SIZE offset, CMultiString &Fields);
    HRESULT ReadFieldOrFrame(ReadContext &pc, ISPPrinterItem *pItem, CMultiString &Fields);
    LPCSTR FindString(CMultiString &ms, LPCSTR lpszName, int index);
    int Result2ErrorCode(HRESULT hResult);

    void SetPrtAlignMode(STPRTALIGNMODE);// 指定打印模式细节(调用EndForm函数前时需要使用)

public:
    // 数据成员
    CSPPtrData                    *m_pData;

private:    
    STPRTALIGNMODE                m_stPrtAlignMode;
};

