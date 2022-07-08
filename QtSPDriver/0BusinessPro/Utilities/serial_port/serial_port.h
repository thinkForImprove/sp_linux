#ifndef SERIAL_PORT_H
#define SERIAL_PORT_H

#include "serial_port_global.h"
#include "__common_version_def.h"


typedef bool(*PFNRcvDataCallBack)(unsigned char* rcv_data, long rcv_len);

class SERIAL_PORT_EXPORT SerialPort
{
DEFINE_STATIC_VERSION_FUNCTIONS("serial_port", "0.0.0.0", TYPE_DYNAMIC);

public:
    SerialPort();
    virtual ~SerialPort();

public:
    bool close_port();
    bool open_port(const char* port, int baudrate, int data_bits, int stop_bits, const char parity, int flow_ctrl);
    long read_port(unsigned char* buffer, long buf_size, long timeout_by_ms, PFNRcvDataCallBack receive_end_callback);
    bool write_port(unsigned char* buffer, long data_size);


protected:
    int m_fd;
};

#endif // SERIAL_PORT_H
