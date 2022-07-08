#include "CryptyData.h"


extern "C" Q_DECL_EXPORT int CreateCryptData(ICryptData *&pInst)
{
    pInst = new CEncryptData;
    return 0;
}
