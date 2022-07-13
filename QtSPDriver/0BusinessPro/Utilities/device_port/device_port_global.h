#ifndef DEVICE_PORT_GLOBAL_H
#define DEVICE_PORT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(DEVICE_PORT_LIBRARY)
#  define DEVICE_PORT_EXPORT Q_DECL_EXPORT
#else
#  define DEVICE_PORT_EXPORT Q_DECL_IMPORT
#endif

#endif // DEVICE_PORT_GLOBAL_H
