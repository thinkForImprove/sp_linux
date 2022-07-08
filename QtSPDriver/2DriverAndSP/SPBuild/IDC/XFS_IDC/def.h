/***************************************************************
* 文件名称：def.h
* 文件描述：用于声明 XFS_XXX 与 DevXXX 共用的变量定义
*
* 版本历史信息
* 变更说明：建立文件
* 变更日期：2021年11月6日
* 文件版本：1.0.0.1
****************************************************************/
#ifndef DEF_H
#define DEF_H

#include <string.h>
#include <QtCore/qglobal.h>
#include <QtTypeDef.h>

//----------宏定义----------------------------------------------------

static const BYTE byXFSVRTU[17] = {"IDC00010100"};          // XFS_IDC 版本号
static const BYTE byDevIDCVRTU[17] = {"DevIDC010100"};      // DevIDC 版本号
static const BYTE byDevCRMVRTU[17] = {"DevCRM010100"};      // DevCRM 版本号


// 设备类型

// 退卡模块(CRM)相关声明
// CRM设备类型
#define CRM_DEV_CRT730B         0       // 退卡模块:CRT-730B

// CRM设备类型(DevCRM.cpp区分不同类型使用)
#define ICRM_TYPE_CRT730B       "0"     // 退卡模块:CRT-730B

#endif // DEF_H
