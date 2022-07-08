/*******************************************************************************
/**
 * @mainpage WB-CS10 API
 *
 * Introduction
 * ============
 *
 * WB-CS10 is a device control library written in C/C++ that is intended to take
 * care of the linux like system when writing software that uses the WB-CS10 device.
 *
 * 
 * WB-CS10 device have three mode, it have serial port or USB HID interface.
 * - 1. Active mode, auto uploading data mode, after swiping the card, device will automatically uploading data
 * - 2. Passive mode, command mode, should send command of enable swiping/read card first, then swiping card, device will automatically uploading data
 * - 3. Passive mode, cache mode, should send command of enable swiping first, then swiping card, the data cached in device, must send command of get track data to get data	
 *
 * USB HID interface of the command mode have the following two mode.
 * - 1. USB HID keyboard mode, active mode, auto read track and uploading data, just using an text file to test this function.
 * - 2. USB HID User-defined mode, passive mode, only could send command to get track data. while sending command, the USB HID keyboard mode is disable.
 *
 * API information
 * ===============
 *
 * The API has been designed from scratch. 
 *
 * The following subsections will help explain the principles of the API.
 *
 * Headers
 * -------
 *
 * To use WB-CS10 functions in your code, you should include the
 * wb-cs10.h header, i.e. "#include <wb-cs10.h>".
 *
 * Functions
 * ---------
 *
 * The functions provided by the library are documented in detail in
 * the following sections:
 *
 * - @ref LibVersion (get library version)
 * - @ref Enumeration (obtaining a list of serial ports on the system)
 * - @ref Debug (monitor read/write data and open/close log)
 * - @ref Ports (opening, closing and getting information about ports)
 * - @ref Device (getting or setting device like get device firmware version)
 * - @ref Magnetic(read magnetic card track data)
 * 
 * Data structures
 * ---------------
 *
 * The library defines one data structures:
 *
 * - @ref port_name, which represents a serial port path.
 *   See @ref Enumeration.
 * - @ref hid_port, which represents a usb hid port information.
 *   See @ref Enumeration.	
 *
 * Return codes and error handling
 * -------------------------------
 *
 * Most functions have return type @ref APIRETURN, please @see APIRETURN for details.
 *
 * Thread safety
 * -------------
 *
 * Certain combinations of calls can be made concurrently, as follows.
 *
 * - Calls using different ports may always be made concurrently, i.e.
 *   it is safe for separate threads to handle their own ports.
 *
 * If two calls, on the same port, do not fit into one of these categories
 * each, then they may not be made concurrently.
 *
 * Debugging
 * ---------
 *
 * The library can output extensive tracing and debugging information. The
 * simplest way to use this is to set the environment variable
 * LIBSERIALPORT_DEBUG to any value; messages will then be output to the
 * standard error stream.
 */

#ifndef _WB_CS10_H_
#define _WB_CS10_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/*******************************************************************************/
/**
 * Macro preceding public API functions
 *
 * 公开函数头
 */
#define LIB_API __attribute__((visibility("default")))

/**
 * Macro preceding private functions
 *
 * 私有函数头
 */
#define LIB_PRIV __attribute__((visibility("hidden")))

/*******************************************************************************
* C90 does not support the boolean data type.
* C99 does include it with this include
* #include <stdbool.h>
* compiler does not support C99, you can define it yourself
********************************************************************************/
//typedef enum { false, true } bool;

/**
 * The @ref port_name device path name size
 *
 * 设备名称长度
 */
#ifndef NAME_SIZE
#undef NAME_SIZE
#define NAME_SIZE	64
#endif

/******************************************************************************/
/**
 * @struct port_name
 * struct for serial port port name.
 */

 /**
  * @struct port_name
  * 串口设备名字结构.
  */
struct port_name
{
	char path[NAME_SIZE];	///< device path("/dev/ttyS0")      设备路径("/dev/ttyS0")
};

/**
 * @struct hid_port
 * struct for usbh hid port information.
 */

 /**
  * @struct hid_port
  * 串口设备名字结构.
  */
struct hid_port
{
	char 			path[NAME_SIZE];		///< device path      			设备路径
	unsigned short 	vendor_id; 				///< device vender id      		设备厂商号
	unsigned short 	product_id; 			///< device product id      	设备产品号
	int 			interface_number;		///< device interface number    设备接口号，默认0, 如果是复合设备可能有多个接口
};

/******************************************************************************/
/**
 * The default device baudrate.
 */
/**
 * 默认波特率.
 */
#define LIB_DEFALUT_BAUDRATE		9600
/******************************************************************************/

/**
 * monitor data call back function.
 */

/**
 * 监控发送接受数据回调.
 */
typedef void (* monitorcallback)(const void *pdata, int datalen);
/******************************************************************************/

/**
 * API return value.
 */
/**
 * API 返回值.
 */
typedef enum APIRETURN
{
    APICMDInProgress 	= -11, 	///< command in progress        	命令运行中
	APICMDCancel		= -10,	///< command cancel 				命令取消
	APINotSupportCMD	= -9,	///< not support command	        不支持命令
	APIUnknownError		= -8,	///< unknown error 					未知错误
	APIFrameError		= -7,	///< data not conform to protocol 	数据不符合协议
	APIReadTimeOut		= -6,	///< read data timeout				读数据超时
	APIReadError		= -5,	///< read data error				读数据出错
	APIWriteTimeOut		= -4,	///< write data timeout				写数据超时
	APIWriteError		= -3,	///< write data error				写数据出错
	APICMDParmError		= -2,	///< API parameter error			API 参数错误
	APICMDFailure		= -1,	///< command execute error			命令执行失败
	APICMDSuccess		= 0,	///< command execute ok				命令执行成功
} APIReturn;

/**
 * Magnetic card track error code.                                                           
 */
 
 /**
 * 磁道错误码.                                                           
 */
typedef enum TRACKERRORCODE
{
	TrackNotRead	= 0xE0, ///< not read this track.
	TrackSTX		= 0xE1, ///< read track error, no starting bit STX.
	TrackETX		= 0xE2,	///< read track error, no ending bit ETX.
	TrackVRC		= 0xE3,	///< read track error, bit check error VRC.
	TrackLRC		= 0xE4,	///< read track error, byte check error LRC.
	TrackBlank		= 0xE5,	///< read track error, blank track.
} TrackErrorCode;

/******************************************************************************/
/** 
* @defgroup LibVersion Library Versions
* 
*  Version number string querying functions
*
*　库版本信息模块，获取版本信息函数.
* 
* @{
*/ 
/******************************************************************************/
/**
 * Get the library version number as a string.
 *
 * @return The library version number string. The returned string is
 *	 static and thus should NOT be freed by the caller.
 *
 * @since 0.1
 */
const char *GetLibVersion(void);
/******************************************************************************/
/** @} */
/** 
* @defgroup Enumeration Port enumeration
*
* Enumerating the serial ports of a system.
*
* 枚举系统中所有串口.
* 
* @{ 
*/
/*******************************************************************************/
/**
 * List the serial ports available on the system.
 *
 * The result obtained is a string of all the serial ports path terminated by a 
 * one '\0', The last one path is terminated by a two '\0'. The user should 
 * allocate a variable of type "char *" which its memory is 256 byte at least 
 * to receive the result.
 * 
 * The result should be freed after use by yourself
 *
 * the result pathlist list have following format:
 * "/dev/ttyS0\0"/dev/ttyS1"\0\0"
 *
 * @param[out] pathlist If any error is returned, the variable pointed to by
 *                      pathlist will be set to "". Otherwise, it will be set
 *                      to point to the serial ports path string. Must not be NULL.
 * @param[in] pathlen the size of the "pathlist" buffer.
 *
 * @return > 0 the number of serial ports, == 0 don't have one serial ports.
 *
 * @since 0.1
 */

/**
 * 列出操作系统上存在的串口.
 *
 * 若成功，pathlist 有为以下格式:
 * "/dev/ttyS0\0"/dev/ttyS1"\0\0"
 *
 * @param[out] pathlist 若出错, pathlist 为 "". 否则, 指向串口路径字符串. 不能为NULL.
 * @param[in] pathlen 参数"pathlist"缓冲区长度.
 *
 * @return > 0 存在串口数量, == 0 不存在串口.
 *
 * @since 0.1
 */
int ListPortSPath(char *pathlist, int pathlen);

/**
 * List the serial ports available on the system.
 *
 * The result obtained is an array of port_name structures, The user should 
 * allocate a array of type "struct sp_port" and pass the array to receive the result.
 *
 * @param[out] patharray the array for storing the serial ports. Must not be NULL.
 * @param[in] arlen the number of the "patharray" array size.
 *
 * @return > 0 the number of serial ports, == 0 don't have one serial ports.
 *
 * @since 0.1
 */

/**
 * 列出操作系统上存在的串口.
 *
 * @param[out] patharray 存储串口路径数组结构体，不能为NULL.
 * @param[in] arlen 参数"patharray"数组结构体个数.
 *
 * @return > 0 存在串口数量, == 0 不存在串口.
 *
 * @since 0.1
 */
int ListPort(struct port_name *patharray, int arlen);

/**
 * List the USB HID device port available on the system.
 * WB-CS10 Device: vendor_id：0x88BE product_id: 0x0002
 * WB-CS10-09 Device: vendor_id：0x3150 product_id: 0x1110
 *
 * @param[in] vendor_id  vender id of device.
 * @param[in] product_id  product id of device.
 *
 * @return > 0 the number of USB HID port, == 0 don't have one ports.
 *
 * @since 0.1
 */
 
/**
 * 列出操作系统上连接指定厂商号和产品号的USB HID设备.
 * <p>
 * WB-CS10 设备 vendor_id：0x88BE product_id: 0x0002  <br>
 * WB-CS10-09 设备 vendor_id：0x3150 product_id: 0x1110
 * 
 * @param[in] vendor_id 设备厂商号.
 * @param[in] product_id 设备产品号.
 *
 * @return > 0 存在设备数量, == 0 不存在.
 *
 * @since 0.1
 */
int ListHIDPort(unsigned short vendor_id, unsigned short product_id);

/**
 * List all the USB HID ports available on the system.
 *
 * @param[in,out] phidarray  usb hid device info array.
 * @param[in] arraylen  number of usb hid device info array.
 *
 * @return > 0 the number of USB HID port, == 0 don't have one ports.
 *
 * @since 0.1
 */
 
/**
 * 列出操作系统上连接的USB HID设备.
 * 
 * @param[in,out] phidarray USB HID设备信息数组缓存.
 * @param[in] arraylen USB HID设备信息数组缓存数量.
 *
 * @return > 0 存在设备数量, == 0 不存在.
 *
 * @since 0.1
 */
int ListHIDPortS(struct hid_port *phidarray, int arraylen);
/*******************************************************************************/
/** @} */

/** 
* @defgroup Debug Debug Monitor
*
* Set Callback function to monitor receive/send data.
*
* 设置回调函数监控发送接收数据.
* 
* @{ 
*/
/*******************************************************************************/
/**
 * Set Monitor Data Call Back function of the specified serial port.
 *
 * @param[in] portfd the serial port file description. Must not be <= 0.
 * @param[in] readcb the call back function(>= NULL) of monitoring serial port read data.
 *            if NULL, disable monitoring data
 * @param[in] writecb the call back function(>= NULL) of monitorint serial port write data.
 *            if NULL, disable monitoring data
 * 
 * @return none
 *
 * @since 0.1
 */

/**
 * 设置指定串口发送接收数据回调函数，监控数据.
 *
 * @param[in] portfd 串口通讯描述符. 不能 <= 0.
 * @param[in] readcb 读串口数据监控回调(>= NULL)，如果为NULL, 不监控
 * @param[in] writecb 写串口数据监控回调(>= NULL)，如果为NULL, 不监控
 *
 * @return 无
 *
 * @since 0.1
 */
void SetMonitorPort(int portfd, monitorcallback readcb, monitorcallback writecb);

/**
* Enable write log to file and error stream.
* The log file record folder is not includeing the file name, 
* the file name is the library name which append year/month/day. like wbcs10_20200328.log
*
* @param[in] level log level, see the value below.
* @param[in] logdir log file record folder, if NULL, the log folder default to subdirectory "log" of the app which invoke this library.
* 
* level value:
* | value	 | 	meaning	    | 
* | ------   |------------- | 
* | 0	|  disable log  |	
* | 1	|  enable log at MESSAGE level  |
* | 2	|  enable log at DEBUG level  |	
* | 3	|  enable log at WARNING level  |
* | 4	|  enable log at ERROR level, record request and response data |	
*
* @return true, success, false, failure.
*
* @since 0.9
*/

/**
* 开启日志, 写入文件和显示在错误输出流中.
*
* 日志记录路径, 路径不包括文件名，文件名和动态库名字相同, 后面加上年月日，例如 wbcs10_20200328.log.
*
* @param[in] level 日志等级, 具体见下面.
* @param[in] logdir 日志记录路径, 传入NULL 则记录日志在调用库程序目录下的"log"目录中.
*
* level 值:
* | 值	 | 	含义	    | 
* | ------   |------------- 	| 
* | 0	|  禁止显示写日志  		    |	
* | 1	|  允许记录信息等级日志 	|
* | 2	|  允许记录调试等级日志 	|	
* | 3	|  允许记录警告等级日志  	|
* | 4	|  允许记录错误等级日志, 记录发送接收数据 |	
*
* @return true, 开启成功, false, 开启失败.
*
* @since 0.9
*/
bool EnableLog(int level, const char *logdir);

/**
* Disable log.
*
* @return None.
*
* @since 0.9
*/

/**
* 禁止日志.
*
* @return 无.
*
* @since 0.9
*/
void DisableLog();
/*******************************************************************************/
/** @} */
/** 
* @defgroup Ports Port handling
*
* Opening, closing and querying ports.
*
* 打开, 关闭指定串口, 查询串口信息.
* 
* @{ 
*/
/*******************************************************************************/
/**
 *  Open the specified device port.
 *   
 *  @param[in] devpath pointer to a port path(serial port: "/dev/ttyS1", USB HID: "/dev/hid"), 
 *  if NULL, automatic open the port on system (devparam: serial port: 9600, USB HID: 0) 
 *  and check whether or not the specified device.
 *  @param[in] devparam port parameter to use when opening the device port.
 *  if device port is serial port, meaning the baudrate of port, 9600, 14400, 19200, 38400, 57600, 115200, default is 9600.
 *  if device port is USB HID, meaning index of device, 0-N, defualt is 0.
 *
 *  @return  > 0 open success, the port file descriptor, <= 0 meaning open failure.
 *
 *@since 0.1
 */

/**
 *  打开指定设备路径.
 *
 *  @param[in] devpath 设备路径(串口: "/dev/ttyS1", USB HID: "/dev/hid"), 
 *  如果为NULL，自动搜索系统上的端口(端口参数: 串口: 9600, USB HID: 0)， 打开并检查是否是相应设备.
 *  @param[in] devparam 打开设备端口参数.
 *  如果打开端口为串口，串口波特率, 默认波特率为9600，波特率值: 9600, 19200, 38400, 57600, 115200.
 *  如果打开端口为USB HID，设备端口索引，默认0.
 *      
 *  @return  > 0 打开成功, 端口设备文件描述符, <= 0 打开失败.
 *        
 *@since 0.1
 */
int OpenPort(const char *devpath, int devparam);

/**
 * Close the specified device port.
 *
 * @param[in] portfd the device port file description. Must not be <= 0.
 *
 * @return true upon success, false meaning a close failure.
 *
 * @since 0.1
 */

/**
 * 关闭指定设备端口.
 * 
 * @param[in] portfd 设备通讯描述符. 不能 <= 0.
 *
 * @return true 关闭成功, false 关闭失败.
 *
 * @since 0.1
 */
bool ClosePort(int portfd);

/**
* Cancel the executing command.
*
* @param[in] portfd the device port file description. Must not be <= 0.
* @param[in] iCancel whether or not cancel(> 0 : cancel  <= 0 don't cancel).
*
* @return none.
*
* @since 0.1
*/

/**
* 取消正在执行操作.
*
* @param[in] portfd 设备通讯描述符. 不能 <= 0.
* @param[in] iCancel 是否取消(> 0 : 取消操作  <= 0 不取消操作).
*
* @return 无.
*
* @since 0.1
*/
void CancelPort(int portfd, int iCancel);

/**
 * Get device port path name.
 *
 * @param[in] portfd the device port file description. Must not be <= 0.
 *
 * @return > 0 Device path name, like "/dev/ttyS1", NULL failed，not open the device port.
 *
 * @since 0.1
 */

/**
 * 查询指定设备路径名字.
 *
 * @param[in] portfd 设备通讯描述符. 不能 <= 0.
 *
 * @return > 0 返回设备路径名称, 如 "/dev/ttyS1", NULL 失败，没有连接设备.
 *
 * @since 0.1
 */
const char* GetPortName(int portfd);
/** @} */
/*******************************************************************************/
/**
* @defgroup Device Device Control
*
*  Get device firmware version and get/set device setting functions.
*
* @{
*/

/**
*  Reset device to get device version information.
*  Reset device configuration to default.
*  1) Cancel swiping card state or disable swiping card.
*  2) Turn off LED 
*  3) Enable track 2, 3
*  4) if device is serial port, the baud rate of device is change to 9600 BPS.
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[out] strVersion store the device version info in ASCII format which at least allocating 64 byte.
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/

/**
 * 复位设备，获取设备固件版本.
 * 复位设备为默认配置
 * 1) 取消刷卡，禁止刷卡
 * 2) 关闭LED
 * 3) 允许读取　2, 3 磁道
 * 4) 如果设备为串口接口，设备波特率修改为 9600 BPS.
 *
 * @param[in] portfd 设备通讯描述符. 不能 <= 0.
 * @param[out] ver 版本信息(最少64字节).
 *
 * @return = 0 success, < 0 failure.
 *
 * @since 0.1
 */
int Reset(int portfd, char *strVersion);

/**
*  Get status and confiuration of device.
*
 * @param[in] portfd the device port file description. Must not be <= 0.
*  @param[out] pFormat the format of data(0: ASCII, 1: Binary).
*  @param[out] pTrack the valid read track ID, see the value below.
*  @param[out] pEable whether or not data valid when swiping the card(0: swiping card invalid, 1: swiping card valid).
* 
* pTrack value:
* | value	 | 	meaning	    | 
* | ------   |------------- | 
* | 0	|  read none track  |	
* | 1	| read track 1  |
* | 2	| read track 2  |	
* | 3	|  read track 3   |
* | 4	|  read track 1 2 |	
* | 5	|  read track 2 3 |
* | 6	|  read track 1 3 |	
* | 7	|  read track 1 2 3 |
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/

/**
*  获取设备状态配置信息.
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[out] pFormat 数据编码格式(0: ASCII, 1: 二进制).
*  @param[out] pTrack 有效读取磁道号, 详情见下面.
*  @param[out] pEnable 设备是否处于刷卡有效状态(0: 刷卡无效状态, 1: 刷卡有效状态).
* 
* pTrack 值:
* | 值	  | 	含义	      | 
* | ------   |------------- | 
* | 0	|  磁卡三轨都不读  |	
* | 1	| 读磁卡一轨   |
* | 2	|   读磁卡二轨   |	
* | 3	|  读磁卡三轨    |
* | 4	|   读磁卡一二轨   |	
* | 5	|  读磁卡二三轨   |
* | 6	|  读磁卡一三轨   |	
* | 7	|  读磁卡一二三轨    |
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.1
*/
int Status(int portfd, unsigned char *pFormat, unsigned char *pTrack, unsigned char *pEnable);

/**
*  Set configuration of device.
*
 * @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] btFormat the format of data(0: ASCII, 1: Binary).
*  @param[in] btTrack the valid read track ID, see the value below.
* 
* btTrack value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 
* | 0	|  read none track  |	
* | 1	|	read track 1  |
* | 2	|	read track 2  |	
* | 3	|  read track 3   |
* | 4	|  read track 1 2 |	
* | 5	|  read track 2 3 |
* | 6	|  read track 1 3 |	
* | 7	|  read track 1 2 3 |
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/

/**
*  设置设备配置.
*
* @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in] btFormat 数据编码格式(0: ASCII, 1: 二进制).
*  @param[in] btTrack 有效读取磁道号, 详情见下面.
* 
* btTrack 值:
* | 值	  | 	含义	      | 
* | ------   |------------- | 
* | 0	|  磁卡三轨都不读  |	
* | 1	| 读磁卡一轨   |
* | 2	|   读磁卡二轨   |	
* | 3	|  读磁卡三轨    |
* | 4	|   读磁卡一二轨   |	
* | 5	|  读磁卡二三轨   |
* | 6	|  读磁卡一三轨   |	
* | 7	|  读磁卡一二三轨    |
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.1
*/
int Setup(int portfd, unsigned char btFormat, unsigned char btTrack);
		
/**
*  Disable swiping card to make swiping card invalid.
*
* @param[in] portfd the device port file description. Must not be <= 0.
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/

/**
*  禁止刷卡，刷卡无效.
*
* @param[in] portfd 设备通讯描述符. 不能 <= 0.
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.1
*/
int Disable(int portfd);	

/**
*  Set the communication baud rate of device.
*  After execute this command, the baudrate of device will change and
*  after power down and Reset command, this baudrate of device will change back to 9600 BPS.
*
* @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] iBaudRate the baud rate which to set, see the value below.
* 	<br>
*  the baud rate value:
*  2400, 4800, 9600, 19200, 38400, 57600, 115200
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.1
*/

/**
*  设置串口设备通讯波特率.
*  该命令被执行成功之后，卡机波特率将被改变，掉电或执行复位操作将恢复默认值(9600 BPS)
* 
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in] iBaudRate 设置波特率, 具体值见下面.
*  <br>
* 	波特率 值:
*  2400, 4800, 9600, 19200, 38400, 57600, 115200
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.1
*/
int SetBPS(int portfd, int iBaudRate);

/**
*  Set mode of USB keyboard and whether or not add track tail char "carriage return".
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] btKB whether or not enable USB keyboard(0: disable, 1: enable(default)).
*  @param[in] btCR whether or add carriage return character to the end of track data(0: not add, 1: add(default)).
*
*  @return  reference to the @ref APIReturn.
*
* @since 0.9
*/

/**
* 设置USB键盘模式和磁道数据结尾是否添加回车符'CR'.
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in] btKB 是否使能USB 键盘模式(0: 不允许, 1: 允许(默认)).
*  @param[in] btCR 是否磁道数据结尾添加回车符'CR'(0: 不添加, 1: 添加(默认)).
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.1
*/
int SetUSBKB(int portfd, unsigned char btKB, unsigned char btCR);

/**
*  Set mode of device LED.
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] btMode whether or not in default mode, see the value below.
* 
* btMode value :
* | value	 |  meaning	    | 
* | ------   |------------- | 
* | 0	|  not in default mode, LED will control by command |	
* | 1	|  default mode, LED will turn on or blink when swiping card |
*
*   @return  reference to the @ref APIReturn.
*
*@since 0.9
*/

/**
*  设置LED模式.
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in] btMode 是否默认模式, 具体见下面.
* 
* btMode 值:
* | value	 |  含义	    | 
* | ------   |------------- | 
* | 0	|  非默认模式, LED由操作命令控制 |	
* | 1	|  默认模式,  刷卡时LED闪烁|
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.9
*/
int SetLEDMode(int portfd, unsigned char btMode);

/**
*  Set the LED of device.
*   
*  @param[in] portfd portfd the device port file description. Must not be <= 0.
*  @param[in] bOnTime LED on time(0-255, unit time is 10ms).
*  @param[in] bNum LED blink number, see the value below.
*
* bNum value:
* | value	| 	meaning	     | 
* | ------  |  ------------- |
* | 0	    |  turn LED off|	
* | 1 - 254	|  the number of blinking|
* | 255	    |  always blinking |
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.9
*/

/**
*  控制设备LED.
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in] bOnTime LED亮时间(0-255, 单位时间为10ms).
*  @param[in] bNum 闪烁次数, 具体见下面.
*
* bNum 值:
* | 值	| 	含义	     | 
* | ------  |  ------------- | 
* | 0	    |  关闭LED|	
* | 1 - 254	|  闪烁次数|
* | 255	    |  一直闪烁 |
*
* @return  详情请见 @ref APIReturn.
*        
*@since 0.9
*/
int SetDeviceLED(int portfd, unsigned char bOnTime, unsigned char bNum);

/**
*  Set mode of device buzzer.
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] btMode whether or not in default mode, see the value below.
* 
* btMode value :
* | value	 |  meaning	    | 
* | ------   |------------- | 
* | 0	|  not in default mode, buzzer will control by command |	
* | 1	|  default mode, buzzer will turn on when swiping card |
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.9
*/

/**
*  设置蜂鸣器模式.
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in] btMode 是否默认模式, 具体见下面.
* 
* btMode 值:
* | 值	 |  含义	    | 
* | ------   |------------- | 
* | 0	|  非模式模式, 蜂鸣器由命令控制 |	
* | 1	|  默认模式, 蜂鸣器响在刷卡完成时 |
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.9
*/
int SetBuzzerMode(int portfd, unsigned char btMode);

/**
*  Set the buzzer of device.
*   
*  @param[in] portfd portfd the device port file description. Must not be <= 0.
*  @param[in] bOnTime Buzzer on time(0-255, unit time is 100ms).
*  @param[in] bNum Buzzer on number, see the value below.
*
* bNum value:
* | value	| 	meaning	     | 
* | ------  |  ------------- | 
* | 0	    |  turn the buzzer off|	
* | 1 - 255	|  the number of buzzer|
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.9
*/

/**
*  控制设备蜂鸣器 Buzzer.
*   
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in] bOnTime 响时间(0-255, 单位时间为100ms).
*  @param[in] bNum 响次数, 具体见下面.
*
* bNum 值:
* | 值	| 	含义	     | 
* | ------  |  ------------- | 
* | 0	    |  关闭蜂鸣器|	
* | 1 - 255	|  响次数|
*
*  @return  详情请见 @ref APIReturn.
*        
*@since 0.9
*/
int SetDeviceBuzzer(int portfd, unsigned char bOnTime, unsigned char bNum);

/**
*  Set operation mode of reading magnetic card.
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] btMode operation mode, see the value below.
*
* btMode value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 0	|  auto uploading data mode, after swiping the card, device will automatically uploading data|
* | 1	|  command mode (passive mode), should send command of enable swiping/read card first, then swiping card, device will automatically uploading data|
* | 2	|  cache mode(passive mode), should send command of enable swiping/read card first, then swiping card, the data cached in device, must send command of get track data to get data|		
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.9
*/

/**
*  设置读取磁卡操作模式.
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in] btMode 操作模式, 具体见下面.
*
* btMode 值:
* | 值	  | 	含义	     | 
* | ------   |------------- | 	
* | 0	|  自动上传模式, 刷卡后, 设备将自动上传数据|
* | 1	|  指令模式(被动模式), 需要发送命令允许 刷/读 卡命令允许刷卡, 然后刷卡， 刷卡完成设备将自动上传数据!
* | 2	|  缓存模式(被动模式), 需要发送命令允许 刷/读 卡命令允许刷卡, 然后刷卡， 刷卡完成磁道数据缓存在设备中，需要发送获取磁道信息指令获取磁道数据!	
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.9
*/
int SetDeviceCtrlMode(int portfd, unsigned char btMode);

/**
*  Get operation mode of reading magnetic card.
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[out] btMode operation mode, see the value below.
*
* btMode value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 0	|  auto uploading data mode, after swiping the card, device will automatically uploading data|
* | 1	|  command mode(passive mode), should send command of enable swiping/read card first, then swiping card, device will automatically uploading data|
* | 2	|  cache mode(passive mode), should send command of enable swiping/read card first, then swiping card, the data cached in device, must send command of get track data to get data|	
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.9
*/

/**
*  获取读取磁卡操作模式.
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[out] btMode 操作模式, 具体见下面.
*
* btMode 值:
* | 值	  | 	含义	      | 
* | ------   |------------- | 	
* | 0	|  自动上传模式, 刷卡后, 设备将自动上传数据|
* | 1	|  指令模式(被动模式), 需要发送命令允许 刷/读 卡命令允许刷卡, 然后刷卡，刷卡完成设备将自动上传数据!
* | 2	|  缓存模式(被动模式), 需要发送命令允许 刷/读 卡命令允许刷卡, 然后刷卡， 刷卡完成磁道数据缓存在设备中，需要发送获取磁道信息指令获取磁道数据!	
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.9
*/
int GetDeviceCtrlMode(int portfd, unsigned char *btMode);
/** @} */
/*******************************************************************************/
/** 
* @defgroup Magnetic read magnetic card track data
*
* Device magnetic card operation.
*
* 设备磁道操作.
* 
* @{ 
*/
/*******************************************************************************/
/**
*  Enable swiping card, and wait to read magnetic track data in command mode(passive mode).
*
*  @param[in] portfd the port file description. Must not be <= 0.
*  @param[in] bFaildContinued whether or not still could swiping card after failure of swiping.
*  @param[out] pFormat the format of data(0: ASCII, 1: Binary).
*  @param[in, out] pTrack1  in: track 1 data buffer, at least 256 byte.<br>
* 							out: track 1 data.
*  @param[in, out] pInOutLen1 in: size of track data buffer "pTrack1" <br>
* 							out: if "pInOutLen1[0]" > 0, track data "pTrack1" ok and is the size of "pTrack1"<br>
*                                if "pInOutLen1[0]" < 0, track data "pTrack1" error and "pTrack1[0]" error code, reference to @ref TrackErrorCode.
*  @param[in, out] pTrack2 	in: track 2 data buffer, at least 256 byte.<br>
* 							out: track 2 data.
*  @param[in, out] pInOutLen2 in: size of track data buffer "pTrack2" <br>
* 							out: if "pInOutLen2[0]" > 0, track data "pTrack2" ok and is the size of "pTrack2"<br>
*                                if "pInOutLen2[0]" < 0, track data "pTrack2" error and "pTrack2[0]" error code, reference to @ref TrackErrorCode.
*  @param[in, out] pTrack3 	in: track 3 data buffer, at least 256 byte.<br>
* 							out: track 3 data.
*  @param[in, out] pInOutLen3 in: size of track data buffer "pTrack3" <br>
* 							 out: if "pInOutLen3[0]" > 0, track data "pTrack3" ok and is the size of "pTrack3"<br>
*                                if "pInOutLen3[0]" < 0, track data "pTrack3" error and "pTrack3[0]" error code, reference to @ref TrackErrorCode.
*  @param[in] iTimeout  wait for swiping card, read track data waiting timeout(ms). if < 0, return until read data, = 0, return immediately,
* 							> 0, return after waitting at most "iTimeout" ms.
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.1
*/

/**
*  允许刷磁卡，等待刷卡读取磁道数据, 命令模式下使用(被动模式).
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in] bFaildContinued 是否刷卡失败后能否继续刷卡.
*  @param[out] pFormat  数据编码格式(0: ASCII, 1: 二进制).
*  @param[in, out] pTrack1 	输入: 磁道1数据缓冲, 最少256字节<br>
* 							输出: 磁道1数据.
*  @param[in, out] pInOutLen1 输入: 磁道缓冲"pTrack1" 大小<br>
* 							输出: 如果"pInOutLen1[0]" > 0, 磁道数据"pTrack1"正常，且为磁道数据"pTrack1"大小，<br>
*                                如果"pInOutLen1[0]" < 0, 磁卡数据"pTrack1"出错, 且"pTrack1[0]" 为错误码，详情见@ref TrackErrorCode.
*  @param[in, out] pTrack2 	输入:  磁道2数据缓冲, 最少256字节<br>
* 							输出: 磁道2数据.
*  @param[in, out] pInOutLen2 输入: 磁道缓冲"pTrack2" 大小<br>
* 							输出: 如果"pInOutLen2[0]" > 0, 磁道数据"pTrack2"正常，且为磁道数据"pTrack2"大小，<br>
*                                如果"pInOutLen2[0]" < 0, 磁卡数据"pTrack2"出错, 且"pTrack2[0]" 为错误码，详情见@ref TrackErrorCode.
*  @param[in, out] pTrack3 	输入:  磁道3数据缓冲, 最少256字节<br>
* 							输出: 磁道3数据.
*  @param[in, out] pInOutLen3 输入: 磁道缓冲"pTrack3" 大小<br>
* 							输出: 如果"pInOutLen3[0]" > 0, 磁道数据"pTrack3"正常，且为磁道数据"pTrack3"大小，<br>
*                                如果"pInOutLen3[0]" < 0, 磁卡数据"pTrack3"出错, 且"pTrack3[0]" 为错误码，详情见@ref TrackErrorCode.
* @param[in] iTimeout  等待刷卡, 读取设备返回磁道数据超时(毫秒). < 0, 如果没有数据, 等待直到获取数据, = 0 如果没有数据, 立即返回, > 0, 最多等待 "iTimeout" ms.
*
*  @return  详情请见 @ref APIReturn.
*        
*@since 0.1
*/
int Enable(int portfd, bool bFaildContinued, unsigned char *pFormat, unsigned char *pTrack1, int *pInOutLen1, 
		unsigned char *pTrack2, int *pInOutLen2, unsigned char *pTrack3, int *pInOutLen3, int iTimeout);
			
/**
*  Resend last valid magnetic track data in command mode(passive mode).
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[out] pFormat the format of data(0: ASCII, 1: Binary).
*  @param[in, out] pTrack1  in: track 1 data buffer, at least 256 byte.<br>
* 							out: track 1 data.
*  @param[in, out] pInOutLen1 in: size of track data buffer "pTrack1" <br>
* 							out: if "pInOutLen1[0]" > 0, track data "pTrack1" ok and is the size of "pTrack1"<br>
*                                if "pInOutLen1[0]" < 0, track data "pTrack1" error and "pTrack1[0]" error code, reference to @ref TrackErrorCode.
*  @param[in, out] pTrack2 	in: track 2 data buffer, at least 256 byte.<br>
* 							out: track 2 data.
*  @param[in, out] pInOutLen2 in: size of track data buffer "pTrack2" <br>
* 							out: if "pInOutLen2[0]" > 0, track data "pTrack2" ok and is the size of "pTrack2"<br>
*                                if "pInOutLen2[0]" < 0, track data "pTrack2" error and "pTrack2[0]" error code, reference to @ref TrackErrorCode.
*  @param[in, out] pTrack3 	in: track 3 data buffer, at least 256 byte.<br>
* 							out: track 3 data.
*  @param[in, out] pInOutLen3 in: size of track data buffer "pTrack3" <br>
* 							 out: if "pInOutLen3[0]" > 0, track data "pTrack3" ok and is the size of "pTrack3"<br>
*                                if "pInOutLen3[0]" < 0, track data "pTrack3" error and "pTrack3[0]" error code, reference to @ref TrackErrorCode.
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.1
*/

/**
*  重新读取上次刷卡有效数据, 指令模式下使用(被动模式).
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[out] pFormat  数据编码格式(0: ASCII, 1: 二进制).
*  @param[in, out] pTrack1 	输入:  磁道1数据缓冲, 最少256字节<br>
* 							输出: 磁道1数据.
*  @param[in, out] pInOutLen1 输入: 磁道缓冲"pTrack1" 大小<br>
* 							输出: 如果"pInOutLen1[0]" > 0, 磁道数据"pTrack1"正常，且为磁道数据"pTrack1"大小，<br>
*                                如果"pInOutLen1[0]" < 0, 磁卡数据"pTrack1"出错, 且"pTrack1[0]" 为错误码，详情见@ref TrackErrorCode.
*  @param[in, out] pTrack2 	输入:  磁道2数据缓冲, 最少256字节<br>
* 							输出: 磁道2数据.
*  @param[in, out] pInOutLen2 输入: 磁道缓冲"pTrack2" 大小<br>
* 							输出: 如果"pInOutLen2[0]" > 0, 磁道数据"pTrack2"正常，且为磁道数据"pTrack2"大小，<br>
*                                如果"pInOutLen2[0]" < 0, 磁卡数据"pTrack2"出错, 且"pTrack2[0]" 为错误码，详情见@ref TrackErrorCode.
*  @param[in, out] pTrack3 	输入:  磁道3数据缓冲, 最少256字节<br>
* 							输出: 磁道3数据.
*  @param[in, out] pInOutLen3 输入: 磁道缓冲"pTrack3" 大小<br>
* 							输出: 如果"pInOutLen3[0]" > 0, 磁道数据"pTrack3"正常，且为磁道数据"pTrack3"大小，<br>
*                                如果"pInOutLen3[0]" < 0, 磁卡数据"pTrack3"出错, 且"pTrack3[0]" 为错误码，详情见@ref TrackErrorCode.
*
*  @return  详情请见 @ref APIReturn.
*        
*@since 0.1
*/
int Resend(int portfd, unsigned char *pFormat,  unsigned char *pTrack1, int *pInOutLen1,  
			unsigned char *pTrack2, int *pInOutLen2, unsigned char *pTrack3, int *pInOutLen3);

/**
*  Enable swiping/read card in cache mode(passive mode).
* 
*  @param[in] portfd the device port file description. Must not be <= 0.
*   
*  @return  reference to the @ref APIReturn.
*
*@since 0.9
*/

/**
*  允许刷磁卡, 缓存模式下使用(被动模式).
* 
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  
*  @return  详情请见 @ref APIReturn.
*        
*@since 0.9
*/
int EnableSwipingCard(int portfd);

/**
*  Get magnetic track data in cache mode(passive mode).
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in, out] pTrack1  in: track 1 data buffer, at least 256 byte.<br>
* 							out: track 1 data.
*  @param[in, out] pInOutLen1 in: size of track data buffer "pTrack1" <br>
* 							out: if "pInOutLen1[0]" > 0, track data "pTrack1" ok and is the size of "pTrack1"<br>
*                                if "pInOutLen1[0]" < 0, track data "pTrack1" error and "pTrack1[0]" error code, reference to @ref TrackErrorCode.
*  @param[in, out] pTrack2 	in: track 2 data buffer, at least 256 byte.<br>
* 							out: track 2 data.
*  @param[in, out] pInOutLen2 in: size of track data buffer "pTrack2" <br>
* 							out: if "pInOutLen2[0]" > 0, track data "pTrack2" ok and is the size of "pTrack2"<br>
*                                if "pInOutLen2[0]" < 0, track data "pTrack2" error and "pTrack2[0]" error code, reference to @ref TrackErrorCode.
*  @param[in, out] pTrack3 	in: track 3 data buffer, at least 256 byte.<br>
* 							out: track 3 data.
*  @param[in, out] pInOutLen3 in: size of track data buffer "pTrack3" <br>
* 							 out: if "pInOutLen3[0]" > 0, track data "pTrack3" ok and is the size of "pTrack3"<br>
*                                if "pInOutLen3[0]" < 0, track data "pTrack3" error and "pTrack3[0]" error code, reference to @ref TrackErrorCode.
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.9
*/

/**
*  获取磁道数据，缓存模式下使用(被动模式).
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in, out] pTrack1  输入: 轨道 1 数据 缓冲, 至少分配256字节.<br>
* 							输出: 轨道 1 数据.
*  @param[in, out] pInOutLen1 输入: 磁道缓冲"pTrack1". 长度<br>
* 							输出: 如果 "pInOutLen1[0]" > 0, 磁道数据 "pTrack1" 成功并且是磁道数据"pTrack1" 长度.<br>
*                                如果 "pInOutLen1[0]" < 0, 磁道数据 "pTrack1" 错误并且 "pTrack1[0]" 为错误码, 具体请参考 @ref TrackErrorCode.
*  @param[in, out] pTrack2  输入: 轨道 2 数据 缓冲, 至少分配256字节.<br>
* 							输出: 轨道 2 数据.
*  @param[in, out] pInOutLen2 输入: 磁道缓冲"pTrack2" 长度.<br>
* 							输出: 如果 "pInOutLen2[0]" > 0, 磁道数据 "pTrack2" 成功并且是磁道数据"pTrack2" 长度.<br>
*                                如果 "pInOutLen2[0]" < 0, 磁道数据 "pTrack2" 错误并且 "pTrack2[0]" 为错误码, 具体请参考 @ref TrackErrorCode.
*  @param[in, out] pTrack3  输入: 轨道 3 数据 缓冲, 至少分配256字节.<br>
* 							输出: 轨道 3 数据.
*  @param[in, out] pInOutLen3 输入: 磁道缓冲"pTrack3" 长度.<br>
* 							输出: 如果 "pInOutLen3[0]" > 0, 磁道数据 "pTrack3" 成功并且是磁道数据"pTrack3" 长度.<br>
*                                如果 "pInOutLen3[0]" < 0, 磁道数据 "pTrack3" 错误并且 "pTrack3[0]" 为错误码, 具体请参考 @ref TrackErrorCode.
*
*  @return  详情请见 @ref APIReturn.
*        
*@since 0.9
*/
int GetTrackCache(int portfd,  unsigned char *pTrack1, int *pInOutLen1, 
	 unsigned char *pTrack2, int *pInOutLen2, unsigned char *pTrack3, int *pInOutLen3);

/**
*  Clear track buffer which store the magnetic card track data in cache mode(passive mode).
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.9
*/

/**
*  清除磁道数据缓存, 缓存模式下使用(被动模式).
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*
*  @return  详情请见 @ref APIReturn.
*
*@since 0.9
*/
int ClearTrackCache(int portfd);

/**
*  Read magnetic track data when in auto uploading data mode.
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in, out] pTrack1  in: track 1 data buffer, at least 256 byte.<br>
* 							out: track 1 data.
*  @param[in, out] pInOutLen1 in: size of track data buffer "pTrack1" <br>
* 							out: if "pInOutLen1[0]" > 0, track data "pTrack1" ok and is the size of "pTrack1"<br>
*                                if "pInOutLen1[0]" <= 0, track data "pTrack1" error.
*  @param[in, out] pTrack2 	in: track 2 data buffer, at least 256 byte.<br>
* 							out: track 2 data.
*  @param[in, out] pInOutLen2 in: size of track data buffer "pTrack2" <br>
* 							out: if "pInOutLen2[0]" > 0, track data "pTrack2" ok and is the size of "pTrack2"<br>
*                                if "pInOutLen2[0]" <= 0, track data "pTrack2" error.
*  @param[in, out] pTrack3 	in: track 3 data buffer, at least 256 byte.<br>
* 							out: track 3 data.
*  @param[in, out] pInOutLen3 in: size of track data buffer "pTrack3" <br>
* 							 out: if "pInOutLen3[0]" > 0, track data "pTrack3" ok and is the size of "pTrack3"<br>
*                                if "pInOutLen3[0]" <= 0, track data "pTrack3" error.
*  @param[in] iTimeout waiting timeout(ms) of getting response. if < 0, return until read data, = 0, return immediately,
* 							> 0, return after waitting at most "iTimeout" ms.
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.9
*/

/**
*  等待刷卡, 读取磁道信息, 主动上传模式下使用(主动模式).
*
*  @param[in] portfd 设备通讯描述符. 不能 <= 0.
*  @param[in, out] pTrack1  输入: 轨道 1 数据 缓冲, 至少分配256字节.<br>
* 							输出: 轨道 1 数据.
*  @param[in, out] pInOutLen1 输入: 磁道缓冲"pTrack1". 长度<br>
* 							输出: 如果 "pInOutLen1[0]" > 0, 磁道数据 "pTrack1" 成功并且是磁道数据"pTrack1" 长度.<br>
*                                如果 "pInOutLen1[0]" <= 0, 磁道数据 "pTrack1" 错误.
*  @param[in, out] pTrack2  输入: 轨道 2 数据 缓冲, 至少分配256字节.<br>
* 							输出: 轨道 2 数据.
*  @param[in, out] pInOutLen2 输入: 磁道缓冲"pTrack2" 长度.<br>
* 							输出: 如果 "pInOutLen2[0]" > 0, 磁道数据 "pTrack2" 成功并且是磁道数据"pTrack2" 长度.<br>
*                                如果 "pInOutLen2[0]" < 0, 磁道数据 "pTrack2" 错误.
*  @param[in, out] pTrack3  输入: 轨道 3 数据 缓冲, 至少分配256字节.<br>
* 							输出: 轨道 3 数据.
*  @param[in, out] pInOutLen3 输入: 磁道缓冲"pTrack3" 长度.<br>
* 							输出: 如果 "pInOutLen3[0]" > 0, 磁道数据 "pTrack3" 成功并且是磁道数据"pTrack3" 长度.<br>
*                                如果 "pInOutLen3[0]" < 0, 磁道数据 "pTrack3" 错误.
*  @param[in] iTimeout  等待刷卡, 读取刷卡数据超时(毫秒 ms). 如果 < 0, 直到刷卡读到数据才返回, = 0, 立即返回,
* 							> 0, 等待"iTimeout" 毫秒，读取刷卡数据.
*
*  @note 磁道数据格式，具体请见通讯协议.
*
*  @return  详情请见 @ref APIReturn.
*        
*@since 0.9
*/
int ReadTrackAuto(int portfd, unsigned char *pTrack1, int *pInOutLen1, 
	unsigned char *pTrack2, int *pInOutLen2, unsigned char *pTrack3, int *pInOutLen3, 
	int iTimeout);
/** @} */
/*******************************************************************************/
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_WB_CS10_H_
