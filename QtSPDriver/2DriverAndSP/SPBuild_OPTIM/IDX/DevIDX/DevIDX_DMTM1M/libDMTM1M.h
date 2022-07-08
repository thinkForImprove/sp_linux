/***************************************************************
* 文件名称: libDMTM1M.h
* 文件描述: DMT-M1-M设备数据处理(提取自厂商Demo)
*
* 版本历史信息
* 变更说明: 建立文件
* 变更日期: 2023年5月16日
* 文件版本: 1.0.0.1
****************************************************************/

#ifndef LIB_DMTM1M_H
#define LIB_DMTM1M_H

#include "QtTypeDef.h"
#include "QtTypeInclude.h"

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iconv.h>



class DMT_M1_M
{
public:
    DMT_M1_M() {};
    ~DMT_M1_M() {};

public:
    // 函数功能：查询卡类型
    // 输入参数：buffer -- 基本信息
    // 返回值：1:身份证, 2:外国人, 3:港澳台
    int dmt_sm_getcardtype(char* buffer)
    {
        // 248-249字节：2 身份证件类型
        //memcpy(type, buffer + 248, 2);
        if ((buffer[248] == ' ') && (buffer[249] == 0x00))
        {
            return 1;
        } else
        if ((buffer[248] == 73) && (buffer[249] == 0x00))
        {
            return 2;
        } else
        if ((buffer[248] == 74) && (buffer[249] == 0x00))
        {
            return 3;
        }
        return -1;
    }

    // 指定编码格式截取字符串
    int code_convert(char *from_charset, char *to_charset, char *inbuf, size_t inlen,
                     char *outbuf, size_t outlen)
    {
        iconv_t cd;
        char **pin = &inbuf;
        char **pout = &outbuf;
        cd = iconv_open(to_charset, from_charset);
        if (cd == 0)
        {
            return -1;
        }

        memset(outbuf, 0, outlen);
        if (iconv(cd, pin, &inlen, pout, &outlen) == -1)
        {
            return -1;
        }

        iconv_close(cd);
        *pout = 0x00; //'\0';
        return 0;
    }

    // BMP文件信息数据填充
    void restoreBmp(unsigned char *srcArray, unsigned char* desArray)
    {
        // 14个字节的bmp文件头
        unsigned char fileHeader[] = {0x42, 0x4d, 0xce, 0x97, 0x00,
                                      0x00, 0x00, 0x00, 0x00, 0x00,
                                      0x36, 0x00, 0x00, 0x00};

        // 40个字节的位图信息头
        unsigned char imageHeader[] = {0x28, 0x00, 0x00, 0x00, 0x66,
                                       0x00, 0x00, 0x00, 0x7e, 0x00,
                                       0x00, 0x00, 0x01, 0x00, 0x18,
                                       0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00};

        memcpy(desArray, fileHeader, 14);
        memcpy(desArray+14, imageHeader, 40);
        // 解析得到的数据为102*126个点的BGR像素值，需要将每个点的B和R值互换
        // 要求每行的数据的长度必须是4的倍数，如果不够需要进行比特填充（以0填充）
        int j = 54;
        int i;
        for (i = 1;  i<=38556/3;  i++)
        {
            // 将B、R值互换
            desArray[j++] = srcArray[3*i-1];
            desArray[j++] = srcArray[3*i-2];
            desArray[j++] = srcArray[3*i-3];
            // 每行需要填充2个字节
            if (i%102 == 0)
            {
                desArray[j++] = 0x00;
                desArray[j++] = 0x00;
            }
        }
    }

    // BMP数据写入指定文件
    void SavBMP(const char *pathname, const void *buf, size_t count)
    {
        int fd, fdw;
        fd = open(pathname, O_RDWR|O_CREAT);
        if (fd < 0)
        {
            perror("open pathname");
        }
        write(fd, buf, count);

        close(fd);
    }

    // 函数功能：从基本信息中提取姓名
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 姓名
    int basemsg_name(char *inbuf, char *outbuf)
    {
        return code_convert("ucs-2le", "utf-8", inbuf, 30, outbuf, 30);
    }

    // 函数功能：从基本信息中提取性别
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 性别
    int basemsg_gender(char *inbuf, char *outbuf)
    {
        return code_convert("ucs-2le", "utf-8", inbuf+30, 2, outbuf, 2);
    }

    // 函数功能：从基本信息中提取名族
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 名族
    int basemsg_nation(char *inbuf, char *outbuf)
    {
        return code_convert("ucs-2le", "utf-8", inbuf+32, 4, outbuf, 4);
    }

    // 函数功能：从基本信息中提取出生
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 出生
    int basemsg_birth(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 8; j ++)
        {
             outbuf[j] = inbuf[36+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }

    // 函数功能：从基本信息中提取住址
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 住址
    int basemsg_address(char *inbuf, char *outbuf)
    {
        return code_convert("ucs-2le", "utf-8", inbuf+52, 70, outbuf, 100);
    }

    // 函数功能：从基本信息中提取身份证号
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 身份证号
    int basemsg_idnum(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 18; j ++)
        {
             outbuf[j] = inbuf[122+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }

    // 函数功能：从基本信息中提取签发机关
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 签发机关
    int basemsg_issue(char *inbuf, char *outbuf)
    {
        return code_convert("ucs-2le", "utf-8", inbuf+158, 30, outbuf, 30);
    }

    // 函数功能：从基本信息中提取有效起始日期
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 有效起始日期
    int basemsg_startdate(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 8; j ++)
        {
             outbuf[j] = inbuf[188+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }

    // 函数功能：从基本信息中提取有效截止日期
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 有效截止日期
    int basemsg_enddate(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 8; j ++)
        {
             outbuf[j] = inbuf[204+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }

    // 函数功能：从基本信息中提取最新住址
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 最新住址
    int basemsg_newaddress(char *inbuf, char *outbuf)
    {
        return code_convert("ucs-2le", "utf-8", inbuf+220, 36, outbuf, 36);
    }

    // 函数功能：从基本信息中提取通行证号
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf -- 通行证号
    int basemsg_permit(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 9; j ++)
        {
            outbuf[j] = inbuf[220+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }

    // 函数功能：从基本信息中提取签发次数
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf --签发次数
    int basemsg_times(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 2; j ++)
        {
            outbuf[j] = inbuf[238+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }

    // 函数功能：从基本信息中提取永久居留证号
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf --永久居留证号
    int basemsg_Permit_Number(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 15; j ++)
        {
            outbuf[j] = inbuf[122+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }

    // 函数功能：从基本信息中提取国籍或所在地区代码
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf --国籍或所在地区代码
    int basemsg_Code(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 3; j ++)
        {
            outbuf[j] = inbuf[152+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }
    /*
    函数功能：从外国人基本信息中提取出生
    输入参数：inbuf -- 基本信息
    输出参数：outbuf -- 出生
    */
    int basemsg_Foreignbirth(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 8; j ++)
        {
            outbuf[j] = inbuf[220+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }

    // 函数功能：从基本信息中提取证件版本号
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf --证件版本号
    int basemsg_Version(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 2; j ++)
        {
            outbuf[j] = inbuf[236+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }

    // 函数功能：从基本信息中提取当前申请受理机关代码
    // 输入参数：inbuf -- 基本信息
    // 输出参数：outbuf --当前申请受理机关代码
    int basemsg_IssueID(char *inbuf, char *outbuf)
    {
        int j = 0;
        for (j = 0; j < 4; j ++)
        {
             outbuf[j] = inbuf[240+j*2];
        }
        outbuf[j] = 0;
        return 0;
    }
};

#endif // LIB_DMTM1M_H
