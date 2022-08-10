/***************************************************************
* 文件名称: ErrorDetail.h
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
 *    Byte 2: 用于标记错误码来源, 1: XFS_XXX, 2: DevXXX, 3: HW, 参考[宏定义 EC_XXX]
 *    Byte 3: 用于标记模块/层次分类, 对于(Byte2)的不同来源, 分类也会不同, 参考[宏定义]
 *    Byte 4: 用于标记硬件设备编码(暂定各模块INI中定义设备类型, 起始1), 只有一种设备可缺省0
 *    Byte 5~12: 用于标记硬件或厂商SDK返回的错误, 规则如下:
 *             (1). 硬件返回16进制, 以字符串保存.
 *             (2). 硬件返回字符串格式, 以16进制字符串保存.
 *             (3). 硬件返回正数整型格式, 转换为16进制字符串保存.
 *             (4). 硬件返回负数整型格式, 取绝对值, 转换为16进制字符串保存.
 *
 * 注: 除Byte1以外, 每一位有效标记为16进制字符形式, 如: 0 1 2 3 4 5 6 7 8 9 A B C D E F
 *
*********************************************************************************/

// Byte2 错误码来源 宏定义
#define EC_XFS                              "1"                 // XFS_XXX
#define EC_DEV                              "2"                 // DevXXX
#define EC_HW                               "3"                 // HW

#define EC_XFS_DEF                          "0"                 //

#define EC_DEV_DEF                          "0"                 // 缺省

#define EC_HW_DEF                           "0"                 //


//*********************************************************************************
// ***************  所有模块 共通 错误码定义(该部分添加需考虑各模块是否共通) **************
//*********************************************************************************
//--------------------------XFS相关--------------------------
// XFS_XXX/DevXXX 共通 错误码定义(1XXX): 库加载错误相关
#define EC_ERR_LibraryNotFound              "1001"              // 动态库未找到
#define EC_ERR_LibraryLoadFail              "1002"              // 动态库加载失败
#define EC_ERR_SPBaseNotFound               "1003"              // libSPBaseXXX.so未找到
#define EC_ERR_SPBaseLoadFail               "1004"              // libSPBaseXXX.so加载失败
#define EC_ERR_DevXXXNotFound               "1005"              // libDevXXX.so未找到
#define EC_ERR_DevXXXLoadFail               "1006"              // libDevXXX.so加载失败
#define EC_ERR_AllPortNotFound              "1007"              // libAllDevPort.so未找到
#define EC_ERR_AllPortLoadFail              "1008"              // libAllDevPort.so加载失败
#define EC_ERR_DevSDKNotFound               "1007"              // 设备SDK库未找到
#define EC_ERR_DevSDKLoadFail               "1008"              // 设备SDK库o加载失败

// XFS_XXX/DevXXX 共通  共通 错误码定义(2XXX): 参数错误相关
#define EC_ERR_ParInvalid                   "2001"              // 无效参数/不支持的参数(SP内)
#define EC_ERR_HWParInvalid                 "2002"              // 硬件接口不支持的参数
#define EC_ERR_SendData_inv                 "2003"              // 下发到硬件数据无效
#define EC_ERR_RecvData_inv                 "2004"              // 接收到硬件返回的数据无效
#define EC_ERR_ReadData_NotComp             "2005"              // 接收到硬件返回的数据不完整

// XFS_XXX/DevXXX 共通  共通 错误码定义(3XXX): 设备错误相关
#define EC_ERR_DevNotSupp                   "3001"              // INI指定设备类型不支持
#define EC_ERR_DevNotSupp                   "3002"              // 设备不支持该功能
#define EC_ERR_DevNotFound                  "3003"              // 设备不存在
#define EC_ERR_DevOpenFail                  "3004"              // 设备Open错误
#define EC_ERR_DevNotOpen                   "3005"              // 设备未Open
#define EC_ERR_DevOffLine                   "3006"              // 设备断线
#define EC_ERR_DevHWErr                     "3007"              // 设备故障
#define EC_ERR_DevPowerOff                  "3008"              // 设备电源关闭或未接通
#define EC_ERR_DevUserErr                   "3009"              // 人为原因的设备故障
#define EC_ERR_DevFraud                     "3010"              // 发现欺诈行为
#define EC_ERR_DevDestroy                   "3011"              // 发现破坏行为

// XFS_XXX/DevXXX 共通 错误码定义(4XXX): 介质错误相关: 卡/身份证/存折/凭条/票据等
#define EC_ERR_MedNotSupp                   "4001"              // 介质不支持
#define EC_ERR_MedInvalid                   "4002"              // 无效介质
#define EC_ERR_MedJammed                    "4003"              // 介质JAM
#define EC_ERR_MedNotFound                  "4004"              // 介质不存在
#define EC_ERR_MedIsHave                    "4005"              // 介质已存在
#define EC_ERR_MedIsExport                  "4006"              // 介质在出口
#define EC_ERR_MedRWFail                    "4007"              // 介质读写错误
#define EC_ERR_MedCommFail                  "4008"              // 介质通信错误

// XFS_XXX/DevXXX 共通 错误码定义(5XXX): 箱体错误相关: 钞箱/回收盒/发卡箱等
#define EC_ERR_BoxNotFound                  "5001"              // 箱体不存在
#define EC_ERR_BoxMissing                   "5002"              // 箱体丢失
#define EC_ERR_BoxHWErr                     "5003"              // 箱体故障
#define EC_ERR_BoxFull                      "5004"              // 箱体满
#define EC_ERR_BoxEmpty                     "5005"              // 箱体空
#define EC_ERR_RetainNoSupp                 "5006"              // 不支持回收

// XFS_XXX/DevXXX 共通 错误码定义(360~37F): 通信问题错误码
#define EC_ERR_ConnFail                     "6001"              // 连接失败
#define EC_ERR_CommErr                      "6002"              // 通信错误
#define EC_ERR_CommTimeOut                  "6003"              // 通信超时
#define EC_ERR_DataRWErr                    "6004"              // 数据读写错误
#define EC_ERR_SDKFail                      "6005"              // SDK调用错误
#define EC_ERR_InTerExec                    "6006"              // 接口执行错误

// XFS_XXX/DevXXX 共通 错误码定义(9XXX): 其他错误相关:
#define EC_ERR_UnKnownErr                   "9001"              // 未知错误
#define EC_ERR_OtherErr                     "9002"              // 其他错误
#define EC_ERR_HWErr                        "9003"              // 硬件错误
#define EC_ERR_ConnInvalid                  "9004"              // 无效的连接
#define EC_ERR_Unsup_CMD                    "9005"              // 不支持的命令或接口
#define EC_ERR_TimeOut                      "9006"              // 超时
#define EC_ERR_ShareMemCrt                  "9007"              // 共享内存建立错误
#define EC_ERR_ShareMemRW                   "9008"              // 共享内存读写错误
#define EC_ERR_FilePathCrtErr               "9009"              // 文件目录创建失败
#define EC_ERR_MemoryApply                  "9010"              // 内存空间申请失败
#define EC_ERR_MemoryErrr                   "9011"              // 内存错误
#define EC_ERR_CodeChange                   "9012"              // 数据格式转换失败


//*********************************************************************************
// ************ 以下为 各SP模块错误码 自行定义(该部分添加避免与共通部分有重复) ************
//*********************************************************************************



//--------------------------------------------------------------------------
// IDC 错误码定义
// 读卡器有加载返卡/发卡的情况, 错误码定义也包含返卡CRM和发卡CRD
// 适用模块: IDC/FIDC/IDX/MSR/CRM/CRD
//--------------------------------------------------------------------------
#define EC_XFS_IDC                          "1"                 // DevIDC
#define EC_XFS_CRM                          "2"                 // DevCRM
#define EC_XFS_CRD                          "3"                 // DevCRM
#define EC_DEV_IDC                          EC_XFS_IDC          // DevIDC
#define EC_DEV_CRM                          EC_XFS_CRM          // DevCRM
#define EC_DEV_CRD                          EC_XFS_CRD          // DevCRD
// 读卡器: XFS_IDC 错误码定义(AXXX):
#define EC_IDC_ERR_libIDCNotFound           "A001"              // libDevIDC.so动态库未找到
#define EC_IDC_ERR_libIDCLoadFail           "A002"              // libDevIDC.so动态库库加载失败
#define EC_IDC_ERR_IDCCRMOffLine            "A003"              // 读卡器返卡同时断线
#define EC_IDC_ERR_PortNotSupp              "A004"              // 不支持的Chip协议
#define EC_IDC_ERR_ChipPowerOff             "A005"              // 芯片卡未通电
#define EC_IDC_ERR_ChipUnknown              "A006"              // 芯片卡状态未知
#define EC_IDC_ERR_ChipNotSupp              "A007"              // 不支持芯片卡操作

// 读卡器: DevIDC 错误码定义(BXXX):
#define EC_IDC_ERR_TamperRelease            "B001"              // 防盗钩释放错误
#define EC_IDC_ERR_TamperNotRelease         "B002"              // 防盗钩未释放
#define EC_IDC_ERR_TeaseCard                "B003"              // 防逗卡保护中


// 返卡模块: CRM 错误码定义:
// XFS_CRM错误码定义(CXXX):
#define EC_CRM_ERR_DevCRMNotFound           "C001"              // libDevCRM.so未找到
#define EC_CRM_ERR_DevCRMLoadFail           "C002"              // libDevCRM.so加载失败
#define EC_CRM_ERR_ININotSupp               "C003"              // INI指定不支持返卡设备
#define EC_CRM_ERR_DevNotSupp               "C004"              // 返卡设备不支持
#define EC_CRM_ERR_DevOpenFail              "C005"              // 返卡设备Open错误
#define EC_CRM_ERR_DevNotOpen               "C006"              // 返卡设备未Open
#define EC_CRM_ERR_DevOffLine               "C007"              // 返卡设备断线
#define EC_CRM_ERR_IDCHaveMed               "C008"              // 读卡器有卡无法执行返卡操作
#define EC_CRM_ERR_CardNoInv                "C009"              // 返卡设备指定卡号规则无效
#define EC_CRM_ERR_CardNotFound             "C010"              // 返卡设备指定卡不存在
#define EC_CRM_ERR_CardNoIsHave             "C011"              // 返卡设备指定卡号已存在

// DevCRM错误码定义(DXXX):
#define EC_CRM_DEV_SlotPosAbnormal          "D001"              // 暂存仓暂存仓位置异常
#define EC_CRM_DEV_StatInvalid              "D002"              // 状态无效
#define EC_CRM_DEV_SlotNoInvalid            "D003"              // 指定暂存仓编号无效


//--------------------------------------------------------------------------
// CAM 错误码定义
// 适用模块: CAM/HCAM
//--------------------------------------------------------------------------
// 摄像: XFS_CAM 错误码定义(AXXX):
#define EC_CAM_XFS_libCAMNotFound           "A001"              // libDevCAM.so动态库未找到
#define EC_CAM_XFS_libCAMLoadFail           "A002"              // libDevCAM.so动态库库加载失败
#define EC_CAM_XFS_FilePathInv              "A003"              // 指定文件路径无效
#define EC_CAM_XFS_NotDisplay               "A004"              // 摄像窗口未打开

// 摄像: DevCAM 错误码定义(BXXX):
#define EC_CAM_DEV_VideoIdxNotFount         "B001"              // 摄像设备号未找到
#define EC_CAM_DEV_GetImageErr              "B002"              // 取图像数据失败
#define EC_CAM_DEV_SaveImageErr             "B003"              // 保存图像失败
#define EC_CAM_DEV_NotLiveDetect            "B003"              // 未检测到活体


//****************************************************************************
// IDevIDC定义 设备相关变量转换 类定义(统一转换)
//****************************************************************************
// 错误码长度
#define EC_ALL_LEN                          12                  // 错误码总长度
#define EC_ERR_POS                          3                   // 错误码起始位置
#define EC_ERR_LEN                          3                   // DevXXX错误码长度
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
    // DevXXX传入对应模块的类型定义
    // stSPName.szName优先检查,不符合默认为DevXXX方式参数
    CErrorDetail()
    {
        // 初始化定义
        MSET_0(m_szSPErrCode);
        MSET_XS(m_szSPErrCode, 0x30, EC_ALL_LEN);
        MCPY_NOLEN(m_szSPErrCodeLast, m_szSPErrCode);
        MCPY_NOLEN(m_szSPErrCodeInit, m_szSPErrCode)
        MSET_0(m_szSPNameID);
        MSET_XS(m_szSPNameID, 0x30, 1);
        MSET_0(m_szXFSNameID);
        MSET_XS(m_szXFSNameID, 0x30, 1);
        MSET_0(m_szDevNameID);
        MSET_XS(m_szDevNameID, 0x30, 1);
        MSET_0(m_szHWNameID);
        MSET_XS(m_szHWNameID, 0x30, 1);
    }

    ~CErrorDetail() {}

public:
    // 返回SP错误码
    CHAR* GetSPErrCode(LPSTR lpErrCode = nullptr)
    {
        if (!MCMP_IS0(m_szSPErrCode, m_szSPErrCodeInit))
        {
            memcpy(m_szSPErrCode, m_szSPNameID, 1);
        }

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

public:
    // 设置SPName(XFS_XXX层使用)
    // 入参参考
    INT SetSPName(LPSTR lpName)
    {
        if (lpName == nullptr)
        {
            return -1;
        }

        for (INT i = 0; i < (sizeof(stSPName)/sizeof(stSPName[0])); i ++)
        {
            if (MCMP_IS0(lpName, stSPName[i].szName))
            {
                m_szSPNameID[0] = stSPName[i].szSerial[0];
                break;
            }
        }

        return SUCCESS;
    }

    // 设置Byte3: XFS Name(XFS_XXX使用)
    INT SetXFSName(LPSTR lpName = nullptr)
    {
        if (lpName == nullptr)
        {
            memcpy(m_szXFSNameID, EC_XFS_DEF, 1);
        } else
        {
            if (strlen(lpName) < 1)
            {
                memcpy(m_szXFSNameID, EC_XFS_DEF, 1);
            } else
            {
                m_szXFSNameID[0] = lpName[0];
            }
        }

        return SUCCESS;
    }

    // 设置Byte3: DevXXX Name(DevXXX使用)
    INT SetDevName(LPSTR lpName = nullptr)
    {
        if (lpName == nullptr)
        {
            memcpy(m_szDevNameID, EC_DEV_DEF, 1);
        } else
        {
            if (strlen(lpName) < 1)
            {
                memcpy(m_szDevNameID, EC_DEV_DEF, 1);
            } else
            {
                m_szDevNameID[0] = lpName[0];
            }
        }

        return SUCCESS;
    }

    // 设置Byte3: HW Name(DevXXX使用)
    INT SetHWName(LPSTR lpName = nullptr)
    {
        if (lpName == nullptr)
        {
            memcpy(m_szHWNameID, EC_HW_DEF, 1);
        } else
        {
            if (strlen(lpName) < 1)
            {
                memcpy(m_szHWNameID, EC_HW_DEF, 1);
            } else
            {
                m_szHWNameID[0] = lpName[0];
            }
        }

        return SUCCESS;
    }

    // 错误码初始化
    // bIsAll: T:当前错误码+上一次错误码初始化, F:只初始化当前错误码
    INT SetErrCodeInit(BOOL bIsAll = FALSE)
    {
        MCPY_NOLEN(m_szSPErrCode, m_szSPErrCodeInit);
        if (bIsAll == TRUE)
        {
            MCPY_NOLEN(m_szSPErrCodeLast, m_szSPErrCodeInit);
        }
        return SUCCESS;
    }

public:
    // 设置XFS_XXX错误码
    // bIsBackup: T当前错误码备份到上一次错误码
    INT SetXFSErrCode(LPSTR lpCode, BOOL bIsBackup = TRUE)
    {
        if (lpCode != nullptr && strlen(lpCode) > 0)
        {
            // 组合错误码
            MCPY_NOLEN(m_szSPErrCodeTmp, m_szSPErrCodeInit);    // 初始化
            memcpy(m_szSPErrCodeTmp, m_szSPNameID, 1);          // Byte1
            memcpy(m_szSPErrCodeTmp + 1, EC_XFS, 1);            // Byte2
            memcpy(m_szSPErrCodeTmp + 2, m_szXFSNameID, 1);     // Byte3
            CopyErrCode(m_szSPErrCodeTmp + EC_ERR_POS, lpCode);

            // 完整错误码保存备份处理
            SetErrCode(m_szSPErrCodeTmp, bIsBackup);
        }

        return SUCCESS;
    }

    // 设置DevXXX错误码
    // bIsBackup: T当前错误码备份到上一次错误码
    INT SetDevErrCode(LPSTR lpCode, BOOL bIsBackup = TRUE)
    {
        if (lpCode != nullptr && strlen(lpCode) > 0)
        {
            // 组合错误码
            MCPY_NOLEN(m_szSPErrCodeTmp, m_szSPErrCodeInit);    // 初始化
            memcpy(m_szSPErrCodeTmp, m_szSPNameID, 1);          // Byte1
            memcpy(m_szSPErrCodeTmp + 1, EC_DEV, 1);            // Byte2
            memcpy(m_szSPErrCodeTmp + 2, m_szDevNameID, 1);     // Byte3
            CopyErrCode(m_szSPErrCodeTmp + EC_ERR_POS, lpCode);

            // 完整错误码保存备份处理
            SetErrCode(m_szSPErrCodeTmp, bIsBackup);
        }

        return SUCCESS;
    }

    // 设置HW错误码(错误码入参为字符串, 设备类型为字符串)
    // lpDevType: 设备类型(只取第一位)
    // bIsBackup: T当前错误码备份到上一次错误码
    INT SetHWErrCodeStr(LPSTR lpCode, LPSTR lpDevType, BOOL bIsBackup = TRUE)
    {
        if (lpCode != nullptr && strlen(lpCode) > 0)
        {
            // 组合错误码
            MCPY_NOLEN(m_szSPErrCodeTmp, m_szSPErrCodeInit);    // 初始化
            memcpy(m_szSPErrCodeTmp, m_szSPNameID, 1);          // Byte1
            memcpy(m_szSPErrCodeTmp + 1, EC_HW, 1);             // Byte2
            memcpy(m_szSPErrCodeTmp + 2, m_szHWNameID, 1);      // Byte3
            if (lpDevType != nullptr && strlen(lpDevType) > 0)
            {
                memcpy(m_szSPErrCodeTmp + 3, lpDevType, 1);     // Byte4
                CopyErrCode(m_szSPErrCodeTmp + 4, lpCode);
            } else
            {
                CopyErrCode(m_szSPErrCodeTmp + EC_ERR_POS, lpCode);
            }

            // 完整错误码保存备份处理
            SetErrCode(m_szSPErrCodeTmp, bIsBackup);
        }

        return SUCCESS;
    }

    // 设置HW错误码(错误码入参为整型, 设备类型为字符串)
    // lpDevType: 设备类型(只取第一位)
    // bIsBackup: T当前错误码备份到上一次错误码
    INT SetHWErrCodeInt(INT nCode, LPSTR lpDevType, BOOL bIsBackup = TRUE)
    {
        INT nCodeTmp = abs(nCode);

        // 转换为16进制
        CHAR szCodeHex[1024] = { 0x00 };
        DataConvertor::Int_To_HexStr(nCodeTmp, szCodeHex, sizeof(szCodeHex));

        if (strlen(szCodeHex) > 0)
        {
            // 组合错误码
            MCPY_NOLEN(m_szSPErrCodeTmp, m_szSPErrCodeInit);    // 初始化
            memcpy(m_szSPErrCodeTmp, m_szSPNameID, 1);          // Byte1
            memcpy(m_szSPErrCodeTmp + 1, EC_HW, 1);             // Byte2
            memcpy(m_szSPErrCodeTmp + 2, m_szHWNameID, 1);      // Byte3
            if (lpDevType != nullptr && strlen(lpDevType) > 0)
            {
                memcpy(m_szSPErrCodeTmp + 3, lpDevType, 1);     // Byte4
                CopyErrCode(m_szSPErrCodeTmp + 4, szCodeHex);
            } else
            {
                CopyErrCode(m_szSPErrCodeTmp + EC_ERR_POS, szCodeHex);
            }

            // 完整错误码保存备份处理
            SetErrCode(m_szSPErrCodeTmp, bIsBackup);
        }

        return SUCCESS;
    }

    // 设置HW错误码(错误码入参为字符串, 设备类型为整型)
    // lpDevType: 设备类型(只取第一位)
    // bIsBackup: T当前错误码备份到上一次错误码
    INT SetHWErrCodeStr(LPSTR lpCode, WORD wDevType, BOOL bIsBackup = TRUE)
    {
        if (lpCode != nullptr)
        {
            CHAR szTypeTmp[2] = { 0x00 };
            if (wDevType < 10)
            {
                szTypeTmp[0] = '0' + wDevType;
            } else
            if (wDevType > 9 && wDevType <= 35)
            {
                szTypeTmp[0] = 'A' + wDevType - 9;
            } else
            {
                return FAIL;
            }

            return SetHWErrCodeStr(lpCode, szTypeTmp, bIsBackup);
        }

        return SUCCESS;
    }

    // 设置HW错误码(错误码入参为整型, 设备类型为整型)
    // lpDevType: 设备类型(只取第一位)
    // bIsBackup: T当前错误码备份到上一次错误码
    INT SetHWErrCodeInt(INT nCode, WORD wDevType, BOOL bIsBackup = TRUE)
    {
        CHAR szTypeTmp[2] = { 0x00 };
        if (wDevType < 10)
        {
            szTypeTmp[0] = '0' + wDevType;
        } else
        if (wDevType > 9 && wDevType <= 35)
        {
            szTypeTmp[0] = 'A' + wDevType - 9;
        } else
        {
            return FAIL;
        }

        return SetHWErrCodeInt(nCode, szTypeTmp, bIsBackup);
    }

    // 设置错误码(完全复制)
    INT SetErrCode(LPSTR lpCode, BOOL bIsBackup = TRUE)
    {
        // 同一个错误码不处理
        if (MCMP_IS0(m_szSPErrCode, lpCode))
        {
            return SUCCESS;
        }

        // 错误码备份
        if (bIsBackup == TRUE)
        {
            ErrCodeBackup();
        }

        MCPY_NOLEN(m_szSPErrCode, lpCode);
    }

private:
    // Byte5~12: 错误码部分复制
    // 右对齐,左补"0";
    INT CopyErrCode(LPSTR lpDest, LPSTR lpSource)
    {
        // 错误码长度>保存长度, 取保存长度位数
        WORD wCodeLen = strlen(lpSource);
        if (wCodeLen > strlen(lpDest))
        {
            wCodeLen = strlen(lpDest);
        }

        wCodeLen --;
        for (INT i = strlen(lpDest) -1 ; i >= 0; i --)
        {
            lpDest[i] = lpSource[wCodeLen];
            if ((wCodeLen --) == 0)
            {
                break;
            }
        }
        return SUCCESS;
    }

    // 当前错误码备份到上一次错误码, 只备份错误, 全"0"或相同不备份
    INT ErrCodeBackup()
    {
        if (!MCMP_IS0(m_szSPErrCode, m_szSPErrCodeInit) ||
            !MCMP_IS0(m_szSPErrCode, m_szSPErrCodeLast))
        {
            MCPY_NOLEN(m_szSPErrCodeLast, m_szSPErrCode);
        }
        return SUCCESS;
    }

private:
    CHAR m_szSPErrCode[EC_ALL_LEN + 1];             // SP错误码
    CHAR m_szSPErrCodeLast[EC_ALL_LEN + 1];         // 上一次SP错误码
    CHAR m_szSPErrCodeInit[EC_ALL_LEN + 1];         // 初始值全"0"
    CHAR m_szSPErrCodeTmp[EC_ALL_LEN + 1];          // 临时组合错误码变量
    CHAR m_szSPNameID[2];                           // SPNameID保存
    CHAR m_szXFSNameID[2];                          // XFS类型ID保存
    CHAR m_szDevNameID[2];                          // Dev类型ID保存
    CHAR m_szHWNameID[2];                           // HW类型ID保存
    BOOL bIsXFS;                                    // 是否用于XFS处理
};

#endif  // ERRORCODE_H



