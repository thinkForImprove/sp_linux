#include "DevSIG_TSD64.h"

// SIG 版本号
static const char *ThisFile = "DevSIG_TSD64.cpp";

//////////////////////////////////////////////////////////////////////////

CDevSIG_TSD64::CDevSIG_TSD64(LPCSTR lpDevType)
{
    ZeroMemory(&m_stStatus, sizeof(DEVCAMSTATUS));
    ZeroMemory(m_pSaveFileName, sizeof(m_pSaveFileName));

    m_pchErrCode = new CHAR[MAX_ERRCODE];
    ZeroMemory(m_pchErrCode, MAX_ERRCODE);

    m_bDevOpenOk = FALSE;
    bDisplyOK = FALSE;              // 签名窗口未打开
    m_lLength = 0;
    m_wAction = CAM_UNKNOWN;        // 窗口操作

    m_sSigIniConfig.init();
    SetLogFile(LOGFILE, ThisFile, lpDevType);
}

CDevSIG_TSD64::~CDevSIG_TSD64()
{
    Close();
}

void CDevSIG_TSD64::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    return;
}

int CDevSIG_TSD64::Open()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_bDevOpenOk = TRUE;

    return CAM_SUCCESS;
}

int CDevSIG_TSD64::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    vEndSignature(m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("endSiganture fail.");

    if (m_pchErrCode != nullptr)
    {
        delete[] m_pchErrCode;
        m_pchErrCode = nullptr;
    }

    m_bDevOpenOk = FALSE;
    return CAM_SUCCESS;
}

int CDevSIG_TSD64::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    vResetDev(m_pchErrCode); 
    TSD64API_ERRORCODE_RETURN("resetDev fail.");

    bDisplyOK = FALSE;
    return CAM_SUCCESS;
}

int CDevSIG_TSD64::GetStatus(DEVCAMSTATUS &stStatus)
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_stStatus.fwDevice[CAM_EXTRA] = DEVICE_OFFLINE;

    if (m_bDevOpenOk == TRUE)
    {
        m_stStatus.fwDevice[CAM_EXTRA] = DEVICE_ONLINE;
    } 

    vGetDevStatus(m_pchErrCode);
    if (strcmp(m_pchErrCode, YGB0000001) == 0)
    {
        m_stStatus.fwMedia[CAM_EXTRA] = MEDIA_OK;
    } else if (strcmp(m_pchErrCode, YGB0000002) == 0)
    {
        m_stStatus.fwDevice[CAM_EXTRA] = DEVICE_BUSY;
        m_stStatus.fwMedia[CAM_EXTRA] = MEDIA_BUSY;
    } else if (strcmp(m_pchErrCode, YGB0000003) == 0)
    {
        m_stStatus.fwDevice[CAM_EXTRA] = DEVICE_OFFLINE;
        m_stStatus.fwMedia[CAM_EXTRA] = MEDIA_UNKNOWN;
        stStatus = m_stStatus;
        return GetErrorCode(m_pchErrCode);
    } else
    {
        m_stStatus.fwDevice[CAM_EXTRA] = DEVICE_UNKNOWN;
        m_stStatus.fwMedia[CAM_EXTRA] = MEDIA_NOTSUPP;
        stStatus = m_stStatus;
        return GetErrorCode(m_pchErrCode);
    }

    stStatus = m_stStatus;
    return CAM_SUCCESS;
}

int CDevSIG_TSD64::GetFWVersion(LPSTR pFWVersion)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    vGetFirmwareVer(pFWVersion, m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("getFirmwareVer fail.");

    return CAM_SUCCESS;
}

int CDevSIG_TSD64::SetProperty(const SIGCONFIGINFO stSigConfig)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    m_sSigIniConfig = stSigConfig;
    // 设置笔迹颜色和宽度
    if (m_sSigIniConfig.wPenWidth > 5)
        m_sSigIniConfig.wPenWidth = 5;
    else if (m_sSigIniConfig.wPenWidth <= 0)
        m_sSigIniConfig.wPenWidth = 1;

   return CAM_SUCCESS;
}

// 窗口操作
int CDevSIG_TSD64::Display(WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight, WORD wHpixel, WORD wVpixel, LPSTR pszTexData, WORD wAlwaysShow, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    if ((bDisplyOK && wAction == CAM_CREATE) ||
            (bDisplyOK && wAction == CAM_RESUME) ||
            (wAction == CAM_ERASE) ||
            (!bDisplyOK && wAction == CAM_DESTROY) ||
            (!bDisplyOK && wAction == CAM_PAUSE))
        return CAM_SUCCESS;

    int nRet = 0;
    m_wAction = (CAM_WINDOW)wAction;
    std::string strTexData;                 // 原生字符串
    std::string::size_type stPosition1 = 0; // 等号(=)位置索引
    std::string::size_type stPosition2 = 0; // 分号(;)位置索引
    std::string strBackgroundTex;           // 背景文字
    std::string strBackgroundPath;          // 图片路径
    ULONG dwStart = CQtTime::GetSysTick();

    // 设置背景文字及图片
    if (pszTexData != NULL && !bDisplyOK)
    {
        strTexData = pszTexData;
        int strLen = 0;
        if((stPosition1 = strTexData.find("=")) != std::string::npos)
        {
            if((stPosition2 = strTexData.find(";")) != std::string::npos)
            {
                strLen = stPosition2 - stPosition1;
                strBackgroundTex = strTexData.substr(stPosition1 + 1, strLen - 1);

                if ((stPosition1 = strTexData.rfind("=")) != std::string::npos)
                {
                    strLen = stPosition2 - stPosition1;
                    strBackgroundPath = strTexData.substr(stPosition1 + 1, strLen - 1);
                }
            }
        }

//        if (strBackgroundTex.empty() && strBackgroundPath.empty())
//            return ERR_INVALID_PARA;
    }

    if (m_wAction == CAM_CREATE)
    {
        // 创建窗口
        nRet = bCreateWindow(wX, wY, wWidth, wHeight, strBackgroundTex, strBackgroundPath);
        if (nRet != 0)
            return nRet;

        bDisplyOK = TRUE;

    } else if (m_wAction == CAM_DESTROY)
    {
        // 销毁窗口
        vEndSignature(m_pchErrCode);
        TSD64API_ERRORCODE_RETURN("endSignature fail.");

        bDisplyOK = FALSE;

    } else if (m_wAction == CAM_PAUSE)
    {
        // 隐藏窗口
        vHideSignWindow(m_pchErrCode);
        if ((strcmp(m_pchErrCode, YGB0000002) != 0 )
                && (strcmp(m_pchErrCode, DRV0000000) != 0))
        {
            TSD64API_ERRORCODE_RETURN("hideSignWindow fail.");
        }

        bDisplyOK = FALSE;

    } else if (m_wAction == CAM_RESUME)
    {
        // 恢复窗口
        vShowSignWindow(m_pchErrCode);
        if ((strcmp(m_pchErrCode, YGB0000002) != 0 )
                && (strcmp(m_pchErrCode, DRV0000000) != 0))
        {
            TSD64API_ERRORCODE_RETURN("showSignWindow fail.");
        }

        bDisplyOK = TRUE;
    }

    // 超时操作
//    if (bDisplyOK)
//    {
//        while(true)
//        {
//            if (dwTimeout != INFINITE)
//            {
//                if ((CQtTime::GetSysTick() - dwStart) >= dwTimeout)
//                {
//                    return ERR_TIMEOUT;
//                }
//            }
//            vGetSignData(m_pSignData, &m_lLength, m_pchErrCode);
//            if (m_lLength <= 0)
//                continue;
//            else
//                break;
//        }
//    }

    return CAM_SUCCESS;
}

// 获取签名
int CDevSIG_TSD64::GetSignature(LPSTR pszKey, LPSTR pszEncKey, LPSTR pszPictureFile, LPSTR pszSignatureFile, DWORD wnd, WORD wEncypt, LPSTR pszCamData, LPWSTR pswzUnicodeCamData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    BYTE ucData[SIGNATURE_DATA_SIZE] = {0};                   // 存放签名数据
    long llen = SIGNATURE_DATA_SIZE;
    int nRet = 0;
    std::string strPictureFile = pszPictureFile;
    BOOL bisPng = FALSE;

    if (wEncypt == ENCRYPT_NONE)
    {
        pszEncKey = NULL;
    } else {
        // 灌注主密钥
        nRet = LoadKey(pszKey, 8, KEY_INDEX_1, KEY_INDEX_3, wEncypt, KEY_MASTER);
        if (nRet < 0)
            return nRet;
        // 灌注工作密钥
        nRet = LoadKey(pszEncKey, 8, KEY_INDEX_2, KEY_INDEX_4, wEncypt, KEY_ENCTRACK);
        if (nRet < 0)
            return nRet;
    }

    // 获取签字窗口的图片,png格式是透明图片
    if (strPictureFile.empty())
    {
        Log(ThisFile, __LINE__, "数据保存图片失败，路径为空");
        return ERR_INVALID_PARA;
    }

    // jpg/bmp -> png
    std::string strFormat = getStringLastNChar(strPictureFile, 3);
    if (strFormat.compare("png") != 0)
    {
        bisPng = FALSE;
        strPictureFile = strPictureFile.substr(0, strPictureFile.size() - 3).append("png");
    } else
        bisPng = TRUE;

    // 获取png图片
//    vGetPngPicture(pszPictureFile, 1, m_pchErrCode);
//    TSD64API_ERRORCODE_RETURN("getPngPicture fail.");

    // 获取明文轨迹数据(仅能保存png图片)
    vGetSignature(ucData, &llen, KEY_INDEX_1, (LPBYTE)pszEncKey, (LPSTR)strPictureFile.c_str(), m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("getSignature fail.");

    //png -> jpg/bmp
    if (!bisPng)
    {
        QImage img(QString::fromStdString(strPictureFile));
        nRet = nPngToOtherFormat(img, strPictureFile.substr(0, strPictureFile.size() - 3).append(strFormat).c_str(), (LPSTR)strFormat.c_str());
        if (nRet < 0)
            return nRet;

        // 删除png图片
        nRet = remove(strPictureFile.substr(0, strPictureFile.size() - 3).append("png").c_str());
        if (nRet != 0)
            return nRet;
//        nRet = nImageFormatTransform(strPictureFile.substr(0, strPictureFile.size() - 3).append(strFormat),
//                                     strPictureFile, (LPSTR)strFormat.c_str());
//        if (nRet < 0)
//            return nRet;
    }

    //存放数据至pszSignatureFile下的*.txt文件
    //type:1 w,h,P1024(x,y,z,timestamp;)
    //type:2 w,h,1024(x,y,z/1023,deltatimems;)
    //type:3 w,h,1024(x,y,z/1023;)
    //type:4 w,h,Pxx,(x,y,z;)
    //type:5 w,h,1024(x,y,z/1023,deltatimeus;)
    vGetSignDataFile(pszSignatureFile, 5, m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("getSignDataFile fail.");

//    nRet = nSaveDataToImage((LPBYTE)ucData, pszSignatureFile, llen);
//    if (nRet < 0)
//        return nRet;

    return CAM_SUCCESS;
}

// 清除签名
int CDevSIG_TSD64::ClearSignature()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    vClearSignature(m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("clearSignature fail.");

    return CAM_SUCCESS;
}

BOOL CDevSIG_TSD64::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

int CDevSIG_TSD64::LoadKey(LPSTR bKey, int iLength, int iIndex, int iDecKeyNum, int iDecMode, int iUse)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 灌注密钥
    vImportKey(bKey, iLength, iIndex, iDecKeyNum, iDecMode, iUse, m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("importKey fail.");

    vGetKeyVerificationCode(bKey, &iLength, iIndex, iDecMode, m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("getKeyVerificationCode fail.");

    return CAM_SUCCESS;
}

int CDevSIG_TSD64::nSaveDataToImage(LPBYTE pData, LPCSTR filename, int nLen)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    FILE *fp = fopen(filename, "w+");

    if (fp == 0) {
        return WFS_ERR_INTERNAL_ERROR;
    }

    fwrite(pData, nLen, 1, fp);
    fclose(fp);

    return WFS_SUCCESS;
}

int CDevSIG_TSD64::bReadImageData(LPSTR lpImgPath, LPBYTE lpImgData)
{
    FILE* fp;

    if ((fp = fopen(lpImgPath, "rb")) == NULL)
    {
        return WFS_ERR_INTERNAL_ERROR;
    }

    fseek(fp, 0, SEEK_END);
    m_lLength = ftell(fp);
    rewind(fp);
    fread((LPSTR)lpImgData, m_lLength, 1, fp);

    fclose(fp);

    return CAM_SUCCESS;
}

int CDevSIG_TSD64::nImageFormatTransform(std::string pDest, std::string pPicture, LPSTR pFormat)
{
    std::string strFileName(pPicture);
    std::string strTempFileName(pDest);
    QImageWriter imageWriter;
    imageWriter.setFormat("png"); //设置输出格式
    imageWriter.setFileName(QString::fromUtf8(strFileName.c_str())); //设置输出文件名
    if(imageWriter.canWrite() )
    {
       if (!imageWriter.write(QImage(strTempFileName.c_str(), pFormat))) //读取文件并转换
       {
            Log(ThisFile, 1, "图片[%s] to [%s]格式转换错误, 错误原因:<%s>", pDest.c_str(), pPicture.c_str(),
                imageWriter.errorString().toStdString().c_str());

            return ERR_IMAGEFORMAT;
       }
    }

    return CAM_SUCCESS;
}

int CDevSIG_TSD64::nPngToOtherFormat(QImage pngImage, LPCSTR pDest, LPSTR pFormat)
{
    QImage Image(pngImage.size(), QImage::Format_ARGB32);
    Image.fill(QColor(Qt::white).rgb());
    QPainter painter(&Image);
    painter.drawImage(0, 0, pngImage);
    if (!Image.save(pDest, pFormat))
    {
        Log(ThisFile, 1, "图片[%s<png>] to [%s<%s>]格式转换错误", pDest, pDest, pFormat);
        return -1;
    }

    return 0;
}

int CDevSIG_TSD64::bCreateWindow(WORD wX, WORD wY, WORD wWidth, WORD wHeight, std::string strBackgroundTex, std::string strBackgroundPath)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 设置签名窗口
    vSetSignWindow(wX, wY, wWidth, wHeight, m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("setSignWindow fail.");

    // 设置背景色和透明度
    vSetBackColorParam(m_sSigIniConfig.wTransparency, m_sSigIniConfig.wBackColor, m_sSigIniConfig.wIsUseBackColor, m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("SetBackColorParam fail.");
    if (!strBackgroundPath.empty())
    {
        // 设置签字背景图片
        vSetBackgroundPicture((LPSTR)strBackgroundPath.c_str(), m_sSigIniConfig.wAlwaysShow, m_pchErrCode);
        TSD64API_ERRORCODE_RETURN("SetBackgroundPicture fail.");
    }

    if (!strBackgroundTex.empty())
    {
        // 设置文本
        vSetTextData(0, 0, strBackgroundTex.c_str(), "宋体", 20, m_sSigIniConfig.wTextColor, m_sSigIniConfig.wAlwaysShow, m_pchErrCode);
        TSD64API_ERRORCODE_RETURN("SetTextData fail.");
    }

    // 设置笔的粗细
    if (wWidth >= 300 && wHeight >= 300)
        m_sSigIniConfig.wPenWidth = 5;
    else
        m_sSigIniConfig.wPenWidth = 3;
    vSetPenMax(m_sSigIniConfig.wPenWidth, m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("SetPenMax fail.");

    // 启动窗口
    vStartSignatureUseSetting(m_pchErrCode);
    TSD64API_ERRORCODE_RETURN("startSignatureUseSetting fail.");

    return 0;
}

std::string CDevSIG_TSD64::getStringLastNChar(std::string str, ULONG lastN)
{
    return str.substr(str.size() - lastN);
}

int CDevSIG_TSD64::GetErrorCode(LPSTR pchErrCode)
{
    if (strcmp(m_pchErrCode, DRV0000001) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_OTHER, ErrorCode:%s", pchErrCode);
        return ERR_OTHER;
    } else if (strcmp(m_pchErrCode, DRV0000002) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_TIMEOUT, ErrorCode:%s", pchErrCode);
        return ERR_TIMEOUT;
    } else if (strcmp(m_pchErrCode, DRV0000003) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_OPENSERIAL, ErrorCode:%s", pchErrCode);
        return ERR_OPENSERIAL;
    } else if (strcmp(m_pchErrCode, DRV0000004) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_SENDDATA, ErrorCode:%s", pchErrCode);
        return ERR_SENDDATA;
    } else if (strcmp(m_pchErrCode, DRV0000005) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_INVALID_PARA, ErrorCode:%s", pchErrCode);
        return ERR_INVALID_PARA;
    } else if (strcmp(m_pchErrCode, DRV0000006) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_NOFINDDLL, ErrorCode:%s", pchErrCode);
        return ERR_NOFINDDLL;
    } else if (strcmp(m_pchErrCode, DRV0000007) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_LOADDLL, ErrorCode:%s", pchErrCode);
        return ERR_LOADDLL;
    } else if (strcmp(m_pchErrCode, YGB0000002) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_DEVBUZY, ErrorCode:%s", pchErrCode);
        return ERR_DEVBUZY;
    } else if (strcmp(m_pchErrCode, YGB0000003) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_DEVOFF, ErrorCode:%s", pchErrCode);
        return ERR_DEVOFF;
    } else if (strcmp(m_pchErrCode, DRVYGB0001) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_USERNOSIGN, ErrorCode:%s", pchErrCode);
        return ERR_USERNOSIGN;
    } else if (strcmp(m_pchErrCode, DRVYGB0002) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_IMAGEFORMAT, ErrorCode:%s", pchErrCode);
        return ERR_IMAGEFORMAT;
    } else if (strcmp(m_pchErrCode, DRVYGB0003) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_SAVEIMAGE, ErrorCode:%s", pchErrCode);
        return ERR_SAVEIMAGE;
    } else if (strcmp(m_pchErrCode, DRVYGB0004) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_CREATESIGNFILE, ErrorCode:%s", pchErrCode);
        return ERR_CREATESIGNFILE;
    } else if (strcmp(m_pchErrCode, DRVYGB0005) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_UNCONNECTDEV, ErrorCode:%s", pchErrCode);
        return ERR_UNCONNECTDEV;
    } else if (strcmp(m_pchErrCode, DRVYGB0006) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_RANGEWINDOWSIZE, ErrorCode:%s", pchErrCode);
        return ERR_RANGEWINDOWSIZE;
    } else if (strcmp(m_pchErrCode, DRVYGB0007) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_RANGEPARAM, ErrorCode:%s", pchErrCode);
        return ERR_RANGEPARAM;
    } else if (strcmp(m_pchErrCode, DRVYGB0008) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_IMPORTKEY, ErrorCode:%s", pchErrCode);
        return ERR_IMPORTKEY;
    } else if (strcmp(m_pchErrCode, DRVYGB0009) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_OTHER, ErrorCode:%s", pchErrCode);
        return ERR_OTHER;
    } else if (strcmp(m_pchErrCode, DRVYGB0010) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_ENCSIGNATURE, ErrorCode:%s", pchErrCode);
        return ERR_ENCSIGNATURE;
    } else if (strcmp(m_pchErrCode, DRVYGB0011) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_NOSTARTSIGN, ErrorCode:%s", pchErrCode);
        return ERR_NOSTARTSIGN;
    } else if (strcmp(m_pchErrCode, DRVYGB0012) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_NOENDSIGN, ErrorCode:%s", pchErrCode);
        return ERR_NOENDSIGN;
    } else if (strcmp(m_pchErrCode, DRVYGB0013) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_INVALID_HWND, ErrorCode:%s", pchErrCode);
        return ERR_INVALID_HWND;
    } else if (strcmp(m_pchErrCode, DRVYGB0014) == 0)
    {
        Log(ThisFile, __LINE__, "ERR_UNSUPPVERSION, ErrorCode:%s", pchErrCode);
        return ERR_UNSUPPVERSION;
    } else {
        return ERR_OTHER;
    }
}
