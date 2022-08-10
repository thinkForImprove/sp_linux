#ifndef WHITELISTCHECK_H
#define WHITELISTCHECK_H
#include <QtCore/qglobal.h>

#if defined(WHITELISTCHECK_LIBRARY)
#define WHITELISTCHECK_EXPORT Q_DECL_EXPORT
#else
#define WHITELISTCHECK_EXPORT Q_DECL_IMPORT
#endif

/************************************************************
********************************作者：zyfang
********************************日期：20191203
********************************函数功能：白名单校验
********************************输入数据：无
********************************输出数据：无
******************************** 返回值：true 校验通过 false校验不通过
*************************************************************/
extern "C" WHITELISTCHECK_EXPORT bool WhiteListCheck();


#endif // WHITELISTCHECK_H
