#ifndef CJSON_OBJECT_GLOBAL_H
#define CJSON_OBJECT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CJSON_OBJECT_LIBRARY)
#  define CJSON_OBJECT_EXPORT Q_DECL_EXPORT
#else
#  define CJSON_OBJECT_EXPORT Q_DECL_IMPORT
#endif

#endif // CJSON_OBJECT_GLOBAL_H
