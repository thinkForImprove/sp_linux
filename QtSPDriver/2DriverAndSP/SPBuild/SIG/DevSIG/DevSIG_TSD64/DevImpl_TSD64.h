#ifndef DEVIMPL_TSD64_H
#define DEVIMPL_TSD64_H

#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "../XFS_SIG/CommonInfo.h"
#include "signature.h"


#define DLL_DEVLIB_NAME  "libtsdsignature.so"        // 缺省动态库名
#define LOG_NAME         "DevImpl_TSD64.log"   // 缺省日志名

#define SAVE_IMG_BMP    0
#define SAVE_IMG_JPG    1
#define SAVE_IMG_PNG    2

#define SAVE_IMG_BMP_S  "BMP"
#define SAVE_IMG_JPG_S  "JPG"
#define SAVE_IMG_PNG_S  "PNG"

typedef unsigned BSDEVICEID;     // 定义设备ID类型

#define FUNC_POINTER_ERROR_RETURN(LPFUNC, LPFUNC2) \
    if(!LPFUNC){   \
        m_bLoadIntfFail = TRUE; \
        return FALSE;   \
    }

#ifndef CALL_MODE
#ifdef _WIN32
#define CALL_MODE	__stdcall
#else
#define CALL_MODE
#endif
#endif

//----接口函数定义-------------------------------------------------------------------------------
// 1. 设置显示器宽高
typedef void (CALL_MODE *psgSetScreenWidthHeight)(int offsetX, int offsetY, unsigned int screenWidth, unsigned int screenHeight, char *pchErrCode);
// 2. 设置签名窗口
typedef void (CALL_MODE *psgSetSignWindow)(int x, int y, int w, int h, char *pchErrCode);
// 3. 设置背景色和透明度
typedef void (CALL_MODE *psgSetBackColorParam)(int Transparency, unsigned long backColor, int useBackColor, char *pchErrCode);
// 4. 设置签字背景图片
typedef void (CALL_MODE *psgSetBackgroundPicture)(char *photoPath, bool bPicAlwaysShow, char* pchErrCode);
// 5. 设置提示文本
typedef void (CALL_MODE *psgSetTextData)(int Left, int Top, const char* string, const char* fontName, int fontSize, unsigned long textColor, bool bTxtAlwaysShow, char* pchErrCode);
// 6. 设置笔的粗细
typedef void (CALL_MODE *psgSetPenMax)(int MaxPressurePixel, char *pchErrCode);
// 7. 使用上述函数的设置值启动一个签名窗口
typedef void (CALL_MODE *psgStartSignatureUseSetting)(char* pchErrCode);
// 8. 开启签名
typedef void (CALL_MODE *psgStartSignature)(int x, int y, int w, int h, char *pchErrCode);
// 9. 开启签名
typedef void (CALL_MODE *psgStartSignPng)(int x, int y, int w, int h, char *photoPath, bool bAlwaysShow, char *pchErrCode);
// 10. 开启签名
typedef void (CALL_MODE *psgStartSignPngPenMax)(int x, int y, int w, int h, unsigned int PenMax, char *photoPath, bool bAlwaysShow, char *pchErrCode);
// 11. 开启签名
typedef void (CALL_MODE *psgStartSignMsgPen)(int x, int y, int w, int h, unsigned int PenMax, const char *pcMsg, bool bAlwaysShow, int iX, int iY, int iFontHeight,	unsigned long ulColor, const char *pccFont, char *pchErrCode);
// 12. 清除签名
typedef void (CALL_MODE *psgClearSignature)(char* pchErrCode);
// 13. 隐藏签名窗口
typedef void (CALL_MODE *psgHideSignWindow)(char *pchErrCode);
// 14. 显示签名窗口
typedef void (CALL_MODE *psgShowSignWindow)(char *pchErrCode);
// 15. 关闭签名窗口
typedef void (CALL_MODE *psgCloseSignWindow)(char *pchErrCode);
// 16. 结束签名
typedef void (CALL_MODE *psgEndSignature)(char *pchErrCode);
// 17. 获取电子签名加密轨迹数据和图片
typedef void (CALL_MODE *psgGetSignature)(unsigned char *psignData, long *psignDataLen, int iIndex, unsigned char *pbWorkKey, char *photoPath, char *pchErrCode);
// 18. 获取电子签名明文二进制轨迹数据
typedef void (CALL_MODE *psgGetSignData)(unsigned char *psignData, long *psignDataLen, char *pchErrCode);
// 19. 获取电子签名明文文本轨迹数据
typedef void (CALL_MODE *psgGetSignDataFile)(char* signFilePath, int type, char *pchErrCode);
// 20. 获取签字图片
typedef void (CALL_MODE *psgGetPngPicture)(char* photoPath, double multiple, char* pchErrCode);
// 21. 获取签字图片
typedef void (CALL_MODE *psgGetPngPictureW2H1)(char* photoPath, double multiple, char* pchErrCode);
// 22. 灌注DES主密钥
typedef void (CALL_MODE *psgSetDESPrimaryKey)(char *pPriKeys, int number, char *pchErrCode);
// 23. 灌注DES主密钥
typedef void (CALL_MODE *psgSetPrimaryKey)(char *pPriKey, int iLength, int iIndex, char *pchErrCode);
// 24. 复位设备
typedef void (CALL_MODE *psgResetDev)(char *pchErrCode);
// 25. 获取设备状态
typedef void (CALL_MODE *psgGetDevStatus)(char *pchErrCode);
// 26. 获取固件版本信息
typedef void (CALL_MODE *psgGetFirmwareVer)(char *strVer, char *pchErrCode);
// 27. 获取电子签名加密轨迹数据
typedef void (CALL_MODE *psgGetSign)(unsigned char *psignData, int iEncryptType, char* pchErrCode);
// 28. 灌注密钥
typedef void (CALL_MODE *psgImportKey)(char* pKey, int iLength, int iIndex, int iDecKeyNum, int iDecMode, int iUse, char* pchErrCode);
// 29. 返回密钥校验值
typedef void (CALL_MODE *psgGetKeyVerificationCode)(char* pKVC, int *iLength, int iIndex, int iEncMode, char* pchErrCode);
// 30. 主密钥转工作密钥(轨迹数据加密用)
typedef void (CALL_MODE *psgMasterKeyToWorkKey)(int iIndex, char* pchErrCode);
// 31. 获取动态库编译日期
typedef void (CALL_MODE *psgGetCompileDate)(char *strDate, char *pchErrCode);
/*
命令编辑、发送接收等处理。
*/

class CDevImpl_TSD64 : public CLogManage
{
public:
    CDevImpl_TSD64();
    virtual ~CDevImpl_TSD64();
public:
    BOOL OpenDevice();
    BOOL OpenDevice(WORD wType);
    BOOL CloseDevice();
    BOOL IsDeviceOpen();

public: // 接口函数封装
    // 1. 设置显示器宽高
    void vSetScreenWidthHeight(int offsetX, int offsetY, unsigned int screenWidth, unsigned int screenHeight, char *pchErrCode);
    // 2. 设置签名窗口
    void vSetSignWindow(int x, int y, int w, int h, char *pchErrCode);
    // 3. 设置背景色和透明度
    void vSetBackColorParam(int Transparency, unsigned long backColor, int useBackColor, char *pchErrCode);
    // 4. 设置签字背景图片
    void vSetBackgroundPicture(char *photoPath, bool bPicAlwaysShow, char* pchErrCode);
    // 5. 设置提示文本
    void vSetTextData(int Left, int Top, const char* string, const char* fontName, int fontSize, unsigned long textColor, bool bTxtAlwaysShow, char* pchErrCode);
    // 6. 设置笔的粗细
    void vSetPenMax(int MaxPressurePixel, char *pchErrCode);
    // 7. 使用上述函数的设置值启动一个签名窗口
    void vStartSignatureUseSetting(char* pchErrCode);
    // 8. 开启签名
    void vStartSignature(int x, int y, int w, int h, char *pchErrCode);
    // 9. 开启签名
    void vStartSignPng(int x, int y, int w, int h, char *photoPath, bool bAlwaysShow, char *pchErrCode);
    // 10. 开启签名
    void vStartSignPngPenMax(int x, int y, int w, int h, unsigned int PenMax, char *photoPath, bool bAlwaysShow, char *pchErrCode);
    // 11. 开启签名
    void vStartSignMsgPen(int x, int y, int w, int h, unsigned int PenMax, const char *pcMsg, bool bAlwaysShow, int iX, int iY, int iFontHeight,	unsigned long ulColor, const char *pccFont, char *pchErrCode);
    // 12. 清除签名
    void vClearSignature(char* pchErrCode);
    // 13. 隐藏签名窗口
    void vHideSignWindow(char *pchErrCode);
    // 14. 显示签名窗口
    void vShowSignWindow(char *pchErrCode);
    // 15. 关闭签名窗口
    void vCloseSignWindow(char *pchErrCode);
    // 16. 结束签名
    void vEndSignature(char *pchErrCode);
    // 17. 获取电子签名加密轨迹数据和图片
    void vGetSignature(unsigned char *psignData, long *psignDataLen, int iIndex, unsigned char *pbWorkKey, char *photoPath, char *pchErrCode);
    // 18. 获取电子签名明文二进制轨迹数据
    void vGetSignData(unsigned char *psignData, long *psignDataLen, char *pchErrCode);
    // 19. 获取电子签名明文文本轨迹数据
    void vGetSignDataFile(char* signFilePath, int type, char *pchErrCode);
    // 20. 获取签字图片
    void vGetPngPicture(char* photoPath, double multiple, char* pchErrCode);
    // 21. 获取签字图片
    void vGetPngPictureW2H1(char* photoPath, double multiple, char* pchErrCode);
    // 22. 灌注DES主密钥
    void vSetDESPrimaryKey(char *pPriKeys, int number, char *pchErrCode);
    // 23. 灌注DES主密钥
    void vSetPrimaryKey(char *pPriKey, int iLength, int iIndex, char *pchErrCode);
    // 24. 复位设备
    void vResetDev(char *pchErrCode);
    // 25. 获取设备状态
    void vGetDevStatus(char *pchErrCode);
    // 26. 获取固件版本信息
    void vGetFirmwareVer(char *strVer, char *pchErrCode);
    // 27. 获取电子签名加密轨迹数据
    void vGetSign(unsigned char *psignData, int iEncryptType, char* pchErrCode);
    // 28. 灌注密钥
    void vImportKey(char* pKey, int iLength, int iIndex, int iDecKeyNum, int iDecMode, int iUse, char* pchErrCode);
    // 29. 返回密钥校验值
    void vGetKeyVerificationCode(char* pKVC, int *iLength, int iIndex, int iEncMode, char* pchErrCode);
    // 30. 主密钥转工作密钥(轨迹数据加密用)
    void vMasterKeyToWorkKey(int iIndex, char* pchErrCode);
    // 31. 获取动态库编译日期
    void vGetCompileDate(char *strDate, char *pchErrCode);

private:
    BSDEVICEID      m_bsDeviceId;
    BOOL            m_bDevOpenOk;
    CHAR            m_szErrStr[1024];

private:
    void Init();

private: // 接口加载
    BOOL bLoadLibrary();
    void vUnLoadLibrary();
    BOOL bLoadLibIntf();

private: // 接口加载
    char        m_szLoadDllPath[MAX_PATH];
    QLibrary    m_LoadLibrary;
    BOOL        m_bLoadIntfFail;
    INT         m_nDevErrCode;
    INT         m_nStatErrCode;


private: // 动态库接口定义
    psgSetScreenWidthHeight             pSetScreenWidthHeight;
    psgSetSignWindow                    pSetSignWindow;
    psgSetBackColorParam                pSetBackColorParam;
    psgSetBackgroundPicture             pSetBackgroundPicture;
    psgSetTextData                      pSetTextData;
    psgSetPenMax                        pSetPenMax;
    psgStartSignatureUseSetting         pStartSignatureUseSetting;
    psgStartSignature                   pStartSignature;
    psgStartSignPng                     pStartSignPng;
    psgStartSignPngPenMax               pStartSignPngPenMax;
    psgStartSignMsgPen                  pStartSignMsgPen;
    psgClearSignature                   pClearSignature;
    psgHideSignWindow                   pHideSignWindow;
    psgShowSignWindow                   pShowSignWindow;
    psgCloseSignWindow                  pCloseSignWindow;
    psgEndSignature                     pEndSignature;
    psgGetSignature                     pGetSignature;
    psgGetSignData                      pGetSignData;
    psgGetSignDataFile                  pGetSignDataFile;
    psgGetPngPicture                    pGetPngPicture;
    psgGetPngPictureW2H1                pGetPngPictureW2H1;
    psgSetDESPrimaryKey                 pSetDESPrimaryKey;
    psgSetPrimaryKey                    pSetPrimaryKey;
    psgResetDev                         pResetDev;
    psgGetDevStatus                     pGetDevStatus;
    psgGetFirmwareVer                   pGetFirmwareVer;
    psgGetSign                          pGetSign;
    psgImportKey                        pImportKey;
    psgGetKeyVerificationCode           pGetKeyVerificationCode;
    psgMasterKeyToWorkKey               pMasterKeyToWorkKey;
    psgGetCompileDate                   pGetCompileDate;
};


#endif // DEVIMPL_TSD64_H
