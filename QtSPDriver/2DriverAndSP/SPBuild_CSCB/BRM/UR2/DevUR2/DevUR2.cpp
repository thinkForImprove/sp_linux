#include "DevUR2.h"
#include <assert.h>
static const char *ThisFile = "UR2_Drv";

//超时定义
#define HT_SEND_TIMEOUT_1     1
#define HT_SEND_TIMEOUT_5     5
#define HT_RESP_TIMEOUT_10    10
#define HT_RESP_TIMEOUT_20    20
#define HT_RESP_TIMEOUT_60    60
#define HT_RESP_TIMEOUT_90    90
#define HT_RESP_TIMEOUT_120   120
#define HT_RESP_TIMEOUT_180   180
#define HT_RESP_TIMEOUT_210   210
#define HT_RESP_TIMEOUT_240   240
#define HT_RESP_TIMEOUT_300   300
#define HT_RESP_TIMEOUT_1200  1200

//最小响应包数据长度 LENGTH + RESP + COMMOM BLOCK 2 + 6 + 34 + 28
#define MIN_RESP_PACKET_LEN 70

//发送指令长度定义
#define HT_CMD_LEN_ZERO       36
#define HT_CMD_LEN_ACTION     8
#define HT_CMD_LEN_QUERYINFO  8
#define HT_CMD_LEN_MEDIAINFO  40    //GetMediaInformation()发送长度

//返回数据长度定义
#define HT_RESP_LEN_COMMON    70
#define HT_RESP_LEN_ZERO      0xA000
#define HT_RESP_LEN_MEDIAINFO 1010  //GetMediaInformation()响应长度
#define HT_RESP_LEN_MAX       14492 //响应数据最大接收长度 7246 * 2

//数据包ID定义
#define PACKET_ID_CMND                  0x0001  //LEN:0x0004
#define PACKET_ID_RESP                  0x0081  //LEN:0x0004
#define PACKET_ID_COMMONBLOCK           0x008E  //LEN:0x0020

#define PACKET_ID_OPINFO                0x050E  //LEN:0x0008  0x0511->0x050E UR2不使用
#define PACKET_ID_HWINFO                0x0512  //LEN:0x0008
#define PACKET_ID_DENOCODESET           0x0513  //LEN:0x0102
#define PACKET_ID_CASSDENOCODE          0x0515  //LEN:0x0144
#define PACKET_ID_ACCEPTDENOCODE        0x0517  //LEN:0x0014
#define PACKET_ID_CASSTYPE              0x0518  //LEN:0x0022
#define PACKET_ID_VERLEVELCASHCOUNT     0x051D  //LEN:0x0014
#define PACKET_ID_VERLEVELSTORE         0x051E  //LEN:0x0014
#define PACKET_ID_VERLEVELDISP          0x051F  //LEN:0x0014 
#define PACKET_ID_NOTEHANDINFO          0x0521  //LEN:0x0024
#define PACKET_ID_USERMEMORYDATA        0x0561  //LEN:0x0084

#define PACKET_ID_STATUSINFO            0x0581  //LEN:0x001A
#define PACKET_ID_SPECIFICFUNC          0x0582  //LEN:0x0004
#define PACKET_ID_MAINTENINFO           0x0585  //LEN:0x1002
#define PACKET_ID_NUMTOTALSTACKEDNOTES  0x0586  //LEN:0x003E

#define PACKET_ID_NUMSTACKEDNOTES       0x058A  //LEN:0x003E
#define PACKET_ID_NUMFEDNOTES           0x058B  //LEN:0x0022
#define PACKET_ID_NUMREJNOTES           0x058C  //LEN:0x0004
#define PACKET_ID_BANKNOTETABLE         0x058D  //LEN:0x07F2
#define PACKET_ID_CASSCONFIG            0x058E  //LEN:0x00FC
#define PACKET_ID_VALIDATORID           0x058F  //LEN:0x0012

#define PACKET_ID_FW_UR2BOOT            0x0591  //LEN:0x001E
#define PACKET_ID_FW_FPGA               0x0592  //LEN:0x001E
#define PACKET_ID_FW_UR2MAIN            0x0593  //LEN:0x001E
#define PACKET_ID_FW_AUTHENTICAT        0x0594  //LEN:0x001E
#define PACKET_ID_FW_BV1                0x0595  //LEN:0x001E
#define PACKET_ID_FW_BV2                0x0596  //LEN:0x001E
#define PACKET_ID_FW_BV3                0x0597  //LEN:0x001E
#define PACKET_ID_FW_BV4                0x0598  //LEN:0x001E
#define PACKET_ID_FW_VT                 0x059C  //LEN:0x0012

#define PACKET_ID_UNFITNUMDENO_16       0x05A8  //LEN:0x0042
#define PACKET_ID_UNFITNUMDENO_32       0x05A9  //LEN:0x0082
#define PACKET_ID_UNFITNUMDENO_64       0x05AA  //LEN:0x0102
#define PACKET_ID_UNFITNUMDENO_128      0x05AB  //LEN:0x0202

#define PACKET_ID_BVWARNINFO            0x05BC  //LEN:0x007A
#define PACKET_ID_NUMDISPNOTESROOM      0x05C0  //LEN:0x001A
#define PACKET_ID_REJECTNUMDENOSOURCE   0x05C2  //LEN:0x0042

#define PACKET_ID_REJECTNUMDENOBV_16    0x05C6  //LEN:0x0042
#define PACKET_ID_REJECTNUMDENOBV_32    0x05C7  //LEN:0x0082
#define PACKET_ID_REJECTNUMDENOBV_64    0x05C8  //LEN:0x0102
#define PACKET_ID_REJECTNUMDENOBV_128   0x05C9  //LEN:0x0202

#define PACKET_ID_REJECTNUMDENO_16      0x05CA  //LEN:0x0042
#define PACKET_ID_REJECTNUMDENO_32      0x05CB  //LEN:0x0082
#define PACKET_ID_REJECTNUMDENO_64      0x05CC  //LEN:0x0102
#define PACKET_ID_REJECTNUMDENO_128     0x05CD  //LEN:0x0202

#define PACKET_ID_STACKENUMDENO_16      0x05D0  //LEN:0x0042
#define PACKET_ID_STACKENUMDENO_32      0x05D1  //LEN:0x0082
#define PACKET_ID_STACKENUMDENO_64      0x05D2  //LEN:0x0102
#define PACKET_ID_STACKENUMDENO_128     0x05D3  //LEN:0x0202

#define PACKET_ID_MISSFEEDROOM          0x05DA  //LEN:0x0004
#define PACKET_ID_FEDNOTECONDITION      0x05DB  //LEN:0x0004
#define PACKET_ID_SECURITYCASS          0x05DC  //LEN:0x0004

#define PACKET_ID_SENSORINFO            0x05E4  //LEN:0x000A
#define PACKET_ID_TESTRESULT1           0x05E5  //LEN:0x0102
#define PACKET_ID_TESTRESULT2           0x05E6  //LEN:0x0402

#define PACKET_ID_LOG_SENSOR            0x05F2  //LEN:0x1C04
#define PACKET_ID_LOG_INTERNALCOMMAND   0x05F3  //LEN:0x1C04
#define PACKET_ID_LOG_NOTESHAND         0x05F4  //LEN:0x1C04
#define PACKET_ID_LOG_MOTORCONTR        0x05F5  //LEN:0x1C04
#define PAKCET_ID_LOG_DATAOTHERS        0x05F6  //LEN:0x7174
#define PAKCET_ID_LOG_SESORLEVEL        0x05F7  //LEN:0x0872
#define PAKCET_ID_LOG_NOTESIDENTIF      0x05F8  //LEN:0x1812

//将buff的第iBegin开始的连续4个字节的数据转换为unsigned long数据
#define  Count4BytesToULValue(buff, iBegin) \
    (static_cast<unsigned long>(buff[iBegin + 0] << 24)) + \
    (static_cast<unsigned long>(buff[iBegin + 1] << 16)) + \
    (static_cast<unsigned long>(buff[iBegin + 2] << 8)) + \
    (static_cast<unsigned long>(buff[iBegin + 3]));

//指针类参数校验，如果为空则返回

#define VERIFYPOINTNOTEMPTY(point)\
    {\
        if(point == nullptr)\
        {\
            Log(ThisModule, ERR_UR_PARAM, "参数%s为NULL", #point);\
            return 0;\
        }\
    }\

///*//Log(ThisModule, ERR_UR_PARAM, "参数%s为NULL", #point);\
//Log(ThisModule, -1, "打开UR连接失败");\*/
///
//拼接命令为CMND包
#define MOSAICCMND(szCMND, iCmdLen, iCommand)\
    {\
        {\
            (szCMND[iCmdLen + 0] = 0x00);\
            (szCMND[iCmdLen + 1] = 0x01);\
            (szCMND[iCmdLen + 2] = 0x00);\
            (szCMND[iCmdLen + 3] = 0x04);\
            (szCMND[iCmdLen + 4] = static_cast<char>((iCommand >> 8) & 0xFF));\
            (szCMND[iCmdLen + 5] = static_cast<char>(iCommand & 0xFF));\
        }\
    }

//分析响应数据的16字节状态信息
//arycStatus：响应数据中的16字节状态
//stAryCassStatus：返回钞箱状态
//stDevStatusInfo：返回设备状态
void ParseRespStatusInfo(const char arycStatus[16],
                         CASSETTE_STATUS stAryCassStatus[MAX_CASSETTE_NUM],
                         BOOL bCassInPos[MAX_CASSETTE_NUM],
                         ST_DEV_STATUS_INFO &stDevStatusInfo)
{
    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        bCassInPos[i] = (arycStatus[3 + i] & 0x01) ? true : false;

        if ((arycStatus[3 + i] & 0x80) || (arycStatus[3 + i] & 0x08))
        {
            stAryCassStatus[i] = CASSETTE_STATUS_FULL;
        }
        else if (arycStatus[3 + i] & 0x10)
        {
            stAryCassStatus[i] = CASSETTE_STATUS_EMPTY;
        }
        else if (arycStatus[3 + i] & 0x40)
        {
            stAryCassStatus[i] = CASSETTE_STATUS_NEAREST_EMPTY;
        }
        else
        {
            stAryCassStatus[i] = CASSETTE_STATUS_NORMAL;
        }
    }
    //1
    stDevStatusInfo.bCashAtShutter      = (arycStatus[0]  & 0x80) ? true : false;
    stDevStatusInfo.bCashAtCS            = (arycStatus[0]  & 0x40) ? true : false;
    stDevStatusInfo.bCashAtCSErrPos   = (arycStatus[0]  & 0x20) ? true : false;
    //2
    stDevStatusInfo.bESCFull               = (arycStatus[1]  & 0x80) ? true : false;
    stDevStatusInfo.bESCEmpty           = (arycStatus[1]  & 0x40) ? true : false;
    stDevStatusInfo.bURJBFull             = (arycStatus[1]  & 0x20) ? true : false;
    stDevStatusInfo.bURJBEmpty          = (arycStatus[1]  & 0x10) ? true : false;
    //3
    stDevStatusInfo.bNotesRJInCashCout = (arycStatus[2]  & 0x80) ? true : false;
    //4
    //5
    stDevStatusInfo.bHCMUPInPos        = (arycStatus[4]  & 0x80) ? true : false;
    stDevStatusInfo.bHCMLOWInPos       = (arycStatus[4]  & 0x40) ? true : false;
    stDevStatusInfo.bURJBOpen          = (arycStatus[4]  & 0x08) ? false : true;
    //6
    bCassInPos[0] = (arycStatus[5] & 0x80) ? true : false;
    bCassInPos[1] = (arycStatus[5] & 0x40) ? true : false;
    bCassInPos[2] = (arycStatus[5] & 0x20) ? true : false;
    bCassInPos[3] = (arycStatus[5] & 0x10) ? true : false;
    bCassInPos[4] = (arycStatus[5] & 0x08) ? true : false;

    for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        bCassInPos[i] = (arycStatus[5] & 0x01) ? true : false;
    }
    stDevStatusInfo.bRearDoorOpen      = (arycStatus[5]  & 0x04) ? false : true;
    stDevStatusInfo.bFrontDoorOpen     = (arycStatus[5]  & 0x02) ? false : true;
    //7
    stDevStatusInfo.bESCOpen      = (arycStatus[6]  & 0x80) ? false : true;
    stDevStatusInfo.bESCInPos      = (arycStatus[6]  & 0x40) ? true : false;
    stDevStatusInfo.bESCRearEnd      = (arycStatus[6]  & 0x20) ? true : false;
    stDevStatusInfo.bCSInPos      = (arycStatus[6]  & 0x10) ? true : false;
    stDevStatusInfo.bBVFanErr      = (arycStatus[6]  & 0x02) ? true : false;
    stDevStatusInfo.bBVOpen      = (arycStatus[6]  & 0x01) ? false : true;
    //8
    stDevStatusInfo.iOutShutterStatus      = (arycStatus[7]  & 0x01) ? UR2SHUTTER_STATUS_CLOSED : UR2SHUTTER_STATUS_OPEN;
    //9 - 13
    for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        if (arycStatus[8 + i] & 0x10)
        {
            stAryCassStatus[i] = CASSETTE_STATUS_EMPTY;
        }
        else if (arycStatus[8 + i] & 0x30)
        {
            stAryCassStatus[i] = CASSETTE_STATUS_NEAREST_EMPTY;
        }
        else if (arycStatus[8 + i] & 0x40)
        {
            stAryCassStatus[i] = CASSETTE_STATUS_NEAR_EMPTY;
        }
        else if (arycStatus[8 + i] & 0x50)
        {
            stAryCassStatus[i] = CASSETTE_STATUS_NEAR_FULL;
        }
        else if (arycStatus[8 + i] & 0x80)
        {
            stAryCassStatus[i] = CASSETTE_STATUS_FULL;
        }
        else
        {
            stAryCassStatus[i] = CASSETTE_STATUS_NORMAL;
        }
    }
    //14
    stDevStatusInfo.bForcedOpenShutter      = (arycStatus[13]  & 0x80) ? true : false;
    stDevStatusInfo.bForcedRemovCashInCS = (arycStatus[13]  & 0x40) ? true : false;
    stDevStatusInfo.bCashLeftInCS               = (arycStatus[13]  & 0x20) ? true : false;
    stDevStatusInfo.bCashExistInESC            = (arycStatus[13]  & 0x10) ? true : false;
    stDevStatusInfo.bReqReadStatus            = (arycStatus[13]  & 0x08) ? true : false;
    stDevStatusInfo.bReqGetOPLog               = (arycStatus[13]  & 0x04) ? true : false;
    stDevStatusInfo.bReqReset                     = (arycStatus[13]  & 0x02) ? true : false;
    //15
    //16
    stDevStatusInfo.bBVWarning                    = (arycStatus[15]  & 0x80) ? true : false;
    //17
    stDevStatusInfo.bDuringEnergy                 = (arycStatus[16]  & 0x20) ? true : false;
}

CUR2Drv::CUR2Drv()
{
    SetLogFile(LOGFILE, ThisFile, "UR2");
    memset(&m_stErrDetail, 0, sizeof(m_stErrDetail));
    memset(&m_stDevStatusInfo, 0, sizeof(m_stDevStatusInfo));
    m_strCashInShutterPort = "COM:/dev/ttyUSB0:57600,N,8,1";

    InitMapPacketID();

    m_pUSBDrv = new CVHUSBDrive("UR2");

    //pthread_mutex_init(&m_mutex_status, NULL);
}

CUR2Drv::~CUR2Drv()
{
    //  CloseUSBConn(CONNECT_TYPE_UR_ZERO);
    //  DeleteCriticalSection(&m_CritSectStatus);
    //  DeleteCriticalSection(&m_CritSectErrDetail);
    CloseURConn();
    CloseBVConn();
    if (m_pUSBDrv != nullptr)
    {
        delete m_pUSBDrv;
        m_pUSBDrv = nullptr;
    }
}

//功能：释放本接口
void CUR2Drv::Release()
{
    //delete this;
}

int CUR2Drv::OpenURConn()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    bool bConnect = m_pUSBDrv->OpenVHUSBConnect(CONNECT_TYPE_UR);
    if (!bConnect)
    {
        Log(ThisModule, -1, "打开UR连接失败");
        return ERR_UR_USB_CONN_ERR;
    }
    Log(ThisModule, 1, "打开UR连接成功");
    return ERR_UR_SUCCESS;
}

//功能：打开BV连接
//输入：eConnectType, 连接类型
//输出：
//返回：0: 成功，非0: 失败
//说明：打开ZVB和UR的USB连接
int CUR2Drv::OpenBVConn()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    bool bConnect = m_pUSBDrv->OpenVHUSBConnect(CONNECT_TYPE_ZERO);
    if (!bConnect)
    {
        Log(ThisModule, -1, "打开ZeroBV连接失败");
        return ERR_UR_USB_CONN_ERR;
    }
    Log(ThisModule, 1, "打开ZeroBV连接成功");
    return ERR_UR_SUCCESS;
}

//功能：关闭UR连接
//输入：
//输出：无
//返回：0: 成功，非0: 失败
//说明：关闭ZVB和HCM的USB连接
int CUR2Drv::CloseURConn()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    bool bConnect = m_pUSBDrv->CloseVHUSBConnect(CONNECT_TYPE_UR);
    if (!bConnect)
    {
        Log(ThisModule, -1, "关闭UR连接失败");
        return ERR_UR_USB_CONN_ERR;
    }
    return ERR_UR_SUCCESS;
}

//功能：关闭BV连接
//输入：
//输出：无
//返回：0: 成功，非0: 失败
//说明：关闭ZVB和HCM的USB连接
int CUR2Drv::CloseBVConn()
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();
    bool bConnect = m_pUSBDrv->CloseVHUSBConnect(CONNECT_TYPE_ZERO);
    if (!bConnect)
    {
        Log(ThisModule, -1, "关闭ZeroBV连接失败");
        return ERR_UR_USB_CONN_ERR;
    }
    return ERR_UR_SUCCESS;
}


//功能：校验返回数据的合法性，分析提取每个包数据至map中
//      校验szResp长度与所有packet数据的总和是否相等,每个packet长度是否与协议相等
//输入：szResp：返回信息缓冲区首地址
//输入：iMinPacketCount：解析出的最小数据包个数，取值为协议中固定包个数，为0则不校验解析后的数据包个数
//输入：ThisModule：调用函数名称
//输出：mapPacket：存储响应数据中的每个Packet包信息
//返回：TRUE:数据校验合格， FALSE:数据校验异常
//说明：无
BOOL CUR2Drv::CheckRespEachPacket(
const char *szResp,
int &iMinPacketCount,
PACKETMAP &mapPacket,
const char *ThisModule)
{
    //校验参数合法性
    // VERIFYPOINTNOTEMPTY(szResp);

    if (!mapPacket.empty())
    {
        mapPacket.clear();
    }

    USHORT usTotalRespLen = MAKEWORD(szResp[1], szResp[0]);
    if (usTotalRespLen < MIN_RESP_PACKET_LEN) //最小包数据长度
    {
        Log(ThisModule, -1, "返回数据校验不合法, 数据包总长度%d小于最小数据包长度%d", usTotalRespLen, MIN_RESP_PACKET_LEN);
        return FALSE;
    }

    //解析返回数据并保存至MAP中
    map<USHORT, USHORT>::iterator itPacketIDLength;
    ST_PACKET_INFO stPacketInfo;
    USHORT usCurPos = 2;
    USHORT usPakcetIDLength = 0;
    while (usCurPos + 4 < usTotalRespLen)
    {
        memset(&stPacketInfo, 0, sizeof(ST_PACKET_INFO));
        stPacketInfo.usPacketID = MAKEWORD(szResp[usCurPos + 1], szResp[usCurPos]);
        stPacketInfo.usLength   = MAKEWORD(szResp[usCurPos + 3], szResp[usCurPos + 2]);
        stPacketInfo.usDataPos  = usCurPos + 4;

        //校验PacketID与长度，对未定义的包忽略校验
        itPacketIDLength = m_mPacketIDLength.find(stPacketInfo.usPacketID);
        if (itPacketIDLength != m_mPacketIDLength.end())
        {
            //期望数据长度为0则不校验该值
            usPakcetIDLength = itPacketIDLength->second;
            if (usPakcetIDLength != 0 && stPacketInfo.usLength != usPakcetIDLength)
            {
                Log(ThisModule, ERR_UR_RESPDATA_ERR, "响应数据包ID0X%04X返回长度为%d,与期望长度%d不符合", \
                    stPacketInfo.usPacketID, stPacketInfo.usLength, usPakcetIDLength);
                return FALSE;
            }
        }

        //PacketID + Length + DATA
        usCurPos = usCurPos + 2 + stPacketInfo.usLength;
        mapPacket.insert(pair<USHORT, const ST_PACKET_INFO>(stPacketInfo.usPacketID, stPacketInfo));
    }

    if (mapPacket.empty() || mapPacket.size() < iMinPacketCount)
    {
        if (mapPacket.empty())
        {
            Log(ThisModule, ERR_UR_RESPDATA_ERR, "未解析出任何数据包");
        }
        else
        {
            Log(ThisModule, ERR_UR_RESPDATA_ERR, "解析出%d个数据包，小于期望解析出的数据包个数%d", mapPacket.size(), iMinPacketCount);
            mapPacket.clear();
        }
        return FALSE;
    }
    iMinPacketCount = mapPacket.size();
    return TRUE;
}

//功能：在数据包Map中查找指定ID包数据起始位置
//输入：mapPacket：存储响应数据中的每个Packet包信息
//输入：usPacketID：Packet包数据ID
//输入：ThisModule：调用函数名称
//输入：bRecordLog：未找到指定ID包时是否记录日志
//返回：>0 成功，<0 失败；见返回码定义
//说明：无
int CUR2Drv::FindMapPacketDataPosforID(const PACKETMAP &mapPacket,
                                       USHORT usPacketID,
                                       const char *ThisModule,
                                       BOOL bRecordLog)
{
    //校验参数合法性
    if (usPacketID == NULL || mapPacket.empty())
    {
        Log(ThisModule, ERR_UR_RESPDATA_ERR, "PacketID无效或Packet包信息数据为空");
        return ERR_UR_RESPDATA_ERR;
    }

    PACKETMAP::const_iterator it = mapPacket.find(usPacketID);
    if (it == mapPacket.end())
    {
        if (bRecordLog)
            Log(ThisModule, ERR_UR_RESPDATA_ERR, "未找到PACKET_ID[0X%04X]数据", usPacketID);
        return ERR_UR_RESPDATA_ERR;
    }

    ST_PACKET_INFO stPacketInfo = it->second;
    int iDataPos = stPacketInfo.usDataPos;

    return iDataPos;
}

//功能：校验提取RESP数据包中的响应码
//输入：szResp：返回信息缓冲区首地址
//输入：mapPacket：Packet包信息数据
//输入：ThisModule：函数名称
//输出：无
//返回：响应码，失败
//说明：无
bool CUR2Drv::ParseRESP(
const char *szResp,
const PACKETMAP &mapPacket,
USHORT &usRespCode,
const char *ThisModule)
{
    if (szResp == nullptr || mapPacket.empty())
    {
        Log(ThisModule, ERR_UR_RESPDATA_ERR, "返回数据PRSE包为NULL或Packet包信息数据为空");
        return false;
    }

    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_RESP, ThisModule);
    if (iDataPos <= 0)
    {
        return false;
    }
    usRespCode = MAKEWORD(szResp[iDataPos + 1], szResp[iDataPos]);
    return true;
}


//功能：将命令执行完成后返回信息中机芯错误信息解析出来保存在成员变量
//输入：szResp：返回信息缓冲区首地址
//输入：mapPacket：Packet包信息数据
//输出：无
//返回：0 成功，<0 失败；见返回码定义
//说明：无
int CUR2Drv::SaveErrDetail(
const char *szResp,
const PACKETMAP &mapPacket,
const char *ThisModule)
{
    //校验参数合法性
    if (szResp == nullptr || mapPacket.empty())
    {
        Log(ThisModule, ERR_UR_RESPDATA_ERR, "返回数据PRSE包为NULL或Packet包信息数据为空");
        return ERR_UR_RESPDATA_ERR;
    }

    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_COMMONBLOCK, ThisModule);
    if (iDataPos <= 0)
    {
        Log(ThisModule, ERR_UR_RESPDATA_ERR, "返回数据PRSE包查找COMMONBLOCK包失败");
        return ERR_UR_RESPDATA_ERR;
    }

    for (int i = 0; i < 4; i++)
    {
        sprintf(m_stErrDetail.ucErrorCode + i * 2, "%02.02X", szResp[iDataPos + i]);
    }
    if (m_stErrDetail.ucErrorCode[0] == '0')
    {
        strcpy(m_stErrDetail.ucErrorCode, m_stErrDetail.ucErrorCode + 1);
    }
    m_stErrDetail.iCassetteError       = szResp[iDataPos + 10];
    m_stErrDetail.usRecoveryCode       = MAKEWORD(szResp[iDataPos + 13], szResp[iDataPos + 12]);
    memcpy(m_stErrDetail.ucPostionCode,  szResp + iDataPos + 14, 16);


    return ERR_UR_SUCCESS;
}

//功能：将命令执行完成后返回信息中机芯状态信息解析出来保存在成员变量
//输入：szResp：返回信息缓冲区首地址
//输入：mapPacket：Packet包信息数据
//输出：无
//返回：0 成功，<0 失败；见返回码定义
//说明：无
int CUR2Drv::SaveStatusInfo(
const char *szResp,
const PACKETMAP &mapPacket,
const char *ThisModule)
{
    //校验参数合法性
    if (szResp == nullptr || mapPacket.empty())
    {
        Log(ThisModule, ERR_UR_RESPDATA_ERR, "返回数据PRSE包为NULL或Packet包信息数据为空");
        return ERR_UR_RESPDATA_ERR;
    }

    int iCount = 0;
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_STATUSINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    m_stDevStatusInfo.bCashAtShutter       = (szResp[iDataPos] & 0x80) ? TRUE : FALSE;
    m_stDevStatusInfo.bCashAtCS            = (szResp[iDataPos] & 0x40) ? TRUE : FALSE;
    m_stDevStatusInfo.bCashAtCSErrPos      = (szResp[iDataPos] & 0x20) ? TRUE : FALSE;
    m_stDevStatusInfo.bCashAtOutCS         = (szResp[iDataPos] & 0x10) ? TRUE : FALSE;  //test#28


    m_stDevStatusInfo.bESCFull             = (szResp[iDataPos + 1] & 0x80) ? TRUE : FALSE;
    m_stDevStatusInfo.bESCEmpty            = (szResp[iDataPos + 1] & 0x40) ? TRUE : FALSE;
    m_stDevStatusInfo.bURJBFull            = (szResp[iDataPos + 1] & 0x20) ? TRUE : FALSE;
    m_stDevStatusInfo.bURJBEmpty           = (szResp[iDataPos + 1] & 0x10) ? TRUE : FALSE;

    m_stDevStatusInfo.bNotesRJInCashCout   = (szResp[iDataPos + 2] & 0x80) ? TRUE : FALSE;
    m_stDevStatusInfo.bESCNearFull         = (szResp[iDataPos + 2] & 0x40) ? TRUE : FALSE;

    m_stDevStatusInfo.bHCMUPInPos          = (szResp[iDataPos + 4] & 0x80) ? TRUE : FALSE;
    m_stDevStatusInfo.bHCMLOWInPos         = (szResp[iDataPos + 4] & 0x40) ? TRUE : FALSE;
    m_stDevStatusInfo.bURJBOpen            = (szResp[iDataPos + 4] & 0x08) ? FALSE : TRUE;

    for (iCount = 0; iCount < (MAX_CASSETTE_NUM - 1); iCount++)
    {
        m_stDevStatusInfo.bCassInPos[iCount] = (szResp[iDataPos + 5] & (0x80 >> iCount)) ? TRUE : FALSE;
    }
    //m_stDevStatusInfo.bRearDoorOpen      = (szResp[iDataPos + 5] & 0x02) ? FALSE : TRUE;
    //m_stDevStatusInfo.bFrontDoorOpen     = (szResp[iDataPos + 5] & 0x01) ? FALSE : TRUE;

    m_stDevStatusInfo.bESCOpen        = (szResp[iDataPos + 6] & 0x80) ? FALSE : TRUE;
    m_stDevStatusInfo.bESCInPos       = (szResp[iDataPos + 6] & 0x40) ? TRUE : FALSE;
    m_stDevStatusInfo.bESCRearEnd     = (szResp[iDataPos + 6] & 0x20) ? TRUE : FALSE;
    m_stDevStatusInfo.bCSInPos        = (szResp[iDataPos + 6] & 0x10) ? TRUE : FALSE;
    m_stDevStatusInfo.bBVFanErr       = (szResp[iDataPos + 6] & 0x02) ? TRUE : FALSE;
    m_stDevStatusInfo.bBVOpen         = (szResp[iDataPos + 6] & 0x01) ? FALSE : TRUE;

    switch (szResp[iDataPos + 7])
    {
    case 0x02:
        m_stDevStatusInfo.iOutShutterStatus = UR2SHUTTER_STATUS_OPEN;
        break;
    case 0x01:
        m_stDevStatusInfo.iOutShutterStatus = UR2SHUTTER_STATUS_CLOSED;
        break;
    case 0x00:
        m_stDevStatusInfo.iOutShutterStatus = UR2SHUTTER_STATUS_OTHERS;
        break;
    default:
        m_stDevStatusInfo.iOutShutterStatus = UR2SHUTTER_STATUS_UNKNOWN;
    }

    for (iCount = 0; iCount < (MAX_CASSETTE_NUM - 1); iCount++)
    {
        switch (szResp[iDataPos + 8 + iCount])
        {
        case 0x00:
            m_stDevStatusInfo.CassStatus[iCount] = CASSETTE_STATUS_NORMAL;
            break;
        case 0x10:
            m_stDevStatusInfo.CassStatus[iCount] = CASSETTE_STATUS_EMPTY;
            break;
        case 0x20:
            m_stDevStatusInfo.CassStatus[iCount] = CASSETTE_STATUS_NEAREST_EMPTY;
            break;
        case 0x30:
            m_stDevStatusInfo.CassStatus[iCount] = CASSETTE_STATUS_NEAR_EMPTY;
            break;
        case 0x40:
            m_stDevStatusInfo.CassStatus[iCount] = CASSETTE_STATUS_NEAR_FULL;
            break;
        case 0x50:
            m_stDevStatusInfo.CassStatus[iCount] = CASSETTE_STATUS_FULL;
            break;
        default:
            m_stDevStatusInfo.CassStatus[iCount] = CASSETTE_STATUS_UNKNOWN;
        }
    }

    m_stDevStatusInfo.bForcedOpenShutter      = (szResp[iDataPos + 13] & 0x80) ? TRUE : FALSE;
    m_stDevStatusInfo.bForcedRemovCashInCS    = (szResp[iDataPos + 13] & 0x40) ? TRUE : FALSE;
    m_stDevStatusInfo.bCashLeftInCS           = (szResp[iDataPos + 13] & 0x20) ? TRUE : FALSE;
    m_stDevStatusInfo.bCashExistInESC         = (szResp[iDataPos + 13] & 0x10) ? TRUE : FALSE;
    m_stDevStatusInfo.bReqReadStatus          = (szResp[iDataPos + 13] & 0x08) ? TRUE : FALSE;
    m_stDevStatusInfo.bReqGetOPLog            = (szResp[iDataPos + 13] & 0x04) ? TRUE : FALSE;
    m_stDevStatusInfo.bReqReset               = (szResp[iDataPos + 13] & 0x02) ? TRUE : FALSE;

    m_stDevStatusInfo.bBVWarning              = (szResp[iDataPos + 15] & 0x80) ? TRUE : FALSE;

    m_stDevStatusInfo.bDuringEnergy           = (szResp[iDataPos + 16] & 0x20) ? TRUE : FALSE;

    return ERR_UR_SUCCESS;
}

//功能：分析暂存钞票数据包信息
//输入：szResp：返回信息缓冲区首地址
//输入：mapPacket：Packet包信息数据
//输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
//输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
//输出：pNumStackedToPerCass: 表示各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
//返回：0 成功，<0 失败；见返回码定义
//说明：无
int CUR2Drv::ParseRespStackedNotes(
const char *szResp,
const PACKETMAP &mapPacket,
int &iNumStackedToCS,
int &iNumStackedToESC,
int pNumStackedToPerCass[MAX_CASSETTE_NUM],
const char *ThisModule)
{
    //校验参数合法性
    if (szResp == nullptr || mapPacket.empty() || pNumStackedToPerCass == nullptr)
    {
        Log(ThisModule, ERR_UR_RESPDATA_ERR, "返回数据PRSE包为NULL或Packet包信息数据为空");
        return ERR_UR_RESPDATA_ERR;
    }

    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMSTACKEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    iNumStackedToCS  = MAKEWORD(szResp[iDataPos + 1], szResp[iDataPos]);
    iNumStackedToESC = MAKEWORD(szResp[iDataPos + 3], szResp[iDataPos + 2]);
    for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        pNumStackedToPerCass[i] = MAKEWORD(szResp[iDataPos + 7 + i * 2], szResp[iDataPos + 6 + i * 2]);
    }

    pNumStackedToPerCass[MAX_CASSETTE_NUM - 1] = MAKEWORD(szResp[iDataPos + 47], szResp[iDataPos + 46]);
    return ERR_UR_SUCCESS;
}

//功能：分析点钞数据包信息
//输入：szResp：返回信息缓冲区首地址
//输入：mapPacket：Packet包信息数据
//输出：iNumCSFed:  CS点钞数
//输出：iNumESCFed: ESC点钞数
//输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
//返回：0 成功，<0 失败；见返回码定义
//说明：无
int CUR2Drv::ParseRespFedNotes(
const char *szResp,
const PACKETMAP &mapPacket,
int &iNumCSFed,
int &iNumESCFed,
int pNumPerCassFed[MAX_CASSETTE_NUM],
const char *ThisModule)
{
    //校验参数合法性
    if (szResp == nullptr || mapPacket.empty() || pNumPerCassFed == nullptr)
    {
        Log(ThisModule, ERR_UR_RESPDATA_ERR, "返回数据PRSE包为NULL或Packet包信息数据为空");
        return ERR_UR_RESPDATA_ERR;
    }

    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMFEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //iNumCSFed  = (int)(((UCHAR)szResp[iDataPos] << 8) + (UCHAR)szResp[iDataPos + 1]);
    //iNumESCFed = (int)(((UCHAR)szResp[iDataPos + 2] << 8) + (UCHAR)szResp[iDataPos + 3]);
    iNumCSFed  = MAKEWORD(szResp[iDataPos + 1], szResp[iDataPos]);
    iNumESCFed = MAKEWORD(szResp[iDataPos + 3], szResp[iDataPos + 2]);
    for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        pNumPerCassFed[i] = MAKEWORD(szResp[iDataPos + 7 + i * 2], szResp[iDataPos + 6 + i * 2]);
    }

    pNumPerCassFed[MAX_CASSETTE_NUM - 1] = 0;
    return ERR_UR_SUCCESS;
}

//功能：分析每种面额钞票存储张数数据包信息
//输入：szResp：返回信息缓冲区首地址
//输入：mapPacket：Packet包信息数据
//输入：iStackeNotesDenoInfoType：处理数据包类型
//输出：stStackeNotesDenoInfo：每种面额钞票的流向、张数信息
//返回：0 成功，<0 失败；见返回码定义
//说明：无
int CUR2Drv::ParseRespNumStackedNotesPerDeno(
const char *szResp,
const PACKETMAP &mapPacket,
const NOTES_DENO_INFO_TYPE iStackeNotesDenoInfoType,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
const char *ThisModule)
{
    //校验参数合法性
    if (szResp == nullptr || mapPacket.empty())
    {
        Log(ThisModule, ERR_UR_RESPDATA_ERR, "返回数据PRSE包为NULL或Packet包信息数据为空");
        return ERR_UR_RESPDATA_ERR;
    }

    stStackeNotesDenoInfo.ucCount = 0;
    memset(stStackeNotesDenoInfo.stStackeNotesInfo, 0, sizeof(ST_STACKE_NOTES_DENO_INFO) * MAX_DENOMINATION_NUM);

    int iDataPos = 0;
    USHORT iPacketIDStart = 0;
    USHORT iPacketIDEnd = 0;
    int iRepNumDeno = 0;

    switch (iStackeNotesDenoInfoType)
    {
    case UNFIT_NOTES_DENO_INFO_TYPE:
        iPacketIDStart = PACKET_ID_UNFITNUMDENO_16;
        iPacketIDEnd = PACKET_ID_UNFITNUMDENO_128;
        break;
    case REJECT_SOURCE_NOTES_DENO_INFO_TYPE:
        iPacketIDStart = PACKET_ID_REJECTNUMDENOSOURCE;
        iPacketIDEnd = PACKET_ID_REJECTNUMDENOSOURCE;
        break;
    case REJECT_DEST_NOTES_DENO_INFO_TYPE:
        iPacketIDStart = PACKET_ID_REJECTNUMDENO_16;
        iPacketIDEnd = PACKET_ID_REJECTNUMDENO_128;
        break;
    case REJECT_BV_NOTES_DENO_INFO_TYPE:
        iPacketIDStart = PACKET_ID_REJECTNUMDENOBV_16;
        iPacketIDEnd = PACKET_ID_REJECTNUMDENOBV_128;
        break;
    case STACKE_NOTES_DENO_INFO_TYPE:
        iPacketIDStart = PACKET_ID_STACKENUMDENO_16;
        iPacketIDEnd = PACKET_ID_STACKENUMDENO_128;
        break;
    default:
        {
            Log(ThisModule, ERR_UR_RESPDATA_ERR, "无效的数据包类型参数: %d", iStackeNotesDenoInfoType);
            return ERR_UR_PARAM;
        }
    }

    for (USHORT i = iPacketIDStart; i <= iPacketIDEnd ; i++)
    {
        iDataPos = FindMapPacketDataPosforID(mapPacket, i, ThisModule, FALSE);
        if (iDataPos <= 0)
        {
            continue;
        }

        switch (i)
        {
        case PACKET_ID_UNFITNUMDENO_16:
        case PACKET_ID_REJECTNUMDENOSOURCE:
        case PACKET_ID_REJECTNUMDENO_16:
        case PACKET_ID_REJECTNUMDENOBV_16:
        case PACKET_ID_STACKENUMDENO_16:
            iRepNumDeno = 16;
            break;

        case PACKET_ID_UNFITNUMDENO_32:
        case PACKET_ID_REJECTNUMDENO_32:
        case PACKET_ID_REJECTNUMDENOBV_32:
        case PACKET_ID_STACKENUMDENO_32:
            iRepNumDeno = 32;
            break;

        case PACKET_ID_UNFITNUMDENO_64:
        case PACKET_ID_REJECTNUMDENO_64:
        case PACKET_ID_REJECTNUMDENOBV_64:
        case PACKET_ID_STACKENUMDENO_64:
            iRepNumDeno = 64;
            break;

        case PACKET_ID_UNFITNUMDENO_128:
        case PACKET_ID_REJECTNUMDENO_128:
        case PACKET_ID_REJECTNUMDENOBV_128:
        case PACKET_ID_STACKENUMDENO_128:
            iRepNumDeno = 128;
            break;
        }

        if (iRepNumDeno == 16 ||
            iRepNumDeno == 32 ||
            iRepNumDeno == 64 ||
            iRepNumDeno == 128)
        {
            char szRespDeno[4 + 1] = {0};
            char szRespDenoEmpty[4 + 1] = {0};
            char szRespDenoInvalid[4 + 1] = {0};
            memset(szRespDenoInvalid, 0xFF, sizeof(szRespDenoInvalid));
            for (int j = 0; j < iRepNumDeno; j++)
            {
                memset(szRespDeno, 0, sizeof(szRespDeno));
                memcpy(szRespDeno, szResp + iDataPos + j * 4, 4);

                if (strncmp(szRespDeno, szRespDenoEmpty, 4) == 0 || strncmp(szRespDeno, szRespDenoInvalid, 4) == 0)
                {
                    break;
                }

                stStackeNotesDenoInfo.ucCount += 1;
                stStackeNotesDenoInfo.stStackeNotesInfo[j].ucDENOCode = szResp[iDataPos + j * 4];
                stStackeNotesDenoInfo.stStackeNotesInfo[j].iDest = CASSETTE_ROOM_ID(szResp[iDataPos + j * 4 + 1]);
                stStackeNotesDenoInfo.stStackeNotesInfo[j].usNumberStack = MAKEWORD(szResp[iDataPos + j * 4 + 3], szResp[iDataPos + j * 4 + 2]);
            }
        }
        else
        {
            Log(ThisModule, ERR_UR_RESPDATA_ERR, "未找到PACKET_ID[0x%04X] 至 [0x%04X] 的数据包", iPacketIDStart, iPacketIDEnd);
            return ERR_UR_RESPDATA_ERR;
        }
    }

    return ERR_UR_SUCCESS;
}

//功能：检查SetUnitInfo()参数中的钱箱类型与RB/AB 操作是否合法
//输入：stArrCassType:同SetUnitInfo()参数stCassType
//返回：合法返回 0；不合法返回 ERR_UR_PARAM
//说明：无
bool CUR2Drv::CheckCSSTpAndRBABOptn(const ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM])
{
    THISMODULE(__FUNCTION__);
    if (nullptr == stCassType)
    {
        Log("SetUnitInfo", ERR_UR_PARAM, "参数stCassType为空");
        return false;
    }

    for (int i = 0; i < MAX_CASSETTE_NUM; i++)
    {
        if (CASSETTE_TYPE_RB == stCassType[i].iCassType)//RB箱类型
        {
            if ((RB_OPERATION_RECYCLE  == stCassType[i].iCassOper)   ||
                (RB_OPERATION_DISPENSE == stCassType[i].iCassOper)   ||
                (RB_OPERATION_DEPOSITE == stCassType[i].iCassOper))
            {
                continue;
            }

            Log("SetUnitInfo", ERR_UR_PARAM, "第：%d个钱箱类型与钱箱操作设置不合法", (i + 1));
            return  false;
        }

        if ((CASSETTE_TYPE_AB  ==  stCassType[i].iCassType)  &&   //AB箱类型并且指定所有面额
            0xFF == stCassType[i].iDenoCode)
        {
            if (AB_OPERATION_DEPREJRET == stCassType[i].iCassOper)
            {
                continue;
            }

            Log("SetUnitInfo", ERR_UR_PARAM, "第：%d个钱箱类型与钱箱操作设置不合法", (i + 1));
            return  false;
        }
    }//end of for 6 个钞箱信息

    return true;
}


//功能：将HCM控制设置参数转化为控制数组，方便数据包处理
//输入：stOperationalInfo: HCM控制设置
//输出：bArrayOPInfo HCM控制设置数组
//返回：无
//说明：无
void CUR2Drv::OPInfoToArray(const ST_OPERATIONAL_INFO stOperationalInfo, char bArrayOPInfo[4])
{
    THISMODULE(__FUNCTION__);
    if (stOperationalInfo.bArticle6Support)
    {
        bArrayOPInfo[0] |= 0x40;
    }

    if (stOperationalInfo.bActiveVerificationLevel)
    {
        bArrayOPInfo[0] |= 0x04;
    }
    if (stOperationalInfo.bRejectUnfitNotesStore)
    {
        bArrayOPInfo[0] |= 0x02;
    }

    if (stOperationalInfo.bReportUnacceptDeno)
    {
        bArrayOPInfo[1] |= 0x02;
    }

    bArrayOPInfo[2] |= 0x40;

    if (stOperationalInfo.bCashCountErrAsWarning)
    {
        bArrayOPInfo[2] |= 0x20;
    }
    if (stOperationalInfo.bUseCorrectionFunction)
    {
        bArrayOPInfo[2] |= 0x10;
    }
    if (stOperationalInfo.bDispErrAsWarning)
    {
        bArrayOPInfo[2] |= 0x04;
    }
    if (stOperationalInfo.bShutterCheckBeforeDispense)
    {
        bArrayOPInfo[2] |= 0x02;
    }

    if (stOperationalInfo.bUseSNImageReadFunction)
    {
        bArrayOPInfo[3] |= 0x80;
    }
    if (stOperationalInfo.bStorDiffSizeNotesInDeposit)
    {
        bArrayOPInfo[3] |= 0x10;
    }
    if (stOperationalInfo.bUseSNReadFuncton)
    {
        bArrayOPInfo[3] |= 0x08;
    }
    //if (!stOperationalInfo.bCassMemoryOperation) 固定值修改00报错
    {
        bArrayOPInfo[3] |= 0x01;
    }
}

//功能：将控制数组转化HCM控制设置参数，方便数据包处理
//输入：bArrayOPInfo HCM控制设置数组
//输出：stOperationalInfo: HCM控制设置
//返回：无
//说明：无
void CUR2Drv::OPInfoFromArray(const char bArrayOPInfo[4], ST_OPERATIONAL_INFO &stOperationalInfo)
{

    THISMODULE(__FUNCTION__);
    memset(&stOperationalInfo, 0, sizeof(stOperationalInfo));
    if (bArrayOPInfo[0] & 0x40)
    {
        stOperationalInfo.bArticle6Support = TRUE;
    }
    if (bArrayOPInfo[0] & 0x04)
    {
        stOperationalInfo.bActiveVerificationLevel = TRUE;
    }
    if (bArrayOPInfo[0] & 0x02)
    {
        stOperationalInfo.bRejectUnfitNotesStore = TRUE;
    }

    if (bArrayOPInfo[1] & 0x02)
    {
        stOperationalInfo.bReportUnacceptDeno = TRUE;
    }

    if (bArrayOPInfo[2] & 0x20)
    {
        stOperationalInfo.bCashCountErrAsWarning = TRUE;
    }
    if (bArrayOPInfo[2] & 0x10)
    {
        stOperationalInfo.bUseCorrectionFunction = TRUE;
    }
    if (bArrayOPInfo[2] & 0x04)
    {
        stOperationalInfo.bDispErrAsWarning = TRUE;
    }
    if (bArrayOPInfo[2] & 0x02)
    {
        stOperationalInfo.bShutterCheckBeforeDispense = TRUE;
    }

    if (bArrayOPInfo[3] & 0x80)
    {
        stOperationalInfo.bUseSNImageReadFunction = TRUE;
    }
    if (bArrayOPInfo[3] & 0x10)
    {
        stOperationalInfo.bStorDiffSizeNotesInDeposit = TRUE;
    }
    if (bArrayOPInfo[3] & 0x08)
    {
        stOperationalInfo.bUseSNReadFuncton = TRUE;
    }

    if (bArrayOPInfo[3] & 0x01)
    {
        stOperationalInfo.bCassMemoryOperation = FALSE;
    }
    else
    {
        stOperationalInfo.bCassMemoryOperation = TRUE;
    }
}

//功能1：获取HCM和ZVB固件版本信息并初始化机芯,在上电或Reboot后需要首先发送该命令
//功能2：获取HCM和ZVB固件版本信息
//输入：bNeedInitial 为TRUE使用功能1，FALSE使用功能2
//输出：szFWVersion: 各固件程序的版本信息字符串，由使用该接口的测试程序进行解析
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：20
int CUR2Drv::GetFWVersion(
char szFWVersion[MAX_FW_DATA_LENGTH],
USHORT &usLen,
BOOL bNeedInitial
)
{
    const char *ThisModule = bNeedInitial ? "GetFWVersionInitial" : "GetFWVersion";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(szFWVersion);

    //拼接命令数据
    USHORT dwCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[HT_CMD_LEN_QUERYINFO + 1] = {0};
    if (bNeedInitial)
    {
        MOSAICCMND(szCmdData, dwCmdLen, 0x0305);
    }
    else
    {
        MOSAICCMND(szCmdData, dwCmdLen, 0x0300);
    }
    dwCmdLen += 6;
    szCmdData[0] = static_cast<char>((dwCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(dwCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, dwCmdLen, szResp, dwRespLen, 366,  HT_RESP_TIMEOUT_20, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 11;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回

    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return    ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x0300:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x0370:
        iRet = ERR_UR_WARN;
        break;  //The Command $0305 is issued but not right after Power ON
    case 0x0380:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x03FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //分析固件信息
    memset(szFWVersion, 0, sizeof(szFWVersion[0]) * MAX_FW_DATA_LENGTH);
    int iDataPos = 0;
    USHORT i = 0;
    for (; i < 8; i++)
    {
        iDataPos = FindMapPacketDataPosforID(mapPacket, i + PACKET_ID_FW_UR2BOOT, ThisModule);
        if (iDataPos <= 0)
        {
            continue;
        }

        //FWName & VER
        memcpy(szFWVersion + (28 * i), szResp + iDataPos, 28);
        usLen += 28;
    }

    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_FW_VT, ThisModule);
    if (iDataPos > 0)
    {
        memcpy(szFWVersion + (28 * i), szResp + iDataPos, 16);
        usLen += 16;
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：设置校验级别，减少废钞率
//输入：stSetVerificationLevel：需要减少废钞率设置为TRUE，否则设置为FALSE
//      建议：cashcount设置为FALSE，storemoney和dispense设置为TRUE以减少废钞率
int CUR2Drv::SetVerificationLevel(SET_VERIFICATION_LEVEL stSetVerificationLevel)
{
    /*
    const char *ThisModule = "SetVerificationLevel";

    //拼接命令数据
    UCHAR szCmdData[6 + 1];
    szCmdData[0] = 0x00;
    szCmdData[1] = 0x06;
    szCmdData[2] = 0x00;//00 F1为自定义命令格式，用于调用_F_SetFnSetting
    szCmdData[3] = 0xF1;

    for (int i = 0; i < 3; i++)
    {
        szCmdData[4] = i + 1;
        switch(i)
        {
        case 0:
            szCmdData[5] = stSetVerificationLevel.bSetForCashCount ? 0x01 : 0x00;
            break;
        case 1:
            szCmdData[5] = stSetVerificationLevel.bSetForStoreMoney ? 0x01 : 0x00;
            break;
        case 2:
            szCmdData[5] = stSetVerificationLevel.bSetForDispense ? 0x01 : 0x00;
            break;
        }

        //发送并接收响应
        char szResp[1024] = {0};
        DWORD dwRespLen = 1024; // 输入：缓冲区长度，输出：实际接收响应字节数
        int iRet = ExecuteCmd(szResp, dwRespLen, 0, szCmdData, 6, HT_RESP_TIMEOUT_120, ThisModule, CONNECT_TYPE_ZERO);
        if (ERR_UR_SUCCESS != iRet)
        {
            Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
            return iRet;
        }
    }

    //命令无返回数据，将错误码清空
    EnterCriticalSection(&m_CritSectErrDetail);
    memset(&m_stErrDetail, 0, sizeof(ST_ERR_DETAIL));
    LeaveCriticalSection(&m_CritSectErrDetail);
    */
    return ERR_UR_SUCCESS;
}

//功能：查询BV序列号信息以及BV支持的币种ID面额额配置表信息
//输入：无
//输出：pBVInfo:BV信息结构
//输出：pArrDenoInfo:面额信息结构数组，数组大小为128;
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：120
//      在上电或Reboot后需要重新发送该命令
int CUR2Drv::GetBanknoteInfo(ST_BV_INFO   &pBVInfo,
                             ST_DENO_INFO pArryDenoInfo[MAX_DENOMINATION_NUM])
{
    const char *ThisModule = "GetBanknoteInfo";

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[HT_CMD_LEN_QUERYINFO + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x6201);
    iCmdLen += 6;
    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen,  szResp, dwRespLen, 2132, HT_RESP_TIMEOUT_120, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 6;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return    ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case static_cast<USHORT>( 0x6200 ):
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case static_cast<USHORT>( 0x6280 ):
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case static_cast<USHORT>( 0x62FF ):
        iRet = ERR_UR_INVALID_COMMAN;
        break;; //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //处理BV序列号"BVZ10/20SerialNumber" 10/20:HOST-BV Advanced/Basic type
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_VALIDATORID, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memcpy(pBVInfo.szBVSerialNumber, szResp + iDataPos, 16);

    //处理BV属性信息
    char szSpecificFunc[3] = {0};
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_SPECIFICFUNC, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memcpy(szSpecificFunc, szResp + iDataPos, 2);

    pBVInfo.bArticle6Support                 = (szSpecificFunc[0] & 0x80) ? TRUE : FALSE;
    pBVInfo.bBackTracingSupport              = (szSpecificFunc[0] & 0x40) ? TRUE : FALSE;
    pBVInfo.bUnknownNotesNumberInDispSupport = (szSpecificFunc[0] & 0x01) ? TRUE : FALSE;
    pBVInfo.bSNImageReadFunctionSupport      = (szSpecificFunc[1] & 0x80) ? TRUE : FALSE;
    //todo pBVInfo.bFullSNImageRecordSupport        = (szSpecificFunc[1] & 0x40) ? TRUE : FALSE;
    pBVInfo.bUnknownNotesEstimationSupport   = (szSpecificFunc[1] & 0x10) ? TRUE : FALSE;
    pBVInfo.bSNReadSNFunctionSupport         = (szSpecificFunc[1] & 0x08) ? TRUE : FALSE;
    pBVInfo.bActiveVerificationLevelSupport  = (szSpecificFunc[1] & 0x04) ? TRUE : FALSE;
    pBVInfo.bUseUnfitLevelSupport            = (szSpecificFunc[1] & 0x02) ? TRUE : FALSE;
    pBVInfo.bRejectSNSupport                 = (szSpecificFunc[1] & 0x01) ? TRUE : FALSE;


    //分析钞票列表
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_BANKNOTETABLE, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    char szNoteID[16 + 1] = {0};
    char szUnavailable[16 + 1] = {0};
    memset(szUnavailable, 0x20, 12);
    for (int i = 0; i < MAX_DENOMINATION_NUM; i++)
    {
        memset(szNoteID, 0, sizeof(szNoteID));
        memcpy(szNoteID, szResp + iDataPos + i * 16, 16);

        //全部为空格时为无效数据，即该ID无效
        if (strncmp(szNoteID, szUnavailable, 16) == 0)
        {
            pArryDenoInfo[i].iCurrencyCode = CURRENCY_CODE_RESERVED;
            pArryDenoInfo[i].iCashValue    = 0;
            pArryDenoInfo[i].ucVersion     = 0;
            pArryDenoInfo[i].ucIssuingBank = 0;
            pArryDenoInfo[i].ucNoteWidth   = 0;
            pArryDenoInfo[i].ucNoteLength  = 0;
            continue;
        }

        if (strncmp(szNoteID, "CNY", 3) == 0)
        {
            pArryDenoInfo[i].iCurrencyCode = CURRENCY_CODE_CNY;
        }
        else if (strncmp(szNoteID, "EUR", 3) == 0)
        {
            pArryDenoInfo[i].iCurrencyCode = CURRENCY_CODE_EUR;
        }
        else
        {
            pArryDenoInfo[i].iCurrencyCode = CURRENCY_CODE_RESERVED;
        }

        int iCashValue = atoi(szNoteID + 3);
        switch (szNoteID[6])
        {
        case ' ':
            pArryDenoInfo[i].iCashValue = iCashValue;
            break;
        case 'K':
            pArryDenoInfo[i].iCashValue = iCashValue * 1000;
            break;
        case 'M':
            pArryDenoInfo[i].iCashValue = iCashValue * 1000000;
            break;
        default:
            pArryDenoInfo[i].iCashValue = 0;
        }

        pArryDenoInfo[i].ucVersion     = szNoteID[7];
        pArryDenoInfo[i].ucIssuingBank = szNoteID[8];
        pArryDenoInfo[i].ucNoteWidth   = szNoteID[10];
        pArryDenoInfo[i].ucNoteLength  = szNoteID[11];
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0X%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：获得所有钞箱信息
//输入：无
//输出：pArrCassInfo:钞箱信息结构数组，数组大小为6;
//返回：0: 成功，非0: 失败
//说明：执行命令的最大时间(秒)：120
int CUR2Drv::GetCassetteInfo(ST_CASSETTE_INFO pArryCassInfo[MAX_CASSETTE_NUM])
{
    THISMODULE(__FUNCTION__);

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pArryCassInfo);

    for (int iCount = 0; iCount < MAX_CASSETTE_NUM; iCount++)
    {
        memset(&pArryCassInfo[iCount], 0, sizeof(ST_CASSETTE_INFO));
    }

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[HT_CMD_LEN_QUERYINFO + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x6300);
    iCmdLen += 6;
    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 324,  HT_RESP_TIMEOUT_120, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }


    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 4;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case static_cast<USHORT>( 0x6300 ):
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case static_cast<USHORT>( 0x6340 ):
        iRet = ERR_UR_WARN;
        break;  //UP or LOW is not in position
    case static_cast<USHORT>( 0x6380 ):
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case static_cast<USHORT>( 0x63FF ):
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "分析RESP包时数据解析错误");
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_CASSCONFIG, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    char szCassInfo[50 + 1] = {0};
    char szUnavailable[50 + 1] = {0};
    memset(szUnavailable, 0x20, 50);

    for (int i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        memset(szCassInfo, 0, sizeof(szCassInfo));
        memcpy(szCassInfo, szResp + iDataPos + i * 50, 50);
        pArryCassInfo[i].iCassNO = CASSETTE_NUMBER(i + CASSETTE_1);

        //全部为空格时为无效数据
        if (strncmp(szCassInfo, szUnavailable, 50) == 0)
        {
            pArryCassInfo[i].iCassType     = CASSETTE_TYPE_UNKNOWN;
            pArryCassInfo[i].iCassOper     = RB_OPERATION_UNKNOWN;
            pArryCassInfo[i].iCurrencyCode = CURRENCY_CODE_RESERVED;
            pArryCassInfo[i].iCashValue    = 0;
            pArryCassInfo[i].ucVersion     = 0;
            pArryCassInfo[i].ucIssuingBank = 0;
            continue;
        }

        switch (szCassInfo[0])
        {
        case 0x01:
            pArryCassInfo[i].iCassType = CASSETTE_TYPE_RB;
            break;
        case 0x03:
            pArryCassInfo[i].iCassType = CASSETTE_TYPE_AB;
            break;
        //case 0x05: pArryCassInfo[i].iCassType = CASSETTE_TYPE_DAB;    break;
        case 0x00:
            pArryCassInfo[i].iCassType = CASSETTE_TYPE_UNLOAD;
            break;
        default:
            pArryCassInfo[i].iCassType = CASSETTE_TYPE_UNKNOWN;
        }

        switch (szCassInfo[2]) // 只考虑了A类型的钞箱，B类型的钞箱的数据在后面12个字节
        {
        case 0x01:
            pArryCassInfo[i].iCassOper = RB_OPERATION_RECYCLE;
            break;
        case 0x02:
            {
                if (i == 0)
                    pArryCassInfo[i].iCassOper = AB_OPERATION_DEPREJRET;
                else
                    pArryCassInfo[i].iCassOper = RB_OPERATION_DEPOSITE;
                break;
            }
        case 0x03:
            pArryCassInfo[i].iCassOper = RB_OPERATION_DISPENSE;
            break;
        case 0x04:
            pArryCassInfo[i].iCassOper = RB_OPERATION_ESCW;
            break;
        default:
            pArryCassInfo[i].iCassOper = RB_OPERATION_UNKNOWN;
        }

        if (strncmp(szCassInfo + 4, "CNY", 3) == 0)
        {
            pArryCassInfo[i].iCurrencyCode = CURRENCY_CODE_CNY;
        }
        else if (strncmp(szCassInfo + 4, "EUR", 3) == 0)
        {
            pArryCassInfo[i].iCurrencyCode = CURRENCY_CODE_EUR;
        }
        else
        {
            pArryCassInfo[i].iCurrencyCode = CURRENCY_CODE_RESERVED;
        }

        char szValue[4] = {0};
        memcpy(szValue, szCassInfo + 7, 3);
        int iCashValue = atoi(szValue);
        switch (szCassInfo[10])
        {
        case ' ':
            pArryCassInfo[i].iCashValue = iCashValue;
            break;
        case 'K':
            pArryCassInfo[i].iCashValue = iCashValue * 1000;
            break;
        case 'M':
            pArryCassInfo[i].iCashValue = iCashValue * 1000000;
            break;
        default:
            pArryCassInfo[i].iCashValue = 0;
        }

        pArryCassInfo[i].ucVersion     = szCassInfo[11];
        pArryCassInfo[i].ucIssuingBank = szCassInfo[12];

        //返回数据中无法获得以下属性
        //DENOMINATION_CODE   iDenoCode;           // 面额代码
        //DESTINATION_REJCET  iCassNoteHandInfo;   // 钞箱对钞票操作类型
        //USHORT              usNearFullLevel;     // 钞箱几乎满设置，钞箱与挡板间距来判断将满单位毫米mm，0机芯不反馈该状态，10mm倍数增加
        //USHORT              usNearEmptyLevel;    // 钞箱将空设置，钞箱与挡板间距来判断将满单位毫米mm，0机芯不反馈该状态，10mm倍数增加
        //USHORT              usNearestEmptyLevel; // 钞箱几乎空设置，钞箱与挡板间距来判断将满单位毫米mm，0机芯不反馈该状态，10mm倍数增加
    }

    //处理URJB箱数据
    pArryCassInfo[MAX_CASSETTE_NUM - 1].iCassNO       = CASSETTE_6;
    pArryCassInfo[MAX_CASSETTE_NUM - 1].iCassType     = CASSETTE_TYPE_URJB;
    pArryCassInfo[MAX_CASSETTE_NUM - 1].iCassOper     = AB_OPERATION_DEPREJRET;
    pArryCassInfo[MAX_CASSETTE_NUM - 1].iCurrencyCode = CURRENCY_CODE_RESERVED;
    pArryCassInfo[MAX_CASSETTE_NUM - 1].iCashValue    = 0;
    pArryCassInfo[MAX_CASSETTE_NUM - 1].ucVersion     = 0;
    pArryCassInfo[MAX_CASSETTE_NUM - 1].ucIssuingBank = 0;
    pArryCassInfo[MAX_CASSETTE_NUM - 1].iDenoCode     = DENOMINATION_CODE_00;

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：设置支持的面额代码
//输入：pArryDENOCode: 需要支持的面额数组取值为DENOMINATION_CODE,已0结束
//      例如支持面额 124,pArryDENOCode=12400000.., 支持面额4，pArryDENOCode=40000..,
//输出：无
//返回：0: 成功，非0: 失败
//说明：执行命令的最大时间(秒)：20
int CUR2Drv::SetDenominationCode(const char pArryDENOCode[MAX_DENOMINATION_NUM])
{
    const char *ThisModule = "SetDenominationCode";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pArryDENOCode);
    int iArryDENOLen = 0;
//    int iArryDENOLen = static_cast<int>( strlen( pArryDENOCode ) );
//    if ( iArryDENOLen <= 0 )
//    {
//        Log( ThisModule, ERR_UR_PARAM, "设置的面额代码数组为空" );
//        return ERR_UR_PARAM;
//    }

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[268 + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x6400);
    iCmdLen += 6;

    char szCmdDENOCode[260 + 1] = {0};
    szCmdDENOCode[0] = 0x05;
    szCmdDENOCode[1] = 0x13;
    szCmdDENOCode[2] = 0x01;
    szCmdDENOCode[3] = 0x02;

    for ( int i = 0; i < 32; i++ )
    {
        char iDENOCode = pArryDENOCode[i];
        if ((6 + (iDENOCode - 1) * 2) <= 260)
        {
            szCmdDENOCode[6 + i * 2] = iDENOCode;
        }
    }

    memcpy(szCmdData + iCmdLen, szCmdDENOCode, 260);
    iCmdLen += 260;
    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_20, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return    ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case static_cast<USHORT>( 0x6400 ):
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case static_cast<USHORT>( 0x6480 ):
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case static_cast<USHORT>( 0x64FF ):
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}


//功能：设置HCM钞箱的配置和操作信息，设置命令执行后必须执行复位动作，否则动作指令报错
//      如果HCM接受设置，设置完成后通过GetUnitInfo命令可读取设置的值，如果HCM不接受该值设置，该值则为0
//输入：tTime: 系统时间
//输入：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0X8000
//输入：stOperationalInfo: HCM控制设置
//输入：stHWConfig: HCM硬件信息设置
//输入：stCassType: 钞箱类型 ST_CASSETTE_INFO结构, 以及钞箱支持的面额 取值范围DENOMINATION_CODE，DRB箱可设置多种面额，目前一个钞箱只支持一种面额设置
//输入：bArryAcceptDenoCode: 可接受面额设置 1-127种面额，128面额为不可识别面额，可接受TRUE，不接受FALSE
//输入：stCashCountLevel: 验钞动作BV校验钞票严格程度
//输入：stStoreMoneyLevel:存钞动作BV校验钞票严格程度
//输入：stDispenseLevel:  挖钞动作BV校验钞票严格程度
//输入：usArryCassCashInPrioritySet:  存钞时钞箱优先级，取值范围 1~10，取值越小优先级越高,例如123456
//输入：usArryCassCashOutPrioritySet: 挖钞时钞箱优先级，取值范围 1~10，取值越小优先级越高
//输出：无
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：20
int CUR2Drv::SetUnitInfo(
int iTotalCashInURJB,
const ST_OPERATIONAL_INFO &stOperationalInfo,
const ST_HW_CONFIG &stHWConfig,
const ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM],
const BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM],
const ST_BV_VERIFICATION_LEVEL &stCashCountLevel,
const ST_BV_VERIFICATION_LEVEL &stStoreMoneyLevel,
const ST_BV_VERIFICATION_LEVEL &stDispenseLevel,
const char usArryCassCashInPrioritySet[MAX_CASSETTE_NUM],
const char usArryCassCashOutPrioritySet[MAX_CASSETTE_NUM],
const BOOL bURJBSupp
)
{

    THISMODULE(__FUNCTION__);
    //校验参数合法性
    VERIFYPOINTNOTEMPTY(&stOperationalInfo);
    VERIFYPOINTNOTEMPTY(&stHWConfig);
    VERIFYPOINTNOTEMPTY(&stCassType);
    VERIFYPOINTNOTEMPTY(&stCassType);
    if (!CheckCSSTpAndRBABOptn(stCassType))
    {
        return ERR_UR_PARAM;
    }

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[736 + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x0600);
    iCmdLen += 6;

    //2.Data and Time(10 bytes)
    char szCmdParam[64 + 1] = {0};
    szCmdParam[0] = 0x00;
    szCmdParam[1] = 0x10;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x08;

    SYSTEMTIME st;
    CQtTime::GetLocalTime(st);
    szCmdParam[4] = (char)st.wYear   % 100;
    szCmdParam[5] = (char)st.wMonth;
    szCmdParam[6] = (char)st.wDay;
    szCmdParam[7] = (char)st.wHour   % 24;
    szCmdParam[8] = (char)st.wMinute % 60;
    szCmdParam[9] = (char)st.wSecond % 60;

    memcpy(szCmdData + iCmdLen, szCmdParam, 10);
    iCmdLen += 10;

    //3.Number of Total Stacked Notes(64 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = static_cast<char>(0x86);
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x3E;
//test#13    memset(szCmdParam + 10, 0xFF, 20);
    memset(szCmdParam + 10, 0x70, 20);                      //test#13
 //test#13   szCmdParam[50] = static_cast<char>((iTotalCashInURJB >> 8) & 0xFF);  //TCR 不支持
 //test#13   szCmdParam[51] = static_cast<char>(iTotalCashInURJB & 0xFF);           //TCR 不支持

    memcpy(szCmdData + iCmdLen, szCmdParam, 64);
    iCmdLen += 64;

    //4.Operational Information(10 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x11;//////
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x08;

    char bArrayOPInfo[4] = {0};
    OPInfoToArray(stOperationalInfo, bArrayOPInfo);
    if(bURJBSupp){
        bArrayOPInfo[2] &= 0xBF;
    } else {
        bArrayOPInfo[2] |= 0x40;
    }
    memcpy(szCmdParam + 6, bArrayOPInfo, 4);

    memcpy(szCmdData + iCmdLen, szCmdParam, 10);
    iCmdLen += 10;

    /*
    //4.1 Operational Information2(16 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x10;//////
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x14;

    memcpy(szCmdData + iCmdLen, szCmdParam, 22);
    iCmdLen += 22;
    */

    //5.Hardware Configuration(10 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x12;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x08;

    szCmdParam[6] = 0x04;                       //test#13  有钞门
    //szCmdParam[6] = static_cast<char>(0x80);    //固定值
    //szCmdParam[7] = 0x09;    //ET Type
//    szCmdParam[6] = 0x61;
/*//test#13   szCmdParam[6] = 0x01;
    if(bURJBSupp){
        szCmdParam[6] |= 0x80;
    }
    if(!stHWConfig.bURJBPoweroffCntlSupp){
        szCmdParam[6] |= 0x40;
    }
    if(!stHWConfig.bUPPoweroffCntlSupp){
        szCmdParam[6] |= 0x20;
    }

    if (stHWConfig.bHaveLane5)
    {
        szCmdParam[8] |= 0x10;
    }
    if (stHWConfig.bHaveLane4)
    {
        szCmdParam[8] |= 0x08;
    }
    if (stHWConfig.bHaveLane3)
    {
        szCmdParam[8] |= 0x04;
    }
    if (stHWConfig.bHaveLane2)
    {
        szCmdParam[8] |= 0x02;
    }
    if (stHWConfig.bHaveLane1)
    {
        szCmdParam[8] |= 0x01;
    }*/ //test#13

    memcpy(szCmdData + iCmdLen, szCmdParam, 10);
    iCmdLen += 10;

    //6.Cassette Denomination code(326 bytes)
    UCHAR szCmdParamCassDeno[326 + 1] = {0};
    szCmdParamCassDeno[0] = 0x05;
    szCmdParamCassDeno[1] = 0x15;
    szCmdParamCassDeno[2] = 0x01;
    szCmdParamCassDeno[3] = 0x44;

    UCHAR szRoomDenoCode[16 + 1] = {0};
    int i = 0;
    for(i = 0; i < MAX_CASSETTE_NUM - 1; i++)
	{
		memset(szRoomDenoCode, 0, sizeof(szRoomDenoCode));
		if (stCassType[i].iCassType != CASSETTE_TYPE_UNKNOWN && stCassType[i].iCassType != CASSETTE_TYPE_UNLOAD)
		{	
            if(stCassType[i].iDenoCode == DENOMINATION_CODE_ALL)
			{
                szRoomDenoCode[15] |= 0x01;
            }
            else
            {
                BYTE u1 = (stCassType[i].iDenoCode - 1) / 8;
                BYTE u2 = (stCassType[i].iDenoCode - 1) % 8;
                szRoomDenoCode[u1] |= 0x80 >> u2;
            }


//            //case DENOMINATION_CODE_00:      szRoomDenoCode[0]  |= 0x01; break;
//			case DENOMINATION_CODE_10_4TH:    szRoomDenoCode[0]  |= 0x80; break;
//			case DENOMINATION_CODE_50_4TH:    szRoomDenoCode[0]  |= 0x40; break;
//			case DENOMINATION_CODE_100_4TH:   szRoomDenoCode[0]  |= 0x20; break;
//			case DENOMINATION_CODE_100_5TH:   szRoomDenoCode[0]  |= 0x10; break;
//			case DENOMINATION_CODE_20_5TH:    szRoomDenoCode[0]  |= 0x08; break;
//			case DENOMINATION_CODE_10_5TH:    szRoomDenoCode[0]  |= 0x04; break;
//			case DENOMINATION_CODE_50_5TH:    szRoomDenoCode[0]  |= 0x02; break;
//            //case DENOMINATION_CODE_100_5TH:  szRoomDenoCode[1]  |= 0x04; break;
//			case DENOMINATION_CODE_100_2015:  szRoomDenoCode[1]  |= 0x02; break;
//			case DENOMINATION_CODE_ALL:       szRoomDenoCode[0]  |= 0x00; break;
//			default:                          szRoomDenoCode[15] |= 0x01;
//			}
            if (3 == i) //AB箱设置
            {
                memset(szRoomDenoCode, 0, sizeof(szRoomDenoCode));
                szRoomDenoCode[15]  |= 0x01;
            }
        }
        memcpy(szCmdParamCassDeno + 6 + i * 16, szRoomDenoCode, 16);
    }

    memcpy(szCmdData + iCmdLen, szCmdParamCassDeno, 326);
    iCmdLen += 326;

    //7.Cassette Type(36 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x18;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x22;

    bool bHasRecycle = false;
    UCHAR szCassType[6 + 1] = {0};
    for (i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        memset(szCassType, 0, sizeof(szCassType));
        switch (stCassType[i].iCassType)
        {
        case CASSETTE_TYPE_RB:
            szCassType[0] = 1;
            break;
        case CASSETTE_TYPE_AB:
            szCassType[0] = 3;
            break;
        //case CASSETTE_TYPE_DAB:     szCassType[0] = 5; break;
        case CASSETTE_TYPE_UNLOAD:
        default:
            szCassType[0] = 0;
            break;
        }

        switch (stCassType[i].iCassOper)
        {
        case RB_OPERATION_RECYCLE:
            szCassType[2] = 1;
            bHasRecycle = true;
            break;
        case AB_OPERATION_DEPREJRET:
        case RB_OPERATION_DEPOSITE:
            szCassType[2] = 2;
            break;
        case RB_OPERATION_DISPENSE:
            szCassType[2] = 3;
            break;
        default:
            szCassType[2] = 0;
            break;
        }
        memcpy(szCmdParam + 6 + i * 6, szCassType, 6);
    }

    memcpy(szCmdData + iCmdLen, szCmdParam, 36);
    iCmdLen += 36;

    //当存在循环箱时，该值bRejectUnfitNotesStore设置为true时命令执行失败，返回数据异常
    if (stOperationalInfo.bRejectUnfitNotesStore && bHasRecycle)
    {
        Log(ThisModule, -1, "设置参数错误，设置循环箱时bRejectUnfitNotesStore设置为true");
        return ERR_UR_PARAM;
    }

    //8.Note Handling Information(38 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x21;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x24;

    for (i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        if (stCassType[i].iCassType != CASSETTE_TYPE_UNKNOWN && stCassType[i].iCassType != CASSETTE_TYPE_UNLOAD)
        {
            szCmdParam[9 + i] = stCassType[i].iCassNoteHandInfo;
        }
    }
    //URJB 固定值
    //szCmdParam[29] = DESTINATION_REJCET_CS;
    szCmdParam[12] = 0x70;
    memcpy(szCmdData + iCmdLen, szCmdParam, 38);
    iCmdLen += 38;

    //9.Acceptable Denomination Code(22 bytes)  可变包，如果参数为空则不添加该数据包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (bArryAcceptDenoCode != nullptr)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x17;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x14;

        for (i = 0; i < MAX_DENOMINATION_NUM; i++)
        {
            if (bArryAcceptDenoCode[i])
            {
                szCmdParam[6 + (i / 8)] |= (0x80 >> (i % 8));
            }
        }

        memcpy(szCmdData + iCmdLen, szCmdParam, 22);
        iCmdLen += 22;
    }

    //10.Verification Level for Cash Count(22 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x1D;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x14;

    char bUnfitLevel = 0;
    char bVerifiationLevel = 0;
    switch (stCashCountLevel.iUnfitLevel)
    {
    case UNFIT_LEVEL_DEFAULT:
        bUnfitLevel = 0x00;
        break;
    case UNFIT_LEVEL_NORMAL:
        bUnfitLevel = 0x01;
        break;
    case UNFIT_LEVEL_SOFT:
        bUnfitLevel = 0x02;
        break;
    case UNFIT_LEVEL_STRICT:
        bUnfitLevel = 0x03;
        break;
    default:
        bUnfitLevel = 0x00;
        break;
    }
    szCmdParam[7]  = bUnfitLevel;
    szCmdParam[10] = bUnfitLevel;
    szCmdParam[11] = bUnfitLevel;
    szCmdParam[13] = bUnfitLevel;

    switch (stCashCountLevel.iVerifiationLevel)
    {
    case VERIFICATION_LEVEL_DEFAULT:
        bVerifiationLevel = 0x00;
        break;
    case VERIFICATION_LEVEL_NORMAL:
        bVerifiationLevel = 0x01;
        break;
    case VERIFICATION_LEVEL_STRICT:
        bVerifiationLevel = 0x02;
        break;
    default:
        bVerifiationLevel = 0x00;
    }

    szCmdParam[21] = bVerifiationLevel;
    memcpy(szCmdData + iCmdLen, szCmdParam, 22);
    iCmdLen += 22;

    //11.Verification Level for Store Money(22 bytes)
/*//test#13 TCR不支持这个命令    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x1E;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x14;

    bUnfitLevel = 0;
    bVerifiationLevel = 0;
    switch (stStoreMoneyLevel.iUnfitLevel)
    {
    case UNFIT_LEVEL_DEFAULT:
        bUnfitLevel = 0x00;
        break;
    case UNFIT_LEVEL_NORMAL:
        bUnfitLevel = 0x01;
        break;
    case UNFIT_LEVEL_SOFT:
        bUnfitLevel = 0x02;
        break;
    case UNFIT_LEVEL_STRICT:
        bUnfitLevel = 0x03;
        break;
    default:
        bUnfitLevel = 0x00;
    }
    szCmdParam[7]  = bUnfitLevel;
    szCmdParam[10] = bUnfitLevel;
    szCmdParam[11] = bUnfitLevel;
    szCmdParam[13] = bUnfitLevel;

    switch (stStoreMoneyLevel.iVerifiationLevel)
    {
    case VERIFICATION_LEVEL_DEFAULT:
        bVerifiationLevel = 0x00;
        break;
    case VERIFICATION_LEVEL_NORMAL:
        bVerifiationLevel = 0x01;
        break;
    case VERIFICATION_LEVEL_STRICT:
        bVerifiationLevel = 0x02;
        break;
    default:
        bVerifiationLevel = 0x00;
    }

    szCmdParam[21] = bVerifiationLevel;
    memcpy(szCmdData + iCmdLen, szCmdParam, 22);
    iCmdLen += 22; */

    //12.Verification Level for Dispense(22 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x1F;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x14;

    bUnfitLevel = 0;
    bVerifiationLevel = 0;
    switch (stDispenseLevel.iUnfitLevel)
    {
    case UNFIT_LEVEL_DEFAULT:
        bUnfitLevel = 0x00;
        break;
    case UNFIT_LEVEL_NORMAL:
        bUnfitLevel = 0x01;
        break;
    case UNFIT_LEVEL_SOFT:
        bUnfitLevel = 0x02;
        break;
    case UNFIT_LEVEL_STRICT:
        bUnfitLevel = 0x03;
        break;
    default:
        bUnfitLevel = 0x00;
    }
    szCmdParam[7]  = bUnfitLevel;
    szCmdParam[10] = bUnfitLevel;
    szCmdParam[11] = bUnfitLevel;
    szCmdParam[13] = bUnfitLevel;

    switch (stDispenseLevel.iVerifiationLevel)
    {
    case VERIFICATION_LEVEL_DEFAULT:
        bVerifiationLevel = 0x00;
        break;
    case VERIFICATION_LEVEL_NORMAL:
        bVerifiationLevel = 0x01;
        break;
    case VERIFICATION_LEVEL_STRICT:
        bVerifiationLevel = 0x02;
        break;
    default:
        bVerifiationLevel = 0x00;
    }

    szCmdParam[21] = bVerifiationLevel;
    memcpy(szCmdData + iCmdLen, szCmdParam, 22);
    iCmdLen += 22;

    //15.Priority Setting(44 bytes) 可变包，如果参数为空则不添加该数据包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (usArryCassCashInPrioritySet != nullptr
        && usArryCassCashOutPrioritySet != nullptr)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x29;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x2A;

        BOOL bAttachPriorityPacket = TRUE;
        for (i = 0; i < MAX_CASSETTE_NUM - 1; i++)
        {
            if (usArryCassCashInPrioritySet[i]  > 10
                || usArryCassCashOutPrioritySet[i] > 10)
            {
                bAttachPriorityPacket = FALSE;
                break;
            }

            szCmdParam[ 4 + i] = usArryCassCashInPrioritySet[i];
            szCmdParam[24 + i] = usArryCassCashOutPrioritySet[i];
        }

        if (bAttachPriorityPacket)
        {
            memcpy(szCmdData + iCmdLen, szCmdParam, 44);
            iCmdLen += 44;
        }
    }

    ////0527,0528,0519,052C,0510 todo
    ///
    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70, HT_RESP_TIMEOUT_20, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }


    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case static_cast<USHORT>( 0x0600 ):
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case static_cast<USHORT>( 0x0680 ):
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case static_cast<USHORT>( 0x06FF ):
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}


//功能：获取HCM钞箱的配置和操作信息
//输入：无
//输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0X8000
//输出：bArryDenoCodeBySet: SetUnitInfo命令中的bArryAcceptDenoCode信息
//输出：bArryAcceptDenoCode: 可接受面额信息 1-127种面额，128面额为不可识别面额，可接受TRUE，不接受FALSE
//输出：stHWConfig: HCM硬件信息设置
//输出：stCassType: 钞箱类型 ST_CASSETTE_INFO结构, 以及钞箱支持的面额 取值范围DENOMINATION_CODE，DRB箱可设置多种面额，目前一个钞箱只支持一种面额设置
//输出：stCashCountLevel: 验钞动作BV校验钞票严格程度
//输出：stStoreMoneyLevel:存钞动作BV校验钞票严格程度
//输出：stDispenseLevel:  挖钞动作BV校验钞票严格程度
//返回：0: 成功，非0: 失败
//说明：执行命令的最大时间(秒)：20
int CUR2Drv::GetUnitInfo(
int &iTotalCashInURJB,
BOOL bArryDenoCodeBySet[MAX_DENOMINATION_NUM],
BOOL bArryAcceptDenoCode[MAX_DENOMINATION_NUM],
ST_OPERATIONAL_INFO &stOperationalInfo,
ST_HW_CONFIG &stHWConfig,
ST_CASSETTE_INFO stCassType[MAX_CASSETTE_NUM],
ST_BV_VERIFICATION_LEVEL &stCashCountLevel,
ST_BV_VERIFICATION_LEVEL &stStoreMoneyLevel,
ST_BV_VERIFICATION_LEVEL &stDispenseLevel)
{

    THISMODULE(__FUNCTION__);
    //校验参数合法性
    VERIFYPOINTNOTEMPTY(bArryDenoCodeBySet);
    VERIFYPOINTNOTEMPTY(bArryAcceptDenoCode);
    VERIFYPOINTNOTEMPTY(stCassType);

    memset(bArryDenoCodeBySet, 0, sizeof(BOOL) * MAX_DENOMINATION_NUM);
    memset(bArryAcceptDenoCode, 0, sizeof(BOOL) * MAX_DENOMINATION_NUM);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[HT_CMD_LEN_QUERYINFO + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x0500);
    iCmdLen += 6;
    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 902,  HT_RESP_TIMEOUT_20, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }


    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 14;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x0500:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x0580:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x05FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //4.Number of Total Stacked Notes(64 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMTOTALSTACKEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    //iTotalCashInURJB = (int)(((UCHAR)szResp[iDataPos + 46] << 8) + (UCHAR)szResp[iDataPos + 47]);
    iTotalCashInURJB = MAKEWORD(szResp[iDataPos + 51], szResp[iDataPos + 50]);

    //5.Denomination Code Settings(260 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_DENOCODESET, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    int i = 0;
    for (i = 0; i < MAX_DENOMINATION_NUM; i++)
    {
        if (szResp[iDataPos + 2 + i * 2] != 0)
        {
            bArryDenoCodeBySet[i] = TRUE;
        }
    }

    //6.Operational Information(10 bytes)
    /*iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_OPINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }z
    */
    //Not specified yet
    if (szResp[iDataPos + 1] == 0)
    {
        memset(&stOperationalInfo, 0, sizeof(stOperationalInfo));
    }
    else
    {
        char bArrayOPInfo[4] = {0};
        memcpy(bArrayOPInfo, szResp + iDataPos + 2, 4);
        OPInfoFromArray(bArrayOPInfo, stOperationalInfo);
    }

    //7.Hardware Configuration(10 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_HWINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    BYTE bArrayHWConfig[4] = {0};
    memcpy(bArrayHWConfig, szResp + iDataPos + 2, 4);
    memset(&stHWConfig, 0, sizeof(stHWConfig));

    stHWConfig.bETType = TRUE;
    if (bArrayHWConfig[2] & 0x10)
    {
        stHWConfig.bHaveLane5 = TRUE;
    }
    if (bArrayHWConfig[2] & 0x08)
    {
        stHWConfig.bHaveLane4 = TRUE;
    }
    if (bArrayHWConfig[2] & 0x04)
    {
        stHWConfig.bHaveLane3 = TRUE;
    }
    if (bArrayHWConfig[2] & 0x02)
    {
        stHWConfig.bHaveLane2 = TRUE;
    }
    if (bArrayHWConfig[2] & 0x01)
    {
        stHWConfig.bHaveLane1 = TRUE;
    }

    //8.Cassette Denomination code(326 chars)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_CASSDENOCODE, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    UCHAR szRoomDenoCode[16 + 1] = {0};
    for (i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        memset(szRoomDenoCode, 0, sizeof(szRoomDenoCode));
        memcpy(szRoomDenoCode, szResp + iDataPos + 2 + i * 16, 16);

        stCassType[i].iCassNO = static_cast<CASSETTE_NUMBER>(i + 1);
        stCassType[i].iCurrencyCode = CURRENCY_CODE_CNY;

        if (szRoomDenoCode[0] & 0x10)
		{
            stCassType[i].iDenoCode = DENOMINATION_CODE_100_C;
			stCassType[i].iCashValue = 100;
			stCassType[i].ucVersion = 0x42;
		}
		else if (szRoomDenoCode[0] & 0x08)
		{
            stCassType[i].iDenoCode = DENOMINATION_CODE_20_C;
			stCassType[i].iCashValue = 20;
			stCassType[i].ucVersion = 0x42;
		}
		else if (szRoomDenoCode[0] & 0x04)
		{
            stCassType[i].iDenoCode = DENOMINATION_CODE_10_C;
			stCassType[i].iCashValue = 10;
			stCassType[i].ucVersion = 0x42;
		}
		else if (szRoomDenoCode[0] & 0x02)
		{
            stCassType[i].iDenoCode = DENOMINATION_CODE_50_C;
			stCassType[i].iCashValue = 50;
			stCassType[i].ucVersion = 0x42;
		}
        else if (szRoomDenoCode[1] & 0x01)
		{
            stCassType[i].iDenoCode = DENOMINATION_CODE_100_D;
            stCassType[i].iCashValue = 100;
            stCassType[i].ucVersion = 44;
		}
        else if(szRoomDenoCode[2] & 0x10)                       //30-00-00-00(FS#0018)
        {                                                       //30-00-00-00(FS#0018)
            stCassType[i].iDenoCode = DENOMINATION_CODE_5_D;    //30-00-00-00(FS#0018)
            stCassType[i].iCashValue = 5;                       //30-00-00-00(FS#0018)
            stCassType[i].ucVersion = 0x44;                     //30-00-00-00(FS#0018)
        }                                                       //30-00-00-00(FS#0018)
		else
		{
			stCassType[i].iDenoCode = DENOMINATION_CODE_00;
			stCassType[i].iCashValue = 0;
			stCassType[i].ucVersion = '*';
		}

	}

    //9.Cassette Type(36 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_CASSTYPE, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    UCHAR szCassType[6] = {0};
    for (i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        memset(szCassType, 0, sizeof(szCassType));
        memcpy(szCassType, szResp + iDataPos + 2 + i * 6, 6);
        switch (szCassType[0])
        {
        case 1:
            stCassType[i].iCassType = CASSETTE_TYPE_RB;
            break;
        case 3:
            stCassType[i].iCassType = CASSETTE_TYPE_AB;
            break;
        //case 5: stCassType[i].iCassType = CASSETTE_TYPE_DAB;     break;
        case 0:
            stCassType[i].iCassType = CASSETTE_TYPE_UNLOAD;
            break;
        default:
            stCassType[i].iCassType = CASSETTE_TYPE_UNKNOWN;
        }

        switch (szCassType[2])
        {
        case 1:
            stCassType[i].iCassOper = RB_OPERATION_RECYCLE;
            break;
        case 2:
            {
                if (i == 3)
                    stCassType[i].iCassOper = AB_OPERATION_DEPREJRET;
                else
                    stCassType[i].iCassOper = RB_OPERATION_DEPOSITE;
                break;
            }
        case 3:
            stCassType[i].iCassOper = RB_OPERATION_DISPENSE;
            break;
        case 0:
            stCassType[i].iCassOper = RB_OPERATION_UNKNOWN;
            break;
        default:
            stCassType[i].iCassOper = RB_OPERATION_UNKNOWN;
        }
    }

    //10.Note Handling Information(38 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NOTEHANDINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    for (i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        stCassType[i].iCassNoteHandInfo = DESTINATION_REJCET(szResp[5 + iDataPos + i]);
    }

    //11.Acceptable Denomination code(22 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_ACCEPTDENOCODE, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    for (i = 0; i < MAX_DENOMINATION_NUM; i++)
    {
        if ((szResp[2 + iDataPos + (i / 8)] & (0x80 >> (i % 8))))
        {
            bArryAcceptDenoCode[i] = TRUE;
        }
    }

    //12.Verification Level for Cash Count(22 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_VERLEVELCASHCOUNT, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    switch (szResp[iDataPos + 7])
    {
    case 0x00:
        stCashCountLevel.iUnfitLevel = UNFIT_LEVEL_DEFAULT;
        break;
    case 0x01:
        stCashCountLevel.iUnfitLevel = UNFIT_LEVEL_NORMAL;
        break;
    case 0x02:
        stCashCountLevel.iUnfitLevel = UNFIT_LEVEL_SOFT;
        break;
    case 0x03:
        stCashCountLevel.iUnfitLevel = UNFIT_LEVEL_STRICT;
        break;
    default:
        stCashCountLevel.iUnfitLevel = UNFIT_LEVEL_DEFAULT;
    }

    switch (szResp[iDataPos + 17])
    {
    case 0x00:
        stCashCountLevel.iVerifiationLevel = VERIFICATION_LEVEL_DEFAULT;
        break;
    case 0x01:
        stCashCountLevel.iVerifiationLevel = VERIFICATION_LEVEL_NORMAL;
        break;
    case 0x02:
        stCashCountLevel.iVerifiationLevel = VERIFICATION_LEVEL_STRICT;
        break;
    default:
        stCashCountLevel.iVerifiationLevel = VERIFICATION_LEVEL_DEFAULT;
    }

    //13.Verification Level for Store Money(22 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_VERLEVELSTORE, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    switch (szResp[iDataPos + 7])
    {
    case 0x00:
        stStoreMoneyLevel.iUnfitLevel = UNFIT_LEVEL_DEFAULT;
        break;
    case 0x01:
        stStoreMoneyLevel.iUnfitLevel = UNFIT_LEVEL_NORMAL;
        break;
    case 0x02:
        stStoreMoneyLevel.iUnfitLevel = UNFIT_LEVEL_SOFT;
        break;
    case 0x03:
        stStoreMoneyLevel.iUnfitLevel = UNFIT_LEVEL_STRICT;
        break;
    default:
        stStoreMoneyLevel.iUnfitLevel = UNFIT_LEVEL_DEFAULT;
    }

    switch (szResp[iDataPos + 17])
    {
    case 0x00:
        stStoreMoneyLevel.iVerifiationLevel = VERIFICATION_LEVEL_DEFAULT;
        break;
    case 0x01:
        stStoreMoneyLevel.iVerifiationLevel = VERIFICATION_LEVEL_NORMAL;
        break;
    case 0x02:
        stStoreMoneyLevel.iVerifiationLevel = VERIFICATION_LEVEL_STRICT;
        break;
    default:
        stStoreMoneyLevel.iVerifiationLevel = VERIFICATION_LEVEL_DEFAULT;
    }

    //14.Verification Level for Dispense(22 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_VERLEVELDISP, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    switch (szResp[iDataPos + 7])
    {
    case 0x00:
        stDispenseLevel.iUnfitLevel = UNFIT_LEVEL_DEFAULT;
        break;
    case 0x01:
        stDispenseLevel.iUnfitLevel = UNFIT_LEVEL_NORMAL;
        break;
    case 0x02:
        stDispenseLevel.iUnfitLevel = UNFIT_LEVEL_SOFT;
        break;
    case 0x03:
        stDispenseLevel.iUnfitLevel = UNFIT_LEVEL_STRICT;
        break;
    default:
        stDispenseLevel.iUnfitLevel = UNFIT_LEVEL_DEFAULT;
    }

    switch (szResp[iDataPos + 17])
    {
    case 0x00:
        stDispenseLevel.iVerifiationLevel = VERIFICATION_LEVEL_DEFAULT;
        break;
    case 0x01:
        stDispenseLevel.iVerifiationLevel = VERIFICATION_LEVEL_NORMAL;
        break;
    case 0x02:
        stDispenseLevel.iVerifiationLevel = VERIFICATION_LEVEL_STRICT;
        break;
    default:
        stDispenseLevel.iVerifiationLevel = VERIFICATION_LEVEL_DEFAULT;
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}


//功能1 复位命令，检测所有执行器与传感器，并将TS、传输通道钞票移动到CS中.
//功能2 复位并取消节能模式，需要满足以下取消节能模式条件才能正常返回，否则返回错误。
//      1.HCM在掉电前成功执行设置节能模式 2.外部Shutter已经关闭 3.HCM CS/TS，传输通道没有钞票残留
//      4.所有的上部、下部机构、CS/TS单元都在正确位置 5.所有HCMJB/TS/BV 均关闭
//      6.上部或下部机构在进入节能模式后没有被访问 7.TS门在进入节能模式后没有被访问
//功能3 快速复位命令，在HCM状态符合快速复位条件，则不执行机械动作，只检测传感器状态。如果HCM状态不满足快速复位条件或者刚上电后则进行复位命令
//      快速复位条件
//      1.CS\TS都为空; 2.没有传感器检测到钞票残留在传输路径上; 3.Shutter门处于关闭状态
//      4.最后一次Reset命令没有返回错误; 5.StoreMoney、Dispense、CashRollback、Reset命令成功返回或返回警告
//      6.所有的内部导轨/挡板均在初始位置; 7.TS没有被打开; 8.HCM没有脱离; 9.CS没有检测到强制移动钞票
//输入：bCancelEnergySaveMode: 是否取消节能模式, TRUE 取消 FLASE: 使用复位命令
//输入：bQuickRest: 是否使用快速复位, TRUE: 快速复位  FALSE: 复位
//输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0X8000
//输出：iNumStackedToCS:  暂存到CS现钞口中的所有钞票张数
//输出：iNumStackedToESC: 暂存到ESC现钞口中的所有钞票张数
//输出：pNumStackedToPerCass: 表示各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
//输出：iNumCSFed:  CS点钞数
//输出：iNumESCFed: ESC点钞数
//输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：180   快速复位与取消不能同时使用
int CUR2Drv::Reset(
int &iTotalCashInURJB,
int &iNumStackedToCS,
int &iNumStackedToESC,
int pNumStackedToPerCass[MAX_CASSETTE_NUM],
int &iNumCSFed,
int &iNumESCFed,
int pNumPerCassFed[MAX_CASSETTE_NUM],
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH],
BOOL bCancelEnergySaveMode,
BOOL bQuickRest
)
{

    THISMODULE(__FUNCTION__);

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pNumStackedToPerCass);
    VERIFYPOINTNOTEMPTY(pNumPerCassFed);
    VERIFYPOINTNOTEMPTY(pMaintenanceInfo);

    memset(pNumStackedToPerCass, 0, sizeof(pNumStackedToPerCass[0]) * MAX_CASSETTE_NUM);
    memset(pNumPerCassFed, 0, sizeof(pNumPerCassFed[0]) * MAX_CASSETTE_NUM);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[HT_CMD_LEN_ACTION + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x0701);

    if (bCancelEnergySaveMode)
    {
        MOSAICCMND(szCmdData, iCmdLen, 0x0712);
    }
    if (bQuickRest)
    {
        MOSAICCMND(szCmdData, iCmdLen, 0x0720);
    }
    iCmdLen += 6;
    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 4334, HT_RESP_TIMEOUT_180, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }


    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 7;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case static_cast<USHORT>( 0x0700 ):
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case static_cast<USHORT>(0x0702):                                       //With notes remained in CS
    case static_cast<USHORT>(0x0703):
    case static_cast<USHORT>(0x0704):
    case static_cast<USHORT>(0x0705):
    case static_cast<USHORT>(0x0706):
    case static_cast<USHORT>( 0x0707 ):
        iRet = ERR_UR_WARN;
        break;
    case static_cast<USHORT>( 0x0780 ):
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case static_cast<USHORT>( 0x07FF ):
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //3.Number of Total Stacked Notes(64 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMTOTALSTACKEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    //iTotalCashInURJB = (int)(((UCHAR)szResp[iDataPos + 50] << 8) + (UCHAR)szResp[iDataPos + 51]);
    iTotalCashInURJB = MAKEWORD(szResp[iDataPos + 51], szResp[iDataPos + 50]);

    //5.Number of Stacked Notes(64 bytes)
    int iRetParse = ParseRespStackedNotes(
                    szResp,
                    mapPacket,
                    iNumStackedToCS,
                    iNumStackedToESC,
                    pNumStackedToPerCass,
                    ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //6.Number of Fed Notes(36 bytes)
    iRetParse = ParseRespFedNotes(szResp, mapPacket, iNumCSFed, iNumESCFed, pNumPerCassFed, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //7.Maintenance Information(4100 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MAINTENINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memset(pMaintenanceInfo, 0, sizeof(char)*MAINTENANCE_INFO_LENGTH);
    memcpy(pMaintenanceInfo, szResp + iDataPos, MAINTENANCE_INFO_LENGTH);

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：请求HCM模块中保存的日志数据
//输入：iLogType: 日志类型，不可取值LOG_INFO_ALLDATA
//输入：iLogTerm: 日志信息周期
//输出：pszLogData: 返回日志数据，最大7174 * 255字节
//输出：iLogDataLen: 返回日志数据长度
//返回：0: 成功，非0: 失败
//说明：执行命令的最大时间(秒)：120
//      日志数据不会因断电而清除，只会被EraseAllLogData指令清除
int CUR2Drv::GetLogData(
LOG_INFO_TYPE iLogType,
LOG_INFO_TERM iLogTerm,
char pszLogData[MAX_LOG_DATA_LENGTH],
DWORD &iLogDataLen
)
{

    THISMODULE(__FUNCTION__);

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pszLogData);

    if ((iLogType == LOG_INFO_STATISTICAL_SPECIFIC || iLogType == LOG_INFO_SENSORLEVEL_SPECIFIC)
        && iLogTerm == TERM_WHOLE_DATA)
    {
        Log(ThisModule, ERR_UR_PARAM, "参数错误，LOG_INFO_STATISTICAL_SPECIFIC 和 LOG_INFO_SENSORLEVEL_SPECIFIC不能指定TERM_WHOLE_DATA");
        return ERR_UR_PARAM;
    }

    memset(pszLogData, 0, sizeof(pszLogData[0]) * MAX_LOG_DATA_LENGTH);
    iLogDataLen = 0;

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[20 + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x0D00);
    iCmdLen += 6;

    //2.ACT
    szCmdData[8]  = 0x00;
    szCmdData[9]  = 0x02;
    szCmdData[10] = 0x00;
    szCmdData[11] = 0x04;
    szCmdData[12] = 0x00;
    szCmdData[13] = 0x00;
    iCmdLen += 6;

    //3.Year and Month
    if (iLogTerm != TERM_WHOLE_DATA)
    {
        SYSTEMTIME tTime;
        CQtTime::GetLocalTime(tTime);

        WORD wYear  = tTime.wYear;
        WORD wMonth = tTime.wMonth;
        if (iLogTerm == TERM_LAST_MONTH)
        {
            if (tTime.wMonth >= 2)
            {
                wMonth = tTime.wMonth - 1;
            }
            else
            {
                wYear  = tTime.wYear - 1;
                wMonth = tTime.wMonth + 12 - 1;
            }

        }
        else if (iLogTerm == MONTH_BEFOR_LAST)
        {
            if (tTime.wMonth >= 3)
            {
                wMonth = (UCHAR)tTime.wMonth - 2;
            }
            else
            {
                wYear  = tTime.wYear - 1;
                wMonth = tTime.wMonth + 12 - 2;
            }
        }
        else if (iLogTerm != TERM_THIS_MONTH)
        {
            Log(ThisModule, ERR_UR_PARAM, "iLogTerm为无效数据[%d]", iLogTerm);
            return ERR_UR_PARAM;
        }

        szCmdData[14] = 0x05;
        szCmdData[15] = 0x25;
        szCmdData[16] = 0x00;
        szCmdData[17] = 0x04;
        szCmdData[18] = (UCHAR)(wYear % 100);  //Year
        szCmdData[19] = (UCHAR)(wMonth & 0xFF);//Month
        iCmdLen += 6;

    }

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = 0;   // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = 0;
    USHORT usTotalRespLen = 0;
    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    USHORT usRespCode = 0;
    USHORT usLogDataStart = 0; //日志区间
    USHORT usLogDataEnd = 0;   //日志区间

    switch (iLogType)
    {
    case LOG_INFO_STATISTICAL_TOTAL:
        usLogDataStart = 0x0300;
        usLogDataEnd = 0x0303;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_STATISTICAL_SPECIFIC:
        usLogDataStart = 0x0310;
        usLogDataEnd = 0x0313;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_ERRCODE:
        usLogDataStart = 0x0500;
        usLogDataEnd = 0x0524;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_WARNINGCODE:
        usLogDataStart = 0x0580;
        usLogDataEnd = 0x0582;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_ERRANALYSIS_GENERAL:
        usLogDataStart = 0x0700;
        usLogDataEnd = 0x075B;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_OPERATIONAL:
        usLogDataStart = 0x0800;
        usLogDataEnd = 0x0811;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_NEAREST_OPERATION:
        usLogDataStart = 0x0880;
        usLogDataEnd = 0x0880;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_ERRANALYSIS_INDIVIDUAL:
        usLogDataStart = 0x2000;
        usLogDataEnd = 0x20FF;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_SENSOR:
        usLogDataStart = 0x3000;
        usLogDataEnd = 0x303F;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_INTERNAL_COMMAND:
        usLogDataStart = 0x3040;
        usLogDataEnd = 0x307F;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_NOTES_HANDLING:
        usLogDataStart = 0x3080;
        usLogDataEnd = 0x30BF;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_MOTOR_CONTROL:
        usLogDataStart = 0x30C0;
        usLogDataEnd = 0x30DF;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_SENSORLEVEL_LATEST:
        usLogDataStart = 0xD010;
        usLogDataEnd = 0xD010;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    case LOG_INFO_SENSORLEVEL_SPECIFIC:
        usLogDataStart = 0xD020;
        usLogDataEnd = 0xD020;
        dwRespLen = HT_RESP_LEN_MAX;
        break;
    default:
        {
            Log(ThisModule, ERR_UR_PARAM, "iLogType为无效数据[%d]", iLogType);
            return ERR_UR_PARAM;
        }
    }

    //发送并接收响应
    for (USHORT i = usLogDataStart; i <= usLogDataEnd; i++)
    {
        szCmdData[12] = static_cast<char>((i >> 8) & 0xFF);
        szCmdData[13] = static_cast<char>(i & 0xFF);

        memset(szResp, 0, sizeof(szResp));
        iRet = 0;
        usTotalRespLen = 0;
        if (!mapPacket.empty())
        {
            mapPacket.clear();
        }
        usRespCode = 0;

        iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70, HT_RESP_TIMEOUT_120, ThisModule);
        if (ERR_UR_SUCCESS != iRet)
        {
            Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
            return iRet;
        }

        int iMinPacketCount = 3;
        if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
        {
            return ERR_UR_RESPDATA_ERR;
        }

        //分析RESP包，如果为无效命令或数据解析异常直接返回
        USHORT usRespCode = 0;
        if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
        {
            return ERR_UR_RESPDATA_ERR;
        }
        switch (usRespCode)
        {
        case 0x0D00:
            iRet = ERR_UR_SUCCESS;
            break;  //Normal End
        case 0x0D01:
            iRet = ERR_UR_WARN;
            break;
        case 0x0D80:
            iRet = ERR_UR_HARDWARE_ERROR;
            break;  //Abnormal End
        case 0x0DFF:
            iRet = ERR_UR_INVALID_COMMAN;
            break;  //Invalid Command
        default:
            Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
            return ERR_UR_RESPDATA_ERR;
        }

        //分析响应数据,保存状态和错误信息
        SaveErrDetail(szResp, mapPacket, ThisModule);
        SaveStatusInfo(szResp, mapPacket, ThisModule);

        if (ERR_UR_INVALID_COMMAN == iRet)
        {
            Log(ThisModule, iRet, "Invalid Command");
            return ERR_UR_INVALID_COMMAN; //Invalid Command
        }

        for (USHORT j = PACKET_ID_LOG_SENSOR; j < PAKCET_ID_LOG_SESORLEVEL; j++)
        {
            int iDataPos = FindMapPacketDataPosforID(mapPacket, j, ThisModule, FALSE);
            if (iDataPos <= 0)
            {
                continue;
            }

            DWORD iLogDataLenTemp = 7170;
            if (j == PAKCET_ID_LOG_DATAOTHERS)
            {
                iLogDataLenTemp = 7172;
            }
            else if (j == PAKCET_ID_LOG_SESORLEVEL)
            {
                iLogDataLenTemp = 2160;
            }

            if (iLogDataLen + iLogDataLenTemp > static_cast<DWORD>(MAX_LOG_DATA_LENGTH))
            {
                Log(ThisModule, -1, "日志数据长度%d大于缓冲区长度%d", iLogDataLen + iLogDataLenTemp, MAX_LOG_DATA_LENGTH);
                return ERR_UR_PARAM;
            }

            memcpy(pszLogData + iLogDataLen, szResp + iDataPos, iLogDataLenTemp);
            iLogDataLen += iLogDataLenTemp;
            break;
        }

        //异常结束
        if (iRet < 0)
        {
            Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
            return iRet;
        }

        if (iRet == ERR_UR_WARN) //日志读取完成
        {
            return ERR_UR_SUCCESS;
        }
    }

    return iRet;
}

//功能：清除HCM模块中所有保存的日志数据
//输入：无
//输出：无
//返回：0: 成功，非0: 失败
//说明：执行命令的最大时间(秒)：120
//      只有在新购买/更换HCM主板时可以使用
int CUR2Drv::EraseAllLogData()
{
    THISMODULE(__FUNCTION__);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[14 + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x0D01);
    iCmdLen += 6;

    //ACT
    szCmdData[8]  = 0x00;
    szCmdData[9]  = 0x02;
    szCmdData[10] = 0x00;
    szCmdData[11] = 0x04;
    szCmdData[12] = static_cast<char>(0xFF);
    szCmdData[13] = static_cast<char>(0xFF);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX;   // 输入：缓冲区长度，输出：实际接收响应字节数
    PACKETMAP mapPacket;

    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_120, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x0D00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x0D01:
        iRet = ERR_UR_WARN;
        break;  //Undefined ACT
    case 0x0D80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x0DFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return 0;
}

//功能：点钞存入的钞票，将钞票从CS钞口送入ESC暂存箱
//输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
//输入：iNumDepositLimit: 存款的最大钞票张数
//输入：iNumAmountLimit:  ESC最大面值总额=iNumAmountLimit * 10^[iPowerIndex]
//输入：iPowerIndex: 钞票指数，如果iNumAmountLimit为0,该值忽略
//输入：bArryAcceptDeno: 可接受的面额:128种面额，TRUE为接受，FALSE为不可接受，NULL为不设置该值使用默认设置
//输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为1时有效，否则忽略
//输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
//输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
//输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
//输出：pNumStackedToPerCass: 表示各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
//输出：iNumCSFed:  CS点钞数
//输出：iNumESCFed: ESC点钞数
//输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
//输出：nRejectNotes: 表示退回到CS的钞票总数
//输出：stStackeNotesDenoInfo：每种面额钞票的流向、张数信息
//输出：usFedNoteCondition: 钞票传输情况取值位运算，FED_NOTE_CONDITION 包含多种情况 CONDITION_SKEW & CONDITION_SHORT_NOTE
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：25张以上的拒钞将返回警告。
//说明：执行命令的最大时间(秒)：180
int CUR2Drv::CashCount(
VALIDATION_MODE iValidateMode,
int iNumDepositLimit,
int iNumAmountLimit,
NOTE_POWER_INDEX iPowerIndex,
const BOOL bArryAcceptDeno[MAX_DENOMINATION_NUM],
const ST_BV_DEPENDENT_MODE stBVDependentMode,
int &iTotalCashInURJB,
int &iNumStackedToCS,
int &iNumStackedToESC,
int pNumStackedToPerCass[MAX_CASSETTE_NUM],
int &iNumCSFed,
int &iNumESCFed,
int pNumPerCassFed[MAX_CASSETTE_NUM],
int &nRejectNotes,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeUnfitNotesDenoInfo,    //test#13
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoInfo,   //test#13
BYTE btProhibitedBox,                                           //test#13
USHORT &usFedNoteCondition,
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
)
{

    THISMODULE(__FUNCTION__);

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pNumStackedToPerCass);
    VERIFYPOINTNOTEMPTY(pNumPerCassFed);
    VERIFYPOINTNOTEMPTY(pMaintenanceInfo);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
//test#13     char szCmdData[56 + 1] = {0};
    char szCmdData[92 + 1] = {0};                       //test#13
    if (iValidateMode == VALIDATION_MODE_REAL)
    {
 //test#13       MOSAICCMND(szCmdData, iCmdLen, 0x1000);
        MOSAICCMND(szCmdData, iCmdLen, 0x1210);                 //test#13
    }
    else
    {
 //test#13       MOSAICCMND(szCmdData, iCmdLen, 0x1900);
        MOSAICCMND(szCmdData, iCmdLen, 0x1A10);                 //test#13
    }
    iCmdLen += 6;

 //test#13   char szCmdParam[50 + 1] = {0};
    char szCmdParam[86 + 1] = {0};              //test#13
    //2.Deposit Limit(6 bytes) 可变包
    if (iNumDepositLimit > 0)
    {
        if (iNumDepositLimit > 10000)
        {
            iNumDepositLimit = 10000;
        }
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x30;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x04;
        szCmdParam[4] = static_cast<char>((iNumDepositLimit >> 8) & 0xFF);
        szCmdParam[5] = static_cast<char>(iNumDepositLimit & 0xFF);
        memcpy(szCmdData + iCmdLen, szCmdParam, 6);
        iCmdLen += 6;
    }

    //3.Acceptable Denomination Code(22 bytes) 可变包，如果参数为空则不添加该数据包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (bArryAcceptDeno != nullptr)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x17;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x14;

        for (int i = 0; i < MAX_DENOMINATION_NUM; i++)
        {
            if (bArryAcceptDeno[i])
            {
                szCmdParam[6 + (i / 8)] |= (0x80 >> (i % 8));
            }
        }

        memcpy(szCmdData + iCmdLen, szCmdParam, 22);
        iCmdLen += 22;
    }

    //4.Room Exclusion(36 bytes)    //test#13  start
        memset(szCmdParam, 0, sizeof(szCmdParam));
        if (btProhibitedBox != 0)
        {
            szCmdParam[0] = 0x05;
            szCmdParam[1] = 0x50;
            szCmdParam[2] = 0x00;
            szCmdParam[3] = 0x22;

            if (btProhibitedBox & 0x01) //Cass1
                szCmdParam[7] = 0x01;
            if (btProhibitedBox & 0x02) //Cass2
                szCmdParam[8] = 0x01;
            if (btProhibitedBox & 0x04) //Cass3
                szCmdParam[9] = 0x01;
            if (btProhibitedBox & 0x08) //Cass4
                szCmdParam[10] = 0x01;
            if (btProhibitedBox & 0x10) //Cass5
                szCmdParam[11] = 0x01;
            if (btProhibitedBox & 0x20) //URJB
                szCmdParam[27] = 0x01;

            memcpy(szCmdData + iCmdLen, szCmdParam, 36);
            iCmdLen += 36;
        }
//test#13  end
    //7.BV Depensent Mode(6 bytes)可变包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (stBVDependentMode.bEnableBVModeSetting)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x2A;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x04;

        switch (stBVDependentMode.iFullImageMode)
        {
        case FULLIMAGE_ALL_NOTES:
            szCmdParam[4] = static_cast<char>( 0x80 );
            break;
        case FULLIMAGE_REJECTE_NOTES:
            szCmdParam[4] = 0x02;
            break;
        case FULLIMAGE_NO_RECORDS:
            szCmdParam[4] = 0x00;
            break;
        default:
            szCmdParam[4] = 0x00;
        }

/* //test#13       switch (stBVDependentMode.iRejcetNoteNOSN)
        {
        case ACTION_REJECT_NOTES:
            szCmdParam[5] = 0x01;
            break;
        case ACTION_NO_REJECT_NOTES:
            szCmdParam[5] = 0x00;
            break;
        default:
            szCmdParam[5] = 0x00;
        }*/  //test#13

        memcpy(szCmdData + iCmdLen, szCmdParam, 6);
        iCmdLen += 6;
    }

 //test#13 start
    //6.Amount Limit(8 bytes) 可变包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (iNumAmountLimit > 0 && iNumAmountLimit < 0x7FFF)
    {
        //AmountLimit = [Digit] * 10^[Power Index]
        szCmdParam[0] = 0x05;
        szCmdParam[1] = static_cast<char>(0x87);
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x06;
        szCmdParam[4] = static_cast<char>((iNumAmountLimit >> 8) & 0xFF);
        szCmdParam[5] = static_cast<char>(iNumAmountLimit & 0xFF);      //Digit
        szCmdParam[6] = iPowerIndex;    //Power Index
        szCmdParam[7] = 0x00;
        memcpy(szCmdData + iCmdLen, szCmdParam, 8);
        iCmdLen += 8;
    }
 //test#13 end
    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 4408, HT_RESP_TIMEOUT_1200, ThisModule);

    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 8;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x1200:  											//test#13 start
    case 0x1A00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x1202:
    case 0x1A02:
 //test by guojy       iRet = ERR_UR_WARN;
        iRet = ERR_UR_SUCCESS;
        break;
    case 0x1203:
    case 0x1A03:
 //       iRet = ERR_UR_WARN_NOTESREJECT;
//test by guojy        iRet = ERR_UR_WARN;
        iRet = ERR_UR_SUCCESS;
        break;
    case 0x1206:
    case 0x1A06:
 //       iRet = ERR_UR_WARN_STOPFEED;
//test by guojy        iRet = ERR_UR_WARN;
        iRet = ERR_UR_SUCCESS;
        break;
    case 0x1207:
    case 0x1A07:
//        iRet = ERR_UR_WARN_TOOMANYRJ;
//test by guojy        iRet = ERR_UR_WARN;
        iRet = ERR_UR_SUCCESS;
        break;
    case 0x1208:
    case 0x1A08:
//        iRet = ERR_UR_WARN_FULL;
 //test by guojy       iRet = ERR_UR_WARN;
        iRet = ERR_UR_SUCCESS;
        break;
    case 0x1250:
    case 0x1A50:
//        iRet = ERR_UR_WARN_BVMEMORYFULL;
//test by guojy        iRet = ERR_UR_WARN;
        iRet = ERR_UR_SUCCESS;
        break;
    case 0x1240:
    case 0x1A40:
//        iRet = ERR_UR_WARN_NONOTES;
//test by guojy        iRet = ERR_UR_WARN;
        iRet = ERR_UR_SUCCESS;
        break;
    case 0x1280:
    case 0x1A80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x12FF:
    case 0x1AFF:												//test#13 end
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //4.Number of Total Stacked Notes(64 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMTOTALSTACKEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    //iTotalCashInURJB = (int)(((UCHAR)szResp[iDataPos + 50] << 8) + (UCHAR)szResp[iDataPos + 51]);
    iTotalCashInURJB = MAKEWORD(szResp[iDataPos + 51], szResp[iDataPos + 50]);

    //5.Number of Stacked Notes(64 bytes)
    int iRetParse = ParseRespStackedNotes(
                    szResp,
                    mapPacket,
                    iNumStackedToCS,
                    iNumStackedToESC,
                    pNumStackedToPerCass,
                    ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //6.Number of Fed Notes(36 bytes)
    iRetParse = ParseRespFedNotes(szResp, mapPacket, iNumCSFed, iNumESCFed, pNumPerCassFed, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //7.Number of Reject Notes(6 bytes)可变包
    nRejectNotes = 0;
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMREJNOTES, ThisModule, FALSE);
    if (iDataPos > 0)
    {
        //nRejectNotes = (int)(((UCHAR)szResp[iDataPos] << 8) + (UCHAR)szResp[iDataPos + 1]);
        nRejectNotes = MAKEWORD(szResp[iDataPos + 1], szResp[iDataPos]);
    }

    //8.Number of Stacked Notes per Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, STACKE_NOTES_DENO_INFO_TYPE, stStackeNotesDenoInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //10.Number of Unfit Notes per Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, UNFIT_NOTES_DENO_INFO_TYPE, stStackeUnfitNotesDenoInfo, ThisModule); //test#13
    if (iRetParse < 0)                      //test#13
    {                                       //test#13
        return iRetParse;                   //test#13
    }                                       //test#13

    //11.Number of Rejected Notes per Denomination and Destination(68,132,260,516 bytes) 可变包
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, REJECT_DEST_NOTES_DENO_INFO_TYPE, stStackeRejectNotesDenoInfo, ThisModule); //test#13

    //13.Fed Note Condition(6 bytes) 可变包
    usFedNoteCondition = 0;
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_FEDNOTECONDITION, ThisModule, FALSE);
    if (iDataPos > 0)
    {
        usFedNoteCondition = szResp[iDataPos + 1] & 0x00FF;
    }

    //14.Maintenance Information(4100 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MAINTENINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memset(pMaintenanceInfo, 0, sizeof(char)*MAINTENANCE_INFO_LENGTH);
    memcpy(pMaintenanceInfo, szResp + iDataPos, MAINTENANCE_INFO_LENGTH);

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}


//功能1：点钞存入的钞票，将钞票从CS钞口送入ESC暂存箱，或者将钞票回收至URJB(通过SET_UNIT_INFO命令设置回收位置或者参数设置)
//功能2：点钞存入的钞票，将钞票从CS钞票回收至ESC暂存箱
//输入：bIgnoreESC: 是否忽略ESC检测，TRUE：忽略，FALSE：不忽略，点钞前如果ESC有钞命令返回失败
//输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
//输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为1时有效，否则忽略
//输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
//输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
//输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
//输出：pNumStackedToPerCass: 表示各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
//输出：iNumCSFed:  CS点钞数
//输出：iNumESCFed: ESC点钞数
//输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
//输出：nRejectNotes: 表示退回到CS的钞票总数
//输出：stStackeNotesDenoInfo：每种面额钞票的流向、张数信息
//输出：usFedNoteCondition: 钞票传输情况取值位运算，FED_NOTE_CONDITION 包含多种情况 CONDITION_SKEW & CONDITION_SHORT_NOTE
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：180
int CUR2Drv::CashCountRetract(
VALIDATION_MODE iValidateMode,
const ST_BV_DEPENDENT_MODE stBVDependentMode,
int &iTotalCashInURJB,
int &iNumStackedToCS,
int &iNumStackedToESC,
int pNumStackedToPerCass[MAX_CASSETTE_NUM],
int &iNumCSFed,
int &iNumESCFed,
int pNumPerCassFed[MAX_CASSETTE_NUM],
int &nRejectNotes,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
USHORT &usFedNoteCondition,
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH],
BOOL bIgnoreESC
)
{
    THISMODULE(__FUNCTION__);

    //校验参数合法性
    //VERIFYPOINTNOTEMPTY(pNumStackedToPerCass);
    //VERIFYPOINTNOTEMPTY(pNumPerCassFed);
    //VERIFYPOINTNOTEMPTY(pMaintenanceInfo);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 chars)
    char szCmdData[56 + 1] = {0};
    if (iValidateMode == VALIDATION_MODE_REAL)
    {
        if (bIgnoreESC)
        {
            MOSAICCMND(szCmdData, iCmdLen, 0x10A1);
        }
        else
        {
            MOSAICCMND(szCmdData, iCmdLen, 0x10A0);
        }
    }
    else
    {
        if (bIgnoreESC)
        {
            MOSAICCMND(szCmdData, iCmdLen, 0x19A1);
        }
        else
        {
            MOSAICCMND(szCmdData, iCmdLen, 0x19A0);
        }
    }
    iCmdLen += 6;

    char szCmdParam[64 + 1] = {0};

    //7.BV Depensent Mode(6 bytes)可变包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (stBVDependentMode.bEnableBVModeSetting)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x2A;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x04;

        switch (stBVDependentMode.iFullImageMode)
        {
        case FULLIMAGE_ALL_NOTES:
            szCmdParam[4] = static_cast<char>( 0x80 );
            break;
        case FULLIMAGE_REJECTE_NOTES:
            szCmdParam[4] = 0x02;
            break;
        case FULLIMAGE_NO_RECORDS:
            szCmdParam[4] = 0x00;
            break;
        default:
            szCmdParam[4] = 0x00;
        }

        switch (stBVDependentMode.iRejcetNoteNOSN)
        {
        case ACTION_REJECT_NOTES:
            szCmdParam[5] = 0x01;
            break;
        case ACTION_NO_REJECT_NOTES:
            szCmdParam[5] = 0x00;
            break;
        default:
            szCmdParam[5] = 0x00;
        }

        memcpy(szCmdData + iCmdLen, szCmdParam, 6);
        iCmdLen += 6;
    }

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 4402, HT_RESP_TIMEOUT_180, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 8;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x1000:
    case 0x1900:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x1002:
    case 0x1902:
    case 0x1003:
    case 0x1903:
    case 0x1004:
    case 0x1904:
    case 0x1006:
    case 0x1906:
    case 0x1007:
    case 0x1907:
    case 0x1010:
    case 0x1910:
    case 0x1020:
    case 0x1920:
    case 0x1031:
    case 0x1931:
    case 0x1032:
    case 0x1932:
    case 0x1033:
    case 0x1933:
    case 0x1040:
    case 0x1940:
    case 0x1050:
    case 0x1950:
        iRet = ERR_UR_WARN;
        break;
    case 0x1080:
    case 0x1980:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x10FF:
    case 0x19FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //4.Number of Total Stacked Notes(64 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMTOTALSTACKEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    //iTotalCashInURJB = (int)(((UCHAR)szResp[iDataPos + 50] << 8) + (UCHAR)szResp[iDataPos + 51]);
    iTotalCashInURJB = MAKEWORD(szResp[iDataPos + 51], szResp[iDataPos + 50]);

    //5.Number of Stacked Notes(64 bytes)
    int iRetParse = ParseRespStackedNotes(
                    szResp,
                    mapPacket,
                    iNumStackedToCS,
                    iNumStackedToESC,
                    pNumStackedToPerCass,
                    ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //6.Number of Fed Notes(36 bytes)
    iRetParse = ParseRespFedNotes(szResp, mapPacket, iNumCSFed, iNumESCFed, pNumPerCassFed, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //7.Number of Reject Notes(6 bytes)可变包
    nRejectNotes = 0;
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMREJNOTES, ThisModule, FALSE);
    if (iDataPos > 0)
    {
        //nRejectNotes = (int)(((UCHAR)szResp[iDataPos + 4] << 8) + (UCHAR)szResp[iDataPos + 5]);
        nRejectNotes = MAKEWORD(szResp[iDataPos + 5], szResp[iDataPos + 4]);
    }

    //8.Number of Stacked Notes per Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, STACKE_NOTES_DENO_INFO_TYPE, stStackeNotesDenoInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //13.Fed Note Condition(6 bytes) 可变包
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_FEDNOTECONDITION, ThisModule, FALSE);
    if (iDataPos > 0)
    {
        usFedNoteCondition = (szResp[iDataPos + 1] & 0x00FF);
    }

    //14.Maintenance Information(4100 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MAINTENINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memset(pMaintenanceInfo, 0, sizeof(char)*MAINTENANCE_INFO_LENGTH);
    memcpy(pMaintenanceInfo, szResp + iDataPos, MAINTENANCE_INFO_LENGTH);

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}


//功能：存入钞票，将钞票从ESC暂存箱送入到指定的各钞箱
//输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
//输入：iDestinationReject: 指定回收位置传入DESTINATION_REJCET_DEFAULT则使用SET_UNIT_INFO中设置的值
//输入：btProhibitedBox: 表示禁止出钞的钱箱。1字节，0位~4位分别表示钱箱1~6,其余位值恒为0
//      位值为1表示禁止从对应的钱箱存钞
//输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为true时有效，否则忽略
//输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
//输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
//输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
//输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
//输出：iNumCSFed:  CS点钞数
//输出：iNumESCFed: ESC点钞数
//输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
//输出：stStackeNotesDenoInfo：每种面额钞票的流向、张数信息
//输出：stStackeUnfitNotesDenoInfo：不适合流通的钞票面额、流向、张数信息
//输出：stStackeRejectNotesDenoInfo：回收的钞票面额、流向、张数信息
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：180
int CUR2Drv::StoreMoney(
VALIDATION_MODE iValidateMode,
BOOL  bCheckCS,
DESTINATION_REJCET iDestinationReject,
char btProhibitedBox,
const ST_BV_DEPENDENT_MODE stBVDependentMode,
int &iTotalCashInURJB,
int &iNumStackedToCS,
int &iNumStackedToESC,
int pNumStackedToPerCass[MAX_CASSETTE_NUM],
int &iNumCSFed,
int &iNumESCFed,
int pNumPerCassFed[MAX_CASSETTE_NUM],
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeUnfitNotesDenoInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoInfo,
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
)
{
    const char *ThisModule = "StoreMoney";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pNumStackedToPerCass);
    VERIFYPOINTNOTEMPTY(pNumPerCassFed);
    VERIFYPOINTNOTEMPTY(pMaintenanceInfo);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[56 + 1] = {0};
    if (iValidateMode == VALIDATION_MODE_REAL)
    {
        if (bCheckCS)
        {
            MOSAICCMND(szCmdData, iCmdLen, 0x3000);
        }
        else
        {
            MOSAICCMND(szCmdData, iCmdLen, 0x3001);
        }

    }
    else
    {
        if (bCheckCS)
        {
            MOSAICCMND(szCmdData, iCmdLen, 0x3F00);
        }
        else
        {
            MOSAICCMND(szCmdData, iCmdLen, 0x3F01);
        }
    }
    iCmdLen += 6;

    char szCmdParam[64 + 1] = {0};
    //2.Room Exclusion(36 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (btProhibitedBox != 0)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x50;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x22;

        if (btProhibitedBox & 0x01) //Cass1
            szCmdParam[7] = 0x01;
        if (btProhibitedBox & 0x02) //Cass2
            szCmdParam[8] = 0x01;
        if (btProhibitedBox & 0x04) //Cass3
            szCmdParam[9] = 0x01;
        if (btProhibitedBox & 0x08) //Cass4
            szCmdParam[10] = 0x01;
        if (btProhibitedBox & 0x10) //Cass5
            szCmdParam[11] = 0x01;
        if (btProhibitedBox & 0x20) //URJB
            szCmdParam[27] = 0x01;

        memcpy(szCmdData + iCmdLen, szCmdParam, 36);
        iCmdLen += 36;
    }

    //3.Destination for Reject(6 bytes) 可变包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (iDestinationReject != DESTINATION_REJCET_DEFAULT)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x51;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x04;
        szCmdParam[4] = 0x00;
        switch (iDestinationReject)
        {
        case DESTINATION_REJCET_CS:
            szCmdParam[5] = 0x01;
            break;
        case DESTINATION_REJCET_DISPENSE:
            szCmdParam[5] = 0x02;
            break;
        case DESTINATION_REJCET_DEPOSIT:
            szCmdParam[5] = 0x03;
            break;
        default:
            szCmdParam[5] = 0x01;
        }

        memcpy(szCmdData + iCmdLen, szCmdParam, 6);
        iCmdLen += 6;
    }

    //4.BV Depensent Mode(6 bytes)可变包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (stBVDependentMode.bEnableBVModeSetting)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x2A;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x04;

        switch (stBVDependentMode.iFullImageMode)
        {
        case FULLIMAGE_ALL_NOTES:
            szCmdParam[4] = static_cast<char>( 0x80 );
            break;//Only one of the bit is valid to specify ?1?.
        case FULLIMAGE_REJECTE_NOTES:
            szCmdParam[4] = 0x02;
            break;
        case FULLIMAGE_NO_RECORDS:
            szCmdParam[4] = 0x00;
            break;
        default:
            szCmdParam[4] = 0x00;
            break;
        }

        switch (stBVDependentMode.iRejcetNoteNOSN)
        {
        case ACTION_REJECT_NOTES:
            szCmdParam[5] = 0x01;
            break;
        case ACTION_NO_REJECT_NOTES:
            szCmdParam[5] = 0x00;
            break;
        default:
            szCmdParam[5] = 0x00;
        }

        memcpy(szCmdData + iCmdLen, szCmdParam, 6);
        iCmdLen += 6;
    }

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 4538,  HT_RESP_TIMEOUT_180, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 9;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x3000:
    case 0x3F00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x3001:
    case 0x3F01:
        iRet = ERR_UR_WARN;
        break;
    case 0x3080:
    case 0x3F80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x30FF:
    case 0x3FFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //4.Number of Total Stacked Notes(64 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMTOTALSTACKEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    iTotalCashInURJB = MAKEWORD(szResp[iDataPos + 51], szResp[iDataPos + 50]);

    //5.Number of Stacked Notes(64 bytes)
    int iRetParse = ParseRespStackedNotes(
                    szResp,
                    mapPacket,
                    iNumStackedToCS,
                    iNumStackedToESC,
                    pNumStackedToPerCass,
                    ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //6.Number of Fed Notes(36 bytes)
    iRetParse = ParseRespFedNotes(szResp, mapPacket, iNumCSFed, iNumESCFed, pNumPerCassFed, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //7.Number of Stacked Notes per Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, STACKE_NOTES_DENO_INFO_TYPE, stStackeNotesDenoInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //10.Number of Unfit Notes per Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, UNFIT_NOTES_DENO_INFO_TYPE, stStackeUnfitNotesDenoInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //11.Number of Rejected Notes per Denomination and Destination(68,132,260,516 bytes) 可变包
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, REJECT_DEST_NOTES_DENO_INFO_TYPE, stStackeRejectNotesDenoInfo, ThisModule);

    //14.Maintenance Information(4100 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MAINTENINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memset(pMaintenanceInfo, 0, sizeof(char)*MAINTENANCE_INFO_LENGTH);
    memcpy(pMaintenanceInfo, szResp + iDataPos, MAINTENANCE_INFO_LENGTH);

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}


//功能：取消存款：将暂存箱中的钞票送至出钞口，将钞票返回给用户
//输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
//输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
//输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
//输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
//输出：iNumCSFed:  CS点钞数
//输出：iNumESCFed: ESC点钞数
//输出：pNumPerCassFed: 表示各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
//输出：stStackeNotesDenoInf: 表示每种面额钞票的流向、张数信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：240
int CUR2Drv::CashRollback(
VALIDATION_MODE iValidateMode,
int &iTotalCashInURJB,
int &iNumStackedToCS,
int &iNumStackedToESC,
int pNumStackedToPerCass[MAX_CASSETTE_NUM],
int &iNumCSFed,
int &iNumESCFed,
int pNumPerCassFed[MAX_CASSETTE_NUM],
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
)
{
    const char *ThisModule = "CashRollback";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pNumStackedToPerCass);
    VERIFYPOINTNOTEMPTY(pNumPerCassFed);
    VERIFYPOINTNOTEMPTY(pMaintenanceInfo);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[14 + 1] = {0};
    if (iValidateMode == VALIDATION_MODE_REAL)
    {
        MOSAICCMND(szCmdData, iCmdLen, 0x3100);
    }
    else
    {
        MOSAICCMND(szCmdData, iCmdLen, 0x3900);
    }
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 4402, HT_RESP_TIMEOUT_240, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }


    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 8;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x3100:
    case 0x3900:
        iRet = ERR_UR_SUCCESS;
        break; //Normal End
    case 0x3140:
    case 0x3940:
        iRet = ERR_UR_WARN;
        break; //Warning End
    case 0x3180:
    case 0x3980:
        iRet = ERR_UR_HARDWARE_ERROR;
        break; //Abnormal End
    case 0x31FF:
    case 0x39FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break; //Invalid Command
    default:
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //4.Number of Total Stacked Notes(64 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMTOTALSTACKEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    iTotalCashInURJB = MAKEWORD(szResp[iDataPos + 51], szResp[iDataPos + 50]);

    //5.Number of Stacked Notes(64 bytes)
    int iRetParse = ParseRespStackedNotes(
                    szResp,
                    mapPacket,
                    iNumStackedToCS,
                    iNumStackedToESC,
                    pNumStackedToPerCass,
                    ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //6.Number of Fed Notes(36 bytes)
    iRetParse = ParseRespFedNotes(szResp, mapPacket, iNumCSFed, iNumESCFed, pNumPerCassFed, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //7.Number of Stacked Notes per Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, STACKE_NOTES_DENO_INFO_TYPE, stStackeNotesDenoInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //14.Maintenance Information(4100 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MAINTENINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memset(pMaintenanceInfo, 0, sizeof(char)*MAINTENANCE_INFO_LENGTH);
    memcpy(pMaintenanceInfo, szResp + iDataPos, MAINTENANCE_INFO_LENGTH);

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能1：指定面额挖钞 pArrDispDeno不为NULL使用
//功能2：指定钞箱挖钞 pArrDispRoom不为NULL使用，冲突时优先使用该功能
//输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
//输入：btProhibitedBox: 表示禁止出钞的钱箱。1字节，0位~4位分别表示钱箱1~6,其余位值恒为0
//      位值为1表示禁止从对应的钱箱存钞
//输入：pArrDispDeno:  结构数组,表示每种面额的钞票出多少张,数组大小为10 NULL不使用该值
//输入：pArrDispRoom:  结构数组,表示指定钞箱钞票出多少张,数组大小为10   NULL不使用该值
//输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为true时有效，否则忽略
//输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
//输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
//输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
//输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
//输出：iNumCSFed:  CS点钞数
//输出：iNumESCFed: ESC点钞数
//输出：pNumPerCassFed: 表示挖钞时各钞箱点钞张数,钞箱1~6的各钞箱点钞数,整型数组,数组大小为6
//输出：pNumDispensedForPerCass: 表示挖钞时各钞箱挖钞张数，钞箱1~6的各钞箱挖钞数,整型数组,数组大小为6
//输出：stStackeNotesDenoInf: 表示每种面额钞票的流向、张数信息
//输出：stStackeRejectNotesDenoSourInfo: 表示回收钞的面额的来源信息流向、张数
//输出：stStackeRejectNotesDenoBVInfo:   表示回收钞的面额的通过BV的流向、张数信息
//输出：stStackeRejectNotesDenoDestInfo: 表示回收钞的面额的目的信息了流向、张数
//输出：stDispMissfeedRoom: 表示发生挖钞失败的钱箱以及原因
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：180
int CUR2Drv::Dispense(
VALIDATION_MODE iValidateMode,
char btProhibitedBox,
const ST_DISP_DENO pArrDispDeno[MAX_DISP_DENO_NUM],
const ST_DISP_ROOM pArrDispRoom[MAX_DISP_ROOM_NUM],
const ST_BV_DEPENDENT_MODE stBVDependentMode,
int &iTotalCashInURJB,
int &iNumStackedToCS,
int &iNumStackedToESC,
int pNumStackedToPerCass[MAX_CASSETTE_NUM],
int &iNumCSFed,
int &iNumESCFed,
int pNumPerCassFed[MAX_CASSETTE_NUM],
int pNumDispensedForPerCass[MAX_CASSETTE_NUM],
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoSourInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoBVInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoDestInfo,
ST_DISP_MISSFEED_ROOM &stDispMissfeedRoom,
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
)
{
    const char *ThisModule = "Dispense";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pNumStackedToPerCass);
    VERIFYPOINTNOTEMPTY(pNumPerCassFed);
    VERIFYPOINTNOTEMPTY(pNumDispensedForPerCass);
    VERIFYPOINTNOTEMPTY(pMaintenanceInfo);
    if ((pArrDispDeno == nullptr && pArrDispRoom == nullptr)
        || (pArrDispDeno != nullptr && pArrDispRoom != nullptr))
    {
        Log(ThisModule, ERR_UR_PARAM, "参数ArrDispDeno和ArrDispRoom都为NULL或都不为NULL");
        return ERR_UR_PARAM;
    }

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[102 + 1] = {0};
    if (iValidateMode == VALIDATION_MODE_REAL)
    {
//test#13        if (pArrDispDeno)
//test#13          {
//test#13              MOSAICCMND(szCmdData, iCmdLen, 0x2001);
//test#13          }
//test#13          else
//test#13          {
            MOSAICCMND(szCmdData, iCmdLen, 0x2011);
//test#13          }
    }
    else
    {
 //test#13         if (pArrDispDeno)
 //test#13         {
 //test#13             MOSAICCMND(szCmdData, iCmdLen, 0x2F01);
 //test#13         }
//test#13          else
 //test#13         {
            MOSAICCMND(szCmdData, iCmdLen, 0x2F11);
 //test#13         }
    }
    iCmdLen += 6;

    //2.Number of Dispense notes per room(52 bytes)
    UCHAR szCmdParam[64] = {0};
    int iAvailableDispData = 0; //有效的配钞数
    int i = 0;
    if (pArrDispRoom != nullptr)
    {
        memset(szCmdParam, 0, sizeof(szCmdParam));
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x40;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x32;

        for (i = 0, iAvailableDispData = 0; i < MAX_DISP_ROOM_NUM; i++)
        {

            if (pArrDispRoom[i].iCashNumber < 0 || pArrDispRoom[i].iCashNumber > 200)
            {
                Log(ThisModule, ERR_UR_PARAM, "pArrDispRoom[%d].iCashNumber:%d错误(应为:>=0,<=200)", i, pArrDispRoom[i].iCashNumber);
                continue;
            }

            if (pArrDispRoom[i].iCashNumber == 0)
            {
                continue;
            }

            switch (pArrDispRoom[i].iCassID)
            {
            case ID_ROOM_1:
            case ID_ROOM_2:
            case ID_ROOM_3:
            case ID_ROOM_5:
                {
                    szCmdParam[4 + iAvailableDispData * 4] = pArrDispRoom[i].iCassID;
                    szCmdParam[6 + iAvailableDispData * 4] = (pArrDispRoom[i].iCashNumber >> 8) & 0xFF;
                    szCmdParam[7 + iAvailableDispData * 4] = pArrDispRoom[i].iCashNumber & 0xFF;
                    iAvailableDispData++;
                    break;
                }
            default:
                {
                    return ERR_UR_PARAM;
                }
            }
        }

        if (iAvailableDispData == 0)
        {
            Log(ThisModule, ERR_UR_PARAM, "pArrDispRoom为无效数据");
            return ERR_UR_PARAM;
        }

        memcpy(szCmdData + iCmdLen, szCmdParam, 52);
        iCmdLen += 52;
    }
 /*//test#13 start   else if (pArrDispDeno != nullptr) //3.Number of Dispense notes per Denomination(52 bytes)
    {
        memset(szCmdParam, 0, sizeof(szCmdParam));
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x41;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x32;

        for (i = 0, iAvailableDispData = 0; i < MAX_DISP_DENO_NUM; i++)
        {
            if (pArrDispDeno[i].iCashNumber < 0 || pArrDispDeno[i].iCashNumber > 200)
            {
                Log(ThisModule, ERR_UR_PARAM, "pArrDispDeno[%d].iCashNumber:%d错误(应为:>=0,<=200)", i, pArrDispDeno[i].iCashNumber);
				continue; 
			}
			
			switch(pArrDispDeno[i].iDenoCode)
			{
            case DENOMINATION_CODE_50_B:
            case DENOMINATION_CODE_50_C:
            case DENOMINATION_CODE_50_D:
            case DENOMINATION_CODE_100_B:
            case DENOMINATION_CODE_100_C:
            case DENOMINATION_CODE_100_D:
            case DENOMINATION_CODE_10_B:
            case DENOMINATION_CODE_10_C:
            case DENOMINATION_CODE_10_D:
                {
                    szCmdParam[4 + iAvailableDispData * 4] = pArrDispDeno[i].iDenoCode;
                    szCmdParam[6 + iAvailableDispData * 4] = (pArrDispDeno[i].iCashNumber >> 8) & 0xFF;
                    szCmdParam[7 + iAvailableDispData * 4] = pArrDispDeno[i].iCashNumber & 0xFF;
                    iAvailableDispData++;
                    break;
                }
            default:
                break;
            }
        }

        if (iAvailableDispData == 0)
        {
            Log(ThisModule, ERR_UR_PARAM, "pArrDispDeno为无效数据");
            return ERR_UR_PARAM;
        }

        memcpy(szCmdData + iCmdLen, szCmdParam, 52);
        iCmdLen += 52;
    }

    //4.Room Exclusion(36 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (btProhibitedBox != 0)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x50;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x22;

        if (btProhibitedBox & 0x01) //Cass1
            szCmdParam[7] = 0x01;
        if (btProhibitedBox & 0x02) //Cass2
            szCmdParam[8] = 0x01;
        if (btProhibitedBox & 0x04) //Cass3
            szCmdParam[9] = 0x01;
        if (btProhibitedBox & 0x08) //Cass4
            szCmdParam[10] = 0x01;
        if (btProhibitedBox & 0x10) //Cass5
            szCmdParam[11] = 0x01;
        if (btProhibitedBox & 0x20) //URJB
            szCmdParam[27] = 0x01;

        memcpy(szCmdData + iCmdLen, szCmdParam, 36);
        iCmdLen += 36;
    }*/ //test#13 end

    //5.BV Dependent Mode(6 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (stBVDependentMode.bEnableBVModeSetting)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x2A;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x04;

        switch (stBVDependentMode.iFullImageMode)
        {
        case FULLIMAGE_ALL_NOTES:
            szCmdParam[4] = 0x80;
            break;
        case FULLIMAGE_REJECTE_NOTES:
            szCmdParam[4] = 0x02;
            break;
        case FULLIMAGE_NO_RECORDS:
            szCmdParam[4] = 0x00;
            break;
        default:
            szCmdParam[4] = 0x00;
            break;
        }

 /*       switch (stBVDependentMode.iRejcetNoteNOSN)
        {
        case ACTION_REJECT_NOTES:
            szCmdParam[5] = 0x01;
            break;
        case ACTION_NO_REJECT_NOTES:
            szCmdParam[5] = 0x00;
            break;
        default:
            szCmdParam[5] = 0x00;
        }*/ //test#13

        memcpy(szCmdData + iCmdLen, szCmdParam, 6);
        iCmdLen += 6;
    }

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 4640, HT_RESP_TIMEOUT_1200, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 11;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x2000:
    case 0x2F00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    //case 0x2003:
    //case 0x2F03: iRet = ERR_UR_SUCCESS;        break;  //End with Warning(Notes rejected)
//test#13    case 0x2040:
//test#13    case 0x2F40:
//test#13         iRet = ERR_UR_HARDWARE_ERROR;
//test#13        break;  //Warn
    case 0x2050:
    case 0x2F50:
        iRet = ERR_UR_WARN;
        break;  //Warn
    case 0x2071:
    case 0x2F71:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Warn
    case 0x2080:
    case 0x2F80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
//test#13     case 0x2081:
//test#13     case 0x2F81:
//test#13         iRet = ERR_UR_HARDWARE_ERROR;
//test#13         break;  //Abnormal End
    case 0x20FF:
    case 0x2FFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //4.Number of Total Stacked Notes(64 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMTOTALSTACKEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    iTotalCashInURJB = MAKEWORD(szResp[iDataPos + 51], szResp[iDataPos + 50]);

    //5.Number of Stacked Notes(64 bytes)
    int iRetParse = ParseRespStackedNotes(
                    szResp,
                    mapPacket,
                    iNumStackedToCS,
                    iNumStackedToESC,
                    pNumStackedToPerCass,
                    ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //6.Number of Fed Notes(36 bytes)
    iRetParse = ParseRespFedNotes(szResp, mapPacket, iNumCSFed, iNumESCFed, pNumPerCassFed, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //7.Number of Dispensed notes per room(28 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMDISPNOTESROOM, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    memset(pNumDispensedForPerCass, 0, sizeof(pNumDispensedForPerCass[0]) * MAX_CASSETTE_NUM);
    for (i = 0; i < MAX_CASSETTE_NUM - 1; i++)
    {
        //pNumDispensedForPerCass[i] = (int)(((UCHAR)szResp[iDataPos + i * 2] << 8) + (UCHAR)szResp[iDataPos + 1 + i * 2]);
        pNumDispensedForPerCass[i] = MAKEWORD(szResp[iDataPos + 1 + i * 2], szResp[iDataPos + i * 2]);
    }

    //8.Number of Stacked Notes per Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, STACKE_NOTES_DENO_INFO_TYPE, stStackeNotesDenoInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //9.Number of Reject Notes per Denomination and Source(68 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, REJECT_SOURCE_NOTES_DENO_INFO_TYPE, stStackeRejectNotesDenoSourInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //10.Number of Reject Notes per BV Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, REJECT_BV_NOTES_DENO_INFO_TYPE, stStackeRejectNotesDenoBVInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //13.Number of Rejected Notes per Denomination and Destination(68,132,260,516 bytes) 可变包
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, REJECT_DEST_NOTES_DENO_INFO_TYPE, stStackeRejectNotesDenoDestInfo, ThisModule);

    //11.Miss-feed Room(6 bytes) 可变包
    memset(&stDispMissfeedRoom, 0, sizeof(stDispMissfeedRoom));
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MISSFEEDROOM, ThisModule, FALSE);
    if (iDataPos > 0)
    {
        if (szResp[iDataPos] & 0x80)
            stDispMissfeedRoom.bRoomMissFeed = TRUE;
        if (szResp[iDataPos] & 0x40)
            stDispMissfeedRoom.bRoomEmpty = TRUE;

        //stDispMissfeedRoom.bArryRoom[0] = FALSE; //AB箱固定值

        if (szResp[iDataPos] & 0x10)
            stDispMissfeedRoom.bArryRoom[0] = TRUE;
        if (szResp[iDataPos] & 0x08)
            stDispMissfeedRoom.bArryRoom[1] = TRUE;
        if (szResp[iDataPos] & 0x04)
            stDispMissfeedRoom.bArryRoom[2] = TRUE;

        //if (szResp[iDataPos] & 0x02)
            stDispMissfeedRoom.bArryRoom[3] = FALSE; // 此为AB箱, 设置为固定值

        if (szResp[iDataPos] & 0x01)
            stDispMissfeedRoom.bArryRoom[4] = TRUE;

        if (szResp[iDataPos + 1] & 0x80)
            stDispMissfeedRoom.bRoomContinuousRejects = TRUE;
        if (szResp[iDataPos + 1] & 0x40)
            stDispMissfeedRoom.bRoomTooManyRejects = TRUE;
    }

    //14.Maintenance Information(4100 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MAINTENINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memset(pMaintenanceInfo, 0, sizeof(char)*MAINTENANCE_INFO_LENGTH);
    memcpy(pMaintenanceInfo, szResp + iDataPos, MAINTENANCE_INFO_LENGTH);

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}


//功能：回收挖钞后残留在ESC中的钞票，并将钞票送至拒钞箱
//输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
//输入：btProhibitedBox: 表示禁止出钞的钱箱。1字节，0位~4位分别表示钱箱1~6,其余位值恒为0
//      位值为1表示禁止从对应的钱箱存钞
//输入：iDestinationReject: 指定回收位置传入DESTINATION_REJCET_DEFAULT则使用SET_UNIT_INFO中设置的值
//输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为true时有效，否则忽略
//输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
//输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
//输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
//输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
//输出：iNumCSFed:  CS点钞数
//输出：iNumESCFed: ESC点钞数
//输出：pNumPerCassFed: 表示回收时回收至各钞箱的钞票张数,钞箱1~6的各钞箱钞票数,整型数组,数组大小为6
//输出：stStackeNotesDenoInf: 表示每种面额钞票的流向、张数信息
//输出：stStackeUnfitNotesDenoInfo:   表示不适合流通的钞票面额、流向、张数信息
//输出：stStackeRejectNotesDenoInfo:  表示回收钞的面额的目的信息了流向、张数
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：180
int CUR2Drv::RetractESCForDispenseRejcet(
VALIDATION_MODE iValidateMode,
char btProhibitedBox,
DESTINATION_REJCET iDestinationReject,
const ST_BV_DEPENDENT_MODE stBVDependentMode,
int &iTotalCashInURJB,
int &iNumStackedToCS,
int &iNumStackedToESC,
int pNumStackedToPerCass[MAX_CASSETTE_NUM],
int &iNumCSFed,
int &iNumESCFed,
int pNumPerCassFed[MAX_CASSETTE_NUM],
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeUnfitNotesDenoInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoInfo,
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
)
{
    return ERR_UR_INVALID_COMMAN;
}


//功能：回收残留在ESC中的钞票，并将钞票送至存钞箱/拒钞箱/回收箱
//输入：iValidateMode: 验钞模式，取值：VALIDATION_MODE_REAL或VALIDATION_MODE_TEST
//输入：btProhibitedBox: 表示禁止出钞的钱箱。1字节，0位~4位分别表示钱箱1~6,其余位值恒为0
//      位值为1表示禁止从对应的钱箱存钞
//输入：iDestinationReject: 指定回收位置传入DESTINATION_REJCET_DEFAULT则使用SET_UNIT_INFO中设置的值
//输入：stBVDependentMode: BV设置是否支持全幅图片，未识别冠字号时是否判别为拒钞，仅在bEnableBVModeSetting为true时有效，否则忽略
//输出：iTotalCashInURJB: URJB中的钞票总数=URJB中原有钞票数+每次操作后进入URJB中的钞票数,URJB<0x8000
//输出：iNumStackedToCS:  暂存到CS出钞口中的所有钞票张数
//输出：iNumStackedToESC: 暂存到ESC出钞口中的所有钞票张数
//输出：pNumStackedToPerCass: 表示挖钞时各钞箱暂存钞票张数,钞箱1~6的各自暂存钞票张数,整型数组,数组大小为6
//输出：iNumCSFed:  CS点钞数
//输出：iNumESCFed: ESC点钞数
//输出：pNumPerCassFed: 表示回收时回收至各钞箱的钞票张数,钞箱1~6的各钞箱钞票数,整型数组,数组大小为6
//输出：stStackeNotesDenoInf: 表示每种面额钞票的流向、张数信息
//输出：stStackeUnfitNotesDenoInfo:   表示不适合流通的钞票面额、流向、张数信息
//输出：stStackeRejectNotesDenoInfo:  表示回收钞的面额的目的信息了流向、张数
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：180
int CUR2Drv::RetractESC(
VALIDATION_MODE iValidateMode,
char btProhibitedBox,
DESTINATION_REJCET iDestinationReject,
const ST_BV_DEPENDENT_MODE stBVDependentMode,
int &iTotalCashInURJB,
int &iNumStackedToCS,
int &iNumStackedToESC,
int pNumStackedToPerCass[MAX_CASSETTE_NUM],
int &iNumCSFed,
int &iNumESCFed,
int pNumPerCassFed[MAX_CASSETTE_NUM],
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeNotesDenoInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeUnfitNotesDenoInfo,
ST_TOTAL_STACKE_NOTES_DENO_INFO &stStackeRejectNotesDenoInfo,
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
)
{
    const char *ThisModule = "RetractESC";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pNumStackedToPerCass);
    VERIFYPOINTNOTEMPTY(pNumPerCassFed);
    VERIFYPOINTNOTEMPTY(pMaintenanceInfo);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[102 + 1] = {0};
    if (iValidateMode == VALIDATION_MODE_REAL)
    {
        MOSAICCMND(szCmdData, iCmdLen, 0x4020);
    }
    else
    {
        MOSAICCMND(szCmdData, iCmdLen, 0x4F20);
    }
    iCmdLen += 6;

    //4.Room Exclusion(36 bytes) 可变包
    UCHAR szCmdParam[64] = {0};
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (btProhibitedBox != 0)
    {
        szCmdParam[0] = (UCHAR)0x05;
        szCmdParam[1] = (UCHAR)0x50;
        szCmdParam[2] = (UCHAR)0x00;
        szCmdParam[3] = (UCHAR)0x22;

        if (btProhibitedBox & 0x01) //Cass1
            szCmdParam[7] = 0x01;
        if (btProhibitedBox & 0x02) //Cass2
            szCmdParam[8] = 0x01;
        if (btProhibitedBox & 0x04) //Cass3
            szCmdParam[9] = 0x01;
        if (btProhibitedBox & 0x08) //Cass4
            szCmdParam[10] = 0x01;
        if (btProhibitedBox & 0x10) //Cass5
            szCmdParam[11] = 0x01;
        if (btProhibitedBox & 0x20) //URJB
            szCmdParam[27] = 0x01;

        memcpy(szCmdData + iCmdLen, szCmdParam, 36);
        iCmdLen += 36;
    }

    //3.Destination for Reject(6 bytes) 可变包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (iDestinationReject != DESTINATION_REJCET_DEFAULT)
    {
        szCmdParam[0] = (UCHAR)0x05;
        szCmdParam[1] = (UCHAR)0x51;
        szCmdParam[2] = (UCHAR)0x00;
        szCmdParam[3] = (UCHAR)0x04;
        szCmdParam[4] = (UCHAR)0x00;
        switch (iDestinationReject)
        {
        case DESTINATION_REJCET_CS:
            szCmdParam[5] = ( UCHAR )0x01;
            break;
        case DESTINATION_REJCET_DISPENSE:
            szCmdParam[5] = ( UCHAR )0x02;
            break;
        case DESTINATION_REJCET_DEPOSIT:
            szCmdParam[5] = ( UCHAR )0x03;
            break;
        default:
            szCmdParam[5] = (UCHAR)0x01;
        }

        memcpy(szCmdData + iCmdLen, szCmdParam, 6);
        iCmdLen += 6;
    }

    //4.BV Depensent Mode(6 bytes)可变包
    memset(szCmdParam, 0, sizeof(szCmdParam));
    if (stBVDependentMode.bEnableBVModeSetting)
    {
        szCmdParam[0] = 0x05;
        szCmdParam[1] = 0x2A;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x04;

        switch (stBVDependentMode.iFullImageMode)
        {
        case FULLIMAGE_ALL_NOTES:
            szCmdParam[4] = 0x80;
            break;
        case FULLIMAGE_REJECTE_NOTES:
            szCmdParam[4] = 0x02;
            break;
        case FULLIMAGE_NO_RECORDS:
            szCmdParam[4] = 0x00;
            break;
        default:
            szCmdParam[4] = 0x00;
            break;
        }

        switch (stBVDependentMode.iRejcetNoteNOSN)
        {
        case ACTION_REJECT_NOTES:
            szCmdParam[5] = 0x01;
            break;
        case ACTION_NO_REJECT_NOTES:
            szCmdParam[5] = 0x00;
            break;
        default:
            szCmdParam[5] = 0x00;
        }

        memcpy(szCmdData + iCmdLen, szCmdParam, 6);
        iCmdLen += 6;
    }

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 4538, HT_RESP_TIMEOUT_180, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 9;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x4000:
    case 0x4F00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x4040:
    case 0x4F40:
    case 0x4050:
    case 0x4F50:
        iRet = ERR_UR_WARN;
        break;
    case 0x4080:
    case 0x4F80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x40FF:
    case 0x4FFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //4.Number of Total Stacked Notes(64 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_NUMTOTALSTACKEDNOTES, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    iTotalCashInURJB = MAKEWORD(szResp[iDataPos + 51], szResp[iDataPos + 50]);

    //5.Number of Stacked Notes(64 bytes)
    int iRetParse = ParseRespStackedNotes(
                    szResp,
                    mapPacket,
                    iNumStackedToCS,
                    iNumStackedToESC,
                    pNumStackedToPerCass,
                    ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //6.Number of Fed Notes(36 bytes)
    iRetParse = ParseRespFedNotes(szResp, mapPacket, iNumCSFed, iNumESCFed, pNumPerCassFed, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //7.Number of Stacked Notes per Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, STACKE_NOTES_DENO_INFO_TYPE, stStackeNotesDenoInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //10.Number of Unfit Notes per Denomination and Destination(68,132,260,516 bytes)
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, UNFIT_NOTES_DENO_INFO_TYPE, stStackeUnfitNotesDenoInfo, ThisModule);
    if (iRetParse < 0)
    {
        return iRetParse;
    }

    //11.Number of Rejected Notes per Denomination and Destination(68,132,260,516 bytes) 可变包
    iRetParse = ParseRespNumStackedNotesPerDeno(szResp, mapPacket, REJECT_DEST_NOTES_DENO_INFO_TYPE, stStackeRejectNotesDenoInfo, ThisModule);

    //12.Maintenance Information(4100 bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MAINTENINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memset(pMaintenanceInfo, 0, sizeof(char)*MAINTENANCE_INFO_LENGTH);
    memcpy(pMaintenanceInfo, szResp + iDataPos, MAINTENANCE_INFO_LENGTH);

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：关闭Shutter门
//输入：bForcible:是否TRUE强制关闭SHUTTER门，尝试关闭10次
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：120
int CUR2Drv::CloseShutter(
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH],
BOOL bForcible
)
{
    const char *ThisModule = "CloseShutter";
    AutoLogFuncBeginEnd();
    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pMaintenanceInfo);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[36] = {0};
    if (bForcible)
    {
        MOSAICCMND(szCmdData, iCmdLen, 0x5840);
    }
    else
    {
        MOSAICCMND(szCmdData, iCmdLen, 0x5800);
    }
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 4170, HT_RESP_TIMEOUT_120, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 4;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x5800:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x5801:
    case 0x5802:
    case 0x5811:
    case 0x5812:
    case 0x5821:
    case 0x5822:
    case 0x5840:
        iRet = ERR_UR_WARN;
        break;
    case 0x5880:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x58FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //9.Maintenance Information(4100 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MAINTENINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memset(pMaintenanceInfo, 0, sizeof(char)*MAINTENANCE_INFO_LENGTH);
    memcpy(pMaintenanceInfo, szResp + iDataPos, MAINTENANCE_INFO_LENGTH);

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：关闭入金口Shutter门
//输入：bForcible:是否TRUE强制关闭SHUTTER门，尝试关闭10次
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：120  //test#13 
int CUR2Drv::CloseCashInShutter(
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH],
BOOL bForcible
)
{
    const char *ThisModule = "CloseCashInShutter";
    AutoLogFuncBeginEnd();
    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pMaintenanceInfo);
    int iRet = true;

    LOGDEVACTION();
    //打开入金口shutter门
    if (m_pDev == nullptr)                                                              //test#11
    {
//        string sComInfo = "COM:/dev/ttyUSB0:57600,N,8,1";
        iRet = m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "CRS", "DevUR2");  //test#11
      if (iRet != 0)     //test#11
      {                                                                                 //test#11
          Log(ThisModule, __LINE__, "加载库(AllDevPort.dll) 失败[%s]", m_pDev.LastError().toStdString().c_str());                      //test#11
          return ERR_DEVPORT_LIBRARY;                                                   //test#11
      }                                                                                 //test#11
//      LOGDEVACTION();                                                                   //test#11
      long lRet = m_pDev->Open(m_strCashInShutterPort.c_str()); //test#11
//      long lRet = m_pDev->Open(sComInfo.c_str());  //test by guojy
      if (0 != lRet)                                                                    //test#11
      {                                                                                 //test#11
          Log(ThisModule, __LINE__, "打开串口失败:%d 串口[%s]", lRet,m_pDev.LastError().toStdString().c_str()); //test#11
          return ERR_DEVPORT_NOTOPEN;                                                   //test#11
      }                                                                                 //test#11
    }

    //发送并接收响应
     char szResp[HT_RESP_LEN_MAX] = {0};
     USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数

     char szReadData[4096] = { 0 };
     DWORD dwReadLen = sizeof(szReadData);
     long lTimeOut = 5000;
     char vtData[8] = { 0x02, 0x07, 0x32, 0x04, 0x00, 0x02, 0x03, 0x30 };
 //    iRet = m_pDev->Send(vtData, sizeof(vtData), (DWORD)lTimeOut);

 //    iRet = m_pDev->Read(szReadData, dwReadLen, (DWORD)lTimeOut);
     iRet = m_pDev->SendAndRead(vtData, sizeof(vtData), szReadData,dwReadLen,(DWORD)lTimeOut);
     if (iRet == ERR_DEVPORT_RTIMEOUT || iRet == ERR_DEVPORT_WTIMEOUT)
     {
         Log(ThisModule, __LINE__, "关闭入金口门命令超时");
         return iRet;
     }
     if (iRet == ERR_DEVPORT_CANCELED)
     {
         Log(ThisModule, __LINE__, "关闭入金口门命令返回取消：%d", iRet);
         return iRet;
     }
     if (iRet < 0)
     {
         Log(ThisModule, __LINE__, "关闭入金口门命令失败：%d", iRet);
         return iRet;
     }

     if(szReadData[0] == 0x62){
         switch (szReadData[1]) {
         case SHUTTER_CLOSE_SUCCESS:
             Log(ThisModule, __LINE__, "入钞口门正常关闭：%d", szReadData[0]);
             break;
         case HAND_SENSOR_TIMEOUT:
         case HAND_SENSOR_RETRY_TIMEOUT:
         case SHUTTER_CLOSE_TIMEOUT:
         case HAND_SENSOR_ON_TIMEOUT:
             Log(ThisModule, __LINE__, "入钞口门关闭异常返回值：%d", szReadData[0]);
             return ERR_DEVPORT_FAIL;
         //Other return value, try again.
         }
     }else{
         iRet = false;
     }

     return iRet;
}

//功能：打开Shutter门
//输入：无
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：120
int CUR2Drv::OpenShutter(
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
)
{
    const char *ThisModule = "OpenShutter";
    AutoLogFuncBeginEnd();
    //校验参数合法性
    VERIFYPOINTNOTEMPTY(pMaintenanceInfo);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[36] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x5801);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 4170,  HT_RESP_TIMEOUT_120, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 4;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x5800:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x5840:
    case 0x5801:
    case 0x5802:
    case 0x5811:
    case 0x5812:
    case 0x5821:
    case 0x5822:
        iRet = ERR_UR_WARN;
        break;
    case 0x5880:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x58FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //9.Maintenance Information(4100 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_MAINTENINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    memset(pMaintenanceInfo, 0, sizeof(char)*MAINTENANCE_INFO_LENGTH);
    memcpy(pMaintenanceInfo, szResp + iDataPos, MAINTENANCE_INFO_LENGTH);

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：打开入金口Shutter门
//输入：无
//输出：pMaintenanceInfo: 维护信息
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：120 //test#13 
int CUR2Drv::OpenCashInShutter(
char pMaintenanceInfo[MAINTENANCE_INFO_LENGTH]
)
{
    const char *ThisModule = "OpenCashInShutter";
     AutoLogFuncBeginEnd();
     //校验参数合法性
     VERIFYPOINTNOTEMPTY(pMaintenanceInfo);
     int iRet = true;

     LOGDEVACTION();
     //打开入金口shutter门
     if (m_pDev == nullptr)                                                              //test#11
     {
         string sComInfo = "COM:/dev/ttyUSB0:57600,N,8,1";
         iRet = m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "CRS", "DevUR2");  //test#11
       if (iRet != 0)     //test#11
       {                                                                                 //test#11
           Log(ThisModule, __LINE__, "加载库(AllDevPort.dll) 失败[%s]", m_pDev.LastError().toStdString().c_str());                      //test#11
           return ERR_DEVPORT_LIBRARY;                                                   //test#11
       }                                                                                 //test#11
 //      LOGDEVACTION();                                                                   //test#11
       long lRet = m_pDev->Open(m_strCashInShutterPort.c_str()); //test#11
 //      long lRet = m_pDev->Open(sComInfo.c_str());  //test by guojy
       if (0 != lRet)                                                                    //test#11
       {                                                                                 //test#11
           Log(ThisModule, __LINE__, "打开串口失败:%d 串口[%s]", lRet,m_pDev.LastError().toStdString().c_str()); //test#11
           return ERR_DEVPORT_NOTOPEN;                                                   //test#11
       }                                                                                 //test#11
     }
     //发送并接收响应
     char szResp[HT_RESP_LEN_MAX] = {0};
     USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数

     char szReadData[4096] = { 0 };
     DWORD dwReadLen = sizeof(szReadData);
     long lTimeOut = 10000;
     char vtData[8] = { 0x02, 0x07, 0x31, 0x04, 0x00, 0x02, 0x03, 0x33 };
     iRet = m_pDev->SendAndRead((LPCSTR)vtData, sizeof(vtData), szReadData,dwReadLen,(DWORD)lTimeOut);

     if (iRet == ERR_DEVPORT_RTIMEOUT || iRet == ERR_DEVPORT_WTIMEOUT)
     {
         Log(ThisModule, __LINE__, "打开入金口门命令超时");
         return iRet;
     }
     if (iRet == ERR_DEVPORT_CANCELED)
     {
         Log(ThisModule, __LINE__, "打开入金口门命令返回取消：%d", iRet);
         return iRet;
     }
     if (iRet < 0)
     {
         Log(ThisModule, __LINE__, "打开入金口门命令失败：%d", iRet);
         return iRet;
     }

     if(szReadData[0] == 0x61){
         switch (szReadData[2]) {
         case SHUTTER_OPEN_SUCCESS:
             Log(ThisModule, __LINE__, "入钞口门正常打开：%d", szReadData[0]);
             break;
         case MAGNET_OPEN_TIMEOUT:
         case HAND_SENSOR_TIMEOUT:
         case HAND_SENSOR_RETRY_TIMEOUT:
         case SHUTTER_OPEN_TIMEOUT:
         case SHUTTER_CLOSE_TIMEOUT:
         case LOCK_OPEN_TIMEOUT:
         case HAND_SENSOR_ON_TIMEOUT:
             Log(ThisModule, __LINE__, "入钞口门打开异常返回值：%d", szReadData[0]);
             return ERR_DEVPORT_FAIL;
         //Other return value, try again.
         }
     }else{
         iRet = false;
     }

     return iRet;
}

//功能：取入金口Shutter门状态
//输入：无
//输出：无
//返回：0 打开，1 关闭
//说明：执行命令的最大时间(秒)：120 //test#13 
int CUR2Drv::GetCashInShutterStatus(
)
{
    const char *ThisModule = "GetCashInShutterStatus";
 //    AutoLogFuncBeginEnd();

     int iRet = true;

     LOGDEVACTION();
     //打开入金口shutter门
     if (m_pDev == nullptr)                                                              //test#11
     {
         string sComInfo = "COM:/dev/ttyUSB0:57600,N,8,1";
         iRet = m_pDev.Load("AllDevPort.dll", "CreateIAllDevPort", "CRS", "DevUR2");  //test#11
       if (iRet != 0)     //test#11
       {                                                                                 //test#11
           Log(ThisModule, __LINE__, "加载库(AllDevPort.dll) 失败[%s]", m_pDev.LastError().toStdString().c_str());                      //test#11
           return ERR_DEVPORT_LIBRARY;                                                   //test#11
       }                                                                                 //test#11
 //      LOGDEVACTION();                                                                   //test#11
       long lRet = m_pDev->Open(m_strCashInShutterPort.c_str()); //test#11
  //     long lRet = m_pDev->Open(sComInfo.c_str());  //test by guojy
       if (0 != lRet)                                                                    //test#11
       {                                                                                 //test#11
           Log(ThisModule, __LINE__, "打开串口失败:%d 串口[%s]", lRet,m_pDev.LastError().toStdString().c_str()); //test#11
           return ERR_DEVPORT_NOTOPEN;                                                   //test#11
       }                                                                                 //test#11
     }
     //发送并接收响应
     char szResp[HT_RESP_LEN_MAX] = {0};
     USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数

     char szReadData[4096] = { 9 };  //test by guojy
     DWORD dwReadLen = sizeof(szReadData);
     long lTimeOut = 10000;
     char cCmdData[5] = { 0x02, 0x04, 0x34, 0x03, 0x33 };
     iRet = m_pDev->SendAndRead((LPCSTR)cCmdData, sizeof(cCmdData), szReadData,dwReadLen,(DWORD)lTimeOut);

    if ((dwReadLen == 0) || (iRet != 0)) {
         //When read failed set shutter status is closed
         m_stDevStatusInfo.CashInShutterStatus = UR2SHUTTER_STATUS_UNKNOWN;
    }else{
        if (szReadData[0] == SHUTTER_STATUS_RESPONSE)
        {
            int iIndex = 1;
            if(dwReadLen == 1){ //test by guojy
                iRet = m_pDev->Read(szReadData,dwReadLen,(DWORD)lTimeOut);//test by guojy
                iIndex = 0;
            }//test by guojy
            if(szReadData[iIndex] == SHUTTER_SENSOR_ON){
                // S2 is Open
                m_stDevStatusInfo.CashInShutterStatus = UR2SHUTTER_STATUS_OPEN;
                return TRUE;
             }
            if(szReadData[iIndex] == SHUTTER_SENSOR_OFF){
               // S1 is Close
                m_stDevStatusInfo.CashInShutterStatus = UR2SHUTTER_STATUS_CLOSED;
               return TRUE;
            }
        }else{
           m_stDevStatusInfo.CashInShutterStatus = UR2SHUTTER_STATUS_UNKNOWN;
        }
    }
    return TRUE;
}


//功能：开始下载固件
//输入：无
//输出：无
//返回：0: 成功，非0: 失败
//说明：进入下载模式成功后，HCM不能响应其他任何命令
//说明：执行命令的最大时间(秒)：210
int CUR2Drv::ProgramDownloadStart()
{
    const char *ThisModule = "ProgramDownloadStart";

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[36] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x0301);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70, HT_RESP_TIMEOUT_210, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x0300:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x0380:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x03FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：下载固件数据
//输入：usControlID: 文件ID
//输入：iPDLBlockType: 固件文件块类型
//输入：uWritingAddress: 下载地址
//输入：uSentDataLen: 已下载过的数据长度
//输入：lpData: 下载的数据
//输入：usDataLen: 下载数据长度
//输出：无
//返回：0 成功，>0 警告，<0 失败
//说明：执行命令的最大时间(秒)：210
int CUR2Drv::ProgramDownloadSendData(
USHORT usControlID,
PDL_BLOCK_TYPE iPDLBlockType,
UINT uWritingAddress,
UINT uSentDataLen,
const char *lpData,
USHORT usDataLen
)
{
    const char *ThisModule = "ProgramDownloadSendData";

    //校验参数合法性
    if (usControlID < PACKET_ID_FW_UR2BOOT || usControlID > PACKET_ID_FW_BV4)
    {
        Log(ThisModule, ERR_UR_PARAM, "usControlID为无效数据[0x%04X]", usControlID);
        return ERR_UR_PARAM;
    }
    /*
        if(uSentDataLen < 0 || usDataLen < 0 || usDataLen > FW_BATCH_DATA_MAXSIZE)
        {
            if (usDataLen < 0 || usDataLen > FW_BATCH_DATA_MAXSIZE)
                Log(ThisModule, ERR_UR_PARAM, "usDataLen为无效数据[%d]", usDataLen);
            else
                Log(ThisModule, ERR_UR_PARAM, "uSentDataLen为无效数据");
            return ERR_UR_PARAM;
        }
    */
    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[7194] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x0302);
    iCmdLen += 6;

    //2.PDL Management Information(14 bytes)
    UCHAR szCmdParam[64 + 1] = {0};
    szCmdParam[0] = 0x00;
    szCmdParam[1] = 0x05;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x0C;

    //Control ID
    szCmdParam[4] = ((usControlID >> 8) & 0xFF);
    szCmdParam[5] = (usControlID & 0xFF);

    //PDL Block Type
    switch (iPDLBlockType)
    {
    case PDL_FIRST_BLOCK:
        szCmdParam[6] = 0x00;
        szCmdParam[7] = 0x00;
        break;
    case PDL_MIDDLE_BLOCK:
        szCmdParam[6] = 0x00;
        szCmdParam[7] = 0x01;
        break;
    case PDL_LAST_BLOCK:
        szCmdParam[6] = 0xFF;
        szCmdParam[7] = 0xFF;
        break;
    default:
        {
            Log(ThisModule, ERR_UR_PARAM, "iPDLBlockType为无效数据[%d]", iPDLBlockType);
            return ERR_UR_PARAM;
        }
    }

    UINT uWritingAddressTemp = uWritingAddress + uSentDataLen;
    szCmdParam[10] = ((uWritingAddressTemp >> 24) & 0xFF);
    szCmdParam[11] = ((uWritingAddressTemp >> 16) & 0xFF);
    szCmdParam[12] = ((uWritingAddressTemp >>  8) & 0xFF);
    szCmdParam[13] = (uWritingAddressTemp & 0xFF);

    memcpy(szCmdData + iCmdLen, szCmdParam, 14);
    iCmdLen += 14;

    //3.PDL data(6~7172 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x00;
    szCmdParam[1] = 0x06;

    USHORT usDataLenTemp = usDataLen + 2;
    szCmdParam[2] = ((usDataLenTemp >> 8) & 0xFF);
    szCmdParam[3] = (usDataLenTemp & 0xFF);

    memcpy(szCmdData + iCmdLen, szCmdParam, 4);
    iCmdLen += 4;

    memcpy(szCmdData + iCmdLen, lpData, usDataLen);
    iCmdLen += usDataLen;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_210, ThisModule, CONNECT_TYPE_UR, TRUE);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x0300:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x0380:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x03FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：结束下载固件
//输入：无
//输出：无
//返回：0 成功，>0 警告，<0 失败
//说明：执行命令的最大时间(秒)：210
int CUR2Drv::ProgramDownloadEnd()
{
    const char *ThisModule = "ProgramDownloadEnd";

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[36] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x0303);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_210, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x0300:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x0380:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x03FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：断开与ATM PC端口通讯连接，并重启模块到上电时的初始状态
//输入：无
//输出：无
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：210
int CUR2Drv::Reboot()
{
    const char *ThisModule = "Reboot";

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[36] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x8000);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70, HT_RESP_TIMEOUT_210, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x8000:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x80FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：写数据到用户内存区域
//输入：iUserMemoryTaget: 写入的内存区域,不可选择USER_MEMORY_TARGET_ALLCASS
//输入：szUserMemoryData: 写入的数据，最大写入128字节
//输入：usDataLen: 数据长度
//输出：无
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：120
int CUR2Drv::UserMemoryWrite(
USER_MEMORY_TARGET iUserMemoryTaget,
const char szUserMemoryData[MAX_USER_MEMORY_DATA_LENGTH],
USHORT usDataLen
)
{
    const char *ThisModule = "UserMemoryWrite";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(szUserMemoryData);
    if (usDataLen <= 0 || usDataLen > 128)
    {
        Log(ThisModule, ERR_UR_PARAM, "usDataLen为无效数据[%d]", usDataLen);
        return ERR_UR_PARAM;
    }

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[7194] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x7B00);
    iCmdLen += 6;

    //2.User Memory Data(134 bytes)
    UCHAR szCmdParam[134 + 1] = {0};
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x61;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x84;

    switch (iUserMemoryTaget)
    {
    case USER_MEMORY_TARGET_CASS1:
        szCmdParam[4] = 0x10;
        break;
    case USER_MEMORY_TARGET_CASS2:
        szCmdParam[4] = 0x20;
        break;
    case USER_MEMORY_TARGET_CASS3:
        szCmdParam[4] = 0x30;
        break;
    case USER_MEMORY_TARGET_CASS4:
        szCmdParam[4] = 0x40;
        break;
    case USER_MEMORY_TARGET_CASS5:
        szCmdParam[4] = 0x50;
        break;
    case USER_MEMORY_TARGET_UR2:
        szCmdParam[4] = 0x80;
        break;
    default:
        {
            Log(ThisModule, ERR_UR_PARAM, "iUserMemoryTaget为无效数据[%d]", iUserMemoryTaget);
            return ERR_UR_PARAM;
        }
    }

    memcpy(szCmdParam + 6, szUserMemoryData, usDataLen);
    memcpy(szCmdData + iCmdLen, szCmdParam, 134);
    iCmdLen += 134;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70, HT_RESP_TIMEOUT_120, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x7B00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x7B01:
        iRet = ERR_UR_WARN;
        break;
    case 0x7B40:
        iRet = ERR_UR_WARN;
        break;
    case 0x7B80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x7BFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}


//功能：读取用户内存的数据
//输入：iUserMemoryTaget: 读取的内存区域
//输出：stUserMemoryData: 用户数据信息结构
//      USER_MEMORY_TARGET_ALLCASS同时读取5个钱箱内存数据;可读取单个数据，若返回读取内存区域USER_MEMORY_TARGET_RESERVED 则为无效数据
//输出：usDataArrayCount: 读取后的stUserMemoryData数组有效数据个数, 取值范围0-5
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：120
int CUR2Drv::UserMemoryRead(
USER_MEMORY_TARGET iUserMemoryTaget,
ST_USER_MEMORY_READ stUserMemoryData[MAX_USER_MEMORY_DATA_ARRAY_NUM],
USHORT &usDataArrayCount
)
{
    const char *ThisModule = "UserMemoryRead";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(stUserMemoryData);
    memset(stUserMemoryData, 0, sizeof(ST_USER_MEMORY_READ) * MAX_USER_MEMORY_DATA_ARRAY_NUM);
    usDataArrayCount = 0;

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[14 + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x7B10);
    iCmdLen += 6;

    //2.User Memory Select(6 bytes)
    UCHAR szCmdParam[6 + 1] = {0};
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x60;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x04;

    switch (iUserMemoryTaget)
    {
    case USER_MEMORY_TARGET_UR2:
        szCmdParam[4] = 0x80;
        szCmdParam[5] = 0x00;
        break;
    case USER_MEMORY_TARGET_CASS1:
    case USER_MEMORY_TARGET_CASS2:
    case USER_MEMORY_TARGET_CASS3:
    case USER_MEMORY_TARGET_CASS4:
    case USER_MEMORY_TARGET_CASS5:
    case USER_MEMORY_TARGET_ALLCASS:
        szCmdParam[4] = 0x00;
        szCmdParam[5] = 0x1F;
        break;
    default:
        {
            Log(ThisModule, ERR_UR_PARAM, "iUserMemoryTaget为无效数据[%d]", iUserMemoryTaget);
            return ERR_UR_PARAM;
        }
    }

    memcpy(szCmdData + iCmdLen, szCmdParam, 6);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_120, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x7B00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x7B40:
        iRet = ERR_UR_WARN;
        break;
    case 0x7B80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x7BFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //3.USER MEMORY DATA(134 bytes) 对于USER_MEMORY_TARGET_ALLCASS参数时可能返回5个钞箱的数据，数据包ID相同均为0x0561,为可变包
    USHORT usDataArrayCountTemp = (dwRespLen - 70) / 134;
    if (usDataArrayCountTemp > 0)
    {
        char szUserMemoryDataPacket[134 + 1] = {0};
        USHORT usPakcetID = 0;
        USHORT usPakcetIDLength = 0;
        USER_MEMORY_TARGET iUserMemoryTagetTemp = USER_MEMORY_TARGET_RESERVED;
        for (int i = 0; i < usDataArrayCountTemp; i++)
        {
            usPakcetID = 0;
            usPakcetIDLength = 0;
            memset(szUserMemoryDataPacket, 0, sizeof(szUserMemoryDataPacket));
            memcpy(szUserMemoryDataPacket, szResp + 70 + i * 134, 134);

            usPakcetID = MAKEWORD(szUserMemoryDataPacket[1], szUserMemoryDataPacket[0]);
            usPakcetIDLength   = MAKEWORD(szUserMemoryDataPacket[3], szUserMemoryDataPacket[2]);
            if (usPakcetID != PACKET_ID_USERMEMORYDATA || usPakcetIDLength != 0x0084)
            {
                Log(ThisModule, -1, "%s命令返回数据异常, 第%d数据包ID为0x%04X,长度为0x%04X", ThisModule, i + 1, usPakcetID, usPakcetIDLength);
                memset(stUserMemoryData, 0, sizeof(ST_USER_MEMORY_READ) * MAX_USER_MEMORY_DATA_ARRAY_NUM);
                usDataArrayCount = 0;
                return ERR_UR_RESPDATA_ERR;
            }

            switch (szUserMemoryDataPacket[4])
            {
            case 0x10:
                iUserMemoryTagetTemp = USER_MEMORY_TARGET_CASS1;
                break;
            case 0x20:
                iUserMemoryTagetTemp = USER_MEMORY_TARGET_CASS2;
                break;
            case 0x30:
                iUserMemoryTagetTemp = USER_MEMORY_TARGET_CASS3;
                break;
            case 0x40:
                iUserMemoryTagetTemp = USER_MEMORY_TARGET_CASS4;
                break;
            case 0x50:
                iUserMemoryTagetTemp = USER_MEMORY_TARGET_CASS5;
                break;
            case 0x80:
                iUserMemoryTagetTemp = USER_MEMORY_TARGET_UR2;
                break;
            default:
                {
                    memset(stUserMemoryData, 0, sizeof(ST_USER_MEMORY_READ) * MAX_USER_MEMORY_DATA_ARRAY_NUM);
                    usDataArrayCount = 0;
                    Log(ThisModule, -1, "%s命令返回UserMemoryTaget数据异常, 第%d数据包UserMemoryTaget为0x%02X", ThisModule, i + 1, (UCHAR)szUserMemoryDataPacket[4]);
                    return ERR_UR_RESPDATA_ERR;
                }
            }

            if (iUserMemoryTaget == USER_MEMORY_TARGET_ALLCASS)
            {
                usDataArrayCount++;
                stUserMemoryData[i].iUserMemoryTaget = iUserMemoryTagetTemp;
                memcpy(stUserMemoryData[i].szUserMemoryData, szUserMemoryDataPacket + 6, 128);
            }
            else if (iUserMemoryTaget == iUserMemoryTagetTemp)
            {
                usDataArrayCount = 1;
                stUserMemoryData[0].iUserMemoryTaget = iUserMemoryTagetTemp;
                memcpy(stUserMemoryData[0].szUserMemoryData, szUserMemoryDataPacket + 6, 128);
            }
        }
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：查询BV模块信息
//输入：无
//输出：stBVWarningInfo: BV警告信息数组
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：20
int CUR2Drv::QueryBVStatusSense(
ST_BV_WARNING_INFO stBVWarningInfo[MAX_BV_WARNING_INFO_NUM]
)
{
    const char *ThisModule = "QueryBVStatusSense";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(stBVWarningInfo);

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[7194] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x0C00);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 194,  HT_RESP_TIMEOUT_20, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 4;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x0C00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x0C80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x0CFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_BVWARNINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    for (int i = 0; i < MAX_BV_WARNING_INFO_NUM; i++)
    {
        stBVWarningInfo[i].iWaringCode = Count4BytesToULValue((UCHAR)szResp, iDataPos + i * 30);
        //stBVWarningInfo[i].usRecoveryCode = (USHORT)(((UCHAR)szResp[iDataPos + 12 + i * 30] << 8) + (UCHAR)(szResp[iDataPos + 13 + i * 30]));
        stBVWarningInfo[i].usRecoveryCode = MAKEWORD(szResp[iDataPos + 13 + i * 30], szResp[iDataPos + 12 + i * 30]);
        memcpy(stBVWarningInfo[i].ucPositionCode, szResp + iDataPos + 14 + i * 30, 16);
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：获取冠字码相关信息
//输入：dwIndex: 获取冠字码序号，从0开始
//输出：lpNoteSerialInfo: 保存冠字码信息的结构指针
//返回：0：成功，其他：失败
//说明：获取冠字码数据前需要调用BVCommStart函数，全部获取结束后调用BVCommEnd函数，结束与BV通信
long CUR2Drv::GetNoteSeiralInfo(DWORD dwIndex, LPSNOTESERIALINFO lpNoteSerialInfo)
{
    const char *ThisModule = "GetNoteSeiralInfo";
    assert(lpNoteSerialInfo != NULL);

    BOOL bHeadData = TRUE;
    char szCmdData[HT_CMD_LEN_ZERO] = {0};
    char szRespData[HT_RESP_LEN_ZERO] = {0};
    szCmdData[0] = 0x00;
    szCmdData[1] = 0x24;
    szCmdData[5] = 0x01;
    szCmdData[6] = static_cast<char>(0x81);

    szCmdData[8] = static_cast<char>(HT_RESP_LEN_ZERO / 256);
    szCmdData[9] = static_cast<char>(HT_RESP_LEN_ZERO % 256);

    szCmdData[10] = static_cast<char>(dwIndex / 256);
    szCmdData[11] = static_cast<char>(dwIndex % 256);

    USHORT dwRespLen = 0;
    int iRet = 0;
    USHORT dwDataLen = 0;

    char aryHead[14] = {0};
    aryHead[0]  = 0x42;//BMP文件头格式
    aryHead[1]  = 0x4D;
    aryHead[10] = 0x36;

    USHORT swCount  = 0;

    USHORT offset = 0;
    while (TRUE)
    {
        if (bHeadData)
        {
            szCmdData[7] = 0x01;//Head data
            bHeadData = FALSE;
        }
        else
        {
            szCmdData[7] = 0x02;//following data
        }

        memset(szRespData, 0, sizeof(char)*HT_RESP_LEN_ZERO);
        dwRespLen = HT_RESP_LEN_ZERO;
        iRet = ExecuteCmd(szCmdData, HT_CMD_LEN_ZERO, szRespData, dwRespLen, HT_RESP_LEN_ZERO, HT_RESP_TIMEOUT_10, ThisModule, CONNECT_TYPE_ZERO);

        if (iRet < 0)
        {
            Log(ThisModule, iRet, "序列号获取失败");
            return iRet;
        }
        if ((szRespData[4] == 0x08) || (szRespData[4] == 0x18))//失败或警告，获取的数据没意义
        {
            //Log(ThisModule, ERR_UR_USB_OTHER, "序列号获取失败");
            return ERR_UR_USB_OTHER;
        }

        if (!(szRespData[15] & 0x02))//data not exist , is not err
        {
            return ERR_UR_WARN;
        }

        dwDataLen = static_cast<unsigned char>(szRespData[18]) * 256 + static_cast<unsigned char>(szRespData[19]);
        lpNoteSerialInfo->dwBVInternalIndex = Count4BytesToULValue((UCHAR)szRespData, 28);

        //写入文件
        if (szRespData[15] & 0x01)//Head block
        {
            if (szRespData[74] == 0x01 && szRespData[75] == 0x01)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_1988_010;
            }
            else if (szRespData[74] == 0x0A && szRespData[75] == 0x01)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_2001_010;
            }
            else if (szRespData[74] == 0x06 && szRespData[75] == 0x02)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_2005_010;
            }
            else if (szRespData[74] == 0x0B && szRespData[75] == 0x01)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_2000_020;
            }
            else if (szRespData[74] == 0x05 && szRespData[75] == 0x02)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_2005_020;
            }
            else if (szRespData[74] == 0x02 && szRespData[75] == 0x01)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_1992_050;
            }
            else if (szRespData[74] == 0x0C && szRespData[75] == 0x01)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_2001_050;
            }
            else if (szRespData[74] == 0x07 && szRespData[75] == 0x02)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_2005_050;
            }
            else if (szRespData[74] == 0x03 && szRespData[75] == 0x01)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_1992_100;
            }
            else if (szRespData[74] == 0x0E && szRespData[75] == 0x01)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_1999_100;
            }
            else if (szRespData[74] == 0x04 && szRespData[75] == 0x02)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_2005_100;
            }
            else if (szRespData[74] == 0x0D && szRespData[75] == 0x03)
            {
                lpNoteSerialInfo->NotesEdition = eNoteEdition_2015_100;
            }
            else
            {
                //Log(ThisModule, 1,    "识别人民币为未知版本：%x%x",szRespData[74],szRespData[75]);
            }
            memcpy(&lpNoteSerialInfo->cSerialImgData[offset], aryHead, 14);
            offset += 14;
            memcpy(lpNoteSerialInfo->cSerialNumber, &szRespData[84], 16);
            memcpy(&lpNoteSerialInfo->cSerialImgData[offset], &szRespData[100], 40);
            offset += 40;
            memcpy(&lpNoteSerialInfo->cSerialImgData[offset], &szRespData[140], dwDataLen - 72);
            offset += (dwDataLen - 72);
            swCount += (dwDataLen - 72);
        }
        else
        {
            memcpy(&lpNoteSerialInfo->cSerialImgData[offset], &szRespData[68], dwDataLen);
            offset += dwDataLen;
            swCount += dwDataLen;
        }

        if (szRespData[15] & 0x80)//Final block
        {
            char temp[10];
            temp[0] = static_cast<char>((swCount + 54) % 256);
            temp[1] = static_cast<char>((swCount + 54) / 256);
            memcpy(&lpNoteSerialInfo->cSerialImgData[2], temp, 2);
            assert(offset < MAX_SNIMAGE_DATA_LENGTH);
            lpNoteSerialInfo->dwImgDataLen = offset;
            break;
        }
    }

    return ERR_UR_SUCCESS;
}

//功能：获取冠字码全幅图像相关信息
//输入：dwIndex: 获取冠字码序号，从0开始
//输出：lpNoteSerialInfoFullImage: 保存冠字码信息的结构指针
//返回：0：成功，其他：失败
//说明：获取冠字码数据前需要调用BVCommStart函数，全部获取结束后调用BVCommEnd函数，结束与BV通信
long CUR2Drv::GetNoteSeiralInfoFullImage(DWORD dwIndex, LPSNOTESERIALINFOFULLIMAGE lpNoteSerialInfoFullImage)
{
    const char *ThisModule = "GetNoteSeiralInfoFullImage";
    assert(lpNoteSerialInfoFullImage != NULL);

    BOOL bHeadData = TRUE;
    char szCmdData[HT_CMD_LEN_ZERO] = {0};
    char szRespData[HT_RESP_LEN_ZERO * 2] = {0};
    szCmdData[0] = 0x00;
    szCmdData[1] = 0x24;
    szCmdData[5] = 0x01;
    szCmdData[6] = static_cast<char>(0x82);

    szCmdData[8] = static_cast<char>(0x80);
    szCmdData[9] = 0x08;

    szCmdData[10] = static_cast<char>(dwIndex / 256);
    szCmdData[11] = static_cast<char>(dwIndex % 256);

    USHORT dwRespLen = 0;
    int iRet = 0;
    DWORD dwDataLen = 0;

    char aryHead[14] = {0};
    aryHead[0]  = 0x42;//BMP文件头格式
    aryHead[1]  = 0x4D;
    aryHead[10] = 0x36;

    USHORT swCount  = 0;

    DWORD offset = 0;
    while (TRUE)
    {
        if (bHeadData)
        {
            szCmdData[7] = 0x01;//Head data
            bHeadData = FALSE;
        }
        else
        {
            szCmdData[7] = 0x02;//following data
        }

        memset(szRespData, 0, sizeof(char)*HT_RESP_LEN_ZERO * 2);
        dwRespLen = static_cast<USHORT>(HT_RESP_LEN_ZERO * 2);
        iRet = ExecuteCmd(szCmdData, HT_CMD_LEN_ZERO, szRespData, dwRespLen, HT_RESP_LEN_ZERO,  HT_RESP_TIMEOUT_10, ThisModule, CONNECT_TYPE_ZERO);

        if (iRet < 0)
        {
            Log(ThisModule, iRet, "序列号全幅获取失败");
            return iRet;
        }
        if ((szRespData[4] == 0x08) || (szRespData[4] == 0x18))//失败或警告，获取的数据没意义
        {
            //Log(ThisModule, ERR_UR_USB_OTHER, "序列号获取失败");
            return ERR_UR_USB_OTHER;
        }

        if (!(szRespData[15] & 0x02))//data not exist , is not err
        {
            return ERR_UR_WARN;
        }

        dwDataLen = static_cast<BYTE>(szRespData[18]) * 256 + static_cast<BYTE>(szRespData[19]);
        lpNoteSerialInfoFullImage->dwBVInternalIndex = Count4BytesToULValue((UCHAR)szRespData, 28);

        //写入文件
        if (szRespData[15] & 0x01)//Head block
        {
            memcpy(&lpNoteSerialInfoFullImage->cFullImgData[offset], aryHead, 14);
            offset += 14;
            memcpy(&lpNoteSerialInfoFullImage->cFullImgData[offset], &szRespData[84], 40);
            offset += 40;
            memcpy(&lpNoteSerialInfoFullImage->cFullImgData[offset], &szRespData[124], dwDataLen - 56);
            offset += (dwDataLen - 56);
            swCount += (dwDataLen - 56);
        }
        else
        {
            if (offset + dwDataLen > MAX_FULLIMAGE_DATA_LENGTH)
            {
                Log(ThisModule, 0, "全副图像数据长度(0x%X)大于预计长度0x%X", offset + dwDataLen, MAX_FULLIMAGE_DATA_LENGTH);
                return -1;
            }
            memcpy(&lpNoteSerialInfoFullImage->cFullImgData[offset], &szRespData[68], dwDataLen);
            offset += dwDataLen;
            swCount += dwDataLen;
        }

        if (szRespData[15] & 0x80)//Final block
        {
            char temp[10];
            temp[0] = static_cast<char>((swCount + 54) % 256);
            temp[1] = static_cast<char>((swCount + 54) / 256);
            memcpy(&lpNoteSerialInfoFullImage->cFullImgData[2], temp, 2);
            //assert(offset < MAX_FULLIMAGE_DATA_LENGTH);
            if (offset > MAX_FULLIMAGE_DATA_LENGTH)
            {
                Log(ThisModule, 0, "全副图像总数据长度(0x%X)大于预计长度0x%X", offset, MAX_FULLIMAGE_DATA_LENGTH);
                return -1;
            }
            lpNoteSerialInfoFullImage->dwImgDataLen = offset;
            break;
        }
    }

    return ERR_UR_SUCCESS;
}


//功能：获取钞票流向面额信息，包括到了何处、面值、版次等
//输入：ucMediaInfoNum: 查询的钞票信息号，取值为1、2、3、4，每次调用自增1，第一次调用值是1
//输出：usNumNotes: 本次返回的钞票信息数，一次最多返回512个钞票信息
//输出：usNumTotalNotes: 产生钞票信息总数
//输出：arryMediaInfo: 返回的钞票信息保存位置数组eh
//返回：0:  成功 后面没有数据了
//      >0: 返回插入arryMediaInfo中的对象个数.如果个数等于512表示后面还有数据，请继续调用本函数获取其余的信息
//      <0: 命令执行失败
//说明：执行命令的最大时间(秒)：20
long CUR2Drv::GetMediaInformation(
char ucMediaInfoNum,
USHORT  &usNumNotes,
USHORT  &usNumTotalNotes,
ST_MEDIA_INFORMATION_INFO arryMediaInfo[MAX_MEDIA_INFO_NUM]
)
{
    const char *ThisModule = "GetMediaInformation";

    //校验参数合法性
    VERIFYPOINTNOTEMPTY(arryMediaInfo);

    //检查参数
    if (ucMediaInfoNum == 0)
    {
        //log_write(LOGFILE, THIS_FILE_USB_DRV,ThisModule, ERR_UR_PARAM, "参数ucMediaInfoNum不能为:%d", ucMediaInfoNum);
        return ERR_UR_PARAM;
    }

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[14] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x7C80);
    iCmdLen += 6;

    //2.ACT(6 bytes)
    char szCmdParam[6 + 1] = {0};
    if (ucMediaInfoNum > 1)
    {
        szCmdParam[0] = 0x00;
        szCmdParam[1] = 0x02;
        szCmdParam[2] = 0x00;
        szCmdParam[3] = 0x04;

        szCmdParam[4] = 0x00;
        szCmdParam[5] = static_cast<char>((ucMediaInfoNum - 1) & 0xFF);

        memcpy(szCmdData + iCmdLen, szCmdParam, 6);
        iCmdLen += 6;
    }

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_20, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x7C00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x7C01:
        iRet = ERR_UR_WARN;
        break;
    case 0x7C80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x7CFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    if (iRet == ERR_UR_WARN)
    {
        Log(ThisModule, 1, "GetMediaInformation数据已全部提取完全");
        return ERR_UR_SUCCESS;
    }

    //4.Notes Identification(6164 bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PAKCET_ID_LOG_NOTESIDENTIF, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    UCHAR szNoteInfoHeader[16 + 1] = {0};
    USHORT usACT = 0;
    memcpy(szNoteInfoHeader, szResp + iDataPos, 16);
    //usNumNotes = (USHORT)(((UCHAR)szNoteInfoHeader[0] << 8) + (UCHAR)szNoteInfoHeader[1]);
    //usNumTotalNotes = (USHORT)(((UCHAR)szNoteInfoHeader[2] << 8) + (UCHAR)szNoteInfoHeader[3]);
    //usACT = (USHORT)(((UCHAR)szNoteInfoHeader[14] << 8) + (UCHAR)szNoteInfoHeader[15]);
    usNumNotes = MAKEWORD(szNoteInfoHeader[1], szNoteInfoHeader[0]);
    usNumTotalNotes = MAKEWORD(szNoteInfoHeader[3], szNoteInfoHeader[2]);
    usACT = MAKEWORD(szNoteInfoHeader[15], szNoteInfoHeader[14]);

    UCHAR szMediaInfo[12 + 1] = {0};
    for (int i = 0; i < usNumNotes; i++)
    {
        memset(szMediaInfo, 0, sizeof(szMediaInfo));
        memcpy(szMediaInfo, szResp + iDataPos + 16 + i * 12, 12);

        arryMediaInfo[i].ulBVInternalCounter = Count4BytesToULValue(szMediaInfo, 0);
        arryMediaInfo[i].iMediaInfoOrigin  = CASSETTE_ROOM_ID(szMediaInfo[4]);
        arryMediaInfo[i].iMediaInfoDest    = CASSETTE_ROOM_ID(szMediaInfo[5]);
        arryMediaInfo[i].iMediaInfoDnoCode = (szMediaInfo[6] == 0x80 ? DENOMINATION_CODE(DENOMINATION_CODE_00) : DENOMINATION_CODE(szMediaInfo[6]));
        arryMediaInfo[i].iBVImage          = BV_IMAGE_TYPE(szMediaInfo[7]);
        arryMediaInfo[i].iMediaInfoRejectCause = MEDIA_INFORMATION_REJECT_CAUSE(szMediaInfo[8]);

        switch (szMediaInfo[9])
        {
        case 0x41:
            arryMediaInfo[i].iNoteCategory = NOTE_CATEGORY_4B;
            break;
        case 0x40:
            arryMediaInfo[i].iNoteCategory = NOTE_CATEGORY_4;
            break;
        case 0x30:
            arryMediaInfo[i].iNoteCategory = NOTE_CATEGORY_3;
            break;
        case 0x20:
            arryMediaInfo[i].iNoteCategory = NOTE_CATEGORY_2;
            break;
        case 0x10:
            arryMediaInfo[i].iNoteCategory = NOTE_CATEGORY_1;
            break;
        default:
            arryMediaInfo[i].iNoteCategory = NOTE_CATEGORY_UNKNOWN;
        }
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return usNumNotes;
}

//功能：PC与ZERO BV通信开始，通知HCM不访问BV，获取冠字码信息前调用
//输入：无
//输出：无
//0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：20
int CUR2Drv::BVCommStart()
{
    const char *ThisModule = "BVCommStart";

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[14] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x7C00);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_20, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x7C00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x7C01:
        iRet = ERR_UR_WARN;
        break;
    case 0x7C80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x7CFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：PC与ZERO BV通信结束，获取冠字码信息结束后调用
//输入：无
//输出：无
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：20
int CUR2Drv::BVCommEnd()
{
    const char *ThisModule = "BVCommEnd";

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[14] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x7C01);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_20, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x7C00:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x7C01:
        iRet = ERR_UR_WARN;
        break;
    case 0x7C80:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x7CFF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：准备进入一下笔交易
//      在取款、存款交易结束后调用该命令
//      UR2检测所有单元是否在原始位置并且无钞票残留
//      UR2关掉传感器以及设备，切换到节能模式
//输入：无
//0 成功，>0 警告，<0 失败；见返回码定义
//说明：：
int CUR2Drv::PreNextTransaction()
{
    const char *ThisModule = "PreNextTransaction";

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[14] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x6000);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_180, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x6000:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x6080:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x60FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：开始存取款交易
//      在取款、存款交易开始前调用该命令
//      UR2打开传感器以及设备，从节能模式中恢复
//输入：无
//0 成功，>0 警告，<0 失败；见返回码定义
//说明：
int CUR2Drv::StartTransaction()
{
    const char *ThisModule = "StartTransaction";

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 bytes)
    char szCmdData[14] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0x6001);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70,  HT_RESP_TIMEOUT_180, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;
    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }
    switch (usRespCode)
    {
    case 0x6000:
        iRet = ERR_UR_SUCCESS;
        break;  //Normal End
    case 0x6080:
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case 0x60FF:
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //异常结束
    if (iRet < 0)
    {
        Log(ThisModule, -1, "%s命令异常结束,错误码为0x%04X", ThisModule, usRespCode);
        return iRet;
    }

    return iRet;
}

//功能：获得设备状态信息
//输入：无
//输出：stDevStatusInfo：解析后的状态信息
//返回：无
void CUR2Drv::GetDevStatusInfo(ST_DEV_STATUS_INFO &stDevStatusInfo)
{
    //    pthread_mutex_lock(&m_mutex_status);
    AutoMutexStl(m_MutexStatus);
    stDevStatusInfo = m_stDevStatusInfo;
    //    pthread_mutex_unlock(&m_mutex_status);
}

// 功能：获取错误返回码
// 输入：无
// 输出：stErrDetail：解析后的错误信息
// 返回：8字节常量字符串
// 说明：若指令正常结束则返回空字串("")
//       若异常结束则返回7字节的错误码字符串，如："05130A03"
//       若警告结束则返回8字节警告码字符串，  如："81xxxxxx"
void CUR2Drv::GetLastErrDetail(ST_ERR_DETAIL &stErrDetail)
{
    //  EnterCriticalSection(&m_CritSectErrDetail);
    memcpy(&stErrDetail, &m_stErrDetail, sizeof(ST_ERR_DETAIL));
    //  LeaveCriticalSection(&m_CritSectErrDetail);
}

// 功能：将响应的错误或警告码转换为对应的错误描述
// 输入：arycErrCode: 响应的错误代号,请传入GetLastErrCode()返回的字符串
// 输出：无
// 返回：成功：返回arycErrCode相关的错误描述，
//       失败：如arycErrCode错误代号未知则返回空串：""；如参数为NULL返回NULL
const char *CUR2Drv::GetErrDesc(const char *arycErrCode)
{
    //详细的错误代号请查看文件《error display manual》
    return  CErrCodeMap::GetInstance()->GetErrDescrStr(arycErrCode);
}

//////////////////////////////////////////////////////////////////////////

//功能：初始化成员变量m_mPacketIDLength，map不为空时将PacketID与包长填充
//输入：无
//输出：无
//返回：无
//说明：无
void CUR2Drv::InitMapPacketID()
{
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_CMND,                 0x0004));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_RESP,                 0x0004));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_COMMONBLOCK,          0x0020));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_OPINFO,               0x0008));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_HWINFO,               0x0008));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_DENOCODESET,          0x0102));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_CASSDENOCODE,         0x0144));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_ACCEPTDENOCODE,       0x0014));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_CASSTYPE,             0x0022));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_VERLEVELCASHCOUNT,    0x0014));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_VERLEVELSTORE,        0x0014));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_VERLEVELDISP,        0x0014));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_NOTEHANDINFO,         0x0024));

    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_USERMEMORYDATA,       0x0084));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_STATUSINFO,           0x001A));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_SPECIFICFUNC,         0x0004));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_MAINTENINFO,          0x1002));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_NUMTOTALSTACKEDNOTES, 0x003E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_NUMSTACKEDNOTES,      0x003E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_NUMFEDNOTES,          0x0022));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_NUMREJNOTES,          0x0004));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_BANKNOTETABLE,        0x07F2));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_CASSCONFIG,           0x00FC));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_VALIDATORID,          0x0012));

    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FW_UR2BOOT,          0x001E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FW_FPGA,              0x001E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FW_UR2MAIN,          0x001E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FW_AUTHENTICAT,       0x001E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FW_BV1,               0x001E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FW_BV2,               0x001E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FW_BV3,               0x001E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FW_BV4,               0x001E));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FW_VT,               0x0012));

    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_UNFITNUMDENO_16,      0x0042));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_UNFITNUMDENO_32,      0x0082));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_UNFITNUMDENO_64,      0x0102));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_UNFITNUMDENO_128,     0x0202));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_BVWARNINFO,           0x007A));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_NUMDISPNOTESROOM,     0x001A));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_REJECTNUMDENOSOURCE,  0x0042));

    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_REJECTNUMDENOBV_16,   0x0042));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_REJECTNUMDENOBV_32,   0x0082));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_REJECTNUMDENOBV_64,   0x0102));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_REJECTNUMDENOBV_128,  0x0202));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_REJECTNUMDENO_16,     0x0042));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_REJECTNUMDENO_32,     0x0082));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_REJECTNUMDENO_64,     0x0102));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_REJECTNUMDENO_128,    0x0202));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_STACKENUMDENO_16,     0x0042));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_STACKENUMDENO_32,     0x0082));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_STACKENUMDENO_64,     0x0102));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_STACKENUMDENO_128,    0x0202));

    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_MISSFEEDROOM,         0x0004));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_FEDNOTECONDITION,     0x0004));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_SECURITYCASS,         0x0004));

    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_LOG_SENSOR,           0x1C04));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_LOG_INTERNALCOMMAND,  0x1C04));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_LOG_NOTESHAND,        0x1C04));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_LOG_MOTORCONTR,       0x1C04));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PAKCET_ID_LOG_DATAOTHERS,       0x1C06));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PAKCET_ID_LOG_SESORLEVEL,       0x0872));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PAKCET_ID_LOG_NOTESIDENTIF,     0x1812));

    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_SENSORINFO,     0x000A));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_TESTRESULT1,     0x0102));
    m_mPacketIDLength.insert(pair<USHORT, USHORT>(PACKET_ID_TESTRESULT2,     0x0402));

    return;
}

//功能：执行SendData和RecvData() for CSimpleThread by liml 2012-4-20
//参数：pParam，指向ThreadParm结构体的指针，具体成员
unsigned int CUR2Drv::Run(LPVOID pParam)
{
    /*
    ThreadParm  *parm = (ThreadParm *)pParam;
    int iRet = ERR_UR_SUCCESS;
    DWORD dwReceivedLen = 0;
    memset(parm->szResp, '\0', parm->dwRespLen);

    iRet = parm->pConnect->SendAndRecvData(
        parm->pszCmd, parm->iCmdLen, parm->szResp, parm->dwRespLen,
        dwReceivedLen,
        HT_SEND_TIMEOUT_20,
        parm->iTimeout,
        parm->pszFuncName,
        parm->bPDL);
    if (ERR_UR_SUCCESS != iRet)
    {
        parm->nRet = iRet;
        return 0;
    }

    if ( (dwReceivedLen < parm->dwMinRespLen) && (parm->pConnect->GetConnectType() == CONNECT_TYPE_UR))
    {
        Log(parm->pszFuncName, -1,
            "%s指令执行失败，获取响应数据长度(%d)错误,小于要求长度(%d)",
            parm->pszFuncName, dwReceivedLen, parm->dwMinRespLen);
        parm->nRet = iRet;
        return 0;
    }

    parm->dwRespLen = dwReceivedLen;
    parm->nRet = ERR_UR_SUCCESS;
    */
    return 0;
}

// 功能：执行pszCmd命令，响应数据返回给pszResp
// 参数：
// 输入：
//      eConnect: USB连接的设备类型
//      pszSendData：发送数据地址
//      dwSendLen：发送数据长度
//      dwRecvedLenInOut：接收数据地址缓存大小
//      dwReqRecvLen: 期望接收数据长度
//      dwRecvTimeout：接收数据超时
//      pFunName：发送命令名称
//      bPDL：是否为下载固件
// 输出：
//      pszRecvData：接收数据地址
//      dwRecvedLenInOut：实际接收数据长度
// 返回：
//  0 ： 成功
//  <0:  失败
int CUR2Drv::ExecuteCmd(const char *pszSendData,
                        USHORT dwSendLen,
                        char *pszRecvData,
                        USHORT &dwRecvedLenInOut,
                        USHORT dwReqRecvLen,
                        USHORT dwRecvTimeout,
                        const char *lpFunName,
                        CONNECT_TYPE eConnectType,
                        bool bPDL)
{
    //THISMODULE(__FUNCTION__);
    //AutoMutexStl(m_MutexAction);
    DWORD dwRevLenInOut = dwRecvedLenInOut;
    long lRes = m_pUSBDrv->SendAndRecvData(eConnectType, pszSendData, dwSendLen, HT_SEND_TIMEOUT_5,
                                           pszRecvData, dwRevLenInOut, dwRecvTimeout, lpFunName, bPDL);
    //校验执行指令返回数据长度，如实际小于最小长度，记录日志并返回错误
    if ((dwRecvedLenInOut < dwReqRecvLen) && (eConnectType == CONNECT_TYPE_UR))
    {
        Log(lpFunName, ERR_UR_RETURN_DATA_LEN, "返回数据长度%d小于最小长度%d", dwRevLenInOut, dwReqRecvLen);
        return ERR_UR_RETURN_DATA_LEN;
    }

    //校验返回数据合法性
    USHORT usTotalRespLen = MAKEWORD(pszRecvData[1], pszRecvData[0]);
    if (!bPDL && (eConnectType != CONNECT_TYPE_ZERO))
    {
        if (static_cast<DWORD>(usTotalRespLen) != dwRevLenInOut)
        {
            Log(lpFunName, -1, "返回的数据长度不合法, 实际返回长度为%d, 解析响应数据应返回长度为%d", dwRevLenInOut, usTotalRespLen);
            return ERR_UR_RETURN_DATA_LEN;
        }
    }
    //Log(lpFunName, lRes, "%s 执行完成iRet:%d", lpFunName, lRes);
    return lRes;
}


/*
long CUR2Drv::GetLocalSystemTime(SYSTEMTIME &stSystemTime)
{
    timeval tv;
    gettimeofday(&tv, NULL);
    time_t tt = time(NULL);
    tm temp = *localtime(&tt);

    stSystemTime.year = 1900 + temp.tm_year;
    stSystemTime.month = 1 + temp.tm_mon;
    stSystemTime.day_of_week = temp.tm_wday;
    stSystemTime.day = temp.tm_mday;
    stSystemTime.hour = temp.tm_hour;
    stSystemTime.minute = temp.tm_min;
    stSystemTime.second = temp.tm_sec;
    stSystemTime.milli_seconds = tv.tv_usec / 1000;

    return 0;
}*/

//功能：获取所有感应器的亮灭状态
//输入：iLen: pszLightData所分配的内存空间，该值需大于48
//输出：pszLightData: 所有感应器的亮灭状态数据，48字节，每1 bit表示一个感应器
//返回：0 成功，>0 警告，<0 失败；见返回码定义
//说明：执行命令的最大时间(秒)：20
int CUR2Drv::ReadAllSensorLight(char *pszLightData, int &iLen)
{
    const char *ThisModule = "ReadAllSensorLight";

    //拼接命令数据
    USHORT iCmdLen = 2; //Total Message Length(itself 2 chars)
    char szCmdData[HT_CMD_LEN_ZERO + 1] = {0};
    MOSAICCMND(szCmdData, iCmdLen, 0xA000);
    iCmdLen += 6;

    char szCmdParam[64 + 1] = {0};
    //2.ACT (6 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x00;
    szCmdParam[1] = 0x02;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x04;
    szCmdParam[4] = 0x04;
    szCmdParam[5] = 0x01;
    memcpy(szCmdData + iCmdLen, szCmdParam, 6);
    iCmdLen += 6;

    //Sensor Test mode (6 bytes)
    memset(szCmdParam, 0, sizeof(szCmdParam));
    szCmdParam[0] = 0x05;
    szCmdParam[1] = 0x76;
    szCmdParam[2] = 0x00;
    szCmdParam[3] = 0x04;
    szCmdParam[4] = 0x01;
    szCmdParam[5] = 0x00;
    memcpy(szCmdData + iCmdLen, szCmdParam, 6);
    iCmdLen += 6;

    szCmdData[0] = static_cast<char>((iCmdLen >> 8) & 0xFF);
    szCmdData[1] = static_cast<char>(iCmdLen & 0xFF);

    //发送并接收响应
    char szResp[HT_RESP_LEN_MAX] = {0};
    USHORT dwRespLen = HT_RESP_LEN_MAX; // 输入：缓冲区长度，输出：实际接收响应字节数
    int iRet = ExecuteCmd(szCmdData, iCmdLen, szResp, dwRespLen, 70, HT_RESP_TIMEOUT_60, ThisModule);
    if (ERR_UR_SUCCESS != iRet)
    {
        Log(ThisModule, iRet, "ExecuteCmd(%s) failed", ThisModule);
        return iRet;
    }

    PACKETMAP mapPacket;
    PACKETMAP::iterator it;
    int iMinPacketCount = 3;
    if (!CheckRespEachPacket(szResp, iMinPacketCount, mapPacket, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    //分析RESP包，如果为无效命令或数据解析异常直接返回
    USHORT usRespCode = 0;

    if (!ParseRESP(szResp, mapPacket, usRespCode, ThisModule))
    {
        return ERR_UR_RESPDATA_ERR;
    }

    switch (usRespCode)
    {
    case static_cast<USHORT>( 0xA000 ):
        iRet = ERR_UR_SUCCESS;
        break;
    case static_cast<USHORT>( 0xA040 ):
        iRet = ERR_UR_PARAM;
        break;  //Undefined ACT
    case static_cast<USHORT>( 0xA080 ):
        iRet = ERR_UR_HARDWARE_ERROR;
        break;  //Abnormal End
    case static_cast<USHORT>( 0xA0FF ):
        iRet = ERR_UR_INVALID_COMMAN;
        break;  //Invalid Command
    default:
        Log(ThisModule, iRet, "数据解析错误, ErrCode%X is Not Defined", usRespCode);
        return ERR_UR_RESPDATA_ERR;
    }

    //分析响应数据,保存状态和错误信息
    SaveErrDetail(szResp, mapPacket, ThisModule);
    SaveStatusInfo(szResp, mapPacket, ThisModule);

    if (ERR_UR_INVALID_COMMAN == iRet)
    {
        Log(ThisModule, iRet, "Invalid Command");
        return ERR_UR_INVALID_COMMAN; //Invalid Command
    }

    //Sensor Info (12bytes)
    int iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_SENSORINFO, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }
    //int nACT = (int)(((UCHAR)szResp[iDataPos] << 8) + (UCHAR)szResp[iDataPos + 1]);
    //int nSensorTestMode = (int)((UCHAR)szResp[iDataPos + 2]);
    UCHAR ucSensorInfo[100] = {0};
    memcpy(ucSensorInfo, szResp + iDataPos + 4, 4);

    //Test Results 2 (1028bytes)
    iDataPos = FindMapPacketDataPosforID(mapPacket, PACKET_ID_TESTRESULT2, ThisModule);
    if (iDataPos <= 0)
    {
        return ERR_UR_RESPDATA_ERR;
    }

    memcpy(pszLightData, szResp + iDataPos, 58);
    iLen = 58;

    return 0;
}


//功能：调整BV验钞级别
//输入：ucVerificationLevel : 0~255表示 检验级别，数值越高越宽松
//输出：无
//返回：0 成功，>0 警告，<0 失败
int  CUR2Drv::SetBVVerificationLevel(char ucVerificationLevel)
{
    const char *ThisModule = "SetBVVerificationLevel";

    return 0;
}

//功能：对变造币进行拒钞处理
//输入：
//输出：
//返回：0 成功，>0 警告，<0 失败
int  CUR2Drv::SetRejectCounterfeitNote()
{
    const char *ThisModule = "SetRejectCounterfeitNote";
    /*
        UCHAR szCmdData[82+1] =  {
            0x00, 0x52, 0x00, 0x00, 0x00, 0x07, 0x01, 0x00, 0x00,  0x00, 0x00,  0x0E, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x43, 0x4E, 0x59, 0x2A, 0x2A, 0x2A, 0x2A, 0x42, 0x20, 0x00, 0x08, 0x00,
            0x00, 0x00 };


        char szResp[36+1] = {0};
        DWORD dwRespLen = 0; // 实际接收响应字节数

        int iRet = ExecuteCmd(szCmdData, 82,szResp, dwRespLen, 36,  HT_SEND_TIMEOUT_1, ThisModule, CONNECT_TYPE_ZERO);
        if (ERR_UR_SUCCESS != iRet)
        {
            Log(ThisModule,iRet, "ExecuteCmd failed :%s", ThisModule);
            return iRet;
        }

        if (szResp[4] == 0x10)
        {
            return ERR_UR_SUCCESS;
        }
        else if (szResp[4] == 0x08)
        {
            return ERR_UR_HARDWARE_ERROR;
        }
        return iRet;
        */
    return 0;
}

//功能：设置不适合流通人民币验钞处理级别
//输入：CmdType ：设置后即将执行的操作动作; bSevereModel: 是否使用更严格的模式
//输出：
//返回：0 成功，>0 警告，<0 失败
int  CUR2Drv::SetUnfitNoteVerifyLevel(eCMDType CmdType, BOOL bSevereModel)
{
    const char *ThisModule = "SetUnfitNoteVerifyLevel";

    /*
    unsigned char szCmdData[92] = {
            0x00, 0x5C, 0x00, 0x00, 0x00, 0x07, 0x01, 0x00, 0x00, 0x00, 0x00, 0x12, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
            0x00, 0x00, 0x00, 0x00, 0x43, 0x4E, 0x59, 0x2A, 0x2A, 0x2A, 0x2A, 0x42, 0x20, 0x00, 0x07, 0x00,
            0x04, 0x00, 0x08, 0x00, 0x02, 0x00, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00 };

    unsigned char szStandardDataCashCount[12]            = {0xFF, 0x04, 0x7F, 0x08, 0xFF, 0x02, 0x1E, 0x03, 0x19, 0x06, 0xFF, 0x00};
    unsigned char szStandardDataCashStoremoney[12] = {0x50, 0x04, 0x7F, 0x08, 0x00, 0x02, 0x1E, 0x03, 0x19, 0x06, 0x50, 0x00};
    unsigned char szStandardDataCashDispense[12]       = {0x50, 0x04, 0x7F, 0x08, 0x00, 0x02, 0x1E, 0x03, 0x19, 0x06, 0x50, 0x00};
    unsigned char szSevereDataCashCount[12]                = {0xFF, 0x04, 0x7F, 0x08, 0xFF, 0x02, 0x14, 0x03, 0x0A, 0x06, 0xFF, 0x00};
    unsigned char szSevereDataCashStoremoney[12]     = {0x32, 0x04, 0x7F, 0x08, 0x00, 0x02, 0x14, 0x03, 0x0A, 0x06, 0x32, 0x00};
    unsigned char szSevereDataCashDispense[12]           = {0x32, 0x04, 0x7F, 0x08, 0x00, 0x02, 0x14, 0x03, 0x0A, 0x06, 0x32, 0x00};
        switch(CmdType)
        {
        case eCMDType_CashCount:
            {
                if (bSevereModel)
                {
                    memcpy(&szCmdData[79], szSevereDataCashCount, 12);
                }
                else
                {
                    memcpy(&szCmdData[79], szStandardDataCashCount, 12);
                }
                break;
            }

        case eCMDType_StoreMoney:
            {
                if (bSevereModel)
                {
                    memcpy(&szCmdData[79], szSevereDataCashStoremoney, 12);
                }
                else
                {
                    memcpy(&szCmdData[79], szStandardDataCashStoremoney, 12);
                }
                break;
            }
        case eCMDType_Dispense:
            {
                if (bSevereModel)
                {
                    memcpy(&szCmdData[79], szSevereDataCashDispense, 12);
                }
                else
                {
                    memcpy(&szCmdData[79], szStandardDataCashDispense, 12);
                }
                break;
            }
        default:
            {
                Log(ThisModule, -1 , "非法的操作类型 :%d", CmdType);
                return 1;
            }
        }

        //日立文档中有错误，上边写的返回数据长度为36但会出错
        char szResp[256] = {0};
        DWORD dwRespLen = 0; // 实际接收响应字节数

        int iRet = ExecuteCmd(szResp, dwRespLen, 256, szCmdData, 92, HT_SEND_TIMEOUT_1, ThisModule, CONNECT_TYPE_ZERO);
        if (ERR_UR_SUCCESS != iRet)
        {
            Log(ThisModule,iRet, "ExecuteCmd failed :%s", ThisModule);
            return iRet;
        }

        if (szResp[4] == 0x10)
        {
            return ERR_UR_SUCCESS;
        }
        else if (szResp[4] == 0x08)
        {
            return ERR_UR_HARDWARE_ERROR;
        }
        return iRet;
        */
    return 0;
}

int CUR2Drv::SetComVal(const char * strCashInShutterPort)
{
    m_strCashInShutterPort = (string)strCashInShutterPort;
}

