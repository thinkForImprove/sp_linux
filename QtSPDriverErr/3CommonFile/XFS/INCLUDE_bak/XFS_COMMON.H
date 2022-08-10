/******************************************************************************
*                                                                             *
* XFS_COMMON.h XFS - 共通处理                                                  *
*                                                                             *
* Included in the XFS Version 3.10 (29/11/2007)                               *
*                                                                             *
******************************************************************************/

#ifndef __INC_XFS_COMMON__H
#define __INC_XFS_COMMON__H

// #ifdef __cplusplus
// extern "C" {
// #endif
/* be aware of alignment */
#pragma pack (push, 1)

/* Service Class 各模块序号/版本 定义 */
/*

 ---------------------------Windows下序号定义---------------------------
          PTR  SIU  IDC  CDM  PIN  VDM  CIM  CAM  CRD  BCR  DEP  TTU
 CLASS_0   1    8    2    3    4    9    13   10   14   15   6    7
 CLASS_1   1    8    2    3    4    9    13   10   14   15   6    7
 CLASS_2   1    8    2    3    4    9    13   10   14   15   6    7
 CLASS_3   1    8    2    3    4    9    13   10   14   15   6    7
 CLASS_4   1    8    2    3    4    9    13   10   14   15   6    7
 CITIC     1    8    2    3    4    9    13   10   14   15   6    7
 SXXH      1    8    2    3    4    9    13   10   14   15   6    7


 ---------------------------LINUX下序号定义---------------------------(比较以CLASS_0为标准,不同标注*)
          PTR  SIU  IDC  CDM  PIN  VDM  CIM  CAM  CRD  BCR  DEP  TTU  ALM
 CLASS_0   1    2    3    4    5    7    9    10   14   15   15   21
 CLASS_1   1    8*   2*   3*   4*   20*  13*  10   14   15   15   21
 CLASS_2   1    2    4*   3*   5    20*  9    10   14   15   15   21
 CLASS_3   1    2    3    4    5    7    9    10   14   15   6*   6*
 CLASS_4   1    8*   2*   3*   4*   9*   13*  10   14   15   6*   7*    11

 CITIC     1    8*   2*   4    4*   7    13*  10   14   15   15   21
 SXXH      1    2    3    4    5    7    9    10   14   15   6*   6*

 PAB

 ---------------------------Windows/LINUX下版本定义---------------------------
          PTR   SIU   IDC   CDM   PIN   VDM   CIM   CAM   CRD   BCR   DEP   TTU
 VER_100  0001  0001  0001  0001  0001  0001  0001  0001  0001  0001  0001  0001 (Version 1.00)
 VER_300  0003  0003  0003  0003  0003  0003  0003  0003  0003  0003  0003  0003 (Version 3.00)
 VER_303  0303  0303  0303  0303  0303  0303  0303  0303  0303  0303  0303  0303 (Version 3.03)
 VER_310  0A03  0A03  0A03  0A03  0A03  0A03  0A03  0A03  0A03  0A03  0A03  0A03 (Version 3.10)
 VER_320  1403  1403  1403  1403  1403  1403  1403  1403  1403  1403  1403  1403 (Version 3.20)


 注:
CLASS_0(缺省): HanKou(汉口银行)
CLASS_1: CSCB(长沙银行)
CLASS_2: CMBC(民生银行)
CLASS_3: SXXH(山西信合)
CLASS_4: JIANGNAN(江南银行)
CITIC(中信银行)
SXXH(山西信合)
: PSBC(邮储):
*/


/* --------验证银行,指定使用的Service_class类别----------------
  宏定义: SET_BANK_CSCB(长沙银行), SET
*/
#if defined(SET_BANK_CSCB)
    #define __SERVICE_CLASS_1
#elif defined(SET_BANK_CMBC)
    #define __SERVICE_CLASS_2
#elif defined(SET_BANK_SXXH)
    #define __SERVICE_CLASS_3
#elif defined(SET_BANK_JIANGNAN)
    #define __SERVICE_CLASS_1
#else
    #define __SERVICE_CLASS_0
#endif

// SERVICE_CLASS_XXX 宏定义
#if defined(__SERVICE_CLASS_1)                                  // CLASS 类型 1
    #ifdef QT_WIN_LINUX_XFS
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (8)
        #define WFS_SERVICE_CLASS_IDC               (2)
        #define WFS_SERVICE_CLASS_CDM               (3)
        #define WFS_SERVICE_CLASS_PIN               (4)
        #define WFS_SERVICE_CLASS_VDM               (9)
        #define WFS_SERVICE_CLASS_CIM               (13)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (6)
        #define WFS_SERVICE_CLASS_TTU               (7)
    #else
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (8)
        #define WFS_SERVICE_CLASS_IDC               (2)
        #define WFS_SERVICE_CLASS_CDM               (3)
        #define WFS_SERVICE_CLASS_PIN               (4)
        #define WFS_SERVICE_CLASS_VDM               (20)
        #define WFS_SERVICE_CLASS_CIM               (13)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (15)
        #define WFS_SERVICE_CLASS_TTU               (21)
    #endif
#elif defined(__SERVICE_CLASS_2)                                // CLASS 类型 2
    #ifdef QT_WIN_LINUX_XFS
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (8)
        #define WFS_SERVICE_CLASS_IDC               (2)
        #define WFS_SERVICE_CLASS_CDM               (3)
        #define WFS_SERVICE_CLASS_PIN               (4)
        #define WFS_SERVICE_CLASS_VDM               (9)
        #define WFS_SERVICE_CLASS_CIM               (13)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (6)
        #define WFS_SERVICE_CLASS_TTU               (7)
    #else
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (2)
        #define WFS_SERVICE_CLASS_IDC               (5)
        #define WFS_SERVICE_CLASS_CDM               (3)
        #define WFS_SERVICE_CLASS_PIN               (5)
        #define WFS_SERVICE_CLASS_VDM               (20)
        #define WFS_SERVICE_CLASS_CIM               (9)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (15)
        #define WFS_SERVICE_CLASS_TTU               (21)
    #endif
#elif defined(__SERVICE_CLASS_3)                                // CLASS 类型 3
    #ifdef QT_WIN_LINUX_XFS
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (8)
        #define WFS_SERVICE_CLASS_IDC               (2)
        #define WFS_SERVICE_CLASS_CDM               (3)
        #define WFS_SERVICE_CLASS_PIN               (4)
        #define WFS_SERVICE_CLASS_VDM               (9)
        #define WFS_SERVICE_CLASS_CIM               (13)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (6)
        #define WFS_SERVICE_CLASS_TTU               (7)
    #else
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (2)
        #define WFS_SERVICE_CLASS_IDC               (3)
        #define WFS_SERVICE_CLASS_CDM               (4)
        #define WFS_SERVICE_CLASS_PIN               (5)
        #define WFS_SERVICE_CLASS_VDM               (7)
        #define WFS_SERVICE_CLASS_CIM               (9)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (6)
        #define WFS_SERVICE_CLASS_TTU               (6)
    #endif
#elif defined(__SERVICE_CLASS_4)                                // CLASS 类型 4
    #ifdef QT_WIN_LINUX_XFS
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (8)
        #define WFS_SERVICE_CLASS_IDC               (2)
        #define WFS_SERVICE_CLASS_CDM               (3)
        #define WFS_SERVICE_CLASS_PIN               (4)
        #define WFS_SERVICE_CLASS_VDM               (9)
        #define WFS_SERVICE_CLASS_CIM               (13)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (6)
        #define WFS_SERVICE_CLASS_TTU               (7)
    #else
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (8)
        #define WFS_SERVICE_CLASS_IDC               (2)
        #define WFS_SERVICE_CLASS_CDM               (3)
        #define WFS_SERVICE_CLASS_PIN               (4)
        #define WFS_SERVICE_CLASS_VDM               (9)
        #define WFS_SERVICE_CLASS_CIM               (13)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (6)
        #define WFS_SERVICE_CLASS_TTU               (7)
        #define WFS_SERVICE_CLASS_ALM               (11)
    #endif
#else                                                           // CLASS 类型 0 (缺省)
    #ifdef QT_WIN_LINUX_XFS
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (8)
        #define WFS_SERVICE_CLASS_IDC               (2)
        #define WFS_SERVICE_CLASS_CDM               (3)
        #define WFS_SERVICE_CLASS_PIN               (4)
        #define WFS_SERVICE_CLASS_VDM               (9)
        #define WFS_SERVICE_CLASS_CIM               (13)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (6)
        #define WFS_SERVICE_CLASS_TTU               (7)
    #else
        #define WFS_SERVICE_CLASS_PTR               (1)
        #define WFS_SERVICE_CLASS_SIU               (2)
        #define WFS_SERVICE_CLASS_IDC               (3)
        #define WFS_SERVICE_CLASS_CDM               (4)
        #define WFS_SERVICE_CLASS_PIN               (5)
        #define WFS_SERVICE_CLASS_VDM               (7)
        #define WFS_SERVICE_CLASS_CIM               (9)
        #define WFS_SERVICE_CLASS_CAM               (10)
        #define WFS_SERVICE_CLASS_CRD               (14)
        #define WFS_SERVICE_CLASS_BCR               (15)
        #define WFS_SERVICE_CLASS_DEP               (15)
        #define WFS_SERVICE_CLASS_TTU               (21)
    #endif
#endif

// SERVICE_CLASS_VERSION_XXX 宏定义
#if defined(SET_SERVICE_CLASS_VER_300)                             // Version 3.00
    #define WFS_SERVICE_CLASS_VERSION_PTR           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_SIU           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_IDC           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_CDM           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_PIN           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_VDM           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_CIM           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_CAM           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_CRD           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_DEP           (0x0003)
    #define WFS_SERVICE_CLASS_VERSION_TTU           (0x0003)
#elif defined(SET_SERVICE_CLASS_VER_303)                           // Version 3.03
    #define WFS_SERVICE_CLASS_VERSION_PTR           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_SIU           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_IDC           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_CDM           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_PIN           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_VDM           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_CIM           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_CAM           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_CRD           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_DEP           (0x0303)
    #define WFS_SERVICE_CLASS_VERSION_TTU           (0x0303)
#elif defined(SET_SERVICE_CLASS_VER_310)                           // Version 3.10
    #define WFS_SERVICE_CLASS_VERSION_PTR           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_SIU           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_IDC           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_CDM           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_PIN           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_VDM           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_CIM           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_CAM           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_CRD           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_DEP           (0x0A03)
    #define WFS_SERVICE_CLASS_VERSION_TTU           (0x0A03)
#elif defined(SET_SERVICE_CLASS_VER_320)                           // Version 3.20
    #define WFS_SERVICE_CLASS_VERSION_PTR           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_SIU           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_IDC           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_CDM           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_PIN           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_VDM           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_CIM           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_CAM           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_CRD           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_DEP           (0x1403)
    #define WFS_SERVICE_CLASS_VERSION_TTU           (0x1403)
#else                                                           // Version 1.00
    #define WFS_SERVICE_CLASS_VERSION_PTR           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_SIU           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_IDC           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_CDM           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_PIN           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_VDM           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_CIM           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_CAM           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_CRD           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_DEP           (0x0001)
    #define WFS_SERVICE_CLASS_VERSION_TTU           (0x0001)
#endif


/*   restore alignment   */
#pragma pack(pop)
// #ifdef __cplusplus
// } /*extern "C"*/
// #endif
#endif /* __INC_XFSBCR__H */
