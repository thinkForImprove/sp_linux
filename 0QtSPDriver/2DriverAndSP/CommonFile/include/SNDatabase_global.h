#ifndef SNDATABASE_GLOBAL_H
#define SNDATABASE_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SNDATABASE_LIBRARY)
#  define SNDATABASE_EXPORT Q_DECL_EXPORT
#else
#  define SNDATABASE_EXPORT Q_DECL_IMPORT
#endif

#endif // SNDATABASE_GLOBAL_H
