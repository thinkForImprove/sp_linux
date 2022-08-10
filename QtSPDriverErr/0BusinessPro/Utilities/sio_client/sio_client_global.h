#ifndef SIO_CLIENT_GLOBAL_H
#define SIO_CLIENT_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(SIO_CLIENT_LIBRARY)
#  define SIO_CLIENT_EXPORT Q_DECL_EXPORT
#else
#  define SIO_CLIENT_EXPORT Q_DECL_IMPORT
#endif

#endif // SIO_CLIENT_GLOBAL_H
