#ifndef LOG_CTRL_GLOBAL_H
#define LOG_CTRL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(LOG_CTRL_LIBRARY)
#  define LOG_CTRL_EXPORT Q_DECL_EXPORT
#else
#  define LOG_CTRL_EXPORT Q_DECL_IMPORT
#endif

#endif // LOG_CTRL_GLOBAL_H
