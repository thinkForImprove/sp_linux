#ifndef DEVIMPL_TPK193_H
#define DEVIMPL_TPK193_H

#include "ILogWrite.h"
#include "QtDLLLoader.h"
#include "../XFS_SIG/CommonInfo.h"


#define DLL_DEVLIB_NAME  "libFaithSignLib.so.1.0.0"         // 缺省动态库名
#define LOG_NAME         "DevImpl_TPK193.log"               // 缺省日志名

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
// 1. 获取设备连接状态
typedef int (CALL_MODE *pGetConnectStatus)();
// 2. 获取设备状态
typedef int (CALL_MODE *pGetDeviceStatus)();
// 3. 重置设备
typedef int (CALL_MODE *pResetDevice)();
// 4. 启动签名
typedef int (CALL_MODE *pStartSignature)(int x, int y, int width, int height);
// 5. 清除签名
typedef int (CALL_MODE *pClearSignature)();
// 6. 结束签名
typedef int (CALL_MODE *pEndSignature)();
// 7. 设置路径
typedef int (CALL_MODE *pSetPath)(char *psgPath, char *imagePath);
// 8. 获取签名(encrypt)
typedef int (CALL_MODE *pGetSignature)(unsigned char *pSignData, int *pLen, int iWorkKey);
// 9. 获取签名(encrypt)
typedef int (CALL_MODE *pGetSignatureWin)(unsigned char *pSignData, int iMK, unsigned char *pucWK, int iAlgorithm, int iWKlen);
// 10. 获取签名(plain)
typedef int (CALL_MODE *pGetSignatureNoEnc)(unsigned char *pSignData);
// 11. 获取签名数据
typedef unsigned char * (CALL_MODE *pGetPsgData)(int &datalen);
// 12. 设置签名笔迹的颜色
typedef int (CALL_MODE *pSetPenColor)(int colorflag);
// 13. 设置签名笔迹的宽度
typedef int (CALL_MODE *pSetPenWidth)(int width);
// 14. 设置Background颜色
typedef int (CALL_MODE *pSetBackgroundColor)(int colorflag);
// 15. 设置Background图片
typedef int (CALL_MODE *pSetBackgroundImage)(const char *path);
// 16. 灌注密钥
typedef int (CALL_MODE *pLoadKey)(char *keydata, int datalen, int keyuse, int index, int algorithm,int decodekey, int checkmode);
// 17. 灌注密钥（按名称）
typedef int (CALL_MODE *pLoadKeyByName)(char *keydata, int datalen, int keyuse, char* keyname, int algorithm, char* decodekey, int checkmode);
// 18. 校验KCV
typedef int (CALL_MODE *pCheckKCV)(unsigned char*kcv, int len);
// 19. 设置加密方式
typedef int (CALL_MODE *pSetEncypt)(int ec_algorithm, int ec_master_key, int ec_work_key);
// 20. 设置加密方式（按名称）
typedef int (CALL_MODE *pSetEncyptByName)(int ec_algorithm, char* ec_master_key, char* ec_work_key);
// 21. 还原签名数据
typedef int (CALL_MODE *pShowSignData)(unsigned char* ucData, int iLength, int iWorkKey, int iMainKey, char *pImgPath);
// 22. 日志设置
typedef int (CALL_MODE *pLogEnable)(bool bEnable);
// 23. 日志设置
typedef int (CALL_MODE *pSetlogPath)(char* pstr);
// 24. 获取KCV
typedef int (CALL_MODE *pGetKCV)(unsigned char* pPlainData, int iPlainDataLen, int index, int algorithm, int checkmode, unsigned char*kcv);
// 25. 计算KCV
typedef int (CALL_MODE *pCalKCV)(int index, unsigned char* ucKcv);
/*
命令编辑、发送接收等处理。
*/

class CDevImpl_TPK193 : public CLogManage
{
public:
    CDevImpl_TPK193();
    virtual ~CDevImpl_TPK193();
public:
    BOOL OpenDevice();
    BOOL OpenDevice(WORD wType);
    BOOL CloseDevice();
    BOOL IsDeviceOpen();

public: // 接口函数封装
    // 1. 获取设备连接状态
    int GetConnectStatus();
    // 2. 获取设备状态
    int GetDeviceStatus();
    // 3. 重置设备
    int ResetDevice();
    // 4. 启动签名
    int StartSignature(int x, int y, int width, int height);
    // 5. 清除签名
    int ClearSign();
    // 6. 结束签名
    int EndSignature();
    // 7. 设置路径
    int SetPath(char *psgPath, char *imagePath);
    // 8. 获取签名(encrypt)
    int GetSign(unsigned char *pSignData, int *pLen = 0, int iWorkKey = -1);
    // 9. 获取签名(encrypt)
    int GetSignatureWin(unsigned char *pSignData, int iMK, unsigned char *pucWK, int iAlgorithm, int iWKlen = 16);
    // 10. 获取签名(plain)
    int GetSignatureNoEnc(unsigned char *pSignData);
    // 11. 获取签名数据
    unsigned char *GetPsgData(int &datalen);
    // 12. 设置签名笔迹的颜色
    int SetPenColor(int colorflag);
    // 13. 设置签名笔迹的宽度
    int SetPenWidth(int width);
    // 14. 设置Background颜色
    int SetBackgroundColor(int colorflag);
    // 15. 设置Background图片
    int SetBackgroundImage(const char *path);
    // 16. 灌注密钥
    int LoadKey(char *keydata, int datalen, int keyuse, int index, int algorithm,int decodekey, int checkmode = 0);
    // 17. 灌注密钥（按名称）
    int LoadKeyByName(char *keydata, int datalen, int keyuse, char* keyname, int algorithm, char* decodekey, int checkmode = 0);
    // 18. 校验KCV
    int CheckKCV(unsigned char*kcv, int len);
    // 19. 设置加密方式
    int SetEncypt(int ec_algorithm, int ec_master_key, int ec_work_key);
    // 20. 设置加密方式（按名称）
    int SetEncyptByName(int ec_algorithm, char* ec_master_key, char* ec_work_key);
    // 21. 还原签名数据
    int ShowSignData(unsigned char* ucData, int iLength, int iWorkKey, int iMainKey, char *pImgPath);
    // 22. 日志设置
    int LogEnable(bool bEnable);
    // 23. 日志设置
    int SetlogPath(char* pstr);
    // 24. 获取KCV
    int GetKCV(unsigned char* pPlainData, int iPlainDataLen, int index, int algorithm, int checkmode, unsigned char*kcv);
    // 25. 计算KCV
    int CalKCV(int index, unsigned char* ucKcv);

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

    pGetConnectStatus                   m_pGetConnectStatus;
    pGetDeviceStatus                    m_pGetDeviceStatus;
    pResetDevice                        m_pResetDevice;
    pStartSignature                     m_pStartSignature;
    pClearSignature                     m_pClearSignature;
    pEndSignature                       m_pEndSignature;
    pSetPath                            m_pSetPath;
    pGetSignature                       m_pGetSignature;
    pGetSignatureWin                    m_pGetSignatureWin;
    pGetSignatureNoEnc                  m_pGetSignatureNoEnc;
    pGetPsgData                         m_pGetPsgData;
    pSetPenColor                        m_pSetPenColor;
    pSetPenWidth                        m_pSetPenWidth;
    pSetBackgroundColor                 m_pSetBackgroundColor;
    pSetBackgroundImage                 m_pSetBackgroundImage;
    pLoadKey                            m_pLoadKey;
    pLoadKeyByName                      m_pLoadKeyByName;
    pCheckKCV                           m_pCheckKCV;
    pSetEncypt                          m_pSetEncypt;
    pSetEncyptByName                    m_pSetEncyptByName;
    pShowSignData                       m_pShowSignData;
    pLogEnable                          m_pLogEnable;
    pSetlogPath                         m_pSetlogPath;
    pGetKCV                             m_pGetKCV;
    pCalKCV                             m_pCalKCV;
};


#endif // DEVIMPL_TPK193_H
