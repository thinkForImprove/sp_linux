#ifndef DEVICE_PORT_H
#define DEVICE_PORT_H

#include <libudev.h>
#include <string>
#include "__common_def.h"
#include "device_port_global.h"

using namespace std;

// 返回值宏定义
#define DP_RET_SUCC             0       // 成功/存在
#define DP_RET_INST_ERR         -1      // 有关实例化失败
#define DP_RET_INPUT_INV        -2      // 无效的入参
#define DP_RET_NOTHAVE          -3      // 不存在



/****************************************************************************************************
 @功能:
 @参数:
 ****************************************************************************************************/
class DEVICE_PORT_EXPORT CDevicePort
{
DEFINE_STATIC_VERSION_FUNCTIONS("device_port", "0.0.0.0", TYPE_DYNAMIC)

public:
    static INT SearchVideoIdxFromVidPid(LPCSTR lpcVid, LPCSTR lpcPid);      // 根据Vid+Pid取得摄像设备索引序号
    static INT SearchDeviceNameIsHave(LPCSTR lpcDevName);                   // 查找设备名是否存在
    static INT SearchDeviceVidPidIsHave(LPCSTR lpcVid, LPCSTR lpcPid);      // 查找设备VidPid是否存在
    static INT SearchVideoXIsHave(WORD wID);                                // 查找/dev/videoX是否存在
    static INT SearchVideoIdxIsHave(WORD wIdx);                             // 查找摄像设备索引序号是否存在
    static INT GetComDevName(LPCSTR lpMode, LPSTR lpDevName, INT nDevNameLen);// 取COM串设备名

private:
    static INT SearchDeviceIsHave(WORD wMode, LPCSTR lpcDevName, LPCSTR lpcVid, LPCSTR lpcPid);// 查找设备是否存在
    static DWORD str_to_toupper(LPCSTR lpcSource, DWORD dwSourceSize, LPSTR lpDest, DWORD dwDestSize);
};

#endif // DEVICE_PORT_H
