#include "TSingleton.h"

#ifdef SINGLETON_SUPPORT_MULT_THREAD
CSimpleCriticalSection g_SingletonCritSect;
#endif //SINGLETON_SUPPORT_MULT_THREAD

