#ifndef VIRTUALKBEVENT_H
#define VIRTUALKBEVENT_H
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <linux/input.h>
#include <linux/uinput.h>

#define KEY_PRESS   0x01
#define KEY_RELEASE 0x00

class CVirtualKBEvent
{
public:
    CVirtualKBEvent();

    //打开设备/dev/uinput并进行初始化设定
    int OpenUinputDevice();

    //发送虚拟按键事件
    bool FireVirtualKeyEvent(unsigned short wKeyCode);

    //关闭设备/dev/uinput
    bool CloseUinputDevice();

    bool isOpen();
private:
    int m_uinputFd;

    int ReportKey(unsigned int type, unsigned int keycode, unsigned int value);
};

#endif // VIRTUALKBEVENT_H
