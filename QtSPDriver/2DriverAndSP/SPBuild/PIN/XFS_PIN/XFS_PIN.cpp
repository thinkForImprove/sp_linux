#include "XFS_PIN.h"

static const char *DEVTYPE = "PIN";
static const char *ThisFile = "XFS_PIN.cpp";
//////////////////////////////////////////////////////////////////////////
#define EPP_SM2_PRIVATE_KEY_NAME "EppSKey"
#define EPP_RSA_PRIVATE_KEY_NAME "EppRSASKey"                   //30-00-00-00(FS#0006)
#define SM2_USER_ID_DATA  "31323334353637383132333435363738"
//中电EPP
#define EPP_CFES_SC_PKEY_NO      0x00           //中电签名中心RSA公钥密钥号
#define EPP_CFES_EPP_RSA_KEY_NO  0x02           //中电EPP EPP RSA密钥对
#define EPP_CFES_HOST_PKEY_NO    0x03           //导入银行后台公钥密钥号
// 按键值映射表
typedef struct tag_map_key_val
{
    BYTE byKey;
    ULONG ulVal;
} MAPKEYVAL;
MAPKEYVAL g_stFuncKey[] = {\
    {0x30, WFS_PIN_FK_0}, {0x31, WFS_PIN_FK_1}, {0x32, WFS_PIN_FK_2}, {0x33, WFS_PIN_FK_3}, {0x34, WFS_PIN_FK_4},
    {0x35, WFS_PIN_FK_5}, {0x36, WFS_PIN_FK_6}, {0x37, WFS_PIN_FK_7}, {0x38, WFS_PIN_FK_8}, {0x39, WFS_PIN_FK_9},
    {0x1B, WFS_PIN_FK_CANCEL}, {0x0D, WFS_PIN_FK_ENTER}, {0x08, WFS_PIN_FK_CLEAR}, {0x22, WFS_PIN_FK_00},
//Del for repeated WFS_PIN_FK_0    {0x2A, WFS_PIN_FK_0}, {0x2E, WFS_PIN_FK_DECPOINT}
    {0x2A, WFS_PIN_FK_HELP}, {0x2E, WFS_PIN_FK_DECPOINT}
};
MAPKEYVAL g_stFDKKey[] = {\
    {0x41, WFS_PIN_FK_FDK01}, {0x42, WFS_PIN_FK_FDK02}, {0x43, WFS_PIN_FK_FDK03}, {0x44, WFS_PIN_FK_FDK04},
    {0x45, WFS_PIN_FK_FDK05}, {0x46, WFS_PIN_FK_FDK06}, {0x47, WFS_PIN_FK_FDK07}, {0x48, WFS_PIN_FK_FDK08}
};
//////////////////////////////////////////////////////////////////////////
#define VERIFYRSASTATUS()                                           \
    if (0 != m_pDev->RSAInitStatus())                               \
    {                                                               \
        Log(ThisModule, __LINE__, "密码键盘没有初始化RSA密钥失败"); \
        return WFS_ERR_PIN_ACCESSDENIED;                            \
    }
/////////////////////////////////////////////////////////////////////////////
CXFS_PIN::CXFS_PIN():m_cCancelEvent(FALSE)              //30-00-00-00(FT#0070)
{
    SetLogFile(LOGFILE, ThisFile, DEVTYPE);
    memset(&m_stStatus, 0x00, sizeof(m_stStatus));
    memset(&m_stCaps, 0x00, sizeof(m_stCaps));
}

CXFS_PIN::~CXFS_PIN() {}

long CXFS_PIN::StartRun()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    // 加载BasePIN
    if (0 != m_pBase.Load("SPBasePIN.dll", "CreateISPBasePIN", DEVTYPE))
    {
        Log(ThisModule, __LINE__, "加载SPBasePIN失败");
        return -1;
    }

    // 注册并开始执行SP
    m_pBase->RegisterICmdFunc(this);
    m_pBase->StartRun();
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::OnOpen(LPCSTR lpLogicalName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strLogicalName = lpLogicalName;
    m_strSPName = m_cXfsReg.GetSPName();
    m_strNotCheckFDK = m_cXfsReg.GetValue("CONFIG", "NotCheckFDK", "0");    
    m_strAllKeyVal = m_cXfsReg.GetValue("CONFIG", "KeyValue", "");
    m_strPort = m_cXfsReg.GetValue("CONFIG", "Port", "");
    int iDevType = m_cXfsReg.GetValue("DevType", (DWORD)0);
    switch(iDevType){
    case 0:
        m_strDevName = "XZ";
        break;
    case 1:
        m_strDevName = "ZT";
        break;
	case 2:
        m_strDevName = "CFES";
        break;	
    case 3:
        m_strDevName = "ZTC90";
        break;
    default:
        m_strDevName = "XZ";
        break;
    }
    //获取按键和键值映射表
    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        QByteArray vtKeyMapKeyValueList = m_cXfsReg.GetValue("CONFIG", "KeyMapKeyValueList", "");
        if(vtKeyMapKeyValueList.size() == 48){
            QByteArray vtFuncKeyValueList = vtKeyMapKeyValueList.mid(0, 32);
            QByteArray vtFDKKeyValueList = vtKeyMapKeyValueList.mid(32);
            QByteArray vtFuncKeyValueListBCD;
            QByteArray vtFDKKeyValueListBCD;
            CAutoHex::Hex2Bcd(vtFuncKeyValueList.constData(), vtFuncKeyValueListBCD);
            CAutoHex::Hex2Bcd(vtFDKKeyValueList.constData(), vtFDKKeyValueListBCD);

            for(int i = 0; i < vtFuncKeyValueListBCD.size(); i++){
                g_stFuncKey[i].byKey = vtFuncKeyValueListBCD[i];
            }
            for(int i = 0; i < vtFDKKeyValueListBCD.size(); i++){
                g_stFDKKey[i].byKey = vtFDKKeyValueListBCD[i];
            }
        } else {
            Log(ThisModule, __LINE__, "键值映射表数据[%s]错误，使用默认值", vtKeyMapKeyValueList.constData());
        }
    }


    std::string strDevDll = m_cXfsReg.GetValue("DriverDllName", "");
    if (strDevDll.empty() || m_strPort.empty())
    {
        Log(ThisModule, __LINE__, "SP=%s的DriverDllName或Port配置项为空或读取失败", m_strSPName.c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    //读取ini配置项
    ReadConfig();               //30-00-00-00(FS#0010)

    // 加载DEV
//    HRESULT hRet = m_pDev.Load(strDevDll.c_str(), "CreateIDevPIN", DEVTYPE);    
    HRESULT hRet = m_pDev.Load(strDevDll.c_str(), "CreateIDevPIN", m_strDevName.c_str());
    if (0 != hRet)
    {
        Log(ThisModule, __LINE__, "加载%s失败, hRet=%d[%s]",
            strDevDll.c_str(), hRet, m_pDev.LastError().toStdString().c_str());
        return WFS_ERR_INTERNAL_ERROR;
    }

    if(m_strDevName == "ZTC90"){
        m_pDev->SetDesKeyLenInfo(m_cKeyC90.GetDesKeyLenInfo());
    }

    // 打开连接
    hRet = m_pDev->Open(m_strPort.c_str());
    if (hRet != 0)
    {
        //更新状态
        OnStatus();
        Log(ThisModule, __LINE__, "打开设备连接失败！, Port=%s", m_strPort.c_str());
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 复位设备
    hRet = m_pDev->Reset();
    if (hRet != 0)
    {
        // 复位失败，不用返回故障，只要更新状态为故障就行了
        Log(ThisModule, __LINE__, "设备复位失败！");
    }
    else
    {
        // 复位成功，设置按键值
        if(m_strDevName == "XZ"){
            m_pDev->SetKeyValue(m_strAllKeyVal.c_str());
        }
    }

    // 获取SPBase的互斥量，此主要用来互斥更新状态
    SPBASEDATA stData;
    m_pBase->GetSPBaseData(stData);
    m_pMutexGetStatus = stData.pMutex;

    // 更新扩展状态
    char szDevVer[256] = {0};
    m_pDev->GetFirmware(szDevVer);
    UpdateExtra("000", szDevVer);

    // 更新一次状态
    OnStatus();

    // 读取配置
    DWORD dwDefValue = 0;
    m_bSupportSM = (BOOL)m_cXfsReg.GetValue("CONFIG", "SupportSM", dwDefValue);
    //读取平安银行配置文件
    CINIFileReader configfile(WSAP_PINGANBANK_CFG);
    CINIReader     crINI = configfile.GetReaderSection("CFG");
    CINIWriter     cwINI = configfile.GetWriterSection("CFG");
    m_bIsSupportSM4 = (BOOL)crINI.GetValue("IsSupportSM4", dwDefValue);
    if (m_bSupportSM == 1 && m_bIsSupportSM4 == 0)
    {
        m_bIsSupportSM4 = 1;
        cwINI.SetValue("IsSupportSM4", m_bIsSupportSM4);
    }

    m_bOnlyUseSM = (BOOL)m_cXfsReg.GetValue("CONFIG", "OnlyUseSM", dwDefValue);
    m_bRemovCustomerLast = (BOOL)m_cXfsReg.GetValue("CONFIG", "RemovCustomerLast", dwDefValue);
    m_dwDesMacType = m_cXfsReg.GetValue("CONFIG", "DesMacType", dwDefValue);
    m_dwSM4MacType = m_cXfsReg.GetValue("CONFIG", "SM4MacType", dwDefValue);
    m_bGetPinEntChkMinLen = m_cXfsReg.GetValue("CONFIG", "GetPinEnterCheckMinLength", dwDefValue) == 1;
    m_wSM4CryptAlgorithm = m_cXfsReg.GetValue("CONFIG", "SM4CryptAlgorithm", dwDefValue);
    if(m_wSM4CryptAlgorithm > 1){
        m_wSM4CryptAlgorithm = 0;
    }
    Log(ThisModule, 1, "打开设备连接成功, Extra=%s", m_cExtra.GetExtraInfo().c_str());
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::OnClose()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (m_pDev != nullptr)
        m_pDev->Close();
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::OnStatus()
{
    // 空闲更新状态
    DEVPINSTATUS stStatus;
    if (m_pDev != nullptr)
        m_pDev->GetStatus(stStatus);
    UpdateStatus(stStatus);
    if(stStatus.wDevice == DEVICE_OFFLINE){
        m_pDev->Close();
        m_pDev->Open(m_strPort.c_str());
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::OnCancelAsyncRequest()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_cCancelEvent.SetEvent();
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::OnUpdateDevPDL()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GetStatus(LPWFSPINSTATUS &lpstStatus)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    //状态
    m_stStatus.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpstStatus = &m_stStatus;
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GetCapabilities(LPWFSPINCAPS &lpstCaps)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 能力
    m_stCaps.wClass = WFS_SERVICE_CLASS_PIN;
    m_stCaps.fwType = WFS_PIN_TYPEEPP | WFS_PIN_TYPEEDM | WFS_PIN_TYPEHSM;
    m_stCaps.bCompound = FALSE;
    m_stCaps.usKeyNum = (m_strDevName == "XZ" || m_strDevName == "CFES") ? KEY_MAX_NUM_XZ : KEY_MAX_NUM;

    m_stCaps.fwAlgorithms = WFS_PIN_CRYPTDESECB | WFS_PIN_CRYPTDESCBC | WFS_PIN_CRYPTDESMAC | WFS_PIN_CRYPTTRIDESECB | \
                            WFS_PIN_CRYPTTRIDESCBC | WFS_PIN_CRYPTTRIDESMAC;
    if (IsSupportSM())
    {
        m_stCaps.fwAlgorithms += (WFS_PIN_CRYPTSM4 | WFS_PIN_CRYPTSM4MAC);
    }
    m_stCaps.fwPinFormats = WFS_PIN_FORM3624 | WFS_PIN_FORMANSI | WFS_PIN_FORMISO0 | WFS_PIN_FORMISO1 | WFS_PIN_FORMISO3 | \
                            WFS_PIN_FORMVISA | WFS_PIN_FORMBANKSYS;
    m_stCaps.fwDerivationAlgorithms = WFS_PIN_CHIP_ZKA;
    m_stCaps.fwPresentationAlgorithms = WFS_PIN_PRESENT_CLEAR;
    m_stCaps.fwDisplay = WFS_PIN_DISPNONE;
    m_stCaps.bIDConnect = FALSE;
    m_stCaps.fwIDKey = WFS_PIN_IDKEYIMPORT | WFS_PIN_IDKEYINITIALIZATION;
    m_stCaps.fwValidationAlgorithms = WFS_PIN_DES | WFS_PIN_VISA;
    m_stCaps.fwKeyCheckModes = (m_strDevName == "XZ" || m_strDevName == "CFES") ? (WFS_PIN_KCVSELF | WFS_PIN_KCVZERO) : WFS_PIN_KCVZERO;
    m_stCaps.lpszExtra = (LPSTR)m_cExtra.GetExtra();
    lpstCaps = &m_stCaps;
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GetKeyDetail(LPSTR lpszKeyName, LPWFSPINKEYDETAIL *&lppKeyDetail)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lppKeyDetail = nullptr;
    m_cKeyDetail.Clear();
    if (lpszKeyName != nullptr)
    {
        PINKEYDATA stKey;
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            stKey = m_cKeyXZ.FindKey(lpszKeyName);
            if(stKey.uNum == KEY_NOT_NUM){
                if(IsSupportSM()){
                    stKey = m_cKeyXZ.FindSMKey(lpszKeyName);
                }
            }
        } else if(m_strDevName == "ZT") {
            stKey = m_cKey.FindKey(lpszKeyName);
            if(stKey.uNum == KEY_NOT_NUM){
                if(IsSupportSM()){
                    stKey = m_cKey.FindKey(lpszKeyName, false);
                }
            }
        } else {
            stKey = m_cKeyC90.FindKey(lpszKeyName);
            if(stKey.uNum == KEY_NOT_NUM){
                if(IsSupportSM()){
                    stKey = m_cKeyC90.FindKey(lpszKeyName, false);
                }
            }
        }

        if (stKey.uNum == KEY_NOT_NUM)
            return WFS_ERR_PIN_KEYNOTFOUND;

        m_cKeyDetail.Add(stKey.szName, stKey.dwUse);
    }
    else
    {
        listPINKEYDATA ltKeyData;
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            listPINKEYDATA ltKeyDataTmp;
            m_cKeyXZ.GetAllDesKey(ltKeyData);
            m_cKeyXZ.GetAllSMKey(ltKeyDataTmp);
            for(auto it : ltKeyDataTmp){
                ltKeyData.push_back(it);
            }
            m_cKeyXZ.GetAllAsymmKey(ltKeyDataTmp);
            for(auto it : ltKeyDataTmp){
                ltKeyData.push_back(it);
            }
            if(ltKeyData.size() == 0){
                return WFS_SUCCESS;
            }
        } else if(m_strDevName == "ZT") {
            if (m_cKey.IsEmpty())
                return WFS_ERR_PIN_KEYNOTFOUND;
            m_cKey.GetAllKey(ltKeyData);
        } else {
            if (m_cKeyC90.IsEmpty())
                return WFS_ERR_PIN_KEYNOTFOUND;
            m_cKeyC90.GetAllKey(ltKeyData);
        }

        for (auto &it : ltKeyData)
        {
            m_cKeyDetail.Add(it.szName, it.dwUse);
        }
    }

    lppKeyDetail = m_cKeyDetail.GetData();
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GetKeyDetailEx(LPSTR lpszKeyName, LPWFSPINKEYDETAILEX *&lppKeyDetail)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lppKeyDetail = nullptr;
    m_cKeyDetailEx.Clear();
    if (lpszKeyName != nullptr)
    {
        PINKEYDATA stKey;
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            stKey = m_cKeyXZ.FindSMKey(lpszKeyName);
            if(stKey.uNum == KEY_NOT_NUM){                      //30-00-00-00(FT#0012)
                stKey = m_cKeyXZ.FindAsymmKey(lpszKeyName);     //30-00-00-00(FT#0012)
            }                                                   //30-00-00-00(FT#0012)
        } else if(m_strDevName == "ZT") {
            stKey = m_cKey.FindKey(lpszKeyName, false);
        } else {
            stKey = m_cKeyC90.FindKey(lpszKeyName, false);
        }
        if (stKey.uNum == KEY_NOT_NUM)
            return WFS_ERR_PIN_KEYNOTFOUND;

        m_cKeyDetailEx.Add(stKey.szName, stKey.dwUse, stKey.byVersion);
    }
    else
    {
        listPINKEYDATA ltKeyData;
        listPINKEYDATA ltAsymmKeyData;
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            m_cKeyXZ.GetAllSMKey(ltKeyData);
//30-00-00-00(FT#0012)            if(ltKeyData.size() == 0){
//30-00-00-00(FT#0012)                return WFS_SUCCESS;
//30-00-00-00(FT#0012)            }
            m_cKeyXZ.GetAllAsymmKey(ltAsymmKeyData);        //30-00-00-00(FT#0012)
            if(!ltAsymmKeyData.empty()){                    //30-00-00-00(FT#0012)
                for(auto it : ltAsymmKeyData){              //30-00-00-00(FT#0012)
                    ltKeyData.push_back(it);                //30-00-00-00(FT#0012)
                }                                           //30-00-00-00(FT#0012)
            }                                               //30-00-00-00(FT#0012)
        } else if(m_strDevName == "ZT") {
            if (m_cKey.IsEmpty())
                return WFS_ERR_PIN_KEYNOTFOUND;

            m_cKey.GetAllKey(ltKeyData);
        } else {
            if (m_cKeyC90.IsEmpty())
                return WFS_ERR_PIN_KEYNOTFOUND;

            m_cKeyC90.GetAllKey(ltKeyData);
        }

        for (auto &it : ltKeyData)
        {
            m_cKeyDetailEx.Add(it.szName, it.dwUse, it.byVersion);
        }
    }

    lppKeyDetail = m_cKeyDetailEx.GetData();
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GetFuncKeyDetail(LPULONG lpFDKMask, LPWFSPINFUNCKEYDETAIL &lpFunKeyDetail)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    m_cFuncKeyDetail.Clear();
    m_cFuncKeyDetail.SetMask(GetFunctionKeys());

    ULONG ulFDK = 0xFFFFFFFF;
    WFSPINFDK szFDKs[FDK_MAX_NUM];
    GetFDKs(szFDKs);
    if (lpFDKMask != nullptr)
        ulFDK = *lpFDKMask;
    for (ULONG i = 0; i < FDK_MAX_NUM; i++)
    {
        if ((ulFDK & szFDKs[i].ulFDK) != 0)
        {
            m_cFuncKeyDetail.AddFDK(szFDKs[i]);
        }
    }

    lpFunKeyDetail = m_cFuncKeyDetail.GetData();
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::Crypt(const LPWFSPINCRYPT lpCrypt, LPWFSXDATA &lpxCryptData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = 0;
    WORD wMode = lpCrypt->wMode;
    CAutoHex cHex;
    QByteArray vtBCD;

    // 随机数
    if (wMode == WFS_PIN_MODERANDOM)
    {
        char szRand[64] = {0};
        lRet = m_pDev->RandData(szRand);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "生成随机数失败：%d", lRet);
            return WFS_ERR_HARDWARE_ERROR;
        }
        // 返回结果
        cHex.Hex2Bcd(szRand, vtBCD);
        m_cXData.Add((LPBYTE)vtBCD.data(), vtBCD.size());
        lpxCryptData = m_cXData.GetData();
        return WFS_SUCCESS;
    }

    if (lpCrypt->lpsKey == nullptr || strlen(lpCrypt->lpsKey) == 0 || lpCrypt->lpxCryptData == nullptr || lpCrypt->lpxCryptData->lpbData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效指针参数或参数为空");
        return WFS_ERR_INVALID_DATA;
    }
    if (lpCrypt->lpxKeyEncKey != nullptr && lpCrypt->lpxKeyEncKey->usLength > 0)
    {
        Log(ThisModule, __LINE__, "不支持二次加解密");
        return WFS_ERR_UNSUPP_DATA;
    }

    BYTE bPadding = lpCrypt->bPadding;    
    WORD wAlgorithm = lpCrypt->wAlgorithm;
    QByteArray strIV;
    QByteArray strWKey(lpCrypt->lpsKey);
    PINKEYDATA stKey;
    QByteArray vtResult;

    UINT wCryptType = ConvertCryptType(wMode, wAlgorithm);
    if (wCryptType == 0)
    {
        Log(ThisModule, __LINE__, "无效加解密类型：wMode=%d,wAlgorithm=%d", wMode, wAlgorithm);
        return WFS_ERR_INVALID_DATA;
    }

    if (!IsSupportSM() && ((wAlgorithm & WFS_PIN_CRYPTSM4) || (wAlgorithm & WFS_PIN_CRYPTSM4MAC)))
    {
        Log(ThisModule, __LINE__, "不支持国密加解密类型：wMode=%d,wAlgorithm=%d", wMode, wAlgorithm);
        return WFS_ERR_UNSUPP_DATA;
    }

    if (lpCrypt->lpxStartValue != nullptr && lpCrypt->lpxStartValue->lpbData != nullptr
        && lpCrypt->lpxStartValue->usLength > 0)
    {       
        if((lpCrypt->lpsStartValueKey != nullptr && strlen(lpCrypt->lpsStartValueKey) > 0)){
            Log(ThisModule, __LINE__, "不支持初始向量数据解密");
            return WFS_ERR_UNSUPP_DATA;
        }

        bool bIVDataLenErr = false;
        if(wAlgorithm & (WFS_PIN_CRYPTSM4 | WFS_PIN_CRYPTSM4MAC)){
            bIVDataLenErr = lpCrypt->lpxStartValue->usLength != 16;
        } else {
            bIVDataLenErr = lpCrypt->lpxStartValue->usLength != 8;
        }
        if(bIVDataLenErr){
            Log(ThisModule, __LINE__, "无效IV长度：IVDataLen=%d", lpCrypt->lpxStartValue->usLength);
            return WFS_ERR_INVALID_DATA;
        }
        cHex.Bcd2Hex(lpCrypt->lpxStartValue->lpbData, lpCrypt->lpxStartValue->usLength);
        strIV = cHex.GetHex();
    }
    else
    {
        if(m_strDevName != "XZ"){
            if (((wAlgorithm & WFS_PIN_CRYPTSM4) || (wAlgorithm & WFS_PIN_CRYPTSM4MAC)))
                strIV = "00000000000000000000000000000000";
            else
                strIV = "0000000000000000";
        }
    }

    // 数据
    cHex.Bcd2Hex(lpCrypt->lpxCryptData->lpbData, lpCrypt->lpxCryptData->usLength);
    if (wAlgorithm & WFS_PIN_CRYPTSM4)
    {
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            stKey = m_cKeyXZ.FindSMKey(strWKey);
            if (m_cKeyXZ.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到国密密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }

            if((stKey.dwUse & WFS_PIN_USECRYPT) == 0){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        } else if(m_strDevName == "ZT") {
            strWKey += KEY_SM4_CRYPT;
            stKey = m_cKey.FindKey(strWKey, false);
            if (!m_cKey.IsSMKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到国密密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        } else {
            strWKey += KEY_SM4_CRYPT;
            stKey = m_cKeyC90.FindKey(strWKey, false);
            if (!m_cKeyC90.IsSMKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到国密密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        }


        lRet = m_pDev->SM4CryptData(stKey.uNum, wCryptType, cHex.GetHex(), bPadding, strIV, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "国密加解密失败：%d", lRet);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }
    else if (wAlgorithm & WFS_PIN_CRYPTSM4MAC)
    {
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            stKey = m_cKeyXZ.FindSMKey(strWKey);
            if (m_cKeyXZ.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到国密密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }

            if((stKey.dwUse & WFS_PIN_USEMACING) == 0){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        } else if(m_strDevName == "ZT") {
            strWKey += KEY_SM4_MACING;
            stKey = m_cKey.FindKey(strWKey, false);
            if (!m_cKey.IsSMKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到国密密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        } else {
            strWKey += KEY_SM4_MACING;
            stKey = m_cKeyC90.FindKey(strWKey, false);
            if (!m_cKeyC90.IsSMKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到国密密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        }


        lRet = m_pDev->SM4MACData(stKey.uNum, wCryptType, cHex.GetHex(), strIV, vtResult, bPadding);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "国密计算MAC失败：%d", lRet);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }
    else if ((wAlgorithm & WFS_PIN_CRYPTDESMAC) || (wAlgorithm & WFS_PIN_CRYPTTRIDESMAC))
    {
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            stKey = m_cKeyXZ.FindKey(strWKey);
            if (m_cKeyXZ.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }

            if((stKey.dwUse & WFS_PIN_USEMACING) == 0){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        } else if(m_strDevName == "ZT") {
            strWKey += KEY_USE_MACING;
            stKey = m_cKey.FindKey(strWKey);
            if (!m_cKey.IsWKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        } else {
            strWKey += KEY_USE_MACING;
            stKey = m_cKeyC90.FindKey(strWKey);
            if (!m_cKeyC90.IsWKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        }

        lRet = m_pDev->DataMAC(stKey.uNum, wCryptType, cHex.GetHex(), strIV, vtResult, bPadding);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "计算MAC失败：%d", lRet);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }
    else
    {
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            stKey = m_cKeyXZ.FindKey(strWKey);
            if (m_cKeyXZ.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }

            if((stKey.dwUse & WFS_PIN_USECRYPT) == 0){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        } else if(m_strDevName == "ZT") {
            strWKey += KEY_USE_CRYPT;
            stKey = m_cKey.FindKey(strWKey);
            if (!m_cKey.IsWKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        } else {
            strWKey += KEY_USE_CRYPT;
            stKey = m_cKeyC90.FindKey(strWKey);
            if (!m_cKeyC90.IsWKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        }

        lRet = m_pDev->DataCrypt(stKey.uNum, wCryptType, cHex.GetHex(), bPadding, strIV, vtResult);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "加解密失败：%d", lRet);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    // 返回结果
    cHex.Hex2Bcd(vtResult.constData(), vtBCD);
    m_cXData.Add((LPBYTE)vtBCD.data(), vtBCD.size());
    lpxCryptData = m_cXData.GetData();
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::ImportKey(const LPWFSPINIMPORT lpImport, LPWFSXDATA &lpxKVC)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
//Del    if (lpImport->lpsKey == nullptr || (lpImport->fwUse != 0 && lpImport->lpxValue == nullptr))
    if ((lpImport->lpsKey == nullptr || strlen(lpImport->lpsKey) == 0 ) ||
       (lpImport->fwUse != 0 && lpImport->lpxValue == nullptr))
    {
        Log(ThisModule, __LINE__, "无效指针参数");
        return WFS_ERR_INVALID_DATA;
    }
    // DES的下载密钥是否只使用国密，有些银行要求使用DES的一样，因此通过此配置区分
    if (IsSupportSM() && IsOnlyUseSM())
    {
        Log(ThisModule, 1, "转换到国密密钥下载");
        WFSPINIMPORTKEYEX stKeyEx;
        memset(&stKeyEx, 0x00, sizeof(stKeyEx));
        stKeyEx.lpsKey = lpImport->lpsKey;
        stKeyEx.lpsEncKey = lpImport->lpsEncKey;
        stKeyEx.lpxValue = lpImport->lpxValue;
        stKeyEx.dwUse = lpImport->fwUse;
        HRESULT hRet = ImportKeyEx(&stKeyEx);
        if (hRet == WFS_SUCCESS)
        {
            QByteArray strNewKey(lpImport->lpsKey);
            PINKEYDATA stKey;
            bool bFindKey = false;
            if(m_strDevName == "XZ" || m_strDevName == "CFES"){
                stKey = m_cKeyXZ.FindSMKey(strNewKey);
                bFindKey = !m_cKeyXZ.IsInvalidKey(stKey);
            } else if(m_strDevName == "ZT") {
                stKey = m_cKey.FindKey(strNewKey, false);
                bFindKey = !m_cKey.IsInvalidKey(stKey);
            } else {
                stKey = m_cKeyC90.FindKey(strNewKey, false);
                bFindKey = !m_cKeyC90.IsInvalidKey(stKey);
            }
            if(!bFindKey){
                Log(ThisModule, __LINE__, "没有找到已下载的密钥：%s", strNewKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }

            // 返回结果
            QByteArray vtBCD;
            CAutoHex::Hex2Bcd(stKey.szKCV, vtBCD);
            m_cXData.Add((LPBYTE)vtBCD.data(), vtBCD.size());
            lpxKVC = m_cXData.GetData();
        }
        return hRet;
    }

    WORD wUse = lpImport->fwUse;
    QByteArray strWKey(lpImport->lpsKey);
    QByteArray strMKey;
    PINKEYDATA stKey;
    if (wUse == 0)// 删除指定密钥
    {
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            stKey = m_cKeyXZ.FindKey(strWKey);
            if (m_cKeyXZ.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            m_cKeyXZ.DeleteKey(strWKey);
        } else if(m_strDevName == "ZT") {
            stKey = m_cKey.FindKey(strWKey);
            if (m_cKey.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            m_cKey.DeleteKey(strWKey);
        } else {
            stKey = m_cKeyC90.FindKey(strWKey);
            if (m_cKeyC90.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            m_cKeyC90.DeleteKey(strWKey);
            m_pDev->SetDesKeyLenInfo(m_cKeyC90.GetDesKeyLenInfo());
        }

        return WFS_SUCCESS;
    }

    // 判断用途
    if (!IsSupportKeyUse(wUse))
    {
        Log(ThisModule, __LINE__, "不支持密钥用途:%d", wUse);
        return WFS_ERR_UNSUPP_DATA;
    }

    long lRet = 0;
    char szKCV[64] = { 0 };
    CAutoHex cHex(lpImport->lpxValue->lpbData, lpImport->lpxValue->usLength);

    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        // 判断用途
        if (wUse & (WFS_PIN_USESVENCKEY | WFS_PIN_USECONSTRUCT | WFS_PIN_USESECURECONSTRUCT | WFS_PIN_USESM4))
        {
            Log(ThisModule, __LINE__, "不支持密钥用途:%d", wUse);
            return WFS_ERR_UNSUPP_DATA;
        }
        wUse = ConvertKeyUse(wUse);

        stKey = m_cKeyXZ.FindKey(strWKey);
        if(!m_cKeyXZ.IsInvalidKey(stKey)){
            if(stKey.dwUse & WFS_PIN_USENODUPLICATE){
                Log(ThisModule, __LINE__, "已下载密钥，有唯一属性：%s", strWKey.constData());
                return WFS_ERR_PIN_DUPLICATEKEY;
            }
        }

        PINKEYDATA stMKey;
        stMKey.uNum = KEY_NOT_NUM;
        if(lpImport->lpsEncKey != nullptr && strlen(lpImport->lpsEncKey) > 0){
            strMKey = lpImport->lpsEncKey;
            stMKey = m_cKeyXZ.FindKey(strMKey);
            if(m_cKeyXZ.IsInvalidKey(stMKey)){
                Log(ThisModule, __LINE__, "没有对应主密钥：%s", strMKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            if(!(stMKey.dwUse & WFS_PIN_USEKEYENCKEY)){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        }

        stKey = m_cKeyXZ.FindKeySlot(strWKey);
        if (m_cKeyXZ.IsMaxKey(stKey))
        {
            Log(ThisModule, __LINE__, "密钥已下载满：%s", strWKey.constData());
            return WFS_ERR_PIN_NOKEYRAM;
        }

        lRet = m_pDev->ImportWKey(stKey.uNum, stMKey.uNum, wUse, cHex.GetHex(), szKCV);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "下载密钥失败：%s", strWKey.constData());
            return WFS_ERR_HARDWARE_ERROR;
        }

        // 保存密钥
        if(lpImport->fwUse & WFS_PIN_USENODUPLICATE){
            wUse |= WFS_PIN_USENODUPLICATE;
        }
        stKey.dwUse = wUse;
        strcpy(stKey.szKCV, szKCV);
        m_cKeyXZ.AddKey(stKey);
        Log(ThisModule, 1, "下载密钥成功：%s]", stKey.szName);
    } else if(m_strDevName == "ZT") {
        PINKEYDATA stMKey;
        memset(&stMKey, 0, sizeof(stMKey));
        stMKey.uNum = 0xFF;
        if(lpImport->lpsEncKey && strlen(lpImport->lpsEncKey) > 0){
            strMKey = lpImport->lpsEncKey;
            stMKey = m_cKey.FindKey(strMKey);
            if (m_cKey.IsInvalidKey(stMKey))
            {
                Log(ThisModule, __LINE__, "没有对应主密钥：%s", strMKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            //判断密钥属性
            if(!m_cKey.IsMKey(stMKey)){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        }

        stKey = m_cKey.FindKey(strWKey);
        if (!m_cKey.IsInvalidKey(stKey))
        {
            if (stKey.dwUse & WFS_PIN_USENODUPLICATE)
            {
                Log(ThisModule, __LINE__, "已下载工作密钥，有唯一属性：%s", strWKey.constData());
                return WFS_ERR_PIN_DUPLICATEKEY;
            }
        }

        // 支持多用途处理
        QByteArray strWKName;
        vectorUINT vtUse;
        if (!SplitKeyUse(wUse, vtUse))
        {
            Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", wUse, strWKey.constData());
            return WFS_ERR_INVALID_DATA;
        }

        //所有Key默认带CRYPT属性
        if((wUse & WFS_PIN_USECRYPT) == 0){
            vtUse.insert(vtUse.begin(), (UINT)PKU_CRYPT);
        }

        // 先删除密钥
        m_cKey.DeleteKey(strWKey);
        UINT uKcvDecryptKeyNo = 0xFF;
        for (auto &it : vtUse)
        {
            if (it == PKU_CRYPT)
                strWKName = strWKey + KEY_USE_CRYPT;
            else if (it == PKU_FUNCTION)
                strWKName = strWKey + KEY_USE_FUNCTION;
            else if (it == PKU_MACING)
                strWKName = strWKey + KEY_USE_MACING;
            else if (it == PKU_KEYENCKEY)                       //FT#0001
                strWKName = strWKey;                            //FT#0001
//          else if (it == PKU_SVENCKEY)
//              strWKName = strWKey + KEY_USE_SVENCHEY;
            else
            {
                Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", it, strWKey.constData());
                return WFS_ERR_INVALID_DATA;
            }

            if(it == PKU_KEYENCKEY){
                stKey = m_cKey.FindMKeySlot(strWKName);
            } else {
                stKey = m_cKey.FindWKeySlot(strWKName);
                if(it == PKU_CRYPT){
                    uKcvDecryptKeyNo = stKey.uNum;
                }
            }

            if (m_cKey.IsMaxKey(stKey))
            {
                Log(ThisModule, __LINE__, "工作密钥已下载满：%s", strWKName.constData());
                return WFS_ERR_PIN_NOKEYRAM;
            }

            if(it == vtUse.back()){
                //如果是最后一个key，获取KCV
                m_pDev->SetDesKcvDecryptKey(uKcvDecryptKeyNo);
                lRet = m_pDev->ImportWKey(stKey.uNum, stMKey.uNum, it, cHex.GetHex(), szKCV);
            } else {
                lRet = m_pDev->ImportWKey(stKey.uNum, stMKey.uNum, it, cHex.GetHex(), nullptr);
            }
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "下载工作密钥失败：%s", strWKName.constData());
                return WFS_ERR_HARDWARE_ERROR;
            }

            // 保存密钥
            stKey.dwUse = wUse;
            strcpy(stKey.szKCV, szKCV);
            m_cKey.AddKey(stKey);
            Log(ThisModule, 1, "多用途下载工作密钥成功：%s]", stKey.szName);
        }
    } else {
        PINKEYDATA stMKey;
        memset(&stMKey, 0, sizeof(stMKey));
        stMKey.uNum = 0xFF;
        if(lpImport->lpsEncKey && strlen(lpImport->lpsEncKey) > 0){
            strMKey = lpImport->lpsEncKey;
            stMKey = m_cKeyC90.FindKey(strMKey);
            if (m_cKeyC90.IsInvalidKey(stMKey))
            {
                Log(ThisModule, __LINE__, "没有对应主密钥：%s", strMKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            //判断密钥属性
            if(!m_cKeyC90.IsMKey(stMKey)){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        }

        stKey = m_cKeyC90.FindKey(strWKey);
        if (!m_cKeyC90.IsInvalidKey(stKey))
        {
            if (stKey.dwUse & WFS_PIN_USENODUPLICATE)
            {
                Log(ThisModule, __LINE__, "已下载工作密钥，有唯一属性：%s", strWKey.constData());
                return WFS_ERR_PIN_DUPLICATEKEY;
            }
        }

        // 支持多用途处理
        QByteArray strWKName;
        vectorUINT vtUse;
        if (!SplitKeyUse(wUse, vtUse))
        {
            Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", wUse, strWKey.constData());
            return WFS_ERR_INVALID_DATA;
        }

        //所有Key默认带CRYPT属性
        if((wUse & WFS_PIN_USECRYPT) == 0){
            vtUse.insert(vtUse.begin(), (UINT)PKU_CRYPT);
        }

        // 先删除密钥
        m_cKeyC90.DeleteKey(strWKey);
        UINT uKcvDecryptKeyNo = 0xFF;
        for (auto &it : vtUse)
        {
            if (it == PKU_CRYPT)
                strWKName = strWKey + KEY_USE_CRYPT;
            else if (it == PKU_FUNCTION)
                strWKName = strWKey + KEY_USE_FUNCTION;
            else if (it == PKU_MACING)
                strWKName = strWKey + KEY_USE_MACING;
            else if (it == PKU_KEYENCKEY)                       //FT#0001
                strWKName = strWKey;                            //FT#0001
//          else if (it == PKU_SVENCKEY)
//              strWKName = strWKey + KEY_USE_SVENCHEY;
            else
            {
                Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", it, strWKey.constData());
                return WFS_ERR_INVALID_DATA;
            }

            if(it == PKU_KEYENCKEY){
                stKey = m_cKeyC90.FindMKeySlot(strWKName);
            } else {
                stKey = m_cKeyC90.FindWKeySlot(strWKName);
                if(it == PKU_CRYPT){
                    uKcvDecryptKeyNo = stKey.uNum;
                }
            }

            if (m_cKeyC90.IsMaxKey(stKey))
            {
                Log(ThisModule, __LINE__, "工作密钥已下载满：%s", strWKName.constData());
                return WFS_ERR_PIN_NOKEYRAM;
            }

            if(it == vtUse.back()){
                //如果是最后一个key，获取KCV
                m_pDev->SetDesKcvDecryptKey(uKcvDecryptKeyNo);
                lRet = m_pDev->ImportWKey(stKey.uNum, stMKey.uNum, it, cHex.GetHex(), szKCV);
            } else {
                lRet = m_pDev->ImportWKey(stKey.uNum, stMKey.uNum, it, cHex.GetHex(), nullptr);
            }
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "下载工作密钥失败：%s", strWKName.constData());
                return WFS_ERR_HARDWARE_ERROR;
            }

            // 保存密钥
            stKey.dwKeyLen = lpImport->lpxValue->usLength;
            stKey.dwUse = wUse;
            strcpy(stKey.szKCV, szKCV);
            m_cKeyC90.AddKey(stKey);
            Log(ThisModule, 1, "多用途下载工作密钥成功：%s]", stKey.szName);
            m_pDev->SetDesKeyLenInfo(m_cKeyC90.GetDesKeyLenInfo());
        }
    }


    // 返回结果
    QByteArray vtBCD;
    cHex.Hex2Bcd(szKCV, vtBCD);
    m_cXData.Add((LPBYTE)vtBCD.data(), vtBCD.size());
    lpxKVC = m_cXData.GetData();
    Log(ThisModule, 1, "下载密钥成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GetPIN(const LPWFSPINGETPIN lpGetPin, LPWFSPINENTRY &lpEntry, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet;
    vectorWFSPINKEY vtKeys;

    CInputDataHelper cInput(lpGetPin);
    cInput.SetNotCheckFDK((m_strNotCheckFDK == "1") ? true : false);

    //证通密码键盘输入最大长度为12
    if(m_strDevName == "ZT"){                                                       //30-00-00-00(FT#0025)
        if((lpGetPin->usMaxLen == 0) || (lpGetPin->usMaxLen > 12)){                 //30-00-00-00(FT#0025)
            Log(ThisModule, __LINE__, "无效最大输入长度:%d", lpGetPin->usMaxLen);      //30-00-00-00(FT#0025)
            return WFS_ERR_INVALID_DATA;                                            //30-00-00-00(FT#0025)
        }                                                                           //30-00-00-00(FT#0025)
    }                                                                               //30-00-00-00(FT#0025)

    if(cInput.MaxLen() == (USHORT)-1){                  //30-00-00-00(FT#0023)
        return WFS_ERR_INVALID_DATA;                    //30-00-00-00(FT#0023)
    }                                                   //30-00-00-00(FT#0023)

    if ((cInput.MinLen() == (USHORT)-1) || (cInput.MaxLen() < cInput.MinLen())) //30-00-00-00(FT#0024)
    {
        Log(ThisModule, __LINE__, "无效输入长度");
        return WFS_ERR_PIN_MINIMUMLENGTH;
    }

//30-00-00-00(FS#0010)    if(m_strDevName == "ZT"){
//30-00-00-00(FS#0010)        if(cInput.MinLen() == 0 && cInput.MaxLen() == 0){
//30-00-00-00(FS#0010)            return WFS_ERR_PIN_INVALIDDATA;
//30-00-00-00(FS#0010)        }
//30-00-00-00(FS#0010)    }

    if(!cInput.IsHasActiveKey()){                       //30-00-00-00(FT#0021)
        Log(ThisModule, __LINE__, "没有激活功能按键");     //30-00-00-00(FT#0021)
        return WFS_ERR_PIN_NOACTIVEKEYS;                //30-00-00-00(FT#0021)
    }                                                   //30-00-00-00(FT#0021)

    if (!cInput.IsSupportGetPinKey(GetFunctionKeys()))
    {
        Log(ThisModule, __LINE__, "有不支持按键");
        return WFS_ERR_PIN_KEYNOTSUPPORTED;
    }

    if(!cInput.IsTermKeyActived()){                     //30-00-00-00(FT#0021)
        return WFS_ERR_INVALID_DATA;                    //30-00-00-00(FT#0021)
    }                                                   //30-00-00-00(FT#0021)
//30-00-00-00(FT#0021)    if (!cInput.IsHasActiveKey())
//30-00-00-00(FT#0021)    {
//30-00-00-00(FT#0021)        Log(ThisModule, __LINE__, "没有激活功能按键");
//30-00-00-00(FT#0021)        return WFS_ERR_PIN_NOACTIVEKEYS;
//30-00-00-00(FT#0021)    }
    //    if (cInput.IsActiveKey(WFS_PIN_FK_CANCEL) && !cInput.IsTermKey(WFS_PIN_FK_CANCEL))
    //    {
    //        Log(ThisModule, __LINE__, "激活的[取消]键不为结束键");
    //        return WFS_ERR_UNSUPP_DATA;
    //    }

    // 设置激活和禁用按键
    bool bClearKeyAsBackspace = false;
    DWORD dwActiveKey = cInput.GetActiveKey();
    if(2 == m_iClearKeyMode){
        //物理上CLEAR和BACKSPACE是同一个按键，WFS_PIN_FK_CLEAR和WFS_PIN_FK_BACKSPACE同时选中时，按WFS_PIN_FK_CLEAR处理
        if((dwActiveKey & WFS_PIN_FK_CLEAR) && (dwActiveKey & WFS_PIN_FK_BACKSPACE)){
            dwActiveKey &= (~WFS_PIN_FK_BACKSPACE);
        }        
    }
    if(dwActiveKey & WFS_PIN_FK_BACKSPACE) bClearKeyAsBackspace = true;
    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        lRet = m_pDev->SetActiveKey(dwActiveKey, cInput.GetActiveFDK());
    } else {
        if(dwActiveKey & WFS_PIN_FK_BACKSPACE){
            dwActiveKey &= (~WFS_PIN_FK_BACKSPACE);
            dwActiveKey |= WFS_PIN_FK_CLEAR;
        }

        QByteArray strKeyVal = GetAllKeyVal(dwActiveKey, cInput.GetActiveFDK());
        lRet = m_pDev->SetKeyValue(strKeyVal);
    }
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "设置激活和禁用按键失败");
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 清事件
//30-00-00-00(FT#0070)    while (m_cCancelEvent.WaitForEvent(1));
    lRet = m_pDev->PinInput(cInput.MinLen(), cInput.MaxLen(), bClearKeyAsBackspace);
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "打开密文输入失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 读取按键
    Log(ThisModule, 1, "开始读取密文输入");
    memset(&m_stPinEntry, 0x00, sizeof(m_stPinEntry));
//    if(m_strDevName == "XZ"){
      lRet = GetPinKeyPress_XZ(vtKeys, cInput, dwTimeOut);
//    } else {
//        lRet = GetKeyPress(vtKeys, cInput, dwTimeOut);
//    }
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "读取密文按键失败：%d", lRet);
        return lRet;
    }

    m_stPinEntry.usDigits = vtKeys.size();
    m_stPinEntry.wCompletion = cInput.GetEndKey().wCompletion;
    lpEntry = &m_stPinEntry;
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GetPinBlock(const LPWFSPINBLOCK lpPinBlock, LPWFSXDATA &lpxPinBlock)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    bool bNoKey = false;
    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        bNoKey = m_cKeyXZ.IsEmpty();
    } else if(m_strDevName == "ZT") {
        bNoKey = m_cKey.IsEmpty();
    } else {
        bNoKey = m_cKeyC90.IsEmpty();
    }
    if (bNoKey)
    {
        Log(ThisModule, __LINE__, "没有下载密钥");
        return WFS_ERR_PIN_ACCESSDENIED;
    }

    //参数检查
    if (lpPinBlock->lpsKey == nullptr || strlen(lpPinBlock->lpsKey) == 0)
    {
        Log(ThisModule, __LINE__, "无效密钥名称");
        return WFS_ERR_INVALID_DATA;
    }
    // 不支持二次加密
    if (lpPinBlock->lpsKeyEncKey != nullptr && strlen(lpPinBlock->lpsKeyEncKey) > 0)
    {
        Log(ThisModule, __LINE__, "不支持二次加密");
        return WFS_ERR_UNSUPP_DATA;
    }
    //判断填充字符(0x00 ~ 0x0F)
    if(lpPinBlock->bPadding > 0x0F){
        Log(ThisModule, __LINE__, "填充字符非法[%02x]", lpPinBlock->bPadding);
        return WFS_ERR_INVALID_DATA;
    }

    if(lpPinBlock->lpsCustomerData == nullptr || strlen(lpPinBlock->lpsCustomerData) == 0){
        Log(ThisModule, __LINE__, "无效用户数据指针参数或数据为空");
        return WFS_ERR_INVALID_DATA;
    }

    UINT uPinFmt = ConvertPinFormat(lpPinBlock->wFormat);
    if (uPinFmt == 0)
    {
        Log(ThisModule, __LINE__, "不支持此模式：wFormat=%d", lpPinBlock->wFormat);
        return WFS_ERR_PIN_FORMATNOTSUPP;
    }

    /*深圳原版程序处理
                // ISO0的bPadding只能为0x0F
                if (uPinFmt == PIN_ISO9564_0 && lpPinBlock->bPadding != 0x0F)
                {
                    Log(ThisModule, __LINE__, "无效填充字符");
                    return WFS_ERR_INVALID_DATA;
                }
    */

    // 用户数据
    QByteArray strCustomerData;
    strCustomerData = lpPinBlock->lpsCustomerData;
    if (0 != FmtCustomerData(strCustomerData, uPinFmt))
    {
        Log(ThisModule, __LINE__, "CustomerData无效：%s", strCustomerData.constData());
        return WFS_ERR_INVALID_DATA;
    }

    long lRet = 0;
    bool bSM4 = false;
    bool bFindKey = false;
    char szPinBlock[64] = { 0 };
    QByteArray strWKey(lpPinBlock->lpsKey);
    // 南京银行Linux，使用方式和DES一样，只通过配置切换是否为国密
    if (IsSupportSM() && IsOnlyUseSM())
    {
        bSM4 = true;
        uPinFmt = PIN_SM4;
        strWKey += KEY_SM4_FUNCTION;
    }
    else
    {
        strWKey += KEY_USE_FUNCTION;
    }

    PINKEYDATA stKey;
    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        strWKey = lpPinBlock->lpsKey;
        if (bSM4)
            stKey = m_cKeyXZ.FindSMKey(strWKey);
        else
            stKey = m_cKeyXZ.FindKey(strWKey);
        if (m_cKeyXZ.IsInvalidKey(stKey))
        {
            Log(ThisModule, __LINE__, "没有找到对应的密钥：%s", strWKey.constData());
            return WFS_ERR_PIN_KEYNOTFOUND;
        }
        if(!(stKey.dwUse & WFS_PIN_USEFUNCTION)){
            return WFS_ERR_PIN_USEVIOLATION;
        }
    } else if(m_strDevName == "ZT") {
        stKey = m_cKey.FindKey(strWKey, !bSM4);
        bFindKey = !m_cKey.IsInvalidKey(stKey);
        if (!bFindKey)
        {
            Log(ThisModule, __LINE__, "没有找到对应的密钥：%s", strWKey.constData());
            return WFS_ERR_PIN_KEYNOTFOUND;
        }
    } else {
        stKey = m_cKeyC90.FindKey(strWKey, !bSM4);
        bFindKey = !m_cKeyC90.IsInvalidKey(stKey);
        if (!bFindKey)
        {
            Log(ThisModule, __LINE__, "没有找到对应的密钥：%s", strWKey.constData());
            return WFS_ERR_PIN_KEYNOTFOUND;
        }
    }

    if (bSM4)
        lRet = m_pDev->SM4PinBlock(stKey.uNum, uPinFmt, strCustomerData, lpPinBlock->bPadding, szPinBlock);
    else
        lRet = m_pDev->GetPinBlock(stKey.uNum, uPinFmt, strCustomerData, lpPinBlock->bPadding, szPinBlock);
    if (lRet != ERR_SUCCESS)
    {
        if(lRet == ERR_NO_PIN){
            return WFS_ERR_PIN_NOPIN;
        }
        Log(ThisModule, __LINE__, "读取PinBlock失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 返回结果
    CAutoHex cHex;
    QByteArray vtBCD;
    cHex.Hex2Bcd(szPinBlock, vtBCD);
    m_cXData.Add((LPBYTE)vtBCD.constData(), vtBCD.size());
    lpxPinBlock = m_cXData.GetData();
    Log(ThisModule, 1, "读取PinBlock成功");
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GetData(const LPWFSPINGETDATA lpPinGetData, LPWFSPINDATA &lpPinData, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet;
    vectorWFSPINKEY vtKeys;
    CInputDataHelper cInput(lpPinGetData);
    cInput.SetNotCheckFDK((m_strNotCheckFDK == "1") ? true : false);

    if(cInput.MaxLen() == (USHORT)-1){                      //30-00-00-00(FT#0023)
        return WFS_ERR_INVALID_DATA;                        //30-00-00-00(FT#0023)
    }                                                       //30-00-00-00(FT#0023)

    if(!cInput.IsHasActiveKey()){                           //30-00-00-00(FT#0021)
        Log(ThisModule, __LINE__, "没有激活功能按键");         //30-00-00-00(FT#0021)
        return WFS_ERR_PIN_NOACTIVEKEYS;                    //30-00-00-00(FT#0021)
    }                                                       //30-00-00-00(FT#0021)
    if (!cInput.IsSupportKey(GetFunctionKeys(), GetFDKs()))
    {
        Log(ThisModule, __LINE__, "有不支持按键");
        return WFS_ERR_PIN_KEYNOTSUPPORTED;
    }

    if(!cInput.IsTermKeyActived()){                     //30-00-00-00(FT#0021)
        return WFS_ERR_INVALID_DATA;                    //30-00-00-00(FT#0021)
    }                                                   //30-00-00-00(FT#0021)
//30-00-00-00(FT#0021)    if (!cInput.IsHasActiveKey())
//30-00-00-00(FT#0021)    {
//30-00-00-00(FT#0021)        Log(ThisModule, __LINE__, "没有激活功能按键");
//30-00-00-00(FT#0021)        return WFS_ERR_PIN_NOACTIVEKEYS;
//30-00-00-00(FT#0021)    }

//30-00-00-00(FS#0010)    if(m_strDevName == "ZT" && cInput.MaxLen() == 0){
//30-00-00-00(FS#0010)        return WFS_ERR_INVALID_DATA;
//30-00-00-00(FS#0010)    }

    // 设置激活和禁用按键
    DWORD dwActiveKey = cInput.GetActiveKey();
    if(2 == m_iClearKeyMode){
        //物理上CLEAR和BACKSPACE是同一个按键，WFS_PIN_FK_CLEAR和WFS_PIN_FK_BACKSPACE同时选中时，按WFS_PIN_FK_CLEAR处理
        if((dwActiveKey & WFS_PIN_FK_CLEAR) && (dwActiveKey & WFS_PIN_FK_BACKSPACE)){
            dwActiveKey &= (~WFS_PIN_FK_BACKSPACE);
        }
    }
    if(m_strDevName == "XZ" || m_strDevName == "CFES"){       
        lRet = m_pDev->SetActiveKey(dwActiveKey, cInput.GetActiveFDK());
    } else {
        if(dwActiveKey & WFS_PIN_FK_BACKSPACE){
            dwActiveKey &= (~WFS_PIN_FK_BACKSPACE);
            dwActiveKey |= WFS_PIN_FK_CLEAR;
        }
        QByteArray strKeyVal = GetAllKeyVal(dwActiveKey, cInput.GetActiveFDK());
        lRet = m_pDev->SetKeyValue(strKeyVal);
    }
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "设置激活和禁用按键失败");
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 清事件
//30-00-00-00(FT#0070)    while (m_cCancelEvent.WaitForEvent(1));
    lRet = m_pDev->TextInput();
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "打开明文输入失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 读取按键
    Log(ThisModule, 1, "开始读取明文输入");
    m_cGetDataKeys.Clear();
    lRet = GetKeyPress(vtKeys, cInput, dwTimeOut);
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "读取明文按键失败：%d", lRet);
        return lRet;
    }

    m_cGetDataKeys.Add(vtKeys);
    lpPinData = m_cGetDataKeys.GetData(cInput.GetEndKey().wCompletion);
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::Initialization(const LPWFSPININIT lpInit, LPWFSXDATA &lpxIdentification)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    lpxIdentification = nullptr;
    if ((lpInit->lpxIdent != nullptr && lpInit->lpxIdent->lpbData != nullptr) ||
        (lpInit->lpxKey != nullptr && lpInit->lpxKey->lpbData != nullptr))
    {
        Log(ThisModule, __LINE__, "不支持指定删除");
        return WFS_ERR_UNSUPP_DATA;
    }

    // 初始化
    long lRet = m_pDev->InitEpp();
    if (ERR_SUCCESS != lRet)
    {
        Log(ThisModule, __LINE__, "初始化失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (IsSupportSM())
    {
        lRet = m_pDev->SMDeleteKey();
        if (ERR_SUCCESS != lRet)
        {
            Log(ThisModule, __LINE__, "清除国密密钥失败：%d", lRet);
            return WFS_ERR_HARDWARE_ERROR;
        }
        // Allow download SM4 Key of same value
        UINT uType = 1;// 0->不相同，1->可相同
        lRet = m_pDev->SM4SaveKeyType(1);
        if (ERR_SUCCESS != lRet)
        {
            Log(ThisModule, __LINE__, "设置国密密钥保存方式失败：%d[uType=%d]", lRet, uType);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }

    // 清安装标志
    lRet = m_pDev->InstallStatus(true);
    if (ERR_SUCCESS != lRet)
    {
        Log(ThisModule, __LINE__, "清安装标志失败：%d", lRet);
    }

    // 删除密钥记录
    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        m_cKeyXZ.DeleteAllKey();
    } else if(m_strDevName == "ZT") {
        m_cKey.DeleteAllKey();
    } else {
        m_cKeyC90.DeleteAllKey();
    }

    // 发送事件
    WFSPININIT stInit;
    stInit.lpxIdent = nullptr;
    stInit.lpxKey = nullptr;
    m_pBase->FireEvent(WFS_SERVICE_EVENT, WFS_SRVE_PIN_INITIALIZED, &stInit);
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::Reset()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = m_pDev->Reset();
    if (0 != lRet)
    {
        Log(ThisModule, __LINE__, "复位失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GenerateKCV(const LPWFSPINGENERATEKCV lpGenerateKCV, LPWFSPINKCV &lpKCV)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if (lpGenerateKCV->lpsKey == nullptr)
    {
        Log(ThisModule, __LINE__, "无效指针参数");
        return WFS_ERR_INVALID_DATA;
    }

    CAutoHex cHex;
    QByteArray vtBCD;
    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        PINKEYDATA stPinKeyData = m_cKeyXZ.FindKey(lpGenerateKCV->lpsKey);
        if(m_cKeyXZ.IsInvalidKey(stPinKeyData)){
            if(IsSupportSM()){
                stPinKeyData = m_cKeyXZ.FindSMKey(lpGenerateKCV->lpsKey);
            }
            if(m_cKeyXZ.IsInvalidKey(stPinKeyData)){
                Log(ThisModule, __LINE__, "没有找到对应的密钥：%s", lpGenerateKCV->lpsKey);
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        }

        char szKcv[64] = {0};
        bool bSM4Key = (stPinKeyData.dwType == 2 || stPinKeyData.dwType == 3);
        long lRet = m_pDev->ReadSymmKeyKCV(stPinKeyData.uNum, lpGenerateKCV->wKeyCheckMode, szKcv, bSM4Key ? 2 : 0);
        if(lRet != ERR_SUCCESS){
            Log(ThisModule, __LINE__, "获取对称密钥KCV失败：%s", lpGenerateKCV->lpsKey);
            return WFS_ERR_HARDWARE_ERROR;
        }

        cHex.Hex2Bcd(szKcv, vtBCD);
    } else if(m_strDevName == "ZT") {
        if (lpGenerateKCV->wKeyCheckMode != WFS_PIN_KCVZERO)
        {
            Log(ThisModule, __LINE__, "不支持此KCV模式：%d", lpGenerateKCV->wKeyCheckMode);
            return WFS_ERR_UNSUPP_DATA;
        }
        if (m_cKey.IsEmpty())
        {
            Log(ThisModule, __LINE__, "没有下载密钥");
            return WFS_ERR_PIN_ACCESSDENIED;
        }

        PINKEYDATA stKey = m_cKey.FindKey(lpGenerateKCV->lpsKey);
        if (m_cKey.IsInvalidKey(stKey))
        {
            if(IsSupportSM()){
                stKey = m_cKey.FindKey(lpGenerateKCV->lpsKey, false);
            }
            if(m_cKey.IsInvalidKey(stKey)){
                Log(ThisModule, __LINE__, "没有找到对应的密钥：%s", lpGenerateKCV->lpsKey);
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        }

        // 返回结果
        cHex.Hex2Bcd(stKey.szKCV, vtBCD);
    } else {
        if (lpGenerateKCV->wKeyCheckMode != WFS_PIN_KCVZERO)
        {
            Log(ThisModule, __LINE__, "不支持此KCV模式：%d", lpGenerateKCV->wKeyCheckMode);
            return WFS_ERR_UNSUPP_DATA;
        }
        if (m_cKeyC90.IsEmpty())
        {
            Log(ThisModule, __LINE__, "没有下载密钥");
            return WFS_ERR_PIN_ACCESSDENIED;
        }

        PINKEYDATA stKey = m_cKeyC90.FindKey(lpGenerateKCV->lpsKey);
        if (m_cKeyC90.IsInvalidKey(stKey))
        {
            if(IsSupportSM()){
                stKey = m_cKeyC90.FindKey(lpGenerateKCV->lpsKey, false);
            }
            if(m_cKeyC90.IsInvalidKey(stKey)){
                Log(ThisModule, __LINE__, "没有找到对应的密钥：%s", lpGenerateKCV->lpsKey);
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
        }

        // 返回结果
        cHex.Hex2Bcd(stKey.szKCV, vtBCD);
    }

    // 返回结果    
    m_cXData.Add((LPBYTE)vtBCD.constData(), vtBCD.size());
    m_stKCV.lpxKCV = m_cXData.GetData();
    lpKCV = &m_stKCV;
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::GetPinBlockEx(const LPWFSPINBLOCKEX lpPinBlockEx, LPWFSXDATA &lpxPinBlock)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    // 判断是否支持国密
    if (!IsSupportSM())
    {
        Log(ThisModule, __LINE__, "不支持国密");
        return WFS_ERR_UNSUPP_COMMAND;
    }

    bool bNoKey = false;
    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        bNoKey = m_cKeyXZ.IsEmpty();
    } else if(m_strDevName == "ZT") {
        bNoKey = m_cKey.IsEmpty();
    } else {
        bNoKey = m_cKeyC90.IsEmpty();
    }
    if (bNoKey)
    {
        Log(ThisModule, __LINE__, "没有下载密钥");
        return WFS_ERR_PIN_ACCESSDENIED;
    }

    //参数检查
    if (lpPinBlockEx->lpsKey == nullptr || strlen(lpPinBlockEx->lpsKey) == 0)
    {
        Log(ThisModule, __LINE__, "无效密钥名称");
        return WFS_ERR_INVALID_DATA;
    }
    // 不支持二次加密
    if (lpPinBlockEx->lpsKeyEncKey != nullptr && strlen(lpPinBlockEx->lpsKeyEncKey) > 0)
    {
        Log(ThisModule, __LINE__, "不支持二次加密");
        return WFS_ERR_UNSUPP_DATA;
    }
    if(lpPinBlockEx->bPadding > 0x0F){
        Log(ThisModule, __LINE__, "填充字符非法[%02x]", lpPinBlockEx->bPadding);
        return WFS_ERR_INVALID_DATA;
    }
    if (lpPinBlockEx->lpsCustomerData == nullptr)
    {
        Log(ThisModule, __LINE__, "无效用户数据指针参数");
        return WFS_ERR_INVALID_DATA;
    }

//    UINT uPinFmt = ConvertPinFormat(lpPinBlockEx->dwFormat);
//    if (uPinFmt == 0)
//    {
//        Log(ThisModule, __LINE__, "不支持此模式：dwFormat=%d", lpPinBlockEx->dwFormat);
//        return WFS_ERR_PIN_FORMATNOTSUPP;
//    }
    //国密pinblock算法只支持一种，依据ap的不同传入的参数不一定是WFS_PIN_FORSM4，该处不做检查，设为固定值
    UINT uPinFmt = PIN_SM4;

    // 用户数据
    QByteArray strCustomerData("000000000000");
    strCustomerData = lpPinBlockEx->lpsCustomerData;
    if (0 != FmtCustomerData(strCustomerData, uPinFmt))
    {
        Log(ThisModule, __LINE__, "CustomerData无效：%s", strCustomerData.constData());
        return WFS_ERR_INVALID_DATA;
    }

    QByteArray strWKey(lpPinBlockEx->lpsKey);
    PINKEYDATA stKey;
    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        stKey = m_cKeyXZ.FindSMKey(strWKey);
        if (m_cKeyXZ.IsInvalidKey(stKey))
        {
            Log(ThisModule, __LINE__, "没有找到对应的密钥：%s", strWKey.constData());
            return WFS_ERR_PIN_KEYNOTFOUND;
        }
    } else if(m_strDevName == "ZT") {
        strWKey += KEY_SM4_FUNCTION;
        stKey = m_cKey.FindKey(strWKey, false);
        if (!m_cKey.IsSMKey(stKey))
        {
            Log(ThisModule, __LINE__, "没有找到对应的密钥：%s", strWKey.constData());
            return WFS_ERR_PIN_KEYNOTFOUND;
        }
    } else {
        strWKey += KEY_SM4_FUNCTION;
        stKey = m_cKeyC90.FindKey(strWKey, false);
        if (!m_cKeyC90.IsSMKey(stKey))
        {
            Log(ThisModule, __LINE__, "没有找到对应的密钥：%s", strWKey.constData());
            return WFS_ERR_PIN_KEYNOTFOUND;
        }
    }


    char szPinBlock[64] = { 0 };
    long lRet = m_pDev->SM4PinBlock(stKey.uNum, PIN_SM4, strCustomerData, lpPinBlockEx->bPadding, szPinBlock);
    if (lRet != ERR_SUCCESS)
    {
        if(lRet == ERR_NO_PIN){         //30-00-00-00(FT#0022)
            return WFS_ERR_PIN_NOPIN;   //30-00-00-00(FT#0022)
        }                               //30-00-00-00(FT#0022)
        Log(ThisModule, __LINE__, "读取PinBlock失败：%d", lRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    // 返回结果
    CAutoHex cHex;
    QByteArray vtBCD;
    cHex.Hex2Bcd(szPinBlock, vtBCD);
    m_cXData.Add((LPBYTE)vtBCD.constData(), vtBCD.size());
    lpxPinBlock = m_cXData.GetData();
    Log(ThisModule, 1, "读取PinBlockEx成功");
    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::ImportKeyEx(const LPWFSPINIMPORTKEYEX lpImportEx)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    if ((lpImportEx->lpsKey == nullptr || strlen(lpImportEx->lpsKey) == 0) ||
        (lpImportEx->dwUse != 0 && lpImportEx->lpxValue == nullptr))
    {
        Log(ThisModule, __LINE__, "无效指针参数");
        return WFS_ERR_INVALID_DATA;
    }
    // 判断是否支持国密
    if (!IsSupportSM())
    {
        Log(ThisModule, __LINE__, "不支持国密");
        return WFS_ERR_UNSUPP_COMMAND;
    }

    Log(ThisModule, __LINE__, "参数:%s, %s, dwUse=%ld, KeyValLen=%d", lpImportEx->lpsKey, lpImportEx->lpsEncKey, lpImportEx->dwUse,
        lpImportEx->lpxValue->usLength);

    DWORD dwUse = lpImportEx->dwUse;
    WORD wKeyCheckMode = lpImportEx->wKeyCheckMode;
    QByteArray strWKey(lpImportEx->lpsKey);
    QByteArray strMKey;
    PINKEYDATA stKey;
    if (dwUse == 0)// 删除指定密钥
    {
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            stKey = m_cKeyXZ.FindSMKey(strWKey);
            if (m_cKeyXZ.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            m_cKeyXZ.DeleteSMKey(strWKey);
        } else if(m_strDevName == "ZT") {
            stKey = m_cKey.FindKey(strWKey, false);
            if (m_cKey.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            m_cKey.DeleteKey(strWKey, false);
        } else {
            stKey = m_cKeyC90.FindKey(strWKey, false);
            if (m_cKeyC90.IsInvalidKey(stKey))
            {
                Log(ThisModule, __LINE__, "没有找到密钥：%s", strWKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            m_cKeyC90.DeleteKey(strWKey, false);
        }
        m_pDev->SMDeleteKey(stKey.uNum);// 如果是国密，还要物理删除
        return WFS_SUCCESS;
    }

    // 判断用途
    if (!IsSupportKeyUse(dwUse))
    {
        Log(ThisModule, __LINE__, "不支持密钥用途:dwUse=%u", dwUse);
        return WFS_ERR_UNSUPP_DATA;
    }

    // 判断校验方式
    if (wKeyCheckMode == WFS_PIN_KCVSELF && (m_strDevName == "ZT"))
    {
        Log(ThisModule, __LINE__, "不支持密钥校验方式:wKeyCheckMode=%d", wKeyCheckMode);
        return WFS_ERR_UNSUPP_DATA;
    }
//    if (wKeyCheckMode == WFS_PIN_KCVZERO)
    if(wKeyCheckMode != WFS_PIN_KCVNONE)
    {
        if (lpImportEx->lpxKeyCheckValue->lpbData == nullptr || lpImportEx->lpxKeyCheckValue->usLength == 0)
        {
            Log(ThisModule, __LINE__, "无效校验值KCV");
            return WFS_ERR_INVALID_DATA;
        }

    }
    // 判断密钥长度是否为16倍数
    if (lpImportEx->lpxValue->usLength == 0 || lpImportEx->lpxValue->usLength % 16 != 0)
    {
        Log(ThisModule, __LINE__, "无效密钥长度: usLength=%d", lpImportEx->lpxValue->usLength);
        return WFS_ERR_PIN_INVALIDKEYLENGTH;
    }


    long lRet = 0;
    char szKCV[64] = { 0 };
    CAutoHex cHex(lpImportEx->lpxValue->lpbData, lpImportEx->lpxValue->usLength);
    CAutoHex cHexKCV;

    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        //密钥属性支持性检查
        if(dwUse & (WFS_PIN_USESVENCKEY | WFS_PIN_USECONSTRUCT | WFS_PIN_USESECURECONSTRUCT)){
            Log(ThisModule, __LINE__, "无效或不支持国密密钥用途：%02X[%s]", dwUse, strWKey.constData());
            return WFS_ERR_INVALID_DATA;
        }

        dwUse = ConvertKeyUse(dwUse);
        PINKEYDATA stMKey;
        stMKey.uNum = KEY_NOT_NUM;
        if(lpImportEx->lpsEncKey != nullptr && strlen(lpImportEx->lpsEncKey) > 0){
            strMKey = lpImportEx->lpsEncKey;
            stMKey = m_cKeyXZ.FindSMKey(strMKey);
            if (m_cKeyXZ.IsInvalidKey(stMKey))
            {
                Log(ThisModule, __LINE__, "没有对应国密主密钥：%s", strMKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }

            if(!(stMKey.dwUse & WFS_PIN_USEKEYENCKEY)){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        }

        stKey = m_cKeyXZ.FindSMKey(strWKey);
        if (!m_cKeyXZ.IsInvalidKey(stKey))
        {
            if(stKey.dwUse & WFS_PIN_USENODUPLICATE){
                Log(ThisModule, __LINE__, "已下载国密密钥，有唯一属性：%s", strWKey.constData());
                return WFS_ERR_PIN_DUPLICATEKEY;
            }
            // 先删除密钥
            m_cKeyXZ.DeleteSMKey(strWKey);
        }

        stKey = m_cKeyXZ.FindSMKeySlot(strWKey);
        if (m_cKeyXZ.IsMaxKey(stKey))
        {
            Log(ThisModule, __LINE__, "国密密钥已下载满：%s", strWKey.constData());
            return WFS_ERR_PIN_NOKEYRAM;
        }

        // 国密
        lRet = m_pDev->SM4ImportWKey(stKey.uNum, stMKey.uNum, dwUse, cHex.GetHex(), szKCV,
                                     WFS_PIN_KCVZERO);
        if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "下载国密密钥失败：%s[lRet=%d]", strWKey.constData(), lRet);
            return WFS_ERR_HARDWARE_ERROR;
        }

        // 校验KCV
        if (wKeyCheckMode != WFS_PIN_KCVNONE)
        {
            char szKcvHexComp[64] = {0};
            strcpy(szKcvHexComp, szKCV);
            if(wKeyCheckMode != WFS_PIN_KCVZERO){
                lRet = m_pDev->ReadSymmKeyKCV(stKey.uNum, wKeyCheckMode, szKcvHexComp, 0x02);
                if(lRet != ERR_SUCCESS){
                    return WFS_ERR_HARDWARE_ERROR;
                }
            }
            cHexKCV.Bcd2Hex(lpImportEx->lpxKeyCheckValue->lpbData, lpImportEx->lpxKeyCheckValue->usLength);
            int nCmpLen = qMin(strlen(cHexKCV.GetHex()), strlen(szKcvHexComp));
            if (qstrnicmp(cHexKCV.GetHex(), szKcvHexComp, nCmpLen) != 0)
            {
                Log(ThisModule, __LINE__, "校验KCV失败，密钥值有误");
                m_cKeyXZ.DeleteSMKey(stKey.szName);
                m_pDev->SMDeleteKey(stKey.uNum);// 如果是国密，还要物理删除
                return WFS_ERR_PIN_KEYINVALID;
            }
        }

        // 保存密钥
        if(lpImportEx->dwUse & WFS_PIN_USENODUPLICATE){
            dwUse |= WFS_PIN_USENODUPLICATE;
        }
        stKey.dwUse = dwUse;
        strcpy(stKey.szKCV, szKCV);
        m_cKeyXZ.AddKey(stKey);
        Log(ThisModule, 1, "下载国密工作密钥成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
    } else if(m_strDevName == "ZT") {
        PINKEYDATA stMKey;
        memset(&stMKey, 0, sizeof(stMKey));
        stMKey.uNum = 0xFF;
        if(lpImportEx->lpsEncKey != nullptr && strlen(lpImportEx->lpsEncKey) > 0){
            strMKey = lpImportEx->lpsEncKey;
            stMKey = m_cKey.FindKey(strMKey, false);
            if (m_cKey.IsInvalidKey(stMKey))
            {
                Log(ThisModule, __LINE__, "没有对应主密钥：%s", strMKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            if(!m_cKey.IsMKey(stMKey)){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        }

        QByteArray strWKName;
        vectorUINT vtUse;
        if (!SplitSMKeyUse(dwUse, vtUse))
        {
            Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", dwUse, strWKey.constData());
            return WFS_ERR_INVALID_DATA;
        }

        // 先删除密钥
        m_cKey.DeleteKey(strWKey, false);
        for (auto &it : vtUse)
        {
            if (it == PKU_CRYPT)
                strWKName = strWKey + KEY_SM4_CRYPT;
            else if (it == PKU_FUNCTION)
                strWKName = strWKey + KEY_SM4_FUNCTION;
            else if (it == PKU_MACING)
                strWKName = strWKey + KEY_SM4_MACING;
            else if (it == PKU_KEYENCKEY)
                strWKName = strWKey;
            else
            {
                Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", it, strWKey.constData());
                return WFS_ERR_INVALID_DATA;
            }

            if(it == PKU_KEYENCKEY){
                stKey = m_cKey.FindMKeySlot(strWKName, lpImportEx->lpsEncKey, false);
            } else {
                stKey = m_cKey.FindWKeySlot(strWKName, false, it);
            }

            if (m_cKey.IsMaxKey(stKey))
            {
                Log(ThisModule, __LINE__, "工作密钥已下载满：%s", strWKName.constData());
                return WFS_ERR_PIN_NOKEYRAM;
            }

            // 国密
//            if(it == vtUse.back()){
                lRet = m_pDev->SM4ImportWKey(stKey.uNum, stMKey.uNum, it, cHex.GetHex(), szKCV);
//            } else {
//                lRet = m_pDev->SM4ImportWKey(stKey.uNum, stMKey.uNum, it, cHex.GetHex(), nullptr);
//            }

            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "下载工作密钥失败：%s[lRet=%d]", strWKName.constData(), lRet);
                return WFS_ERR_HARDWARE_ERROR;
            }

            // 校验KCV
            if (wKeyCheckMode == WFS_PIN_KCVZERO)
            {
                cHexKCV.Bcd2Hex(lpImportEx->lpxKeyCheckValue->lpbData, lpImportEx->lpxKeyCheckValue->usLength);
                int nCmpLen = qMin(strlen(cHexKCV.GetHex()), strlen(szKCV));
                if (qstrnicmp(cHexKCV.GetHex(), szKCV, nCmpLen) != 0)
                {
                    Log(ThisModule, __LINE__, "校验KCV失败，密钥值有误");
                    m_cKey.DeleteKey(stKey.szName, false);
                    m_pDev->SMDeleteKey(stKey.uNum);// 如果是国密，还要物理删除
                    return WFS_ERR_PIN_KEYINVALID;
                }
            }

            // 保存密钥
            stKey.dwUse = dwUse;
            strcpy(stKey.szKCV, szKCV);
            m_cKey.AddKey(stKey);
            Log(ThisModule, 1, "下载国密工作密钥成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
        }
    } else {
        PINKEYDATA stMKey;
        memset(&stMKey, 0, sizeof(stMKey));
        stMKey.uNum = 0xFF;
        if(lpImportEx->lpsEncKey != nullptr && strlen(lpImportEx->lpsEncKey) > 0){
            strMKey = lpImportEx->lpsEncKey;
            stMKey = m_cKeyC90.FindKey(strMKey, false);
            if (m_cKeyC90.IsInvalidKey(stMKey))
            {
                Log(ThisModule, __LINE__, "没有对应主密钥：%s", strMKey.constData());
                return WFS_ERR_PIN_KEYNOTFOUND;
            }
            if(!m_cKeyC90.IsMKey(stMKey)){
                return WFS_ERR_PIN_USEVIOLATION;
            }
        }

        QByteArray strWKName;
        vectorUINT vtUse;
        if (!SplitSMKeyUse(dwUse, vtUse))
        {
            Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", dwUse, strWKey.constData());
            return WFS_ERR_INVALID_DATA;
        }

        // 先删除密钥
        m_cKeyC90.DeleteKey(strWKey, false);
        for (auto &it : vtUse)
        {
            if (it == PKU_CRYPT)
                strWKName = strWKey + KEY_SM4_CRYPT;
            else if (it == PKU_FUNCTION)
                strWKName = strWKey + KEY_SM4_FUNCTION;
            else if (it == PKU_MACING)
                strWKName = strWKey + KEY_SM4_MACING;
            else if (it == PKU_KEYENCKEY)
                strWKName = strWKey;
            else
            {
                Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", it, strWKey.constData());
                return WFS_ERR_INVALID_DATA;
            }

            if(it == PKU_KEYENCKEY){
                stKey = m_cKeyC90.FindMKeySlot(strWKName, lpImportEx->lpsEncKey, false);
            } else {
                stKey = m_cKeyC90.FindWKeySlot(strWKName, false, it);
            }

            if (m_cKeyC90.IsMaxKey(stKey))
            {
                Log(ThisModule, __LINE__, "工作密钥已下载满：%s", strWKName.constData());
                return WFS_ERR_PIN_NOKEYRAM;
            }

            // 国密
//            if(it == vtUse.back()){
                lRet = m_pDev->SM4ImportWKey(stKey.uNum, stMKey.uNum, it, cHex.GetHex(), szKCV);
//            } else {
//                lRet = m_pDev->SM4ImportWKey(stKey.uNum, stMKey.uNum, it, cHex.GetHex(), nullptr);
//            }

            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "下载工作密钥失败：%s[lRet=%d]", strWKName.constData(), lRet);
                return WFS_ERR_HARDWARE_ERROR;
            }

            // 校验KCV
            if (wKeyCheckMode == WFS_PIN_KCVZERO)
            {
                cHexKCV.Bcd2Hex(lpImportEx->lpxKeyCheckValue->lpbData, lpImportEx->lpxKeyCheckValue->usLength);
                int nCmpLen = qMin(strlen(cHexKCV.GetHex()), strlen(szKCV));
                if (qstrnicmp(cHexKCV.GetHex(), szKCV, nCmpLen) != 0)
                {
                    Log(ThisModule, __LINE__, "校验KCV失败，密钥值有误");
                    m_cKeyC90.DeleteKey(stKey.szName, false);
                    m_pDev->SMDeleteKey(stKey.uNum);// 如果是国密，还要物理删除
                    return WFS_ERR_PIN_KEYINVALID;
                }
            }

            // 保存密钥
            stKey.dwUse = dwUse;
            strcpy(stKey.szKCV, szKCV);
            m_cKeyC90.AddKey(stKey);
            Log(ThisModule, 1, "下载国密工作密钥成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
        }
    }

    return WFS_SUCCESS;
}

//30-00-00-00(FS#0006)
HRESULT CXFS_PIN::StarKeyExchange(LPWFSPINSTARTKEYEXCHANGE &lpStartKeyExchange)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    //清空结果数据
    m_cStartKeyExchange.Clear();

    QByteArray randomData;
    if(m_bRSARandomMode){
        //获取随机数        
        long lRet = m_pDev->RSAExportEppRandData(randomData);
        if(lRet != ERR_SUCCESS){
            Log(ThisModule, __LINE__, "获取随机数失败");
            return WFS_ERR_HARDWARE_ERROR;
        }

        //Hex to Bcd
        QByteArray vtTemp = randomData;
        CAutoHex::Hex2Bcd((LPSTR)vtTemp.constData(), randomData);
    }

    m_cStartKeyExchange.Add((LPBYTE)randomData.constData(), randomData.size());
    lpStartKeyExchange = m_cStartKeyExchange.GetData();

    return WFS_SUCCESS;
}

//30-00-00-00(FS#0005)
HRESULT CXFS_PIN::ImportRSAPublicKey(const LPWFSPINIMPORTRSAPUBLICKEY lpPublicKey, LPWFSPINIMPORTRSAPUBLICKEYOUTPUT &lpPublicKeyOutput)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(m_strDevName == "ZT" || m_strDevName == "CFES") {
        QByteArray valueHex;
        QByteArray signatureHex;
        //参数检查
        if(lpPublicKey->lpsKey == nullptr || strlen(lpPublicKey->lpsKey) == 0){
            return WFS_ERR_INVALID_DATA;
        }

        if(lpPublicKey->lpxValue == nullptr || lpPublicKey->lpxValue->usLength == 0 ||
           lpPublicKey->lpxValue->lpbData == nullptr){
            return WFS_ERR_INVALID_DATA;
        }

        if(lpPublicKey->dwUse != WFS_PIN_USERSAPUBLICVERIFY && lpPublicKey->dwUse != WFS_PIN_USERSAPUBLIC){
            return WFS_ERR_PIN_USEVIOLATION;
        }

        CAutoHex::Bcd2Hex(lpPublicKey->lpxValue->lpbData, lpPublicKey->lpxValue->usLength, valueHex);

        if(m_strDevName == "ZT"){
            if(m_bRSAUseSM2Key){
                if(lpPublicKey->dwRSASignatureAlgorithm != WFS_PIN_SIGN_NA){
                    return WFS_ERR_PIN_SIG_NOT_SUPP;
                }

                if(lpPublicKey->lpxValue->usLength != 64){
                    return WFS_ERR_PIN_INVALIDKEYLENGTH;
                }

                //命令执行
                //删除同名key
                m_cKey.DeleteSM2Key(lpPublicKey->lpsKey);

                PINKEYDATA stPKHost;
                QByteArray strKeyName = lpPublicKey->lpsKey;
                stPKHost = m_cKey.FindSM2KeySlot(strKeyName);
                if(m_cKey.IsMaxKey(stPKHost)){
                    return WFS_ERR_PIN_NOKEYRAM;
                }
                long lRet = m_pDev->SM2ImportKey(stPKHost.uNum, 0xFF, true, nullptr, valueHex.constData(), nullptr);
                if(lRet != 0){
                    Log(ThisModule, __LINE__, "导入主机SM2公钥失败");
                    return WFS_ERR_HARDWARE_ERROR;
                }

                //保存密钥数据
                m_cKey.AddKey(stPKHost);
                //结果数据拷贝
                m_cImportRsaPK.Clear();
                m_cImportRsaPK.AddBaseData(WFS_PIN_RSA_KCV_NONE);
                lpPublicKeyOutput = m_cImportRsaPK.GetData();
            } else {
                PINKEYDATA stPKHost;
                PINKEYDATA stVerifSigKey;
                stVerifSigKey.uNum = 0xFF;
                stPKHost = m_cKey.FindRSAKeySlot(lpPublicKey->lpsKey, true);
                if(m_cKey.IsMaxKey(stPKHost)){
                    return WFS_ERR_PIN_NOKEYRAM;
                }

                RSASIGNALGOTYPE eRSASignAlgo = RSA_SIGN_NO;
                switch(lpPublicKey->dwRSASignatureAlgorithm){
                case WFS_PIN_SIGN_RSASSA_PKCS1_V1_5:
                    eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;
                    break;
                case WFS_PIN_SIGN_RSASSA_PSS:
                    eRSASignAlgo = RSA_SIGN_SSA_PSS;
                    break;
                case WFS_PIN_SIGN_NA:
                    break;
                default:
                    return WFS_ERR_PIN_SIG_NOT_SUPP;
                    break;
                }

                if(eRSASignAlgo != RSA_SIGN_NO){
                    CAutoHex::Bcd2Hex(lpPublicKey->lpxSignature->lpbData, lpPublicKey->lpxSignature->usLength, signatureHex);
                    if((lpPublicKey->lpsSigKey != nullptr) && (strlen(lpPublicKey->lpsSigKey) > 0)){
                        if(!m_cKey.FindRSAKey(lpPublicKey->lpsSigKey, stVerifSigKey)){
                            Log(ThisModule, __LINE__, "验签私钥未找到:%s", lpPublicKey->lpsSigKey);
                        }
                    }
                }

                long lRet = m_pDev->RSAImportRootHostPK(stPKHost.uNum, stVerifSigKey.uNum, eRSASignAlgo, valueHex, signatureHex);
                if(lRet != 0){
                    Log(ThisModule, __LINE__, "导入主机RSA公钥失败");
                    return WFS_ERR_HARDWARE_ERROR;
                }

                //保存密钥数据
                m_cKey.AddKey(stPKHost);
                //结果数据拷贝
                m_cImportRsaPK.Clear();
                m_cImportRsaPK.AddBaseData(WFS_PIN_RSA_KCV_NONE);
                lpPublicKeyOutput = m_cImportRsaPK.GetData();
            }
        } else {
            if(m_bRSAUseSM2Key){
                return WFS_ERR_UNSUPP_COMMAND;
            } else {
                PINKEYDATA stPKHost;
                PINKEYDATA stVerifSigKey;                   //验签密钥使用中电签名中心公钥，密钥索引固定为0x00
                strcpy(stPKHost.szName, lpPublicKey->lpsKey);
                stPKHost.uNum = EPP_CFES_HOST_PKEY_NO;
                stPKHost.dwType = 5;
                stPKHost.dwUse = lpPublicKey->dwUse;
                stVerifSigKey.uNum = 0xFF;

                RSASIGNALGOTYPE eRSASignAlgo = RSA_SIGN_NO;
                switch(lpPublicKey->dwRSASignatureAlgorithm){
                case WFS_PIN_SIGN_RSASSA_PKCS1_V1_5:
                    eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;
                    break;
                case WFS_PIN_SIGN_RSASSA_PSS:
                    eRSASignAlgo = RSA_SIGN_SSA_PSS;
                    break;
                case WFS_PIN_SIGN_NA:
                    break;
                default:
                    return WFS_ERR_PIN_SIG_NOT_SUPP;
                    break;
                }

                if(eRSASignAlgo != RSA_SIGN_NO){
                    stVerifSigKey.uNum = EPP_CFES_SC_PKEY_NO;
                    if((lpPublicKey->lpsSigKey != nullptr) && (strlen(lpPublicKey->lpsSigKey) > 0)){
                        if(!m_cKey.FindRSAKey(lpPublicKey->lpsSigKey, stVerifSigKey)){
                            Log(ThisModule, __LINE__, "验签私钥未找到:%s", lpPublicKey->lpsSigKey);
                            stVerifSigKey.uNum = EPP_CFES_SC_PKEY_NO;
                        }
                    }
                    if(lpPublicKey->lpxSignature == nullptr || lpPublicKey->lpxSignature->lpbData == nullptr ||
                       lpPublicKey->lpxSignature->usLength == 0){
                        Log(ThisModule, __LINE__, "签名数据为空");
                        return WFS_ERR_PIN_INVALIDDATA;
                    }
                    CAutoHex::Bcd2Hex(lpPublicKey->lpxSignature->lpbData, lpPublicKey->lpxSignature->usLength, signatureHex);
                }

                long lRet = m_pDev->RSAImportRootHostPK(stPKHost.uNum, stVerifSigKey.uNum, eRSASignAlgo, valueHex, signatureHex);
                if(lRet != 0){
                    Log(ThisModule, __LINE__, "导入银行后台RSA公钥失败");
                    return WFS_ERR_HARDWARE_ERROR;
                }

                //保存密钥数据
                m_cKeyXZ.AddKey(stPKHost);
                //结果数据拷贝
                m_cImportRsaPK.Clear();
                m_cImportRsaPK.AddBaseData(WFS_PIN_RSA_KCV_NONE);
                lpPublicKeyOutput = m_cImportRsaPK.GetData();
            }
        }

    } else {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

//30-00-00-00(FS#0005)
HRESULT CXFS_PIN::ExportRSAIssuerSignedItem(const LPWFSPINEXPORTRSAISSUERSIGNEDITEM lpSignedItem, LPWFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT &lpSignedItemOutput)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(m_strDevName == "ZT" || m_strDevName == "CFES") {
        WORD wExportItemType = lpSignedItem->wExportItemType;
        //参数检查
        if(wExportItemType != WFS_PIN_EXPORT_EPP_ID && wExportItemType != WFS_PIN_EXPORT_PUBLIC_KEY){
            return WFS_ERR_INVALID_DATA;
        }

        if(m_strDevName == "ZT"){
            if(wExportItemType == WFS_PIN_EXPORT_PUBLIC_KEY &&
               (lpSignedItem->lpsName == nullptr || !strlen(lpSignedItem->lpsName))){
                return WFS_ERR_INVALID_DATA;
            }

            if(m_bRSAUseSM2Key){
                //命令执行
                long lRet;
                char szDataHex[129] = {0};
                QByteArray dataHex;
                QByteArray dataBcd;
                QByteArray signatureDataBcd;

                m_cExportRsaItem.Clear();
                if(wExportItemType == WFS_PIN_EXPORT_PUBLIC_KEY){
                    PINKEYDATA stPKeyData;
                    QByteArray strPKeyName = lpSignedItem->lpsName;
                    if(m_cKey.FindSM2Key(strPKeyName, stPKeyData) == false){
                        //生成SM2密钥对
                        if(m_cKey.FindSM2Key(EPP_SM2_PRIVATE_KEY_NAME, stPKeyData)){
                            m_cKey.DeleteSM2Key(EPP_SM2_PRIVATE_KEY_NAME);
                        }

                        PINKEYDATA stSKeyData;
                        strcpy(stSKeyData.szName, EPP_SM2_PRIVATE_KEY_NAME);
                        stSKeyData.uNum = 0x05;
                        stSKeyData.dwType = 4;

                        strcpy(stPKeyData.szName, strPKeyName.constData());
                        stPKeyData.uNum = 0x06;
                        stPKeyData.dwType = 4;

                        lRet = m_pDev->SM2GenerateKey(stSKeyData.uNum, stPKeyData.uNum);
                        if(lRet != 0){
                            Log(ThisModule, __LINE__, "生成SM2密钥对失败");
                            return WFS_ERR_HARDWARE_ERROR;
                        }

                        //保存密钥数据
                        m_cKey.AddKey(stSKeyData);
                        m_cKey.AddKey(stPKeyData);
                    }

                    //导出公钥
                    lRet = m_pDev->SM2ExportPKey(stPKeyData.uNum, 0xFF, nullptr,  szDataHex, nullptr);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "导出EPP　SM2公钥失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }

                    //结果数据拷贝
                    CAutoHex::Hex2Bcd(szDataHex, dataBcd);
                    m_cExportRsaItem.AddBaseData(WFS_PIN_SIGN_NA);
                    m_cExportRsaItem.AddValueData((LPBYTE)dataBcd.constData(), dataBcd.size());
                } else {
                    //Todo Rsa random ?
                    //获取随机数
                    lRet = m_pDev->RSAExportEppRandData(dataHex);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "生成随机数失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }

                    //计算ZA值
                    QByteArray zaDataHex;
                    lRet = m_pDev->SM3CaculateZAData(SM2_USER_ID_DATA, 0x06, zaDataHex);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "生成ZA数据失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }

                    //获取签名数据
                    char cSignatureHex[129] = {0};
                    lRet = m_pDev->SM2SignData(0x05, zaDataHex.constData(), dataHex.constData(), cSignatureHex);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "获取签名数据失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }

                    //结果数据拷贝
                    m_cExportRsaItem.AddBaseData(WFS_PIN_SIGN_SM2);
                    CAutoHex::Hex2Bcd(dataHex, dataBcd);
                    m_cExportRsaItem.AddValueData((LPBYTE)dataBcd.constData(), dataBcd.size());
                    CAutoHex::Hex2Bcd(cSignatureHex, signatureDataBcd);
                    m_cExportRsaItem.AddSignatureData((LPBYTE)signatureDataBcd.constData(), signatureDataBcd.size());
                }
            } else {
                //命令执行
                long lRet;
                QByteArray dataHex;
                QByteArray signatureDataHex;
                QByteArray dataBcd;
                QByteArray signatureDataBcd;
                RSASIGNALGOTYPE eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;         //固定为该算法

                m_cExportRsaItem.Clear();
                if(wExportItemType == WFS_PIN_EXPORT_PUBLIC_KEY){
                    PINKEYDATA stPKeyData;
                    PINKEYDATA stSKeyData;
                    QByteArray strPKeyName = lpSignedItem->lpsName;
                    if(m_cKey.FindRSAKey(strPKeyName, stPKeyData) == false){
                        return WFS_ERR_PIN_KEYNOTFOUND;
                    }

                    strPKeyName = strPKeyName + KEY_RSA_SKEY_SUFFIX;
                    if(m_cKey.FindRSAKey(strPKeyName, stSKeyData) == false){
                        return WFS_ERR_PIN_KEYNOTFOUND;
                    }

                    //导出公钥
                    lRet = m_pDev->RSAExportEppEncryptPK(stPKeyData.uNum, stSKeyData.uNum, eRSASignAlgo, dataHex, signatureDataHex);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "导出EPP　RSA公钥失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }
                } else {
                    //获取随机数
                    //签名私钥号获取方法
                    PINKEYDATA stSKey;
    //                string strSKeyName = m_strRSAKeyPairName + KEY_RSA_SKEY_SUFFIX;
    //                if(m_cKey.FindRSAKey(strSKeyName.c_str(), stSKey) == false){
    //                    return WFS_ERR_PIN_KEYNOTFOUND;
    //                }
                    //签名私钥使用EPP预置密钥
                    eRSASignAlgo = RSA_SIGN_NO;
                    stSKey.uNum = 0x24;

                    lRet = m_pDev->RSAExportEppSN(stSKey.uNum, eRSASignAlgo, dataHex, signatureDataHex);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "生成随机数失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }
                }

                //结果数据拷贝
                DWORD dwRSASignAlgo = WFS_PIN_SIGN_NA;
                if(eRSASignAlgo == RSA_SIGN_SSA_PKCS1_V1_5){
                    dwRSASignAlgo = WFS_PIN_SIGN_RSASSA_PKCS1_V1_5;
                } else if(eRSASignAlgo == RSA_SIGN_SSA_PSS){
                    dwRSASignAlgo = WFS_PIN_SIGN_RSASSA_PSS;
                }
                CAutoHex::Hex2Bcd(dataHex.constData(), dataBcd);
                CAutoHex::Hex2Bcd(signatureDataHex.constData(), signatureDataBcd);
                m_cExportRsaItem.AddBaseData(dwRSASignAlgo);
                m_cExportRsaItem.AddValueData((LPBYTE)dataBcd.constData(), dataBcd.size());
                m_cExportRsaItem.AddSignatureData((LPBYTE)signatureDataBcd.constData(), signatureDataBcd.size());
            }

            lpSignedItemOutput = m_cExportRsaItem.GetData();
        } else {
            if(m_bRSAUseSM2Key){
                return WFS_ERR_UNSUPP_COMMAND;
            } else {
                //命令执行
                long lRet;
                QByteArray dataHex;
                QByteArray signatureDataHex;
                QByteArray dataBcd;
                QByteArray signatureDataBcd;
                RSASIGNALGOTYPE eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;         //要求使用的签名算法

                m_cExportRsaItem.Clear();
                if(wExportItemType == WFS_PIN_EXPORT_PUBLIC_KEY){
                    //导出公钥为预置公钥(默认密钥号为0x02)，公钥由签名中心私钥签名．因非主动导入，不判断公钥名称
                    lRet = m_pDev->RSAExportEppEncryptPK(EPP_CFES_EPP_RSA_KEY_NO, 0xFF, eRSASignAlgo, dataHex, signatureDataHex);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "导出EPP RSA公钥失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }
                } else {
                    //指定使用的签名算法
                    eRSASignAlgo = RSA_SIGN_NO;

                    //预置私钥进行签名
                    lRet = m_pDev->RSAExportEppSN(0xFF, eRSASignAlgo, dataHex, signatureDataHex);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "生成随机数失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }
                }

                //结果数据拷贝
                DWORD dwRSASignAlgo = WFS_PIN_SIGN_NA;
                if(eRSASignAlgo == RSA_SIGN_SSA_PKCS1_V1_5){
                    dwRSASignAlgo = WFS_PIN_SIGN_RSASSA_PKCS1_V1_5;
                } else if(eRSASignAlgo == RSA_SIGN_SSA_PSS){
                    dwRSASignAlgo = WFS_PIN_SIGN_RSASSA_PSS;
                }
                CAutoHex::Hex2Bcd(dataHex.constData(), dataBcd);
                CAutoHex::Hex2Bcd(signatureDataHex.constData(), signatureDataBcd);
                m_cExportRsaItem.AddBaseData(dwRSASignAlgo);
                m_cExportRsaItem.AddValueData((LPBYTE)dataBcd.constData(), dataBcd.size());
                m_cExportRsaItem.AddSignatureData((LPBYTE)signatureDataBcd.constData(), signatureDataBcd.size());
            }

            lpSignedItemOutput = m_cExportRsaItem.GetData();
        }
    } else {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

//30-00-00-00(FS#0005)
HRESULT CXFS_PIN::ImportRSASignedDesKey(const LPWFSPINIMPORTRSASIGNEDDESKEY lpSignedDESKey, LPWFSPINIMPORTRSASIGNEDDESKEYOUTPUT &lpSignedDESKeyOutput)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(m_strDevName == "ZT" || m_strDevName == "CFES") {
        QByteArray value;
        QByteArray signature;
        PINKEYDATA stSKEpp;
        PINKEYDATA stPKHost;
        DWORD dwUse = lpSignedDESKey->dwUse;

        //参数检查
        if((lpSignedDESKey->lpsKey == nullptr) || (strlen(lpSignedDESKey->lpsKey) == 0)){
            return WFS_ERR_INVALID_DATA;
        }

        if(dwUse == 0){
            //删除SM4密钥
            PINKEYDATA stKeySM4;
            stKeySM4 = m_cKey.FindKey(lpSignedDESKey->lpsKey, false);
            if(!m_cKey.IsInvalidKey(stKeySM4)){
                m_cKey.DeleteKey(lpSignedDESKey->lpsKey, false);
            }
            return WFS_SUCCESS;
        }

        if((lpSignedDESKey->lpxValue == nullptr) ||
           (lpSignedDESKey->lpxValue->usLength == 0) || (lpSignedDESKey->lpxValue->lpbData == nullptr)){
            return WFS_ERR_PIN_KEYNOVALUE;
        }

        if(m_strDevName == "ZT"){
            if(m_bRSAUseSM2Key){
                //查找EPP SM2私钥
                if(lpSignedDESKey->lpsDecryptKey != nullptr && strlen(lpSignedDESKey->lpsDecryptKey) > 0){
                    if(m_cKey.FindSM2Key(EPP_SM2_PRIVATE_KEY_NAME, stSKEpp) == false){
                        Log(ThisModule, __LINE__, "EPP SM2私钥未找到");
                        return WFS_ERR_PIN_KEYNOTFOUND;
                    }
                }

                //验签参数
                stPKHost.uNum = 0xFF;
                if(lpSignedDESKey->dwRSASignatureAlgorithm != WFS_PIN_SIGN_NA){
                    //查找主机SM2公钥
                    if((lpSignedDESKey->lpsSigKey != nullptr) && (strlen(lpSignedDESKey->lpsSigKey) > 0)){
                        if(m_cKey.FindSM2Key(lpSignedDESKey->lpsSigKey, stPKHost) == false){
                            Log(ThisModule, __LINE__, "HOST SM2公钥未找到");
                            return WFS_ERR_PIN_KEYNOTFOUND;
                        }
                    }

                    if(lpSignedDESKey->lpxSignature == nullptr || lpSignedDESKey->lpxSignature->lpbData == nullptr ||
                       lpSignedDESKey->lpxSignature->usLength == 0){
                        Log(ThisModule, __LINE__, "签名数据为空");
                        return WFS_ERR_PIN_INVALIDDATA;
                    }
                    CAutoHex::Bcd2Hex(lpSignedDESKey->lpxSignature->lpbData, lpSignedDESKey->lpxSignature->usLength, signature);
                }

                CAutoHex::Bcd2Hex(lpSignedDESKey->lpxValue->lpbData, lpSignedDESKey->lpxValue->usLength, value);

                vectorUINT vtUse;
                if(!SplitKeyUse(dwUse, vtUse)) {
                    Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", dwUse, lpSignedDESKey->lpsKey);
                    return WFS_ERR_INVALID_DATA;
                }

                //命令执行
                long lRet;
                //删除同名密钥
                QByteArray strKeyName = lpSignedDESKey->lpsKey;
                m_cKey.DeleteKey(strKeyName, false);

                //计算ZA
                QByteArray AZData;
                if(signature.size() > 0){
                    lRet = m_pDev->SM3CaculateZAData(SM2_USER_ID_DATA, stPKHost.uNum, AZData);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "验签时计算ZA数据失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }
                }

                //根据Key用途逐个下载
                PINKEYDATA stKey;
                for (auto &it : vtUse)
                {
                    QByteArray strUseKeyName;
                    if (it == PKU_CRYPT)
                        strUseKeyName = strKeyName + KEY_SM4_CRYPT;
                    else if (it == PKU_FUNCTION)
                        strUseKeyName = strKeyName + KEY_SM4_FUNCTION;
                    else if (it == PKU_MACING)
                        strUseKeyName = strKeyName + KEY_SM4_MACING;
                    else if (it == PKU_KEYENCKEY)
                        strUseKeyName = strKeyName;
                    else
                    {
                        Log(ThisModule, __LINE__, "无效或不支持工作密钥用途：%02X[%s]", it, strKeyName.constData());
                        return WFS_ERR_INVALID_DATA;
                    }

                    if(it == PKU_KEYENCKEY){
                        stKey = m_cKey.FindMKeySlot(strUseKeyName, "", false);
                    } else {
                        stKey = m_cKey.FindWKeySlot(strUseKeyName, false, it);
                    }

                    if (m_cKey.IsMaxKey(stKey))
                    {
                        Log(ThisModule, __LINE__, "工作密钥已下载满：%s", strKeyName.constData());
                        return WFS_ERR_PIN_NOKEYRAM;
                    }

                    //下载国密密钥
                    char szKcvHex[64] = {0};
                    lRet = m_pDev->SM4RemoteImportKey(stKey.uNum, stSKEpp.uNum, stPKHost.uNum, it,
                                                      AZData.constData(), value.constData(), signature.constData(), szKcvHex);
                    if(lRet != 0){
                        //删除已下载属性密钥
                        m_cKey.DeleteKey(strKeyName, false);
                        Log(ThisModule, __LINE__, "远程导入SM4密钥失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }

                    //保存密钥
                    strcpy(stKey.szKCV, szKcvHex);
                    m_cKey.AddKey(stKey);
                }

                //结果数据拷贝
                QByteArray kcvBcd;
                CAutoHex::Hex2Bcd(stKey.szKCV, kcvBcd);
                m_cImportRsaSignedDesKey.Clear();
                m_cImportRsaSignedDesKey.AddBaseData(WFS_PIN_KEYDOUBLE, WFS_PIN_KCVZERO);
                m_cImportRsaSignedDesKey.Add((LPBYTE)kcvBcd.constData(), kcvBcd.size());
                lpSignedDESKeyOutput = m_cImportRsaSignedDesKey.GetData();
            } else {
                long lRet = ERR_SUCCESS;
                PINKEYDATA stPKEpp;
                stPKEpp.uNum = 0xFF;
                //查找EPP RSA私钥
                if(lpSignedDESKey->lpsDecryptKey != nullptr && strlen(lpSignedDESKey->lpsDecryptKey) > 0){
                    string strDecryptKeyName = strDecryptKeyName + lpSignedDESKey->lpsDecryptKey + KEY_RSA_SKEY_SUFFIX;
                    if(m_cKey.FindRSAKey(strDecryptKeyName.c_str(), stSKEpp) == false){
                        Log(ThisModule, __LINE__, "EPP SM2私钥未找到");
                        return WFS_ERR_PIN_KEYNOTFOUND;
                    }
                    //查找EPP RSA公钥
                    m_cKey.FindRSAKey(strDecryptKeyName.c_str(), stPKEpp);
                } else {
                    stSKEpp.uNum = 0xFF;
                }

                //验签参数
                stPKHost.uNum = 0xFF;
                if(lpSignedDESKey->dwRSASignatureAlgorithm != WFS_PIN_SIGN_NA){
                    //查找主机RSA公钥
                    if((lpSignedDESKey->lpsSigKey != nullptr) && (strlen(lpSignedDESKey->lpsSigKey) > 0)){
                        if(m_cKey.FindRSAKey(lpSignedDESKey->lpsSigKey, stPKHost) == false){
                            Log(ThisModule, __LINE__, "HOST SM2公钥未找到");
                            return WFS_ERR_PIN_KEYNOTFOUND;
                        }
                    } else {
                        return WFS_ERR_PIN_INVALIDDATA;
                    }

                    if(lpSignedDESKey->lpxSignature == nullptr || lpSignedDESKey->lpxSignature->lpbData == nullptr ||
                       lpSignedDESKey->lpxSignature->usLength == 0){
                        Log(ThisModule, __LINE__, "签名数据为空");
                        return WFS_ERR_PIN_INVALIDDATA;
                    }

                    CAutoHex::Bcd2Hex(lpSignedDESKey->lpxSignature->lpbData, lpSignedDESKey->lpxSignature->usLength, signature);
                } else {
                    stPKHost.uNum = 0xFF;
                }
                RSADATAALGOTYPE eRSADataAlgo = RSA_ENC_SHA_256;
                if(lpSignedDESKey->dwRSAEncipherAlgorithm == WFS_PIN_CRYPT_RSAES_PKCS1_V1_5){
                    eRSADataAlgo = RSA_ENC_PKCS1_V1_5;
                } else if(lpSignedDESKey->dwRSAEncipherAlgorithm == WFS_PIN_CRYPT_RSAES_OAEP) {
                    eRSADataAlgo = RSA_ENC_SHA_1;
                }
                RSASIGNALGOTYPE eRSASignAlgo = RSA_SIGN_NO;
                if(lpSignedDESKey->dwRSASignatureAlgorithm == WFS_PIN_SIGN_RSASSA_PKCS1_V1_5){
                    eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;
                } else if (lpSignedDESKey->dwRSASignatureAlgorithm == WFS_PIN_SIGN_RSASSA_PSS){
                    eRSASignAlgo = RSA_SIGN_SSA_PSS;
                }

                CAutoHex::Bcd2Hex(lpSignedDESKey->lpxValue->lpbData, lpSignedDESKey->lpxValue->usLength, value);

                //固定为主密钥，忽略传入参数

                //命令执行
                QByteArray strKeyName = lpSignedDESKey->lpsKey;

                PINKEYDATA stKey;
                stKey = m_cKey.FindMKeySlot(strKeyName);
                if (m_cKey.IsMaxKey(stKey))
                {
                    Log(ThisModule, __LINE__, "工作密钥已下载满：%s", strKeyName.constData());
                    return WFS_ERR_PIN_NOKEYRAM;
                }

                //加密的密钥数据处理
                QByteArray formatedKeyData;
                QByteArray keyData;
                formatedKeyData.append((LPSTR)lpSignedDESKey->lpxValue->lpbData, lpSignedDESKey->lpxValue->usLength);
                lRet = AnalyKeyDataFromFormatedData(stPKEpp.uNum, stSKEpp.uNum, eRSADataAlgo, formatedKeyData, keyData);
                if(lRet != ERR_SUCCESS){
                    if(lRet == ERR_PARAM){
                        return WFS_ERR_PIN_INVALIDDATA;
                    } else {
                        return WFS_ERR_HARDWARE_ERROR;
                    }
                }

                //下载DES主密钥
                QByteArray vtKcvHex;
                lRet = m_pDev->RSAImportDesKey(stKey.uNum, stSKEpp.uNum, stPKHost.uNum, eRSADataAlgo, eRSASignAlgo,
                                                    keyData, signature, vtKcvHex, m_bRSARandomMode);
                if(lRet != 0){
                    Log(ThisModule, __LINE__, "远程导入DES密钥失败");
                    return WFS_ERR_HARDWARE_ERROR;
                }

                //保存密钥
                strcpy(stKey.szKCV, vtKcvHex.constData());
                m_cKey.AddKey(stKey);

                //结果数据拷贝
                QByteArray kcvBcd;
                CAutoHex::Hex2Bcd(stKey.szKCV, kcvBcd);
                m_cImportRsaSignedDesKey.Clear();
                m_cImportRsaSignedDesKey.AddBaseData(WFS_PIN_KEYDOUBLE, WFS_PIN_KCVZERO);
                m_cImportRsaSignedDesKey.Add((LPBYTE)kcvBcd.constData(), kcvBcd.size());
                lpSignedDESKeyOutput = m_cImportRsaSignedDesKey.GetData();
            }
        } else {
            if(m_bRSAUseSM2Key){
                return WFS_ERR_UNSUPP_COMMAND;
            } else {
                long lRet = ERR_SUCCESS;
                stSKEpp.uNum = EPP_CFES_EPP_RSA_KEY_NO;
                //查找EPP RSA私钥
                if(lpSignedDESKey->lpsDecryptKey != nullptr && strlen(lpSignedDESKey->lpsDecryptKey) > 0){
                    stSKEpp = m_cKeyXZ.FindAsymmKey(lpSignedDESKey->lpsDecryptKey, 5);
                    if(m_cKeyXZ.IsInvalidKey(stSKEpp)){
                        Log(ThisModule, __LINE__, "EPP RSA私钥未找到");
                        stSKEpp.uNum = EPP_CFES_EPP_RSA_KEY_NO;
                    }
                }

                //验签参数
                stPKHost.uNum = 0xFF;
                if(lpSignedDESKey->dwRSASignatureAlgorithm != WFS_PIN_SIGN_NA){
                    //查找主机RSA公钥
                    if((lpSignedDESKey->lpsSigKey != nullptr) && (strlen(lpSignedDESKey->lpsSigKey) > 0)){
                        stPKHost = m_cKeyXZ.FindAsymmKey(lpSignedDESKey->lpsSigKey, 5);
                        if(m_cKeyXZ.IsInvalidKey(stPKHost)){
                            Log(ThisModule, __LINE__, "银行后台RSA公钥未找到");
                            return WFS_ERR_PIN_KEYNOTFOUND;
                        }
                    } else {
                        return WFS_ERR_PIN_INVALIDDATA;
                    }

                    if(lpSignedDESKey->lpxSignature == nullptr || lpSignedDESKey->lpxSignature->lpbData == nullptr ||
                       lpSignedDESKey->lpxSignature->usLength == 0){
                        Log(ThisModule, __LINE__, "签名数据为空");
                        return WFS_ERR_PIN_INVALIDDATA;
                    }

                    CAutoHex::Bcd2Hex(lpSignedDESKey->lpxSignature->lpbData, lpSignedDESKey->lpxSignature->usLength, signature);
                } else {
                    stPKHost.uNum = 0xFF;
                }
                RSADATAALGOTYPE eRSADataAlgo = RSA_ENC_SHA_256;
                if(lpSignedDESKey->dwRSAEncipherAlgorithm == WFS_PIN_CRYPT_RSAES_PKCS1_V1_5){
                    eRSADataAlgo = RSA_ENC_PKCS1_V1_5;
                } else if(lpSignedDESKey->dwRSAEncipherAlgorithm == WFS_PIN_CRYPT_RSAES_OAEP) {
                    eRSADataAlgo = RSA_ENC_SHA_1;
                }
                RSASIGNALGOTYPE eRSASignAlgo = RSA_SIGN_NO;
                if(lpSignedDESKey->dwRSASignatureAlgorithm == WFS_PIN_SIGN_RSASSA_PKCS1_V1_5){
                    eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;
                } else if (lpSignedDESKey->dwRSASignatureAlgorithm == WFS_PIN_SIGN_RSASSA_PSS){
                    eRSASignAlgo = RSA_SIGN_SSA_PSS;
                }

                CAutoHex::Bcd2Hex(lpSignedDESKey->lpxValue->lpbData, lpSignedDESKey->lpxValue->usLength, value);

                //固定为主密钥，忽略传入参数

                //命令执行
                QByteArray strKeyName = lpSignedDESKey->lpsKey;

                PINKEYDATA stKey;
                if(m_iRSARemoteImportSymmKeyType == 0){
                    stKey = m_cKeyXZ.FindKeySlot(strKeyName);
                } else {
                    stKey = m_cKeyXZ.FindSMKeySlot(strKeyName);
                }
                if (m_cKey.IsMaxKey(stKey))
                {
                    Log(ThisModule, __LINE__, "主密钥已下载满：%s", strKeyName.constData());
                    return WFS_ERR_PIN_NOKEYRAM;
                }

                stKey.dwUse = lpSignedDESKey->dwUse;
                //加密的密钥数据处理
//                QByteArray formatedKeyData;
//                QByteArray keyData;
//                formatedKeyData.append((LPSTR)lpSignedDESKey->lpxValue->lpbData, lpSignedDESKey->lpxValue->usLength);
//                lRet = AnalyKeyDataFromFormatedData(stPKEpp.uNum, stSKEpp.uNum, eRSADataAlgo, formatedKeyData, keyData);
//                if(lRet != ERR_SUCCESS){
//                    if(lRet == ERR_PARAM){
//                        return WFS_ERR_PIN_INVALIDDATA;
//                    } else {
//                        return WFS_ERR_HARDWARE_ERROR;
//                    }
//                }

                //下载DES主密钥
                QByteArray vtKcvHex;
                lRet = m_pDev->RSAImportDesKey(stKey.uNum, stSKEpp.uNum, stPKHost.uNum, eRSADataAlgo, eRSASignAlgo,
                                               value, signature, vtKcvHex, m_bRSARandomMode,
                                               m_iRSARemoteImportSymmKeyType);
                if(lRet != 0){
                    Log(ThisModule, __LINE__, "远程导入对称密钥失败");
                    return WFS_ERR_HARDWARE_ERROR;
                }

                //保存密钥
                strcpy(stKey.szKCV, vtKcvHex.constData());
                m_cKeyXZ.AddKey(stKey);

                //结果数据拷贝
                QByteArray kcvBcd;
                CAutoHex::Hex2Bcd(stKey.szKCV, kcvBcd);
                m_cImportRsaSignedDesKey.Clear();
                m_cImportRsaSignedDesKey.AddBaseData(WFS_PIN_KEYDOUBLE, WFS_PIN_KCVZERO);
                m_cImportRsaSignedDesKey.Add((LPBYTE)kcvBcd.constData(), kcvBcd.size());
                lpSignedDESKeyOutput = m_cImportRsaSignedDesKey.GetData();
            }
        }
    } else {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

//30-00-00-00(FS#0006)
HRESULT CXFS_PIN::GenerateRSAKeyPair(const LPWFSPINGENERATERSAKEYPAIR lpGenerateRSAKeyPair)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    if(m_strDevName == "XZ"){
        return WFS_ERR_UNSUPP_COMMAND;
    } else if(m_strDevName == "ZT") {
        if(m_bRSAUseSM2Key){
            return WFS_ERR_UNSUPP_COMMAND;
        } else {
            //参数检查
            if(lpGenerateRSAKeyPair->lpsKey == nullptr || (strlen(lpGenerateRSAKeyPair->lpsKey) == 0)){
                return WFS_ERR_PIN_INVALIDDATA;
            }
            if(lpGenerateRSAKeyPair->dwUse != WFS_PIN_USERSAPRIVATE && lpGenerateRSAKeyPair->dwUse != WFS_PIN_USERSAPRIVATESIGN){
                return WFS_ERR_PIN_USEVIOLATION;
            }
            if(lpGenerateRSAKeyPair->wModulusLength != 1024 &&
               lpGenerateRSAKeyPair->wModulusLength != 2048){
                return WFS_ERR_PIN_INVALID_MOD_LEN;
            }

            if(lpGenerateRSAKeyPair->wExponentValue != WFS_PIN_DEFAULT &&
               lpGenerateRSAKeyPair->wExponentValue != WFS_PIN_EXPONENT_1 &&
               lpGenerateRSAKeyPair->wExponentValue != WFS_PIN_EXPONENT_4 &&
               lpGenerateRSAKeyPair->wExponentValue != WFS_PIN_EXPONENT_16){
                Log(ThisModule, __LINE__, "生成rsa密钥对指定的幂指数参数错误：%d", lpGenerateRSAKeyPair->wExponentValue);
                return WFS_ERR_PIN_INVALIDDATA;
            }

            int iExponent;
            switch(lpGenerateRSAKeyPair->wExponentValue){
            case WFS_PIN_EXPONENT_4:
                iExponent = 17;
                break;
            case WFS_PIN_EXPONENT_16:
                iExponent = 65537;
                break;
            default:
                iExponent = 3;
                break;
            }

            PINKEYDATA stSKeyData;
            PINKEYDATA stPKeyData;

            string strSKeyName = strSKeyName + lpGenerateRSAKeyPair->lpsKey + KEY_RSA_SKEY_SUFFIX;
            stSKeyData = m_cKey.FindRSAKeySlot(strSKeyName.c_str(), false);
            if(m_cKey.IsMaxKey(stSKeyData)){
                return WFS_ERR_PIN_NOKEYRAM;
            }

            stPKeyData = m_cKey.FindRSAKeySlot(lpGenerateRSAKeyPair->lpsKey);
            if(m_cKey.IsMaxKey(stPKeyData)){
                return WFS_ERR_PIN_NOKEYRAM;
            }

            long lRet = m_pDev->RSAGenerateKeyPair(stPKeyData.uNum, stSKeyData.uNum,
                                          lpGenerateRSAKeyPair->wModulusLength,
                                          iExponent);
            if(lRet != 0){
                return WFS_ERR_HARDWARE_ERROR;
            }

            //保存密钥对名称
            m_strRSAKeyPairName = lpGenerateRSAKeyPair->lpsKey;
            //保存密钥
            m_cKey.AddKey(stSKeyData);
            m_cKey.AddKey(stPKeyData);
        }
    } else {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

HRESULT CXFS_PIN::ExportRSAEppSignedItem(const LPWFSPINEXPORTRSAEPPSIGNEDITEM lpEppSignedItem, LPWFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT &lpEppSignedItemOutput)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    m_cExportRsaEppSignedItem.Clear();

    if(m_strDevName == "ZT") {
        PINKEYDATA stSKeyData;
        stSKeyData.uNum = 0xFF;
        RSASIGNALGOTYPE eRSASignAlgoReq = RSA_SIGN_NO;
        RSASIGNALGOTYPE eRSASignAlgo = RSA_SIGN_NO;
        WORD wExportItemType = lpEppSignedItem->wExportItemType;
        //参数检查
        if(wExportItemType != WFS_PIN_EXPORT_EPP_ID && wExportItemType != WFS_PIN_EXPORT_PUBLIC_KEY){
            return WFS_ERR_INVALID_DATA;
        }

        if(wExportItemType == WFS_PIN_EXPORT_PUBLIC_KEY &&
           (lpEppSignedItem->lpsName == nullptr || !strlen(lpEppSignedItem->lpsName))){
            return WFS_ERR_INVALID_DATA;
        }

        //判断签名私钥是否存在
        if(lpEppSignedItem->dwSignatureAlgorithm != WFS_PIN_SIGN_NA){
            if((lpEppSignedItem->lpsSigKey != nullptr) && (strlen(lpEppSignedItem->lpsSigKey) > 0)){
                bool bSignKeyExist = false;
                if(m_strDevName == "ZT"){
                    bSignKeyExist = m_cKey.FindRSAKey(lpEppSignedItem->lpsSigKey, stSKeyData);
                } else {
                    stSKeyData = m_cKeyXZ.FindAsymmKey(lpEppSignedItem->lpsSigKey, 5);
                    bSignKeyExist = !m_cKeyXZ.IsInvalidKey(stSKeyData);
                }
                if(!bSignKeyExist){
                    Log(ThisModule, __LINE__, "签名密钥不存在:SigKey:%s", lpEppSignedItem->lpsSigKey);
                    return WFS_ERR_PIN_NORSAKEYPAIR;
                }
            }
        }

        switch(lpEppSignedItem->dwSignatureAlgorithm){
        case WFS_PIN_SIGN_RSASSA_PKCS1_V1_5:
            eRSASignAlgo = RSA_SIGN_SSA_PKCS1_V1_5;
            break;
        case WFS_PIN_SIGN_RSASSA_PSS:
            eRSASignAlgo = RSA_SIGN_SSA_PSS;
            break;
        case WFS_PIN_SIGN_NA:
        default:
            break;
        }
        eRSASignAlgoReq = eRSASignAlgo;

        if(m_strDevName == "ZT"){
            if(m_bRSAUseSM2Key){
                return WFS_ERR_UNSUPP_COMMAND;
            } else {
                //命令执行
                long lRet;
                QByteArray dataHex;
                QByteArray signatureDataHex;
                QByteArray dataBcd;
                QByteArray signatureDataBcd;


                if(wExportItemType == WFS_PIN_EXPORT_PUBLIC_KEY){
                    PINKEYDATA stPKeyData;
                    QByteArray strPKeyName = lpEppSignedItem->lpsName;
                    if(m_cKey.FindRSAKey(strPKeyName, stPKeyData) == false){
                        return WFS_ERR_PIN_KEYNOTFOUND;
                    }

                    //导出公钥
                    lRet = m_pDev->RSAExportEppEncryptPK(stPKeyData.uNum, stSKeyData.uNum, eRSASignAlgo, dataHex, signatureDataHex);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "导出EPP　RSA公钥失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }
                } else {
                    //获取随机数
                    lRet = m_pDev->RSAExportEppSN(stSKeyData.uNum, eRSASignAlgo, dataHex, signatureDataHex);
                    if(lRet != 0){
                        Log(ThisModule, __LINE__, "生成随机数失败");
                        return WFS_ERR_HARDWARE_ERROR;
                    }
                }

                //结果数据拷贝
                CAutoHex::Hex2Bcd(dataHex.constData(), dataBcd);
                CAutoHex::Hex2Bcd(signatureDataHex.constData(), signatureDataBcd);
                m_cExportRsaEppSignedItem.AddValueData((LPBYTE)dataBcd.constData(), dataBcd.size());
                //如果要求签名算法和实际签名算法不一致，签名数据返回NULL
                if(eRSASignAlgoReq == eRSASignAlgo){
                    if(wExportItemType == WFS_PIN_EXPORT_PUBLIC_KEY){
                        m_cExportRsaEppSignedItem.AddSelfSignatureData((LPBYTE)signatureDataBcd.constData(), signatureDataBcd.size());
                    } else {
                        m_cExportRsaEppSignedItem.AddSignatureData((LPBYTE)signatureDataBcd.constData(), signatureDataBcd.size());
                    }
                }
            }
        } else {
            return WFS_ERR_UNSUPP_COMMAND;
//            if(m_bRSAUseSM2Key){
//                return WFS_ERR_UNSUPP_COMMAND;
//            } else {
//                //命令执行
//                long lRet;
//                QByteArray dataHex;
//                QByteArray signatureDataHex;
//                QByteArray dataBcd;
//                QByteArray signatureDataBcd;

//                if(wExportItemType == WFS_PIN_EXPORT_PUBLIC_KEY){
//                    PINKEYDATA stSKeyData;
//                    QByteArray strSKeyName = lpEppSignedItem->lpsSigKey;
//                    stSKeyData = m_cKeyXZ.FindAsymmKey(strSKeyName, 5);
//                    if(m_cKeyXZ.IsInvalidKey(stSKeyData)){
//                        return WFS_ERR_PIN_KEYNOTFOUND;
//                    }

//                    //导出公钥
//                    lRet = m_pDev->RSAExportEppEncryptPK(0x02, stSKeyData.uNum, eRSASignAlgo, dataHex, signatureDataHex);
//                    if(lRet != 0){
//                        Log(ThisModule, __LINE__, "导出EPP　RSA公钥失败");
//                        return WFS_ERR_HARDWARE_ERROR;
//                    }
//                } else {
//                    //获取随机数
//                    lRet = m_pDev->RSAExportEppSN(stSKeyData.uNum, eRSASignAlgo, dataHex, signatureDataHex);
//                    if(lRet != 0){
//                        Log(ThisModule, __LINE__, "生成随机数失败");
//                        return WFS_ERR_HARDWARE_ERROR;
//                    }
//                }

//                //结果数据拷贝
//                CAutoHex::Hex2Bcd(dataHex.constData(), dataBcd);
//                CAutoHex::Hex2Bcd(signatureDataHex.constData(), signatureDataBcd);
//                m_cExportRsaEppSignedItem.AddValueData((LPBYTE)dataBcd.constData(), dataBcd.size());
//                //如果要求签名算法和实际签名算法不一致，签名数据返回NULL
//                if(eRSASignAlgoReq == eRSASignAlgo){
////                    if(wExportItemType == WFS_PIN_EXPORT_PUBLIC_KEY){
////                        m_cExportRsaEppSignedItem.AddSelfSignatureData((LPBYTE)signatureDataBcd.constData(), signatureDataBcd.size());
////                    } else {
////                        m_cExportRsaEppSignedItem.AddSignatureData((LPBYTE)signatureDataBcd.constData(), signatureDataBcd.size());
////                    }
//                    m_cExportRsaEppSignedItem.AddSignatureData((LPBYTE)signatureDataBcd.constData(), signatureDataBcd.size());
//                }

//                //Todo:SelfSignature(暂时不支持)
//            }
        }

        lpEppSignedItemOutput = m_cExportRsaEppSignedItem.GetData();
    } else {
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}
//////////////////////////////////////////////////////////////////////////

bool CXFS_PIN::UpdateStatus(const DEVPINSTATUS &stStatus)
{
    AutoMutex(*m_pMutexGetStatus);// 必须加此互斥，防止同时读写数据问题
    WORD fwEncStat = WFS_PIN_ENCNOTREADY;
    WORD fwDevice  = WFS_PIN_DEVHWERROR;

    // 设备状态
    UpdateExtra(stStatus.szErrCode);
    switch (stStatus.wDevice)
    {
    case DEVICE_OFFLINE:
        fwDevice = WFS_PIN_DEVOFFLINE;
        break;
    case DEVICE_ONLINE:
        {
            fwDevice = WFS_PIN_DEVONLINE;
            if(m_strDevName == "XZ" || m_strDevName == "CFES"){
                if(m_cKeyXZ.IsEmpty()){
                    fwEncStat = WFS_PIN_ENCINITIALIZED;
                } else {
                    fwEncStat = WFS_PIN_ENCREADY;
                }
            } else if(m_strDevName == "ZT") {
                if (m_cKey.IsEmpty())
                    fwEncStat = WFS_PIN_ENCNOTINITIALIZED;
                else if (m_cKey.IsAllMKey())
                    fwEncStat = WFS_PIN_ENCINITIALIZED;
                else
                    fwEncStat = WFS_PIN_ENCREADY;
            } else {
                if (m_cKeyC90.IsEmpty())
                    fwEncStat = WFS_PIN_ENCNOTINITIALIZED;
                else if (m_cKeyC90.IsAllMKey())
                    fwEncStat = WFS_PIN_ENCINITIALIZED;
                else
                    fwEncStat = WFS_PIN_ENCREADY;
            }
        }
        break;
    default:
        break;
    }

    // 判断状态是否有变化
    if (m_stStatus.fwDevice != fwDevice)
    {
        m_pBase->FireStatusChanged(fwDevice);
        // 故障时，也要上报故障事件
        if (fwDevice == WFS_PIN_DEVHWERROR)
            m_pBase->FireHWErrorStatus(WFS_ERR_ACT_RESET, m_cExtra.GetErrDetail("ErrorDetail"));
    }
    m_stStatus.fwDevice = fwDevice;
    m_stStatus.fwEncStat = fwEncStat;

    //键盘为非法移除状态时，清空密钥
    if(stStatus.bIllegalRemove){
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            m_cKeyXZ.DeleteAllKey();
        } else if(m_strDevName == "ZT") {
            m_cKey.DeleteAllKey();
        } else {
            m_cKeyC90.DeleteAllKey();
        }
    }

    return true;
}

void CXFS_PIN::UpdateExtra(string strErrCode, string strDevVer /*= ""*/)
{
    if (strErrCode == "000")
        strErrCode.insert(0, "000000");// 没故障时显示
    else
        strErrCode.insert(0, "001909");// 固化的前六位

    m_cExtra.AddExtra("ErrorDetail", strErrCode.c_str());
    if (!strDevVer.empty())
    {
        char szSPVer[64] = { 0 };
        char szVer[64] = { "0001001" };
        sprintf(szSPVer, "%08sV%07s", "PIN_V303", szVer);
        m_cExtra.AddExtra("VRTCount", "2");
        m_cExtra.AddExtra("VRTDetail[00]", szSPVer);                // SP版本程序名称8位+版本8位
        m_cExtra.AddExtra("VRTDetail[01]", strDevVer.c_str());      // Firmware(1)版本程序名称8位+版本8位

        //特殊要求，增加UID字段
        string strUid = "0219-CMBC-TIMB1102-2021-";
        char szSN[9] = {0};
        char szTemp[MAX_PATH] = {0};
        char *pStart = (LPSTR)strrchr(strDevVer.c_str(), ':');
        char *pEnd = (LPSTR)strrchr(strDevVer.c_str(), ']');
        if(m_strDevName == "CFES"){
            if(strlen(pStart) > 8){
                pStart = pStart + (strlen(pStart) - 8);
            }
            pEnd = (LPSTR)strDevVer.c_str() + strDevVer.size();
        }
        if(pStart && pEnd){
            memcpy(szTemp, pStart + 1, pEnd - pStart - 1);
            memcpy(szSN, szTemp, 8);
        } else {
            strcpy(szSN, "00000000");
        }
        strUid.append(szSN);
        m_cExtra.AddExtra("UID", strUid.c_str());
    }
}

bool CXFS_PIN::IsStatusOK()
{
    return (m_stStatus.fwDevice == WFS_PIN_DEVONLINE);
}

ULONG CXFS_PIN::GetFunctionKeys()
{
//30-00-00-00(FS#0010)    ULONG ulKeys = WFS_PIN_FK_1 | WFS_PIN_FK_2 | WFS_PIN_FK_3 | WFS_PIN_FK_CANCEL | WFS_PIN_FK_4 | WFS_PIN_FK_5 | WFS_PIN_FK_6 | WFS_PIN_FK_CLEAR |
    ULONG ulKeys = WFS_PIN_FK_1 | WFS_PIN_FK_2 | WFS_PIN_FK_3 | WFS_PIN_FK_CANCEL | WFS_PIN_FK_4 | WFS_PIN_FK_5 | WFS_PIN_FK_6 |            //30-00-00-00(FS#0010)
                   (m_iClearKeyMode == 0 ? WFS_PIN_FK_CLEAR : (m_iClearKeyMode == 1 ? WFS_PIN_FK_BACKSPACE : (WFS_PIN_FK_CLEAR | WFS_PIN_FK_BACKSPACE))) |                                                               //30-00-00-00(FS#0010)
                   WFS_PIN_FK_7 | WFS_PIN_FK_8 | WFS_PIN_FK_9 | WFS_PIN_FK_ENTER | WFS_PIN_FK_DECPOINT | WFS_PIN_FK_0 | WFS_PIN_FK_00;
    return ulKeys;
}

ULONG CXFS_PIN::GetFDKs(LPWFSPINFDK pszstFDK)
{
    // 杭州银行要求增加特殊F10和F12支持
    ULONG ulFDKKey = 0;
    WFSPINFDK stFDK[FDK_MAX_NUM] = {\
        {WFS_PIN_FK_FDK01, 1, 1}, {WFS_PIN_FK_FDK02, 1, 2}, {WFS_PIN_FK_FDK03, 2, 1}, {WFS_PIN_FK_FDK04, 2, 2},
        {WFS_PIN_FK_FDK05, 3, 1}, {WFS_PIN_FK_FDK06, 3, 2}, {WFS_PIN_FK_FDK07, 4, 1}, {WFS_PIN_FK_FDK08, 4, 2},
    };

    for (UINT i = 0; i < sizeof(stFDK) / sizeof(stFDK[0]); i++)
    {
        ulFDKKey |= stFDK[i].ulFDK;
    }
    if (pszstFDK != nullptr)
        memcpy(pszstFDK, stFDK, sizeof(stFDK));
    return ulFDKKey;
}

bool CXFS_PIN::IsSupportKeyUse(DWORD dwUse)
{
    // 判断用途
    if (dwUse & WFS_PIN_USECRYPT)
        return true;
    if (dwUse & WFS_PIN_USEFUNCTION)
        return true;
    if (dwUse & WFS_PIN_USEMACING)
        return true;
    if (dwUse & WFS_PIN_USEKEYENCKEY)
        return true;
    if (dwUse & WFS_PIN_USESVENCKEY)
        return true;
    if (dwUse & WFS_PIN_USESM4)
        return true;
    return false;
}

UINT CXFS_PIN::ConvertKeyUse(DWORD dwUse)
{
    // 判断用途
    UINT uKeyUse = 0;
    if (dwUse & WFS_PIN_USECRYPT)
        uKeyUse |= PKU_CRYPT;
    if (dwUse & WFS_PIN_USEFUNCTION)
        uKeyUse |= PKU_FUNCTION;
    if (dwUse & WFS_PIN_USEMACING)
        uKeyUse |= PKU_MACING;
    if (dwUse & WFS_PIN_USEKEYENCKEY)
        uKeyUse |= PKU_KEYENCKEY;
//    if (dwUse & WFS_PIN_USESVENCKEY)
//        uKeyUse |= PKU_SVENCKEY;
    return uKeyUse;
}

ULONG CXFS_PIN::ConvertKeyVal(BYTE byKeyVal, bool &bFDK)
{
    for (UINT i = 0; i < sizeof(g_stFuncKey) / sizeof(g_stFuncKey[0]); i++)
    {
        if (byKeyVal == g_stFuncKey[i].byKey)
        {
            bFDK = false;
            if(g_stFuncKey[i].ulVal == WFS_PIN_FK_CLEAR && m_iClearKeyMode == 1){        //30-00-00-00(FS#0010)
                return WFS_PIN_FK_BACKSPACE;        //30-00-00-00(FS#0010)
            }                                       //30-00-00-00(FS#0010)
            return g_stFuncKey[i].ulVal;
        }
    }
    for (UINT k = 0; k < sizeof(g_stFDKKey) / sizeof(g_stFDKKey[0]); k++)
    {
        if (byKeyVal == g_stFDKKey[k].byKey)
        {
            bFDK = true;
            return g_stFDKKey[k].ulVal;
        }
    }
    return WFS_PIN_FK_UNUSED;
}

long CXFS_PIN::GetKeyPress(vectorWFSPINKEY &vtKey, CInputDataHelper &cInput, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CAutoCloseInput _autoclose(m_pDev, cInput.IsGetPin() ? true : false);

    WFSPINKEY stKey;
    EPP_KEYVAL stKeyVal;
    bool bEndInput = false;
    ULONG ulStart = CQtTime::GetSysTick();
    int iDoubleZeroLen = 0;
    while (CQtTime::GetSysTick() - ulStart < dwTimeOut)
    {
        if (m_cCancelEvent.WaitForEvent(10))
        {
            Log(ThisModule, __LINE__, "用户取消输入");
            return WFS_ERR_CANCELED;
        }

        long lRet = m_pDev->ReadKeyPress(stKeyVal, 300);
        if (lRet == ERR_DEVPORT_RTIMEOUT || // 读取超时
            lRet == ERR_DEVPORT_READERR)// 读取失败(USB转串口的，会返回此值)
        {
            continue;
        }
        else if (lRet == ERR_DEVPORT_CANCELED)
        {
            Log(ThisModule, __LINE__, "用户取消输入");
            return WFS_ERR_CANCELED;
        }
        else if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "读取按键失败：%d", lRet);
//            return lRet;
            return WFS_ERR_HARDWARE_ERROR;
        }
        Log(ThisModule, 1, "读取按键：%d[%s]", stKeyVal.byKeyVal, stKeyVal.szKeyName);

        // 分析按键
//        // 分析按键
//        if(stKeyVal.byKeyVal == 0x2A){
//            //数字按键
//            stKey.ulDigit = 0x00;
//            stKey.wCompletion = WFS_PIN_COMPCONTINUE;
//            if(cInput.IsAutoEnd() && cInput.MaxLen() != 0 && vtKey.size() + 1 >= cInput.MaxLen()){
//                stKey.wCompletion = WFS_PIN_COMPAUTO;
//            }
//            vtKey.push_back(stKey);
//        } else {
//            bool bFDK = false;
//            stKey.ulDigit = ConvertKeyVal(stKeyVal.byKeyVal, bFDK);
//            if(bFDK){
//                stKey.wCompletion = WFS_PIN_COMPCONTFDK;
//                if(cInput.IsTermFDK(stKey.ulDigit)){
//                    stKey.wCompletion = WFS_PIN_COMPFDK;
//                }
//            } else {
//                switch(stKey.ulDigit){
//                case WFS_PIN_FK_CLEAR:                          //Backspace/Clear
//                    vtKey.clear();
//                case WFS_PIN_FK_ENTER: {                         //Enter
//                    bool bTerminate = cInput.IsTermKey(stKey.ulDigit);
//                    if(bTerminate && m_bGetPinEntChkMinLen){
//                        bTerminate = vtKey.size() >= cInput.MinLen();
//                    }
//                    stKey.wCompletion = GetKeyCompletion(stKey.ulDigit, bTerminate);
//                    break;
//                }
//                case WFS_PIN_FK_CANCEL:                          //Cancel
//                    vtKey.clear();
//                    stKey.wCompletion = GetKeyCompletion(stKey.ulDigit, cInput.IsTermKey(stKey.ulDigit));
//                    break;
//                  //因为内部按键值转换表，会被屏蔽掉
//        //        case 0xA5:                          //键盘卡键
//        //        case 0xAA:                          //自动结束结束输入返回码
//                default:                              //不支持按键或非法返回值，忽略
//                    continue;
//                    break;
//                }
//            }
//        }
//        //发送按键事件
//        FireKeyPressEvent(stKey);

//        if(stKey.wCompletion != WFS_PIN_COMPCONTINUE && stKey.wCompletion != WFS_PIN_COMPCONTFDK){
//            cInput.SetEndKey(stKey);
//            bEndInput = true;
//        }

//        if (bEndInput)
//        {
//            Log(ThisModule, 1, "正常结束输入");
//            return WFS_SUCCESS;
//        }
        bool bFDK = false;
        ULONG ulKeyVal = ConvertKeyVal(stKeyVal.byKeyVal, bFDK);
        if(m_iClearKeyMode == 2){
            if(ulKeyVal == WFS_PIN_FK_CLEAR){
                if((cInput.GetActiveKey() & (WFS_PIN_FK_BACKSPACE | WFS_PIN_FK_CLEAR)) == WFS_PIN_FK_BACKSPACE){
                    ulKeyVal = WFS_PIN_FK_BACKSPACE;
                }
            }
        }
        if (!bFDK && cInput.IsActiveKey(ulKeyVal))// 功能按键
        {
            stKey.ulDigit = ulKeyVal;
            stKey.wCompletion = GetKeyCompletion(ulKeyVal, cInput.IsTermKey(ulKeyVal));
            switch (ulKeyVal)
            {
            case WFS_PIN_FK_CANCEL:
            case WFS_PIN_FK_CLEAR:
                vtKey.clear();// 清按键
                FireKeyPressEvent(stKey);// 发送按键
                break;
            case WFS_PIN_FK_BACKSPACE:                      //30-00-00-00(FS#0010)
            {                                               //30-00-00-00(FS#0010)
                if(vtKey.size() > 0){                       //30-00-00-00(FS#0010)
                    vtKey.pop_back();      //删除最后一个按键 //30-00-00-00(FS#0010)
                }                                           //30-00-00-00(FS#0010)
                FireKeyPressEvent(stKey);   //发送按键       //30-00-00-00(FS#0010)
            }                                               //30-00-00-00(FS#0010)
                break;                                      //30-00-00-00(FS#0010)
            case WFS_PIN_FK_ENTER:
                FireKeyPressEvent(stKey);// 发送按键
                break;
            case WFS_PIN_FK_00:
                if(cInput.IsAutoEnd() && cInput.MaxLen() != 0){
                    if(vtKey.size() + iDoubleZeroLen + 2 == cInput.MaxLen()){
                        stKey.wCompletion = WFS_PIN_COMPAUTO;
                    } else if(vtKey.size() + iDoubleZeroLen + 2 > cInput.MaxLen()) {
                        //忽略该按键
                        continue;
                    }
                }
                iDoubleZeroLen += 1;
                vtKey.push_back(stKey);
                FireKeyPressEvent(stKey);//
                break;
            default:
            {
                if (cInput.IsAutoEnd() && cInput.MaxLen() != 0 && vtKey.size() + iDoubleZeroLen + 1 == cInput.MaxLen())
                    stKey.wCompletion = WFS_PIN_COMPAUTO;
                if(cInput.MaxLen() == 0 || (cInput.MaxLen() != 0 && vtKey.size() + iDoubleZeroLen < cInput.MaxLen())){
                    vtKey.push_back(stKey);
                    FireKeyPressEvent(stKey);// 发送按键
                }
            }
                break;
            }

            if (cInput.IsTermKey(ulKeyVal) || stKey.wCompletion == WFS_PIN_COMPAUTO)
            {
                if (!cInput.IsGetPin() || (cInput.IsGetPin() && vtKey.size() + iDoubleZeroLen >= cInput.MinLen()))
                {
                    bEndInput = true;
                    cInput.SetEndKey(stKey);
                }

                if (cInput.IsGetPin() && ulKeyVal == WFS_PIN_FK_CANCEL)
                {
                    Log(ThisModule, __LINE__, "用户取消输入");
                    cInput.SetEndKey(stKey);
                    return WFS_ERR_CANCELED;
                }
            }
        }
        else if (bFDK && cInput.IsActiveFDK(ulKeyVal))
        {
            stKey.ulDigit = ulKeyVal;
            stKey.wCompletion = WFS_PIN_COMPCONTFDK;
            if (cInput.IsTermFDK(ulKeyVal))
            {
                bEndInput = true;
                stKey.wCompletion = WFS_PIN_COMPFDK;
                cInput.SetEndKey(stKey);
            }

            if (vtKey.size() + iDoubleZeroLen + 1 == cInput.MaxLen() && cInput.IsAutoEnd()){
                bEndInput = true;
                stKey.wCompletion = WFS_PIN_COMPAUTO;
            }
            if(cInput.MaxLen() == 0 || (cInput.MaxLen() != 0 && vtKey.size() + iDoubleZeroLen < cInput.MaxLen())){
                vtKey.push_back(stKey);
                FireKeyPressEvent(stKey);// 发送按键
            }
//            FireKeyPressEvent(stKey);   // 发送按键事件
        }

        if(m_bSystemKbEvtSupp){
            FireSystemKbEvent(ulKeyVal);
        }
        if (bEndInput)
        {
            Log(ThisModule, 1, "正常结束输入");
            return WFS_SUCCESS;
        }
    }

    Log(ThisModule, __LINE__, "用户输入超时");
    return WFS_ERR_TIMEOUT;
}

long CXFS_PIN::GetPinKeyPress_XZ(vectorWFSPINKEY &vtKeys, CInputDataHelper &cInput, DWORD dwTimeOut)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    CAutoCloseInput _autoclose(m_pDev, cInput.IsGetPin() ? true : false);

    WFSPINKEY stKey;
    EPP_KEYVAL stKeyVal;
    bool bEndInput = false;
    ULONG ulStart = CQtTime::GetSysTick();
    while (CQtTime::GetSysTick() - ulStart < dwTimeOut)
    {
        if (m_cCancelEvent.WaitForEvent(10))
        {
            Log(ThisModule, __LINE__, "用户取消输入");
            return WFS_ERR_CANCELED;
        }

        long lRet = m_pDev->ReadKeyPress(stKeyVal, 300);
        if (lRet == ERR_DEVPORT_RTIMEOUT || // 读取超时
            lRet == ERR_DEVPORT_READERR)// 读取失败(USB转串口的，会返回此值)
        {
            continue;
        }
        else if (lRet == ERR_DEVPORT_CANCELED)
        {
            Log(ThisModule, __LINE__, "用户取消输入");
            return WFS_ERR_CANCELED;
        }
        else if (lRet != ERR_SUCCESS)
        {
            Log(ThisModule, __LINE__, "读取按键失败：%d", lRet);
//            return lRet;
            return WFS_ERR_HARDWARE_ERROR;
        }
        Log(ThisModule, 1, "读取按键：%d[%s]", stKeyVal.byKeyVal, stKeyVal.szKeyName);

        // 分析按键
        bool bFireKeyEvent = true;                  //30-00-00-00(FS#0008)
        if(stKeyVal.byKeyVal == 0x2A){
            //数字按键
            stKey.ulDigit = 0x00;
            stKey.wCompletion = WFS_PIN_COMPCONTINUE;
            if(cInput.IsAutoEnd() && cInput.MaxLen() != 0 && vtKeys.size() + 1 >= cInput.MaxLen()){
                stKey.wCompletion = WFS_PIN_COMPAUTO;
            }
            vtKeys.push_back(stKey);
        } else {
            bool bFDK = false;
            stKey.ulDigit = ConvertKeyVal(stKeyVal.byKeyVal, bFDK);
            if(m_iClearKeyMode == 2){
                if(stKey.ulDigit == WFS_PIN_FK_CLEAR){
                    if((cInput.GetActiveKey() & (WFS_PIN_FK_BACKSPACE | WFS_PIN_FK_CLEAR)) == WFS_PIN_FK_BACKSPACE){
                        stKey.ulDigit = WFS_PIN_FK_BACKSPACE;
                    }
                }
            }
            if(bFDK){
                stKey.wCompletion = WFS_PIN_COMPCONTFDK;
                if(cInput.IsTermFDK(stKey.ulDigit)){
                    stKey.wCompletion = WFS_PIN_COMPFDK;
                }
            } else {
                bool bTerminate = cInput.IsTermKey(stKey.ulDigit);
                stKey.wCompletion = GetKeyCompletion(stKey.ulDigit, bTerminate);
                switch(stKey.ulDigit){
                case WFS_PIN_FK_CLEAR:                          //Clear
                    vtKeys.clear();
                    break;
                case WFS_PIN_FK_ENTER: {                         //Enter
                    if(bTerminate && m_bGetPinEntChkMinLen){
                        bTerminate = vtKeys.size() >= cInput.MinLen();
                        if(!bTerminate && !m_bFireEnterEventBefMinLen){     //30-00-00-00(FS#0008)
                            bFireKeyEvent = false;                          //30-00-00-00(FS#0008)
                        }                                                   //30-00-00-00(FS#0008)
                    }
                    stKey.wCompletion = GetKeyCompletion(stKey.ulDigit, bTerminate);
                    break;
                }
                case WFS_PIN_FK_BACKSPACE:          //30-00-00-00(FS#0010)
                {                                   //30-00-00-00(FS#0010)
                    if(vtKeys.size() > 0){          //30-00-00-00(FS#0010)
                        vtKeys.pop_back();          //30-00-00-00(FS#0010)
                    }                               //30-00-00-00(FS#0010)
                }                                   //30-00-00-00(FS#0010)
                    break;                          //30-00-00-00(FS#0010)
                case WFS_PIN_FK_CANCEL:                          //Cancel
                    vtKeys.clear();
                    break;
                  //因为内部按键值转换表，会被屏蔽掉
        //        case 0xA5:                          //键盘卡键
        //        case 0xAA:                          //自动结束结束输入返回码
                default:                              //不支持按键或非法返回值，忽略
                    continue;
                    break;
                }
            }
        }
        //发送按键事件
        if(bFireKeyEvent){                                      //30-00-00-00(FS#0008)
           FireKeyPressEvent(stKey);                            //30-00-00-00(FS#0008)
        }                                                       //30-00-00-00(FS#0008)

        if(stKey.wCompletion != WFS_PIN_COMPCONTINUE && stKey.wCompletion != WFS_PIN_COMPCONTFDK){
            cInput.SetEndKey(stKey);
            bEndInput = true;
        }

        if (bEndInput)
        {
            Log(ThisModule, 1, "正常结束输入");
            return WFS_SUCCESS;
        }
    }

    Log(ThisModule, __LINE__, "用户输入超时");
    return WFS_ERR_TIMEOUT;
}


WORD CXFS_PIN::GetKeyCompletion(ULONG ulKeyVal, bool bTermKey)
{
    WORD wCompletion = 0;
    switch (ulKeyVal)
    {
    case WFS_PIN_FK_ENTER:
        wCompletion = bTermKey ? WFS_PIN_COMPENTER : WFS_PIN_COMPCONTINUE;
        break;
    case WFS_PIN_FK_CANCEL:
        wCompletion = bTermKey ? WFS_PIN_COMPCANCEL : WFS_PIN_COMPCONTINUE;
        break;
    case WFS_PIN_FK_CLEAR:
        wCompletion = bTermKey ? WFS_PIN_COMPCLEAR : WFS_PIN_COMPCONTINUE;
        break;
    case WFS_PIN_FK_BACKSPACE:
        wCompletion = bTermKey ? WFS_PIN_COMPBACKSPACE : WFS_PIN_COMPCONTINUE;
        break;
    default:
        wCompletion = bTermKey ? WFS_PIN_COMPFK : WFS_PIN_COMPCONTINUE;
        break;
    }
    return wCompletion;
}

bool CXFS_PIN::FireKeyPressEvent(WFSPINKEY stKey)
{
    return m_pBase->FireEvent(WFS_EXECUTE_EVENT, WFS_EXEE_PIN_KEY, &stKey);
}

UINT CXFS_PIN::ConvertPinFormat(WORD wFormat)
{
    WORD wFmt = 0;
    switch (wFormat)
    {
    case WFS_PIN_FORM3624:
        wFmt = PIN_IBM3624;
        break;
    case WFS_PIN_FORMANSI:
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            wFmt = PIN_ASCII;
        } else {
            wFmt = PIN_ISO9564_0;
        }
        break;
    case WFS_PIN_FORMISO0:
        wFmt = PIN_ISO9564_0;
        break;
    case WFS_PIN_FORMISO1:
        wFmt = PIN_ISO9564_1;
        break;
    case WFS_PIN_FORMISO3:
    {
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            wFmt = PIN_ISO9564_3;
        }
    }
        break;
    case WFS_PIN_FORMVISA:
    {
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            wFmt = PIN_VISA;
        }
    }
        break;
    case WFS_PIN_FORMVISA3:
    {
        if(m_strDevName == "XZ" || m_strDevName == "CFES"){
            wFmt = PIN_VISA3;
        }
    }
        break;
    default:
        break;
    }
    return wFmt;
}

long CXFS_PIN::FmtCustomerData(QByteArray &vtData, UINT uPinFmt)
{
    THISMODULE(__FUNCTION__);
    if(m_iPinblockCustomerDataMode == 0){
        /* 原处理
        if ((!m_bRemovCustomerLast && vtData.length() < 12) || (m_bRemovCustomerLast && vtData.length() < 13))
        {
            Log(ThisModule, __LINE__, "无效用户数据长度");
            return WFS_ERR_INVALID_DATA;
        }
        */
        bool bInvalidCustomData = false;
        if (!m_bRemovCustomerLast) {
            if(uPinFmt != PIN_ISO9564_1 && vtData.length() < 12){
                bInvalidCustomData = true;
            }
            if(uPinFmt == PIN_ISO9564_1 && vtData.length() < 10){
                bInvalidCustomData = true;
            }
        } else {
            if(uPinFmt != PIN_ISO9564_1 && vtData.length() < 13){
                bInvalidCustomData = true;
            }
            if(uPinFmt == PIN_ISO9564_1 && vtData.length() < 11){
                bInvalidCustomData = true;
            }
        }
        if(bInvalidCustomData){
            Log(ThisModule, __LINE__, "无效用户数据长度");
            return WFS_ERR_INVALID_DATA;
        }

        for (int i = 0; i < vtData.length(); i++)
        {
            if (!isdigit(vtData[i]))
            {
                Log(ThisModule, __LINE__, "CustomerData有非数字字符：%s", vtData.constData());
                return WFS_ERR_INVALID_DATA;
            }
        }
        /*原处理
         // 13->取卡号的最后13位，再去掉最后一位校验位
        int nRemovLen = m_bRemovCustomerLast ? 13 : 12;
        int uPos = vtData.length() - nRemovLen;// 起始位置
        vtData   = vtData.mid(uPos, 12);
        */
        int iDataLen = uPinFmt == PIN_ISO9564_1 ? 10 : 12;
        if(m_bRemovCustomerLast){
            vtData.chop(1);
        }
        vtData = vtData.right(iDataLen);
    } else if(m_iPinblockCustomerDataMode == 1){
        bool bInvalidCustomData = false;
        if((uPinFmt != PIN_ISO9564_1 && vtData.length() < 12) ||
           (uPinFmt == PIN_ISO9564_1 && vtData.length() < 10)){
            bInvalidCustomData = true;
        }

        if(bInvalidCustomData){
            Log(ThisModule, __LINE__, "无效用户数据长度");
            return WFS_ERR_INVALID_DATA;
        }

        for (int i = 0; i < vtData.length(); i++)
        {
            if (!isdigit(vtData[i]))
            {
                Log(ThisModule, __LINE__, "CustomerData有非数字字符：%s", vtData.constData());
                return WFS_ERR_INVALID_DATA;
            }
        }
        int iDataLen = (uPinFmt == PIN_ISO9564_1 ? 10 : 12);
        if((uPinFmt != PIN_ISO9564_1 && vtData.length() > 12) ||
           (uPinFmt == PIN_ISO9564_1 && vtData.length() > 10)){
            vtData.chop(1);
        }
        vtData = vtData.right(iDataLen);
    } else {
        //数据长度不足时，自动在数据前补0
        int iDataLen = vtData.size();
        if(iDataLen <= 0){
            return WFS_ERR_INVALID_DATA;
        } else {
            for (int i = 0; i < iDataLen; i++)
            {
                if (!isdigit(vtData[i]))
                {
                    Log(ThisModule, __LINE__, "CustomerData有非数字字符：%s", vtData.constData());
                    return WFS_ERR_INVALID_DATA;
                }
            }

            //前补零
            int iLen = (uPinFmt == PIN_ISO9564_1 ? 11 : 13);
            if(vtData.size() < iLen){
                vtData.prepend(iLen - vtData.size(), 0x30);
            }

            //去掉最后一位校验位
            vtData.chop(1);
            vtData = vtData.right(iLen - 1);
        }
    }

    return 0;
}

UINT CXFS_PIN::ConvertCryptType(WORD wMode, WORD wAlgorithm)
{
    WORD wFmt = 0;
    if (wMode == WFS_PIN_MODEENCRYPT)
    {
        switch (wAlgorithm)
        {
        case WFS_PIN_CRYPTDESECB:
            wFmt = ECB_EN;
            break;
        case WFS_PIN_CRYPTDESCBC:
            wFmt = CBC_EN;
            break;
        case WFS_PIN_CRYPTDESMAC:
            wFmt = GetMacType();
            break;
        case WFS_PIN_CRYPTTRIDESECB:
            wFmt = ECB_EN;
            break;
        case WFS_PIN_CRYPTTRIDESCBC:
            wFmt = CBC_EN;
            break;
        case WFS_PIN_CRYPTTRIDESMAC:
            wFmt = GetMacType();
            break;
        case WFS_PIN_CRYPTSM4:
        {
            wFmt = CBC_EN;
            if(m_wSM4CryptAlgorithm == 0){
                wFmt = ECB_EN;
            }
            break;
        }
        case WFS_PIN_CRYPTSM4MAC:
            wFmt = GetMacType(true);
            break;
        default:
            break;
        }
    }
    else if (wMode == WFS_PIN_MODEDECRYPT)
    {
        switch (wAlgorithm)
        {
        case WFS_PIN_CRYPTDESECB:
            wFmt = ECB_DE;
            break;
        case WFS_PIN_CRYPTDESCBC:
            wFmt = CBC_DE;
            break;
        case WFS_PIN_CRYPTDESMAC:
//            wFmt = GetMacType();
            break;
        case WFS_PIN_CRYPTTRIDESECB:
            wFmt = ECB_DE;
            break;
        case WFS_PIN_CRYPTTRIDESCBC:
            wFmt = CBC_DE;
            break;
        case WFS_PIN_CRYPTTRIDESMAC:
//            wFmt = GetMacType();
            break;
        case WFS_PIN_CRYPTSM4:
        {
            wFmt = CBC_DE;
            if(m_wSM4CryptAlgorithm == 0){
                wFmt = ECB_DE;
            }
            break;
        }
        case WFS_PIN_CRYPTSM4MAC:
//            wFmt = GetMacType(true);
            break;
        default:
            break;
        }
    }

    return wFmt;
}

WORD CXFS_PIN::GetMacType(bool bSM4)
{
    WORD wFmt = 0;
    if (!bSM4)
    {
        //;计算DESMAC格式:0->ANSIX919,1->ANSIX99,2->PBOC,3->CBC
        switch (m_dwDesMacType)
        {
        case 0:
            wFmt = MAC_ANSIX919;
            break;
        case 1:
            wFmt = MAC_ANSIX99;
            break;
        case 2:
            wFmt = MAC_PBOC;
            break;
        case 3:
            wFmt = MAC_CBC;
            break;
        default:
            wFmt = MAC_ANSIX919;
            break;
        }
    }
    else
    {
        //;计算SM4MAC格式:0->BANKSYS，1->PBOC
        switch (m_dwSM4MacType)
        {
        case 0:
            wFmt = MAC_BANKSYS;
            break;
        case 1:
            wFmt = MAC_PBOC;
            break;
        default:
            wFmt = MAC_BANKSYS;
            break;
        }
    }

    return wFmt;
}

bool CXFS_PIN::IsSupportSM()
{
    return m_bSupportSM ? true : false;
}

bool CXFS_PIN::IsOnlyUseSM()
{
    return m_bOnlyUseSM ? true : false;
}

bool CXFS_PIN::SplitKeyUse(DWORD dwUse, vectorUINT &vtUse)
{
    // 判断用途，暂时只支持下面三种用途
    vtUse.clear();
    if (dwUse & WFS_PIN_USECRYPT)
        vtUse.push_back(PKU_CRYPT);
    if (dwUse & WFS_PIN_USEFUNCTION)
        vtUse.push_back(PKU_FUNCTION);
    if (dwUse & WFS_PIN_USEMACING)
        vtUse.push_back(PKU_MACING);
//    if (dwUse & WFS_PIN_USESVENCKEY)
//        vtUse.push_back(PKU_SVENCKEY);
    if (dwUse & WFS_PIN_USEKEYENCKEY)
        vtUse.push_back(PKU_KEYENCKEY);

    return !vtUse.empty();
}

bool CXFS_PIN::SplitSMKeyUse(DWORD dwUse, vectorUINT &vtUse)
{
    // 判断用途，国密暂时只支持下面三种用途
    vtUse.clear();
    if (dwUse & WFS_PIN_USECRYPT)
        vtUse.push_back(PKU_CRYPT);
    if (dwUse & WFS_PIN_USEFUNCTION)
        vtUse.push_back(PKU_FUNCTION);
    if (dwUse & WFS_PIN_USEMACING)
        vtUse.push_back(PKU_MACING);
    if (dwUse & WFS_PIN_USEKEYENCKEY)
        vtUse.push_back(PKU_KEYENCKEY);

    return !vtUse.empty();
}

QByteArray CXFS_PIN::GetAllKeyVal(ULONG ulActiveKey, ULONG ulActiveFDK)
{
    MAPKEYVAL *pKey = nullptr;
    QByteArray vtBcdKey;// = QByteArray::fromHex(m_strAllKeyVal.c_str());
    CAutoHex::Hex2Bcd(m_strAllKeyVal.c_str(), vtBcdKey);
    for (auto &it : vtBcdKey)
    {
        for (UINT i = 0; i < sizeof(g_stFuncKey) / sizeof(g_stFuncKey[0]); i++)
        {
            pKey = &g_stFuncKey[i];
            if (it == pKey->byKey)
            {
                if (!(pKey->ulVal & ulActiveKey))
                {
                    it = 0x00;
                    break;
                }
            }
        }

        // 判断是否要检测FDK
        if (m_strNotCheckFDK == "1")
            continue;

        for (UINT k = 0; k < sizeof(g_stFDKKey) / sizeof(g_stFDKKey[0]); k++)
        {
            pKey = &g_stFDKKey[k];
            if (it == pKey->byKey)
            {
                if (!(pKey->ulVal & ulActiveFDK))
                {
                    it = 0x00;
                    break;
                }
            }
        }
    }

    // 一定要转换为大写的，因H5X不支持小写的按键值
    vtBcdKey = vtBcdKey.toHex().toUpper();
    return vtBcdKey;
}

//30-00-00-00(FS#0010)
void CXFS_PIN::ReadConfig()
{
    THISMODULE(__FUNCTION__);

    QByteArray strFile("/PINConfig.ini");
#ifdef QT_WIN32
    strFile.prepend(SPETCPATH);
#else
    strFile.prepend(SPETCPATH);
#endif

    CINIFileReader cINIFileReader;
    if (!cINIFileReader.LoadINIFile(strFile.constData()))
    {
        Log(ThisModule, __LINE__, "加载配置文件失败：%s", strFile.constData());
    }

    CINIReader cIR = cINIFileReader.GetReaderSection("PINInfo");
    m_iClearKeyMode = (int)cIR.GetValue("ClearKeyAsBackspace", 0);
    m_bRSAUseSM2Key = (int)cIR.GetValue("RSAUseSM2Key", 0) == 1;
    m_bFireEnterEventBefMinLen = (int)cIR.GetValue("FireEnterEventBefMinLen", 0) == 1;        //30-00-00-00(FS#0008)
    m_bRSARandomMode = (int)cIR.GetValue("RSARandomMode", 0) == 1;  //30-00-00-00(FS#0006)
    m_iRSARemoteImportSymmKeyType = (int)cIR.GetValue("RSARemoteImportSymmKeyType", 0);
    m_iPinblockCustomerDataMode = (int)cIR.GetValue("PinblockCustomerDataMode", 0);
	m_bSystemKbEvtSupp = (int)cIR.GetValue("SystemKbEvtSupp", 0);

    return;
}

long CXFS_PIN::AnalyKeyDataFromFormatedData(UINT uPKeyEppNo, UINT uSKeyEppNo, RSADATAALGOTYPE eRSADataAlgo, QByteArray formatedData, QByteArray &keyData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = ERR_SUCCESS;

    //1.解密,原始数据格式(分量1(16字节)+校验值(8字节)+分量2(16字节)+ 校验值(8字节)+分量3(16字节)+ 校验值(8字节))
    QByteArray result;
    lRet = m_pDev->RSADecryptData(uSKeyEppNo, eRSADataAlgo, formatedData, result);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "解密加密密钥数据错误:SKeyEppNo:%d,RSADataAlgo:%d", uSKeyEppNo, eRSADataAlgo);
        return lRet;
    }
    //2.获取３个分量数据，最终密钥数据=分量１^分量2^分量3
    QByteArray keyVector1;
    QByteArray keyVector2;
    QByteArray keyVector3;
    if(result.size() != (16 + 8) * 3){
        Log(ThisModule, __LINE__, "加密密钥数据长度错误:%d", result.size());
        return ERR_PARAM;
    } else {
        keyVector1.append(result.constData(), 16);
        keyVector2.append(result.constData() + 24, 16);
        keyVector3.append(result.constData() + 48, 16);
        for(int i = 0; i < keyVector1.size(); i++){
            keyVector1[i] = keyVector1.at(i) ^ keyVector2.at(i) ^ keyVector3.at(i);
        }
    }

    //3.明文密钥数据加密
    result.clear();
    lRet = m_pDev->RSAEncryptData(uPKeyEppNo, eRSADataAlgo, keyVector1, result);
    if(lRet != ERR_SUCCESS){
        Log(ThisModule, __LINE__, "明文密钥数据加密失败:PKeyEppNo:%d,RSADataAlgo:%d", uPKeyEppNo, eRSADataAlgo);
        return lRet;
    }

    keyData.append(result);
    return lRet;
}

bool CXFS_PIN::FireSystemKbEvent(ULONG ulLFSKeyCode)
{
    int iRet = 0;
    if(!m_cVirtualKBEvent.isOpen()){
        iRet = m_cVirtualKBEvent.OpenUinputDevice();
        if(iRet != 0){
            Log(__FUNCTION__, __LINE__, "打开设备/dev/uinput失败");
            return false;
        }
    }

    WORD wSystemKbKeyCode;
    switch(ulLFSKeyCode){
    case WFS_PIN_FK_0: wSystemKbKeyCode = KEY_0; break;
    case WFS_PIN_FK_1: wSystemKbKeyCode = KEY_1; break;
    case WFS_PIN_FK_2: wSystemKbKeyCode = KEY_2; break;
    case WFS_PIN_FK_3: wSystemKbKeyCode = KEY_3; break;
    case WFS_PIN_FK_4: wSystemKbKeyCode = KEY_4; break;
    case WFS_PIN_FK_5: wSystemKbKeyCode = KEY_5; break;
    case WFS_PIN_FK_6: wSystemKbKeyCode = KEY_6; break;
    case WFS_PIN_FK_7: wSystemKbKeyCode = KEY_7; break;
    case WFS_PIN_FK_8: wSystemKbKeyCode = KEY_8; break;
    case WFS_PIN_FK_9: wSystemKbKeyCode = KEY_9; break;
    case WFS_PIN_FK_DECPOINT: wSystemKbKeyCode = KEY_DOT; break;
    case WFS_PIN_FK_CLEAR: wSystemKbKeyCode = KEY_BACKSPACE; break;
    case WFS_PIN_FK_ENTER: wSystemKbKeyCode = KEY_ENTER; break;
    default:
        Log(__FUNCTION__, __LINE__, "不支持的按键值[0x%x]转换为系统按键值", ulLFSKeyCode);
        return false;
    }

    if(!m_cVirtualKBEvent.FireVirtualKeyEvent(wSystemKbKeyCode)){
        Log(__FUNCTION__, __LINE__, "发送虚拟按键[%d]失败", wSystemKbKeyCode);
        return false;
    }

    return true;
}

// History = 2021/04/18,SP  30-00-00-00,CFES(Guojy): 30-00-00-00(FS#0003) for guangda 远程密钥下载
HRESULT CXFS_PIN::EncIo(const LPWFSPINENCIO lpEncIO, LPWFSPINENCIO &lpEncIoData)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    long lRet = 0;  

    if ((lpEncIO->ulDataLength == 0) || (lpEncIO->lpvData == nullptr ))
    {
        Log(ThisModule, __LINE__, "无效指针参数");
        return WFS_ERR_INVALID_DATA;
    }

    if(m_strDevName == "XZ" || m_strDevName == "CFES"){
        Log(ThisModule, 1, "旭子键盘远程密钥下载");

        LPSTR lpsKey = nullptr;
        DWORD dwUse = 0;
        WORD wExportItemType = 0;
        LPSTR lpsSigKey = nullptr;
        DWORD dwSM2SigAlg = 0;
        LPWFSXDATA lpxKeyValue = nullptr; //RSA PK
        LPSTR lpsDecryptKey = nullptr;

        CAutoHex cHex;
        QByteArray vtBCD;
        wLoadKeyCatalog = 0;
        WORD wProtocol          = lpEncIO->wProtocol;

        //Check excute command
        WORD wCommandIn = ((LPPROTCHNIN)lpEncIO->lpvData)->wCommand;

        if(wCommandIn == WFS_CMD_ENC_IO_CHN_GENERATE_SM2_KEY_PAIR) {
            LPPROTCHNGENERATESM2KEYPAIRIN lpGenerateSm2KeyPairIn = (LPPROTCHNGENERATESM2KEYPAIRIN)lpEncIO->lpvData;
            lpsKey			= lpGenerateSm2KeyPairIn->lpsKey;
            dwUse			= lpGenerateSm2KeyPairIn->dwUse;

            QByteArray strWKey(lpsKey);
            PINKEYDATA stKey;

            // 判断用途
            if (dwUse != PROT_CHN_USESM2PRIVATE && dwUse != PROT_CHN_USESM2PRIVATESIGN)
            {
               Log(ThisModule, __LINE__, "不支持密钥用途:%d", dwUse);
               return WFS_ERR_INVALID_DATA;
            }

            stKey = m_cKeyXZ.FindAsymmKeySlot(strWKey);
            if (m_cKeyXZ.IsMaxKey(stKey))
            {
                Log(ThisModule, __LINE__, "密钥已下载满：%s", strWKey.constData());
                return WFS_ERR_PIN_NOKEYRAM;
            }

            lRet = m_pDev->SM2GenerateKey(stKey.uNum, 0xFF);
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "下载SM2密钥失败：%s", strWKey.constData());
                return WFS_ERR_HARDWARE_ERROR;
            }

            //返回结果
            m_stGenerateSm2KeyPairData.Add(WFS_CMD_ENC_IO_CHN_GENERATE_SM2_KEY_PAIR, PROT_CHN_SUCCESS);
            m_stENCIOData.Add((LPBYTE)m_stGenerateSm2KeyPairData.GetData(), sizeof(m_stGenerateSm2KeyPairData), wProtocol);
            lpEncIoData = m_stENCIOData.GetData();

            //保存密钥
            stKey.dwUse = dwUse;
            m_cKeyXZ.AddKey(stKey);

            Log(ThisModule, 1, "下载密钥对成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
        } else if(wCommandIn == WFS_CMD_ENC_IO_CHN_EXPORT_SM2_EPP_SIGNED_ITEM) {
            LPPROTCHNEXPORTSM2EPPSIGNEDITEMIN lpExportSm2EppSignedItemIn = (LPPROTCHNEXPORTSM2EPPSIGNEDITEMIN)lpEncIO->lpvData;
            lpsKey			= lpExportSm2EppSignedItemIn->lpsName;
            wExportItemType = lpExportSm2EppSignedItemIn->wExportItemType;
            lpsSigKey       = lpExportSm2EppSignedItemIn->lpsSigKey;

            QByteArray strWKey(lpsKey);
            PINKEYDATA stKey;

            stKey = m_cKeyXZ.FindAsymmKey(strWKey);
            if (stKey.uNum == KEY_NOT_NUM)
                return WFS_ERR_PIN_KEYNOTFOUND;

            char cPKeyData[129] = {0};
            lRet = m_pDev->SM2ExportPKeyForGD(stKey.uNum,(LPSTR)cPKeyData);
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "SM2导出公钥失败");
                return WFS_ERR_HARDWARE_ERROR;
            }

            // 返回结果
            cHex.Hex2Bcd(cPKeyData, vtBCD);
            m_stExportSm2EppSignItemData.Add((LPBYTE)vtBCD.constData(), vtBCD.size(), wCommandIn, 0);
            m_stENCIOData.Add((LPBYTE)m_stExportSm2EppSignItemData.GetData(), sizeof(m_stExportSm2EppSignItemData), wProtocol);
            lpEncIoData = m_stENCIOData.GetData();
            Log(ThisModule, 1, "导出密钥成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
        } else if(wCommandIn == WFS_CMD_ENC_IO_CHN_IMPORT_SM2_SIGNED_SM4_KEY) {
            LPPROTCHNIMPORTSM2SIGNEDSM4KEY lpImportSm2SignSm4KeyIn = (LPPROTCHNIMPORTSM2SIGNEDSM4KEY)lpEncIO->lpvData;
            lpsKey			= lpImportSm2SignSm4KeyIn->lpsKey;
            dwUse			= lpImportSm2SignSm4KeyIn->dwUse;
            lpxKeyValue  = lpImportSm2SignSm4KeyIn->lpxValue;
            lpsDecryptKey   = lpImportSm2SignSm4KeyIn->lpsDecryptKey;
            QByteArray strWKey(lpsKey);
            QByteArray strDecryptKey(lpsDecryptKey);
            QByteArray strSignKey(lpImportSm2SignSm4KeyIn->lpsSigKey);
            PINKEYDATA stKey;
            PINKEYDATA stDKey;
            PINKEYDATA stSignKey;
            stSignKey.uNum = KEY_NOT_NUM;

            // 判断用途
            if (!IsSupportKeyUse(dwUse))
            {
               Log(ThisModule, __LINE__, "不支持密钥用途:%d", dwUse);
               return WFS_ERR_INVALID_DATA;
            }

            if(lpxKeyValue == nullptr || lpxKeyValue->usLength == 0){
                Log(ThisModule, __LINE__, "入力数据为空:");
                return WFS_ERR_INVALID_DATA;
            }

            long lRet = 0;
            char szKCV[64] = { 0 };
            dwUse = ConvertKeyUse(dwUse);
            stKey = m_cKeyXZ.FindSMKeySlot(strWKey);    //国密主密钥
            if (m_cKeyXZ.IsMaxKey(stKey))
            {
                Log(ThisModule, __LINE__, "密钥已下载满：%s", strWKey.constData());
                return WFS_ERR_PIN_NOKEYRAM;
            }

            if(strDecryptKey.isEmpty()){
                listPINKEYDATA asymmKeyList;
                m_cKeyXZ.GetAllAsymmKey(asymmKeyList);

                if(asymmKeyList.size() == 0){
                    Log(ThisModule, __LINE__, "解密key未找到：未下载SM2秘钥");
                    return WFS_ERR_PIN_KEYNOTFOUND;
                } else {
                    stDKey = *(asymmKeyList.begin());
                }
            } else {
                stDKey = m_cKeyXZ.FindAsymmKey(strDecryptKey); //解密key
                if(m_cKeyXZ.IsInvalidKey(stDKey))
                {
                    Log(ThisModule, __LINE__, "解密key未找到：%s", strDecryptKey.data());
                    return WFS_ERR_PIN_KEYNOTFOUND;
                }
            }

            QByteArray vtSignature;
            QByteArray vtUserId;
            QByteArray vtKeyValue;
            if(!strSignKey.isEmpty()){
                stSignKey = m_cKeyXZ.FindAsymmKey(strSignKey); //验签key
                if(m_cKeyXZ.IsInvalidKey(stSignKey))
                {
                    Log(ThisModule, __LINE__, "验签key未找到：%s", strSignKey.data());
                    return WFS_ERR_PIN_KEYNOTFOUND;
                }

                if(lpImportSm2SignSm4KeyIn->lpxSignature->usLength == 0 ||
                   lpImportSm2SignSm4KeyIn->lpxSignature->lpbData == nullptr){
                    Log(ThisModule, __LINE__, "验签数据为空");
                    return WFS_ERR_PIN_INVALIDDATA;
                }

                CAutoHex::Bcd2Hex(lpImportSm2SignSm4KeyIn->lpxSignature->lpbData,
                                  lpImportSm2SignSm4KeyIn->lpxSignature->usLength, vtSignature);
            }

            CAutoHex::Bcd2Hex(lpImportSm2SignSm4KeyIn->lpxValue->lpbData,
                              lpImportSm2SignSm4KeyIn->lpxValue->usLength, vtKeyValue);
            lRet = m_pDev->SM4RemoteImportKey(stKey.uNum, stDKey.uNum, stSignKey.uNum, dwUse,
                                              vtUserId.constData(), vtKeyValue.constData(), vtSignature.constData(), szKCV);
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "下载工作密钥失败：%s", strWKey.constData());
                return WFS_ERR_HARDWARE_ERROR;
            }

            // 保存密钥
            stKey.dwUse = dwUse;
            strcpy(stKey.szKCV, szKCV);
            m_cKeyXZ.AddKey(stKey);
            Log(ThisModule, 1, "多用途下载工作密钥成功：%s]", stKey.szName);

            // 返回结果
            QByteArray vtBCD;
            cHex.Hex2Bcd(szKCV, vtBCD);
            m_stImportSm4Data.Add((LPBYTE)vtBCD.constData(), vtBCD.size(), wCommandIn, PROT_CHN_SUCCESS);
            m_stENCIOData.Add((LPBYTE)m_stImportSm4Data.GetData(), sizeof(m_stImportSm4Data), wProtocol);
            lpEncIoData = m_stENCIOData.GetData();
            Log(ThisModule, 1, "下载密钥成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
        } else {
            return WFS_ERR_UNSUPP_COMMAND;
        }
    } if(m_strDevName == "ZTC90"){
        LPSTR lpsKey = nullptr;
        DWORD dwUse = 0;
        WORD wExportItemType = 0;
        LPSTR lpsSigKey = nullptr;
        DWORD dwSM2SigAlg = 0;
        LPWFSXDATA lpxKeyValue = nullptr; //RSA PK
        LPSTR lpsDecryptKey = nullptr;

        CAutoHex cHex;
        QByteArray vtBCD;
        wLoadKeyCatalog = 0;
        WORD wProtocol          = lpEncIO->wProtocol;

        //Check excute command
        WORD wCommandIn = ((LPPROTCHNIN)lpEncIO->lpvData)->wCommand;

        if(wCommandIn == WFS_CMD_ENC_IO_CHN_GENERATE_SM2_KEY_PAIR) {
            LPPROTCHNGENERATESM2KEYPAIRIN lpGenerateSm2KeyPairIn = (LPPROTCHNGENERATESM2KEYPAIRIN)lpEncIO->lpvData;
            lpsKey			= lpGenerateSm2KeyPairIn->lpsKey;
            dwUse			= lpGenerateSm2KeyPairIn->dwUse;

            QByteArray strWKey(lpsKey);
            PINKEYDATA stKey;

            // 判断用途
            if (dwUse != PROT_CHN_USESM2PRIVATE && dwUse != PROT_CHN_USESM2PRIVATESIGN)
            {
               Log(ThisModule, __LINE__, "不支持密钥用途:%d", dwUse);
               return WFS_ERR_INVALID_DATA;
            }

            stKey = m_cKeyC90.FindSM2KeySlot(strWKey);
            if (m_cKeyC90.IsMaxKey(stKey))
            {
                Log(ThisModule, __LINE__, "密钥已下载满：%s", strWKey.constData());
                return WFS_ERR_PIN_NOKEYRAM;
            }

            lRet = m_pDev->SM2GenerateKey(stKey.uNum, 0xFF);
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "下载SM2密钥失败：%s", strWKey.constData());
                return WFS_ERR_HARDWARE_ERROR;
            }

            //返回结果
            m_stGenerateSm2KeyPairData.Add(WFS_CMD_ENC_IO_CHN_GENERATE_SM2_KEY_PAIR, PROT_CHN_SUCCESS);
            m_stENCIOData.Add((LPBYTE)m_stGenerateSm2KeyPairData.GetData(), sizeof(m_stGenerateSm2KeyPairData), wProtocol);
            lpEncIoData = m_stENCIOData.GetData();

            //保存密钥
            stKey.dwUse = dwUse;
            m_cKeyC90.AddKey(stKey);

            Log(ThisModule, 1, "下载密钥对成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
        } else if(wCommandIn == WFS_CMD_ENC_IO_CHN_EXPORT_SM2_EPP_SIGNED_ITEM) {
            LPPROTCHNEXPORTSM2EPPSIGNEDITEMIN lpExportSm2EppSignedItemIn = (LPPROTCHNEXPORTSM2EPPSIGNEDITEMIN)lpEncIO->lpvData;
            lpsKey			= lpExportSm2EppSignedItemIn->lpsName;
            wExportItemType = lpExportSm2EppSignedItemIn->wExportItemType;
            lpsSigKey       = lpExportSm2EppSignedItemIn->lpsSigKey;

            QByteArray strWKey(lpsKey);
            PINKEYDATA stKey;

            if(!m_cKeyC90.FindSM2Key(strWKey, stKey))
                return WFS_ERR_PIN_KEYNOTFOUND;

            char cPKeyData[64] = {0};
            lRet = m_pDev->SM2ExportPKeyForGD(stKey.uNum,(LPSTR)cPKeyData);
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "SM2导出公钥失败");
                return WFS_ERR_HARDWARE_ERROR;
            }

            // 返回结果
            cHex.Hex2Bcd(cPKeyData, vtBCD);
            m_stExportSm2EppSignItemData.Add((LPBYTE)vtBCD.constData(), vtBCD.size(), wCommandIn, 0);
            m_stENCIOData.Add((LPBYTE)m_stExportSm2EppSignItemData.GetData(), sizeof(m_stExportSm2EppSignItemData), wProtocol);
            lpEncIoData = m_stENCIOData.GetData();
            Log(ThisModule, 1, "导出密钥成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
        } else if(wCommandIn == WFS_CMD_ENC_IO_CHN_IMPORT_SM2_SIGNED_SM4_KEY) {
            LPPROTCHNIMPORTSM2SIGNEDSM4KEY lpImportSm2SignSm4KeyIn = (LPPROTCHNIMPORTSM2SIGNEDSM4KEY)lpEncIO->lpvData;
            lpsKey			= lpImportSm2SignSm4KeyIn->lpsKey;
            dwUse			= lpImportSm2SignSm4KeyIn->dwUse;
            lpxKeyValue  = lpImportSm2SignSm4KeyIn->lpxValue;
            lpsDecryptKey   = lpImportSm2SignSm4KeyIn->lpsDecryptKey;
            QByteArray strWKey(lpsKey);
            QByteArray strMKey(lpsDecryptKey);
            PINKEYDATA stKey;
            PINKEYDATA stDKey;

            // 判断用途
            if (!IsSupportKeyUse(dwUse))
            {
               Log(ThisModule, __LINE__, "不支持密钥用途:%d", dwUse);
               return WFS_ERR_INVALID_DATA;
            }

            if(lpxKeyValue == nullptr || lpxKeyValue->usLength == 0){
                Log(ThisModule, __LINE__, "入力数据为空:");
                return WFS_ERR_INVALID_DATA;
            }

            long lRet = 0;
            char szKCV[64] = { 0 };
            dwUse = ConvertKeyUse(dwUse);
            stKey = m_cKeyC90.FindMKeySlot(strWKey);    //国密主密钥
            if (m_cKeyC90.IsMaxKey(stKey))
            {
                Log(ThisModule, __LINE__, "密钥已下载满：%s", strWKey.constData());
                return WFS_ERR_PIN_NOKEYRAM;
            }

            if(strMKey.isEmpty()){
                listPINKEYDATA asymmKeyList;
                m_cKeyC90.GetAllKey(asymmKeyList);

                if(asymmKeyList.size() == 0){
                    Log(ThisModule, __LINE__, "解密key未找到：未下载SM2秘钥");
                    return WFS_ERR_PIN_KEYNOTFOUND;
                } else {
                    stDKey = *(asymmKeyList.begin());
                }
            } else {
                if(!m_cKeyC90.FindSM2Key(strMKey, stDKey)) //解密key
                {
                    Log(ThisModule, __LINE__, "解密key未找到：%s", strMKey.data());
                    return WFS_ERR_PIN_KEYNOTFOUND;
                }
            }

            CAutoHex cHex;
            BYTE byKeyValueTemp[1024] = {0};
            strcpy((char *)byKeyValueTemp, cHex.Bcd2Hex(lpxKeyValue->lpbData, lpxKeyValue->usLength));
            lRet = m_pDev->SM4RemoteImportKey(stKey.uNum, stDKey.uNum, 0xFF, dwUse, "", (LPSTR)byKeyValueTemp, "", szKCV);
            if (lRet != ERR_SUCCESS)
            {
                Log(ThisModule, __LINE__, "下载工作密钥失败：%s", strWKey.constData());
                return WFS_ERR_HARDWARE_ERROR;
            }

            // 保存密钥
            stKey.dwUse = dwUse;
            strcpy(stKey.szKCV, szKCV);
            m_cKeyC90.AddKey(stKey);
            Log(ThisModule, 1, "多用途下载工作密钥成功：%s]", stKey.szName);

            // 返回结果
            QByteArray vtBCD;
            cHex.Hex2Bcd(szKCV, vtBCD);
            m_stImportSm4Data.Add((LPBYTE)vtBCD.constData(), vtBCD.size(), wCommandIn, PROT_CHN_SUCCESS);
            m_stENCIOData.Add((LPBYTE)m_stImportSm4Data.GetData(), sizeof(m_stImportSm4Data), wProtocol);
            lpEncIoData = m_stENCIOData.GetData();
            Log(ThisModule, 1, "下载密钥成功：%s[Type=%d[密钥类型：0->主密钥，1工作密钥，2->国密密钥]]", stKey.szName, stKey.dwType);
        } else {
            return WFS_ERR_UNSUPP_COMMAND;
        }
    } else{
        Log(ThisModule, 1, "证通键盘远程密钥下载");
        return WFS_ERR_UNSUPP_COMMAND;
    }

    return WFS_SUCCESS;
}

//30-00-00-00(FT#0070)
HRESULT CXFS_PIN::OnClearCancelSemphoreCount()
{
    while(m_cCancelEvent.WaitForEvent(1));
    return WFS_SUCCESS;
}
