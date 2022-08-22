#include "AdapterUR2.h"
#include <assert.h>
#include <time.h>
#include <unistd.h>
#include <map>
#include <QDir>
#include <QTextStream>
using namespace std;

#define THIS_FILE_ADP       "UR2_Adapter"   //适配层模块名
#define VERSION_HCM_SP      "1.0.0.1"

static const char *URIniName = "AdpConfig_UR2.ini";
static const char *ThisFile = "AdapterUR2";
static const char *CashAcceptorIniName = "CashAcceptor.ini"; //test#5

//检查输入输出指针是否为空
#define VERIFYPOINTNOTEMPTY(point) \
    {\
        if(NULL == point)\
        {\
            Log(ThisModule, WFS_ERR_INVALID_POINTER, "指针%s为空", #point);\
            return WFS_ERR_INVALID_POINTER;\
        }\
    }



//检查钞箱序号是否有效
#define VERIFYCASSINDEX(uIndex)\
    {\
        if ((uIndex > ADP_MAX_CU_SIZE) || (uIndex == 0))\
        {\
            return WFS_ERR_UNSUPP_DATA;\
        }\
    }

//将所有钞箱错误类型设置为没有使用
#define SETCASSUNITUNUSED(arryCUErr)\
    {\
        for(int k = 0; k < ADP_MAX_CU_SIZE; k++)\
        {\
            arryCUErr[k] = ADP_CUERROR_NOTUSED;\
        }\
    }


CUR2Adapter::CUR2Adapter()
{
    SetLogFile(LOGFILE, ThisFile, "UR2");
    m_pUR2Dev  = nullptr;
    m_bBRMInitialized = FALSE;
    m_bIsNeedReset = TRUE;
    m_bExchangeActive = FALSE;
    m_bCashOutBeforePresent = FALSE;
    m_bOpenShutterJammed = FALSE;
    m_bCloseShutterJammed = FALSE;
    m_bConnectNormal = FALSE;
    m_bSIUOpen = FALSE;                             //30-00-00-00(FS#0004)
    memset(m_aryRetractCount, 0, sizeof(ULONG) * ADP_MAX_CU_SIZE);
    memset(m_aryulRetractCountPerDeno, 0, sizeof(m_aryulRetractCountPerDeno));
    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        m_arybEnableCassUnit[i] = true;
    }
    memset(m_szFWVersion, 0, sizeof(m_szFWVersion));
    memset(m_szSPVersion, 0, sizeof(m_szSPVersion));
    memset(m_szRejectBill, 0, sizeof(m_szRejectBill));
    memset(m_szLastErrCode, 0, sizeof(m_szLastErrCode));
    memset(m_szLastErrCodeAction, 0, sizeof(m_szLastErrCodeAction));
    strcpy(m_szLastErrCode, NORMAL_ERRCODE);
    strcpy(m_szLastErrCodeAction, NORMAL_ERRCODE);
    memset(m_bCassVerifyFail, 0, sizeof(m_bCassVerifyFail));
    memset(m_szFWFilesPath, 0, sizeof(m_szFWFilesPath));
    memset(m_szApAmountLimitINIItem, 0, sizeof(m_szApAmountLimitINIItem));    //30-00-00-00(FS#0009)

    m_strConfigFile = string(SPETCPATH) + "/" + URIniName;
    m_strCashAcceptorFile = string(WSAPCFGPATH) + "/" + CashAcceptorIniName;         //test#5

    InitADPConfig();
    InitADPStatus();
    m_pErrCodeTrans = new ErrCodeTranslate(m_aryADPCassSettingInfo);

    m_sLastestInternalCounts.clear();
    iTooManeyItemsHappenFlg = 0;                        //test#7
    m_eLfsCmdId = ADP_OTHER;                            //test#8
    m_iNumStackedToESC = 0;                             //30-00-00-00(FT#0037)

    Log("CUR2Adapter", 1, "CUR2Adapter Finish");
}


CUR2Adapter::~CUR2Adapter()
{
    Close();
    m_bIsNeedReset = TRUE;
    m_bExchangeActive = FALSE;
    m_bCashOutBeforePresent = FALSE;
    m_bOpenShutterJammed = FALSE;
    m_bCloseShutterJammed = FALSE;
    //m_pListener = NULL;
    if (m_pErrCodeTrans != NULL)
    {
        delete m_pErrCodeTrans;
        m_pErrCodeTrans = NULL;
    }
}



//功能：打开机芯通信连路,建立PC与机芯间的通信
//输入：无
//输出：无
//返回：XFS通用错误码
//说明：串口号、USB端口号等参数由适配层自己决定，不由SP传入
long CUR2Adapter::Open()
{
    const char *ThisModule = "Open";
    AutoLogFuncBeginEnd();
    int iRet = 0;
    if (nullptr == m_pUR2Dev)
    {
        iRet = CreateURDevice("UR2", m_pUR2Dev);
        if ((iRet < 0) || (m_pUR2Dev == nullptr))
        {
            m_pUR2Dev = nullptr;
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "导入驱动库失败(ReturnValue:%d)", iRet);
            return WFS_ERR_HARDWARE_ERROR;
        }

    }
    iRet = OpenUSBConnect(CONNECT_TYPE_UR_ZERO);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "打开USB连接失败");
        return iRet;
    }

    if(!m_bBRMInitialized)
    {
        iRet = InitSetting();
    }

    if(m_pCryptData == nullptr)
    {
        string strdllname = "libCryptyData.so";
        if (0 != m_pCryptData.Load(strdllname.c_str(), "CreateCryptData"))
        {
            Log(ThisModule, __LINE__, "加载库失败: DllName=%s", strdllname.c_str());
            return false;
        }
    }

    //30-00-00-00(FS#0004) add start
    //安全门状态是否支持
    if(m_sDevConfigInfo.bSafeDoorStatusSupport){
        if(m_pDevSIU == nullptr){
            //加载Dev
            string strDevDll = "libDevSIU_IOMC";
            if (0 != m_pDevSIU.Load(strDevDll.c_str(), "CreateIDevSIU", GetIOMCHWType().c_str()))
            {
                Log(ThisModule, __LINE__, "加载库失败: DllName=%s[%s]", strDevDll.c_str(),
                        m_pDevSIU.LastError().toStdString().c_str());
            } else {
                if(m_pDevSIU->Open("") != 0){
                    Log(ThisModule, __LINE__, "现金模块打开SIU失败");
                } else {
                    m_bSIUOpen = TRUE;
                }
            }
        }
    }
    //30-00-00-00(FS#0004) add end
    return WFS_SUCCESS;
}

//功能：关闭机芯连接
//输入：无
//输出：无
//返回：XFS通用错误码
//说明：内部方法
long CUR2Adapter::CloseUSBConnect(CONNECT_TYPE ct)
{
    const char *ThisModule = "CloseUSBConnect";
    assert(m_pUR2Dev != NULL);

    int iRet = 0;
    if (ct & CONNECT_TYPE_UR)
    {
        iRet = m_pUR2Dev->CloseURConn();
        if (iRet < 0)
        {
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "关闭UR2连接失败(ReturnValue:%d)", iRet);
            return WFS_ERR_HARDWARE_ERROR;
        }
        m_bConnectNormal = FALSE;
    }

    if (ct & CONNECT_TYPE_ZERO)
    {
        iRet = m_pUR2Dev->CloseBVConn();
        if (iRet < 0)
        {
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "关闭ZERO连接失败(ReturnValue:%d)", iRet);
            return WFS_ERR_HARDWARE_ERROR;
        }

        m_bBVConnectNormal = FALSE;
    }

    m_bBRMInitialized = FALSE;
    return WFS_SUCCESS;
}

//功能：打开机芯连接
//输入：无
//输出：无
//返回：XFS通用错误码
//说明：内部方法
long CUR2Adapter::OpenUSBConnect(CONNECT_TYPE ct)
{
    const char *ThisModule = "OpenUSBConnect";
    assert(m_pUR2Dev != NULL);


    int iRet = 0;

    //打开HCM连接
    if (ct & CONNECT_TYPE_UR)
    {
        //可能为重新打开的连接， 打开前关闭一次连接
        CloseUSBConnect(CONNECT_TYPE_UR);
        iRet = m_pUR2Dev->OpenURConn();
        if (iRet < 0)
        {
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "打开UR2失败(ReturnValue:%d)", iRet);
            CloseUSBConnect(CONNECT_TYPE_UR);
            return WFS_ERR_HARDWARE_ERROR;
        }
        m_bConnectNormal = TRUE;
    }


    //打开ZERO-BV连接
    if (ct & CONNECT_TYPE_ZERO)
    {
        CloseUSBConnect(CONNECT_TYPE_ZERO);
        iRet = m_pUR2Dev->OpenBVConn();
        if (iRet < 0)
        {
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "打开ZERO-BV失败(ReturnValue:%d)", iRet);
            CloseUSBConnect(CONNECT_TYPE_ZERO);
            return WFS_ERR_HARDWARE_ERROR;
        }
        m_bBVConnectNormal = TRUE;
    }

    return WFS_SUCCESS;
}

//功能：关闭机芯
//输入：无
//输出：无
//返回：XFS通用错误码
//说明：
long CUR2Adapter::Close()
{
    const char *ThisModule = "Close";

    if (NULL != m_pUR2Dev)
    {
        int iRet = CloseUSBConnect(CONNECT_TYPE_UR_ZERO);
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "关闭驱动连接失败");
            return iRet;
        }
        m_bConnectNormal = FALSE;
        m_bBVConnectNormal = FALSE;
        m_pUR2Dev->Release();
        m_pUR2Dev = NULL;
    }

    if(m_pDevSIU != nullptr){                   //30-00-00-00(FS#0004)
        m_pDevSIU->Close();                     //30-00-00-00(FS#0004)
        m_pDevSIU->Release();                   //30-00-00-00(FS#0004)
        m_bSIUOpen = FALSE;                     //30-00-00-00(FS#0004)
    }                                           //30-00-00-00(FS#0004)
    return WFS_SUCCESS;
}

//功能：初始化机芯状态
//输入：无
//输出：无
//返回：无
//说明：
void CUR2Adapter::InitADPStatus()
{
    //设备状态
    m_sDevStatus.stDevice = ADP_DEVICE_OFFLINE;
    m_sDevStatus.stInShutter = ADP_INSHUTTER_UNKNOWN;
    m_sDevStatus.stOutShutter = ADP_OUTSHUTTER_UNKNOWN;
    m_sDevStatus.stInPutPosition = ADP_INPOS_UNKNOWN;
    m_sDevStatus.stOutPutPosition = ADP_OUTPOS_UNKNOWN;
    m_sDevStatus.stBanknoteReader = ADP_BR_UNKNOWN;
    m_sDevStatus.stSafeDoor = ADP_SAFEDOOR_UNKNOWN;
    m_sDevStatus.stStacker = ADP_STACKER_UNKNOWN;
    m_sDevStatus.stTransport = ADP_TRANSPORT_UNKNOWN;

    //钱箱状态
    for (int i = 0 ; i < ADP_MAX_CU_SIZE; i++)
    {
        m_aryADPCassSettingInfo[i].bActive = FALSE;
        m_aryADPCassSettingInfo[i].iCassType = ADP_CASSETTE_UNKNOWN;
        m_aryADPCassSettingInfo[i].ulValue = 0;
        m_aryADPCassSettingInfo[i].usDenoCode = 0;
        memcpy(m_aryADPCassSettingInfo[i].CurrencyID, "   ", 3);
        m_aryADPCassStatusInfo[i] = ADP_CASHUNIT_INOP;
        m_aryCassStatusWhenEndEx[i] = ADP_CASHUNIT_INOP;
    }
}

//功能：设置相关配置选项
//输入：无
//输出：无
//返回：无
//说明：
long CUR2Adapter::InitADPConfig()
{
    const char *ThisModule = "InitADPConfig";
    int iValue = 0;
    m_ReaderConfigFile.LoadINIFile(m_strConfigFile);
    CINIReader cINI = m_ReaderConfigFile.GetReaderSection("BRMInfo");

    m_ReaderCashAcceptorFile.LoadINIFile(m_strCashAcceptorFile);            //test#5

    //是否启用补正处理（0：不启用，1：启用 默认为0）
    m_sDevConfigInfo.bRetractCashCountCalibration = (int)cINI.GetValue("RetractCashCountCalibration", "0");     //30-00-00-00(FT#0037)

    iValue = (int)cINI.GetValue("CashOutNoteType", "1");
    m_sDevConfigInfo.CashOutNoteType = (iValue == 1) ? VALIDATION_MODE_TEST : VALIDATION_MODE_REAL;
    iValue = (int)cINI.GetValue("CashInNoteType", "1");
    m_sDevConfigInfo.CashInNoteType = (iValue == 1) ? VALIDATION_MODE_TEST : VALIDATION_MODE_REAL;

    iValue = (int)cINI.GetValue("CheckCSWhenStoreMoney", "1");
    m_sDevConfigInfo.bCheckCSWhenStoreMoney = (iValue == 1);

    iValue = (int)cINI.GetValue("URJBSupp", 0);
    m_sDevConfigInfo.bURJBSupp = (iValue == 1) ? TRUE : FALSE;

    m_stHWConfig.bUPPoweroffCntlSupp = (int)cINI.GetValue("UPPoweroffCntlSupp", 0) == 1 ? TRUE : FALSE;
    m_stHWConfig.bURJBPoweroffCntlSupp = (int)cINI.GetValue("URJBPoweroffCntlSupp", 0) == 1 ? TRUE : FALSE;

    //挖钞时如下情况发生时 上报错误还是警告 missfeed  cass empty  continuous reject (10)  reject 100 or more in total
    m_stOperationalInfo.bDispErrAsWarning = (int)cINI.GetValue("ReportWarnWhenDispenseMisfeed", "1") == 1;

    m_stOperationalInfo.bShutterCheckBeforeDispense = (int)cINI.GetValue("ShutterCheckBeforeDispense", "0") == 1;

    m_sDevConfigInfo.bSafeDoorStatusSupport = (int)cINI.GetValue("SafeDoorStatusSupport", "0") == 1;
    //BV检验程度，0：不启用 1：默认 2：正常  3：严格
    m_sDevConfigInfo.stCashCountLevel.iVerifiationLevel = (VERIFICATION_LEVEL)(int)cINI.GetValue("VerificationLevelWhenCashCount", "1");
    m_sDevConfigInfo.stStoreMoneyLevel.iVerifiationLevel = (VERIFICATION_LEVEL)(int)cINI.GetValue("VerificationLevelWhenStoreMoney", "1");
    m_sDevConfigInfo.stDispenseLevel.iVerifiationLevel = (VERIFICATION_LEVEL)(int)cINI.GetValue("VerificationLevelWhenDispense", "1");

    //BV检验UnfitNote程度，0：不启用 1：默认 2：正常  3：款松 4：严格 5：全部识别为拒钞
    m_sDevConfigInfo.stCashCountLevel.iUnfitLevel = (UNFIT_LEVEL)(int)cINI.GetValue("VerificationUnfitLevelWhenCashCount", "1");
    m_sDevConfigInfo.stStoreMoneyLevel.iUnfitLevel = (UNFIT_LEVEL)(int)cINI.GetValue("VerificationUnfitLevelWhenStoreMoney", "1");
    m_sDevConfigInfo.stDispenseLevel.iUnfitLevel = (UNFIT_LEVEL)(int)cINI.GetValue("VerificationUnfitLevelWhenDispense", "1");

    //是否将UnFIt钞票存入回收箱
    m_stOperationalInfo.bRejectUnfitNotesStore = ((int)cINI.GetValue("RejectUnfitNoteWhenStoreMoney", "0") == 1);

    //不可接收钞票类型检测时上报方式 Unknown    result of BV indentification
    m_stOperationalInfo.bReportUnacceptDeno = (int)cINI.GetValue("ReportUnacceptableDeno", "0");

    m_sDevConfigInfo.DispenseByDeno = (int)cINI.GetValue("DispenseByDeno", "0");

    //获取冠子码信息功能异常时是否设置BV为故障状态,0：不设置 1：设置 默认为0
    m_sDevConfigInfo.SetBVErrWhenConnectAbnormal = ((int)cINI.GetValue("SetBVErrWhenConnectAbnormal", "0") == 1);

    //是否需要降低验钞废钞率
    m_sDevConfigInfo.stSetVerificationLevel.bSetForCashCount = ((int)cINI.GetValue("ReduceRejectRateCashCount", "0") == 1);//todo
    //是否需要降低存款废钞率
    m_sDevConfigInfo.stSetVerificationLevel.bSetForStoreMoney = ((int)cINI.GetValue("ReduceRejectRateStoreCash", "0") == 1);
    //是否需要降低挖钞废钞率
    m_sDevConfigInfo.stSetVerificationLevel.bSetForDispense = ((int)cINI.GetValue("ReduceRejectRateDispense", "0") == 1);

    m_sDevConfigInfo.dwVerLevDispense = (DWORD)cINI.GetValue("VerificationLevelOfDispense", 200);
    m_sDevConfigInfo.dwVerLevCashCount = (DWORD)cINI.GetValue("VerificationLevelOfCashCount", 200);
    m_sDevConfigInfo.dwVerLevStoreCash = (DWORD)cINI.GetValue("VerificationLevelOfStoreMoney", 200);

    //1-TS，2-AB，3-URJB（开启URJB箱，如没则回收到AB）
    m_sDevConfigInfo.usDefaultRetractArea = (int)cINI.GetValue("DefaultRetractPosition", "1");
    if (m_sDevConfigInfo.usDefaultRetractArea <= 0 ||
        m_sDevConfigInfo.usDefaultRetractArea > 3)
    {
        m_sDevConfigInfo.usDefaultRetractArea = 2;
    }

    LPCSTR szValue = (LPCSTR)cINI.GetValue("FWFilesSavePath", "/usr/local/CFES/DATA/FW");
    strcpy(m_szFWFilesPath,szValue);

    m_sDevConfigInfo.bRejectCounterfeitNote = ((int)cINI.GetValue("RejectCounterfeitNote", "0") == 1);
    m_sDevConfigInfo.usUnfitNotesVerifyMode = (int)cINI.GetValue("UnfitNotesVerifyMode", "0");

    //是否启用处理1999版人民币
    m_sDevConfigInfo.bGenerateCNY1999 = ((int)cINI.GetValue("GenerateCNY1999", "1") == 1);

    m_sDevConfigInfo.bCloseShutterForce = ((int)cINI.GetValue("ForceWhenCloseShutter", "1") == 1);

    //关门失败重试次数
    m_sDevConfigInfo.dwCloseShutterRetryTimes = (DWORD)cINI.GetValue("CloseShutterRetryTimes", 3);  //30-00-00-00(FT#0053)

    //获取AP存款限额配置文件信息
    string str = (LPCSTR)cINI.GetValue("APAmountLimitConfigFileItem", "");          //30-00-00-00(FS#0009)
    if(str.size() > 0){                                                             //30-00-00-00(FS#0009)
        CAutoSplitByStep autoSplitByStep(str.c_str(), ",");                         //30-00-00-00(FS#0009)
        if(autoSplitByStep.Count() >= 3){                                           //30-00-00-00(FS#0009)
            for(int i = 0; i < 3; i++){                                             //30-00-00-00(FS#0009)
                strcpy(m_szApAmountLimitINIItem[i], autoSplitByStep.At(i));         //30-00-00-00(FS#0009)
            }                                                                       //30-00-00-00(FS#0009)
        }                                                                           //30-00-00-00(FS#0009)
    }                                                                               //30-00-00-00(FS#0009)

    //30-00-00-00(FS#0018) add start
    //NoteID合并
    szValue = (LPCSTR)cINI.GetValue("NoteIDGroups", "");
    if (strlen(szValue) > 0) {
        QString qStrValue = szValue;
        QStringList qStrListGroups = qStrValue.split(",");
        foreach (QString qStrGroup, qStrListGroups) {
            if (qStrGroup.length() <= 0) {
                continue;
            }
            QStringList qStrListItems = qStrGroup.split(":");
            if (qStrListItems.size() > 1) {
                BYTE byMainID = (BYTE)qStrListItems[0].toUInt();
                m_sDevConfigInfo.mapNoteIDGroups.insert(std::map<BYTE, BYTE>::value_type(byMainID, byMainID));
                QStringList qStrListSubIDs = qStrListItems[1].split("|");
                foreach(QString qStrID, qStrListSubIDs) {
                    m_sDevConfigInfo.mapNoteIDGroups.insert(std::map<BYTE, BYTE>::value_type((BYTE)qStrID.toUInt(), byMainID));
                }
            }
        }
    }
    //30-00-00-00(FS#0018) add end

    //SN Config
/*30-00-00-00(FS#0030) delete start
    cINI = m_ReaderConfigFile.GetReaderSection("SNInfo");
    //钞票记录方式0：记录全副图片2：只记录冠字码图片，
    m_sDevConfigInfo.stBVDependentMode.iFullImageMode = (RECORD_FULLIMAGE_MODE)(int)cINI.GetValue("FullSNImageRecord", "0");

    //是否启用BV设置模式（0：不启用，1：启用 默认为1）
    m_sDevConfigInfo.stBVDependentMode.bEnableBVModeSetting = ((int)cINI.GetValue("EnableBVModeSetting", "1") == 1);
30-00-00-00(FS#0030) delete end*/
    GetFullImgSNBVSettingInfo();                    //30-00-00-00(FS#0030)

    GetAcceptDispenseRoomsPriorityInfo();           //30-00-00-00(FS#0022)

    //Section SelfCount
    cINI = m_ReaderConfigFile.GetReaderSection("SelfCount");                                                 //30-00-00-00(FS#0022)
    m_stHWConfig.bSelfCountRB5CfgAutoSwitch = (int)cINI.GetValue("SelfCountRB5CfgAutoSwitch", 1);            //30-00-00-00(FS#0022)
    m_stHWConfig.bSelfCountRB5SameDenoLimit = (int)cINI.GetValue("SelfCountRB5SameDenoLimit", 0);            //30-00-00-00(FS#0022)
    m_stHWConfig.bSelfCountDiffDenoAbort = (int)cINI.GetValue("SelfCountDiffDenoAbort", 0);                  //30-00-00-00(FS#0022)

    //获取BV鉴伪级别设定
    cINI = m_ReaderConfigFile.GetReaderSection("ZeroBVSetting");                                                    //30-00-00-00(FS#0025)
    m_sDevConfigInfo.byBVSettingLevel = (int)cINI.GetValue("LevelSetting", 255);                                    //30-00-00-00(FS#0025)
    m_sDevConfigInfo.strBVSettingInfo = (LPCSTR)cINI.GetValue("AgileSetting", "C40/69/60/127/127/254/5/7/127/255/255/254/48,S40/69/60/127/127/254/5/7/127/255/255/254/48,D40/69/60/127/127/254/5/7/127/255/255/254/48");    //30-00-00-00(FS#0025)

    //SNBlacklist
    szValue = (LPCSTR)cINI.GetValue("SNBlacklistFile", "");
    if ((szValue != nullptr) && (strlen(szValue) > 0)) {
        m_sDevConfigInfo.strSNBlacklistFile = string(SPETCPATH) + "/" +szValue;
    }

    InitCapabilities();

    strcpy(m_szSPVersion, VERSION_HCM_SP);

    //设置BV支持的所有类型钞票
    //InitBVSupportNotesType();

    //30-00-00-00(FS#0019) add start
    //SNConfig.ini
    //设置AP指定的获取SP是否支持2018 FSN配置文件配置项
    string strSNConfigFilePath = SPETCPATH;
    strSNConfigFilePath += "/SNConfig.ini";
    CINIFileReader iniFile;
    if(iniFile.LoadINIFile(strSNConfigFilePath.c_str())){
        CINIReader iniReader = iniFile.GetReaderSection("DEFAULT");
        string strAPFSN18SuppConfigFilePath = (LPCSTR)iniReader.GetValue("APFSN18SuppConfigFilePath", "");
        if(!strAPFSN18SuppConfigFilePath.empty()){
            bool bFSN18Supp = (int)iniReader.GetValue("FSN18Sup", 0) == 1;

            if(iniFile.LoadINIFile(strAPFSN18SuppConfigFilePath.c_str())){
                CINIWriter iniWriter = iniFile.GetWriterSection("SPSupport2018FSN");
                iniWriter.SetValue("IfSPSupport2018FSN", bFSN18Supp ? 1 : 0);
            }
        }
    }
    //30-00-00-00(FS#0019) add end

    return WFS_SUCCESS;
}

//功能：初始化进出钞机芯能力
//输入：无
//输出：无
//返回：XFS通用错误码
//说明：
long CUR2Adapter::InitCapabilities()
{
    const char *ThisModule = "InitCapabilities";
    int iValue = 0;
    CINIReader cINI = m_ReaderConfigFile.GetReaderSection("BRMInfo");

//test#3    iValue = (int)cINI.GetValue("CashInMaxNum", "200");
    iValue = (int)cINI.GetValue("CashOutMaxNum", "200");   //test#3
    m_sCDMCapabilities.wMaxDispenseItems = ((iValue <= 200) && (iValue > 0)) ? iValue : 200;
    m_sCDMCapabilities.bShutter = TRUE;
    m_sCDMCapabilities.bShutterControl = TRUE;//固定值 见SR001006
    m_sCDMCapabilities.fwRetractAreas = (WFS_CDM_RA_RETRACT | WFS_CDM_RA_STACKER | WFS_CDM_RA_REJECT);
    m_sCDMCapabilities.fwRetractTransportActions = WFS_CDM_NOTSUPP;
    m_sCDMCapabilities.fwRetractStackerActions = WFS_CDM_RETRACT;
    m_sCDMCapabilities.bSafeDoor = ((int)cINI.GetValue("SafeDoorStatusSupport", "0") == 1);
    m_sCDMCapabilities.bIntermediateStacker = ((int)cINI.GetValue("CDMLogicalStackerSupport", "0") == 1);
    m_sCDMCapabilities.bItemsTakenSensor = TRUE;//
    m_sCDMCapabilities.fwMoveItems = 0;//固定表示不支持

    LPCSTR szValue = (LPCSTR)cINI.GetValue("InOutPutPosition", "RIGHT");
    if (strcmp(szValue, "LEFT") == 0)
    {
        m_sCDMCapabilities.fwPositions = WFS_CDM_POSLEFT;
        m_sCIMCapabilities.fwPositions = WFS_CIM_POSINLEFT | WFS_CIM_POSOUTLEFT;
    }
    else if (strcmp(szValue, "RIGHT")  == 0)
    {
        m_sCDMCapabilities.fwPositions = WFS_CDM_POSRIGHT;
        m_sCIMCapabilities.fwPositions = WFS_CIM_POSINRIGHT | WFS_CIM_POSOUTRIGHT;
    }
    else if (strcmp(szValue, "CENTER") == 0)
    {
        m_sCDMCapabilities.fwPositions = WFS_CDM_POSCENTER;
        m_sCIMCapabilities.fwPositions = WFS_CIM_POSINCENTER | WFS_CIM_POSOUTCENTER;
    }
    else if (strcmp(szValue, "TOP") == 0)
    {
        m_sCDMCapabilities.fwPositions = WFS_CDM_POSTOP;
        m_sCIMCapabilities.fwPositions = WFS_CIM_POSINTOP | WFS_CIM_POSOUTTOP;
    }
    else if (strcmp(szValue, "BOTTOM") == 0)
    {
        m_sCDMCapabilities.fwPositions = WFS_CDM_POSBOTTOM;
        m_sCIMCapabilities.fwPositions = WFS_CIM_POSINBOTTOM | WFS_CIM_POSOUTBOTTOM;
    }
    else if (strcmp(szValue, "FRONT") == 0)
    {
        m_sCDMCapabilities.fwPositions = WFS_CDM_POSFRONT;
        m_sCIMCapabilities.fwPositions = WFS_CIM_POSINFRONT | WFS_CIM_POSOUTFRONT;
    }
    else if (strcmp(szValue, "REAR") == 0)
    {
        m_sCDMCapabilities.fwPositions = WFS_CDM_POSREAR;
        m_sCIMCapabilities.fwPositions = WFS_CIM_POSINREAR | WFS_CIM_POSOUTREAR;
    }
    else
    {
        m_sCDMCapabilities.fwPositions = WFS_CDM_POSLEFT;
        m_sCIMCapabilities.fwPositions = WFS_CIM_POSINLEFT | WFS_CIM_POSOUTLEFT;
    }

    iValue = (int)cINI.GetValue("CashInMaxNum", "200");
    Log(ThisModule,  1, "CashInMaxNum=%d", iValue);
    m_sCIMCapabilities.wMaxCashInItems = ((iValue <= 300) && (iValue >= 0)) ? iValue : 300;
    m_sCIMCapabilities.bShutter = TRUE;
    m_sCIMCapabilities.bShutterControl = ((int)cINI.GetValue("CIMShutterControl", "0") == 1) ;//SR002006
    m_sCIMCapabilities.bSafeDoor = FALSE;
    m_sCIMCapabilities.fwIntermediateStacker = m_sCIMCapabilities.wMaxCashInItems;
    m_sCIMCapabilities.bItemsTakenSensor = TRUE;
    m_sCIMCapabilities.bItemsInsertedSensor = TRUE;
    m_sCIMCapabilities.fwRetractAreas = WFS_CIM_RA_RETRACT | WFS_CIM_RA_STACKER /*| WFS_CIM_RA_REJECT */;
    m_sCIMCapabilities.fwRetractTransportActions = WFS_CIM_NOTSUPP;
    m_sCIMCapabilities.fwRetractStackerActions = WFS_CIM_RETRACT;

    return WFS_SUCCESS;
}

long CUR2Adapter::SetVerLevel()
{
    LPCSTR ThisModule = "SetVerLevel";
    int iRet = m_pUR2Dev->SetVerificationLevel(m_sDevConfigInfo.stSetVerificationLevel);
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)
    {
        Log(ThisModule,  iRet, "SetVerificationLevel Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

long CUR2Adapter::GetBanknoteInfo()
{
    LPCSTR ThisModule = "GetBanknoteInfo";
    ST_BV_INFO pBVInfo;
    ST_DENO_INFO pArryDenoInfo[MAX_DENOMINATION_NUM];
    int iRet = m_pUR2Dev->GetBanknoteInfo(pBVInfo, pArryDenoInfo);
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)
    {
        Log(ThisModule,  iRet, "GetBanknoteInfo Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

long CUR2Adapter::SetDenoCode()
{
    LPCSTR ThisModule = "SetDenoCode";
    char pArryDENOCode[MAX_DENOMINATION_NUM] = {1, 2, 3, 4, 5, 6, 7}; //固定默认配置
    int iRet = m_pUR2Dev->SetDenominationCode(pArryDENOCode) ;
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)
    {
        Log(ThisModule,  iRet, "SetDenominationCode Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

long CUR2Adapter::GetCassetteInfo()
{
    LPCSTR ThisModule = "GetCassetteInfo";
    ST_CASSETTE_INFO pArryCassInfo[MAX_CASSETTE_NUM];
    int iRet = m_pUR2Dev->GetCassetteInfo(pArryCassInfo);
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)
    {
        Log(ThisModule,  iRet, "GetCassetteInfo Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

long CUR2Adapter::SetUnitInfo()
{
    LPCSTR ThisModule = "SetUnitInfo";
    SYSTEMTIME tTime;
    CQtTime::GetLocalTime(tTime);
    ST_OPERATIONAL_INFO stOperationalInfo = m_stOperationalInfo;
    ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM];
    BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM];

    memset(bArryAcceptDenoCode, 0, sizeof(bArryAcceptDenoCode));
    map<BYTE, WFSCIMNOTETYPE>::const_iterator it = m_mapSupportNotID.begin();
    for (; it != m_mapSupportNotID.end(); it++)
    {
        bArryAcceptDenoCode[it->first - 1] = TRUE;
    }
    Log(ThisModule,  1, "ArryDeno: %d|%d|%d|%d|%d|%d|%d|%d|", bArryAcceptDenoCode[0], bArryAcceptDenoCode[1],
        bArryAcceptDenoCode[2], bArryAcceptDenoCode[3], bArryAcceptDenoCode[4], bArryAcceptDenoCode[5], bArryAcceptDenoCode[6], bArryAcceptDenoCode[7]);

    ST_BV_VERIFICATION_LEVEL stCashCountLevel = m_sDevConfigInfo.stCashCountLevel;
    ST_BV_VERIFICATION_LEVEL stStoreMoneyLevel = m_sDevConfigInfo.stStoreMoneyLevel;
    ST_BV_VERIFICATION_LEVEL stDispenseLevel = m_sDevConfigInfo.stDispenseLevel;
    char usArryCassCashInPrioritySet[MAX_ROOM_NUM];                                         //30-00-00-00(FS#0022)
    char usArryCassCashOutPrioritySet[MAX_ROOM_NUM];                                        //30-00-00-00(FS#0022)

    for (int i = 0; i < MAX_ROOM_NUM; i++)                                                  //30-00-00-00(FS#0022)
    {
//30-00-00-00(FS#0022)        usArryCassCashInPrioritySet[i] = 1;
//30-00-00-00(FS#0022)        usArryCassCashOutPrioritySet[i] = 1;
        usArryCassCashInPrioritySet[i] = m_stHWConfig.byAcceptRoomsPriority[i];             //30-00-00-00(FS#0022)
        usArryCassCashOutPrioritySet[i] = m_stHWConfig.byDispenseRoomsPriority[i];          //30-00-00-00(FS#0022)
    }

    int iTotalCashInURJB = 0;
//    ST_HW_CONFIG stHWConfig;

    if (!m_bBRMInitialized)
    {
        for (int i = 0; i < MAX_CASSETTE_NUM ; i++)
        {
            stCassType[i].iCassNO = (CASSETTE_NUMBER)(i + 1);
            stCassType[i].ucVersion = 'B';
            if (i == 3)
            {
                stCassType[i].iCassNoteHandInfo = DESTINATION_REJECT_CASH1_COMBINE;
                stCassType[i].iCassType = CASSETTE_TYPE_AB;
                stCassType[i].iCassOper = AB_OPERATION_DEPREJRET;
                stCassType[i].iDenoCode = DENOMINATION_CODE_ALL;
                continue;
            }
            stCassType[i].iCassType = CASSETTE_TYPE_RB;
            stCassType[i].iCassOper = RB_OPERATION_RECYCLE;
            stCassType[i].iDenoCode = DENOMINATION_CODE_100_C;
            stCassType[i].iCassNoteHandInfo = DESTINATION_REJCET_DEFAULT;
        }
        //启动时假设所有钞箱正常的情况下做一次初始化设置，此后机芯动作就能返回正确的查询结果，
        //并能据此设置与实际钞箱状况正确对应的初始化值
        int iRet = m_pUR2Dev->SetUnitInfo(iTotalCashInURJB, /*tTime,*/ stOperationalInfo,   m_stHWConfig, stCassType, bArryAcceptDenoCode,
                                          stCashCountLevel, stStoreMoneyLevel, stDispenseLevel, usArryCassCashInPrioritySet, usArryCassCashOutPrioritySet,
                                          m_sDevConfigInfo.bURJBSupp);
        UpdateADPDevStatus(iRet);
    }
/*
    bool bNeedSetUnitInfo = false;
    int iTotalCashInURJB = 0;
    BOOL bArryDenoCodeBySet[MAX_DENOMINATION_NUM];
    BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM];
    ST_BV_VERIFICATION_LEVEL stCashCountLevel;
    ST_BV_VERIFICATION_LEVEL stStoreMoneyLevel;
    ST_BV_VERIFICATION_LEVEL stDispenseLevel;
    ST_OPERATIONAL_INFO stOperationalInfo;
    ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM];
    ST_HW_CONFIG stHWConfig;
    iRet = m_pUR2Dev->GetUnitInfo(iTotalCashInURJB, bArryDenoCodeBySet, bArryAcceptDenoCode, stOperationalInfo,
                                      stHWConfig, stCassType, stCashCountLevel, stStoreMoneyLevel, stDispenseLevel);
    if (iRet < 0)
    {
        bNeedSetUnitInfo = true;
    }
    if(stHWConfig.bHaveLane1 != m_aryADPCassSettingInfo[0].bActive ||
       stHWConfig.bHaveLane2 != m_aryADPCassSettingInfo[1].bActive ||
       stHWConfig.bHaveLane3 != m_aryADPCassSettingInfo[2].bActive ||
       stHWConfig.bHaveLane4 != m_aryADPCassSettingInfo[3].bActive ||
       stHWConfig.bHaveLane5 != m_aryADPCassSettingInfo[4].bActive){
        bNeedSetUnitInfo = true;
    }
    if(!bNeedSetUnitInfo){
        return WFS_SUCCESS;
    }
*/
    for (int i = 0; i < MAX_CASSETTE_NUM ; i++)
    {
        stCassType[i].iCassNO = (CASSETTE_NUMBER)(i + 1);
        stCassType[i].ucVersion = 'B';
        if (i == 3)
        {
            if (m_aryADPCassSettingInfo[i].bActive)
            {
                stCassType[i].iCassNoteHandInfo = DESTINATION_REJECT_CASH1_COMBINE;
                stCassType[i].iCassType = CASSETTE_TYPE_AB;
                stCassType[i].iCassOper = AB_OPERATION_DEPREJRET;
                stCassType[i].iDenoCode = DENOMINATION_CODE_ALL;
            }
            continue;
        }

        stCassType[i].iCassNoteHandInfo = DESTINATION_REJCET_DEFAULT;
        if (!m_aryADPCassSettingInfo[i].bActive
            || (m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_MISSING))//钞箱被拔走
        {
            stCassType[i].iCassType = CASSETTE_TYPE_UNLOAD;//如果钞箱未启用，类型必须设为此值
            continue;
        }

        //校验币种是否为CNY, 机芯只支持CNY
        if (memcmp(m_aryADPCassSettingInfo[i].CurrencyID, "CNY", 3) == 0)
        {
            //每个钞箱对应币种
            stCassType[i].iDenoCode = DENOMINATION_CODE(m_aryADPCassSettingInfo[i].usDenoCode);
        }
        else
        {
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱(%d)设置的币种类型不为CNY", i + 1);
            return WFS_ERR_UNSUPP_DATA;
        }

        //校验循环箱支持的钞箱类型是否正确
        switch (m_aryADPCassSettingInfo[i].iCassType)
        {
        case ADP_CASSETTE_BILL:
            stCassType[i].iCassType = CASSETTE_TYPE_RB;
            stCassType[i].iCassOper = RB_OPERATION_DISPENSE;
            break;
        case ADP_CASSETTE_CASHIN :
            stCassType[i].iCassType = CASSETTE_TYPE_RB;
            stCassType[i].iCassOper = RB_OPERATION_DEPOSITE;
            break;
        case ADP_CASSETTE_RETRACT:
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱%d类型不能为回收箱", i + 1);
            return WFS_ERR_UNSUPP_DATA;
        case ADP_CASSETTE_REJECT:
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱%d类型不能为拒钞箱", i + 1);
            return WFS_ERR_UNSUPP_DATA;
        case ADP_CASSETTE_RECYCLING :
            stCassType[i].iCassType = CASSETTE_TYPE_RB;
            stCassType[i].iCassOper = RB_OPERATION_RECYCLE;
            break;
        default:
            char szTemp[128];
            CassType2String(m_aryADPCassSettingInfo[i].iCassType, szTemp);
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱%d类型(%s)错误,不能识别", i + 1, szTemp);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    //判断是否需要发送SetUnitInfo命令
    //30-00-00-00(FT#0010) add start
    bool bNeedSetUnitInfo = false;
    int iTotalCashInURJB_HW = 0;
    BOOL bArryDenoCodeBySet_HW[MAX_DENOMINATION_NUM];
    BOOL bArryAcceptDenoCode_HW[MAX_DENOMINATION_NUM];
    ST_BV_VERIFICATION_LEVEL stCashCountLevel_HW;
    ST_BV_VERIFICATION_LEVEL stStoreMoneyLevel_HW;
    ST_BV_VERIFICATION_LEVEL stDispenseLevel_HW;
    ST_OPERATIONAL_INFO stOperationalInfo_HW;
    ST_CASSETTE_INFO stCassType_HW[MAX_CASSETTE_NUM];
    ST_HW_CONFIG stHWConfig_HW;
    int iRet = m_pUR2Dev->GetUnitInfo(iTotalCashInURJB_HW, bArryDenoCodeBySet_HW, bArryAcceptDenoCode_HW, stOperationalInfo_HW,
                                      stHWConfig_HW, stCassType_HW, stCashCountLevel_HW, stStoreMoneyLevel_HW, stDispenseLevel_HW);
    if (iRet < 0) {
        bNeedSetUnitInfo = true;
    } else {
        for(int i = 0; i < MAX_CASSETTE_NUM - 1; i++){
            if((stCassType_HW[i].iCassType != stCassType[i].iCassType) ||
               (stCassType_HW[i].iCassOper != stCassType[i].iCassOper) ||
               (stCassType_HW[i].iDenoCode != stCassType[i].iDenoCode && stCassType[i].iCassType != CASSETTE_TYPE_AB) ||
               (stCassType_HW[i].iCassNoteHandInfo != stCassType[i].iCassNoteHandInfo)){
                bNeedSetUnitInfo = true;
                break;
            }
        }
    }

    if(!bNeedSetUnitInfo){
        return WFS_SUCCESS;
    }
    //30-00-00-00(FT#0010) add end

    iRet = m_pUR2Dev->SetUnitInfo(iTotalCashInURJB, /*tTime,*/ stOperationalInfo,   m_stHWConfig, stCassType, bArryAcceptDenoCode,
                                      stCashCountLevel, stStoreMoneyLevel, stDispenseLevel, usArryCassCashInPrioritySet, usArryCassCashOutPrioritySet,
                                      m_sDevConfigInfo.bURJBSupp);
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet != 0)
    {
        Log(ThisModule,  iRet, "SetUnitInfo Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

long CUR2Adapter::GetUnitInfo()
{
    LPCSTR ThisModule = "GetUnitInfo";
    int iTotalCashInURJB = 0;
    BOOL bArryDenoCodeBySet[MAX_DENOMINATION_NUM];
    BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM];
    ST_BV_VERIFICATION_LEVEL stCashCountLevel;
    ST_BV_VERIFICATION_LEVEL stStoreMoneyLevel;
    ST_BV_VERIFICATION_LEVEL stDispenseLevel;
    ST_OPERATIONAL_INFO stOperationalInfo;
    ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM];
    ST_HW_CONFIG stHWConfig;
    int iRet = m_pUR2Dev->GetUnitInfo(iTotalCashInURJB, bArryDenoCodeBySet, bArryAcceptDenoCode, stOperationalInfo,
                                      stHWConfig, stCassType, stCashCountLevel, stStoreMoneyLevel, stDispenseLevel);
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)
    {
        Log(ThisModule,  iRet, "GetUnitInfo Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}


//功能：初始化设置机芯
//输入：pArrCassInfo：初始化钞箱配置
//输出：无
//返回：XFS通用错误码
//说明：
long CUR2Adapter::InitSetting()
{
    const char* ThisModule = "InitSetting";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    Log(ThisModule,  0, "InitSetting start");
    long lres = 0;
    if (m_szFWVersion[0] == 0 || m_bIsNeedReset)
    {
        lres = QueryFWVersion();
        if (lres < 0)
            return lres;
        //获取机芯驱动文件名
        QString strDriverFileName;
//        QDir DriverDir("/usr/local/CFES/BIN/");
        QDir DriverDir("/home/cfes/CFES/BIN/");
        QStringList filters;
        filters << "*.so.*.*.*";
        QFileInfoList fileInfoList = DriverDir.entryInfoList(filters);

        for (int i = 0; i < fileInfoList.size(); ++i)
        {
            if (fileInfoList.at(i).fileName().contains("DevUR2"))
            {
                strDriverFileName = fileInfoList.at(i).fileName();
                break;
            }
        }
        //增加第一次reset记录机芯型号 固件版本 驱动文件等信息
        Log(ThisModule, 1, "机芯型号:UR2,固件版本:%s,驱动文件:[%s]", m_szFWVersion, strDriverFileName.toLocal8Bit().data());
    }
    /*
    lres =  GetUnitInfo();
    if (lres < 0)
    {
        return lres;
    }

    lres =  GetBanknoteInfo();
    if (lres < 0)
    {
        return lres;
    }

    lres =  SetDenoCode();
    if (lres < 0)
    {
        return lres;
    }
    */

    lres =  GetBanknoteInfoAndSetDenoCode();
    if (lres < 0)
    {
        return lres;
    }
    lres =  GetCassetteInfo();
    if (lres < 0)
    {
        return lres;
    }


    Log(ThisModule,  1, "InitSetting success");
    return WFS_SUCCESS;
}

long CUR2Adapter::InitUnitSetting()
{
    long lres =  SetUnitInfo();
    if (lres < 0)
    {
        return lres;
    }
    lres =  GetUnitInfo();
    if (lres < 0)
    {
        return lres;
    }
    m_bBRMInitialized = TRUE;
    return lres;
}
long CUR2Adapter::StartBVCommunication()
{
    LPCSTR ThisModule = "StartBVCommunication";
    int iRet = m_pUR2Dev->BVCommStart();
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)
    {
        Log(ThisModule,  iRet, "StartBVCommunication Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

long CUR2Adapter::EndBVCommunication()
{
    LPCSTR ThisModule = "EndBVCommunication";
    int iRet = m_pUR2Dev->BVCommEnd();
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)
    {
        Log(ThisModule,  iRet, "EndBVCommunication Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}
//功能：设置等待完成的接口
//输入：pWait：回调接口
//输出：无
//返回：无
//说明：
/*
void CUR2Adapter::SetWaitCallback (IWaitComplete *pWait)
{
    const char* ThisModule = "SetWaitCallback";
    if (pWait == NULL)
    {
        Log(ThisModule, WFS_ERR_INVALID_POINTER, "设置等待回调指针为空");
        return;
    }

    if (m_pUR2Dev == NULL)
    {
        Log(ThisModule, WFS_ERR_INVALID_POINTER, "驱动库未成功加载");
        return;
    }
    m_pUR2Dev->SetWaitCallback(pWait);
}

//功能：设置状态改变侦听接口
// 参数：
//  pListener：【输入】，回调接口指针，如果NULL，取消侦听
//返回值：TRUE，设置成功，支持状态侦听；否则，不支持状态侦听
BOOL CUR2Adapter::SetStatusChangeListener(IStatusChangeListener *pListener)
{
    const char* ThisModule = "SetStatusChangeListener";
    return 0;
}
*/

//功能：取设备状态
//输入：Status：设备状态
//输出：无
//返回：无
//说明：
long CUR2Adapter::GetStatus(ADPSTATUSINFOR &Status)
{
    LPCSTR ThisModule = "GetStatus";
    if (!m_bConnectNormal || !m_bBRMInitialized)
    {
        //Log(ThisModule, WFS_ERR_HARDWARE_ERROR,"m_bConnectNormal:%d,m_bBRMInitialized:%d", m_bConnectNormal, m_bBRMInitialized);
        m_sDevStatus.stDevice = !m_bConnectNormal ? ADP_DEVICE_HWERROR : ADP_DEVICE_OFFLINE;
        m_sDevStatus.stInPutPosition = ADP_INPOS_UNKNOWN;
        m_sDevStatus.stOutPutPosition = ADP_OUTPOS_UNKNOWN;
        m_sDevStatus.stBanknoteReader = ADP_BR_UNKNOWN;
        m_sDevStatus.stSafeDoor = ADP_SAFEDOOR_NOTSUPPORTED;
        m_sDevStatus.stInShutter = ADP_INSHUTTER_UNKNOWN;
        m_sDevStatus.stOutShutter = ADP_OUTSHUTTER_UNKNOWN;
        m_sDevStatus.stStacker = ADP_STACKER_UNKNOWN;
        m_sDevStatus.stTransport = ADP_TRANSPORT_UNKNOWN;
    }
    else if (m_bIsNeedReset)
    {
        //Log(ThisModule, WFS_ERR_HARDWARE_ERROR,"m_bIsNeedReset = TRUE");
        m_sDevStatus.stDevice = ADP_DEVICE_HWERROR;
    }
    Status = m_sDevStatus;

    if(m_FWDStatus == FWDOWNSTATUS_DLING)
        Status.stDevice = ADP_DEVICE_BUSY;

    return WFS_SUCCESS;
}

//功能：根据设备状态更新适配层内部状态
//输入：iRet: 机芯动作返回的返回值
//输出：无
//返回：无
//说明：内部方法
void CUR2Adapter::UpdateADPDevStatus(int iRet, int iBVRet)
{
    const char *ThisModule = "UpdateADPDevStatus";

    //处理驱动设备状态未读到的情况
    if (iRet != ERR_UR_SUCCESS &&
        iRet != ERR_UR_WARN && //返回成功，正常状态
        iRet != ERR_UR_HARDWARE_ERROR)
    {
        m_bIsNeedReset = TRUE;
        m_sDevStatus.stDevice = ADP_DEVICE_HWERROR;
        m_sDevStatus.stInShutter = ADP_INSHUTTER_UNKNOWN;
        m_sDevStatus.stOutShutter = ADP_OUTSHUTTER_UNKNOWN;
        m_sDevStatus.stInPutPosition = ADP_INPOS_UNKNOWN;
        m_sDevStatus.stOutPutPosition = ADP_OUTPOS_UNKNOWN;
        m_sDevStatus.stBanknoteReader = ADP_BR_UNKNOWN;
        m_sDevStatus.stSafeDoor = ADP_SAFEDOOR_UNKNOWN;
        m_sDevStatus.stStacker = ADP_STACKER_UNKNOWN;
        m_sDevStatus.stTransport = ADP_TRANSPORT_UNKNOWN;
        m_bConnectNormal = FALSE;
        return;
    }

    if (iBVRet < ERR_UR_SUCCESS)
    {
        //m_bIsNeedReset = TRUE;
        //m_sDevStatus.stDevice = ADP_DEVICE_HWERROR;
        //m_bBVConnectNormal = FALSE;
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "iBVRet < ERR_UR_SUCCESS");
    }

    //更新设备状态和钞箱状态
    ST_DEV_STATUS_INFO stDevStatusInfo;
    m_pUR2Dev->GetDevStatusInfo(stDevStatusInfo);

    //处理整机状态
    if (!m_bIsNeedReset) //如果需求复位，则保存以前的整机状态，否则更新状态
    {
        if (!m_bExchangeActive &&
            (m_pErrCodeTrans->IsDeviceNotInPosition(m_szLastErrCode)
             //|| stDevStatusInfo.bURJBOpen
             || stDevStatusInfo.bESCOpen
             || stDevStatusInfo.bBVOpen
             || !stDevStatusInfo.bCassInPos[3]
             || !stDevStatusInfo.bHCMUPInPos
             || !stDevStatusInfo.bHCMLOWInPos
             //|| stDevStatusInfo.bRearDoorOpen
             || !stDevStatusInfo.bESCInPos
             //|| stDevStatusInfo.bFrontDoorOpen
             //|| stDevStatusInfo.bForcedOpenShutter
             //|| stDevStatusInfo.bForcedRemovCashInCS
             || !stDevStatusInfo.bCSInPos
             || stDevStatusInfo.bReqReset))
        {
            m_sDevStatus.stDevice = ADP_DEVICE_USERERROR;
            m_bIsNeedReset = TRUE;            
            Log(ThisModule, 0, "IsDeviceNotInPosition:%d", m_pErrCodeTrans->IsDeviceNotInPosition(m_szLastErrCode));
            Log(ThisModule, 0, "stDevStatusInfo.bESCOpen:%d", stDevStatusInfo.bESCOpen);
            Log(ThisModule, 0, "stDevStatusInfo.bBVOpen:%d", stDevStatusInfo.bBVOpen);
            Log(ThisModule, 0, "stDevStatusInfo.bCassInPos[3]:%d", stDevStatusInfo.bCassInPos[3]);
            Log(ThisModule, 0, "stDevStatusInfo.bHCMUPInPos:%d", stDevStatusInfo.bHCMUPInPos);
            Log(ThisModule, 0, "stDevStatusInfo.bHCMLOWInPos:%d", stDevStatusInfo.bHCMLOWInPos);
            Log(ThisModule, 0, "stDevStatusInfo.bRearDoorOpen:%d", stDevStatusInfo.bRearDoorOpen);
            Log(ThisModule, 0, "stDevStatusInfo.bESCInPos:%d", stDevStatusInfo.bESCInPos);
            Log(ThisModule, 0, "stDevStatusInfo.bFrontDoorOpen:%d", stDevStatusInfo.bFrontDoorOpen);
            Log(ThisModule, 0, "stDevStatusInfo.bCSInPos:%d", stDevStatusInfo.bCSInPos);
            Log(ThisModule, 0, "stDevStatusInfo.bReqReset:%d", stDevStatusInfo.bReqReset);
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "m_sDevStatus.stDevice = ADP_DEVICE_USERERROR");
        }
        else if (iRet < 0)
        {
            m_sDevStatus.stDevice = ADP_DEVICE_HWERROR;
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "iRet < 0");
        }
        else
        {
            m_sDevStatus.stDevice = ADP_DEVICE_ONLINE;
        }

        if (iRet < 0) //返回码小于0必定需要复位
        {
            m_bIsNeedReset = TRUE;
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "iRet < 0 m_bIsNeedReset = TRUE");
        }
    }

    //设置错误码
    if(0 == strcmp(m_szLastErrCodeAction, NORMAL_ERRCODE) ||
       0 == strcmp(m_szLastErrCodeAction, UP_UNIT_NOT_IN_POS) ||
       0 == strcmp(m_szLastErrCodeAction, LOW_UNIT_NOT_IN_POS) ||
       0 == strcmp(m_szLastErrCodeAction, NEED_RESET)){
        if(!stDevStatusInfo.bHCMUPInPos){
            strcpy(m_szLastErrCodeAction, UP_UNIT_NOT_IN_POS);
        } else if(!stDevStatusInfo.bHCMLOWInPos){
            strcpy(m_szLastErrCodeAction, LOW_UNIT_NOT_IN_POS);
        } else if(stDevStatusInfo.bReqReset){
            strcpy(m_szLastErrCodeAction, NEED_RESET);
        }
    }

    //处理Shutter状态
    if (m_sCDMCapabilities.bShutter
        || m_sCIMCapabilities.bShutter)
    {
        //处理门阻塞状态
        if (m_pErrCodeTrans->IsOpenShutterJammed(m_szLastErrCode))
        {
            m_bOpenShutterJammed = TRUE;
        }
        else if (m_pErrCodeTrans->IsCloseShutterJammed(m_szLastErrCode))
        {
            m_bCloseShutterJammed = TRUE;
        }

        if (m_bCloseShutterJammed || m_bOpenShutterJammed)
        {
            m_sDevStatus.stInShutter = ADP_INSHUTTER_JAMMED;
            m_sDevStatus.stDevice = ADP_DEVICE_HWERROR;
        }
        else if (stDevStatusInfo.iOutShutterStatus == UR2SHUTTER_STATUS_CLOSED
                 /*stDevStatusInfo.iInnShutterStatus == UR2SHUTTER_STATUS_CLOSED*/)//门处于关闭状态并且 阻塞标识已清除
        {
            m_sDevStatus.stInShutter = ADP_INSHUTTER_CLOSED;
            m_sDevStatus.stOutShutter = ADP_OUTSHUTTER_CLOSED;
        }
        else //门处于打开状态并且 阻塞标识已清除
        {
            m_sDevStatus.stInShutter = ADP_INSHUTTER_OPEN;
            m_sDevStatus.stOutShutter = ADP_OUTSHUTTER_OPEN;
        }
    }
    else
    {
        m_sDevStatus.stInShutter = ADP_INSHUTTER_NOTSUPPORTED;
        m_sDevStatus.stOutShutter = ADP_OUTSHUTTER_NOTSUPPORTED;
    }


    // 获取安全门状态  SafeDoorStatusSupport (SR002005)
    if (m_sDevConfigInfo.bSafeDoorStatusSupport)
    {
        //CINIReader cINI = m_ReaderConfigFile.GetReaderSection("BRMInfo");
        // read_profile_int("default","safe_switch", 0, "/etc/ndt/sp/ndt_siu_controlbox_sp.ini");
//30-00-00-00(FS#0004)        DWORD dwIniValue = 1;

//30-00-00-00(FS#0004)         if (dwIniValue == 2)
//30-00-00-00(FS#0004)         {
//30-00-00-00(FS#0004)             m_sDevStatus.stSafeDoor = ADP_SAFEDOOR_OPEN;
//30-00-00-00(FS#0004)         }
//30-00-00-00(FS#0004)         else if (dwIniValue == 1)
//30-00-00-00(FS#0004)         {
//30-00-00-00(FS#0004)             m_sDevStatus.stSafeDoor = ADP_SAFEDOOR_CLOSED;
//30-00-00-00(FS#0004)         }
//30-00-00-00(FS#0004)         else
//30-00-00-00(FS#0004)         {
//30-00-00-00(FS#0004)             m_sDevStatus.stSafeDoor = ADP_SAFEDOOR_UNKNOWN;
//30-00-00-00(FS#0004)         }

        //30-00-00-00(FS#0004) add start        
        if(m_bSIUOpen){
            DEVSIUSTATUS stSiuStatus;
            if(m_pDevSIU->GetStatus(stSiuStatus) == 0){
                m_sDevStatus.stSafeDoor = stSiuStatus.wDoors[DOOR_SAFE] == DOOR_OPEN ?
                                          ADP_SAFEDOOR_OPEN : ADP_SAFEDOOR_CLOSED;
            } else {
                m_sDevStatus.stSafeDoor = ADP_SAFEDOOR_UNKNOWN;
            }
        }
        //30-00-00-00(FS#0004) add end
    }
    else
    {
        m_sDevStatus.stSafeDoor = ADP_SAFEDOOR_NOTSUPPORTED;
    }  

    if (stDevStatusInfo.bCashAtCS
        || stDevStatusInfo.bCashAtCSErrPos
        || stDevStatusInfo.bCashAtShutter
        || stDevStatusInfo.bCashLeftInCS)
    {
        m_sDevStatus.stInPutPosition = ADP_INPOS_NOTEMPTY;
        m_sDevStatus.stOutPutPosition = ADP_OUTPOS_NOTEMPTY;
    }
    else
    {
        m_sDevStatus.stInPutPosition = ADP_INPOS_EMPTY;
        m_sDevStatus.stOutPutPosition = ADP_OUTPOS_EMPTY;
        m_bCashOutBeforePresent = FALSE;
    }

    m_sDevStatus.stShutterPosition = ADP_SHUTPOS_EMPTY;
    if(stDevStatusInfo.bCashAtShutter){
        m_sDevStatus.stShutterPosition = ADP_SHUTPOS_NOTEMPTY;
    }

    // 获取暂存区状态
    if (stDevStatusInfo.bESCEmpty)
    {
        m_sDevStatus.stStacker = ADP_STACKER_EMPTY;
    }
    else if (stDevStatusInfo.bESCFull || stDevStatusInfo.bESCNearFull)
    {
//30-00-00-00(FT#0029)   m_sDevStatus.stStacker = ADP_STACKER_NOTEMPTY;
        m_sDevStatus.stStacker = ADP_STACKER_FULL;      //30-00-00-00(FT#0029)
    }
    else
    {
        m_sDevStatus.stStacker = ADP_STACKER_NOTEMPTY;
    }

/*30-00-00-00(FT#0011) del start
    if (m_bCashOutBeforePresent
        && m_sDevStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY
        && m_sCDMCapabilities.bIntermediateStacker)
    {
        m_sDevStatus.stStacker = ADP_STACKER_NOTEMPTY;
    }
30-00-00-00(FT#0011) del end*/
    // 获取传输通道状态
    m_sDevStatus.stTransport = ADP_TRANSPORT_EMPTY;


    // 获取钞票鉴别模块状态
    if (stDevStatusInfo.bBVOpen
        || (!m_bBVConnectNormal
            && m_sDevConfigInfo.SetBVErrWhenConnectAbnormal))
    {
        m_sDevStatus.stBanknoteReader = ADP_BR_INOP;
    }
    else
    {
        m_sDevStatus.stBanknoteReader = ADP_BR_OK;
    }
    UpdateADPCassStatus(stDevStatusInfo);
}

//功能：机芯动作后更新钞箱状态
//输入：无
//输出：无
//返回：无
//说明：内部方法
void CUR2Adapter::UpdateADPCassStatus(const ST_DEV_STATUS_INFO &stDevStatusInfo)
{
    for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        if (!stDevStatusInfo.bCassInPos[i])
        {
            m_aryADPCassStatusInfo[i] = ADP_CASHUNIT_MISSING;
        }
        else
        {
            switch (stDevStatusInfo.CassStatus[i])
            {
            case CASSETTE_STATUS_NORMAL:
                m_aryADPCassStatusInfo[i] = ADP_CASHUNIT_OK;
                break;
            case CASSETTE_STATUS_FULL:
                m_aryADPCassStatusInfo[i] = ADP_CASHUNIT_FULL;
                break;
            case CASSETTE_STATUS_NEAR_EMPTY:
            case  CASSETTE_STATUS_NEAREST_EMPTY:
                m_aryADPCassStatusInfo[i] = ADP_CASHUNIT_LOW;
                break;
            case CASSETTE_STATUS_EMPTY:
                m_aryADPCassStatusInfo[i] = ADP_CASHUNIT_EMPTY;
                break;
            case CASSETTE_STATUS_NEAR_FULL:
                m_aryADPCassStatusInfo[i] = ADP_CASHUNIT_HIGH;
                break;
            default:
                m_aryADPCassStatusInfo[i] = ADP_CASHUNIT_OK;
                break;
            }
        }
    }

    //URJB单独处理
    m_aryADPCassStatusInfo[ADP_MAX_CU_SIZE - 1] = ADP_CASHUNIT_UNKNOWN;

}


//功能：查询机芯状态信息
//输入：无
//输出：无
//返回：XFS通用错误码
//说明：内部方法
long CUR2Adapter::QueryDevInfo()
{
    const char *ThisModule = "QueryDevInfo";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)

    int iRet = 0;
    int iTotalCashInURJB = 0;
    BOOL bArryDenoCodeBySet[MAX_DENOMINATION_NUM];
    BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM] = {FALSE};
    ST_OPERATIONAL_INFO stOperationalInfo;
    ST_HW_CONFIG stHWConfig;
    ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM];
    ST_BV_VERIFICATION_LEVEL stCashCountLevel;
    ST_BV_VERIFICATION_LEVEL stStoreMoneyLevel;
    ST_BV_VERIFICATION_LEVEL stDispenseLevel;
    iRet = m_pUR2Dev->GetUnitInfo(iTotalCashInURJB, bArryDenoCodeBySet, bArryAcceptDenoCode, stOperationalInfo,
                                  stHWConfig, stCassType, stCashCountLevel, stStoreMoneyLevel, stDispenseLevel);

    SNOTESERIALINFO sNoteSNInfo;
    int i = m_pUR2Dev->GetNoteSeiralInfo(0, &sNoteSNInfo);

    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet, i);
    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR,
            "获取驱动信息失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return WFS_SUCCESS;
}

//功能：更新状态，该函数会访问设备，仅在不调用其他动作函数时更新状态
//输入：无
//输出：无
//返回：无
//说明：
long CUR2Adapter::UpdateStatus()
{
    const char *ThisModule = "UpdateStatus";
    return QueryDevInfo();
}

//功能：获取出钞机芯能力
//输入：无
//输出：lpCaps：出钞机芯能力结构指针,定义同xfs的CDM部分。
//返回：XFS通用错误码
//说明：[配置项在SP启动时读取，运行过程修改配置项不影响返回值]
long CUR2Adapter::GetCDMCapabilities(LPADPCDMCAPS lpCaps)
{
    const char *ThisModule = "GetCDMCapabilities";
    VERIFYPOINTNOTEMPTY(lpCaps)
    *lpCaps = m_sCDMCapabilities;
    return WFS_SUCCESS;
}

//功能：获取进钞机芯能力
//输入：无
//输出：lpCaps：进钞机芯能力结构指针,定义同xfs的CIM部分。
//返回：XFS通用错误码
//说明：[配置项在SP启动时读取，运行过程修改配置项不影响返回值]
long CUR2Adapter::GetCIMCapabilities(LPADPCIMCAPS lpCaps)
{
    const char *ThisModule = "GetCIMCapabilities";
    VERIFYPOINTNOTEMPTY(lpCaps)
    *lpCaps = m_sCIMCapabilities;
    return WFS_SUCCESS;
}

//功能：检查 SP设置的币种ID 是否已存在可支持币种ID列表中
//输入：SupportNotID：设置的币种信息
//输出：无
//返回：XFS通用错误码
//说明：内部方法
BOOL CUR2Adapter::NoteIsSupported(const ADP_NOTETYPE &SupportNotID) const
{
    map<BYTE, WFSCIMNOTETYPE>::const_iterator it;
    for (it = m_mapSupportNotID.begin(); it != m_mapSupportNotID.end(); it++)
    {
        if (SupportNotID.CIMNoteType.ulValues == it->second.ulValues &&
            SupportNotID.CIMNoteType.usRelease == it->second.usRelease &&
            memcmp(SupportNotID.CIMNoteType.cCurrencyID, it->second.cCurrencyID, 3) == 0)
        {
            return TRUE;
        }
    }
    return FALSE;
}


//功能：比较SP设置的币种是否存在，并返回面额代号
//输入：lpNotesType：币种信息
//输出：无
//返回：无
//说明：内部方法
USHORT CUR2Adapter::GetHTIDOfNoteType(const LPWFSCIMNOTETYPE lpNotesType) const
{
    const char *ThisModule = "GetHTIDOfNoteType";
    assert(lpNotesType != NULL);
    map<BYTE, WFSCIMNOTETYPE>::const_iterator it;
    for (it = m_mapBVSupportNoteID.begin(); it != m_mapBVSupportNoteID.end(); it++)
    {
        if (lpNotesType->usNoteID == it->second.usNoteID &&
            lpNotesType->usNoteID == NOTE_ID_UNKNOWN)
        {
            assert(it->first > 0);
            return it->first;
        }
        else if ((it->second.usNoteID == NOTE_ID_UNKNOWN) &&
                 ((memcmp(lpNotesType->cCurrencyID, "   ", 3) == 0) ||
                  (lpNotesType->ulValues == 0)))
        {
            return it->first;
        }
        else if ((lpNotesType->ulValues == it->second.ulValues) &&
                 (lpNotesType->usRelease == it->second.usRelease) &&
                 (memcmp(lpNotesType->cCurrencyID, it->second.cCurrencyID, 3) == 0))
        {
            assert(it->first > 0);
            return it->first;
        }
    }
    return 0;
}

//功能：设置BV支持的钞票类型
//输入：lpList：整机支持的钞票列表,同XFS定义
//输出：无
//返回：XFS通用错误码及CIM错误码
//说明： SP必须在初始化时设置钞箱信息前调用一次
long CUR2Adapter::SetSupportNoteTypes(const LPWFSCIMNOTETYPELIST lpList)
{
    const char *ThisModule = "SetSupportNoteTypes";
    VERIFYPOINTNOTEMPTY(lpList)

    if (lpList->usNumOfNoteTypes <= 0)
    {
        Log(ThisModule, WFS_ERR_INVALID_DATA, "钞票ID列表为空");
        return WFS_ERR_INVALID_DATA;
    }
    //每次重新设置前清空数组
    m_mapSupportNotID.clear();
    LPWFSCIMNOTETYPE *ppCIMNoteType  = lpList->lppNoteTypes;
    for (USHORT i = 0; i < lpList->usNumOfNoteTypes; i++)
    {
        if(lpList->lppNoteTypes[i]->usNoteID == NOTE_ID_UNKNOWN){       //30-00-00-00(FS#0021)
            continue;                                                   //30-00-00-00(FS#0021)
        }                                                               //30-00-00-00(FS#0021)
        ADP_NOTETYPE SupportNotID;
        BYTE usSupportNoteCode = GetHTIDOfNoteType(ppCIMNoteType[i]);
        //校验 币种 面额 版本是否被机芯支持
        if (usSupportNoteCode == 0)
        {
            Log(ThisModule, WFS_ERR_UNSUPP_DATA,
                "ID%d类型钞票不被支持", ppCIMNoteType[i]->usNoteID);
            return WFS_ERR_UNSUPP_DATA;
        }
        SupportNotID.CIMNoteType = *ppCIMNoteType[i];
        SupportNotID.usDenoCode = usSupportNoteCode;
        //检查该钞票ID项是否已存在
        if (NoteIsSupported(SupportNotID))
        {
            Log(ThisModule, WFS_ERR_UNSUPP_DATA,
                "ID%d类型钞票已经设置", ppCIMNoteType[i]->usNoteID);
            return WFS_ERR_UNSUPP_DATA;
        }
        m_mapSupportNotID[usSupportNoteCode] = *ppCIMNoteType[i];
    }

    return WFS_SUCCESS;
}

//功能：设置钞箱是否启用
//输入：bEnable：大小为5的数组,指示各钞箱是否启用,出钞、存款交易前调用。
//输出：无
//返回：XFS通用错误码
//说明：
long CUR2Adapter::EnableCashUnit(const BOOL bEnable[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "EnableCashUnit";
    VERIFYPOINTNOTEMPTY(bEnable)

    for (int i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        m_arybEnableCassUnit[i] = bEnable[i];
    }
    return WFS_SUCCESS;
}

//功能：指定各出钞箱出钞数,进行点钞
//输入：aryulItems,各钞箱要挖钞张数
//输出：aryulDispenseCount：每个钞箱点出的钞数,不含废钞数；
//输出：aryulRejects：对于取款箱,表示废钞数；对于回收箱,表示回收数,理论上,所有废钞箱、回收箱的Reject数总和应等于出钞箱、循环箱的Reject数之和。
//输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
//说明：无
long CUR2Adapter::Dispense(const ULONG aryulItems[ADP_MAX_CU_SIZE],
                           ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                           ULONG aryulRejects[ADP_MAX_CU_SIZE],
                           ADP_CUERROR  arywCUError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "Dispense";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev);
    VERIFYPOINTNOTEMPTY(aryulItems);
    VERIFYPOINTNOTEMPTY(aryulDispenseCount);
    VERIFYPOINTNOTEMPTY(aryulRejects);
    VERIFYPOINTNOTEMPTY(arywCUError);
    AutoLogFuncBeginEnd();
    SETCASSUNITUNUSED(arywCUError)
    ZeroMemory(m_szRejectBill, sizeof(m_szRejectBill));

    //检查是否具有出钞能力
    int iRet = IsDeviceDispensable(aryulItems, arywCUError);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "设备不满足出钞能力");
        return iRet;
    }

    //从物理钞箱挖钞
    ULONG iRejBillPerDenAll[ADP_MAX_DENOMINATION_NUM] = {0};
    iRet = DispenseFromAllCass(aryulItems, aryulDispenseCount, aryulRejects, arywCUError, iRejBillPerDenAll);

    //记录出钞日志
    Log(ThisModule, (iRet == WFS_SUCCESS) ? 0 : -1,
        "出钞%s: 应出钞：cass1(%d),cass2(%d),cass3(%d),cass4(%d), cass5(%d), 实际出钞: cass1(%d),cass2(%d),cass3(%d),cass4(%d),cass5(%d)",
        (iRet == WFS_SUCCESS) ? "成功" : "失败", aryulItems[0], aryulItems[1], aryulItems[2], aryulItems[3], aryulItems[4],
        aryulDispenseCount[0], aryulDispenseCount[1], aryulDispenseCount[2], aryulDispenseCount[3], aryulDispenseCount[4]);
    memset(m_aryRetractCount, 0, sizeof(m_aryRetractCount));

    if (iRet != WFS_SUCCESS)
    {
        //如果数目不相符，则回收已经在出钞口的钞票, 回收的钞票作为拒钞数返回
        //        if (m_sDevStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY
        //            || m_sDevStatus.stStacker == ADP_STACKER_NOTEMPTY)
        //        {
        //            int iRetRetract = RetractNoteInCSAfterDispense(aryulDispenseCount, aryulRejects, iRejBillPerDenAll, arywCUError);
        //            Log(ThisModule, iRetRetract, "回收已挖钞票%s", iRetRetract < 0 ? "失败":"成功");
        //            //可能挖钞后失败回收也失败，按面额记录的回收数也必须清零
        //            if (aryulRejects[0] == 0)
        //            {
        //                ZeroMemory(iRejBillPerDenAll, sizeof(iRejBillPerDenAll));
        //            }
        //        }
    }

    m_aryRetractCount[3] = aryulRejects[3];
    SaveRejectBill(iRejBillPerDenAll);
    m_bCashOutBeforePresent = TRUE;
    return iRet;
}

//功能：回收Dispense后在出钞口的钞票
//输入： aryulDispenseCount：每个钞箱的出钞张数
//输出：aryulRejects:每个钞箱的拒钞张数
//       iRejBillPerDenAll：每种面额的拒钞张数
//       arywCUError: 钞箱错误类型
//返回：XFS通用错误码及CDM错误码
//说明：内部方法
long CUR2Adapter::RetractNoteInCSAfterDispense(ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                                               ULONG aryulRejects[ADP_MAX_CU_SIZE],
                                               ULONG iRejBillPerDenAll[ADP_MAX_CU_SIZE],
                                               ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "RetractNoteInCSAfterDispense";
    assert(aryulDispenseCount != NULL);
    assert(aryulRejects != NULL);
    assert(iRejBillPerDenAll != NULL);
    assert(arywCUError != NULL);

    //如果发生硬件错误，要复位一次才能正常回收
    USHORT usRejectArea = 4;//回收到废钞箱
    USHORT usIndex = 1;

    int iRet = Init(arywCUError);
    if (iRet < 0)
    {
        Log(ThisModule, iRet,   "复位错误");
        return iRet;
    }

    iRet = RetractNotes(usRejectArea, usIndex, arywCUError);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "回收已挖出的钞票时发生错误");
        memcpy(iRejBillPerDenAll, m_aryulRetractCountPerDeno, sizeof(ULONG) * ADP_MAX_DENOMINATION_NUM);
        return iRet;
    }

    int TotalRetractCount = 0;
    for (int i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        if (aryulRejects[i] > 0)
        {
            TotalRetractCount += aryulRejects[i];
        }

        if (aryulDispenseCount[i] > 0)
        {
            TotalRetractCount += aryulDispenseCount[i];
            if (i != 0)
            {
                aryulRejects[i] += aryulDispenseCount[i];
            }
            iRejBillPerDenAll[m_aryADPCassSettingInfo[i].usDenoCode - 1] += aryulDispenseCount[i];
            aryulDispenseCount[i] = 0;

        }
    }
    aryulRejects[3] += m_aryRetractCount[3];

    if (TotalRetractCount != m_aryRetractCount[3])
    {
        Log(ThisModule, iRet, "回收已挖出的钞票总数(%d)和已挖出的钞票总数(%d)不一致", m_aryRetractCount[0], TotalRetractCount);
    }
    return WFS_SUCCESS;
}

//功能：开始进入存款模式
//输入：无
//输出：无
//返回：XFS通用错误码及CIM错误码
//说明：
long CUR2Adapter::StartCashIn()
{
    const char *ThisModule = "StartCashIn";
//30-00-00-00(FS#0009) add start
    //获取ap ini配置入金限额值
    iAmountLimitForAPTmp = 0;
    if(strlen(m_szApAmountLimitINIItem[0]) > 0 && strlen(m_szApAmountLimitINIItem[1]) > 0 &&
       strlen(m_szApAmountLimitINIItem[2]) > 0){
        //判断文件是否存在
        QFileInfo info(m_szApAmountLimitINIItem[0]);
        if (info.exists())
        {
            CINIFileReader iniFile;
            iniFile.LoadINIFile(m_szApAmountLimitINIItem[0]);
            CINIReader iniReader = iniFile.GetReaderSection(m_szApAmountLimitINIItem[1]);
            iAmountLimitForAPTmp = (int)iniReader.GetValue(m_szApAmountLimitINIItem[2], 10000);

            //更新sp相关配置
            CINIWriter cINI = m_ReaderConfigFile.GetWriterSection("BRMInfo");
            cINI.SetValue("CashCountMaxAmount", iAmountLimitForAPTmp);
        } else {
            iAmountLimitForAPTmp = 10000;
            Log(ThisModule, __LINE__, "%s not exist", m_szApAmountLimitINIItem[0]);
        }
    }
//30-00-00-00(FS#0009) add end

/* 30-00-00-00(FS#0009) del start
    CINIReader cINICash = m_ReaderCashAcceptorFile.GetReaderSection("CFG");         //test#5
    iAmountLimitForAP = (LPCSTR)cINICash.GetValue("OnceAceeptCashAmount", "0");     //test#5
     Log(ThisModule, 0, "iAmountLimitForAP 入金限额：%d", iAmountLimitForAP);        //test#5
     if(((strcmp(iAmountLimitForAP ,"a") >= 0) &&                                   //test#5
           (strcmp(iAmountLimitForAP ,"z") <= 0))||                                 //test#5
      ((strcmp(iAmountLimitForAP ,"A") >= 0) &&                                     //test#5
        (strcmp(iAmountLimitForAP ,"Z") <= 0)))                                     //test#5
     {                                                                              //test#5
         Log(ThisModule, -1,                                                        //test#5
             "配置项OnceAceeptCashAmount未正确配置，该值小于0或不为数字!");               //test#5
         return WFS_ERR_INVALID_DATA;                                               //test#5
     }                                                                              //test#5

     if ( !(strcmp(iAmountLimitForAP,"0") >= 0) &&                                  //test#5
           ( strcmp(iAmountLimitForAP,"-1") > 0))                                   //test#5
     {                                                                              //test#5
         Log(ThisModule, -1,                                                        //test#5
             "配置项OnceAceeptCashAmount未正确配置，该值小于0或不为数字!");               //test#5
         return WFS_ERR_INVALID_DATA;                                               //test#5
     }                                                                              //test#5

     iAmountLimitForAPTmp = atoi(iAmountLimitForAP);                                //test#5
30-00-00-00(FS#0009) del end*/
    return WFS_SUCCESS;
}

//功能：验钞
//输入：无
//输出：pResult：各钞票种类张数信息,单条数据的格式为"[钞票ID:char6][张数:char6]"样式.
//               例：ID为1的钞票16张,ID为2的钞票20张："000001000016000002000020",pResult指向内存由调用者分配管理.
//输出：ulRejCount:废钞张数
//输出：iErrReject: 拒钞发生原因
//返回：XFS通用错误码及CIM错误码
//说明：
long CUR2Adapter::ValidateAndCounting(char pResult[256], ULONG &ulRejCount, ADP_ERR_REJECT &iErrReject, ULONG ulMaxNumber, ULONG ulMaxAmount)  //30-00-00-00(FS#0002)
{
    const char *ThisModule = "ValidateAndCounting";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    VERIFYPOINTNOTEMPTY(pResult)

    //清返回数据
    iErrReject = ADP_ERR_REJECT_OK;
    ulRejCount = 0;
    ZeroMemory(pResult, sizeof(char) * 256);

    SetVerificationLevel(SAVESERIALOPERAT_C);
    SetRejectCounterfeitNote();
    SetUnfitnoteVerificationLevel(SAVESERIALOPERAT_C);

    //执行点钞操作

    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    int nRejectNotes = 0;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    USHORT usFedNoteCondition = 0;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH];
    BOOL bArryAcceptDeno[MAX_DENOMINATION_NUM] = {FALSE};
    for (int i = 0; i < MAX_DENOMINATION_NUM; i++)
    {
        map<BYTE, WFSCIMNOTETYPE>::iterator it = m_mapSupportNotID.find(i + 1);
        if (it != m_mapSupportNotID.end()
            && it->second.bConfigured)
        {
            bArryAcceptDeno[i] = TRUE;
        }
    }

    CINIReader cINI = m_ReaderConfigFile.GetReaderSection("BRMInfo");
    int iAmountLimit = (int)cINI.GetValue("CashCountMaxAmount", "0");

    if(iAmountLimitForAPTmp != 0){                          //test#5   //30-00-00-00(FS#0009)
        iAmountLimit = iAmountLimitForAPTmp;                //test#5
    }                                                       //test#5
    int iAmount = iAmountLimit / 10;
    int iValue = (int)cINI.GetValue("CashInMaxNum", "200");
    NOTE_POWER_INDEX iPowerIndex = POWER_INDEX_1;

    if(ulMaxAmount > 0 && ulMaxAmount != (ULONG)0xFFFFFFFF){//30-00-00-00(FS#0002)
        iAmount = ulMaxAmount / 10;                         //30-00-00-00(FS#0002)
    }                                                       //30-00-00-00(FS#0002)
    if(ulMaxNumber > 0 && ulMaxNumber != (ULONG)0xFFFFFFFF){//30-00-00-00(FS#0002)
        iValue = ulMaxNumber;                               //30-00-00-00(FS#0002)
    }                                                       //30-00-00-00(FS#0002)

    Log(ThisModule, 0, "CashCount 最大张数：%d, 最大金额为:%d", iValue, iAmountLimit);
    m_sDevConfigInfo.stBVDependentMode = m_sDevConfigInfo.stCashInBVDependentMode;              //30-00-00-00(FS#0030)
    m_pUR2Dev->SetZeroBVSettingInfo(BVSET_CMDTYPE_CASHCOUNT, m_sDevConfigInfo.byBVSettingLevel, m_sDevConfigInfo.strBVSettingInfo.c_str());     //30-00-00-00(FS#0025)
    int iRet = m_pUR2Dev->CashCount(m_sDevConfigInfo.CashInNoteType,  iValue, iAmount, iPowerIndex,
                                    bArryAcceptDeno, m_sDevConfigInfo.stBVDependentMode, iTotalCashInURJB,
                                    iNumStackedToCS, iNumStackedToESC, pNumStackedToPerCass, iNumCSFed, iNumESCFed, pNumPerCassFed,
                                    nRejectNotes, stStackeNotesDenoInfo, usFedNoteCondition, pMaintenanceInfo);
    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "验钞");
    UpdateADPDevStatus(iRet);

    Log(ThisModule, iRet < 0 ? iRet : 1, "CS点钞数：%d, 暂存箱存钞数：%d，拒钞数:%d, CS存钞数:%d",
        iNumCSFed, iNumStackedToESC, nRejectNotes, iNumStackedToCS);
    //保存序列号，钞票过了BV就应该有序列号记录，所以也要尝试记录
 //test#4   if (m_sDevConfigInfo.CashInNoteType == VALIDATION_MODE_REAL)
 //test#4    {
        CThreadGenerateSN SNWork(SAVESERIALOPERAT_C, m_eLfsCmdId, iRet);    //test#8
        QueryNoteMediaInfo(&SNWork);
        QueryNoteSerialInfo(&SNWork);
 //test#4     }

    //处理关门失败
    if (m_pErrCodeTrans->IsCloseShutterJammed(m_szLastErrCode))
    {
        Log(ThisModule, WFS_ERR_CIM_SHUTTERNOTCLOSED,
            "CS门口有钞票或其他杂物挡住导致关门错误(ErrCode:%s)(ErrDesc:%s)(ReturnValue:%d)",
            m_szLastErrCode, m_pUR2Dev->GetErrDesc(m_szLastErrCode), iRet);
        return WFS_ERR_CIM_SHUTTERNOTCLOSED;
    }

    if ((m_sDevStatus.stOutPutPosition == ADP_INPOS_NOTEMPTY) || (iRet > 0))
    {
        //分析拒钞原因，
        m_pErrCodeTrans->GetRejectCauseOfCashCount(m_szLastErrCode, iErrReject);
    }

    //保存点钞数据
    ulRejCount = nRejectNotes;
    int ioffset = 0;
    int iCassOffset[MAX_CASSETTE_NUM] = {0};
    for (int j = 0; j < MAX_DENOMINATION_NUM ; j++)
    {
        if (stStackeNotesDenoInfo.stStackeNotesInfo[j].usNumberStack <= 0)
        {
            continue;
        }

        if (iRet < 0)
        {
            Log(ThisModule, 0, "Index:%d, NOTEID:%d, NOTESNUM:%d, CASS:%d", j,
                stStackeNotesDenoInfo.stStackeNotesInfo[j].ucDENOCode,
                stStackeNotesDenoInfo.stStackeNotesInfo[j].usNumberStack,
                stStackeNotesDenoInfo.stStackeNotesInfo[j].iDest);
        }

        map<BYTE, WFSCIMNOTETYPE>::const_iterator it = m_mapSupportNotID.find(stStackeNotesDenoInfo.stStackeNotesInfo[j].ucDENOCode);
        if (it != m_mapSupportNotID.end() && it->second.bConfigured
            && (stStackeNotesDenoInfo.stStackeNotesInfo[j].iDest == ID_ESC))
        {
            //形成验钞结果数据
            sprintf(pResult + 12 * ioffset, "%06x%06d", it->second.usNoteID,
                    stStackeNotesDenoInfo.stStackeNotesInfo[j].usNumberStack);
            ioffset++;
            stStackeNotesDenoInfo.stStackeNotesInfo[j].usNumberStack = 0;
            if (ioffset * 12 > 256)
            {
                Log(ThisModule, WFS_ERR_INTERNAL_ERROR,  "pResult长度大于最大长度(256)");
                return WFS_ERR_INTERNAL_ERROR;
            }
        }
    }

    //处理其他点钞错误
    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "点钞失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    //点钞数为0
    if (iNumStackedToESC == 0)
    {
        if (m_sDevStatus.stInPutPosition == ADP_INPOS_EMPTY
            || iErrReject == ADP_ERR_REJECT_NOBILL)
        {
            Log(ThisModule, WFS_ERR_CIM_NOITEMS, "CS没有检测到钞票(ErrCode:%s)", m_szLastErrCode);
            iErrReject = ADP_ERR_REJECT_NOBILL;
            return WFS_ERR_CIM_NOITEMS;
        }
        else
        {
            if (iErrReject == ADP_ERR_REJECT_STACKERFULL
                && (m_sDevStatus.stStacker == ADP_STACKER_NOTEMPTY || m_sDevStatus.stStacker == ADP_STACKER_FULL))      //30-00-00-00(FT#0029)
            {
                Log(ThisModule, WFS_ERR_CIM_TOOMANYITEMS, "TS钞票过多，验钞失败(ErrCode:%s)", m_szLastErrCode);
                return WFS_ERR_CIM_TOOMANYITEMS;
            }
            else if (m_pErrCodeTrans->IsTooManyItemInCSOfCashCount(m_szLastErrCode))
            {
                Log(ThisModule, WFS_ERR_CIM_TOOMANYITEMS, "CS钞票过多，验钞失败(ErrCode:%s)", m_szLastErrCode);
                return WFS_ERR_CIM_TOOMANYITEMS;
            }
        }
    }else{ //test#7 start

        if (iErrReject == ADP_ERR_REJECT_STACKERFULL
            && (m_sDevStatus.stStacker == ADP_STACKER_NOTEMPTY || m_sDevStatus.stStacker == ADP_STACKER_FULL))                  //30-00-00-00(FT#0029)
        {
            Log(ThisModule, WFS_ERR_CIM_TOOMANYITEMS, "TS钞票过多，验钞失败(ErrCode:%s)", m_szLastErrCode);
            iTooManeyItemsHappenFlg = 1;
        }
        else if (m_pErrCodeTrans->IsTooManyItemInCSOfCashCount(m_szLastErrCode))
        {
             Log(ThisModule, WFS_ERR_CIM_TOOMANYITEMS, "CS钞票过多，验钞失败(ErrCode:%s)", m_szLastErrCode);
             iTooManeyItemsHappenFlg = 1;
        }

    }//test#7 end
    Log(ThisModule, 0, "CashCountResult: %s", pResult);
    SaveRejectBill(ulRejCount);

    return WFS_SUCCESS;
}

//功能：将进钞钞票存入钞箱
//输入：无
//输出  ppNoteCounts：存放各钞箱存入的各种钞票张数,每个数组成员返回内容为各通道钞箱的各种钞票张数列表。单条格式为："[钞票ID:char6][钞票张数:char6]",
//      如："000001000030000002000013"代表钞票ID为1的钞票有30张,ID为2的钞票有13张;
//：     arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
//返回：XFS通用错误码及CIM错误码
//说明：
long CUR2Adapter::StoreCash(char ppNoteCounts[ADP_MAX_CU_SIZE][256], ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "StoreCash";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    VERIFYPOINTNOTEMPTY(ppNoteCounts)
    VERIFYPOINTNOTEMPTY(arywCUError)

    ZeroMemory(ppNoteCounts, sizeof(char) * ADP_MAX_CU_SIZE * 256);
    ZeroMemory(m_szRejectBill, sizeof(m_szRejectBill));

    //检查TS有无钞票
    if (m_sDevStatus.stStacker == ADP_STACKER_EMPTY)
    {
        Log(ThisModule, WFS_ERR_CIM_NOITEMS, "TS没有钞币");
        //return WFS_ERR_CIM_NOITEMS;
    }

    //AB箱不能被禁止
    if (!m_arybEnableCassUnit[3])
    {
        Log(ThisModule, WFS_ERR_UNSUPP_DATA, "AB箱被禁止");
        return WFS_ERR_UNSUPP_DATA;
    }

    SETCASSUNITUNUSED(arywCUError);

    SetVerificationLevel(SAVESERIALOPERAT_S);
    SetRejectCounterfeitNote();
    SetUnfitnoteVerificationLevel(SAVESERIALOPERAT_S);

    BOOL  bCheckCS = TRUE;
    DESTINATION_REJCET iDestinationReject = DESTINATION_REJCET_DEFAULT;
    BYTE ucProhibited = 0x00;
    GetProhibitedCashUnitBit(m_arybEnableCassUnit, ucProhibited);
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeUnfitNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoInfo;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    m_sDevConfigInfo.stBVDependentMode = m_sDevConfigInfo.stCashInEndBVDependentMode;       //30-00-00-00(FS#0030)
    m_pUR2Dev->SetZeroBVSettingInfo(BVSET_CMDTYPE_STOREMONEY, m_sDevConfigInfo.byBVSettingLevel, m_sDevConfigInfo.strBVSettingInfo.c_str());    //30-00-00-00(FS#0025)
    int iRet = m_pUR2Dev->StoreMoney(
               m_sDevConfigInfo.CashInNoteType,
               bCheckCS,
               iDestinationReject,
               ucProhibited,
               m_sDevConfigInfo.stBVDependentMode,
               iTotalCashInURJB,
               iNumStackedToCS,
               iNumStackedToESC,
               pNumStackedToPerCass,
               iNumCSFed,
               iNumESCFed,
               pNumPerCassFed,
               stStackeNotesDenoInfo,
               stStackeUnfitNotesDenoInfo,
               stStackeRejectNotesDenoInfo,
               pMaintenanceInfo);
    // 强制检查CS
    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "存款");
    UpdateADPDevStatus(iRet);

    //保存序列号
  //test#4   if (m_sDevConfigInfo.CashInNoteType == VALIDATION_MODE_REAL)
  //test#4   {
        CThreadGenerateSN SNWork(SAVESERIALOPERAT_S, m_eLfsCmdId, iRet);    //test#8
        QueryNoteMediaInfo(&SNWork);
        QueryNoteSerialInfo(&SNWork);
  //test#4   }

    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        if (pNumStackedToPerCass[i] > 0)
        {
            arywCUError[i] = ADP_CUERROR_OK;
        }
    }
    BOOL bCassErr = m_pErrCodeTrans->GetCUErr(m_szLastErrCode, m_arybEnableCassUnit, arywCUError);

    //AB箱中存入的钞票代表存款中的拒钞数
    SaveRejectBill(pNumStackedToPerCass[3]);

    ST_STACKE_NOTES_DENO_INFO stNoteDeInfo;
    int iCassOffset[MAX_CASSETTE_NUM] = {0};
    UINT iTotalNum = 0;
    //合格钞票
    for (int j = 0; j < MAX_DENOMINATION_NUM ; j++)
    {
        stNoteDeInfo = stStackeNotesDenoInfo.stStackeNotesInfo[j];
        if (stNoteDeInfo.usNumberStack > 0)
        {
            BYTE ucCassID = stNoteDeInfo.iDest >> 4;
            Log(ThisModule, 1, "DenoIndex:%d, 钞箱号: %x", j, ucCassID);
            if (ucCassID >= MAX_CASSETTE_NUM || ucCassID <= 0)
            {
                Log(ThisModule, WFS_ERR_INTERNAL_ERROR, "存钞失败,钞箱号不合法: %d", ucCassID);
                return WFS_ERR_INTERNAL_ERROR;
            }

            if (iCassOffset[ucCassID - 1] >= 256)
            {
                Log(ThisModule, WFS_ERR_INTERNAL_ERROR, "钞箱%d存钞结果缓存长度异常: %d", ucCassID, iCassOffset[ucCassID - 1]);
                return WFS_ERR_INTERNAL_ERROR;
            }

            map<BYTE, WFSCIMNOTETYPE>::const_iterator mit = m_mapSupportNotID.find(stNoteDeInfo.ucDENOCode);
            if (mit != m_mapSupportNotID.end() && mit->second.bConfigured)
            {
                //计算存钞结果数据
                sprintf(ppNoteCounts[ucCassID - 1] + 12 * iCassOffset[ucCassID - 1], "%06x%06d", mit->second.usNoteID, stNoteDeInfo.usNumberStack);
            }
            else if (stNoteDeInfo.ucDENOCode == (MAX_DENOMINATION_NUM + 1))
            {
                sprintf(ppNoteCounts[ucCassID - 1] + 12 * iCassOffset[ucCassID - 1], "%06x%06d", NOTE_ID_UNKNOWN, stNoteDeInfo.usNumberStack);
            }
            else
            {
                Log(ThisModule, WFS_ERR_INTERNAL_ERROR,
                    "存钞出现未设置的币种: %d， 张数：%d", stNoteDeInfo.ucDENOCode, stNoteDeInfo.usNumberStack);
            }
            iTotalNum +=  stNoteDeInfo.usNumberStack;
            iCassOffset[ucCassID - 1]++;
        }
    }

    //不合格钞票
    ST_STACKE_NOTES_DENO_INFO stNoteUnfiDeInfo;
    for (int j = 0; j < MAX_DENOMINATION_NUM ; j++)
    {
        stNoteUnfiDeInfo = stStackeUnfitNotesDenoInfo.stStackeNotesInfo[j];
        if (stNoteUnfiDeInfo.usNumberStack > 0)
        {
            BYTE ucCassID = stNoteUnfiDeInfo.iDest >> 4;
            if (ucCassID >= MAX_CASSETTE_NUM || ucCassID <= 0)
            {
                Log(ThisModule, WFS_ERR_INTERNAL_ERROR, "Unfit废钞存钞失败,钞箱号不合法: %d", ucCassID);
                return WFS_ERR_INTERNAL_ERROR;
            }
            if (iCassOffset[ucCassID - 1] >= 256)
            {
                Log(ThisModule, WFS_ERR_INTERNAL_ERROR, "钞箱%d存钞结果缓存长度异常: %d", ucCassID, iCassOffset[ucCassID - 1]);
                return WFS_ERR_INTERNAL_ERROR;
            }

            map<BYTE, WFSCIMNOTETYPE>::const_iterator mit = m_mapSupportNotID.find(stNoteUnfiDeInfo.ucDENOCode);
            if (mit != m_mapSupportNotID.end() && mit->second.bConfigured)
            {
                //计算存钞结果数据
                sprintf(ppNoteCounts[ucCassID - 1] + 12 * iCassOffset[ucCassID - 1], "%06x%06d", mit->second.usNoteID, stNoteUnfiDeInfo.usNumberStack);
            }
            else if (stNoteUnfiDeInfo.ucDENOCode == (MAX_DENOMINATION_NUM + 1))
            {
                sprintf(ppNoteCounts[ucCassID - 1] + 12 * iCassOffset[ucCassID - 1], "%06x%06d", NOTE_ID_UNKNOWN, stNoteUnfiDeInfo.usNumberStack);
            }
            else
            {
                Log(ThisModule, WFS_ERR_INTERNAL_ERROR, "存钞出现未设置的unfit币种: %d， 张数：%d", stNoteDeInfo.ucDENOCode, stNoteDeInfo.usNumberStack);
            }
            iTotalNum +=  stNoteUnfiDeInfo.usNumberStack;
            iCassOffset[ucCassID - 1]++;

        }
    }

    if (iRet < 0)
    {
        Log(ThisModule, 1, "存钞结果: CASS1: %s CASS2: %s CASS3: %s CASS4: %s CASS5:  %s",  ppNoteCounts[0],
            ppNoteCounts[1],  ppNoteCounts[2],  ppNoteCounts[3],  ppNoteCounts[4]);
    }

    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "存钞失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return bCassErr ? WFS_ERR_CIM_CASHUNITERROR : WFS_ERR_HARDWARE_ERROR;
    }

    if (iTotalNum != iNumESCFed)
    {
        Log(ThisModule, WFS_ERR_INTERNAL_ERROR, "存钞失败, 进钞钞票数校验错误: iNumESCFed:%d，iTotalNum:%d ", iNumESCFed, iTotalNum);
    }

    if (iNumESCFed == 0)
    {
        return WFS_ERR_CIM_NOITEMS;
    }

    return WFS_SUCCESS;
}

//功能：退回存款钞票 将存款点钞完的钞票退出到出钞口
//输入：无
//输出：arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
//返回：XFS通用错误码及CIM错误码
//说明：
long CUR2Adapter::RollBack(ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "RollBack";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    VERIFYPOINTNOTEMPTY(arywCUError)

    SETCASSUNITUNUSED(arywCUError)
    int iTotalCashInURJB;
    int iNumStackedToCS;
    int iNumStackedToESC;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed;
    int iNumESCFed;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};

    int iRet = m_pUR2Dev->CashRollback(
               m_sDevConfigInfo.CashInNoteType,
               iTotalCashInURJB,
               iNumStackedToCS,
               iNumStackedToESC,
               pNumStackedToPerCass,
               iNumCSFed,
               iNumESCFed,
               pNumPerCassFed,
               stStackeNotesDenoInfo,
               pMaintenanceInfo);
    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "验钞后退钞");
    UpdateADPDevStatus(iRet);

    m_pErrCodeTrans->GetCUErr(m_szLastErrCode, m_arybEnableCassUnit, arywCUError);

    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "取消存款失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

//功能：将Dispense后存在暂存区的钞票送到门口
//输入：无
//输出：无
//返回：XFS通用错误码及CDM错误码
//说明：
long CUR2Adapter::Present()
{
    const char *ThisModule = "Present";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)

    m_bCashOutBeforePresent = FALSE;

    int iRet = 0;
    //Dispense后钞票已经被送至出钞口，这里只检查出钞口是否关闭和CS是否有钞票
    if (m_sDevStatus.stOutShutter == ADP_OUTSHUTTER_CLOSED)
    {
        if (m_sDevStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY)
        {
            iRet = OpenDeviceShutter();
            if (iRet < 0)
            {
                Log(ThisModule, iRet,   "Shutter门打开失败");
                return iRet;
            }
            return WFS_SUCCESS;
        }
        else if (m_sDevStatus.stOutPutPosition == ADP_OUTPOS_EMPTY)
        {
            Log(ThisModule, WFS_ERR_CDM_NOITEMS, "出钞口没有钞币");
            return WFS_ERR_CDM_NOITEMS;
        }
        else
        {
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "出钞口状态异常");
            return WFS_ERR_HARDWARE_ERROR;
        }
    }
    else if (m_sDevStatus.stOutShutter == ADP_OUTSHUTTER_OPEN)
    {
        Log(ThisModule, WFS_ERR_CDM_SHUTTEROPEN, "出钞门已经打开");
        return WFS_ERR_CDM_SHUTTEROPEN;
    }
    else
    {
        Log(ThisModule, WFS_ERR_CDM_SHUTTERNOTOPEN, "出钞门状态异常");
        return WFS_ERR_CDM_SHUTTERNOTOPEN;
    }
    return 0;
}

//功能：回收 将Present后在门口的钞票,Dispense后存在暂存区的钞票回收到回收箱
//输入：usRetraceArea：钞票回收的目的位置
//      usIndex:当usRetraceArea指定为某个钞箱时有效,表示钞箱索引,从1开始,依次增1
//输出：arywCUError：钞箱错误原因
//返回：XFS通用错误码及CDM错误码
//说明：无
long CUR2Adapter::Retract(ADP_RETRACTEARE usRetractArea, USHORT &usIndex, ADP_CUERROR arywCUERROR[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "Retract";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    VERIFYPOINTNOTEMPTY(arywCUERROR)
    SETCASSUNITUNUSED(arywCUERROR)

    //如果没有设置则取配置文件默认的目的位置
    //1:回收箱，2：通道 3：暂存区，4：废钞箱  5：循环箱
    QueryDevInfo();
    ADPSTATUSINFOR stdevstatusinital = m_sDevStatus;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    int iRet = m_pUR2Dev->CloseShutter(pMaintenanceInfo, FALSE);
    if (0 > iRet)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "回收前关门失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return iRet;
    }
    if (m_sDevStatus.stInPutPosition == ADP_INPOS_EMPTY
        && stdevstatusinital.stInPutPosition == ADP_INPOS_NOTEMPTY)
    {
        return WFS_ERR_CDM_ITEMSTAKEN;
    }
    return RetractNotesToDesArea(usRetractArea, usIndex, arywCUERROR);
}

//功能：回收拒钞 将Dispense后存在暂存区的钞票回收到废钞箱
//输入：usIndex,表示钞箱索引,从1开始,依次增1
//输出：arywCUError：钞箱错误原因
//返回：XFS通用错误码及CDM错误码
//说明：
long CUR2Adapter::Reject(USHORT usIndex, ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "Reject";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    VERIFYPOINTNOTEMPTY(arywCUError)
    VERIFYCASSINDEX(usIndex)

    SETCASSUNITUNUSED(arywCUError)

 /*   if (!m_bCashOutBeforePresent)
    {
        Log(ThisModule, WFS_ERR_CDM_NOITEMS, "暂存区为非空");
        return WFS_ERR_CDM_NOITEMS;
    }*/  //test#15

    if (usIndex != 4)
    {
        Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱%d不为废钞箱", usIndex);
        return WFS_ERR_UNSUPP_DATA;
    }

    ULONG aryRetractCountPerDeno[ADP_MAX_DENOMINATION_NUM] = {0};

    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM];
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM];
    int nRejectNotes = 0;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    USHORT usFedNoteCondition = 0;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH];
    BOOL bIgnoreESC = TRUE;

    m_sDevConfigInfo.stBVDependentMode.bEnableBVModeSetting = FALSE;            //30-00-00-00(FS#0030)
    int iRet = m_pUR2Dev->CashCountRetract(
               m_sDevConfigInfo.CashOutNoteType,
               m_sDevConfigInfo.stBVDependentMode,
               iTotalCashInURJB,
               iNumStackedToCS,
               iNumStackedToESC,
               pNumStackedToPerCass,
               iNumCSFed,
               iNumESCFed,
               pNumPerCassFed,
               nRejectNotes,
               stStackeNotesDenoInfo,
               usFedNoteCondition,
               pMaintenanceInfo,
               bIgnoreESC);
    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "Reject To ESC");
    UpdateADPDevStatus(iRet);

    m_bCashOutBeforePresent = FALSE;

    SaveRejBillPerDenoALL(stStackeNotesDenoInfo, aryRetractCountPerDeno);
    SaveRejectBill(aryRetractCountPerDeno);
    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "回收钞票到ESC失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    /*
    if (m_sDevConfigInfo.CashOutNoteType == VALIDATION_MODE_REAL)
    {
        CThreadGenerateSN SNWork(SAVESERIALOPERAT_J, iRet);
        QueryNoteMediaInfo(&SNWork);
        QueryNoteSerialInfo(&SNWork);
    }*/


    //ESC to AB-Box
    ZeroMemory(aryRetractCountPerDeno, sizeof(aryRetractCountPerDeno));

    DESTINATION_REJCET iDestinationReject = DESTINATION_REJCET_DEFAULT;
    iTotalCashInURJB = 0;
    iNumStackedToCS = 0;
    iNumStackedToESC = 0;
    iNumCSFed = 0;
    iNumESCFed = 0;
    stStackeNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeUnfitNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoInfo;
    BOOL bEnable[MAX_CASSETTE_NUM] = {FALSE};
    bEnable[3] = TRUE;
    BYTE btProhibitedBox = 0;
    GetProhibitedCashUnitBit(bEnable, btProhibitedBox);

    m_sDevConfigInfo.stBVDependentMode = m_sDevConfigInfo.stRejectBVDependentMode;      //30-00-00-00(FS#0030)
    iRet = m_pUR2Dev->RetractESC(
           m_sDevConfigInfo.CashInNoteType, btProhibitedBox, iDestinationReject, m_sDevConfigInfo.stBVDependentMode,
           iTotalCashInURJB, iNumStackedToCS,  iNumStackedToESC, pNumStackedToPerCass, iNumCSFed, iNumESCFed,
           pNumPerCassFed, stStackeNotesDenoInfo, stStackeUnfitNotesDenoInfo,  stStackeRejectNotesDenoInfo,
           pMaintenanceInfo);
    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "Reject to AB-Box");
    UpdateADPDevStatus(iRet);

    //获取钞箱错误信息
    for (int i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        if (pNumStackedToPerCass[i] > 0)
        {
            arywCUError[i] = ADP_CUERROR_OK;
        }
    }

    BOOL bCassErr = m_pErrCodeTrans->GetCUErr(m_szLastErrCode, m_arybEnableCassUnit, arywCUError);

    /*
    if (m_sDevConfigInfo.CashOutNoteType == VALIDATION_MODE_REAL)
    {
        CThreadGenerateSN SNWork(SAVESERIALOPERAT_J, iRet);
        QueryNoteMediaInfo(&SNWork);
        QueryNoteSerialInfo(&SNWork);
    }*/

    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        m_aryRetractCount[i] += pNumStackedToPerCass[i];
    }

    SaveRejBillPerDenoALL(stStackeNotesDenoInfo, aryRetractCountPerDeno);
    SaveRejectBill(aryRetractCountPerDeno);
    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "ESC回收到废钞箱失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    return 0;
}


//功能：复位机芯
//输入：无
//输出：arywCUError： 钞箱错误原因
//返回：XFS通用错误码
//说明：内部方法
long CUR2Adapter::Init(ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "Init";
    VERIFYPOINTNOTEMPTY(arywCUError);
    VERIFYPOINTNOTEMPTY(m_pUR2Dev);

    //复位后清除内部临时有效的状态和数据
    ZeroMemory(m_szRejectBill, sizeof(m_szRejectBill));
    ZeroMemory(m_aryRetractCount, sizeof(m_aryRetractCount));
    m_bCashOutBeforePresent = FALSE;

    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    BOOL bCancelEnergySaveMode = FALSE;
    BOOL bQuickRest = TRUE;

    int iRet = m_pUR2Dev->Reset(
               iTotalCashInURJB,
               iNumStackedToCS,
               iNumStackedToESC,
               pNumStackedToPerCass,
               iNumCSFed,
               iNumESCFed,
               pNumPerCassFed,
               pMaintenanceInfo,
               bCancelEnergySaveMode,
               bQuickRest);
    if (iRet >= 0)
    {
        m_bIsNeedReset = FALSE;
        m_bCloseShutterJammed = FALSE;
        m_bOpenShutterJammed = FALSE;
    }

    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "复位");
    UpdateADPDevStatus(iRet);

    BOOL bCassErr = m_pErrCodeTrans->GetCUErr(m_szLastErrCode, m_arybEnableCassUnit, arywCUError);
    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "驱动复位失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

//功能：复位机芯,将机芯置恢复到已知正常状态
//输入：usRetraceArea：钞票回收的目的位置
//      usIndex:当usRetraceArea指定为某个钞箱时有效,表示钞箱索引,从1开始,依次增1
//输出：arywCUError：钞箱错误原因
//      bMediaDetected：复位过程中是否检测到介质
//返回：XFS通用错误码及CDM错误码
//说明：
long CUR2Adapter::Reset(ADP_RETRACTEARE &usRetraceArea, USHORT &usIndex,
                        BOOL &bMediaDetected, ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE])

{
    const char *ThisModule = "Reset";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    VERIFYPOINTNOTEMPTY(arywCUError)

    SETCASSUNITUNUSED(arywCUError)
    ZeroMemory(m_aryRetractCount, sizeof(ULONG)*ADP_MAX_CU_SIZE);

    Log(ThisModule, 1, "usRetraceArea:%d,usIndex:%d", usRetraceArea, usIndex);

    if (!m_bConnectNormal)
    {
        int iRet = OpenUSBConnect(CONNECT_TYPE_UR);
        if (iRet < 0)
        {
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "重新连接机芯失败(ReturnValue;%d)", iRet);
            return iRet;
        }
    }

    if (!m_bBVConnectNormal)
    {
        int iRet = OpenUSBConnect(CONNECT_TYPE_ZERO);
        if (iRet < 0)
        {
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "重新连接ZERO失败(ReturnValue;%d)", iRet);
        }
    }

    int iRet = 0;

    iRet = InitUnitSetting();
    if (iRet < 0)
    {
        Log(ThisModule, iRet,   "初始化设置失败");
        return iRet;
    }

    iRet = Init(arywCUError);

    if (iRet < 0)
    {
        Log(ThisModule, iRet, "驱动复位失败");
        return iRet;
    }

    for (int i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        m_aryCassStatusWhenEndEx[i] = m_aryADPCassStatusInfo[i];
    }

    //回收复位后在钞口的钞票
    if (m_sDevStatus.stOutPutPosition == ADP_OUTPOS_NOTEMPTY)
    {
        bMediaDetected = TRUE;
        iRet = RetractNotesToDesArea(usRetraceArea, usIndex, arywCUError);
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "回收复位后门口钞票失败");
            return iRet;
        }
    }
    else
    {
        bMediaDetected = FALSE;
    }

    return WFS_SUCCESS;
}

//功能：取钞票每次有产生读取钞票序列号动作后的序列号
//输入：无
//输出：dwIndex：第几个条目,从0开始,至总数量-1；
//      usNoteID：币种代码；
//      arycSerialNumber[128]：序列号。
//返回：XFS通用错误码
//说明：
long CUR2Adapter::GetNoteSeiralNumbers(ULONG dwIndex, USHORT &usNoteID, char arycSerialNumber[128])
{
    const char *ThisModule = "GetNoteSeiralNumbers";

    return WFS_SUCCESS;
}

//功能：设钞箱信息
//输入：wHopperID:钞箱序号,从1开始,依次增1,最大ADP_MAX_CU_SIZE
//      Currency:币种
//      ulValue：面额
//      eType：钞箱类型;
//      lpusNoteIDs:以0结尾的整数数组,表示钞箱支持的钞票类型ID列表,
//          如果要收纳所有不可接受的币种,那么ID列表最后一个的值为NOTE_ID_UNKNOWN。
//          REJECT、出钞箱，该值可为NULL
//      ulCount: 当前张数
//输出：无
//返回：XFS通用错误码及CIM错误码
//说明：设置完成后可能须要立即调用初始化复位方法
long CUR2Adapter::SetCassetteInfo(WORD wHopperID, const char Currency[3], ADP_CASSETTE_TYPE eType,
                                  ULONG ulValue, WORD *lpusNoteIDs, ULONG ulCount)
{
    const char *ThisModule = "SetCassetteInfo";
    VERIFYPOINTNOTEMPTY(Currency)
    VERIFYCASSINDEX(wHopperID)
    ADP_CASS_INFO ADPCassInfo;
    UINT uIDCount = 0;
    UINT i = 0;
    map<BYTE, WFSCIMNOTETYPE>::iterator it;
    char szTemp[128] = {0};

    if (wHopperID == ADP_MAX_CU_SIZE)
    {
        return WFS_SUCCESS;
    }

    if (eType == ADP_CASSETTE_UNKNOWN)//表示未启用
    {
        if (m_aryADPCassSettingInfo[wHopperID - 1].bActive)
        {
            m_aryADPCassSettingInfo[wHopperID - 1].bActive = FALSE;
        }
        return WFS_SUCCESS;
    }

    if (wHopperID == 4)//AB箱特性
    {
        if (eType == ADP_CASSETTE_RECYCLING
            || eType == ADP_CASSETTE_BILL
            || eType == ADP_CASSETTE_CASHIN) //不能为进钞箱，尽管InitSetting可以成功，但没有AB箱，不能做出钞动作
        {
            m_bCassVerifyFail[wHopperID - 1] = TRUE;
            CassType2String(eType, szTemp);
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "设置错误，Cass4不能设置为 %s", szTemp);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    if (lpusNoteIDs[0] == 0x00)//出钞箱
    {
        if (eType != ADP_CASSETTE_BILL)
        {
            m_bCassVerifyFail[wHopperID - 1] = TRUE;
            CassType2String(eType, szTemp);
            Log(ThisModule, WFS_ERR_UNSUPP_DATA,
                "钞箱信息设置错误wHopperID=%hd,eType=%s,ulvalue=%d,currency=%c%c%c,",
                wHopperID, szTemp, ulValue, Currency[0], Currency[1], Currency[2]);
            return WFS_ERR_UNSUPP_DATA;
        }
        else
        {
            it = m_mapSupportNotID.begin();
            USHORT usDenoCode = 0;
            USHORT usRelease = 0;
            while (it != m_mapSupportNotID.end())
            {
                if (it->second.ulValues == ulValue
                    && (memcmp(it->second.cCurrencyID, Currency, 3) == 0))
                {
                    if ((it->second.usRelease > usRelease)
                        && (it->second.usRelease != 1999))
                    {
                        usRelease = it->second.usRelease;
                        usDenoCode = it->first;
                        memcpy(ADPCassInfo.CurrencyID, Currency, 3);
                    }
                }
                it++;
            }
            ADPCassInfo.usDenoCode = usDenoCode;

            if (usDenoCode == 0)
            {
                m_bCassVerifyFail[wHopperID - 1] = TRUE;
                CassType2String(eType, szTemp);
                Log(ThisModule, WFS_ERR_UNSUPP_DATA,
                    "钞箱信息设置错误wHopperID=%hd,eType=%s,ulvalue=%d,currency=\"%c%c%c\"",
                    wHopperID, szTemp, ulValue, Currency[0], Currency[1], Currency[2]);
                return WFS_ERR_UNSUPP_DATA;
            }
        }
    }
    else if (lpusNoteIDs[0] == NOTE_ID_UNKNOWN)// 所有介质，包括allUnfit, allfit, suport
    {
        if (eType == ADP_CASSETTE_RECYCLING
            || eType == ADP_CASSETTE_BILL
            || eType == ADP_CASSETTE_CASHIN)
        {
            m_bCassVerifyFail[wHopperID - 1] = TRUE;
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱(%hd)信息设置错误:出钞箱,循环箱或进钞箱不能接受所有类型介质", wHopperID);
            return WFS_ERR_UNSUPP_DATA;
        }
        else
        {
            if (memcmp(Currency, "   ", 3) != 0)
            {
                m_bCassVerifyFail[wHopperID - 1] = TRUE;
                Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱(%hd)信息设置错误，Currency设置错误: \"%c%c%c\"", wHopperID, Currency[0], Currency[1], Currency[2]);
                return WFS_ERR_UNSUPP_DATA;
            }
            ADPCassInfo.usDenoCode = 0;
            memcpy(ADPCassInfo.CurrencyID, Currency, 3);//此处currency 应为"   "
        }
    }
    else if ((lpusNoteIDs[0] != 0x0000) && (lpusNoteIDs[1] == 0x0000))
    {
        it = m_mapSupportNotID.begin();
        while (it != m_mapSupportNotID.end())
        {
            if (it->second.usNoteID == lpusNoteIDs[0])
            {
                ADPCassInfo.usDenoCode = it->first;
                memcpy(ADPCassInfo.CurrencyID, it->second.cCurrencyID, 3);
                break;
            }
            it++;
        }
        if (it == m_mapSupportNotID.end())
        {
            m_bCassVerifyFail[wHopperID - 1] = TRUE;
            CassType2String(eType, szTemp);
            Log(ThisModule, WFS_ERR_UNSUPP_DATA,
                "钞箱%hd信息设置错误: eType=%s,ulvalue=%d,currency=%c%c%c,",
                wHopperID, szTemp, ulValue, Currency[0], Currency[1], Currency[2]);
            return WFS_ERR_UNSUPP_DATA;
        }

        if (it->second.ulValues != ulValue ||
            memcmp(it->second.cCurrencyID, Currency, 3) != 0)
        {
            m_bCassVerifyFail[wHopperID - 1] = TRUE;
            Log(ThisModule, WFS_ERR_UNSUPP_DATA,
                "钞箱%hd信息设置错误(面值或币种不一致): NoteId=%hd, ValueOfNoteId=%d,Value=%hd,CurrencyIDOfNoteID=%.3s,CurrencyID=%.3s",
                wHopperID, lpusNoteIDs[0], it->second.ulValues, ulValue, it->second.cCurrencyID, Currency);
            return WFS_ERR_UNSUPP_DATA;
        }
    }
    else
    {
        //RB类型箱只能接受1种面值
        if (eType == ADP_CASSETTE_BILL
            || eType == ADP_CASSETTE_CASHIN
            || eType == ADP_CASSETTE_RECYCLING)
        {
            m_bCassVerifyFail[wHopperID - 1] = TRUE;
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱(%hd)信息设置错误:只接受一种面值", wHopperID);
            return WFS_ERR_UNSUPP_DATA;
        }
        char szNoteID[ADP_MAX_DENOMINATION_NUM] = {0};
        int offset = 0;
        while (lpusNoteIDs[i] != 0x0000)
        {
            if (lpusNoteIDs[i] == NOTE_ID_UNKNOWN)
            {
                ADPCassInfo.usDenoCode = 0;
                memcpy(ADPCassInfo.CurrencyID, Currency, 3);
                break;
            }
            offset += sprintf(szNoteID + offset, "%hd ", lpusNoteIDs[i]);
            i++;
        }
        if ((i == m_mapSupportNotID.size()) || (lpusNoteIDs[i] == NOTE_ID_UNKNOWN))
        {
            ADPCassInfo.usDenoCode = 0;
            memcpy(ADPCassInfo.CurrencyID, Currency, 3);
        }
        else
        {
            m_bCassVerifyFail[wHopperID - 1] = TRUE;
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱(%d)信息设置错误:币种ID种类设置错误NoteIDList:%s", wHopperID, szNoteID);
            return WFS_ERR_UNSUPP_DATA;
        }
    }

    ADPCassInfo.bActive = TRUE; //设置该钱箱表示启用此钱箱
    ADPCassInfo.ulCount = ulCount;//URJB须设定初始钞数
    ADPCassInfo.iCassType = eType;
    ADPCassInfo.ulValue = ulValue;

    if (VertifyIsNeedResetCassChanged(ADPCassInfo, wHopperID))
    {
        m_bIsNeedReset = TRUE;
        //Log(ThisModule, WFS_ERR_HARDWARE_ERROR,"VertifyIsNeedResetCassChanged failed");
    }

    m_aryADPCassSettingInfo[wHopperID - 1] = ADPCassInfo;
    m_bCassVerifyFail[wHopperID - 1] = FALSE;

    return WFS_SUCCESS;
}

//功能：检查设置钞箱信息后是否需要复位
//输入： ADPCassInfo：设置的钞箱信息
//       usCassID:  钞箱号
//输出：无
//返回：XFS通用错误码及CDM错误码
//说明：内部方法
BOOL CUR2Adapter::VertifyIsNeedResetCassChanged(const ADP_CASS_INFO &ADPCassInfo, WORD usCassID) const
{
    if (ADPCassInfo.bActive != m_aryADPCassSettingInfo[usCassID - 1].bActive
        || ADPCassInfo.iCassType != m_aryADPCassSettingInfo[usCassID - 1].iCassType
        || ADPCassInfo.ulValue != m_aryADPCassSettingInfo[usCassID - 1].ulValue
        || ADPCassInfo.usDenoCode != m_aryADPCassSettingInfo[usCassID - 1].usDenoCode
        || memcmp(ADPCassInfo.CurrencyID, m_aryADPCassSettingInfo[usCassID - 1].CurrencyID, 3) != 0)
    {
        return TRUE;
    }
    return FALSE;
}

//功能：取钞箱信息
//输入：wHopperID,钞箱序号,从1开始,依次增1
//输出：
//      eStatus:状态（取值定义与XFS相同）
//      cCassID:物理钞箱ID
//返回：XFS通用错误码及CIM错误码
//说明：1)  由于上层没有用到长、宽、厚等信息,适配层可在下面自己做相关校验。
long CUR2Adapter::GetCassetteInfo(WORD wHopperID, CASHUNIT_STATUS &eStatus, char cCassID[5])
{
    const char *ThisModule = "GetCassetteInfo";
    VERIFYCASSINDEX(wHopperID)
    VERIFYPOINTNOTEMPTY(cCassID)
    //AutoLogFuncBeginEnd();
    //Log(ThisModule, 1, "wHopperID:%d",wHopperID);
    if (m_bCassVerifyFail[wHopperID - 1])
    {
        eStatus = ADP_CASHUNIT_INOP;
    }
    else
    {
        eStatus = m_aryADPCassStatusInfo[wHopperID - 1];
    }
    char arycBuf[256] = {0};
    sprintf(arycBuf, "Cass%d", wHopperID);
    memcpy(cCassID, arycBuf, 5);
    return WFS_SUCCESS;
}

//功能：检查钞箱是否具有出钞能力
//输入：aryCassItems：每个钞箱须出钞张数
//输出：arywCUError：钞箱错误类型
//返回：XFS通用错误码及CDM错误码
//说明：内部方法
int CUR2Adapter::IsDeviceDispensable(const ULONG aryCassItems[ADP_MAX_CU_SIZE],
                                     ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "IsDeviceDispensable";
    assert(aryCassItems != NULL);
    assert(arywCUError != NULL);

    if (!m_arybEnableCassUnit[3])//AB钞箱不能被禁用
    {
        arywCUError[3] = ADP_CUERROR_FATAL;
        Log(ThisModule, WFS_ERR_CDM_CASHUNITERROR, "AB箱被禁止");
        return WFS_ERR_CDM_CASHUNITERROR;
    }
    arywCUError[3] = ADP_CUERROR_OK;

    //检查设备能力是否满足取钞要求
    for (int i = 1;  i < MAX_CASSETTE_NUM; i++)
    {
        if (aryCassItems[i] == 0)//不需要出钞 不做检查
        {
            continue;
        }
        //出钞箱须没有被禁止
        else if (!m_arybEnableCassUnit[i])
        {
            Log(ThisModule, WFS_ERR_CDM_CASHUNITERROR, "出钞箱(%d)被禁止", i + 1);
            arywCUError[i] = ADP_CUERROR_FATAL;
            return WFS_ERR_CDM_CASHUNITERROR;
        }
        //出钞箱须具有取钞功能 只能是取款箱或循环箱
        else if ((m_aryADPCassSettingInfo[i].iCassType != ADP_CASSETTE_BILL) &&
                 (m_aryADPCassSettingInfo[i].iCassType != ADP_CASSETTE_RECYCLING))
        {
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱(%d)不是出钞箱", i + 1);
            return WFS_ERR_UNSUPP_DATA;
        }
        //出钞张数不能大于出钞能力
        else if (aryCassItems[i] > m_sCDMCapabilities.wMaxDispenseItems)
        {
            Log(ThisModule, WFS_ERR_UNSUPP_DATA, "钞箱(%d)出钞数(%d)大于最大出钞张数(%d)", i + 1, aryCassItems[i], m_sCDMCapabilities.wMaxDispenseItems);
            return WFS_ERR_UNSUPP_DATA;
        }
        //检查出钞箱是否为空
        else if (m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_EMPTY)
        {
            arywCUError[i] = ADP_CUERROR_EMPTY;
            Log(ThisModule, WFS_ERR_CDM_CASHUNITERROR, "钞箱(%d)已空", i + 1);
            return WFS_ERR_CDM_CASHUNITERROR;
        }
        else if (m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_INOP
                 || m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_MISSING
                 || m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_MANIP)
        {
            arywCUError[i] = ADP_CUERROR_FATAL;
            char strStatus[128] = {0};
            CassStatus2String(m_aryADPCassStatusInfo[i], strStatus);
            Log(ThisModule, WFS_ERR_CDM_CASHUNITERROR, "钞箱(%d)状态(%s)异常", i + 1, strStatus);
            return WFS_ERR_CDM_CASHUNITERROR;
        }
        else
        {
            arywCUError[i] = ADP_CUERROR_OK;
        }
    }
    return WFS_SUCCESS;
}

//功能：出钞
//输入：aryCassCount：每个钞箱须挖钞张数
//输出：aryulDispenseCount：实际挖钞张数
//      aryulRejects：钞箱实际拒钞数
//      arywCUError:  钞箱出错原因
//      aryulRejBillPerDenAll:每种面额拒钞数
//返回：XFS通用错误码及CDM错误码
//说明：内部方法
long CUR2Adapter::DispenseFromAllCass(const ULONG aryCassCount[ADP_MAX_CU_SIZE],
                                      ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                                      ULONG aryulRejects[ADP_MAX_CU_SIZE],
                                      ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE],
                                      ULONG aryulRejBillPerDenAll[ADP_MAX_DENOMINATION_NUM])
{
    const char *ThisModule = "DispenseFromAllCass";
    assert(aryCassCount != NULL);
    assert(aryulDispenseCount != NULL);
    assert(aryulRejects != NULL);
    assert(arywCUError != NULL);
    assert(aryulRejBillPerDenAll != NULL);

    ZeroMemory(aryulDispenseCount, sizeof(ULONG)*ADP_MAX_CU_SIZE);
    ZeroMemory(aryulRejects, sizeof(ULONG)*ADP_MAX_CU_SIZE);

    //AB箱不能禁止
    BOOL bEnable[MAX_CASSETTE_NUM] = {FALSE, FALSE, FALSE, TRUE, FALSE};
    BYTE ucProhibited = 0;

    ST_DISP_DENO pArryDispDeno[MAX_DISP_DENO_NUM];
    ST_DISP_ROOM pArryDispRoom[MAX_DISP_ROOM_NUM];
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumCassFed[MAX_CASSETTE_NUM] = {0};
    int pNumDispensedForPerCass[MAX_CASSETTE_NUM] = {0};
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoSourInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoBVInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoDestInfo;
    ST_DISP_MISSFEED_ROOM stDispMissfeedRoom;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH];

    BOOL bDispenseByDeno = FALSE;

    DWORD *paryDispenseCountPreDeno = new DWORD[m_mapBVSupportNoteID.size()];//按默认面额顺序的出钞数
    memset(paryDispenseCountPreDeno, 0, sizeof(DWORD)*m_mapBVSupportNoteID.size());
    DWORD ulTotalCount = 0;

    for (int i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        if (aryCassCount[i] > 0)
        {
            bEnable[i] = TRUE;
            pArryDispRoom[i].iCassID = (CASSETTE_ROOM_ID)(0x0A | ((i + 1) << 4));
            pArryDispRoom[i].iCashNumber = aryCassCount[i];
            ulTotalCount += aryCassCount[i];

            paryDispenseCountPreDeno[m_aryADPCassSettingInfo[i].usDenoCode - 1] += aryCassCount[i];
        }
        if ((m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_LOW
             && m_sDevConfigInfo.DispenseByDeno == 2)
            || m_sDevConfigInfo.DispenseByDeno == 1)
        {
            bDispenseByDeno = TRUE;
        }

    }

    int m = 0;
    for (int i = 0; i < m_mapBVSupportNoteID.size(); i++)
    {
        if (paryDispenseCountPreDeno[i] > 0)
        {
            pArryDispDeno[m].iDenoCode = (DENOMINATION_CODE)(i + 1);
            pArryDispDeno[m].iCashNumber = paryDispenseCountPreDeno[i];
            m++;
        }
    }

    delete paryDispenseCountPreDeno;
    paryDispenseCountPreDeno = NULL;

    if (bDispenseByDeno)
    {
        for (int i = 0; i < MAX_CASSETTE_NUM; i++)
        {
            if (m_aryADPCassSettingInfo[i].iCassType != ADP_CASSETTE_RECYCLING
                && m_aryADPCassSettingInfo[i].iCassType != ADP_CASSETTE_BILL
                && m_aryADPCassSettingInfo[i].iCassType != ADP_CASSETTE_RETRACT
                && m_aryADPCassSettingInfo[i].iCassType != ADP_CASSETTE_REJECT)
            {
                bEnable[i] = FALSE;
            }
            else
            {
                bEnable[i] = m_arybEnableCassUnit[i];
            }
        }
    }

    GetProhibitedCashUnitBit(bEnable, ucProhibited);

    SetVerificationLevel(SAVESERIALOPERAT_D);
    SetUnfitnoteVerificationLevel(SAVESERIALOPERAT_D);
    SetRejectCounterfeitNote();

    m_sDevConfigInfo.stBVDependentMode = m_sDevConfigInfo.stDispenseBVDependentMode;        //30-00-00-00(FS#0030)
    m_pUR2Dev->SetZeroBVSettingInfo(BVSET_CMDTYPE_DISPENSE, m_sDevConfigInfo.byBVSettingLevel, m_sDevConfigInfo.strBVSettingInfo.c_str());          //30-00-00-00(FS#0025)
    int iRet = m_pUR2Dev->Dispense(m_sDevConfigInfo.CashOutNoteType, ucProhibited,
                                   bDispenseByDeno ? pArryDispDeno : NULL, bDispenseByDeno ? NULL : pArryDispRoom,
                                   m_sDevConfigInfo.stBVDependentMode, iTotalCashInURJB, iNumStackedToCS, iNumStackedToESC,
                                   pNumStackedPerCass, iNumCSFed, iNumESCFed, pNumCassFed, pNumDispensedForPerCass, stStackeNotesDenoInfo,
                                   stStackeRejectNotesDenoSourInfo, stStackeRejectNotesDenoBVInfo, stStackeRejectNotesDenoDestInfo,
                                   stDispMissfeedRoom, pMaintenanceInfo);

    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "挖钞");
    UpdateADPDevStatus(iRet);

    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        if (pNumDispensedForPerCass[i] > 0)
        {
            arywCUError[i] = ADP_CUERROR_OK;
            bEnable[i] = TRUE;
        }
    }

    //得到钞箱出错原因，只与钞箱错误有关
    BOOL bErrCass = m_pErrCodeTrans->GetCUErr(m_szLastErrCode, bEnable, arywCUError);
    bErrCass = GetCUErrorInfoForDispense(stDispMissfeedRoom, arywCUError);             //30-00-00-00(FS#0020)

    if (iRet != 0)
    {
        Log(ThisModule, 0, "NumStackedToCS:%d,iNumCSFed:%d, iNumStackedToESC:%d", iNumStackedToCS, iNumCSFed, iNumStackedToESC);
        Log(ThisModule, 0, "FedNote：cass1(%d),cass2(%d),cass3(%d),cass4(%d),cass5(%d)",
            pNumCassFed[0], pNumCassFed[1], pNumCassFed[2], pNumCassFed[3], pNumCassFed[4]);
        Log(ThisModule, 0, "DispenseNote：cass1(%d),cass2(%d),cass3(%d),cass4(%d),cass5(%d)",
            pNumDispensedForPerCass[0], pNumDispensedForPerCass[1], pNumDispensedForPerCass[2], \
            pNumDispensedForPerCass[3], pNumDispensedForPerCass[4]);
        Log(ThisModule, 0, "pNumStackedPerCass：cass1(%d),cass2(%d),cass3(%d),cass4(%d),cass5(%d)",
            pNumStackedPerCass[0], pNumStackedPerCass[1], pNumStackedPerCass[2],  pNumStackedPerCass[3], pNumStackedPerCass[4]);


        ST_STACKE_NOTES_DENO_INFO stNoteDeInfo;
        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
        {
            stNoteDeInfo = stStackeNotesDenoInfo.stStackeNotesInfo[j];
            if (stNoteDeInfo.usNumberStack > 0)
            {
                Log(ThisModule, 0, "stStackeNotesDenoInfo j:%d, NOTEID:%d, NOTESNUM:%d, CASS:%s", j,
                    stNoteDeInfo.ucDENOCode, stNoteDeInfo.usNumberStack, CassID2String(stNoteDeInfo.iDest).c_str());
            }
        }
        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
        {
            stNoteDeInfo = stStackeRejectNotesDenoSourInfo.stStackeNotesInfo[j];
            if (stNoteDeInfo.usNumberStack > 0)
            {
                Log(ThisModule, 1, "stStackeRejectNotesDenoSourInfo j:%d, NOTEID:%d, NOTESNUM:%d, CASS:%s", j,
                    stNoteDeInfo.ucDENOCode, stNoteDeInfo.usNumberStack, CassID2String(stNoteDeInfo.iDest).c_str());
            }
        }
        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
        {
            stNoteDeInfo = stStackeRejectNotesDenoBVInfo.stStackeNotesInfo[j];
            if (stNoteDeInfo.usNumberStack > 0)
            {
                Log(ThisModule, 1, "stStackeRejectNotesDenoBVInfo j:%d, NOTEID:%d, NOTESNUM:%d, CASS:%s", j,
                    stNoteDeInfo.ucDENOCode, stNoteDeInfo.usNumberStack, CassID2String(stNoteDeInfo.iDest).c_str());
            }
        }
        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
        {
            stNoteDeInfo = stStackeRejectNotesDenoDestInfo.stStackeNotesInfo[j];
            if (stNoteDeInfo.usNumberStack > 0)
            {
                Log(ThisModule, 1, "stStackeRejectNotesDenoDestInfo j:%d, NOTEID:%d, NOTESNUM:%d, CASS:%s", j,
                    stNoteDeInfo.ucDENOCode, stNoteDeInfo.usNumberStack, CassID2String(stNoteDeInfo.iDest).c_str());
            }
        }
    }

    //挖钞不成功但CS可能有部分钞，点钞数仍要返回
    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        aryulDispenseCount[i] = pNumDispensedForPerCass[i];
        if ((pNumCassFed[i] - pNumDispensedForPerCass[i]) > 0
            && i != 3)
        {
            aryulRejects[i] = pNumCassFed[i] - pNumDispensedForPerCass[i];
        }

    }
    aryulRejects[3] = pNumStackedPerCass[3];
    SaveRejBillPerDenoALL(stStackeRejectNotesDenoSourInfo, aryulRejBillPerDenAll);

    //保存序列号（挖钞失败也产生序列号）
 //test#4   if (m_sDevConfigInfo.CashOutNoteType == VALIDATION_MODE_REAL)
 //test#4    {
        CThreadGenerateSN SNWork(SAVESERIALOPERAT_D, m_eLfsCmdId, iRet);     //test#8
        QueryNoteMediaInfo(&SNWork);
        QueryNoteSerialInfo(&SNWork);
  //test#4   }


    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR,
            "挖钞失败硬件错误(ErrCode:%s)(ReturnValue:%d)，应挖钞C1:%d|C2:%d|C3:%d|C4:%d|C5:%d,   实际挖钞C1:%d|C2:%d|C3:%d|C4:%d|C5:%d",
            m_szLastErrCode, iRet, aryCassCount[0], aryCassCount[1], aryCassCount[2], aryCassCount[3], aryCassCount[4],
            aryulDispenseCount[0], aryulDispenseCount[1], aryulDispenseCount[2], aryulDispenseCount[3], aryulDispenseCount[4]);
        return bErrCass ? WFS_ERR_CDM_CASHUNITERROR : WFS_ERR_HARDWARE_ERROR;
    }
    //检查挖钞数和实际需求是否相符 属异常错误
    else if (ulTotalCount != iNumStackedToCS)
    {
        Log(ThisModule, WFS_ERR_CDM_CASHUNITERROR, "挖钞失败(ErrCode:%s)应挖钞数(%d),实际挖钞数(%d)",
            m_szLastErrCode, ulTotalCount, iNumStackedToCS);
        return WFS_ERR_CDM_CASHUNITERROR;
    }

    return WFS_SUCCESS;
}


//功能：测试钞箱配钞
//输入：无
//输出：aryCassItems：每个钞箱须出钞张数
//返回：XFS通用错误码及CDM错误码
//说明：内部方法
long CUR2Adapter::TestCashUnitMixNotes(ULONG aryCassItems[ADP_MAX_CU_SIZE]) const
{
    assert(aryCassItems != NULL);
    memset(aryCassItems, 0, sizeof(unsigned short)*ADP_MAX_CU_SIZE);
    for (int i = 0;  i < ADP_MAX_CU_SIZE; i++)
    {
        //出钞箱须具有取钞功能 只能是取款箱或循环箱
        if ((m_aryADPCassSettingInfo[i].iCassType != ADP_CASSETTE_BILL) &&
            (m_aryADPCassSettingInfo[i].iCassType != ADP_CASSETTE_RECYCLING))
        {
            continue;
        }
        else if (!m_arybEnableCassUnit[i])
        {
            continue;
        }
        //检查出钞箱是否为空
        else if (m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_EMPTY)
        {
            continue;
        }
        else if (m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_INOP
                 || m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_MISSING
                 || m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_MANIP)
        {
            continue;
        }
        else
        {
            aryCassItems[i] = 1;
        }
    }
    return WFS_SUCCESS;
}

//功能：测试钞箱。
//输入：无
//输出：aryulDispenseCount：测试钞箱时每个钞箱点出的钞数,不含废钞数；
//      aryulRejects：对于取款箱,表示废钞数,对于回收箱,表示回收数;
//      arywCUError：钞箱错误原因,0---正常；1--钞箱钞空；2---钞箱硬件故障；3---钞箱满。
//返回：XFS通用错误码及CDM错误码
//说明：
long CUR2Adapter::TestCashUnits(ULONG aryulDispenseCount[ADP_MAX_CU_SIZE],
                                ULONG aryulRejects[ADP_MAX_CU_SIZE],
                                ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "TestCashUnits";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev);
    VERIFYPOINTNOTEMPTY(aryulDispenseCount);
    VERIFYPOINTNOTEMPTY(aryulRejects);
    VERIFYPOINTNOTEMPTY(arywCUError);

    SETCASSUNITUNUSED(arywCUError)
    QueryDevInfo();

    ULONG aryCassItems[ADP_MAX_CU_SIZE] = {0};//测试钞箱配钞
    TestCashUnitMixNotes(aryCassItems);

    int iRet = IsDeviceDispensable(aryCassItems, arywCUError);
    if (iRet < 0)
    {
        Log(ThisModule, iRet, "设备不满足出钞能力");
        return iRet;
    }
    ULONG iRejBillPerDenAll[ADP_MAX_DENOMINATION_NUM] = {0};
    iRet = DispenseFromAllCass(aryCassItems, aryulDispenseCount, aryulRejects, arywCUError, iRejBillPerDenAll);

    //记录出钞日志
    Log(ThisModule, (iRet == WFS_SUCCESS) ? 0 : -1,
        "测试出钞%s: 应出钞：cass1(%d),cass2(%d),cass3(%d),cass4(%d),实际出钞: cass1(%d),cass2(%d),cass3(%d),cass4(%d)",
        (iRet == WFS_SUCCESS) ? "成功" : "失败", aryCassItems[0], aryCassItems[1], aryCassItems[2], aryCassItems[3],
        aryulDispenseCount[0], aryulDispenseCount[1], aryulDispenseCount[2], aryulDispenseCount[3]);

    long lReturnValue = (iRet != WFS_SUCCESS) ? iRet : WFS_SUCCESS;
    //如果钞口有钞就回收所有的钞票
    if (m_sDevStatus.stInPutPosition == ADP_INPOS_NOTEMPTY)
    {
        //回收已挖出的钞票
        int iRetRetract = RetractNoteInCSAfterDispense(aryulDispenseCount, aryulRejects, iRejBillPerDenAll, arywCUError);
        if (iRetRetract < 0)
        {
            Log(ThisModule, iRetRetract,    "测试出钞回收失败");
            lReturnValue = iRetRetract;
        }
    }

    SaveRejectBill(iRejBillPerDenAll);

    return lReturnValue;
}

//30-00-00-00(FS#0007)
//输入：byRoomExclusion：room是否在测试范围内(0:范围内 1:范围外)
long CUR2Adapter::TestCashUnitsHT(BYTE byRoomExclusion[2][5], TESTDISPINFO &stTestDispInfo, ADP_CUERROR wCUErrors[])
{
    const char *ThisModule = "TestCashUnitsHT";
    TESTCASHUNITSOUTPUT stTestCashUnitsOutput = {0};
    int iRet = m_pUR2Dev->TestCashUnits(m_sDevConfigInfo.CashOutNoteType, byRoomExclusion, 0, stTestCashUnitsOutput);

    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "TestCashUnitsHT");
    UpdateADPDevStatus(iRet);

    memcpy(&stTestDispInfo, &stTestCashUnitsOutput, sizeof(stTestCashUnitsOutput.wNumOfCSPerRoom) * 4);
    //计算room拒钞张数
    for(int i = 0; i < 2; i++){
        for(int j = 0; j < 5; j++){
            stTestDispInfo.wRBBoxRJCount[i][j] = stTestDispInfo.wCountFromRoom[i][j] - stTestDispInfo.wNumofCSPerRoom[i][j];
            if(stTestDispInfo.wRBBoxRJCount[i][j] < 0){
                stTestDispInfo.wRBBoxRJCount[i][j] = 0;
            }
        }
    }
    for(int i = 0; i < stTestCashUnitsOutput.stPerDenomiNumofRJ.ucCount; i++){
        //NoteId转换为内部id
        map<BYTE, WFSCIMNOTETYPE>::const_iterator mit = m_mapSupportNotID.find(stTestCashUnitsOutput.stPerDenomiNumofRJ.stStackeNotesInfo[i].ucDENOCode);
        if(mit != m_mapBVSupportNoteID.end()){
            stTestDispInfo.stPerDenomiNumofRJ[i].byNoteID = mit->second.usNoteID;
            stTestDispInfo.stPerDenomiNumofRJ[i].ulCount = stTestCashUnitsOutput.stPerDenomiNumofRJ.stStackeNotesInfo[i].usNumberStack;
        }
    }
    bool bCUError = false;
    for(int i = 0; i < 5; i++){
        ADP_CUERROR eCUErr;
        switch(stTestCashUnitsOutput.eRoomError[0][i]){
        case NO_ERROR:
            eCUErr = ADP_CUERROR_OK;
            break;
        case CST_EMPTY:
            eCUErr = ADP_CUERROR_EMPTY;
            break;
        default:
            eCUErr = ADP_CUERROR_FATAL;
        }
        if(eCUErr != ADP_CUERROR_OK){
            bCUError = true;
        }
        wCUErrors[i] = eCUErr;
    }

    if(bCUError){
        return WFS_ERR_CDM_CASHUNITERROR;
    }
    //返回值处理
    if(iRet != 0){        
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}

//功能：取固件版本。
//输入：无
//输出：cVersion,统计信息,长度包括0结尾不超过512个字符
//返回：XFS通用错误码
//说明：
long CUR2Adapter::GetFirmwareVersion(char cVersion[512])
{
    const char *ThisModule = "GetFirmwareVersion";
    VERIFYPOINTNOTEMPTY(cVersion)
    if (m_szFWVersion[0] == 0)
    {
        int iRet = QueryFWVersion();
        if (iRet < 0)
        {
            Log(ThisModule, iRet, "取版本信息失败");
            return iRet;
        }
    }
    //保存获取的硬件版本信息
    memset(cVersion, 0, sizeof(char) * 512);
    memcpy(cVersion, m_szFWVersion, MAX_FW_DATA_LENGTH - 1);
    return WFS_SUCCESS;

}

//功能：取固件版本。
//输入：无
//输出：无
//返回：XFS通用错误码
//说明：内部函数 获取版本信息并写入成员数据
long CUR2Adapter::QueryFWVersion()
{
    const char *ThisModule = "QueryFWVersion";

    char szFWVerInfo[MAX_FW_DATA_LENGTH] = {0};
    USHORT usLen = 0;
    int iRet = m_pUR2Dev->GetFWVersion(szFWVerInfo, usLen, TRUE);
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)  // todo
    {
        Log(ThisModule, iRet, "取版本信息失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

    memset(m_szFWVersion, 0, sizeof(char)*MAX_FW_DATA_LENGTH);
    char szBuf[32] = {0};
    string strTemp;
    for (int i = 0; i < 8; i++)
    {
        memcpy(szBuf, szFWVerInfo + 28 * i, 28);
        DeleteChar(szBuf, ' ');
        FWVersionInfo FWverinfo;
        FWverinfo.usCTLID = i + 0X0591;
        if (strlen(szBuf) > 0)
        {
            memcpy(FWverinfo.sProgramName, szFWVerInfo + 28 * i, 16);
            memcpy(FWverinfo.sVersion, szFWVerInfo + 28 * i + 16, 8);

            strTemp += FWverinfo.sProgramName;
            strTemp += " = ";
            strTemp += FWverinfo.sVersion;
            strTemp += "\r\n";

            DeleteChar(FWverinfo.sProgramName, ' ');
            DeleteChar(FWverinfo.sVersion, ' ');
            m_mapURFWVerInfo[i] = FWverinfo;
        }
    }

    if (strTemp.size() >= sizeof(m_szFWVersion))
    {
        Log(ThisModule, WFS_ERR_INTERNAL_ERROR, "取版本信息失败,获取字符串长度(%d) > 指定长度(%d)", strTemp.size(), sizeof(m_szFWVersion));
        return WFS_ERR_INTERNAL_ERROR;
    }
    Log(ThisModule, 1, "取版本信息成功：%s", strTemp.c_str());
    strTemp.copy(m_szFWVersion, strTemp.size(), 0);
    return WFS_SUCCESS;
}

LPCSTR CUR2Adapter::ConvertStatusToStr(CASSETTE_STATUS boxstatus)
{
    switch (boxstatus)
    {
    case (ADP_CASHUNIT_OK):
        return "NORMAL";
    case (ADP_CASHUNIT_FULL):
        return "FULL";
    case (ADP_CASHUNIT_HIGH):
        return "NEARFULL";
    case (ADP_CASHUNIT_LOW):
        return "NEAREMPTY";
    case (ADP_CASHUNIT_EMPTY):
        return "EMPTY";
    case (ADP_CASHUNIT_MISSING):
        return "MISSING";
    default:
        return "UNKNOWN";
    }
}
//功能：获取附加状态
//输入：pExtra：附加信息设置接口指针,调用该接口完成附加状态的设置
//输出：无
//返回：XFS通用错误码
//说明：
long CUR2Adapter::GetExtraInfor(IBRMExtra *pExtra)
{
    const char *ThisModule = "GetExtraInfor";
    VERIFYPOINTNOTEMPTY(pExtra)
    ST_DEV_STATUS_INFO stDevStatusInfo;
    m_pUR2Dev->GetDevStatusInfo(stDevStatusInfo);
    //    pExtra->AddExtra("CS_EXIST_CASH", (stDevStatusInfo.bCashAtCS ? "1" : "0"));
    //    pExtra->AddExtra("SHUTTER_EXIST_CASH", (stDevStatusInfo.bCashAtShutter ? "1" : "0"));
    //    pExtra->AddExtra("CSERRPOS_EXIST_CASH", (stDevStatusInfo.bCashAtCSErrPos ? "1" : "0"));
    //    pExtra->AddExtra("BRM_SET", (m_bBRMInitialized ? "1" : "0"));
    //    pExtra->AddExtra("ESC_EMPTY", (stDevStatusInfo.bESCEmpty ? "1" : "0"));
    //    pExtra->AddExtra("ESC_FULL", (stDevStatusInfo.bESCFull ? "1" : "0"));
    //    pExtra->AddExtra("ESC_OPEN", (stDevStatusInfo.bESCOpen ? "1" : "0"));
    //    pExtra->AddExtra("BVFAN_WARNING", (stDevStatusInfo.bBVFanErr ? "1" : "0"));
    //    pExtra->AddExtra("BV_OPEN", (stDevStatusInfo.bBVOpen ? "1" : "0"));
    //    pExtra->AddExtra("URJB_FULL", (stDevStatusInfo.bURJBFull ? "1" : "0"));
    //    pExtra->AddExtra("URJB_EMPTY", (stDevStatusInfo.bURJBEmpty ? "1" : "0"));
    //    pExtra->AddExtra("URJB_SHUTTER", (stDevStatusInfo.bURJBOpen ? "1" : "0"));

    pExtra->AddExtra("OUTSHUTTER_CLOSED", (stDevStatusInfo.iOutShutterStatus == UR2SHUTTER_STATUS_CLOSED ? "1" : "0"));
    //  pExtra->AddExtra("INNERSHUTTER_CLOSED", (stDevStatusInfo.iInnShutterStatus == UR2SHUTTER_STATUS_CLOSED ? "1" : "0"));

    for (int i = 0; i < (MAX_CASSETTE_NUM - 1); i++)
    {
        char sztemp[128] = {0};
        sprintf(sztemp, "BOX%d_INPOSITION", i + 1);
        pExtra->AddExtra(sztemp, (stDevStatusInfo.bCassInPos[i] ? "1" : "0"));
        sprintf(sztemp, "BOX%d_STATUS", i + 1);
        pExtra->AddExtra(sztemp, ConvertStatusToStr(stDevStatusInfo.CassStatus[i]));
    }

    pExtra->AddExtra("REQRESET", (stDevStatusInfo.bReqReset ? "1" : "0"));

    pExtra->AddExtra("UPPER_UNIT", (stDevStatusInfo.bHCMUPInPos ? "1" : "0"));
    pExtra->AddExtra("LOWER_UNIT", (stDevStatusInfo.bHCMLOWInPos ? "1" : "0"));

    std::string strPosCode;
    char szLampIdx[10];
    for (size_t i = 0; i < sizeof(m_ucPostionCode); i++) {
        for (int j = 0; j < 8; j++) {
            BYTE byMask = (0x80 >> j);
            if ((m_ucPostionCode[i] & byMask) != 0x00) {
                size_t sztLampNo = 8 * i + j + 1;
                memset(szLampIdx, 0x00, sizeof(szLampIdx));
                sprintf(szLampIdx, "%lu", sztLampNo);
                if (strPosCode.empty() == false) {
                    strPosCode.append(",");
                }
                strPosCode.append(szLampIdx);
            }
        }
    }
    pExtra->AddExtra("POSITION_CODE",  strPosCode.c_str());

 //test#1   pExtra->AddExtra("ACTION_ERR_DETAIL", m_szLastErrCodeAction);

    if(memcmp(m_szLastErrCodeAction, "00000000",8) != 0){           //test#1
        pExtra->AddExtra("ERR_DETAIL",  m_szLastErrCodeAction);     //test#1
    }else if(memcmp(m_szLastErrCode, "00000000",8) != 0){           //test#1
        pExtra->AddExtra("ERR_DETAIL",  m_szLastErrCode);
    }else{                                                          //test#1
        pExtra->AddExtra("ERR_DETAIL",  m_szLastErrCodeAction);     //test#1
    }                                                               //test#1

    pExtra->AddExtra("FIRMWARE VERSION",  m_szFWVersion);
    pExtra->AddExtra("FWDownLoading",  m_FWDStatus == FWDOWNSTATUS_DLING ? "1" : "0");

    pExtra->AddExtra("REJECT_BILL", m_szRejectBill);
    pExtra->AddExtra("SP VERSION", m_szSPVersion);

    /*char szBuff[32] = {0};
    string strTemp;
    for (int j = 0; j < 16; j++)
    {
        sprintf(szBuff, "%02x", m_ucPostionCode[j]);
        strTemp += szBuff;
    }
    //pExtra->AddExtra("POSITION_CODE",  strTemp.c_str());
    sprintf(szBuff, "%d", m_usRecoveryCode);
    //pExtra->AddExtra("RECOVERY_CODE",  szBuff);*/

    return WFS_SUCCESS;
}


//功能：通知机芯进入交换状态,机芯可根据实际情况执行相应的预处理。
//输入：arybExFlag：将被交换的钱箱。
//输出：无
//返回：XFS通用错误码及CDM错误码
//说明：
long CUR2Adapter::StartExchange(const BOOL arybExFlag[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "StartExchange";
    VERIFYPOINTNOTEMPTY(arybExFlag)
    if (!m_bConnectNormal)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "通信连接出现故障");
        return WFS_ERR_HARDWARE_ERROR;
    }
    m_bExchangeActive = TRUE;
    return WFS_SUCCESS;
}

//功能：通知机芯退出交换状态,机芯可根据实际情况执行相应的收尾处理。
//输入：无
//输出：无
//返回：XFS通用错误码及CDM错误码
//说明：
long CUR2Adapter::EndExchange()
{
    const char *ThisModule = "EndExchange";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    for (int i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        if (((m_aryADPCassStatusInfo[i] == ADP_CASHUNIT_MISSING) && (m_aryCassStatusWhenEndEx[i] != ADP_CASHUNIT_MISSING))
            || ((m_aryADPCassStatusInfo[i] != ADP_CASHUNIT_MISSING) && (m_aryCassStatusWhenEndEx[i] == ADP_CASHUNIT_MISSING)))
        {
            m_bIsNeedReset = TRUE;
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "VertifyCass failed");
        }
        m_aryCassStatusWhenEndEx[i] = m_aryADPCassStatusInfo[i];
    }

    m_bExchangeActive = FALSE;
    return QueryDevInfo();
}


//功能：在执行复位机芯、Retract及Reject操作时,机芯可能会回收到钞票,通过此方法可获取回收到每个钞箱的钞数。
//输入：无
//输出：arywRetractCount 获取回收钞票张数，在执行可能产生回收的操作之后,得到回收的钞数信息。
//返回：0---获取成功；
//      其它---获取失败
//说明：
long CUR2Adapter::GetRetractCount(ULONG arywRetractCount[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "GetRetractCount";
    VERIFYPOINTNOTEMPTY(arywRetractCount)
    memcpy(arywRetractCount, m_aryRetractCount, sizeof(m_aryRetractCount));
    memset(m_aryRetractCount, 0, sizeof(m_aryRetractCount));
    return WFS_SUCCESS;
}

//功能：开门
//输入：无
//输出：无
//返回：XFS通用错误码
//说明：
long CUR2Adapter::OpenShutter(BOOL bInShutter)
{
    const char *ThisModule = "OpenShutter";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    return OpenDeviceShutter();
}

//功能：关门
//输入：无
//输出：无
//返回：XFS通用错误码
//说明：
long CUR2Adapter::CloseShutter(BOOL bInShutter)
{
    const char *ThisModule = "CloseShutter";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    return CloseDeviceShutter();
}

long CUR2Adapter::RetractNotesToAB(CASSETTE_NUMBER RetractCassNum, ADP_CUERROR arywCuError[ADP_MAX_CU_SIZE])
{
    LPCSTR ThisModule = "RetractNotesToAB";

    ULONG aryRetractCountPerDeno[ADP_MAX_DENOMINATION_NUM] = {0};

    DESTINATION_REJCET iDestinationReject = DESTINATION_REJCET_DEFAULT;
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM] = {0};
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM] = {0};
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeUnfitNotesDenoInfo;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeRejectNotesDenoInfo;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH];
    BOOL bEnable[MAX_CASSETTE_NUM] = {FALSE};
    if (RetractCassNum == CASSETTE_4 || RetractCassNum == CASSETTE_6)
    {
        bEnable[RetractCassNum - 1] = TRUE;
    }
    else
    {
        Log(ThisModule, WFS_ERR_UNSUPP_DATA, "回收钞箱ID非法（%d）", RetractCassNum);
        return WFS_ERR_UNSUPP_DATA;
    }
    BYTE btProhibitedBox = 0;
    GetProhibitedCashUnitBit(bEnable, btProhibitedBox);
    Log(ThisModule, 1, "RetractToAB Start");

    m_sDevConfigInfo.stBVDependentMode = m_sDevConfigInfo.stRetractBVDependentMode;        //30-00-00-00(FS#0030)
    int iRet = m_pUR2Dev->RetractESC(
               m_sDevConfigInfo.CashInNoteType, btProhibitedBox, iDestinationReject, m_sDevConfigInfo.stBVDependentMode,
               iTotalCashInURJB, iNumStackedToCS,  iNumStackedToESC, pNumStackedToPerCass, iNumCSFed, iNumESCFed,
               pNumPerCassFed, stStackeNotesDenoInfo, stStackeUnfitNotesDenoInfo,  stStackeRejectNotesDenoInfo,
               pMaintenanceInfo);
    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "回收到回收箱");
    UpdateADPDevStatus(iRet);

    if (m_sDevConfigInfo.bRetractCashCountCalibration)                          //30-00-00-00(FT#0037)
    {                                                                           //30-00-00-00(FT#0037)
        ST_STACKE_NOTES_DENO_INFO* lpstNoteInfo = nullptr;                      //30-00-00-00(FT#0037)
        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)                          //30-00-00-00(FT#0037)
        {                                                                       //30-00-00-00(FT#0037)
            lpstNoteInfo = &stStackeNotesDenoInfo.stStackeNotesInfo[j];         //30-00-00-00(FT#0037)
            if (lpstNoteInfo->ucDENOCode == DENOMINATION_CODE_100_C)            //30-00-00-00(FT#0037)
            {                                                                   //30-00-00-00(FT#0037)
                if (m_iNumStackedToESC <= lpstNoteInfo->usNumberStack)          //30-00-00-00(FT#0037)
                    break;                                                      //30-00-00-00(FT#0037)
                lpstNoteInfo->usNumberStack = m_iNumStackedToESC;               //30-00-00-00(FT#0037)
            } else if (lpstNoteInfo->ucDENOCode == 0x80)                        //30-00-00-00(FT#0037)
            {                                                                   //30-00-00-00(FT#0037)
                lpstNoteInfo->usNumberStack = 0;                                //30-00-00-00(FT#0037)
            }                                                                   //30-00-00-00(FT#0037)
        }                                                                       //30-00-00-00(FT#0037)
                                                                                //30-00-00-00(FT#0037)
        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)                          //30-00-00-00(FT#0037)
        {                                                                       //30-00-00-00(FT#0037)
            lpstNoteInfo = &stStackeUnfitNotesDenoInfo.stStackeNotesInfo[j];    //30-00-00-00(FT#0037)
            if (lpstNoteInfo->ucDENOCode == 0x80)                               //30-00-00-00(FT#0037)
                lpstNoteInfo->usNumberStack = 0;                                //30-00-00-00(FT#0037)
        }                                                                       //30-00-00-00(FT#0037)
                                                                                //30-00-00-00(FT#0037)
        pNumStackedToPerCass[3] = m_iNumStackedToESC;                           //30-00-00-00(FT#0037)
        m_iNumStackedToESC = 0;                                                 //30-00-00-00(FT#0037)
    }                                                                           //30-00-00-00(FT#0037)

    if (TRUE)
    {
        Log(ThisModule, 0, "NumStackedToCS:%d,iNumCSFed:%d, iNumStackedToESC:%d", iNumStackedToCS, iNumCSFed, iNumStackedToESC);
        Log(ThisModule, 0, "FedNote：cass1(%d),cass2(%d),cass3(%d),cass4(%d),cass5(%d)",
            pNumPerCassFed[0], pNumPerCassFed[1], pNumPerCassFed[2], pNumPerCassFed[3], pNumPerCassFed[4]);
        Log(ThisModule, 0, "pNumStackedPerCass：cass1(%d),cass2(%d),cass3(%d),cass4(%d),cass5(%d)",
            pNumStackedToPerCass[0], pNumStackedToPerCass[1], pNumStackedToPerCass[2],  pNumStackedToPerCass[3], pNumStackedToPerCass[4]);


        ST_STACKE_NOTES_DENO_INFO stNoteDeInfo;
        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
        {
            stNoteDeInfo = stStackeNotesDenoInfo.stStackeNotesInfo[j];
            if (stNoteDeInfo.usNumberStack > 0)
            {
                Log(ThisModule, 0, "stStackeNotesDenoInfo j:%d, NOTEID:%d, NOTESNUM:%d, CASS:%s", j,
                    stNoteDeInfo.ucDENOCode, stNoteDeInfo.usNumberStack, CassID2String(stNoteDeInfo.iDest).c_str());
            }
        }

        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
        {
            stNoteDeInfo = stStackeUnfitNotesDenoInfo.stStackeNotesInfo[j];
            if (stNoteDeInfo.usNumberStack > 0)
            {
                Log(ThisModule, 1, "stStackeUnfitNotesDenoInfo j:%d, NOTEID:%d, NOTESNUM:%d, CASS:%s", j,
                    stNoteDeInfo.ucDENOCode, stNoteDeInfo.usNumberStack, CassID2String(stNoteDeInfo.iDest).c_str());
            }
        }

        for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
        {
            stNoteDeInfo = stStackeRejectNotesDenoInfo.stStackeNotesInfo[j];
            if (stNoteDeInfo.usNumberStack > 0)
            {
                Log(ThisModule, 1, "stStackeRejectNotesDenoInfo j:%d, NOTEID:%d, NOTESNUM:%d, CASS:%s", j,
                    stNoteDeInfo.ucDENOCode, stNoteDeInfo.usNumberStack, CassID2String(stNoteDeInfo.iDest).c_str());
            }
        }
    }

    //获取钞箱错误信息
    for (int i = 0; i < ADP_MAX_CU_SIZE; i++)
    {
        if (pNumStackedToPerCass[i] > 0)
        {
            arywCuError[i] = ADP_CUERROR_OK;
        }
    }

    BOOL bCassErr = m_pErrCodeTrans->GetCUErr(m_szLastErrCode, m_arybEnableCassUnit, arywCuError);

 //test#4   if (m_sDevConfigInfo.CashOutNoteType == VALIDATION_MODE_REAL)
 //test#4  {
        CThreadGenerateSN SNWork(SAVESERIALOPERAT_R, m_eLfsCmdId, iRet);    //test#8
        QueryNoteMediaInfo(&SNWork);
        QueryNoteSerialInfo(&SNWork);
 //test#4   }

    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        m_aryRetractCount[i] += pNumStackedToPerCass[i];
    }

    SaveRejBillPerDenoALL(stStackeNotesDenoInfo, aryRetractCountPerDeno);
    SaveRejectBill(aryRetractCountPerDeno);
    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "CS回收失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return WFS_ERR_HARDWARE_ERROR;
    }
    return 0;
}

long  CUR2Adapter::RetractNotesToESC(ADP_CUERROR arywCuError[ADP_MAX_CU_SIZE], int iRequiredRetractArea)    //test#8
{
    LPCSTR ThisModule = "RetractNotesToESC";

    ULONG aryRetractCountPerDeno[ADP_MAX_DENOMINATION_NUM] = {0};

    DESTINATION_REJCET iDestinationReject = DESTINATION_REJCET_DEFAULT;
    BYTE btProhibitedBox = 0;
    int iTotalCashInURJB = 0;
    int iNumStackedToCS = 0;
    int iNumStackedToESC = 0;
    int pNumStackedToPerCass[MAX_CASSETTE_NUM];
    int iNumCSFed = 0;
    int iNumESCFed = 0;
    int pNumPerCassFed[MAX_CASSETTE_NUM];
    int nRejectNotes = 0;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stStackeNotesDenoInfo;
    USHORT usFedNoteCondition = 0;
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH];
    BOOL bIgnoreESC = TRUE;

    m_sDevConfigInfo.stBVDependentMode.bEnableBVModeSetting = FALSE;        //30-00-00-00(FS#0030)
    int iRet = m_pUR2Dev->CashCountRetract(
               m_sDevConfigInfo.CashOutNoteType,
               m_sDevConfigInfo.stBVDependentMode,
               iTotalCashInURJB,
               iNumStackedToCS,
               iNumStackedToESC,
               pNumStackedToPerCass,
               iNumCSFed,
               iNumESCFed,
               pNumPerCassFed,
               nRejectNotes,
               stStackeNotesDenoInfo,
               usFedNoteCondition,
               pMaintenanceInfo,
               bIgnoreESC);
    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "回收到ESC");
    UpdateADPDevStatus(iRet);

    m_iNumStackedToESC = iNumStackedToESC;                  //30-00-00-00(FT#0037)
    SaveRejBillPerDenoALL(stStackeNotesDenoInfo, aryRetractCountPerDeno);
    SaveRejectBill(aryRetractCountPerDeno);
    if (iRet < 0)
    {
        Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "回收钞票到ESC失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return WFS_ERR_HARDWARE_ERROR;
    }

 //test#4   if (m_sDevConfigInfo.CashOutNoteType == VALIDATION_MODE_REAL)
 //test#4   {
    if(iRequiredRetractArea != 1 && iRequiredRetractArea != 4){             //test#8
        CThreadGenerateSN SNWork(SAVESERIALOPERAT_R, m_eLfsCmdId, iRet);    //test#8
        QueryNoteMediaInfo(&SNWork);
        QueryNoteSerialInfo(&SNWork);
    }                                                                       //test#8
 //test#4   }

    return 0;
}

//功能：回收钞票
//输入：usRetraceArea：钞票回收的目的位置 1:回收箱，2：通道 3：暂存区，4：废钞箱 5：循环箱 6:出钞口 仅支持1，3，4，6
//      usIndex: 物理钞箱的序号从1开始
//输出：arywCuError: 钞箱错误原因
//返回：XFS通用错误码及CDM错误码
//说明：内部函数，执行CDM和CIM的回收指令
long CUR2Adapter::RetractNotes(USHORT usRetraceArea, USHORT usIndex, ADP_CUERROR arywCuError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "RetractNotes";
    assert(m_pUR2Dev != NULL);
    assert(arywCuError != NULL);

    ZeroMemory(m_aryRetractCount, sizeof(ULONG)*ADP_MAX_CU_SIZE);
    ZeroMemory(m_szRejectBill, sizeof(m_szRejectBill));
    ZeroMemory(m_aryulRetractCountPerDeno, sizeof(m_aryulRetractCountPerDeno));
    m_bCashOutBeforePresent = FALSE;
    QueryDevInfo();


    Log(ThisModule, 1, "usRetraceArea:%d,usIndex:%d", usRetraceArea, usIndex);
    if (usRetraceArea == 2 || usRetraceArea == 5)
    {
        return WFS_ERR_CDM_UNSUPPOSITION;
    }
    else if (usRetraceArea == 3)
    {
        return RetractNotesToESC(arywCuError, usRetraceArea) ;      //test#8
    }

    if (m_sDevStatus.stStacker == ADP_STACKER_NOTEMPTY || m_sDevStatus.stStacker == ADP_STACKER_FULL)         //30-00-00-00(FT#0029)
    {
        //将TS的钞票退回到CS
        int iRet = RollBack(arywCuError);
        if (iRet < 0)
        {
            Log(ThisModule, WFS_ERR_HARDWARE_ERROR, "TS钞票退回到CS失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
            return iRet;
        }

    }
    else if (m_sDevStatus.stStacker == ADP_STACKER_EMPTY)
    {
        if (m_sDevStatus.stInPutPosition == ADP_INPOS_EMPTY)
        {
            Log(ThisModule, WFS_ERR_CDM_NOITEMS, "CS TS均无钞票可回收");
            return WFS_ERR_CDM_NOITEMS;
        }
        else if (m_sDevStatus.stInPutPosition != ADP_INPOS_NOTEMPTY)
        {
            Log(ThisModule, WFS_ERR_CDM_NOITEMS, "CS状态异常:%d", m_sDevStatus.stInPutPosition);
            return WFS_ERR_HARDWARE_ERROR;
        }
    }
    else
    {
        Log(ThisModule, WFS_ERR_CDM_NOITEMS, "ESC状态异常:%d", m_sDevStatus.stStacker);
        return WFS_ERR_HARDWARE_ERROR;
    }

    if (usRetraceArea == 6)
    {
        return WFS_SUCCESS;
    }

    int iRet = RetractNotesToESC(arywCuError, usRetraceArea) ;
    if (iRet < 0)
    {
        return iRet;
    }
    Log(ThisModule, 1, "RetractNotesToESC Success");
    //仅支持回收箱类型为 AB 或者 URJB, 废钞箱为AB,
    if ((usRetraceArea == 1 && usIndex == 4)
        || (usRetraceArea == 4 && usIndex == 4))
    {
        if (m_aryADPCassStatusInfo[3] == CASSETTE_STATUS_FULL)
        {
            arywCuError[3] = ADP_CUERROR_FULL;
            Log(ThisModule, WFS_ERR_CDM_CASHUNITERROR, "回收失败:AB箱满");
            return WFS_ERR_CDM_CASHUNITERROR;
        }

        if (m_aryADPCassStatusInfo[3] == CASSETTE_STATUS_UNKNOWN)
        {
            arywCuError[3] = ADP_CUERROR_FATAL;
            Log(ThisModule, WFS_ERR_CDM_CASHUNITERROR, "回收失败:AB箱状态异常");
            return WFS_ERR_CDM_CASHUNITERROR;
        }

        return RetractNotesToAB(CASSETTE_4, arywCuError);
    }
    else if (usRetraceArea == 1 && usIndex == 6)
    {
        Log(ThisModule, WFS_ERR_CDM_UNSUPPOSITION, "回收失败: 回收位置不支持URJB");
        return WFS_ERR_UNSUPP_DATA;
        //return RetractNotesToAB(CASSETTE_6, arywCuError);
    }
    else
    {
        Log(ThisModule, WFS_ERR_CDM_UNSUPPOSITION, "回收失败:回收位置不支持,回收目的位置:%d，钞箱序号:%d", usRetraceArea, usIndex);
        return WFS_ERR_CDM_UNSUPPOSITION;
    }

    return WFS_SUCCESS;
}


//功能：用bit位设置启用或禁止的钞箱
//输入：bEnable：FALSE表示禁止
//输出：usProhibited：1字节，0位~4位分别表示钱箱1~5,其余位值恒为0。位值为1表示禁用该钱箱
//返回：无
//说明：内部方法
void CUR2Adapter::GetProhibitedCashUnitBit(const BOOL bEnable[MAX_CASSETTE_NUM], BYTE &usProhibited)
{
    assert(bEnable != NULL);
    BYTE bTemp = 0x00;
    usProhibited = 0x00;

    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        bTemp = bEnable[i] ? 0x00 : (0x01 << i);
        usProhibited |= bTemp;
    }
}

//功能：设置机芯BV支持的所有币种类型 见文档 Denomination Code Table
//输入：无
//输出：无
//返回：无
//说明：内部方法
void CUR2Adapter::InitBVSupportNotesType()
{
    WFSCIMNOTETYPE BVSupportNoteInfo;
    BVSupportNoteInfo.bConfigured = TRUE;// 无意义
    BVSupportNoteInfo.usNoteID = 0;      // 无意义
    memcpy(BVSupportNoteInfo.cCurrencyID, "CNY", 3);//只支持人民币

    BVSupportNoteInfo.ulValues = 10;
    BVSupportNoteInfo.usRelease = 4;
    m_mapBVSupportNoteID[0x01] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 50;
    BVSupportNoteInfo.usRelease = 4;
    m_mapBVSupportNoteID[0x02] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 100;
    BVSupportNoteInfo.usRelease = 4;
    m_mapBVSupportNoteID[0x03] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 100;
    BVSupportNoteInfo.usRelease = 5;
    m_mapBVSupportNoteID[0x04] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 20;
    BVSupportNoteInfo.usRelease = 5;
    m_mapBVSupportNoteID[0x05] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 10;
    BVSupportNoteInfo.usRelease = 5;
    m_mapBVSupportNoteID[0x06] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 50;
    BVSupportNoteInfo.usRelease = 5;
    m_mapBVSupportNoteID[0x07] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 1;
    BVSupportNoteInfo.usRelease = 5;
    m_mapBVSupportNoteID[0x08] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 5;
    BVSupportNoteInfo.usRelease = 5;
    m_mapBVSupportNoteID[0x09] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 100;
    BVSupportNoteInfo.usRelease = 1999;
    m_mapBVSupportNoteID[0x0E] = BVSupportNoteInfo;
    BVSupportNoteInfo.ulValues = 0;
    BVSupportNoteInfo.usRelease = 0;
    BVSupportNoteInfo.usNoteID = NOTE_ID_UNKNOWN;
    m_mapBVSupportNoteID[0x80] = BVSupportNoteInfo;
    return ;
}


//功能：获取钞票Media信息
//输入：pwork: 处理Media信息的类指针
//输出：无
//返回：TRUE:获取成功，FALSE:获取失败
//说明：内部方法

BOOL CUR2Adapter::QueryNoteMediaInfo(CThreadGenerateSN *pwork)
{
    const char *ThisModule = "QueryNoteMediaInfo";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)

    BOOL bSucc = TRUE;
    //查询钞票序列号外的其他信息（到了何处、拒钞原因等）
    long lRet = 0;
    USHORT usIndex = 1;
    USHORT  usNumNotes = 0;
    USHORT  usNumTotalNotes = 0;
    ST_MEDIA_INFORMATION_INFO  aryMedioInfo[MAX_MEDIA_INFO_NUM];
    BOOL bSearchBVInternalCount(TRUE);
    pwork->setFwVerInfo(m_szFWVersion);      //30-00-00-00(FS#0001)   
    BYTE byMediaInfoType = 0;        //0:所有纸币 1:有全画像纸币 2:有冠字号纸币 //30-00-00-00(FS#0022)

    while (TRUE)
    {
        lRet = m_pUR2Dev->GetMediaInformation(byMediaInfoType, usIndex, usNumNotes, usNumTotalNotes, aryMedioInfo);
        if (lRet < 0)
        {
            Log(ThisModule, lRet, "调用驱动接口：GetMediaInformation失败");
            //EndBVCommunication();
            return FALSE;
        }

        for (int i = 0; i < lRet; i++)
        {
            if (bSearchBVInternalCount)
            {
                //判断是否取到了前一笔的过期数据
                if (m_sLastestInternalCounts.find(aryMedioInfo[i].ulBVInternalCounter) != m_sLastestInternalCounts.end())
                {
                    Log(ThisModule, lRet, "InternalCount：%d 在上一笔交易中已出现");
                    //EndBVCommunication();
                    return FALSE;
                }
                bSearchBVInternalCount = FALSE;
                m_sLastestInternalCounts.clear();
            }
            //自动精查，AB返回RB纸币信息忽略，防止记录两次
            if(ADP_CIM_SELFCOUNT == m_eLfsCmdId && ID_ROOM_5 == aryMedioInfo[i].iMediaInfoOrigin){      //30-00-00-00(FS#0022)
                continue;                                                                               //30-00-00-00(FS#0022)
            }                                                                                           //30-00-00-00(FS#0022)

            m_sLastestInternalCounts.insert(aryMedioInfo[i].ulBVInternalCounter);
            pwork->InsertTraceInfoDetail(aryMedioInfo[i]);
        }

        if (lRet >= 0  && lRet < MAX_MEDIA_INFO_NUM)
        {
            //EndBVCommunication();
            return TRUE;
        }
        usIndex++;
    }

    //EndBVCommunication();

}

//功能：获取序列号信息
//输入：enuSSO : 调用本函数的操作
//输出：无
//返回：TRUE:获取成功，FALSE:获取失败
//说明：内部方法
BOOL CUR2Adapter::QueryNoteSerialInfo(CThreadGenerateSN *pwork)
{
    const char *ThisModule = "QueryNoteSerialInfo";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)

    //读取所有的序列号信息
    long lRet = 0;
    ULONG  dwIndex(0);

    SNOTESERIALINFO sNoteSNInfo;
    ULONG dwIndexFullImg(0);

    for (dwIndex; dwIndex < m_sLastestInternalCounts.size(); dwIndex++)
    {
        if (pwork->IsNeedSaveSNIMG())
        {
            lRet = m_pUR2Dev->GetNoteSeiralInfo(dwIndex, &sNoteSNInfo);
            if (lRet >= 0)
            {
                pwork->InsertSNInfoDetail(sNoteSNInfo);
            }
        }
    }

    if (m_sDevConfigInfo.stBVDependentMode.bEnableBVModeSetting &&
        m_sDevConfigInfo.stBVDependentMode.iFullImageMode != FULLIMAGE_NO_RECORDS &&
        pwork->IsNeedSaveFullImg())
    {
        for (dwIndex = 0; dwIndex < (m_sLastestInternalCounts.size() * 2); dwIndex++)
        {
            SNOTESERIALINFOFULLIMAGE NoteSerialInfoFullImage;
            lRet = m_pUR2Dev->GetNoteSeiralInfoFullImage(dwIndexFullImg++, &NoteSerialInfoFullImage);
            if (lRet >= 0 && (NoteSerialInfoFullImage.dwImgDataLen > 0))
            {
                if (pwork->IsFullImgDataExist(TRUE, NoteSerialInfoFullImage.dwBVInternalIndex))
                {
                    pwork->InsertFullImageInfoDetail(NoteSerialInfoFullImage, FALSE);
                }
                else
                {
                    pwork->InsertFullImageInfoDetail(NoteSerialInfoFullImage, TRUE);
                }
            }
        }
    }
    return TRUE;
}


//功能：验钞后保存拒钞数
//输入： iNumInRJSD：拒钞数
//输出：无
//返回：XFS通用错误码及CIM错误码
//说明：内部方法
void CUR2Adapter::SaveRejectBill(int iNumInRJSD)
{
    ZeroMemory(m_szRejectBill, sizeof(m_szRejectBill));
    memset(&m_szRejectBill[0], '0', (m_mapSupportNotID.size() + 1) * 3);
    sprintf(&m_szRejectBill[0] + m_mapSupportNotID.size() * 3, "%03d", iNumInRJSD);
    m_szRejectBill[(m_mapSupportNotID.size() + 1) * 3] = 0;
}

//功能：挖钞，回收后保存拒钞数
//输入：pNumRejectedPerDenoALL：每种面额的拒钞数
//输出：无
//返回：XFS通用错误码及CIM错误码
//说明：内部方法
void CUR2Adapter::SaveRejectBill(ULONG pNumRejRetPerDenoALL[ADP_MAX_DENOMINATION_NUM])
{
    const char *ThisModule = "SaveRejectBill";
    assert(pNumRejRetPerDenoALL != NULL);
    ZeroMemory(m_szRejectBill, sizeof(m_szRejectBill));
    memcpy(m_aryulRetractCountPerDeno, pNumRejRetPerDenoALL, sizeof(ULONG) * ADP_MAX_DENOMINATION_NUM);
    map<BYTE, WFSCIMNOTETYPE>::iterator it = m_mapSupportNotID.begin();
    int offset = 0;
    for (it; it != m_mapSupportNotID.end(); it++)
    {
        sprintf(&m_szRejectBill[0] + offset * 3, "%03d", pNumRejRetPerDenoALL[it->first - 1]);
        pNumRejRetPerDenoALL[it->first - 1] = 0;
        offset++;
    }

    for (int i = 0; i < ADP_MAX_DENOMINATION_NUM - 1; i++)
    {
        if (pNumRejRetPerDenoALL[i] > 0)
        {
            pNumRejRetPerDenoALL[ADP_MAX_DENOMINATION_NUM - 1] += pNumRejRetPerDenoALL[i];
        }
    }

    sprintf(&m_szRejectBill[0] + offset * 3, "%03d", pNumRejRetPerDenoALL[ADP_MAX_DENOMINATION_NUM - 1]);
    offset++;
    assert(offset < 16);
    m_szRejectBill[offset * 3] = 0;
}

//功能：每次挖钞或或回收后将每种面额的拒钞数或回收数累加保存
//输入：pNumRejectedPerDenoALL：单次动作产生的拒钞数或回收数
//
//输出：iRejBillPerDenAll：单笔交易或操作产生的拒钞数或回收数
//返回：XFS通用错误码
//说明：内部方法
void CUR2Adapter::SaveRejBillPerDenoALL(const ST_TOTAL_STACKE_NOTES_DENO_INFO &stNumRejRetPerDeno,
                                        ULONG pAllRejRetBillPerDen[ADP_MAX_DENOMINATION_NUM])
{
    assert(pAllRejRetBillPerDen != NULL);

    ST_STACKE_NOTES_DENO_INFO stNoteDeInfo;
    for (int j = 0; j < MAX_DENOMINATION_NUM; j++)
    {
        stNoteDeInfo = stNumRejRetPerDeno.stStackeNotesInfo[j];
        if (stNoteDeInfo.usNumberStack > 0)
        {
            pAllRejRetBillPerDen[stNoteDeInfo.ucDENOCode - 1] += stNoteDeInfo.usNumberStack;
        }
    }
}

//功能：打开钞门
//输入：无
//输出：无
//返回：XFS通用错误码及CDM错误码
//说明：内部函数
long CUR2Adapter::OpenDeviceShutter()
{

    const char *ThisModule = "OpenDeviceShutter";
    AutoLogFuncBeginEnd();
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)

    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    m_bCashOutBeforePresent = FALSE;
    int iRet = m_pUR2Dev->OpenShutter(pMaintenanceInfo);
    if (iRet == 0)
    {
        m_bOpenShutterJammed = FALSE;
    }
    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "开门");
    UpdateADPDevStatus(iRet);
    BOOL bOpenShutterJammed = m_pErrCodeTrans->IsOpenShutterJammed(m_szLastErrCode);
    if (iRet != 0)
    {
        Log(ThisModule, iRet, "打开设备钞门失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return bOpenShutterJammed ? WFS_ERR_CDM_SHUTTERNOTOPEN : WFS_ERR_HARDWARE_ERROR;
    }
    Log(ThisModule, iRet, "打开设备钞门成功(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
    return WFS_SUCCESS;
}

//功能：关闭钞门
//输入：FreeRetry 是否重试
//输出：无
//返回：XFS通用错误码及CDM错误码
//说明：内部函数
long CUR2Adapter::CloseDeviceShutter()
{
    int iRet = 0;
    const char *ThisModule = "CloseDeviceShutter";
    VERIFYPOINTNOTEMPTY(m_pUR2Dev)
    char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH] = {0};
    //30-00-00-00(FT#0053) add start
    //正常关门
    iRet = m_pUR2Dev->CloseShutter(pMaintenanceInfo, FALSE);
    if(iRet != ERR_UR_SUCCESS){
        for(int i = 0; i < m_sDevConfigInfo.dwCloseShutterRetryTimes; i++){
            iRet = m_pUR2Dev->CloseShutter(pMaintenanceInfo, FALSE);
            if(iRet == ERR_UR_SUCCESS){
                break;
            }

            CQtTime::Sleep(1000);
        }
    }

    //强制关门
    if(iRet != ERR_UR_SUCCESS && m_sDevConfigInfo.bCloseShutterForce){
        iRet = m_pUR2Dev->CloseShutter(pMaintenanceInfo, TRUE);
    }

    if(ERR_UR_SUCCESS == iRet){
        m_bCloseShutterJammed = FALSE;
    }
    //30-00-00-00(FT#0053) add end

    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, "关门");
    UpdateADPDevStatus(iRet);
    BOOL bCloseShutterJammed = m_pErrCodeTrans->IsCloseShutterJammed(m_szLastErrCode);
/*30-00-00-00(FT#0053) del start
    if (bCloseShutterJammed)
    {
        iRet = m_pUR2Dev->CloseShutter(pMaintenanceInfo, TRUE);
        if (iRet == 0)
        {
            m_bCloseShutterJammed = FALSE;
        }
        SaveLastErrCode(iRet, ThisModule);
        SaveLastActionErrCode(iRet, "重新尝试关门");
        UpdateADPDevStatus(iRet);
        bCloseShutterJammed = m_pErrCodeTrans->IsCloseShutterJammed(m_szLastErrCode);
    }
30-00-00-00(FT#0053) del start*/
    if (iRet != 0)
    {
        Log(ThisModule, iRet, "关闭设备钞门失败(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
        return bCloseShutterJammed ? WFS_ERR_CDM_SHUTTERNOTCLOSED : WFS_ERR_HARDWARE_ERROR;
    }
    Log(ThisModule, iRet, "关闭设备钞门成功(ErrCode:%s)(ReturnValue:%d)", m_szLastErrCode, iRet);
    return WFS_SUCCESS;
}

//更新动作类操作产生的错误码
void CUR2Adapter::SaveLastActionErrCode(int iRet, LPCSTR lpModule)
{
//    ZeroMemory(m_szLastErrCodeAction, sizeof(m_szLastErrCodeAction));
    strcpy(m_szLastErrCodeAction, NORMAL_ERRCODE);
    if (iRet < 0)
    {
        strcpy(m_szLastErrCodeAction, m_szLastErrCode);
    }
}

//功能：在可能产生有效错误码的机芯操作后获取错误码
//输入：iRet：机芯动作返回值
//输出：无
//返回：XFS通用错误码
//说明：内部函数
void CUR2Adapter::SaveLastErrCode(int iRet, LPCSTR lpModule, BOOL bForceUpdate/* = FALSE*/)
{
    char szLastErrCode[MAX_ERRCODE_LENGTH + 1];
    strcpy(szLastErrCode, m_szLastErrCode);
    //ZeroMemory(m_szLastErrCode, sizeof(m_szLastErrCode));

    ST_ERR_DETAIL stErrDetail;
    m_pUR2Dev->GetLastErrDetail(stErrDetail);

    if ((iRet == ERR_UR_WARN
        || iRet == ERR_UR_HARDWARE_ERROR) || bForceUpdate)
    {
        memcpy(m_szLastErrCode, stErrDetail.ucErrorCode, 8);
        //m_szLastErrCode[MAX_ERRCODE_LENGTH] = 0;
        m_szLastErrCode[8] = 0x00;
        m_usRecoveryCode = stErrDetail.usRecoveryCode;                           //恢复码
        memcpy(m_ucPostionCode, stErrDetail.ucPostionCode, sizeof(m_ucPostionCode));                         //错误位置
        if (strcmp(szLastErrCode, m_szLastErrCode) != 0 || strcmp(lpModule, "QueryDevInfo") != 0)
        {
            Log("SaveErrCode", iRet, "执行命令%s时出现警告或错误:%s", lpModule, m_szLastErrCode);
        }
    }
    else if (m_bIsNeedReset == FALSE)
    {
        strcpy(m_szLastErrCode, "00000000");
        if (strcmp(lpModule, "QueryDevInfo") != 0) {
            memcpy(m_ucPostionCode, stErrDetail.ucPostionCode, sizeof(m_ucPostionCode));
        }
    }
}

//功能：分析SP传入的回收位置，并回收存在的钞票
//输入：usRetractArea：回收目的位置
//      usRetractIndex: 钞箱序号
//      arywCUError: 钞箱错误原因
//输出：usRetractArea:
//返回：XFS通用错误码
//说明：内部函数
long CUR2Adapter::RetractNotesToDesArea(ADP_RETRACTEARE &usRetractArea,
                                        USHORT &usRetractIndex,
                                        ADP_CUERROR arywCUError[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "RetractNotesToDesArea";
    assert(arywCUError != NULL);
    USHORT usRetractDesArea = 0;
    USHORT usRetractDesIndex = 0;

    if (usRetractArea == ADP_RETRACTEARE_NOSET)
    {
        switch (m_sDevConfigInfo.usDefaultRetractArea)
        {
        case 1:
            usRetractDesArea = 3;
            break;
        case 3:
            if (!m_aryADPCassSettingInfo[MAX_CASSETTE_NUM - 1].bActive)
            {
                usRetractDesArea = 1;
                usRetractDesIndex = 4;
            }
            else
            {
                usRetractDesArea = 1;
                usRetractDesIndex = ADP_MAX_CU_SIZE;
            }
            break;
        case 2:
        default:
            usRetractDesArea = 1;
            usRetractDesIndex = 4;
            break;
        }
        //如果SP没有设置回收位置，则将适配层回收位置返回给SP
        usRetractArea = (ADP_RETRACTEARE)usRetractDesArea;
        usRetractIndex = usRetractDesIndex;
        return RetractNotes(usRetractDesArea, usRetractDesIndex, arywCUError);
    }
    else
    {
        usRetractDesArea = (USHORT)usRetractArea;
        return RetractNotes(usRetractDesArea, usRetractIndex, arywCUError);
    }
    return 0;
}

void CUR2Adapter::CassType2String(ADP_CASSETTE_TYPE iType, char szCassType[128])
{
    assert(szCassType != NULL);
    ZeroMemory(szCassType, sizeof(char) * 128);
    switch (iType)
    {
    case (ADP_CASSETTE_BILL):
        strcpy(szCassType, "ADP_CASSETTE_BILL");
        break;
    case (ADP_CASSETTE_CASHIN):
        strcpy(szCassType, "ADP_CASSETTE_CASHIN");
        break;
    case (ADP_CASSETTE_RECYCLING):
        strcpy(szCassType, "ADP_CASSETTE_RECYCLING");
        break;
    case (ADP_CASSETTE_RETRACT):
        strcpy(szCassType, "ADP_CASSETTE_RETRACT");
        break;
    case (ADP_CASSETTE_REJECT):
        strcpy(szCassType, "ADP_CASSETTE_REJECT");
        break;
    case (ADP_CASSETTE_UNKNOWN):
        strcpy(szCassType, "ADP_CASSETTE_UNKNOWN");
        break;
    default:
        break;
    }
}

void CUR2Adapter::CassStatus2String(CASHUNIT_STATUS stCassStatus, char szCassStatus[128])
{
    assert(szCassStatus != NULL);
    ZeroMemory(szCassStatus, sizeof(char) * 128);
    switch (stCassStatus)
    {
    case (ADP_CASHUNIT_OK):
        strcpy(szCassStatus, "ADP_CASHUNIT_OK");
        break;
    case (ADP_CASHUNIT_FULL):
        strcpy(szCassStatus, "ADP_CASHUNIT_FULL");
        break;
    case (ADP_CASHUNIT_HIGH):
        strcpy(szCassStatus, "ADP_CASHUNIT_HIGH");
        break;
    case (ADP_CASHUNIT_LOW):
        strcpy(szCassStatus, "ADP_CASHUNIT_LOW");
        break;
    case (ADP_CASHUNIT_EMPTY):
        strcpy(szCassStatus, "ADP_CASHUNIT_EMPTY");
        break;
    case (ADP_CASHUNIT_INOP):
        strcpy(szCassStatus, "ADP_CASHUNIT_INOP");
        break;
    case (ADP_CASHUNIT_MISSING):
        strcpy(szCassStatus, "ADP_CASHUNIT_MISSING");
        break;
    case (ADP_CASHUNIT_MANIP):
        strcpy(szCassStatus, "ADP_CASHUNIT_MANIP");
        break;
    case (ADP_CASHUNIT_UNKNOWN):
        strcpy(szCassStatus, "ADP_CASHUNIT_UNKNOWN");
        break;
    default:
        break;
    }
}

//功能:  判断在钞箱启用条件下，存取款是否可以进行
//输入:  bCDM:是否是CDM操作。
//       bCassEnable:钞箱启用条件。TRUE:该钞箱被启用; FALSE: 不被启用
//返回：TRUE:  存取款可以进行；FALSE: 存取款不可以进行
BOOL CUR2Adapter::IsOperatable(BOOL bCDM, const BOOL bCassEnable[ADP_MAX_CU_SIZE])
{
    const char *ThisModule = "IsOperatable";
    VERIFYPOINTNOTEMPTY(bCassEnable)
    //AB箱如果被禁止，则不能进行存取款
    if (!bCassEnable[3])
    {
        return FALSE;
    }
    else
    {
        return TRUE;
    }
}

long CUR2Adapter::GetRetractNoteIdAndCountPairs(char arycResult[256], ULONG &ulRejCount)
{
    assert(arycResult != NULL);
    ulRejCount = 0;
    memset(arycResult, '\0', sizeof(char) * 256);
    ULONG aryulRetractCountPerDeno[ADP_MAX_DENOMINATION_NUM] = {0};
    memcpy(aryulRetractCountPerDeno, m_aryulRetractCountPerDeno, sizeof(m_aryulRetractCountPerDeno));
    memset(m_aryulRetractCountPerDeno, 0, sizeof(m_aryulRetractCountPerDeno));
    map<BYTE, WFSCIMNOTETYPE>::iterator it = m_mapSupportNotID.begin();
    int ioffset(0);
    for (it; it != m_mapSupportNotID.end(); it++)//得到所有的可识别的钞票回收信息
    {
        if (aryulRetractCountPerDeno[it->first - 1] > 0)
        {
            sprintf(arycResult + ioffset * 12, "%06x%06d", it->second.usNoteID, aryulRetractCountPerDeno[it->first - 1]);
            ioffset++;
            aryulRetractCountPerDeno[it->first - 1] = 0;
        }
    }

    for (int i = 0; i < ADP_MAX_DENOMINATION_NUM; i++)
    {
        ulRejCount = ulRejCount + aryulRetractCountPerDeno[i];
    }
    if (ulRejCount > 0)
    {
        sprintf(arycResult + ioffset * 12, "%06x%06d", NOTE_ID_UNKNOWN, ulRejCount);
    }
    Log("GetRetractNoteIdAndCountPairs", 1, "arycResult:%s", arycResult);
    return 0;
}

void CUR2Adapter::SetLFSCmdId(ADP_LFS_CMD_ID eLfsCmdId)
{
    m_eLfsCmdId = eLfsCmdId;
    return;
}

//30-00-00-00(FS#0022) add start
long CUR2Adapter::SelfCount(bool  bEnableRoom[5][2],
                            ULONG ulNumSelfCountPerRoom[ADP_MAX_CU_SIZE],
                            ULONG ulNumRejectPerRoom[ADP_MAX_CU_SIZE],
                            bool  bErrCUInfo[ADP_MAX_CU_SIZE])
{
    LPCSTR ThisModule = "SelfCount";
    AutoLogFuncBeginEnd();

    CThreadGenerateSN SNWork(SAVESERIALOPERAT_SELFCOUNT, m_eLfsCmdId, ERR_UR_SUCCESS);
    long lRet = WFS_SUCCESS;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stRejectDenoDstInfoSum;
    ST_TOTAL_STACKE_NOTES_DENO_INFO stRejectDenoSrcInfoSum;
    ULONG ulNumFeedPerRoomSum[ADP_MAX_CU_SIZE] = {0, 0, 0, 0, 0, 0};
    m_sDevConfigInfo.stBVDependentMode = m_sDevConfigInfo.stSelfCountBVDependentMode;        //30-00-00-00(FS#0030)
    int iRet = ERR_UR_SUCCESS;

    BOOL bFirstExec = TRUE;
    do{
        ST_TOTAL_STACKE_NOTES_DENO_INFO stRejectDenoDstInfo;
        ST_TOTAL_STACKE_NOTES_DENO_INFO stRejectDenoSrcInfo;
        ULONG ulNumFeedPerRoom[ADP_MAX_CU_SIZE] = {0, 0, 0, 0, 0, 0};
        ULONG ulNumSelfCountPerRoomOnce[ADP_MAX_CU_SIZE] = {0, 0, 0, 0, 0, 0};
        iRet = m_pUR2Dev->SelfCount(m_sDevConfigInfo.CashOutNoteType, bEnableRoom, ulNumSelfCountPerRoomOnce,
                                    ulNumFeedPerRoom, stRejectDenoDstInfo, stRejectDenoSrcInfo, m_sDevConfigInfo.stBVDependentMode,
                                    iRet == ERR_UR_BV_IMG_FULL);
        //画像满或成功时取BV数据
        if(iRet == ERR_UR_BV_IMG_FULL || iRet == ERR_UR_SUCCESS){
            QueryNoteMediaInfo(&SNWork);
            QueryNoteSerialInfo(&SNWork);
        }

        //返当次信息者累加
        stRejectDenoDstInfoSum = stRejectDenoDstInfoSum + stRejectDenoDstInfo;
        stRejectDenoSrcInfoSum = stRejectDenoSrcInfoSum + stRejectDenoSrcInfo;
        for(int i = 0; i < ADP_MAX_CU_SIZE; i++){
            ulNumFeedPerRoomSum[i] += ulNumFeedPerRoom[i];
        }
        //返累加信息者赋值
        for(int i = 0; i < ADP_MAX_CU_SIZE; i++){
            ulNumSelfCountPerRoom[i] = ulNumSelfCountPerRoomOnce[i];
        }

        if(iRet != ERR_UR_BV_IMG_FULL){
            break;
        }

    } while(true);

    SNWork.SetResult(iRet);
    SaveLastErrCode(iRet, ThisModule);
    SaveLastActionErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);

    Log(ThisModule, __LINE__, "自动精查各钞箱计数结果:%d, %d, %d, %d, %d", ulNumSelfCountPerRoom[0], ulNumSelfCountPerRoom[1],
        ulNumSelfCountPerRoom[2], ulNumSelfCountPerRoom[3], ulNumSelfCountPerRoom[4]);
    Log(ThisModule, __LINE__, "自动精查各钞箱出钞张数:%d, %d, %d, %d, %d", ulNumFeedPerRoomSum[0], ulNumFeedPerRoomSum[1],
        ulNumFeedPerRoomSum[2], ulNumFeedPerRoomSum[3], ulNumFeedPerRoomSum[4]);
    //RB5为中转钞箱，出钞计数设置为0
    ulNumFeedPerRoomSum[4] = 0;

    //计算各钞箱拒钞张数
    for(int i = 0; i < stRejectDenoSrcInfoSum.ucCount; i++){
        ST_STACKE_NOTES_DENO_INFO denoSrcInfo = stRejectDenoSrcInfoSum.stStackeNotesInfo[i];
        if(denoSrcInfo.usNumberStack > 0){
            int iBoxNum = ((denoSrcInfo.iDest & 0xF0) >> 4);
//            int iRoomNum = (denoSrcInfo.iDest & 0x0F) - 0x0A;
            if(iBoxNum > 0){
                ulNumRejectPerRoom[iBoxNum - 1] += denoSrcInfo.usNumberStack;
            }
        }
    }

    Log(ThisModule, __LINE__, "自动精查各钞箱拒钞张数:%d, %d, %d, %d, %d", ulNumRejectPerRoom[0], ulNumRejectPerRoom[1],
        ulNumRejectPerRoom[2], ulNumRejectPerRoom[3], ulNumRejectPerRoom[4]);
    //保存拒钞金种详细计数信息
    ULONG ulRejBillPerDenAll[ADP_MAX_DENOMINATION_NUM] = {0};
    SaveRejBillPerDenoALL(stRejectDenoDstInfoSum, ulRejBillPerDenAll);
    SaveRejectBill(ulRejBillPerDenAll);



    if(iRet < 0){
        if(iRet == ERR_UR_RJ_BOX_FULL){
            return WFS_ERR_CIM_CASHUNITERROR;
        }
        return WFS_ERR_HARDWARE_ERROR;
    }

    return lRet;
}

string CUR2Adapter:: CassID2String(CASSETTE_ROOM_ID iCassID)
{
    switch (iCassID)
    {
    case ( ID_CS ):
        return "CS";
    case ( ID_ROOM_1 ):
        return "R1";
    case ( ID_ROOM_2 ):
        return "R2";
    case ( ID_ROOM_3 ):
        return "R3";
    case ( ID_ROOM_4 ):
        return "A1";
    case ( ID_ROOM_5 ):
        return "R4";
    default:
        return "UN";
    }
}

void CUR2Adapter::DeleteChar(char *src, char ch)
{
    char *p = src;
    while (*src != NULL)
    {
        if (*src != ch)
        {
            *p = *src;
            p++;
        }
        src++;
    }
    *p = '\0' ;
}

BOOL CUR2Adapter::IsNeedDownLoadFW()
{
    if (m_FWDStatus != FWDOWNSTATUS_UNSET)
    {
        return m_FWDStatus == FWDOWNSTATUS_NEEDDL;
    }

    if (InnerNeedDownLoad())
    {
        m_FWDStatus = FWDOWNSTATUS_NEEDDL;
        return TRUE;
    }

    m_FWDStatus = FWDOWNSTATUS_NOTNEEDDL;
    return FALSE;
}

BOOL CUR2Adapter::InnerNeedDownLoad()
{
    LPCSTR ThisModule = "IsNeedDownLoadFW";
    AutoLogFuncBeginEnd();

    if (m_mapURFWVerInfo.size() == 0)
    {
        Log(ThisModule, __LINE__, "机芯固件版本信息为空");
        return FALSE;
    }

    if(m_pCryptData == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pCryptData == nullptr");
        return 0;
    }

    m_mapProFilesInfo.clear();
    string strFWSavDir = m_szFWFilesPath;

    QDir QSaveDir(strFWSavDir.c_str());
    if (!QSaveDir.exists())
    {
        Log(ThisModule, 0, "固件文件保存路径错误:%s", strFWSavDir.c_str());
        return FALSE;
    }

    CINIFileReader FWFileINI;
    QFile QFile;
    char szConFile[MAX_PATH] = {0};
    char szSection[32] = {0};

    for (int i = 0; i < 2; i++)
    {
        LPCSTR szConFileName = (i == 1) ? "UR2CTL.INI" : "UR_BVZ20.INI";
        sprintf(szConFile, "%s/%s", strFWSavDir.c_str(), szConFileName);
        QFile.setFileName(szConFile);
        if (!QFile.exists())
        {
            Log(ThisModule, 0, "FW配置文件（%s）不存在", szConFile);
            continue;
        }
        if (!FWFileINI.LoadINIFile(szConFile))
        {
            Log(ThisModule, __LINE__, "FW配置文件（%s）加载失败", szConFile);
            continue;
        }
        CINIReader cINISetUp = FWFileINI.GetReaderSection("SETUP");
        DWORD dwFileCount = cINISetUp.GetValue("FILES", DWORD(0));
        for (DWORD j = 1; j <= dwFileCount; j++)
        {
            ProgramFileInfo pfi;
            sprintf(szSection, "PROG%02d", j);
            CINIReader cINI = FWFileINI.GetReaderSection(szSection);
            LPCSTR lpValue = cINI.GetValue("CTL_TYPE", "");
            strcpy(pfi.szFWType, lpValue);
            //获取固件模块版本
            lpValue = cINI.GetValue("CTL_VERSION", "");
            strcpy(pfi.szFWVersion, lpValue);
            //获取固件程序文件名
            lpValue = cINI.GetValue("CTL_FILENAME", "");
            sprintf(pfi.szFilePath, "%s/%s", strFWSavDir.c_str(), lpValue);
            QFile.setFileName(pfi.szFilePath);
            if (!QFile.exists())
            {
                Log(ThisModule, __LINE__, "固件升级文件（%s）不存在", pfi.szFilePath);
                return FALSE;
            }
            Log(ThisModule, __LINE__, "固件升级文件（%s)", pfi.szFilePath);
            lpValue = cINI.GetValue("MD5", "");
            QByteArray strMD5Value = lpValue;
            //CEncryptData cEncryData;
            //string strMD5 = cEncryData.GetMD5(pfi.szFilePath);
            QByteArray strMD5 = m_pCryptData->GetMD5(pfi.szFilePath);
            if(strMD5 != strMD5Value)
            {
                Log(ThisModule, __LINE__, "MD5校验失败FileMD5:%s, INIMD5:%s", strMD5.data(), strMD5Value.data());
                continue;
            }

            //获取固件模块块ID
            lpValue = cINI.GetValue("CTL_ID", "");
            pfi.ucCTLID = std::stoul(lpValue, nullptr, 16);
            //获取load address
            lpValue = cINI.GetValue("CTL_LOAD_ADDRESS", "");
            pfi.uLoadAdress = std::stoul(lpValue, nullptr, 16);
            //获取Check SUM
            lpValue = cINI.GetValue("CTL_CHECKSUM", "");
            pfi.uSUM = std::stoul(lpValue, nullptr, 16);
            //获取MaxSize
            lpValue = cINI.GetValue("CTL_MAX_PACKET_SIZE", "");
            pfi.uMaxSize = std::stoul(lpValue, nullptr, 16);

            m_mapProFilesInfo[pfi.ucCTLID] = pfi;
        }
    }
    if (m_mapProFilesInfo.size() <= 0)
    {
        Log(ThisModule, 0, "配置文件中更新文件数为0");
        return FALSE;
    }

    //版本比较，不需要下载则返回
    TYPEMAPFWUR::const_iterator itFW = m_mapURFWVerInfo.begin();
    TYPEMAPFWDATA::const_iterator itPF = m_mapProFilesInfo.begin();
    BOOL bNeedDownLoad = TRUE;
    for (; itPF != m_mapProFilesInfo.end(); itPF++)
    {
        for (; itFW != m_mapURFWVerInfo.end(); itFW++)
        {
            if ((itFW->second.usCTLID == itPF->second.ucCTLID) && (strcmp(itFW->second.sVersion, itPF->second.szFWVersion) == 0))
            {
                bNeedDownLoad = FALSE;
                break;
            }
        }
        if(bNeedDownLoad)
            break;
    }

    //if (itPF == m_mapProFilesInfo.end() && itFW == m_mapURFWVerInfo.end())
    if(!bNeedDownLoad)
    {
        Log(ThisModule, 0, "固件版本已是最新，不需要更新");
        return FALSE;
    }

    return TRUE;
}

//30-00-00-00(FS#0020)
bool CUR2Adapter::GetCUErrorInfoForDispense(ST_DISP_MISSFEED_ROOM &stDispMissfeedRoom, ADP_CUERROR arywCUError[])
{
    bool bCUError = false;
    //初始化输出结果
    for(int i = 0; i < ADP_MAX_CU_SIZE; i++){
        arywCUError[i] = ADP_CUERROR_OK;
    }

    for(int i = 0; i < ADP_MAX_CU_SIZE - 1; i++){
        if(stDispMissfeedRoom.bArryRoom[i]){
            if(stDispMissfeedRoom.bRoomContinuousRejects){
                arywCUError[i] = ADP_CUERROR_CONTINUEREJECT;
            } else if(stDispMissfeedRoom.bRoomEmpty){
                arywCUError[i] = ADP_CUERROR_EMPTY;
            } else if(stDispMissfeedRoom.bRoomMissFeed){
                arywCUError[i] = ADP_CUERROR_MISSFEED;
            } else if(stDispMissfeedRoom.bRoomTooManyRejects){
                arywCUError[i] = ADP_CUERROR_TOOMANYREJECT;
            } else {
                //TBD:
            }

            bCUError = true;
        }
    }

    return bCUError;
}

//30-00-00-00(FS#0030)
void CUR2Adapter::GetFullImgSNBVSettingInfo()
{
    CINIReader iniReader = m_ReaderConfigFile.GetReaderSection("SNInfo");

    //CashIn
    string strTemp = (LPCSTR)iniReader.GetValue("CashInBVFullImgSNSetting", "");
    SetActionFullImgSNBVSettingInfo(strTemp, m_sDevConfigInfo.stCashInBVDependentMode);

    //CashInEnd
    strTemp = (LPCSTR)iniReader.GetValue("CashInEndBVFullImgSNSetting", "");
    SetActionFullImgSNBVSettingInfo(strTemp, m_sDevConfigInfo.stCashInEndBVDependentMode);

    //Dispense
    strTemp = (LPCSTR)iniReader.GetValue("DispenseBVFullImgSNSetting", "");
    SetActionFullImgSNBVSettingInfo(strTemp, m_sDevConfigInfo.stDispenseBVDependentMode);

    //Retract
    strTemp = (LPCSTR)iniReader.GetValue("RetractBVFullImgSNSetting", "");
    SetActionFullImgSNBVSettingInfo(strTemp, m_sDevConfigInfo.stRetractBVDependentMode);

    //Reject
    strTemp = (LPCSTR)iniReader.GetValue("RejectBVFullImgSNSetting", "");
    SetActionFullImgSNBVSettingInfo(strTemp, m_sDevConfigInfo.stRejectBVDependentMode);

    //Selfcount
    strTemp = (LPCSTR)iniReader.GetValue("SelfCountBVFullImgSNSetting", "");
    SetActionFullImgSNBVSettingInfo(strTemp, m_sDevConfigInfo.stSelfCountBVDependentMode);

    return;
}

//30-00-00-00(FS#0030)
void CUR2Adapter::SetActionFullImgSNBVSettingInfo(string strSetting, ST_BV_DEPENDENT_MODE &stBVSettings)
{
    CAutoSplitByStep autoSplitByStep(strSetting.c_str(), ",");

    int iCount = autoSplitByStep.Count();
    if(iCount >= 3){
        stBVSettings.bEnableBVModeSetting = (atoi(autoSplitByStep.At(0)) == 1 ? TRUE : FALSE);
        stBVSettings.iFullImageMode = (RECORD_FULLIMAGE_MODE)atoi(autoSplitByStep.At(1));
        stBVSettings.iRejcetNoteNOSN = (atoi(autoSplitByStep.At(2)) == 1 ? ACTION_REJECT_NOTES : ACTION_NO_REJECT_NOTES);
    }

    return;
}

//30-00-00-00(FS#0022)
void CUR2Adapter::GetAcceptDispenseRoomsPriorityInfo()
{
    CINIReader iniReader = m_ReaderConfigFile.GetReaderSection("BRMInfo");

    CAutoSplitByStep autoSplitByStep;
    //存入
    string strTemp = (LPCSTR)iniReader.GetValue("AcceptRoomsPriority", "4,3,2,1,5,0,0,0,0,0");
    if(!strTemp.empty()){
        autoSplitByStep.SplitByStep(strTemp.c_str(), ",");
        if(autoSplitByStep.Count() >= 10){
            for(int i = 0; i < 10; i++){
                m_stHWConfig.byAcceptRoomsPriority[i] = atoi(autoSplitByStep.At(i));
                if(m_stHWConfig.byAcceptRoomsPriority[i] < 0 || m_stHWConfig.byAcceptRoomsPriority[i] > 10){
                    //值非法时，优先级设为最低
                    m_stHWConfig.byAcceptRoomsPriority[i] = 10;
                }
            }
        }
    }

    //取出
    strTemp = (LPCSTR)iniReader.GetValue("DispenseRoomsPriority", "2,3,4,5,1,0,0,0,0,0");
    if(!strTemp.empty()){
        autoSplitByStep.SplitByStep(strTemp.c_str(), ",");
        if(autoSplitByStep.Count() >= 10){
            for(int i = 0; i < 10; i++){
                m_stHWConfig.byDispenseRoomsPriority[i] = atoi(autoSplitByStep.At(i));
                if(m_stHWConfig.byDispenseRoomsPriority[i] < 0 || m_stHWConfig.byDispenseRoomsPriority[i] > 10){
                    //值非法时，优先级设为最低
                    m_stHWConfig.byDispenseRoomsPriority[i] = 10;
                }
            }
        }
    }

    return;
}

string CUR2Adapter::GetIOMCHWType()
{
    string strHWType = "HT";

    //从AtmStartConfig.ini中获取
    CINIFileReader cINI;
    string strIniFilePath = SPETCPATH;
    strIniFilePath += "/AtmStartConfig.ini";
    cINI.LoadINIFile(strIniFilePath.c_str());

    CINIReader cINIReader = cINI.GetReaderSection("DEVICE");
    string strPorParam = (LPCSTR)cINIReader.GetValue("PortParam", "");

    if(!strPorParam.empty()){
        strHWType = "CFES";
    }

    return strHWType;
}

bool CUR2Adapter::SetBlacklistInfo()
{
    if (m_sDevConfigInfo.strSNBlacklistFile.empty()) {
        return true;
    }

    QFile qCSVFile(QString::fromStdString(m_sDevConfigInfo.strSNBlacklistFile));
    if (!qCSVFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return true;
    }

    std::vector<BLACKLIST_INFO> infoList;
    QTextStream textStream(&qCSVFile);
    textStream.setCodec("UTF-8");
    while (!textStream.atEnd()) {
        QString line = textStream.readLine().trimmed();
        if (line.isEmpty()) {
            break;
        }
        if ((line.at(0) == '#') || (line.at(0) == ';')) {
            continue;
        }
        QStringList qslFields = line.split(",");
        if (qslFields.size() < 5) {
            continue;
        }
        BLACKLIST_INFO info;
        bool illegal = false;
        for (int i = 0; i < 5; i++) {
            const char* field = qslFields.at(i).trimmed().toStdString().c_str();
            if (i == 0) {
                if (strlen(field) != 3) {
                    illegal = true;
                    break;
                }
                strcpy(info.cCurrency, field);
            } else if (i == 1) {
                if ((strlen(field) == 0) || (strlen(field) > 4)) {
                    illegal = true;
                    break;
                }
                strcpy(info.cValue, field);
            } else if (i == 2) {
                if (strlen(field) != 1) {
                    illegal = true;
                    break;
                }
                info.cVersion = field[0];
            } else if (i == 3) {
                if (strlen(field) != 1) {
                    illegal = true;
                    break;
                }
                info.cAction = field[0];
            } else if (i == 4) {
                if ((strlen(field) == 0) || (strlen(field) > 16)) {
                    illegal = true;
                    break;
                }
                strcpy(info.cSerialNumber, field);
            }
        }
        if (illegal) {
            continue;
        }
        infoList.push_back(info);
    }
    qCSVFile.close();

    int iRet = m_pUR2Dev->SetBlackList(infoList);
    if (iRet != ERR_UR_SUCCESS) {
        return false;
    }

    return true;
}

//功能：删除目录下的文件，包括子文件夹
//输入：
//输出：无
//返回：成功删除返回true,否则返回false
//说明：内部方法
bool CUR2Adapter::RemoveDir(const char *pDirPath) const
{
    //查找目录下的文件,存在则删除
    QDir dir(pDirPath);
    dir.setFilter(QDir::Files);
    QStringList qFileList = dir.entryList();
    while (qFileList.size() > 0)
    {
        if (false == dir.remove(qFileList.back()))
            return false;
        qFileList.removeLast();
    }
    //查找目录下的子文件夹,存在则删除
    dir.setFilter(QDir::Dirs);
    QStringList qSubDirList = dir.entryList();
    if (qSubDirList.size() <= 2) //不存在子文件夹
        return true;
    qSubDirList.removeOne(".");
    qSubDirList.removeOne("..");
    while (qSubDirList.size() > 0)
    {
        QString strSubDirPath = QString("%1/%2").arg(pDirPath).arg(qSubDirList.back());
        if (!RemoveDir(qPrintable(strSubDirPath)))
            return false;
        dir.rmdir(strSubDirPath);
        qSubDirList.removeLast();
    }
    return true;
}

//test#7
int CUR2Adapter::GetResultFlg()
{
    const char *ThisModule = "GetResultFlg";
    AutoLogFuncBeginEnd();

    return iTooManeyItemsHappenFlg;
}

//test#7
void CUR2Adapter::SetResultFlg()
{
    const char *ThisModule = "SetResultFlg";
    AutoLogFuncBeginEnd();

    iTooManeyItemsHappenFlg = 0;;
}


long CUR2Adapter::DownLoadFW()
{
    const char *ThisModule = "DownLoadFWData";
    AutoLogFuncBeginEnd();

    const TYPEMAPFWDATA MapFWData = m_mapProFilesInfo;
    if (MapFWData.size() <= 0)
    {
        Log(ThisModule, 0, "配置文件中更新文件数为0，无需升级");
        return 0;
    }
    if(m_pCryptData == nullptr)
    {
        Log(ThisModule, __LINE__, "m_pCryptData == nullptr");
        return 0;
    }
    Log(ThisModule, 1, "开始下载固件");
    m_FWDStatus = FWDOWNSTATUS_DLING;
    //开始下载固件程序
    long lRes = m_pUR2Dev->ProgramDownloadStart();
    if (lRes < 0)
    {
        m_FWDStatus = FWDOWNSTATUS_UNSET;
        return WFS_ERR_HARDWARE_ERROR;
    }

    TYPEMAPFWDATA::const_iterator it;
    size_t i;
    for (i = 1, it = MapFWData.begin(); it != MapFWData.end() && i <= MapFWData.size(); it++, i++)
    {
        FILE *fp = fopen(it->second.szFilePath, "rb");
        if (fp == nullptr)
        {
            Log(ThisModule, __LINE__, "读取下载文件(%s)失败", it->second.szFilePath);
            m_pUR2Dev->ProgramDownloadEnd();
            m_FWDStatus = FWDOWNSTATUS_UNSET;
            return WFS_ERR_INVALID_DATA;
        }

        fseek(fp, 0L, SEEK_END);
        long nLen = ftell(fp);
        if (nLen == 0)
        {
            Log(ThisModule, __LINE__, "固件文件(%s)为空文件", it->second.szFilePath);
            m_pUR2Dev->ProgramDownloadEnd();
            m_FWDStatus = FWDOWNSTATUS_UNSET;
            return WFS_ERR_INVALID_DATA;
        }

        fseek(fp, 0, SEEK_SET);
        BYTE szDataBuff[10240] = {0};
        BOOL bLastData = FALSE;

        size_t iLen = fread(szDataBuff, sizeof(BYTE), 16, fp);
        BYTE   uOffset = szDataBuff[0];

        UINT lDownLoadedLen = 0;
        PDL_BLOCK_TYPE PDLBlockType = PDL_FIRST_BLOCK;

        //ULONG ulMaxSize = 1024;
        while (!bLastData)
        {
            size_t iReadLen = fread(szDataBuff, sizeof(BYTE), it->second.uMaxSize, fp);
            if (ferror(fp))
            {
                Log(ThisModule, __LINE__, "固件下载过程中读取文件(%s)失败,MaxSize%d", it->second.szFilePath, it->second.uMaxSize);
                m_pUR2Dev->ProgramDownloadEnd();
                m_FWDStatus = FWDOWNSTATUS_UNSET;
                return WFS_ERR_INVALID_DATA;
            }
            if (feof(fp))  // 已到文件末尾
            {
                bLastData = TRUE;
            }

            if (lDownLoadedLen == 0)
            {
                PDLBlockType = PDL_FIRST_BLOCK;
            }
            else if (bLastData)
            {
                PDLBlockType = PDL_LAST_BLOCK;
            }
            else
            {
                PDLBlockType = PDL_MIDDLE_BLOCK;
            }

            QByteArray vtHexIn;
            QByteArray vtHexOut;
            vtHexIn.append((char*)szDataBuff, iReadLen);
            {
                //cEncryData.DecryptData(vtHexIn, vtHexOut);
                m_pCryptData->DecryptData(vtHexIn, vtHexOut);
                int iLen = vtHexOut.size();

                if(bLastData)
                {
                    QByteArray vtHexTemp = vtHexOut;
                    vtHexOut.clear();
                    if(iLen >= uOffset)
                    {
                        vtHexOut = vtHexTemp.left(iLen - uOffset);
                    }
                }
            }
            lRes = m_pUR2Dev->ProgramDownloadSendData(it->second.ucCTLID, PDLBlockType,
                                                      it->second.uLoadAdress, lDownLoadedLen, vtHexOut.constData(),
                                                      static_cast<USHORT>(vtHexOut.size()));
            if (lRes < 0)
            {
                fclose(fp);
                m_pUR2Dev->ProgramDownloadEnd();
                m_FWDStatus = FWDOWNSTATUS_UNSET;
                return WFS_ERR_HARDWARE_ERROR;
            }

            lDownLoadedLen += iReadLen;
        }

        fclose(fp);
    }

    lRes = m_pUR2Dev->ProgramDownloadEnd();
    if (lRes < 0)
    {
        m_FWDStatus = FWDOWNSTATUS_UNSET;
        return WFS_ERR_HARDWARE_ERROR;
    }

    Log(ThisModule, lRes != 0 ? __LINE__ : 1, "下载固件%s", lRes != 0 ? "失败" : "成功");

    //m_pUR2Dev->Reboot();
    CQtTime::Sleep(60 * 1000);

    lRes = CloseUSBConnect(CONNECT_TYPE_UR_ZERO);
    if (lRes < 0)
    {
        Log(ThisModule, __LINE__, "下载固件后关闭USB连接失败");
        // return WFS_ERR_HARDWARE_ERROR;
    }

    lRes = Open();
    if (lRes < 0)
    {
        Log(ThisModule, __LINE__, "下载固件后重新打开USB连接失败");
        // return WFS_ERR_HARDWARE_ERROR;
    }
    m_FWDStatus = FWDOWNSTATUS_UNSET;
    return lRes;
}

void CUR2Adapter::SetDownLoadFWFlag(BOOL bFlag)
{
    //m_bDownloadFW = bFlag;
}


long CUR2Adapter::SetUnfitnoteVerificationLevel(EnumSaveSerialOperation enCmd)
{
    const char *ThisModule = "SetUnfitnoteVerificationLevel";

    eCMDType CmdType;
    BOOL bSevereMode = FALSE;
    switch (enCmd)
    {
    case SAVESERIALOPERAT_D:
        CmdType = eCMDType_Dispense;
        break;
    case SAVESERIALOPERAT_C:
        CmdType = eCMDType_CashCount;
        break;
    case SAVESERIALOPERAT_S:
        CmdType = eCMDType_StoreMoney;
        break;
    default:
        return 1;
    }

    if (m_sDevConfigInfo.usUnfitNotesVerifyMode == 1)
    {
        bSevereMode = FALSE;
    }
    else if (m_sDevConfigInfo.usUnfitNotesVerifyMode == 2)
    {
        bSevereMode = TRUE;
    }
    else
    {
        return 1;
    }

    int iRet =  m_pUR2Dev->SetUnfitNoteVerifyLevel(CmdType, bSevereMode);
    if (iRet != 0)
    {
        Log(ThisModule, iRet, "SetUnfitNoteVerifyLevel  failed");
    }

    return iRet;

}

int  CUR2Adapter::SetRejectCounterfeitNote()
{
    const char *ThisModule = "SetRejectCounterfeitNote";

    int iRet  = 0;
    if (m_sDevConfigInfo.bRejectCounterfeitNote)
    {
        iRet =  m_pUR2Dev->SetRejectCounterfeitNote();
        if (iRet != 0)
        {
            Log(ThisModule, iRet, "SetRejectCounterfeitNote  failed");
        }
    }
    return iRet;
}

void CUR2Adapter::InitBVSupportNotesDeno(ST_DENO_INFO pArryDenoInfo[MAX_DENOMINATION_NUM])
{
    const char* ThisModule = "InitBVSupportNotesDeno";
    m_mapBVSupportNoteID.clear();
    WFSCIMNOTETYPE BVSupportNoteInfo;
    for (int i = 0; i < MAX_DENOMINATION_NUM; i++)
    {
        if (pArryDenoInfo[i].iCurrencyCode == CURRENCY_CODE_CNY)
        {
            BVSupportNoteInfo.usNoteID = 0;
            BVSupportNoteInfo.bConfigured = TRUE;
            memcpy(BVSupportNoteInfo.cCurrencyID, "CNY", 3);
            BVSupportNoteInfo.ulValues = pArryDenoInfo[i].iCashValue;
            BVSupportNoteInfo.usRelease = pArryDenoInfo[i].ucVersion - 'A' + 4;  //
            m_mapBVSupportNoteID[i + 1] = BVSupportNoteInfo;
        }
    }
    memcpy(BVSupportNoteInfo.cCurrencyID, "   ", 3);
    BVSupportNoteInfo.bConfigured = TRUE;
    BVSupportNoteInfo.ulValues = 0;
    BVSupportNoteInfo.usRelease = 0;
    BVSupportNoteInfo.usNoteID = NOTE_ID_UNKNOWN;
    m_mapBVSupportNoteID[0x80] = BVSupportNoteInfo;

    //支持币种信息写入ini文件
    WriteNoteVersionInfoToIni();                    //30-00-00-00(FT#0002)

    //LogWrite(ThisModule, 1, "BV支持的钞票列表数为%d",m_mapBVSupportNoteID.size());
}

//30-00-00-00(FT#0002) add start
void CUR2Adapter::WriteNoteVersionInfoToIni()
{
    string strIniFilePath = string(SPETCPATH) + "/" + "BRMSPConfig.ini";
    CINIFileReader cIniFileReader(strIniFilePath.c_str());
    cIniFileReader.SetNewSection("BV BankNote Table");
    CINIWriter cIniWriter = cIniFileReader.GetWriterSection("BV BankNote Table");
    for(auto it : m_mapBVSupportNoteID){
        char szKeyName[6] = {0};
        char szCurrencyId[4] = {0};
        char szKeyValue[MAX_PATH] = {0};
        sprintf(szKeyName, "ID%d", it.first);
        memcpy(szCurrencyId, it.second.cCurrencyID, 3);
        sprintf(szKeyValue, "%s %04lu %d", szCurrencyId,
                it.second.ulValues, it.second.usRelease);
        cIniWriter.AddValue(szKeyName, szKeyValue);
    }
}
//30-00-00-00(FT#0002) add end

void CUR2Adapter::GenerateDenoArray(BYTE pArryDENOCode[MAX_DENOMINATION_NUM])
{
    for (BYTE i = 0; i < MAX_DENOMINATION_NUM; i++)
    {
        map<BYTE, WFSCIMNOTETYPE>::iterator it = m_mapBVSupportNoteID.find(i+1);
/* 30-00-00-00(FS#0018) del start
        if(it != m_mapBVSupportNoteID.end())
        {
            pArryDENOCode[i] = (DENOMINATION_CODE)(i+1);
            if((it->second.ulValues == 100) &&
               (it->second.usRelease == 7))
            {
                pArryDENOCode[i] = DENOMINATION_CODE_100_C;
            }
        }
        else
        {
            continue;
        }
30-00-00-00(FS#0018) del end*/
        //30-00-00-00(FS#0018) add start
        if(it != m_mapBVSupportNoteID.end()) {
            map<BYTE, BYTE>::iterator it2 = m_sDevConfigInfo.mapNoteIDGroups.find(i+1);
            if (it2 != m_sDevConfigInfo.mapNoteIDGroups.end()) {
                pArryDENOCode[i] = it2->second;
            } else {
                pArryDENOCode[i] = (DENOMINATION_CODE)(i+1);
                if((it->second.ulValues == 100) &&
                        (it->second.usRelease == 7))
                {
                    pArryDENOCode[i] = DENOMINATION_CODE_100_C;
                }
            }
        }
        //30-00-00-00(FS#0018) add end
    }
}

long CUR2Adapter::GetBanknoteInfoAndSetDenoCode()
{
    LPCSTR ThisModule = "GetBanknoteInfoAndSetDenoCode";
    ST_BV_INFO pBVInfo;
    ST_DENO_INFO pArryDenoInfo[MAX_DENOMINATION_NUM];
    int	iRet = m_pUR2Dev->GetBanknoteInfo(pBVInfo, pArryDenoInfo);
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)
    {
        Log(ThisModule,  iRet, "GetBanknoteInfo Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }
    InitBVSupportNotesDeno(pArryDenoInfo);
    //////
    char pArryDENOCode[MAX_DENOMINATION_NUM] = {0};
    //GenerateDenominationToUR (pArryDenoInfo, pArryDENOCode);
    GenerateDenoArray((BYTE*)pArryDENOCode);
    iRet = m_pUR2Dev->SetDenominationCode(pArryDENOCode) ;
    SaveLastErrCode(iRet, ThisModule);
    UpdateADPDevStatus(iRet);
    if (iRet < 0)
    {
        Log(ThisModule,  iRet, "SetDenominationCode Failed");
        return WFS_ERR_HARDWARE_ERROR;
    }
    return WFS_SUCCESS;
}


long CUR2Adapter::SetVerificationLevel(EnumSaveSerialOperation enCmd)
{
    const char *ThisModule = "SetVerificationLevel";

    int iValue = 0;
    switch (enCmd)
    {
    case SAVESERIALOPERAT_D:
        {
            iValue = m_sDevConfigInfo.dwVerLevDispense;
            break;
        }
    case SAVESERIALOPERAT_C:
        {
            iValue = m_sDevConfigInfo.dwVerLevCashCount;
            break;
        }
    case SAVESERIALOPERAT_S:
        {
            iValue = m_sDevConfigInfo.dwVerLevStoreCash;
            break;
        }
    default:
        return 1;
    }

    if (iValue < 30 || iValue > 255)
    {
        return 1;
    }

    int iRet =  m_pUR2Dev->SetBVVerificationLevel(BYTE(iValue));
    if (iRet != 0)
    {
        Log(ThisModule, iRet, "SetBVVerificationLevel  failed");
    }

    return iRet;

}

long CUR2Adapter::UpdatePostionStatus()
{
    return QueryDevInfo();
}

void  CUR2Adapter::GetDevConfig(ADPDEVINFOR &devinfo)
{
    devinfo.bHaveInShutter = TRUE;
    devinfo.bHaveOutShutter = TRUE;
}
