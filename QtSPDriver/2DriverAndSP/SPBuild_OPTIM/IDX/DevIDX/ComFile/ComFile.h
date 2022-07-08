/***************************************************************
* 文件名称: ComFile.h
* 文件描述: 身份证通用处理 头文件
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2023年5月16日
* 文件版本: 1.0.0.1
****************************************************************/

#ifndef DEVIDX_COMFILE_H
#define DEVIDX_COMFILE_H

#include "QtTypeDef.h"
#include "QtTypeInclude.h"


//-----------------------身份证信息串处理------------------------------
enum EN_IDINFOMODE
{
    IDINFO_00 = 0,              // 缺省
    IDINFO_01 = 1,              // 适用长沙银行
    IDINFO_02 = 2,              // 适用陕西信合
    IDINFO_03 = 3,              // 适用建设银行
};


// 国内二代身份证信息结构体
typedef struct ST_IDCARD_INFO
{
    CHAR szName[128];               // 姓名
    CHAR szSex[12];                 // 性别
    CHAR szNation[32];              // 民族
    CHAR szBirthday[32];            // 出生日期
    CHAR szAddress[256];            // 住址信息
    CHAR szNumber[64];              // 身份证号码
    CHAR szDepartment[128];         // 签发机关
    CHAR szSTimeLimit[32];          // 起始有效日期
    CHAR szETimeLimit[32];          // 结束有效日期
    CHAR szImage[256];              // 头像信息文件(路径)
    CHAR szFingerData1[1024];       // 指纹信息数据1
    CHAR szFingerData2[1024];       // 指纹信息数据2
    INT  nFinger1DataLen;           // 指纹信息数据1长度
    INT  nFinger2DataLen;           // 指纹信息数据1长度
    ST_IDCARD_INFO()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(ST_IDCARD_INFO));
    }
} STIDCARDINFO, *LPSTIDCARDINFO;

// 国外身份证信息结构体
typedef struct ST_IDFOREIGN_INFO
{
    CHAR szNameENG[128];            // 英文姓名
    CHAR szSex[32];                 // 性别
    CHAR szIDCardNO[64];            // 证件号码
    CHAR szNation[32];              // 国籍
    CHAR szNameCHN[128];            // 中文姓名
    CHAR szSTimeLimit[32];          // 证件签发日期开始
    CHAR szETimeLimit[32];          // 证件签发日期结束
    CHAR szBorn[64];                // 出生日期
    CHAR szIDVersion[12];           // 证件版本
    CHAR szDepartment[128];         // 签发机关
    CHAR szIDType[12];              // 证件类型
    CHAR szReserve[64];             // 保留信息
    CHAR szImage[1024];             // 头像
    ST_IDFOREIGN_INFO()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(ST_IDFOREIGN_INFO));
    }
} STIDFOREIGNINFO, *LPSTIDFOREIGNINFO;

// 港澳台通行证信息结构体
typedef struct ST_IDGAT_INFO
{
    CHAR szName[128];               // 姓名
    CHAR szSex[12];                 // 性别
    CHAR szNation[32];              // 民族
    CHAR szBirthday[32];            // 出生日期
    CHAR szAddress[256];            // 住址信息
    CHAR szNumber[64];              // 身份证号码
    CHAR szDepartment[128];         // 签发机关
    CHAR szSTimeLimit[32];          // 起始有效日期
    CHAR szETimeLimit[32];          // 结束有效日期
    CHAR szPassport[32];            // 通行证号码
    CHAR szIssue[12];               // 签发次数
    CHAR szImage[256];              // 头像信息文件
    CHAR szFingerData1[1024];       // 指纹信息数据1
    CHAR szFingerData2[1024];       // 指纹信息数据2
    INT  nFinger1DataLen;           // 指纹信息数据1长度
    INT  nFinger2DataLen;           // 指纹信息数据1长度
    CHAR szIDType[12];              // 证件类型
    ST_IDGAT_INFO()
    {
        Clear();
    }
    void Clear()
    {
        memset(this, 0x00, sizeof(ST_IDGAT_INFO));
    }
} STIDGATINFO, *LPSTIDGATINFO;

// 身份证信息组合处理类
class CIDCARD_ORGAN
{
public:
    CIDCARD_ORGAN() {};
    ~CIDCARD_ORGAN() {};

public:
    // 国内二代身份证组合串
    INT GetIDCardStr(STIDCARDINFO stInfo, EN_IDINFOMODE enMode, LPSTR lpRetStr, INT nRetSize);
    // 国外身份证组合串
    INT GetIDForeignStr(STIDFOREIGNINFO stInfo, EN_IDINFOMODE enMode, LPSTR lpRetStr, INT nRetSize);
    // 港澳台通行证组合串
    INT GetIDGATStr(STIDGATINFO stInfo, EN_IDINFOMODE enMode, LPSTR lpRetStr, INT nRetSize);
};

#endif // DEVIDX_COMFILE_H
