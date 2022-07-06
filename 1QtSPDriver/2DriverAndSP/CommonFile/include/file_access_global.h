#ifndef FILE_ACCESS_GLOBAL_H
#define FILE_ACCESS_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(FILE_ACCESS_LIBRARY)
#  define FILE_ACCESS_EXPORT Q_DECL_EXPORT
#else
#  define FILE_ACCESS_EXPORT Q_DECL_IMPORT
#endif

#endif // FILE_ACCESS_GLOBAL_H
