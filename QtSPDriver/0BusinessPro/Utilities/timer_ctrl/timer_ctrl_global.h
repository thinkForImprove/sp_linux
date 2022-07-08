#ifndef TIMER_CTRL_GLOBAL_H
#define TIMER_CTRL_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(TIMER_CTRL_LIBRARY)
#  define TIMER_CTRL_EXPORT Q_DECL_EXPORT
#else
#  define TIMER_CTRL_EXPORT Q_DECL_IMPORT
#endif

#endif // TIMER_CTRL_GLOBAL_H
