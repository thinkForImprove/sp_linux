TEMPLATE = subdirs

# 定制编译顺序
CONFIG += ordered

# 子项目
SUBDIRS += \
    #utilities/algorithm \
    Utilities/cjson_object \
    Utilities/data_convertor \
    Utilities/file_access \
    Utilities/device_port \
    #utilities/serial_port \
    #utilities/sio_client \
    #utilities/synchronism \
    #utilities/data_structure \
    #utilities/memory_buffer \
    #utilities/log_ctrl \
    #utilities/timer_ctrl

SUBDIRS += \
    ShareHeaderFile \
    XfsSPIHelper \
    AllDevPort \
    IomcComm

