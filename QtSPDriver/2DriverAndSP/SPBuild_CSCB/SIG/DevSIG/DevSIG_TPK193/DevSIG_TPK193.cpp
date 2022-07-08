#include "DevSIG_TPK193.h".h"

// SIG 版本号
static const char *ThisFile = "DevSIG_TPK193.cpp";

//////////////////////////////////////////////////////////////////////////

CDevSIG_TPK193::CDevSIG_TPK193(LPCSTR lpDevType)
{
    ZeroMemory(&m_stStatus, sizeof(DEVCAMSTATUS));
    ZeroMemory(m_pSaveFileName, sizeof(m_pSaveFileName));

    m_bDevOpenOk = FALSE;
    bDisplyOK = FALSE;              // 签名窗口未打开
    m_lLength = 0;
    m_wAction = CAM_UNKNOWN;        // 窗口操作

    m_sSigIniConfig.init();
    SetLogFile(LOGFILE, ThisFile, lpDevType);
}

CDevSIG_TPK193::~CDevSIG_TPK193()
{
    Close();
}

void CDevSIG_TPK193::Release()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //delete this;
    return;
}

int CDevSIG_TPK193::Open()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    int nRet = GetConnectStatus();
    if (nRet != 0)
    {
        Log(ThisFile, 0, "GetConnectStatus fail. ret: %d", nRet);
        return ERR_DEVOFF;
    }

    m_bDevOpenOk = TRUE;

    return CAM_SUCCESS;
}

int CDevSIG_TPK193::Close()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    EndSignature();
    m_bDevOpenOk = FALSE;
    return CAM_SUCCESS;
}

int CDevSIG_TPK193::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    int nRet = ResetDevice();
    if (nRet != 0)
    {
        Log(ThisFile, 0, "ResetDevice fail. ret: %d", nRet);
        return ERR_OTHER;
    }

    bDisplyOK = FALSE;
    return CAM_SUCCESS;
}

int CDevSIG_TPK193::GetStatus(DEVCAMSTATUS &stStatus)
{
    //THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    int nRet = -1;
    if (m_bDevOpenOk)
    {
        m_stStatus.fwDevice[CAM_EXTRA] = DEVICE_ONLINE;
        nRet = GetDeviceStatus();
        if (nRet != 0)
        {
            Log(ThisFile, 0, "GetDeviceStatus fail. ret: %d", nRet);
            return ERR_OTHER;
        }
    } else
    {
        m_stStatus.fwDevice[CAM_EXTRA] = DEVICE_OFFLINE;
    }

    if (nRet == 0)
        m_stStatus.fwMedia[CAM_EXTRA] = MEDIA_OK;
    else if (nRet == 1)
        m_stStatus.fwMedia[CAM_EXTRA] = MEDIA_BUSY;
    else
        m_stStatus.fwMedia[CAM_EXTRA] = MEDIA_UNKNOWN;

    stStatus = m_stStatus;
    return CAM_SUCCESS;
}

int CDevSIG_TPK193::GetFWVersion(LPSTR pFWVersion)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    return CAM_SUCCESS;
}

int CDevSIG_TPK193::SetProperty(const SIGCONFIGINFO stSigConfig)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    Log(ThisFile, 0, "st_sig_config_info: [szDevDllName]:<%s>, [wEncrypt]:<%d>, [pszPsgPath]:<%s>, [szImagePath]:<%s>, [wPenWidth]:<%d>, [wPenMultiple]:<%d>, [wPenColor]:<%d>, [wTextColor]:<%04X>, [wBackColor]:<%04X>, [wIsUseBackColor]:<%d>, [wTransparency]:<%d>, [wAlwaysShow]:<%d>, [wSaveTime]:<%d>, [pszKey]:<%s>, [pszEncKey]:<%s>, [pszSignImagePath]:<%s>, [wAPILog]:<%d>, [pszLogFile]:<%s>",
        stSigConfig.szDevDllName, stSigConfig.wEncrypt, stSigConfig.pszPsgPath, stSigConfig.szImagePath,
        stSigConfig.wPenWidth, stSigConfig.wPenMultiple, stSigConfig.wPenColor, stSigConfig.wTextColor,
        stSigConfig.wBackColor, stSigConfig.wIsUseBackColor, stSigConfig.wTransparency, stSigConfig.wAlwaysShow,
        stSigConfig.wSaveTime, stSigConfig.pszKey, stSigConfig.pszEncKey,
        stSigConfig.pszSignImagePath, stSigConfig.wAPILog, stSigConfig.pszLogFile);

    int nRet = 0;
    m_sSigIniConfig = stSigConfig;

    // 设置笔迹颜色和宽度
    if (m_sSigIniConfig.wPenWidth > 5)
        m_sSigIniConfig.wPenWidth = 5;
    else if (m_sSigIniConfig.wPenWidth <= 0)
        m_sSigIniConfig.wPenWidth = 1;

    nRet = SetPenColor(m_sSigIniConfig.wPenColor);
    if (nRet != 0)
    {
        Log(ThisFile, 0, "SetPenColor fail. ret: %d", nRet);
        return ERR_OTHER;
    }
    nRet = SetPenWidth(m_sSigIniConfig.wPenWidth);
    if (nRet != 0)
    {
        Log(ThisFile, 0, "SetPenWidth fail. ret: %d", nRet);
        return ERR_OTHER;
    }

    // 设置Background颜色,写死白色(3)
    nRet = SetBackgroundColor(3);
    if (nRet != 0)
    {
        Log(ThisFile, 0, "SetBackgroundColor fail. ret: %d", nRet);
        return ERR_OTHER;
    }

    // 设置日志
    if (m_sSigIniConfig.wAPILog == 1)
    {
        nRet = LogEnable(true);
        if (nRet != 0)
        {
            Log(ThisFile, 0, "LogEnable fail. ret: %d", nRet);
            return ERR_OTHER;
        }
        nRet = SetlogPath(m_sSigIniConfig.pszLogFile);
        if (nRet != 0)
        {
            Log(ThisFile, 0, "SetlogPath fail. ret: %d", nRet);
            return ERR_OTHER;
        }
    }
    else
    {
        nRet = LogEnable(false);
        if (nRet != 0)
        {
            Log(ThisFile, 0, "LogEnable fail. ret: %d", nRet);
            return ERR_OTHER;
        }
    }

   return CAM_SUCCESS;
}

// 窗口操作
int CDevSIG_TPK193::Display(WORD wAction, WORD wX, WORD wY, WORD wWidth, WORD wHeight, WORD wHpixel, WORD wVpixel, LPSTR pszTexData, WORD wAlwaysShow, DWORD dwTimeout)
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
    }

    if (m_wAction == CAM_CREATE)
    {
        // 创建窗口
        nRet = bCreateWindow(wX, wY, wWidth, wHeight, strBackgroundTex, strBackgroundPath);
        //nRet = StartSignature(wX, wY, wWidth, wHeight);
        if (nRet != 0)
            return nRet;

        bDisplyOK = TRUE;

    } else if (m_wAction == CAM_DESTROY)
    {
        // 销毁窗口
        EndSignature();
        bDisplyOK = FALSE;

    } else if (m_wAction == CAM_PAUSE)
    {
        // 隐藏窗口
        //vHideSignWindow();
        //bDisplyOK = FALSE;

    } else if (m_wAction == CAM_RESUME)
    {
        // 恢复窗口
        //ShowSignData(m_pchErrCode);

        //bDisplyOK = TRUE;
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
int CDevSIG_TPK193::GetSignature(LPSTR pszKey, LPSTR pszEncKey, LPSTR pszPictureFile, LPSTR pszSignatureFile, DWORD wnd, WORD wEncypt, LPSTR pszCamData, LPWSTR pswzUnicodeCamData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //AutoMutex(m_cMutex);

    BYTE ucData[SIGNATURE_DATA_SIZE] = { 0x00 };                   // 存放签名数据
    int nLen = 0;
    int nRet = 0;
    std::string strPictureFile = pszPictureFile;
    BOOL bisPng = FALSE;

    // 设置路径
    nRet = SetPath(pszSignatureFile, pszPictureFile);
    if (nRet != 0)
    {
        Log(ThisFile, 0, "SetPath fail. ret: %d, SignatureFile: %s, PictureFile: %s",
            nRet, pszSignatureFile, pszPictureFile);
        return ERR_OTHER;
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


    if (wEncypt == ENCRYPT_NONE)
    {
        pszEncKey = NULL;
        // 灌注主密钥
        nRet = nLoadKey(pszKey, 0, KEY_INDEX_0, KEY_INDEX_0, wEncypt, KEY_PLAIN, 0);
        if (nRet < 0)
            return ERR_OTHER;
        // 输出签名数据到指定路径。
        nRet = GetSignatureNoEnc(ucData);
        if (nRet != 0)
        {
            Log(ThisFile, 0, "GetSignatureNoEnc fail. ret: %d", nRet);
            return ERR_OTHER;
        }
    } else {
        // 灌注主密钥
        nRet = nLoadKey(pszKey, strlen(pszKey), KEY_INDEX_1, KEY_INDEX_1, wEncypt, KEY_INDEX_2, 2);
        if (nRet < 0)
            return ERR_OTHER;

        // 灌注工作密钥
        nRet = nLoadKey(pszEncKey, strlen(pszEncKey), KEY_INDEX_2, KEY_INDEX_3, wEncypt, KEY_INDEX_4, 2);
        if (nRet < 0)
            return ERR_OTHER;

        nRet = GetSign(ucData, &nLen, KEY_INDEX_3);
        if (nRet != 0)
        {
            Log(ThisFile, 0, "GetSignature fail. ret: %d, len: %d", nRet, nLen);
            return ERR_OTHER;
        }
        //GetSignatureWin(ucData, KEY_INDEX_1, (LPBYTE)pszEncKey, wEncypt);
    }

    // TODO TEST: save png
//    nRet = nSaveDataToImage((LPBYTE)ucData, pszPictureFile, SIGNATURE_DATA_SIZE);
//    if (nRet < 0)
//        return nRet;
    // TODO END

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
    }

    //存放数据至pszSignatureFile下的*.txt文件
    nRet = nSaveDataToImage((LPBYTE)ucData, pszSignatureFile, SIGNATURE_DATA_SIZE);
    if (nRet < 0)
        return nRet;

    return CAM_SUCCESS;
}

// 清除签名
int CDevSIG_TPK193::ClearSignature()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    int nRet = ClearSign();
    if (nRet != 0)
    {
        Log(ThisFile, 0, "ClearSignature fail. ret: %d", nRet);
        return ERR_OTHER;
    }

    return CAM_SUCCESS;
}

BOOL CDevSIG_TPK193::IsDeviceOpen()
{
    return (m_bDevOpenOk == TRUE ? TRUE : FALSE);
}

int CDevSIG_TPK193::nLoadKey(LPSTR keydata, int datalen, int keyuse, int index, int algorithm, int decodekey, int checkmode)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    // 灌注密钥
    int nRet = LoadKey(keydata, datalen, keyuse, index, algorithm, decodekey, checkmode);
    if (nRet < 0)
    {
        Log(ThisFile, 0, "LoadKey fail. ret: %d", nRet);
        return nRet;
    }
    nRet = CheckKCV((LPBYTE)keydata, datalen);
    if (nRet != 0)
    {
        Log(ThisFile, 0, "CheckKCV fail. 密钥校验不一致, ret: %d", nRet);
        return nRet;
    }

    return CAM_SUCCESS;
}

int CDevSIG_TPK193::nSaveDataToImage(LPBYTE pData, LPCSTR filename, int nLen)
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

int CDevSIG_TPK193::bReadImageData(LPSTR lpImgPath, LPBYTE lpImgData)
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

int CDevSIG_TPK193::nImageFormatTransform(std::string pDest, std::string pPicture, LPSTR pFormat)
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

int CDevSIG_TPK193::nPngToOtherFormat(QImage pngImage, LPCSTR pDest, LPSTR pFormat)
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

int CDevSIG_TPK193::bCreateWindow(WORD wX, WORD wY, WORD wWidth, WORD wHeight, std::string strBackgroundTex, std::string strBackgroundPath)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(m_cMutex);

    int nRet = 0;
    if (!strBackgroundPath.empty())
    {
        // 设置签字背景图片
        nRet = SetBackgroundImage((LPSTR)strBackgroundPath.c_str());
        if (nRet != 0)
        {
            Log(ThisFile, 0, "SetBackgroundImage fail. ret: %d", nRet);
            return ERR_OTHER;
        }
    }

// 辰展暂不支持设置背景文本
//    if (!strBackgroundTex.empty())
//    {
//        TODO
//    }

    // 启动窗口
    nRet = StartSignature(wX, wY, wWidth, wHeight);
    if (nRet != 0)
    {
        Log(ThisFile, 0, "StartSignature fail. ret: %d, wX: %d, wY: %d, width: %d, height: %d",
            nRet, wX, wY, wWidth, wHeight);
        return ERR_OTHER;
    }

    return 0;
}

std::string CDevSIG_TPK193::getStringLastNChar(std::string str, ULONG lastN)
{
    return str.substr(str.size() - lastN);
}
