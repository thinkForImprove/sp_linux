#include "VirtualKBEvent.h"

CVirtualKBEvent::CVirtualKBEvent()
{
    m_uinputFd = -1;
}

int CVirtualKBEvent::OpenUinputDevice()
{
    int iRet = 0;
    m_uinputFd = open("/dev/uinput", O_RDWR | O_NDELAY);
    if(m_uinputFd < 0){
        printf("/dev/uinput 打开失败:%d\n", m_uinputFd);
        return -1;
    }

    //to set uinput dev
    struct uinput_user_dev stUinputDev;
    memset(&stUinputDev, 0, sizeof(stUinputDev));
    snprintf(stUinputDev.name, UINPUT_MAX_NAME_SIZE, "uinput-custom-dev");
    stUinputDev.id.version = 1;
    stUinputDev.id.bustype = BUS_VIRTUAL;

    ioctl(m_uinputFd, UI_SET_EVBIT, EV_SYN);            //同步事件
    ioctl(m_uinputFd, UI_SET_EVBIT, EV_KEY);            //键盘事件
//    ioctl(m_uinputFd, UI_SET_EVBIT, EV_MSC);            //其他事件

    for(int i = 0; i < 256; i++){
        ioctl(m_uinputFd, UI_SET_KEYBIT, i);
    }
//    ioctl(m_uinputFd, UI_SET_MSCBIT, KEY_CUSTOM_UP);
//    ioctl(m_uinputFd, UI_SET_MSCBIT, KEY_CUSTOM_DOWN);

    iRet = write(m_uinputFd, &stUinputDev, sizeof(stUinputDev));
    if(iRet < 0){
        printf("/dev/uinput write失败:%d\n", iRet);
        return -2;
    }

    iRet = ioctl(m_uinputFd, UI_DEV_CREATE);
    if(iRet < 0){
        printf("/dev/uinput ioctl[UI_DEV_CREATE]失败:%d\n", iRet);
        close(m_uinputFd);
        m_uinputFd = -1;
        return -3;
    }

    return 0;
}

bool CVirtualKBEvent::FireVirtualKeyEvent(unsigned short wKeyCode)
{
    //发送按键按下事件
    if(ReportKey(EV_KEY, wKeyCode, KEY_PRESS) != 0){
        return false;
    }
    //发送按键松开事件
    if(ReportKey(EV_KEY, wKeyCode, KEY_RELEASE) != 0){
        return false;
    }

    return true;
}

bool CVirtualKBEvent::CloseUinputDevice()
{
    if(m_uinputFd >= 0){
        close(m_uinputFd);
        m_uinputFd = -1;
    }

    return true;
}

bool CVirtualKBEvent::isOpen()
{
    return m_uinputFd >= 0;
}

int CVirtualKBEvent::ReportKey(unsigned int type, unsigned int keycode, unsigned int value)
{
    int iRet = 0;
    struct input_event key_event;
    memset(&key_event, 0, sizeof(struct input_event));

    gettimeofday(&key_event.time, NULL);
    key_event.type = type;
    key_event.code = keycode;
    key_event.value = value;
    iRet = write(m_uinputFd, &key_event, sizeof(struct input_event));
    if(iRet <= 0){
        printf("/dev/uinput write失败:%d\n", iRet);
        return -1;
    }

    gettimeofday(&key_event.time, NULL);
    key_event.type = EV_SYN;
    key_event.code = SYN_REPORT;
    key_event.value = 0;//event status sync
    iRet = write(m_uinputFd, &key_event, sizeof(struct input_event));
    if(iRet <= 0){
        printf("/dev/uinput write失败:%d\n", iRet);
        return -2;
    }

    return 0;
}
