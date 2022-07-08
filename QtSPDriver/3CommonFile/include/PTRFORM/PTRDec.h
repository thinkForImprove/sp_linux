#pragma once
#include "QtTypeDef.h"

typedef struct tagRECT
{
    LONG    left;   // 通用: Position X
    LONG    top;    // 通用: Position Y
    LONG    right;  // 通用: Size X + Position X
    LONG    bottom; // 通用: Size Y + Position Y
} RECT, *LPRECT;

typedef const RECT *LPCRECT;

typedef struct tagPOINT
{
    LONG  x;
    LONG  y;
} POINT, *LPPOINT;

typedef struct tagSIZE
{
    LONG        cx;
    LONG        cy;
} SIZE, *PSIZE, *LPSIZE;

typedef SIZE               SIZEL;
typedef SIZE               *PSIZEL, *LPSIZEL;


typedef struct tagSIZE3
{
    LONG cx;
    LONG cy;
    LONG cz;
} SIZE3;



//////////////ITEM

enum ITEMTYPE       // 类型定义
{
    ITEM_NONE = -1,
    ITEM_FORM = 0,  // FORM
    ITEM_SUBFORM,   // 子FORM
    ITEM_FRAME,     // 框架
    ITEM_FIELD,     // 字段域
    ITEM_MEDIA      // MEDIA
};
enum MEDIATYPE
{
    MT_GENERIC = 0,
    MT_MULTIPART,
    MT_PASSBOOK
};
enum PAPERSOURCE
{
    PS_NONE  = 0,
    PS_ANY   = 1,
    PS_UPPER = 2,
    PS_LOWER = 4,
    PS_EXTERNAL = 8,
    PS_AUX  = 16,
    PS_AUX2 = 32,
    PS_PARK = 64
};
enum PASSBOOKFOLD
{
    PF_NONE = 0,
    PF_HORIZONTAL,
    PF_VERTICAL
};
enum FORMBASE         // 基准
{
    FORM_INCH = 0,    // 英寸
    FORM_MM,          // 毫米
    FORM_ROWCOLUMN    // 行列
};
enum FORMALIGN        // 对齐方式
{
    TOPLEFT = 0,
    TOPRIGHT,
    BOTTOMLEFT,
    BOTTOMRIGHT
};
enum FORMORIENTATION
{
    PORTRAIT = 0,     // (default)
    LANDSCAPE
};
enum OVER_FLOW        // 溢出处理方式 // KylinOS中math.h中重复定义,改名为OVER_FLOW // 30-00-00-00(FT#0063)
{
    OF_TERMINATE = 0,   // 终止
    OF_TRUNCATE,        // 舍位
    OF_BESTFIT,         // 最适合的(尽可能将数据应用于字段)
    OF_OVERWRITE,       // 重写(覆盖)
    OF_WORDWRAP         // 自动换行
};
enum HORIZONTAL     // 水平对齐方式
{
    LEFT = 0,           // 左
    RIGHT,              // 右
    HCENTER,            // 中间
    JUSTIFY             //
};
enum VERTICAL
{
    BOTTOM = 0,
    VCENTER,
    TOP
};
enum CLASS
{
    CLASS_STATIC = 0,
    CLASS_OPTIONAL = 1,
    CLASS_REQUIRED
};
enum FIELDTYPE
{
    FT_TEXT = 0,    // 缺省文本
    FT_MICR,        // 磁码
    FT_OCR,         // 文字识别
    FT_MSF,         // 存折
    FT_BARCODE,     // 条码
    FT_GRAPHIC,     // 图片
    FT_PAGEMARK
};
enum SCALING
{
    SCALING_BESTFIT = 0,    // (default) scale to size indicated
    SCALING_ASIS,           // render at native size
    SCALING_MAINTAINASPECT  // scale as close as possible to size indicated while maintaining the aspect ratio and not losing graphic information.
};
enum BARCODEPOS
{
    BCP_NONE    = 0,       // (default)
    BCP_ABOVE,
    BCP_BELOW,
    BCP_BOTH
};
enum ACCESS
{
    ACCESS_READ = 1,
    ACCESS_WRITE = 2,     // (default)
    ACCESS_READWRITE = 3
};
enum CASE
{
    CASE_NOCHANGE = 0,    // (default)
    CASE_UPPER,
    CASE_LOWER
};
enum FRAMETYPE
{
    RECTANGLE = 0,     // (default)
    ROUNDED_CORNER,
    ELLIPSE
};
enum FILLSTYLE
{
    FILL_NONE = 0,      // (default)
    FILL_SOLID,         // Solid color
    FILL_BDIAGONAL,     // Downward hatch (left to right) at 45 degrees
    FILL_CROSS,         // Horizontal and vertical crosshatch
    FILL_DIAGCROSS,     // Crosshatch at 45 degrees
    FILL_FDIAGONAL,     // Upward hatch (left to right) at 45 degrees
    FILL_HORIZONTAL,    // Horizontal hatch
    FILL_VERTICAL       // Vertical hatch
};


typedef DWORD  COLORREF;



struct  ISPPrinterItem
{
    virtual ITEMTYPE GetItemType() const = 0;
    virtual LPCSTR GetName() const = 0;
    virtual SIZE GetSize() const  = 0;      // SIZE:X,Y
    virtual SIZE GetOrigSize() const = 0;
    virtual SIZE3 GetPosition() const = 0;
    virtual SIZE3 GetOrigPosition() const = 0;
    virtual void Release() = 0;
};
struct  ISPPrinterContainerItem
{
    virtual DWORD GetSubItemCount() const = 0;
    virtual ISPPrinterItem *GetSubItem(DWORD index) const = 0;
};
struct  ISPPrinterMedia : public ISPPrinterItem
{
    virtual BOOL IsLoadSucc() const = 0;
    virtual MEDIATYPE GetMediaType() const = 0;
    virtual PAPERSOURCE GetPaperSource() const = 0;
    virtual FORMBASE GetOrigUNIT(SIZE *pSize) const = 0;
    virtual void GetPrintArea(RECT &rc) const = 0;          // PrintArea:相对于物理媒介左上角开始的打印区域(X,Y,Width,Height)
    virtual void GetOrigPrintArea(RECT &rc) const = 0;
    virtual void GetRestrictedArea(RECT &rc) const = 0;
    virtual void GetOrigRestrictedArea(RECT &rc) const = 0;
    virtual PASSBOOKFOLD GetPassbookFold() const = 0;
    virtual DWORD GetStaggering() const = 0;
    virtual DWORD GetPageCount() const = 0;
    virtual DWORD GetLineCount() const = 0;
};
struct  ISPPrinterForm : public ISPPrinterContainerItem, public ISPPrinterItem
{
    virtual BOOL IsLoadSucc() const = 0;        // Form是否已加载/已加载成功
    virtual FORMALIGN GetAlign() const = 0;
    virtual FORMBASE GetOrigUNIT(SIZE *pSize) const = 0;
    virtual FORMORIENTATION GetOrientation() const = 0;
    virtual WORD GetVersion() const = 0;
    virtual WORD GetLangID() const = 0;
    virtual LPCSTR GetCopyright() const = 0;
    virtual LPCSTR GetTitle() const = 0;
    virtual LPCSTR GetComment() const = 0;
    virtual LPCSTR GetUserPrompt() const = 0;
};
struct  ISPPrinterSubform : public ISPPrinterItem, public ISPPrinterContainerItem
{
};
struct  ISPPrinterPrintable
{
    virtual LPCSTR GetHead() const = 0;
    virtual LPCSTR GetFooter() const = 0;
    virtual BOOL IsFrontSide() const = 0;
    virtual OVER_FLOW GetOverflow() const = 0;  // 30-00-00-00(FT#0063)
    virtual DWORD GetStyle() const = 0;
    virtual HORIZONTAL GetHorizAlign() const = 0;
    virtual VERTICAL GetVertAlign() const = 0;
    virtual COLORREF GetColor() const = 0;
    virtual WORD GetLangID() const = 0;
    virtual CLASS GetClass() const = 0;
};
struct  ISPPrinterField : public ISPPrinterPrintable, public ISPPrinterItem
{
    virtual ISPPrinterField *GetFollows() = 0;
    virtual FIELDTYPE GetFieldType() const = 0;
    virtual SCALING GetScaling() const = 0;
    virtual BARCODEPOS GetBarcodePos() const = 0;
    virtual ACCESS GetAccess() const = 0;
    virtual CASE GetCASE() const = 0;
    virtual LPCSTR GetFontName() const = 0;
    virtual DWORD GetFontSize() const = 0;
    virtual DWORD GetCPI() const = 0;
    virtual DWORD GetLPI() const = 0;
    virtual DWORD GetRepeatCount() const = 0;
    virtual SIZE GetRepeatOffset() const = 0;
    virtual LPCSTR GetFormat() const = 0;
    virtual LPCSTR GetInitValue() const = 0;
};
struct  ISPPrinterFrame : public ISPPrinterPrintable, public ISPPrinterItem
{
    virtual DWORD GetRepeatX(DWORD *pOffset) const = 0;
    virtual BOOL GetRepeatY(DWORD *pOffset) const = 0;
    virtual FRAMETYPE GetFrameType() const = 0;
    virtual COLORREF GetFillColor() const = 0;
    virtual FILLSTYLE GetFillStyle() const = 0;
    virtual char GetSubstSign() const = 0;
    virtual ISPPrinterField *GetFrame() = 0;
    virtual ISPPrinterField *GetTitle() = 0;
};
struct  ISPPrinterList
{
    virtual DWORD GetCount() const = 0;
    virtual ISPPrinterItem *GetAt(DWORD Index) const = 0;
    virtual ISPPrinterItem *Find(LPCSTR szName) = 0;
};
