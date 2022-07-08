#ifndef SERIAL_PORT_GLOBAL_H
#define SERIAL_PORT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SERIAL_PORT_LIBRARY)
#  define SERIAL_PORT_EXPORT Q_DECL_EXPORT
#else
#  define SERIAL_PORT_EXPORT Q_DECL_IMPORT
#endif

#endif // SERIAL_PORT_GLOBAL_H
