/***************************************************************
* 文件名称: ComFile.cpp
* 文件描述: 身份证通用处理 接口封装
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2023年5月16日
* 文件版本: 1.0.0.1
****************************************************************/

#include "ComFile.h"


// 国内二代身份证组合串
// stInfo: 入参 信息结构体
// enMode: 入参 类别(根据不同银行区分)
// lpRetStr: 回参 返回处理后字符串
// nRetSize: 入参 处理后字符串Buff大小
// 返回值: >=0:成功(返回处理后字符串大小), <0:错误
INT CIDCARD_ORGAN::GetIDCardStr(STIDCARDINFO stInfo, EN_IDINFOMODE enMode, LPSTR lpRetStr, INT nRetSize)
{
    INT nSize = 0;
    CHAR szBuff[65536] = { 0x00 };

    if (lpRetStr == nullptr)
    {
        return -1;
    }

    if (nRetSize < 1)
    {
        return -2;
    }

    if (enMode == IDINFO_00)
    {
        sprintf(szBuff,
                "Name=%s|Sex=%s|Nation=%s|Born=%s|IDCardNo=%s|Address=%s|GrantDept=%s|"
                "UserLifeBegin=%s|UserLifeEnd=%s|Finger1=",
                QString(stInfo.szName).trimmed().toStdString().c_str(),             // 姓名
                QString(stInfo.szSex).trimmed().toStdString().c_str(),              // 性别
                QString(stInfo.szNation).trimmed().toStdString().c_str(),           // 民族
                QString(stInfo.szBirthday).trimmed().toStdString().c_str(),         // 出生日期
                QString(stInfo.szNumber).trimmed().toStdString().c_str(),           // 身份证号码
                QString(stInfo.szAddress).trimmed().toStdString().c_str(),          // 住址信息
                QString(stInfo.szDepartment).trimmed().toStdString().c_str(),       // 签发机关
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str(),       // 有效日期开始
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str()        // 有效日期结束
               );
        for (int i = 0; i < stInfo.nFinger1DataLen; i ++)
        {
            sprintf(szBuff + strlen(szBuff), "%02x", (BYTE)(stInfo.szFingerData1[i]));
        }
        sprintf(szBuff + strlen(szBuff), "|Finger2=");
        for (int i = 0; i < stInfo.nFinger2DataLen; i ++)
        {
            sprintf(szBuff + strlen(szBuff), "%02x", (BYTE)(stInfo.szFingerData2[i]));
        }
        sprintf(szBuff + strlen(szBuff), "|PhotoFileName=%s|",
                QString(stInfo.szImage).toStdString().c_str());                     // 头像信息文件
    } else
    if (enMode == IDINFO_01)
    {
        sprintf(szBuff,
                "IDType=0|Name=%s|Sex=%s|Nation=%s|Born=%s|Address=%s|IDCardNo=%s|GrantDept=%s|"
                "UserLifeBegin=%s|UserLifeEnd=%s|PhotoFileName=%s|",
                QString(stInfo.szName).trimmed().toStdString().c_str(),             // 姓名
                QString(stInfo.szSex).trimmed().toStdString().c_str(),              // 性别
                QString(stInfo.szNation).trimmed().toStdString().c_str(),           // 民族
                QString(stInfo.szBirthday).trimmed().toStdString().c_str(),         // 出生日期
                QString(stInfo.szAddress).trimmed().toStdString().c_str(),          // 住址信息
                QString(stInfo.szNumber).trimmed().toStdString().c_str(),           // 身份证号码
                QString(stInfo.szDepartment).trimmed().toStdString().c_str(),       // 签发机关
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str(),       // 有效日期开始
                QString(stInfo.szETimeLimit).trimmed().toStdString().c_str(),       // 有效日期结束
                QString(stInfo.szImage).toStdString().c_str());                     // 头像信息文件
    } else
    if (enMode == IDINFO_02)
    {
        sprintf(szBuff,
                "IDType=0|Name=%s|Sex=%s|Nation=%s|Born=%s|Address=%s|IDCardNo=%s|GrantDept=%s|"
                "UserLifeBegin=%s|UserLifeEnd=%s|PhotoFileName=%s|FingerDataOne=",
                QString(stInfo.szName).trimmed().toStdString().c_str(),         // 姓名
                QString(stInfo.szSex).trimmed().toStdString().c_str(),          // 性别
                QString(stInfo.szNation).trimmed().toStdString().c_str(),       // 民族
                QString(stInfo.szBirthday).trimmed().toStdString().c_str(),     // 出生日期
                QString(stInfo.szAddress).trimmed().toStdString().c_str(),      // 住址信息
                QString(stInfo.szNumber).trimmed().toStdString().c_str(),       // 身份证号码
                QString(stInfo.szDepartment).trimmed().toStdString().c_str(),   // 签发机关
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str(),   // 有效日期开始
                QString(stInfo.szETimeLimit).trimmed().toStdString().c_str(),   // 有效日期结束
                QString(stInfo.szImage).toStdString().c_str());                 //头像信息文件
        for (int i = 0; i < stInfo.nFinger1DataLen; i ++)
        {
            sprintf(szBuff + strlen(szBuff), "%02x", (BYTE)(stInfo.szFingerData1[i]));
        }
        sprintf(szBuff + strlen(szBuff), "|FingerDataTwo=");
        for (int i = 0; i < stInfo.nFinger2DataLen; i ++)
        {
            sprintf(szBuff + strlen(szBuff), "%02x", (BYTE)(stInfo.szFingerData2[i]));
        }
    } else
    if (enMode == IDINFO_03)
    {

    } else
    {
        return -3;
    }

    nSize = strlen(szBuff);
    nSize = (nSize >= nRetSize ? nRetSize - 1 : nSize);
    memcpy(lpRetStr, szBuff, nSize);

    return nSize;
}

// 国外身份证组合串
// stInfo: 入参 信息结构体
// enMode: 入参 类别(根据不同银行区分)
// lpRetStr: 回参 返回处理后字符串
// nRetSize: 入参 处理后字符串Buff大小
// 返回值: >=0:成功(返回处理后字符串大小), <0:错误
INT CIDCARD_ORGAN::GetIDForeignStr(STIDFOREIGNINFO stInfo, EN_IDINFOMODE enMode, LPSTR lpRetStr, INT nRetSize)
{
    INT nSize = 0;
    CHAR szBuff[65536] = { 0x00 };

    if (lpRetStr == nullptr)
    {
        return -1;
    }

    if (nRetSize < 1)
    {
        return -2;
    }

    if (enMode == IDINFO_00)
    {
        sprintf(szBuff,
                "FgnNameEN=%s|FgnName=%s|FgnSex=%s|FgnNation=%s|FgnBirthDay=%s|"
                "FgnCardId=%s|FgnIssAuth=%s|FgnStartDate=%s|FgnEndDate=%s|"
                "FgnCardVar=%s|FgnCardSign=%s|Finger1=%s|Finger2=%s|PhotoFileName=%s|",
                QString(stInfo.szNameENG).trimmed().toStdString().c_str(),          // 英文姓名
                QString(stInfo.szNameCHN).trimmed().toStdString().c_str(),          // 中文姓名
                QString(stInfo.szSex).trimmed().toStdString().c_str(),              // 性别
                QString(stInfo.szNation).trimmed().toStdString().c_str(),           // 国籍
                QString(stInfo.szBorn).trimmed().toStdString().c_str(),             // 出生日期
                QString(stInfo.szIDCardNO).trimmed().toStdString().c_str(),         // 证件号码
                QString(stInfo.szDepartment).trimmed().toStdString().c_str(),       // 签发机关
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str(),       // 证件签发日期开始
                QString(stInfo.szETimeLimit).trimmed().toStdString().c_str(),       // 证件签发日期结束
                QString(stInfo.szIDVersion).trimmed().toStdString().c_str(),        // 证件版本
                QString(stInfo.szIDType).trimmed().toStdString().c_str(),           // 证件类型
                "", "",                                                             // 指纹信息数据1,2
                QString(stInfo.szImage).trimmed().toStdString().c_str()             // 头像
               );
    } else
    if (enMode == IDINFO_01)
    {
        sprintf(szBuff,
                "IDType=1|EnglishName=%s|Sex=%s|IDCardNO=%s|Nationality=%s|ChineseName=%s|"
                "UserLiftBegin=%s|UserLiftEnd=%s|Born=%s|VersionNumber=%s|"
                "OrganizationCode=%s|ReserveItem=%s|PhotoFileName=%s|Finger2=%s|PhotoFileName=%s",
                QString(stInfo.szNameENG).trimmed().toStdString().c_str(),      // 英文姓名
                QString(stInfo.szSex).trimmed().toStdString().c_str(),          // 性别
                QString(stInfo.szIDCardNO).trimmed().toStdString().c_str(),     // 证件号码
                QString(stInfo.szNation).trimmed().toStdString().c_str(),       // 国籍
                QString(stInfo.szNameCHN).trimmed().toStdString().c_str(),      // 中文姓名
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str(),   // 证件签发日期开始
                QString(stInfo.szETimeLimit).trimmed().toStdString().c_str(),   // 证件签发日期结束
                QString(stInfo.szBorn).trimmed().toStdString().c_str(),         // 出生日期
                QString(stInfo.szIDVersion).trimmed().toStdString().c_str(),    // 证件版本
                QString(stInfo.szDepartment).trimmed().toStdString().c_str(),   // 签发机关
                QString(stInfo.szReserve).trimmed().toStdString().c_str(),      // 保留信息
                QString(stInfo.szImage).trimmed().toStdString().c_str()         // 头像
               );
    } else
    if (enMode == IDINFO_02)
    {
        sprintf(szBuff,
                "IDType=1|EnglishName=%s|Sex=%s|IDCardNO=%s|Nationality=%s|ChineseName=%s|"
                "UserLiftBegin=%s|UserLiftEnd=%s|Born=%s|VersionNumber=%s|"
                "OrganizationCode=%s|ReserveItem=%s|PhotoFileName=%s",
                QString(stInfo.szNameENG).trimmed().toStdString().c_str(),      // 英文姓名
                QString(stInfo.szSex).trimmed().toStdString().c_str(),          // 性别
                QString(stInfo.szIDCardNO).trimmed().toStdString().c_str(),     // 证件号码
                QString(stInfo.szNation).trimmed().toStdString().c_str(),       // 国籍
                QString(stInfo.szNameCHN).trimmed().toStdString().c_str(),      // 中文姓名
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str(),   // 证件签发日期开始
                QString(stInfo.szETimeLimit).trimmed().toStdString().c_str(),   // 证件签发日期结束
                QString(stInfo.szBorn).trimmed().toStdString().c_str(),         // 出生日期
                QString(stInfo.szIDVersion).trimmed().toStdString().c_str(),    // 证件版本
                QString(stInfo.szDepartment).trimmed().toStdString().c_str(),   // 签发机关
                QString(stInfo.szReserve).trimmed().toStdString().c_str(),      // 保留信息
                QString(stInfo.szImage).trimmed().toStdString().c_str()         // 头像
               );
    } else
    if (enMode == IDINFO_03)
    {

    } else
    {
        return -3;
    }

    nSize = strlen(szBuff);
    nSize = (nSize >= nRetSize ? nRetSize - 1 : nSize);
    memcpy(lpRetStr, szBuff, nSize);

    return nSize;
}


// 港澳台通行证组合串
// 国外身份证组合串
// stInfo: 入参 信息结构体
// enMode: 入参 类别(根据不同银行区分)
// lpRetStr: 回参 返回处理后字符串
// nRetSize: 入参 处理后字符串Buff大小
// 返回值: >=0:成功(返回处理后字符串大小), <0:错误
INT CIDCARD_ORGAN::GetIDGATStr(STIDGATINFO stInfo, EN_IDINFOMODE enMode, LPSTR lpRetStr, INT nRetSize)
{
    INT nSize = 0;
    CHAR szBuff[65536] = { 0x00 };

    if (lpRetStr == nullptr)
    {
        return -1;
    }

    if (nRetSize < 1)
    {
        return -2;
    }

    if (enMode == IDINFO_00)
    {
        sprintf(szBuff,
                "Name=%s|Sex=%s|Born=%s|IDCardNo=%s|Address=%s|GrantDept=%s|UserLifeBegin=%s|"
                "UserLifeEnd=%s|PermitNo=%s|IssueCount=%s|FgnCardSign=%s|Finger1=",
                QString(stInfo.szName).trimmed().toStdString().c_str(),             // 姓名
                QString(stInfo.szSex).trimmed().toStdString().c_str(),              // 性别
                QString(stInfo.szBirthday).trimmed().toStdString().c_str(),         // 出生日期
                QString(stInfo.szNumber).trimmed().toStdString().c_str(),           // 身份证号码
                QString(stInfo.szAddress).trimmed().toStdString().c_str(),          // 住址信息
                QString(stInfo.szDepartment).trimmed().toStdString().c_str(),       // 签发机关
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str(),       // 有效日期开始
                QString(stInfo.szETimeLimit).trimmed().toStdString().c_str(),       // 有效日期结束
                QString(stInfo.szPassport).trimmed().toStdString().c_str(),         // 通行证号码
                QString(stInfo.szIssue).trimmed().toStdString().c_str(),            // 签发次数
                "J"                                                                 // 证件类型标识
               );
        for (int i = 0; i < stInfo.nFinger1DataLen; i ++)
        {
            sprintf(szBuff + strlen(szBuff), "%02x", (BYTE)(stInfo.szFingerData1[i]));
        }
        sprintf(szBuff + strlen(szBuff), "|Finger2=");
        for (int i = 0; i < stInfo.nFinger2DataLen; i ++)
        {
            sprintf(szBuff + strlen(szBuff), "%02x", (BYTE)(stInfo.szFingerData2[i]));
        }
        sprintf(szBuff + strlen(szBuff), "|PhotoFileName=%s|",
                QString(stInfo.szImage).toStdString().c_str());                     // 头像信息文件
    } else
    if (enMode == IDINFO_01)
    {
        sprintf(szBuff,
                "IDType=2|Name=%s|Sex=%s|Born=%s|Address=%s|IDCardNo=%s|GrantDept=%s|UserLifeBegin=%s|"
                "UserLifeEnd=%s|PassNO=%s|IssueNumber=%s|FingerDataOne=",
                QString(stInfo.szName).trimmed().toStdString().c_str(),         // 姓名
                QString(stInfo.szSex).trimmed().toStdString().c_str(),          // 性别
                QString(stInfo.szBirthday).trimmed().toStdString().c_str(),     // 出生日期
                QString(stInfo.szAddress).trimmed().toStdString().c_str(),      // 住址信息
                QString(stInfo.szNumber).trimmed().toStdString().c_str(),       // 身份证号码
                QString(stInfo.szDepartment).trimmed().toStdString().c_str(),   // 签发机关
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str(),   // 有效日期开始
                QString(stInfo.szETimeLimit).trimmed().toStdString().c_str(),   // 有效日期结束
                QString(stInfo.szPassport).trimmed().toStdString().c_str(),     // 通行证号码
                QString(stInfo.szIssue).trimmed().toStdString().c_str()         // 签发次数
               );
        for (int i = 0; i < stInfo.nFinger1DataLen; i ++)
        {
            sprintf(szBuff + strlen(szBuff), "%02x", (BYTE)(stInfo.szFingerData1[i]));
        }
        sprintf(szBuff + strlen(szBuff), "|FingerDataTwo=");
        for (int i = 0; i < stInfo.nFinger2DataLen; i ++)
        {
            sprintf(szBuff + strlen(szBuff), "%02x", (BYTE)(stInfo.szFingerData2[i]));
        }
        sprintf(szBuff + strlen(szBuff), "|ICSerialNumber=|CardSerialNumber=");
    } else
    if (enMode == IDINFO_02)
    {
        sprintf(szBuff,
                "IDType=2|Name=%s|Sex=%s|Born=%s|Address=%s|IDCardNo=%s|GrantDept=%s|UserLifeBegin=%s|"
                "UserLifeEnd=%s|PassNO=%s|IssueNumber=%s",
                QString(stInfo.szName).trimmed().toStdString().c_str(),         // 姓名
                QString(stInfo.szSex).trimmed().toStdString().c_str(),          // 性别
                QString(stInfo.szBirthday).trimmed().toStdString().c_str(),     // 出生日期
                QString(stInfo.szAddress).trimmed().toStdString().c_str(),      // 住址信息
                QString(stInfo.szNumber).trimmed().toStdString().c_str(),       // 身份证号码
                QString(stInfo.szDepartment).trimmed().toStdString().c_str(),   // 签发机关
                QString(stInfo.szSTimeLimit).trimmed().toStdString().c_str(),   // 有效日期开始
                QString(stInfo.szETimeLimit).trimmed().toStdString().c_str(),   // 有效日期结束
                QString(stInfo.szPassport).trimmed().toStdString().c_str(),     // 通行证号码
                QString(stInfo.szIssue).trimmed().toStdString().c_str()         // 签发次数
               );
    } else
    if (enMode == IDINFO_03)
    {

    } else
    {
        return -3;
    }

    nSize = strlen(szBuff);
    nSize = (nSize >= nRetSize ? nRetSize - 1 : nSize);
    memcpy(lpRetStr, szBuff, nSize);

    return nSize;
}
