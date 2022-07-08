#include "AdapterUR2.h"
#include <QtCore/qglobal.h>

#if defined(IBRMADAPTER_LIBRARY)
#  define ADAPTERUR2SHARED_EXPORT Q_DECL_EXPORT
#else
#  define ADAPTERUR2SHARED_EXPORT Q_DECL_IMPORT
#endif

extern "C" ADAPTERUR2SHARED_EXPORT int CreateBRMAdapter(IBRMAdapter *&pAdapter)
{
    pAdapter = new CUR2Adapter();
    return WFS_SUCCESS;
}
