#include "XFS_SIG.h"

#include <algorithm>
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
//#include <QSettings>
#include <QMetaType>
#include <QWindow>

// SIG SP 版本号
BYTE    byVRTU[17] = {"HWSIGSTE00000100"};

static const char *DEVTYPE = "SIG";
static const char *ThisFile = "XFS_SIG.cpp";
//////////////////////////////////////////////////////////////////////////
CXFS_SIG::CXFS_SIG(): m_pMutexGetStatus(nullptr)
{
    strcpy(m_szLogType, DEVTYPE);
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    ZeroMemory(&m_sSigIniConfig, sizeof(SIGCONFIGINFO));
    ZeroMemory(&m_stCamInfo, sizeof(SIGHOWWININFO));
    ZeroMemory(&m_stStatus, sizeof(WFSCAMSTATUS));
    ZeroMemory(&m_stCaps, sizeof(WFSCAMCAPS));
    m_pszData = NULL;
    m_iLength = 0;
    m_sSigIniConfig.init();
    qRegisterMetaType<SIGHOWWININFO>("SIGHOWWININFO");
    connect(this, SIGNAL(vSignShowWin(SIGHOWWININFO)), this, SLOT(vSlotShowWin(SIGHOWWININFO)), Qt::AutoConnection);
    connect(this, SIGNAL(vSignHideWin()), this, SLOT(vSlotHideWin()), Qt::AutoConnection);
}

CXFS_SIG::~CXFS_SIG()
{
    OnClose();
}

HRESULT CXFS_SIG::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 加载BaseCAM
    if (0 != m_pBase.Load("SPBaseSIG.dll", "CreateISPBaseSIG", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBaseSIG失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return 0;
}

HRESULT CXFS_SIG::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;
    CHAR pFWVersion[MAX_LEN_FWVERSION] = {0x00};

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();

    // 读INI
    InitConifig();

    // 初始化设备状态结构
    InitStatus();

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    if ((access(LOG_SAVE_PATH, F_OK)) == -1)
    {
        mkdir(LOG_SAVE_PATH, S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    } else if ((access(SIGNATURE_FILE_SAVE_PATH, F_OK)) == -1)
    {
        mkdir(SIGNATURE_FILE_SAVE_PATH, S_IRUSR | S_IWUSR | S_IXUSR | S_IRWXG | S_IRWXO);
    }

    // 加载设备驱动动态库
    if (m_pDev == NULL)
    {
        if (m_sSigIniConfig.wDeviceType == 0)
            memcpy(m_szDevType, IDEV_TYPE_TSD, 3);
        else if (m_sSigIniConfig.wDeviceType == 1)
            memcpy(m_szDevType, IDEV_TYPE_TPK, 3);
        else
            return WFS_ERR_HARDWARE_ERROR;

        hRet = m_pDev.Load(m_sSigIniConfig.szDevDllName, "CreateIDevSIG", m_szDevType);
        if (hRet != 0)
        {
            Log(ThisModule, __LINE__,
                "加载库失败: DriverDllName=%s, ReturnCode:%s, DevType:%s",
                m_sSigIniConfig.szDevDllName,
                m_pDev.LastError().toUtf8().constData(),
                m_szDevType);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    // 打开连接
    hRet = m_pDev->Open();
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "Open fail．ReturnCode:%d", hRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 设置属性
    hRet = m_pDev->SetProperty(m_sSigIniConfig);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "设置属性失败．ReturnCode:%d", hRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    hRet = m_pDev->GetFWVersion((LPSTR)pFWVersion);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "获取固件版本失败．ReturnCode:%d", hRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    m_cExtra.AddExtra("VRTCount", "4");
    m_cExtra.AddExtra("VRTDetail[00]", (LPSTR)byVRTU);
    m_cExtra.AddExtra("VRTDetail[01]", (LPSTR)pFWVersion);

    // 更新一次状态
    if (OnStatus() < 0)
        return WFS_ERR_HARDWARE_ERROR;

    Log(ThisModule, 1, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    return WFS_SUCCESS;
}

HRESULT CXFS_SIG::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pDev != nullptr)
    {
        m_pDev->Close();
    }

//    if (showWin != nullptr)
//    {
//        delete showWin;
//        showWin = nullptr;
//    }

    if(m_pszData != nullptr)
    {
        delete[] m_pszData;
        m_pszData = nullptr;
        m_iLength = 0;
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_SIG::OnStatus()
{
    // 空闲更新状态
    DEVCAMSTATUS stStatus;
    if (m_pDev != nullptr)
    {
        if (m_pDev->GetStatus(stStatus) < 0)
            return WFS_ERR_HARDWARE_ERROR;
    }
    UpdateStatus(stStatus);

    return WFS_SUCCESS;
}

HRESULT CXFS_SIG::OnCancelAsyncRequest()
{
    return WFS_SUCCESS;
}

HRESULT CXFS_SIG::OnUpdateDevPDL()
{
    return WFS_SUCCESS;
}

HRESULT CXFS_SIG::GetStatus(LPWFSCAMSTATUS &lpstStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //状态
    lpstStatus = &m_stStatus;

    return WFS_SUCCESS;
}

HRESULT CXFS_SIG::GetCapabilities(LPWFSCAMCAPS &lpstCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 能力值
    m_stCaps.wClass                         = WFS_SERVICE_CLASS_CAM;
    m_stCaps.fwType                         = WFS_CAM_TYPE_CAM;
    m_stCaps.fwCameras[WFS_CAM_ROOM]        = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_PERSON]      = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_EXITSLOT]    = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[WFS_CAM_EXTRA]       = WFS_CAM_AVAILABLE;
    m_stCaps.fwCameras[4]                   = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[5]                   = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[6]                   = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.fwCameras[7]                   = WFS_CAM_NOT_AVAILABLE;
    m_stCaps.usMaxPictures                  = 1000;
    m_stCaps.fwCamData                      = WFS_CAM_AUTOADD;
    m_stCaps.usMaxDataLength                = 67;
    m_stCaps.fwCharSupport                  = WFS_CAM_ASCII;
    m_stCaps.lpszExtra                      = (LPSTR)m_cExtra.GetExtra();
    m_stCaps.bPictureFile                   = TRUE;
    m_stCaps.bAntiFraudModule               = FALSE;

    lpstCaps = &m_stCaps;

    return WFS_SUCCESS;
}

HRESULT CXFS_SIG::Display(const WFSCAMDISP &stDisply, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;  
    LPWFSCAMDISP lpDisplay = NULL;
    lpDisplay = (LPWFSCAMDISP)&stDisply;
    WORD wAction = lpDisplay->wAction;

    if(wAction != WFS_CAM_CREATE
            && wAction != WFS_CAM_DESTROY
            && wAction != WFS_CAM_PAUSE
            && wAction != WFS_CAM_RESUME
            && wAction != WFS_CAM_ERASE)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败:入参wAction[%d]无效．ReturnCode:%d",
            lpDisplay->wAction, hRet);
        return WFS_ERR_INVALID_DATA;
    }

    if(lpDisplay->wHeight == 0 || lpDisplay->wWidth == 0)
    {
        Log(ThisModule, __LINE__, "创建摄像窗口(Display)失败:入参wHeight[%d]/wWidth[%d]/hWnd[%d]无效．ReturnCode:%d",
            lpDisplay->wHeight, lpDisplay->wWidth, lpDisplay->hWnd, hRet);
        return WFS_ERR_INVALID_DATA;
    }

    // 创建窗口
    if (wAction == WFS_CAM_CREATE)
    {
        m_stCamInfo.init();
        m_stCamInfo.hWnd = lpDisplay->hWnd;
        m_stCamInfo.wX = lpDisplay->wX;
        m_stCamInfo.wY = lpDisplay->wY;
        m_stCamInfo.wWidth = lpDisplay->wWidth;
        m_stCamInfo.wHeight = lpDisplay->wHeight;
        /*m_stCamInfo.wHpixel = lpDisplay->wHpixel;
        m_stCamInfo.wVpixel = lpDisplay->wVpixel;
        m_stCamInfo.pszTexData = lpDisplay->pszTexData;
        长沙高存Display没有这三个参数,长沙XFS规范电子签名用DisplayEx。
        长沙高存SP不包含电子签名模块,为保证编译正常,注销该项。
        若后续添加签名模块,需要在XFSCAM.H中增加DisplayEx支持并修改相关联的处理分支
        */
    }
    // 清除签名
    else if (wAction == WFS_CAM_ERASE)
    {
        hRet = m_pDev->ClearSignature();
        if (hRet != 0)
        {
            Log(ThisModule, __LINE__, "清除签名失败．ReturnCode:%d", hRet);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    //TEXT=请在下方空白处签名!;Background=/usr/local/CFES/DATA/FORM/SIG/SIGN.png
    //hRet = m_pDev->Display(wAction, m_stCamInfo.wX, m_stCamInfo.wY, m_stCamInfo.wWidth, m_stCamInfo.wHeight, m_stCamInfo.wHpixel, m_stCamInfo.wVpixel, lpDisplay->pszTexData, m_sSigIniConfig.wAlwaysShow, dwTimeout);
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "创建签字窗口(Display)失败．ReturnCode:%d", hRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (wAction == WFS_CAM_DESTROY)
        m_stCamInfo.init();

    m_stDisplay = stDisply;
    return WFS_SUCCESS;
}

HRESULT CXFS_SIG::GetSignature(const WFSCAMGETSIGNATURE &stSignature, WFSCAMSIGNDATA &stSignData, DWORD dwTimeout)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    QString qImgData;
    HRESULT hRet = 0;
    CHAR pszPicturePath[MAX_PATH] = { 0x00 };

    stSignData.status = WFS_CAM_DATASRCNOTSUPP;
    stSignData.data_length = 0;

    if(m_pszData != nullptr)
    {
        delete[] m_pszData;
        m_pszData = nullptr;
    }
    m_pszData = new BYTE[SIGNATURE_DATA_SIZE];
    ZeroMemory(m_pszData, SIGNATURE_DATA_SIZE);

    if (stSignature.picture_file == NULL)
    {
        memcpy(pszPicturePath, m_sSigIniConfig.szImagePath, sizeof(m_sSigIniConfig.szImagePath));
    } else
    {
        memcpy(pszPicturePath, stSignature.picture_file, MAX_PATH);
        QString path = pszPicturePath;
        hRet = nWriteData(m_sSigIniConfig.pszSignImagePath, path.append("\n").toStdString().c_str(), 1);
        if (hRet < 0)
            return hRet;
    }

    if (stSignature.key == NULL || stSignature.enc_key == NULL ||
            strlen(stSignature.key) < 1 || strlen(stSignature.enc_key) < 1)
        hRet = m_pDev->GetSignature(m_sSigIniConfig.pszKey, m_sSigIniConfig.pszEncKey, pszPicturePath, m_sSigIniConfig.pszPsgPath, stSignature.wnd, m_sSigIniConfig.wEncrypt, stSignature.cam_data, stSignature.unicode_cam_data);
    else
        hRet = m_pDev->GetSignature(stSignature.key, stSignature.enc_key, pszPicturePath, m_sSigIniConfig.pszPsgPath, stSignature.wnd, m_sSigIniConfig.wEncrypt, stSignature.cam_data, stSignature.unicode_cam_data);
    if (hRet == ERR_USERNOSIGN)
    {
        Log(ThisModule, __LINE__, "获取签名数据失败．传入的key,enc_key错误或路径错误, ReturnCode:%d", hRet);
        stSignData.status = WFS_CAM_DATASRCMISSING;
        return WFS_ERR_CAM_NOTSIGN_ERROR;
    }
    else if (hRet == ERR_INVALID_PARA)
    {
        Log(ThisModule, __LINE__, "获取签名数据失败．传入的key[%s]错误,enc_key[%s]错误或路径[%s]错误, ReturnCode:%d",
            stSignature.key, stSignature.enc_key, pszPicturePath, hRet);
        stSignData.status = WFS_CAM_DATASRCMISSING;
        return WFS_ERR_HARDWARE_ERROR;
    } else if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "获取签名数据失败, ReturnCode:%d", hRet);
        stSignData.status = WFS_CAM_DATASRCMISSING;
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 获取签名轨迹数据返回给结构体
    if (!bReadImageData(m_sSigIniConfig.pszPsgPath, m_pszData))
    {
        stSignData.status = WFS_CAM_DATASRCMISSING;
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (m_pszData == nullptr || m_iLength == 0)
    {
        stSignData.status = WFS_CAM_DATAMISSING;
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 校验数据
    qImgData = QString(QLatin1String((LPSTR)m_pszData));
    if (m_stDisplay.wWidth != qImgData.left(qImgData.indexOf(",")).toInt() ||
            m_stDisplay.wHeight != qImgData.mid(qImgData.indexOf(",") + 1, 3).toInt())
    {
        Log(ThisModule, __LINE__, "签名数据校验失败，读取图片数据内容窗口宽为:%d，窗口高为:%d.",
            qImgData.left(qImgData.indexOf(",")).toInt(),
            qImgData.mid(qImgData.indexOf(",") + 1, 3).toInt());
        stSignData.status = WFS_CAM_DATASRCMISSING;

        return WFS_ERR_HARDWARE_ERROR;
    }

    stSignData.status = WFS_CAM_DATAOK;
    stSignData.data = m_pszData;
    stSignData.data_length = m_iLength;

    return WFS_SUCCESS;
}

HRESULT CXFS_SIG::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    HRESULT hRet = 0;

    hRet = m_pDev->Reset();
    if (hRet != 0)
    {
        Log(ThisModule, __LINE__, "设备复位失败．ReturnCode:%d", hRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}



void CXFS_SIG::InitConifig()
{
    //WORD wTmp = 0;

    ZeroMemory(&m_sSigIniConfig, sizeof(SIGCONFIGINFO));

    strcpy(m_sSigIniConfig.szDevDllName, m_cXfsReg.GetValue("default", "DriverDllName", ""));

    // -----[DEVICE_CONFIG]下参数------------
    // 设备类型(0/特思达TSD64,1/辰展TPK193)
    m_sSigIniConfig.wDeviceType = (WORD)m_cXfsReg.GetValue("DEVICE_CONFIG", "DeviceType", (DWORD)0);

    // ;设置加密算法, 0:明文 1:DES 2:3DES 3:AES 4:SM4
    m_sSigIniConfig.wEncrypt = m_cXfsReg.GetValue("Encry", "Algorithm", ENCRYPT_NONE);

    // 获取主密钥 子密钥
    strcpy(m_sSigIniConfig.pszKey, m_cXfsReg.GetValue("Encry", "Key", ""));
    strcpy(m_sSigIniConfig.pszEncKey, m_cXfsReg.GetValue("Encry", "EncKey", ""));


    // 签名数据路径
    strcpy(m_sSigIniConfig.pszPsgPath, m_cXfsReg.GetValue("Data", "PsgPath", PSGINFOFILE));
    if (strlen((char*)m_sSigIniConfig.pszPsgPath) < 2 ||
        m_sSigIniConfig.pszPsgPath[0] != '/') {
        ZeroMemory(m_sSigIniConfig.pszPsgPath,
               sizeof(m_sSigIniConfig.pszPsgPath));
        memcpy(m_sSigIniConfig.pszPsgPath, PSGINFOFILE, strlen(PSGINFOFILE));
    }

    // 图片保存路径
    strcpy(m_sSigIniConfig.szImagePath, m_cXfsReg.GetValue("Data", "ImagePath", IMAGEINFOFILE));
    if (strlen((char*)m_sSigIniConfig.szImagePath) < 2 ||
        m_sSigIniConfig.szImagePath[0] != '/') {
        ZeroMemory(m_sSigIniConfig.szImagePath,
               sizeof(m_sSigIniConfig.szImagePath));
        memcpy(m_sSigIniConfig.szImagePath, IMAGEINFOFILE, strlen(IMAGEINFOFILE));
    }

    // 设置签字笔迹宽度，默认3
    m_sSigIniConfig.wPenWidth = m_cXfsReg.GetValue("Property", "PenWidth", (DWORD)0);

    // 设置笔迹放大倍数, 默认不放大(0)
    m_sSigIniConfig.wPenMultiple = m_cXfsReg.GetValue("Property", "PenMultiple", (DWORD)3);

    // 设置显示，默认为0签字时隐藏
    m_sSigIniConfig.wAlwaysShow = m_cXfsReg.GetValue("Property", "AlwaysShow", (DWORD)0);

    // 设置背景颜色
    m_sSigIniConfig.wBackColor = bSet16BitColor(m_cXfsReg.GetValue("Property", "BackColor", (DWORD)1));

    // 是否使用背景颜色, 0透明颜色, 1使用背景颜色
    m_sSigIniConfig.wIsUseBackColor = m_cXfsReg.GetValue("Property", "IsUseBackColor", (DWORD)0);

    // 设置笔迹颜色, 0：黑色 1：蓝色 2：红色
    m_sSigIniConfig.wPenColor = m_cXfsReg.GetValue("Property", "PenColor", (DWORD)0);

    // 设置文本颜色
    m_sSigIniConfig.wTextColor = bSet16BitColor(m_cXfsReg.GetValue("Property", "TextColor", (DWORD)0));

    // 设置背景透明度(0~255), 0完全透明, 255不透明
    m_sSigIniConfig.wTransparency = m_cXfsReg.GetValue("Property", "Transparency", (DWORD)255);

    // SignaturePic备份保存天数
    m_sSigIniConfig.wSaveTime = m_cXfsReg.GetValue("Clear", "SavaTime", int(30));

    // 存放民生下发过的签名图像的路径
    strcpy(m_sSigIniConfig.pszSignImagePath, m_cXfsReg.GetValue("Clear", "SignImagePath", SAVEIMAGEFILE));
    if (strlen((char*)m_sSigIniConfig.pszSignImagePath) < 2 ||
        m_sSigIniConfig.pszSignImagePath[0] != '/') {
        ZeroMemory(m_sSigIniConfig.pszSignImagePath,
               sizeof(m_sSigIniConfig.pszSignImagePath));
        memcpy(m_sSigIniConfig.pszSignImagePath, SAVEIMAGEFILE, strlen(SAVEIMAGEFILE));
    }

    // 设置启用接口日志, 0不启用, 1启用
    m_sSigIniConfig.wAPILog = m_cXfsReg.GetValue("Property", "APILog", int(0));

    // 日志存放路径
    strcpy(m_sSigIniConfig.pszLogFile, m_cXfsReg.GetValue("Property", "LogFile", APILOGFILE));
    if (strlen((char*)m_sSigIniConfig.pszLogFile) < 2 ||
        m_sSigIniConfig.pszLogFile[0] != '/') {
        ZeroMemory(m_sSigIniConfig.pszLogFile,
               sizeof(m_sSigIniConfig.pszLogFile));
        memcpy(m_sSigIniConfig.pszLogFile, APILOGFILE, strlen(APILOGFILE));
    }

    // SignaturePic保存图片过期清理
    vDelImageInfoFile();
}

void CXFS_SIG::InitStatus()
{
    ZeroMemory(&m_stStatus, sizeof(WFSCAMSTATUS));
    m_stStatus.fwDevice      = WFS_CAM_DEVNODEVICE;
    for (int i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        m_stStatus.fwMedia[i] = WFS_CAM_MEDIAUNKNOWN;
    for (int i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        m_stStatus.fwCameras[i] = WFS_CAM_CAMNOTSUPP;
    for (int i = 0; i < WFS_CAM_CAMERAS_SIZE; i ++)
        m_stStatus.usPictures[i] = WFS_CAM_CAMNOTSUPP;

    m_stStatus.lpszExtra = NULL;
    m_stStatus.wAntiFraudModule = 0;
}

bool CXFS_SIG::UpdateStatus(const DEVCAMSTATUS &stStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    AutoMutex(*m_pMutexGetStatus);

    WORD fwDevice = WFS_CAM_DEVONLINE;

    switch (stStatus.fwDevice[WFS_CAM_EXTRA])
    {
    case DEVICE_OFFLINE:
        fwDevice = WFS_CAM_DEVOFFLINE;
        break;
    case DEVICE_ONLINE:
        fwDevice = WFS_CAM_DEVONLINE;
        break;
    default:
        fwDevice = WFS_CAM_DEVHWERROR;
        break;
    }

    // 判断状态是否有变化
    if (m_stStatus.fwDevice != fwDevice)
    {
        m_pBase->FireStatusChanged(fwDevice);
        // 故障时，也要上报故障事件
        if (fwDevice == WFS_CAM_DEVHWERROR)
            m_pBase->FireHWErrorStatus(WFS_ERR_ACT_RESET, m_cExtra.GetErrDetail("ErrorDetail"));
    }
    m_stStatus.fwDevice = fwDevice;
    m_stStatus.fwMedia[WFS_CAM_EXTRA] =
            (stStatus.fwMedia[WFS_CAM_EXTRA] == MEDIA_OK ? WFS_CAM_MEDIAOK : WFS_CAM_MEDIAUNKNOWN);
    m_stStatus.fwCameras[WFS_CAM_EXTRA] =
            (stStatus.fwCameras[WFS_CAM_EXTRA] == STATUS_OK ? WFS_CAM_AVAILABLE : WFS_CAM_NOT_AVAILABLE);

    // 扩展状态
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();

    return true;
}

COLORREF CXFS_SIG::cSetColor(WORD r/* = 0*/, WORD g/* = 0*/, WORD b/* = 0*/)
{
    COLORREF col = RGB(r, g, b);
    return col;
}

COLORREF CXFS_SIG::bSet16BitColor(DWORD wIndex)
{
    COLORREF col;

    switch (wIndex) {
    case COLOR_BLACK:
        col = cSetColor();
        break;
    case COLOR_WHITE:
        col = cSetColor(255, 255, 255);
        break;
    case COLOR_DARKGREY:
        col = cSetColor(169, 169, 169);
        break;
    case COLOR_GREY:
        col = cSetColor(190, 190, 190);
        break;
    case COLOR_LIGHTGREY:
        col = cSetColor(211, 211, 211);
        break;
    case COLOR_RED:
        col = cSetColor(255, 0, 0);
        break;
    case COLOR_GREEN:
        col = cSetColor(0, 255, 0);
        break;
    case COLOR_BLUE:
        col = cSetColor(0, 0, 255);
        break;
    case COLOR_CYAN:
        col = cSetColor(0, 255, 255);
        break;
    case COLOR_YELLOW:
        col = cSetColor(255, 255, 0);
        break;
    default:
        col = cSetColor();
        break;
    }

    return col;
}

void CXFS_SIG::vSlotShowWin(SIGHOWWININFO stSigShowInfo)
{
    if (showWin != nullptr)
    {
        showWin->hide();
        delete showWin;
        showWin = nullptr;
    }

    QWidget *myWidget = nullptr;
    myWidget = QWidget::createWindowContainer(QWindow::fromWinId((WId)stSigShowInfo.hWnd));

    showWin = new MainWindow(myWidget);

//    if ((nRet = showWin->nConnSharedMem(m_sCamIniConfig.szCamDataSMemName)) > 0)
//    {
//        showWin->Release();
//        return;
//    }

    showWin->ShowWin(stSigShowInfo.wX, stSigShowInfo.wY,
                     stSigShowInfo.wWidth, stSigShowInfo.wHeight);

    //showWin->show();
}

void CXFS_SIG::vSlotHideWin()
{
    if (showWin != nullptr)
    {
        showWin->hide();
        delete showWin;
        showWin = nullptr;
    }
}

/*****************************************************************************
** 清除指定日期以前的Signature Image 备份保存文件
*/
void CXFS_SIG::vDelImageInfoFile()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    QString strall;
    int nLine = 0;
    const DWORD nDaySec = 24*60*60;

    nLine = nGetLine(m_sSigIniConfig.pszSignImagePath, strall);
    if (nLine > 0 && !strall.isEmpty())
    {
        QDateTime curTime = QDateTime::currentDateTime();
        DWORD cur = curTime.toTime_t();
        DWORD create = 0;
        QString image;

        for (int i = 0; i < nLine; i++)
        {
            if (i > 30)
                break;
            image = qGetLineData(i, nLine, strall);
            if (!image.isEmpty())
            {
                vGetImageCreateTime(image, &create);
                // 时间差 > 30 天(可配值项)
                if (((cur - create) / nDaySec) > m_sSigIniConfig.wSaveTime)
                {
                    if (!QFile::remove(image) ||
                        nWriteData(m_sSigIniConfig.pszSignImagePath, strall.toStdString().c_str(), 2) < 0)
                    {
                        Log(ThisModule, __LINE__, "清理过期图片失败,清理图片路径:[%s],配置过期时间:[%d天],存放图片数组文件:[%s]",
                            image.toStdString().c_str(), m_sSigIniConfig.wSaveTime, m_sSigIniConfig.pszSignImagePath);
                        break;
                    }
                }
            } else
                break;
        }
    } else
        Log(ThisModule, __LINE__, "存放图片数组文件不存在");
}

bool CXFS_SIG::bReadImageData(LPSTR lpImgPath, LPBYTE lpImgData)
{
    FILE* fp;

    if ((fp = fopen(lpImgPath, "rb")) == NULL)
    {
        return false;
    }

    fseek(fp, 0, SEEK_END);
    m_iLength = ftell(fp);
    rewind(fp);
    fread((LPSTR)lpImgData, m_iLength, 1, fp);

    fclose(fp);

    return true;
}

int CXFS_SIG::nWriteData(QString filename, LPCSTR pData, int mode)
{
    int nRet = WFS_ERR_HARDWARE_ERROR;
    QFile file(filename);

    if (mode == 1)
    {
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
        {
            return nRet;
        }
    } else if (mode == 2)
    {
        if (!file.open(QIODevice::WriteOnly|QIODevice::Text))
        {
            return nRet;
        }
    } else{
        return nRet;
    }

    if (pData == NULL)
    {
        return nRet;
    }

    QTextStream stream(&file);
    stream << pData;
    file.close();
    nRet = CAM_SUCCESS;

    return nRet;
}

int CXFS_SIG::nGetLine(QString filname, QString& strall)
{
    int Index = 0;
    int nLine = 0;

    QFile file(filname);
    if (file.exists())
    {
        strall = file.readAll();
        while (Index != -1)
        {
            Index = strall.indexOf('\n', Index + 1);
            nLine++;
        }
    }

    return nLine;
}

QString CXFS_SIG::qGetLineData(int nNum, int nAllLine, QString &strall)
{
    QString image;

    int nLine = nAllLine;
    if (nNum == 0)
    {
        int nIndex = strall.indexOf('\n');
        image = strall.left(nIndex);
        strall.remove(0, nIndex+1);
    } else
    {
        int nTemp = nNum;
        int nIndex = 0, nIndex2 = 0;

        while (nTemp--)
        {
            //
            nIndex = strall.indexOf('\n', nIndex+1);
            if (nIndex != -1)
            {
                nIndex2 = strall.indexOf('\n', nIndex+1);
            }
        }

        if(nNum < nLine - 1)
        {
            image = strall.mid(nIndex+1, nIndex2 - nIndex);
            strall.remove(nIndex+1, nIndex2 - nIndex);
        }
        else if(nNum == nLine - 1)
        {
            int len = strall.length();
            image = strall.mid(nIndex, len - nIndex);
            strall.remove(nIndex, len-nIndex);
        }
    }


    if (image.isEmpty())
    {
        return "";
    } else {
        return image;
    }
}

void CXFS_SIG::vGetImageCreateTime(QString filename, DWORD *time)
{
    QFileInfo info(filename);
    if (info.exists())
    {
        *time = info.birthTime().toTime_t();
    }
}

void CXFS_SIG::FireNoSignatureData()
{
    m_pBase->FireEvent(WFS_USER_EVENT, WFS_USRE_CAM_MEDIATHRESHOLD, nullptr);
}

void CXFS_SIG::FireInvalidData()
{
    m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_CAM_INVALIDDATA, nullptr);
}
