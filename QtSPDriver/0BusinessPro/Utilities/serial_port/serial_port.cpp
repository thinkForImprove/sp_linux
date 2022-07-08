#include "framework.h"
#include "serial_port.h"


SerialPort::SerialPort() :
    m_fd(-1)
{
}

SerialPort::~SerialPort()
{
}


/**
 @功能：	关闭串口
 @参数：	无
 @返回：	true：成功     false：失败
 */
bool SerialPort::close_port()
{
    int ret = 0;

    if (m_fd == -1) {
        return true;
    }

    ret = close(m_fd);
    if (ret == 0) {
        m_fd = -1;
        return true;
    }

    REPORT_APP_EVENT_API(EVENTLOG_ERROR_TYPE, get_product_name(), "close", ret, true);

    return false;
}

/**
 @功能：	打开串口
 @参数：	port：串口文件（如："/dev/ttyAMA0"）
 @      baudrate：波特率                data_bits：数据位
 @      stop_bits：停止位（1, 1.5, 2）   parity：奇偶校验（N/n, O/o, E/e）
 @      flow_ctrl：控制流（0：无, 1：RTS/CTS）
 @返回：	true：成功     false：失败
 */
bool SerialPort::open_port(const char* port, int baudrate, int data_bits, int stop_bits, const char parity, int flow_ctrl)
{
    static std::map<int, int> baudrate_table;
    std::map<int, int>::iterator baudrate_it;
    if (baudrate_table.size() == 0) {
        baudrate_table.insert(std::map<int, int>::value_type(50, B50));
        baudrate_table.insert(std::map<int, int>::value_type(75, B75));
        baudrate_table.insert(std::map<int, int>::value_type(110, B110));
        baudrate_table.insert(std::map<int, int>::value_type(134, B134));
        baudrate_table.insert(std::map<int, int>::value_type(150, B150));
        baudrate_table.insert(std::map<int, int>::value_type(200, B200));
        baudrate_table.insert(std::map<int, int>::value_type(300, B300));
        baudrate_table.insert(std::map<int, int>::value_type(600, B600));
        baudrate_table.insert(std::map<int, int>::value_type(1200, B1200));
        baudrate_table.insert(std::map<int, int>::value_type(1800, B1800));
        baudrate_table.insert(std::map<int, int>::value_type(2400, B2400));
        baudrate_table.insert(std::map<int, int>::value_type(4800, B4800));
        baudrate_table.insert(std::map<int, int>::value_type(9600, B9600));
        baudrate_table.insert(std::map<int, int>::value_type(19200, B19200));
        baudrate_table.insert(std::map<int, int>::value_type(38400, B38400));
        baudrate_table.insert(std::map<int, int>::value_type(57600, B57600));
        baudrate_table.insert(std::map<int, int>::value_type(115200, B115200));
        baudrate_table.insert(std::map<int, int>::value_type(230400, B230400));
        baudrate_table.insert(std::map<int, int>::value_type(460800, B460800));
        baudrate_table.insert(std::map<int, int>::value_type(500000, B500000));
        baudrate_table.insert(std::map<int, int>::value_type(576000, B576000));
        baudrate_table.insert(std::map<int, int>::value_type(921600, B921600));
        baudrate_table.insert(std::map<int, int>::value_type(1000000, B1000000));
        baudrate_table.insert(std::map<int, int>::value_type(1152000, B1152000));
        baudrate_table.insert(std::map<int, int>::value_type(1500000, B1500000));
        baudrate_table.insert(std::map<int, int>::value_type(2000000, B2000000));
        baudrate_table.insert(std::map<int, int>::value_type(2500000, B2500000));
        baudrate_table.insert(std::map<int, int>::value_type(3000000, B3000000));
        baudrate_table.insert(std::map<int, int>::value_type(3500000, B3500000));
        baudrate_table.insert(std::map<int, int>::value_type(4000000, B4000000));
    }
    static std::map<int, int> databits_table;
    std::map<int, int>::iterator databits_it;
    if (databits_table.size() == 0) {
        databits_table.insert(std::map<int, int>::value_type(5, CS5));
        databits_table.insert(std::map<int, int>::value_type(6, CS6));
        databits_table.insert(std::map<int, int>::value_type(7, CS7));
        databits_table.insert(std::map<int, int>::value_type(8, CS8));
    }
    struct termios port_attr;

    if ((port == nullptr) || (strlen(port) == 0) ||
            (baudrate <= 0) || (data_bits <= 0) ||
            ((stop_bits != 1) && (stop_bits != 2)) ||
            (parity == '\0') || (flow_ctrl < 0)) {
        return false;
    }
    baudrate_it = baudrate_table.find(baudrate);
    if (baudrate_it == baudrate_table.end()) {
        return false;
    }
    databits_it = databits_table.find(data_bits);
    if (databits_it == databits_table.end()) {
        return false;
    }

    close_port();

    // Open
    m_fd = open(port, O_RDWR | O_NOCTTY | O_NONBLOCK);
    if (m_fd == -1) {
        REPORT_APP_EVENT_API_MSG(EVENTLOG_ERROR_TYPE, get_product_name(), "open", m_fd, true, port);
        return false;
    }
    tcflush(m_fd, TCIOFLUSH);
    if (tcgetattr(m_fd, &port_attr) == -1) {
        REPORT_APP_EVENT_API(EVENTLOG_ERROR_TYPE, get_product_name(), "tcgetattr", -1, true);
        close_port();
        return false;
    }
    port_attr.c_cflag |= (CLOCAL | CREAD);
    port_attr.c_cflag &= (~CSIZE);
    port_attr.c_iflag = IGNBRK;
    port_attr.c_lflag &= (~(ICANON | ECHO | ECHOE | ISIG));
    port_attr.c_oflag &= (~OPOST);
    port_attr.c_cc[VTIME] = 0;
    port_attr.c_cc[VMIN] = 0;

    // Baudrate
    cfsetispeed(&port_attr, baudrate_it->second);
    cfsetospeed(&port_attr, baudrate_it->second);

    // Databits
    port_attr.c_cflag |= databits_it->second;

    // Stopbits
    if (stop_bits == 1) {
        port_attr.c_cflag &= (~CSTOPB);
    } else {
        port_attr.c_cflag |= CSTOPB;
    }

    // Parity
    switch (parity) {
    case 'N':
    case 'n':
        //No parity
        port_attr.c_iflag &= (~INPCK);
        port_attr.c_cflag &= (~PARENB);
        break;
    case 'O':
    case 'o':
        //odd parity
        port_attr.c_iflag |= INPCK;
        port_attr.c_cflag |= (PARENB | PARODD);
        break;
    case 'E':
    case 'e':
        //even parity
        port_attr.c_iflag |= INPCK;
        port_attr.c_cflag |= PARENB;
        port_attr.c_cflag &= (~PARODD);
        break;
    default:
        close_port();
        return false;
        break;
    }

    // Flowctrl
    if (flow_ctrl == 0) {
        port_attr.c_cflag &= (~CRTSCTS);
    } else {
        port_attr.c_cflag |= CRTSCTS;
    }

    // Setattr
    if (tcsetattr(m_fd, TCSANOW, &port_attr) == -1) {
        REPORT_APP_EVENT_API(EVENTLOG_ERROR_TYPE, get_product_name(), "tcsetattr", -1, true);
        close_port();
        return false;
    }

    return true;
}

/**
 @功能：	读取串口数据
 @参数：	buffer：数据缓冲区        buf_size：缓冲区长度
 @      timeout_by_ms：超时时间（ms单位，<=0：无超时）
 @      receive_end_callback：接收数据是否结束的回调函数（返回true代表结束）
 @返回：	>0：成功（数据长度）     =0：失败（超时）   <0：失败
 */
long SerialPort::read_port(unsigned char* buffer, long buf_size, long timeout_by_ms, PFNRcvDataCallBack receive_end_callback)
{
    long read_len = 0;
    fd_set rfds;
    struct timespec time_start;
    struct timespec time_end;
    struct timeval timeout;
    int ret = 0;

    if (m_fd == -1) {
        return -1;
    }
    if ((buffer == nullptr) || (buf_size == 0)) {
        return -1;
    }

    FD_ZERO(&rfds);
    FD_SET(m_fd, &rfds);
    if (!FD_ISSET(m_fd, &rfds)) {
        REPORT_APP_EVENT_API(EVENTLOG_ERROR_TYPE, get_product_name(), "FD_ISSET", false, true);
        return -1;
    }

    clock_gettime(CLOCK_REALTIME, &time_start);
    do {
        timeout.tv_sec = 0;
        timeout.tv_usec = 1000 * 100;       //100ms
        ret = select(m_fd + 1, &rfds, NULL, NULL, &timeout);
        if (ret > 0) {
            long once_len = (long)read(m_fd, buffer + read_len, buf_size - read_len);
            if (once_len < 0) {
                REPORT_APP_EVENT_API(EVENTLOG_ERROR_TYPE, get_product_name(), "read", once_len, true);
                return once_len;
            }
            read_len += once_len;
            if (receive_end_callback) {
                if (receive_end_callback(buffer, read_len)) {
                    break;
                }
            } else {
                break;
            }
        } else if (ret < 0) {
            REPORT_APP_EVENT_API(EVENTLOG_ERROR_TYPE, get_product_name(), "select", ret, true);
            return (long)ret;
        } else {

        }

        clock_gettime(CLOCK_REALTIME, &time_end);
        time_t interval = (time_end.tv_sec - time_start.tv_sec) * 1000 +
                          (time_end.tv_nsec - time_start.tv_nsec) / (1000 * 1000);
        if (interval > timeout_by_ms) {
            return 0;
        }

        if (buf_size <= read_len) {
            return -1;
        }
    } while (FD_ISSET(m_fd, &rfds));

    return read_len;
}

/**
 @功能：	写入串口数据
 @参数：	buffer：数据缓冲区        data_size：数据长度
 @返回：	true：成功     false：失败
 */
bool SerialPort::write_port(unsigned char* buffer, long data_size)
{
    long write_len = 0;

    if (m_fd == -1) {
        return false;
    }
    if ((buffer == nullptr) || (data_size <= 0)) {
        return false;
    }

    write_len = write(m_fd, buffer, data_size);
    if (write_len <= 0) {
        REPORT_APP_EVENT_API(EVENTLOG_ERROR_TYPE, get_product_name(), "write", write_len, true);
        return false;
    }

    return true;
}
