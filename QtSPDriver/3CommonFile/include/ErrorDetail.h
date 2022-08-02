/***************************************************************
* 文件名称: ErrorCode.h
* 文件描述: 声明SP错误码及处理接口
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2022年5月18日
* 文件版本: 1.0.0.1
****************************************************************/
#ifndef ERRORCODE_H
#define ERRORCODE_H

#include "QtTypeDef.h"
#include "data_convertor.h"

/*********************************************************************************
 * 错误码字符串用于 STATUS中ErrDetail/LaseErrDetail显示
 * 错误码字符串长度12Byte, 分为3节, 每节具体分类如下：
 *    Byte 1: 用于标记所属模块, 参考[struct stSPName 结构体]
 *    Byte 2~4: 用于标记XFS_XXX/DevXXX层出现的错误, 每一位有效标记1~F, 分类如下:
 *         101~1FF: 标记XFS_XXX层通用错误码, 各SP模块都适用, 范围划分如下:
 *             (1). 库加载错误相关(101~A0F)
 *             (2). 参数检查错误相关(110~12F)
 *             (3). 设备错误相关(130~14F)
 *             (4). 介质错误相关,用于卡/身份证/存折/凭条/票据等(150~16F)
 *             (5). 箱体错误相关,用于钞箱/回收盒/发卡箱等部件(170~18F)
 *             (6). 其他错误(190~19F)
 *             (7). 110~1FF保留
 *         201～2FF: 标记XFS_XXX层模块内错误码, 适用各SP模块不相同的错误码, 各SP模块自行定义
 *         301~3FF: 标记DevXXX层通用错误码, 各SP模块都适用, 范围划分如下:
 *             (1). 库加载错误相关(101~A0F)
 *             (2). 参数检查错误相关(110~12F)
 *             (3). 设备错误相关(130~14F)
 *             (4). 介质错误相关,用于卡/身份证/存折/凭条/票据等(150~16F)
 *             (5). 箱体错误相关,用于钞箱/回收盒/发卡箱等部件(170~18F)
 *             (6). 其他错误(190~19F)
 *             (7). 110~1FF保留
 *         401～4FF: 标记DevXXX层模块内错误码, 适用各SP模块不相同的错误码, 各SP模块自行定义
 *    Byte 5~12: 用于标记硬件或厂商SDK返回的错误, 该节有两种记录方式, 如下:
 *             (1). 对于支持多类硬件的模块, Byte5用于指定硬件编号(暂定各模块INI中定义设备类型)
 *             (2). 对于只支持一类硬件的模块, 可使用全部8Byte或自由设置
 *        注: 该节有效标记为[0~F], 避免使用特殊符号.
 *
 * 注: 除Byte1以外, 每一位有效标记为16进制字符形式, 如: 0 1 2 3 4 5 6 7 8 9 A B C D E F
 *
*********************************************************************************/



//*********************************************************************************
// ***************  所有模块 共通 错误码定义(该部分添加需考虑各模块是否共通) **************
//*********************************************************************************
//--------------------------XFS相关--------------------------
// XFS 共通 错误码定义(101~A0F): 库加载错误相关
#define EC_XFS_SPBaseNotFound               "101"               // libSPBaseXXX.so未找到
#define EC_XFS_SPBaseLoadFail               "102"               // libSPBaseXXX.so加载失败
#define EC_XFS_DevXXXNotFound               "103"               // libDevXXX.so未找到
#define EC_XFS_DevXXXLoadFail               "104"               // libDevXXX.so加载失败

// XFS 共通 错误码定义(110~12F): 参数检查错误相关
#define EC_XFS_ParInvalid                   "110"               // 无效参数/不支持的参数

// XFS 共通 错误码定义(130~14F): 设备错误相关
#define EC_XFS_DevNotSupp                   "130"               // INI指定设备类型不支持
#define EC_XFS_DevOpenFail                  "131"               // 设备Open错误
#define EC_XFS_DevNotFound                  "132"               // 设备不存在
#define EC_XFS_DevOffLine                   "133"               // 设备断线
#define EC_XFS_DevPowerOff                  "134"               // 设备电源关闭或未接通
#define EC_XFS_DevUserErr                   "135"               // 人为原因的设备故障
#define EC_XFS_DevHWErr                     "136"               // 设备故障
#define EC_XFS_DevFraud                     "137"               // 发现欺诈动作

// XFS 共通 错误码定义(150~16F): 介质错误相关: 卡/身份证/存折/凭条/票据等
#define EC_XFS_MedNotSupp                   "150"               // 介质不支持
#define EC_XFS_MedInvalid                   "151"               // 无效介质
#define EC_XFS_MedJammed                    "152"               // 介质JAM
#define EC_XFS_MedNotFound                  "153"               // 介质不存在
#define EC_XFS_MedIsExport                  "154"               // 介质在出口

// XFS 共通 错误码定义(170~18F): 箱体错误相关: 钞箱/回收盒/发卡箱等
#define EC_XFS_BoxNotFound                  "170"               // 箱体不存在
#define EC_XFS_BoxMissing                   "171"               // 箱体丢失
#define EC_XFS_BoxHWErr                     "172"               // 箱体故障
#define EC_XFS_BoxFull                      "173"               // 箱体满
#define EC_XFS_BoxEmpty                     "174"               // 箱体空
#define EC_XFS_RetainNoSupp                 "175"               // 不支持回收

// XFS 共通 错误码定义(190~19F): 其他错误相关:
#define EC_XFS_UnKnownErr                   "190"               // 未知错误
#define EC_XFS_ShareMemCrt                  "191"               // 共享内存建立错误
#define EC_XFS_ShareMemRW                   "192"               // 共享内存读写错误
#define EX_XFS_FilePathCrtErr               "193"               // 文件目录创建失败
#define EX_XFS_MemoryApply                  "194"               // 内存空间申请失败
#define EX_XFS_CodeChange                   "195"               // 数据格式转换失败
#define EX_XFS_ConnInvalid                  "196"               // 无效的连接


//--------------------------DevXXX相关--------------------------

// DevXXX 共通 错误码定义(301~30F): 参数/数据检查错误相关
// 用于检查接口入参, 下发到硬件或接收来自硬件的数据
#define EC_DEV_ParInvalid                   "310"               // DevXXX接口不支持的参数
#define EC_DEV_HWParInvalid                 "311"               // 硬件接口不支持的参数
#define EC_DEV_RecvData_inv                 "312"               // 接收到硬件返回的数据无效
#define EC_DEV_ReadData_NotComp             "313"               // 接收到硬件返回的数据不完整

// DevXXX 共通 错误码定义(310~32F): 设备错误相关
#define EC_DEV_DevOpenFail                  "310"               // 设备Open错误
#define EC_DEV_DevNotFound                  "311"               // 设备不存在
#define EC_DEV_DevOffLine                   "312"               // 设备断线
#define EC_DEV_DevHWErr                     "313"               // 设备故障
#define EC_DEV_DevFraud                     "314"               // 发现欺诈操作
#define EC_DEV_DevDestroy                   "315"               // 发现破坏操作
#define EC_DEV_DevNotSupp                   "316"               // 设备不支持该功能

// DevXXX 共通 错误码定义(330~35F): 介质错误
#define EC_DEV_MedNotSupp                   "330"               // 介质不支持
#define EC_DEV_MedInvalid                   "331"               // 无效介质
#define EC_DEV_MedJammed                    "332"               // 介质JAM
#define EC_DEV_MedNotFound                  "333"               // 介质不存在
#define EC_DEV_MedIsHave                    "334"               // 介质已存在
#define EC_DEV_MedRWFail                    "335"               // 介质读写错误
#define EC_DEV_MedCommFail                  "336"               // 介质通信错误

// DevXXX 共通 错误码定义(360~37F): 通信问题错误码
#define EC_DEV_ConnFail                     "360"               // 连接失败
#define EC_DEV_CommErr                      "361"               // 通信错误
#define EC_DEV_CommTimeOut                  "362"               // 通信超时
#define EC_DEV_DataRWErr                    "363"               // 数据读写错误
#define EC_DEV_SDKFail                      "364"               // SDK调用错误
#define EX_DEV_InTerExec                    "365"               // 接口执行错误

// DEV 共通 错误码定义(380~39F): 其他错误相关:
#define EC_DEV_LibraryNotFound              "380"               // 动态库未找到
#define EC_DEV_LibraryLoadFail              "381"               // 动态库库加载失败
#define EC_DEV_UnKnownErr                   "382"               // 未知错误
#define EC_DEV_OtherErr                     "383"               // 其他错误
#define EC_DEV_HWErr                        "384"               // 硬件报错
#define EC_DEV_Unsup_CMD                    "385"               // 不支持的命令或接口
#define EC_DEV_TimeOut                      "386"               // 超时
#define EC_DEV_ShareMemCrt                  "387"               // 共享内存建立错误
#define EC_DEV_ShareMemRW                   "388"               // 共享内存读写错误
#define EC_DEV_FilePathCrtErr               "380"               // 文件目录创建失败
#define EC_DEV_MemoryErrr                   "381"               // 内存错误



//*********************************************************************************
// ************ 以下为 各SP模块错误码 自行定义(该部分添加避免与共通部分有重复) ************
//*********************************************************************************



//--------------------------------------------------------------------------
// IDC 错误码定义
// 读卡器有加载返卡/发卡的情况, 错误码定义也包含返卡CRM和发卡CRD
// 适用模块: IDC/FIDC/IDX/MSR/CRM/CRD
//--------------------------------------------------------------------------

// 读卡器: XFS_IDC 错误码定义(201～2FF):
#define EC_IDC_XFS_libIDCNotFound           "201"               // libDevIDC.so动态库未找到
#define EC_IDC_XFS_libIDCLoadFail           "202"               // libDevIDC.so动态库库加载失败
#define EC_IDC_XFS_CRMOffLine               "203"               // 读卡器返卡同时断线
#define EC_IDC_XFS_PortNotSupp              "204"               // 不支持的Chip协议
#define EC_IDC_XFS_ChipPowerOff             "205"               // 芯片卡未通电
#define EC_IDC_XFS_ChipUnknown              "206"               // 芯片卡状态未知
#define EC_IDC_XFS_ChipNotSupp              "207"               // 不支持芯片卡操作

// 读卡器: DevIDC 错误码定义(401~4FF):
#define EC_IDC_DEV_TamperRelease            "401"               // 防盗钩释放错误
#define EC_IDC_DEV_TamperNotRelease         "402"               // 防盗钩未释放
#define EC_IDC_DEV_TeaseCard                "403"               // 防逗卡保护中


// 返卡模块: CRM 错误码定义(501~5FF):
// XFS_CRM错误码定义(501~57F):
#define EC_CRM_XFS_DevCRMNotFound           "501"               // libDevCRM.so未找到
#define EC_CRM_XFS_DevCRMLoadFail           "502"               // libDevCRM.so加载失败
#define EC_CRM_XFS_ININotSupp               "5O1"               // INI指定不支持返卡设备
#define EC_CRM_XFS_DevNotSupp               "504"               // 返卡设备不支持
#define EC_CRM_XFS_DevOpenFail              "505"               // 返卡设备Open错误
#define EC_CRM_XFS_DevNotOpen               "506"               // 返卡设备未Open
#define EC_CRM_XFS_DevOffLine               "507"               // 返卡设备断线
#define EC_CRM_XFS_IDCHaveMed               "518"               // 读卡器有卡无法执行返卡操作
#define EC_CRM_XFS_CardNoInv                "519"               // 返卡设备指定卡号规则无效
#define EC_CRM_XFS_CardNotFound             "510"               // 返卡设备指定卡不存在

// DevCRM错误码定义(580~5FF):
#define EC_CRM_DEV_SlotPosAbnormal          "581"               // 暂存仓暂存仓位置异常
#define EC_CRM_DEV_StatInvalid              "582"               // 状态无效
#define EC_CRM_DEV_SlotNoInvalid            "583"               // 指定暂存仓编号无效


//--------------------------------------------------------------------------
// CAM 错误码定义
// 适用模块: CAM/HCAM
//--------------------------------------------------------------------------
// 摄像: XFS_CAM 错误码定义(201～2FF):
#define EC_CAM_XFS_libCAMNotFound           "201"               // libDevCAM.so动态库未找到
#define EC_CAM_XFS_libCAMLoadFail           "202"               // libDevCAM.so动态库库加载失败
#define EC_CAM_XFS_FilePathInv              "203"               // 指定文件路径无效
#define EC_CAM_XFS_NotDisplay               "204"               // 摄像窗口未打开

// 摄像: DevCAM 错误码定义(401~4FF):
#define EC_CAM_DEV_VideoIdxNotFount         "401"               // 摄像设备号未找到
#define EC_CAM_DEV_GetImageErr              "402"               // 取图像数据失败
#define EC_CAM_DEV_SaveImageErr             "403"               // 保存图像失败
#define EC_CAM_DEV_NotLiveDetect            "403"               // 未检测到活体


//****************************************************************************
// IDevIDC定义 设备相关变量转换 类定义(统一转换)
//****************************************************************************
// 错误码长度
#define EC_ALL_LEN                          12                  // 错误码总长度
#define EC_XFS_LEN                          3                   // XFS_XXX错误码长度
#define EC_DEV_LEN                          3                   // DevXXX错误码长度
#define EC_HW_LEN                           8                   // 硬件返回错误码长度

static struct
{
    CHAR szSerial[2];
    CHAR szName[12];
} stSPName[] = {
                { "1", "BCR" },  { "2", "BRM" },  { "3", "CAM" },  { "4", "COR" },
                { "5", "CPR" },  { "6", "CSR" },  { "7", "DEP" },  { "8", "DPR" },
                { "9", "DSR" },  { "A", "FIDC" }, { "B", "FIG" },  { "C", "IDC" },
                { "D", "IDX" },  { "E", "JPR" },  { "F", "MSR" },  { "G", "PIN" },
                { "H", "PPR" },  { "I", "PTR" },  { "J", "SIG" },  { "K", "SIU" },
                { "L", "UKEY" }, { "M", "VDM" },  { "N", "HCAM"}
               };

class CErrorDetail
{
public:
    // 入参: 指定模块名或者SP定义设备编号
    // XFS_XXX传入 stSPName.szName指定的模块名
    // DevXXX传入对应模块INI定义的设备类型(1Byte)
    // stSPName.szName优先检查,不符合默认为DevXXX方式参数
    CErrorDetail(LPSTR lpSPName = nullptr)
    {
        MSET_0(m_szSPErrCode);
        MSET_0(m_szSPErrCodeLast);
        MSET_XS(m_szSPErrCode, 0x30, EC_ALL_LEN);
        MSET_XS(m_szSPErrCodeLast, 0x30, EC_ALL_LEN);
        MSET_0(m_szXFSNameID);
        MSET_0(m_szDevNameID);

        if (lpSPName == nullptr)
        {
            return;
        }

        for (INT i = 0; i < (sizeof(stSPName)/sizeof(stSPName[0])); i ++)
        {
            if (MCMP_IS0(lpSPName, stSPName[i].szName))
            {
                m_szSPErrCode[0] = stSPName[i].szSerial[0];
                m_szXFSNameID[0] = stSPName[i].szSerial[0];
                break;
            }
        }
        MCPY_NOLEN(m_szSPErrCodeInit, m_szSPErrCode);
    }
    ~CErrorDetail() {}

public:
    // 返回SP错误码
    CHAR* GetSPErrCode(LPSTR lpErrCode = nullptr)
    {
        if (lpErrCode != nullptr)
        {
            MCPY_LEN(lpErrCode, m_szSPErrCode, strlen(m_szSPErrCode));
        }
        return m_szSPErrCode;
    }
    // 返回上一次SP错误码
    CHAR* GetSPErrCodeLast(LPSTR lpErrCode = nullptr)
    {
        if (lpErrCode != nullptr)
        {
            MCPY_LEN(lpErrCode, m_szSPErrCodeLast, strlen(m_szSPErrCodeLast))
        }
        return m_szSPErrCodeLast;
    }
    // 返回Dev+HW错误码
    CHAR* GetDevHWErrCode(LPSTR lpErrCode = nullptr)
    {
        return GetSPErrCode(lpErrCode);
    }
    // 初始化错误码
    INT SetErrCodeInit(LPSTR lpCode = nullptr)
    {        
        if (lpCode != nullptr)
        {
            if (!(MCMP_IS0(m_szSPErrCode, lpCode)))
            {
                MSET_0(m_szSPErrCodeLast);
                MCPY_NOLEN(m_szSPErrCodeLast, m_szSPErrCode);
                MSET_0(m_szSPErrCode);
                MCPY_NOLEN(m_szSPErrCode, lpCode);
            }
        } else
        {            
            if (!(MCMP_IS0(m_szSPErrCode, m_szSPErrCodeInit)))
            {
                MSET_0(m_szSPErrCodeLast);
                MCPY_NOLEN(m_szSPErrCodeLast, m_szSPErrCode);
                MCPY_NOLEN(m_szSPErrCode, m_szSPErrCodeInit);
            }
        }
        return 0;
    }
    // 设置XFS分段错误码
    INT SetXFSErrCode(LPSTR lpCode)
    {
        if (lpCode != nullptr)
        {
            if (MCMP_IS0((m_szSPErrCode + 1), lpCode))
            {
                return 0;
            }
            MSET_XS(m_szSPErrCodeLast, 0x30, EC_ALL_LEN);
            MCPY_NOLEN(m_szSPErrCodeLast, m_szSPErrCode);
            CHAR szTmp[32] = { 0x00 };
            if (strlen(lpCode) > 1)
            {
                sprintf(szTmp, "%03s", lpCode);
            } else
            {
                sprintf(szTmp, "000");
            }
            memcpy(m_szSPErrCode + 1, szTmp, 3);
        }
        return 0;
    }    
    // 设置错误码
    INT SetErrCode(LPSTR lpCode, LPSTR lpType = nullptr)
    {
        if (lpType != nullptr)
        {
            m_szSPErrCode[0] = lpType[0];
        }

        if (lpCode != nullptr)
        {
            if (MCMP_IS0((m_szSPErrCode + 1), lpCode))
            {
                return 0;
            }
            MSET_XS(m_szSPErrCodeLast, 0x30, EC_ALL_LEN);
            MCPY_NOLEN(m_szSPErrCodeLast, m_szSPErrCode);
            INT nLen = strlen(lpCode) > (strlen(m_szSPErrCode) - 1) ?
                        (strlen(m_szSPErrCode) - 1) : strlen(lpCode);
            memcpy(m_szSPErrCode + 1, lpCode, nLen);
        }
        return 0;
    }
    // 设置HW分段错误码
    INT SetDevHWErrCode(LPSTR lpCode)
    {
        if (lpCode != nullptr && strlen(lpCode) > 1)
        {            
            WORD wDevHWLen = EC_ALL_LEN - 1 - 3;
            memcpy(m_szSPErrCode + 1 + 3, lpCode,
                   strlen(lpCode) > wDevHWLen ? wDevHWLen : strlen(lpCode));
        }
        return 0;
    }
    // 设置Dev分段错误码
    INT SetDevErrCode(LPSTR lpCode)
    {        
        return SetXFSErrCode(lpCode);
    }
    // 设置硬件返回分段错误码(入参字符串)
    // lpcDevType: INI定义设备类型编号(1Byte)
    INT SetHWErrCodeStr(LPCSTR lpcCode, LPCSTR lpcDevType = nullptr)
    {
        MSET_XS(m_szSPErrCode + 4, 0x30, EC_ALL_LEN - 4);
        if (lpcDevType != nullptr)
        {
            CHAR szTmp[64] = { 0x00 };
            sprintf(szTmp, "%c%07s", lpcDevType[0], lpcCode);
            memcpy(m_szSPErrCode + 1 + 3, szTmp, 8);
        } else
        {
            CHAR szTmp[64] = { 0x00 };
            sprintf(szTmp, "%08s", lpcCode);
            memcpy(m_szSPErrCode + 1 + 3, szTmp, 8);
        }
        return 0;
    }
    // 设置硬件返回分段错误码(入参字符串)
    // wDevID: INI定义设备类型编号(整数)
    INT SetHWErrCodeStr(LPCSTR lpcCode, WORD wDevID)
    {
        MSET_XS(m_szSPErrCode + 4, 0x30, EC_ALL_LEN - 4);
        if (wDevID <= 16)
        {
            CHAR szTmp[64] = { 0x00 };
            sprintf(szTmp, "%X%07s", wDevID, lpcCode);
            memcpy(m_szSPErrCode + 1 + 3, szTmp, 8);
        } else
        {
            CHAR szTmp[64] = { 0x00 };
            sprintf(szTmp, "%08s", lpcCode);
            memcpy(m_szSPErrCode + 1 + 3, szTmp, 8);
        }
        return 0;
    }
    // 设置硬件返回分段错误码(入参正整数)
    INT SetHWErrCodeHex(UINT unCode, LPCSTR lpcDevType = nullptr)
    {
        CHAR szHex[1024] = { 0x00 };
        DataConvertor::Int_To_HexStr(unCode, szHex, sizeof(szHex));

        MSET_XS(m_szSPErrCode + 4, 0x30, EC_ALL_LEN - 4);
        if (lpcDevType != nullptr)
        {
            CHAR szTmp[64] = { 0x00 };
            sprintf(szTmp, "%c%07s", lpcDevType[0], szHex);
            memcpy(m_szSPErrCode + 1 + 3, szTmp, 8);
        } else
        {
            CHAR szTmp[64] = { 0x00 };
            sprintf(szTmp, "%08s", szHex);
            memcpy(m_szSPErrCode + 1 + 3, szTmp, 8);
        }
        return 0;
    }
    // 设置硬件返回分段错误码(入参正整数)
    INT SetHWErrCodeHex(UINT unCode, WORD wDevID)
    {
        CHAR szHex[1024] = { 0x00 };
        DataConvertor::Int_To_HexStr(unCode, szHex, sizeof(szHex));

        MSET_XS(m_szSPErrCode + 4, 0x30, EC_ALL_LEN - 4);
        if (wDevID <= 16)
        {
            CHAR szTmp[64] = { 0x00 };
            sprintf(szTmp, "%X%07s", wDevID, szHex);
            memcpy(m_szSPErrCode + 1 + 3, szTmp, 8);
        } else
        {
            CHAR szTmp[64] = { 0x00 };
            sprintf(szTmp, "%08s", szHex);
            memcpy(m_szSPErrCode + 1 + 3, szTmp, 8);
        }
        return 0;
    }

private:
    CHAR m_szSPErrCode[EC_ALL_LEN + 1];             // SP错误码
    CHAR m_szSPErrCodeInit[EC_ALL_LEN + 1];
    CHAR m_szSPErrCodeLast[EC_ALL_LEN + 1];         // 上一次SP错误码
    CHAR m_szXFSNameID[2];                          // SPNameID保存
    CHAR m_szDevNameID[2];                          // DevNameID保存
    BOOL bIsXFS;                                    // 是否用于XFS处理
};

#endif  // ERRORCODE_H



