/*****************************************************************************
** FileName     : RPR_USB.CPP                                               **
** Describtion  : 硬件设备USB处理                                             **
** Date         : 2019/7/8                                                  **
** V-R-T        : RPR01000000                                               **
** History      : 2019/7/8 RPR01000000 JiaoChong(CFES) Create               **
*****************************************************************************/
#include "rpr_usb.h"


extern DevPTR_RPR *devPTR;

// 加载 硬件设备链接库
static QLibrary lib("/hots/lib/libVHSPR.so");

// 加载硬件设备链接库函数原型
static unsigned int(*pFnSPRUSB)(unsigned int, PSTR_DRV) = nullptr;
static unsigned int(*pInfSPRUSB)(unsigned int, PSTR_DRV) = nullptr;
static pthread_mutex_t hMutex = PTHREAD_MUTEX_INITIALIZER;

// 设备操作句柄
static unsigned int uiDrvHnd = 0;

// 硬件设备操作类生命
RPR_USB *pRPR_USB;

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : RPR_USB()                                                 **
** Describtion  : 构造函数                                                  **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
RPR_USB::RPR_USB()
{
    Buff_hnd = nullptr;
    lpStrDrvDll    = nullptr;
    lpbySensLP_Buf = nullptr;
    lpbyVrtLP_Buf  = nullptr;
    lpbySndLP_Buf  = nullptr;
    lpbyOpnSLP_Buf = nullptr;
    lpbyOpnRLP_Buf = nullptr;
    lpbyVrtDrv_Buf = nullptr;
    lpbyVendor_Buf = nullptr;
    uiOpenTimer = OPENTIMER_DEF;
    uiSendTimer = SENDTIMER_DEF;
    uiSndLP_sz = 0;
    bEPDwn = FALSE;
    bLPDwn = FALSE;
    bTMOut = FALSE;
    uiDrvHnd = 0;

    status = Syoki_st; // 用来判断回调函数的状态:初始状态
    lOpenEndRet = NG;  // Open回调函数返回值:初始NG

    bSnsData = FALSE;

    ulUsbErrCode = 0x00000000;  // 硬件设备返回错误码
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long lUsbDllLoad()                                        **
** Describtion  : 加载Lib函数                                                **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::lUsbDllLoad()
{
    if ((pFnSPRUSB != nullptr) && (pInfSPRUSB != nullptr))
    {
        // 链接库函数已加载，返回OK
        return OK;
    }
    if (lib.load())
    {
        pFnSPRUSB = reinterpret_cast<unsigned int(*)(unsigned int, PSTR_DRV)>(lib.resolve("FnSPRUSB"));
        pInfSPRUSB = reinterpret_cast<unsigned int(*)(unsigned int, PSTR_DRV)>(lib.resolve("InfSPRUSB"));
        if ((pFnSPRUSB == nullptr) || (pInfSPRUSB == nullptr))
        {
            // 链接库函数引用加载失败
            return NG;
        }
    }
    else
    {
        // 链接库加载失败
        return NG;
    }
    return OK;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long N_Open_Usb()                                         **
** Describtion  : 打开设备+启动Sensor+Reset                                   **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::N_Open_Usb()
{
    long lRtn = NG;
    Buff_hnd = reinterpret_cast<LPBYTE>(malloc(4400));
    memset(Buff_hnd, 0, 4400);

    if (Buff_hnd)
    {
        lpStrDrvDll    = (Buff_hnd + 16);     //   48 BYTE(USB  DRVDLL PTR)
        lpbySensLP_Buf = (Buff_hnd + 96);     //   14 BYTE(SENS BUFF)
        lpbyVrtLP_Buf  = (Buff_hnd + 112);    //   16 BYTE(VRT BUFF)
        lpbySndLP_Buf  = (Buff_hnd + 196);    // 4096 BYTE(SEND BUFF)
        lpbyOpnSLP_Buf = (Buff_hnd + 2000);   //   16 BYTE(OPEN SEND BUFF)
        lpbyOpnRLP_Buf = (Buff_hnd + 2500);   //   16 BYTE(OPEN RECV BUFF)
        lpbyVrtDrv_Buf = (Buff_hnd + 3000);   //   32 BYTE(DRV VRT BUFF)
        lpbyVendor_Buf = (Buff_hnd + 3500);   //   24 BYTE(VENDOR BUFF)
        lpbyPrevSensLP_Buf = (Buff_hnd + 4278); //   14 BYTE(SENS BUFF)

        // 加载Lib函数
        lRtn = lUsbDllLoad();
        if (lRtn == OK)
        {
            status = Syoki_st;  // 回调函数状态初始化
            lOpenEndRet = NG;   // 回调函数处理Open返回值初始化NG

            // 下发Open指令
            uiDrvHnd = 0;
            lRtn = lUsbDllCall(SPR_DRV_INF_OPEN, SPR_PRM_OPEN, 0);
            if (lRtn == OK)
            {
                // 无限循环，每隔1秒判断Open回调函数是否完成，完成则跳出循环
                while (1)
                {
                    sleep(1);
                    if (status == OpenEnd_W) // 回调函数状态:Open已处理完
                    {
                        status = Syoki_st;  // 回调函数状态初始化
                        break;
                    }
                }

                // 回调函数处理Open返回值OK
                if (lOpenEndRet == OK)
                {
                    // 摘取设备Handle
                    uiDrvHnd = *((unsigned int *)(lpbyOpnRLP_Buf + 4));

                    lRtn = lUsbDllCall(SPR_DRV_INF_INFGET, SPR_PRM_SENS, 0);
                    lRtn = lUsbDllCall(SPR_DRV_INF_SENS, SPR_PRM_START, 0);
                    if (lRtn == OK)
                    {
                        // 设备初始化
                        lRtn = N_Reset_Usb(TRUE);
                    }
                    else
                    {
                        // Sensor指令下发失败
                        devPTR->N_SetErrCode(szErrUSB_SnsCall);
                    }

                    // 设备初始化
                    //lRtn = N_Reset_Usb(TRUE);
                }
                else
                {
                    // OPEN回调函数返回失败
                    devPTR->N_SetErrCode(szErrUSB_OpenBack);
                    lRtn = lOpenEndRet;
                }
            }
            else
            {
                // Open指令下发失败
                devPTR->N_SetErrCode(szErrUSB_OpenCall);
            }
        }
        else
        {
            // 加载Lib链接库失败
            devPTR->N_SetErrCode(szErrUSB_DLLLoad);
        }
    }

    return lRtn;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long N_Reset_Req()                                         **
** Describtion  : 重置设备                                                   **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::N_Reset_Usb(BOOL bInit)
{
    long lRtn = NG;

    // 设备句柄为空
    if (uiDrvHnd == 0)
    {
        devPTR->N_SetErrCode(szErrUSB_HadInval);
        return NG;
    }

    // 设备处于终止状态
    if (bEPDwn == TRUE)
    {
        devPTR->N_SetErrCode(szErrUSB_DevStop);
        return NG;
    }
    else
    {
        status = Syoki_st;  // 回调函数状态初始化
        lRcvEndRet = NG;   // 回调函数处理Snd返回值初始化NG
        lRtn = lUsbDllCall(SPR_DRV_FN_DATARCV, SPR_PRM_SEND_VENDOR_IN, DRV_VENDOR_RESET);
        if (lRtn == OK)
        {
            // 无限循环，每隔1秒判断Reset回调函数是否完成，完成则跳出循环
            while (1)
            {
                sleep(1);
                if (status == RCV_W1)   // 回调函数状态:RCV已处理完
                {
                    status = Syoki_st;  // 回调函数状态初始化
                    break;
                }
            }

            // 回调函数处理Open返回值OK
            if (lRcvEndRet != OK)
            {
                devPTR->N_SetErrCode(szErrUSB_ResetBack);
            }

            lRtn = lRcvEndRet;
        }
        else
        {
            devPTR->N_SetErrCode(szErrUSB_ResetCall);
        }
    }

    return lRtn;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long N_Close_Req()                                        **
** Describtion  : 设备停止                                                   **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::N_Close_Usb()
{
    long  lRtn  = OK;

    // 设备不为空 同时 不处于终止状态
    if (uiDrvHnd != 0 && bEPDwn == FALSE)
    {
        status = Syoki_st;
        //
        lRtn = lUsbDllCall(SPR_DRV_INF_SENS, SPR_PRM_END, 0);

        //
        lRtn = lUsbDllCall(SPR_DRV_INF_CLOSE, SPR_PRM_CLOSE, 0);

        uiDrvHnd = 0;
    }

    return lRtn;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long N_CutPaper_Usb()                                     **
** Describtion  : 设备切纸                                                   **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::N_CutPaper_Usb(LPBYTE lpData, LONG ulDataSz)
{
    long lRtn = NG;

    // 设备句柄为空
    if (uiDrvHnd == 0)
    {
        devPTR->N_SetErrCode(szErrUSB_HadInval);
        return NG;
    }

    // 设备处于终止状态
    if (bEPDwn == TRUE)
    {
        devPTR->N_SetErrCode(szErrUSB_DevStop);
        return NG;
    }
    else
    {
        status = Syoki_st;  // 回调函数状态初始化
        lRcvEndRet = NG;   // 回调函数处理Open返回值初始化NG
        lRtn = lUsbDllCall(SPR_DRV_FN_DATASEND, SPR_PRM_SEND_COMMAND, lpData, ulDataSz);
        if (lRtn == OK)
        {
            // 无限循环，每隔1秒判断Open回调函数是否完成，完成则跳出循环
            while (1)
            {
                sleep(1);
                if (status == RCV_W1)   // 回调函数状态:RCV已处理完
                {
                    status = Syoki_st;  // 回调函数状态初始化
                    break;
                }
            }

            // 回调函数处理Open返回值OK
            if (lRcvEndRet != OK)
            {
                devPTR->N_SetErrCode(szErrUSB_CutBack);
            }

            lRtn = lRcvEndRet;
        }
        else
        {
            devPTR->N_SetErrCode(szErrUSB_CutCall);
        }
    }

    return lRtn;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long N_SendData_Usb()                                     **
** Describtion  : 发送数据到设备                                              **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::N_SendData_Usb(LPBYTE lpData, LONG ulDataSz)
{
    long lRtn = NG;

    // 设备句柄为空
    if (uiDrvHnd == 0)
    {
        devPTR->N_SetErrCode(szErrUSB_HadInval);
        return NG;
    }

    // 设备处于终止状态
    if (bEPDwn == TRUE)
    {
        devPTR->N_SetErrCode(szErrUSB_DevStop);
        return NG;
    }
    else
    {
        status = Syoki_st;  // 回调函数状态初始化
        lRcvEndRet = NG;   // 回调函数处理Open返回值初始化NG
        lRtn = lUsbDllCall(SPR_DRV_FN_DATASEND, SPR_PRM_SEND_COMMAND, lpData, ulDataSz);
        // 无限循环，每隔1秒判断Open回调函数是否完成，完成则跳出循环
        if (lRtn == OK)
        {
            // 无限循环，每隔1秒判断Open回调函数是否完成，完成则跳出循环
            while (1)
            {
                sleep(1);
                if (status == RCV_W1)   // 回调函数状态:RCV已处理完
                {
                    status = Syoki_st;  // 回调函数状态初始化
                    break;
                }
            }

            // 回调函数处理Open返回值OK
            if (lRcvEndRet != OK)
            {
                devPTR->N_SetErrCode(szErrUSB_SndBack);
            }

            lRtn = lRcvEndRet;
        }
        else
        {
            devPTR->N_SetErrCode(szErrUSB_SndCall);
        }
    }

    return lRtn;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : BYTE N_GetStatus_Usb()                                    **
** Describtion  : 获取设备状态字符串                                           **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
BYTE RPR_USB::N_GetStatus_Usb(enSPRSTATUS enSPRStatus)
{
    long lRtn = NG;
    int  iCounter = 50;
    LPBYTE lpSns = nullptr;
    WORD    wSz = 0;
    WORD    wCount = 0;

    // 设备句柄为空
    if (uiDrvHnd == 0)
    {
        devPTR->N_SetErrCode(szErrUSB_HadInval);
        return NG;
    }

    //lRtn = lUsbDllCall(SPR_DRV_INF_SENS, SPR_PRM_START, 0);
    lRtn = lUsbDllCall(SPR_DRV_INF_INFGET, SPR_PRM_SENS, 0);

    if (lRtn == OK)
    {
        // 等待0.5秒，确保准确获取状态
        sleep(1 / 2);

        lpSns = lpbySensLP_Buf + 4;
        wSz = SPR_SNS_BUFF_SIZE - 4;
        while (*lpSns == 0xFF)
        {
            sleep(1 / 5);
            lpSns = lpbySensLP_Buf + 4;
            wSz = SPR_SNS_BUFF_SIZE - 4;
            lRtn = OK;

            --iCounter;
            if (iCounter <= 0)
            {
                lRtn = NG;
                break;
            }
        }
    }

    if (lRtn == NG)
    {
        devPTR->N_SetErrCode(szErrUSB_StatusCall);
    }

    return (*(lpSns + enSPRStatus));
}



/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : BYTE N_GetStatus_Usb()                                    **
** Describtion  : 获取设备状态字符串                                           **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
LPBYTE RPR_USB::N_GetStatus_Usb()
{
    long lRtn = NG;
    int  iCounter = 50;
    LPBYTE lpSns = nullptr;
    WORD    wSz = 0;
    WORD    wCount = 0;

    // 设备句柄为空
    if (uiDrvHnd == 0)
    {
        devPTR->N_SetErrCode(szErrUSB_HadInval);
        return NG;
    }

    //lRtn = lUsbDllCall(SPR_DRV_INF_SENS, SPR_PRM_START, 0);
    lRtn = lUsbDllCall(SPR_DRV_INF_INFGET, SPR_PRM_SENS, 0);

    if (lRtn == OK)
    {
        // 等待0.5秒，确保准确获取状态
        sleep(1 / 2);

        lpSns = lpbySensLP_Buf + 4;
        wSz = SPR_SNS_BUFF_SIZE - 4;
        while (*lpSns == 0xFF)
        {
            sleep(1 / 5);
            lpSns = lpbySensLP_Buf + 4;
            wSz = SPR_SNS_BUFF_SIZE - 4;
            lRtn = OK;

            --iCounter;
            if (iCounter <= 0)
            {
                lRtn = NG;
                break;
            }
        }
    }

    if (lRtn == NG)
    {
        devPTR->N_SetErrCode(szErrUSB_StatusCall);
    }

    return lpSns;
}


/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : UINT N_GetDevHandle_Usb()                                 **
** Describtion  : 获取设备操作句柄                                            **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
UINT RPR_USB::N_GetDevHandle_Usb()
{
    return uiDrvHnd;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : ULONG N_GetErrCode_Usb()                                  **
** Describtion  : 获取硬件设备错误码                                           **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
ULONG RPR_USB::N_GetErrCode_Usb()
{
    return ulUsbErrCode;
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : void N_OnJnbKan()                                         **
** Describtion  : 回调函数调用Open处理                                         **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
void RPR_USB::N_OnJnbKan(WPARAM wParam, LPARAM lParam)
{
    long lRet = 0;

    lRet = lUsbDrvSyuryoCheck(lParam, DRV_RETURN_ONJNBKAN);
    if (lRet == OK)
    {
        bEPDwn = FALSE;
    }

    lOpenEndRet = lRet;
    status = OpenEnd_W;
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : void N_OnSensor()                                         **
** Describtion  : 回调函数调用Sensor处理                                       **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
void RPR_USB::N_OnSensor(WPARAM wParam, LPARAM lParam)
{
    long lRet = 0;

    lRet = lUsbDrvSyuryoCheck(lParam, DRV_RETURN_ONSENSER);
    if (lRet == OK)
    {
        bSnsData = TRUE;
    }
}

/*****************************************************************************
** FunctionType : private                                                   **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : void N_OnSndEnd()                                         **
** Describtion  : 回调函数调用SND处理                                         **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
void RPR_USB::N_OnSndEnd(WPARAM wParam, LPARAM lParam)
{
    long lRet = 0;

    lRet = lUsbDrvSyuryoCheck(lParam, DVMSG_HSPRT_SNDE);
    if (lRet == OK)
    {
        bEPDwn = FALSE;
    }

    lRcvEndRet = lRet;
    status = RCV_W1;
}

/*****************************************************************************
** FunctionType : Public                                                    **
** FileName     : RPR_USB.CPP                                               **
** ClassName    :                                                           **
** Symbol       :  ::                                                       **
** Function     : void N_SwitchFunc()                                       **
** Describtion  : 回调函数                                                   **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
void N_SwitchFunc(void *pMsg, void *vpErrCd)
{
    uint uiMsg = uintptr_t(pMsg);
    uint lErr = uintptr_t(vpErrCd);

    switch (uiMsg)
    {
    case DVMSG_HSPRT_JBKAN: // OPEN处理
        pRPR_USB->N_OnJnbKan(uiMsg, lErr);
        break;
    case DVMSG_HSPRT_SNS:   // Sensor处理
        pRPR_USB->N_OnSensor(uiMsg, lErr);
        break;
    case DVMSG_HSPRT_SNDE:  // SEND 处理
        pRPR_USB->N_OnSndEnd(uiMsg, lErr);
        break;
    default:
        break;
    }
}

/*****************************************************************************
** FunctionType : Private                                                   **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long lUsbDllCall_all()                                    **
** Describtion  : USB DLL 函数下发指令(不包含数据传入)                          **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::lUsbDllCall(unsigned int uifncNo, unsigned short usParam, BYTE i)
{
    return lUsbDllCall_all(uifncNo, usParam, i, nullptr, 0, 0, 0);
}

/*****************************************************************************
** FunctionType : Private                                                   **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long lUsbDllCall_all()                                    **
** Describtion  : USB DLL 函数下发指令(包含数据传入)                            **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::lUsbDllCall(unsigned int uifncNo, unsigned short usParam, LPBYTE lpData, LONG ulDataSz)
{
    return lUsbDllCall_all(uifncNo, usParam, 0, lpData, ulDataSz, 0, 0);
}

/*****************************************************************************
** FunctionType : Private                                                   **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long lUsbDllCall_all()                                    **
** Describtion  : USB DLL 函数下发指令                                        **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::lUsbDllCall_all(unsigned int uifncNo, unsigned short usParam, BYTE i,
                              LPBYTE lpData, LONG ulDataSz, WORD wParamValue, WORD wLength)
{
    long lRtn = OK;
    long lDrvRtn = OK;
    BYTE byOpenInBuff[] = "HSPUSB";
    LPBYTE lpbyOutBuff = nullptr;
    ulUsbErrCode = 0x00000000;

    struct VENDOR_IN
    {
        BYTE    byBmRequest;
        BYTE    byRequest;
        WORD    wValue;
        WORD    wIndex;
        WORD    wLength;
    } stVendorIn;

    memset(&strDrvInf_SPR, 0x00, sizeof(strDrvInf_SPR));

    if ((pFnSPRUSB == nullptr) || (pInfSPRUSB == nullptr))
    {
        devPTR->N_SetErrCode(szErrUSB_DLLFunInval);
        return NG;
    }

    switch (uifncNo)
    {
    case SPR_DRV_INF_OPEN:
        memset(lpbyOpnSLP_Buf, 0x00, SPR_OPN_SBUF_SIZE);
        memcpy(lpbyOpnSLP_Buf, &byOpenInBuff[0], sizeof(byOpenInBuff));
        memset(lpbyOpnRLP_Buf, 0x00, SPR_OPN_RBUF_SIZE);
        strDrvInf_SPR.usParam = usParam;
        strDrvInf_SPR.pvDataInBuffPtr  = lpbyOpnSLP_Buf;
        strDrvInf_SPR.uiDataInBuffSz   = sizeof(byOpenInBuff);
        strDrvInf_SPR.pvDataOutBuffPtr = lpbyOpnRLP_Buf;
        strDrvInf_SPR.uiDataOutReqSz   = SPR_OPN_RBUF_SIZE;
        //strDrvInf_SPR.uiDataOutBuffSz  = SPR_OPN_RBUF_SIZE;
        strDrvInf_SPR.uiTimer          = uiOpenTimer;
        strDrvInf_SPR.pvCallBackPtr    = (void *)::N_SwitchFunc;
        strDrvInf_SPR.uiWndMsg         = DVMSG_HSPRT_JBKAN;
        lDrvRtn = (*pInfSPRUSB)(uifncNo, &strDrvInf_SPR);
        break;
    case SPR_DRV_INF_CLOSE:
        strDrvInf_SPR.uiDrvHnd         = uiDrvHnd;
        strDrvInf_SPR.usParam          = usParam;
        strDrvInf_SPR.pvCallBackPtr    = nullptr;
        strDrvInf_SPR.uiWndMsg         = NULL;
        // ト＂ライハ＂DLL起动(情报系)
        lDrvRtn = (*pInfSPRUSB)(uifncNo, &strDrvInf_SPR);
        break;
    case SPR_DRV_INF_INFGET:    // 情报取得处理
        switch (usParam)
        {
        case SPR_PRM_SENS:          // Sensor情报
            memset(lpbySensLP_Buf, 0x00, SPR_SNS_BUFF_SIZE);
            strDrvInf_SPR.uiDrvHnd         = uiDrvHnd;
            strDrvInf_SPR.usParam          = usParam;
            strDrvInf_SPR.pvDataOutBuffPtr = lpbySensLP_Buf + 4;
            strDrvInf_SPR.uiDataOutReqSz   = SPR_SNS_BUFF_SIZE - 4;
            strDrvInf_SPR.pvCallBackPtr    = nullptr;
            strDrvInf_SPR.uiWndMsg         = NULL;
            break;
        case SPR_PRM_VRT:       // VRT情报
            strDrvInf_SPR.uiDrvHnd         = uiDrvHnd;
            strDrvInf_SPR.usParam          = usParam;
            strDrvInf_SPR.pvDataOutBuffPtr = lpbyVrtDrv_Buf;
            strDrvInf_SPR.uiDataOutReqSz   = 32;
            strDrvInf_SPR.pvCallBackPtr    = nullptr;
            strDrvInf_SPR.uiWndMsg         = NULL;
            break;
        default:
            break;
        }
        lDrvRtn = (*pInfSPRUSB)(uifncNo, &strDrvInf_SPR);
        break;
    case SPR_DRV_INF_SENS:      // Sensor变化通知开始/停止
        switch (usParam)
        {
        case SPR_PRM_START:
            //memset(&strDrvInf_SPR,0,sizeof(&strDrvInf_SPR));  //linux
            strDrvInf_SPR.uiDrvHnd         = uiDrvHnd;
            strDrvInf_SPR.usParam          = usParam;
            strDrvInf_SPR.pvDataOutBuffPtr = lpbySensLP_Buf;
            strDrvInf_SPR.uiDataOutReqSz   = SPR_SNS_BUFF_SIZE;
            strDrvInf_SPR.pvCallBackPtr    = (void *)::N_SwitchFunc;
            strDrvInf_SPR.uiWndMsg         = DVMSG_HSPRT_SNS;
            break;
        case SPR_PRM_END:
            strDrvInf_SPR.uiDrvHnd         = uiDrvHnd;
            strDrvInf_SPR.usParam          = usParam;
            strDrvInf_SPR.pvCallBackPtr    = nullptr;
            strDrvInf_SPR.uiWndMsg         = NULL;
            break;
        default:
            break;
        }
        lDrvRtn = (*pInfSPRUSB)(uifncNo, &strDrvInf_SPR);
        break;
    case SPR_DRV_FN_DATASEND:   // 数据发送控制
        switch (usParam)
        {
        case SPR_PRM_SEND_COMMAND:
            lpbySndLP_Buf = lpData;
            uiSndLP_sz = ulDataSz;
            if (uiSndLP_sz != 0)
            {
                strDrvInf_SPR.uiDrvHnd         = uiDrvHnd;
                strDrvInf_SPR.usParam          = usParam;
                strDrvInf_SPR.pvDataInBuffPtr  = lpbySndLP_Buf;
                strDrvInf_SPR.uiDataInBuffSz   = uiSndLP_sz;
                strDrvInf_SPR.uiTimer          = uiSendTimer;
                strDrvInf_SPR.pvCallBackPtr    = (void *)::N_SwitchFunc;
                strDrvInf_SPR.uiWndMsg         = DVMSG_HSPRT_SNDE;

            }
            else
            {
                return DVEP_RET_NoDt;
            }
            break;
        default:
            break;
        }
        lDrvRtn = (*pFnSPRUSB)(uifncNo, &strDrvInf_SPR);
        break;
    case SPR_DRV_FN_DATARCV:    // テ＂—タ受信制御
        switch (usParam)
        {
        case SPR_PRM_SEND_VENDOR_IN:
            switch (i)
            {
            case DRV_VENDOR_VRT:
                lpbyOutBuff = lpbyVrtLP_Buf;
                stVendorIn.byBmRequest = 0xC1;
                stVendorIn.byRequest = i;
                stVendorIn.wValue = wParamValue;
                stVendorIn.wIndex = 0;
                stVendorIn.wLength = wLength;
                break;
            case DRV_VENDOR_RESET:              // RESET执行
                wLength = SPR_SNS_BUFF_SIZE - 4;
                lpbyOutBuff = lpbyVendor_Buf;
                stVendorIn.byBmRequest = 0xC1;
                stVendorIn.byRequest = i;
                stVendorIn.wValue = 0;
                stVendorIn.wIndex = 0;
                stVendorIn.wLength = wLength;
                break;
            case DRV_VENDOR_TRACEABILITY:
            case DRV_VENDOR_DIPSET_REQ:
            case DRV_VENDOR_MAINT_COUNTER:
            case DRV_VENDOR_SETFUNCTION:
            case DRV_VENDOR_USB_ERROR:
            case DRV_VENDOR_ERROR_LOG:
            case DRV_VENDOR_HEADOPEN_DOT:
            case DRV_VENDOR_EXTRA_STATUS:
                lpbyOutBuff = lpbyVendor_Buf;
                stVendorIn.byBmRequest = 0xC1;
                stVendorIn.byRequest = i;
                stVendorIn.wValue = wParamValue;
                stVendorIn.wIndex = 0;
                stVendorIn.wLength = wLength;
                break;
            case DRV_VENDOR_ROLL_REPLENISH:
            case DRV_VENDOR_ROLL_SAVE:
                lpbyOutBuff = lpbyVendor_Buf;
                stVendorIn.byBmRequest = 0x41;
                stVendorIn.byRequest = i;
                stVendorIn.wValue = wParamValue;
                stVendorIn.wIndex = 0;
                stVendorIn.wLength = wLength;
                break;
            default:
                break;
            }
            memset(lpbySndLP_Buf, 0x00, SPR_SND_BUFF_SIZE);
            memcpy(lpbySndLP_Buf, (void *)&stVendorIn, sizeof(stVendorIn));
            strDrvInf_SPR.uiDrvHnd         = uiDrvHnd;
            strDrvInf_SPR.usParam          = usParam;
            strDrvInf_SPR.pvDataInBuffPtr  = lpbySndLP_Buf;
            strDrvInf_SPR.uiDataInBuffSz   = sizeof(stVendorIn);
            strDrvInf_SPR.pvDataOutBuffPtr = lpbyOutBuff;
            strDrvInf_SPR.uiDataOutReqSz   = wLength + 4;
            strDrvInf_SPR.uiTimer          = uiSendTimer;
            strDrvInf_SPR.pvCallBackPtr    = (void *)::N_SwitchFunc;
            strDrvInf_SPR.uiWndMsg         = DVMSG_HSPRT_SNDE;

            break;
        default:
            break;
        }
        lDrvRtn = (*pFnSPRUSB)(uifncNo, &strDrvInf_SPR);
        break;
    default:
        break;
    }

    // 下发Close指令，直接返回OK;否则，进入错误处理
    if (uifncNo == SPR_DRV_INF_CLOSE)
    {
        return OK;
    }
    else
    {
        // USB处理结果验证
        lRtn = lUsbDrvSyuryoCheck(lDrvRtn, DRV_RETURN_DRVKIDO);
    }

    // USB处理结果验证返回值处理
    if ((lRtn == OK) && (uifncNo == SPR_DRV_INF_INFGET))
    {
        if (usParam == SPR_PRM_SENS)
        {
            unsigned int length = 10;
            memcpy(lpbySensLP_Buf, &length, sizeof(unsigned int));
            if (strDrvInf_SPR.uiDataOutBuffSz < 10)
            {
                lRtn = DVEP_EPIJYOU;
            }
        }
        if (usParam == SPR_PRM_VRT)
        {
            if (strDrvInf_SPR.uiDataOutBuffSz < 16)
            {
                lRtn = DVEP_EPIJYOU;
            }
        }
    }
    else
    {

    }
    return lRtn;
}


/*****************************************************************************
** FunctionType : Private                                                   **
** FileName     : RPR_USB.CPP                                               **
** ClassName    : RPR_USB                                                   **
** Symbol       : RPR_USB::                                                 **
** Function     : long lUsbDrvSyuryoCheck()                                 **
** Describtion  : 指令处理结果验证                                             **
** Parameter    :                                                           **
** Return       :                                                           **
** Note         :                                                           **
** Date         : 2019/7/8                                                  **
** V-R-T        :                                                           **
** History      :                                                           **
*****************************************************************************/
long RPR_USB::lUsbDrvSyuryoCheck(long lDrvRtn, BYTE byProcType)
{
    long lRtn = OK;

    // 设备返回错误码记录
    ulUsbErrCode = lDrvRtn;

    if (lDrvRtn == USB_RtnCd_OK1)   // & 0x00000000
    {
        // 正常结束
        lRtn = OK;
    }
    else
    {
        // USB_RtnCd_OK2=0x33000000
        // USB_RtnCd_MSK_ERRLVL=0xFF000000
        if ((lDrvRtn & USB_RtnCd_MSK_ERRLVL) == USB_RtnCd_OK2)
        {
            // 正常结束
            lRtn = OK;
        }
        else
        {
            // 异常终了
            if ((lDrvRtn & USB_RtnCd_MSK_LPDWN) == 0) // & 0x20000000
            {
                // LP down(Win32API ERROR)
                lRtn   = DVEP_ERR_LPDWN;
                bLPDwn = TRUE;
            }
            else if (lDrvRtn == USB_RtnCd_CMD_TIMEOUT) // & 0xF5A00000
            {
                if (byProcType != DRV_RETURN_ONSENSER) // != serson变化通知
                {
                    // 超时（sensor通知以外）
                    lRtn = DVEP_ERR_CMDTOUT;
                }
            }
            else
            {
                lRtn = DVEP_ERR_EPDWN;
                bEPDwn = TRUE;
            }
        }
    }

    return lRtn;
}












