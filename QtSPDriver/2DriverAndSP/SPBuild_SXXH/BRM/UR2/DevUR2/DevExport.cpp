#include "DevUR2.h"
//#include "devur2_global.h"

extern "C" DEVUR2SHARED_EXPORT int CreateURDevice(const char *pName, IURDevice *&pDevice)
{
    if (pName == nullptr || strstr(pName, "UR2") != nullptr)
    {
        pDevice = new CUR2Drv;
    }
    else
    {
        pDevice = nullptr;
        return ERR_UR_NO_DEVICE;
    }

    return ERR_UR_SUCCESS;
}
