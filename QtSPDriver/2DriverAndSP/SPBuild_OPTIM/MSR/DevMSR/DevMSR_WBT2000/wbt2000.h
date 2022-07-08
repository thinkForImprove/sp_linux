
/**
 * @mainpage WBT2000 API
 *
 * Introduction
 * ============
 *
 * WBT2000 is a device control library written in C/C++ that is intended to take
 * care of the linux like system when writing software that uses the WBT2000 device.
 *
* Applicable device version
* ===============
*  This library could operate these device version, if some function fail, maybe the device 
*  don't have this function. 
*
* Universal version
* -------
* RS232 serial port, have read/write track function, some foreign customer have erase track function！
*
* ZD version
* -------
* USB to serial, only have read track function.
*
* GDYT version
* -------
* USB to serial, only have read track function, add buzzer mode function.
*
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
 * To use WBT2000 functions in your code, you should include the
 * wbt2000.h header, i.e. "#include <wbt2000.h>".
 *
 * Functions
 * ---------
 *
 * The functions provided by the library are documented in detail in
 * the following sections:
 *
 * - @ref LibVersion (get library version)
 * - @ref Debug (Monitor data and log function)
 * - @ref Enumeration (obtaining a list of serial ports on the system)
 * - @ref Ports (opening, closing and getting information about ports)
 * - @ref Device (getting or setting device like get device firmware version)
 * - @ref Magnetic (read and write magnetic card track data)
 * 
 * Data structures
 * ---------------
 *
 * The library defines one data structures:
 *
 * - @ref port_name, which represents a serial port path.
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

#ifndef _WBT2000_H_
#define _WBT2000_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

/*******************************************************************************/
/**
 * Macro preceding public API functions
 */
#define  LIB_API __attribute__((visibility("default")))

/**
 * Macro preceding private functions
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
struct port_name
{
	char path[NAME_SIZE];	///< device path(/dev/ttyS0)    
};

/**
 * @struct hid_port
 * struct for usbh hid port information.
 */
struct hid_port
{
	char 			path[NAME_SIZE];		///< device path      			
	unsigned short 	vendor_id; 				///< device vender id      	
	unsigned short 	product_id; 			///< device product id      	
	int 			interface_number;		///< device interface number 
};

/******************************************************************************/
/**
 * The default device baudrate.
 */
#define LIB_DEFALUT_BAUDRATE		9600
/******************************************************************************/

/**
 * monitor data call back function.
 */
typedef void (* monitorcallback)(const void *pdata, int datalen);
/******************************************************************************/

/**
 * API return value.
 */
typedef enum APIRETURN
{
	APICMDInProgress 	= -11, 	///< command in progress       
	APICMDCancel		= -10,	///< command cancel 				
	APINotSupportCMD	= -9,	///< not support command	       
	APIUnknownError		= -8,	///< unknown error 				
	APIFrameError		= -7,	///< data not conform to protocol 	
	APIReadTimeOut		= -6,	///< read data timeout				
	APIReadError		= -5,	///< read data error				
	APIWriteTimeOut		= -4,	///< write data timeout				
	APIWriteError		= -3,	///< write data error				
	APICMDParmError		= -2,	///< API parameter error		
	APICMDFailure		= -1,	///< command execute error		
	APICMDSuccess		= 0,	///< command execute ok		
} APIReturn;

/******************************************************************************/
/** 
* @defgroup LibVersion Library versions
* 
*  Version number string querying functions
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
 * "/dev/ttyS1\0/dev/ttyUSB0\0\0"
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
int ListPort(struct port_name *patharray, int arlen);
/*******************************************************************************/
/** @} */

/** 
* @defgroup Debug Debug monitor
*
* Set Callback function to monitor receive/send data.
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

void SetMonitorPort(int portfd, monitorcallback readcb, monitorcallback writecb);

/**
 * Enable log function
 * 
 * @param[in] iLogLevel log level.
 * level value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 0	| NONE  |
* | 1	| MESSAGE  |	
* | 2	|  DEBUG   |
* | 3	|  WARNING |	
* | 4	|  ERROR |
*
* @return none
* 
*/
void EnableLog(int iLogLevel);

/**
 * Get log level
 * 
 * level value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 0	| NONE  |
* | 1	| MESSAGE  |	
* | 2	|  DEBUG   |
* | 3	|  WARNING |	
* | 4	|  ERROR |
*
* @return log level 
* 
*/
int GetLogLevel();

/**
 * Disable log
*
* @return none
* 
*/
void DisableLog();
/*******************************************************************************/
/** @} */
/** 
* @defgroup Ports Port handling
*
* Opening, closing and querying ports.
* 
* @{ 
*/
/*******************************************************************************/
/**
 *  Open the specified device port.
 *   
 *  @param[in] devpath pointer to a port path(serial port/USB to serial: "/dev/ttyUSB0"), 
 *  if NULL, automatic open the port on system (devparam: serial port: 9600) 
 *  and check whether or not the specified device.
 *  @param[in] devparam port parameter to use when opening the device port.
 *  if device port is serial port, meaning the baudrate of port, 9600, 14400, 19200, 38400, 57600, 115200, default is 9600.
 *
 *  @return  > 0 open success, the port file descriptor, <= 0 meaning open failure.
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
void CancelPort(int portfd, int iCancel);

/**
 * Get device port path name.
 *
 * @param[in] portfd the device port file description. Must not be <= 0.
 *
 * @return > 0 Device path name，如 /dev/ttyUSB0, NULL failed，not open the device port.
 *
 * @since 0.1
 */
const char* GetPortName(int portfd);
/** @} */
/*******************************************************************************/
/**
* @defgroup Device Device control
*
*  Get device firmware version and get/set device setting functions.
*
* @{
*/
/**
*  Soft reset device to exit read/write/erase state.
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/
int Reset(int portfd);

/**
*  Get device firmware version. 
*   
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[out] strVer device firmware version(ASCII) buffer which at least 64 byte.
*      
*  @return  reference to the @ref APIReturn.
*        
*@since 0.1
*/
int GetFirmVer(int portfd, char *strVer);

/**
*  Set the mode of writing magnetic card track.
*  __Only universal version have this function__
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] type  type of writing card(0: Low coercivity magnetic stripe is being written  1: High coercivity magnetic stripe is being written).
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/
int SetHiLoCo(int portfd, unsigned char type);

/**
*  Set the mode of read magnetic card.
*  __Only universal version have this function__
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] type  type of read card. see the value below.
*
* type value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 0	| disable auto read track  |
* | 1	| enable auto read track 1  |
* | 2	| enable auto read track 2  |	
* | 3	| enable auto read track 3   |
* | 4	| enable auto read track 1 2 |	
* | 5	| enable auto read track 2 3 |
* | 6	| enable auto read track 1 3 |	
* | 7	| enable auto read track 1 2 3 |
* | 8	| enable auto read all track |
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/
int SetReadMGCardMode(int portfd, unsigned char type);

/**
*  Set the mode device buzzer.
*  default configuration is disable buzzer after swiping card. 
*  after power on again, this configuration can't be saved. 
*
*  __Only customized version(GDYT) have this function__
* 
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] type  type of buzzer. see the value below.
*
* type value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 0	| disable buzzer after swiping card |
* | 1	| enable buzzer after swiping card |
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/
int SetBuzzerMode(int portfd, unsigned char type);

/**
*  Get the mode device buzzer.
*  __Only customized version(GDYT) have this function__
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[out] type  type of buzzer. see the value below.
*
* type value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 0	| disable buzzer after swiping card |
* | 1	| enable buzzer  after swiping card |
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/
int GetBuzzerMode(int portfd, unsigned char *type);

/**
*  Control the number of buzzer which beeps.
*  __Only customized version(ZD, GDYT) have this function__
* 
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] num  number of buzzer(0-19).
*
*  @return  reference to the @ref APIReturn.
*
*@since 0.1
*/
int SetBuzzer(int portfd, unsigned char num);

/**
*  Set the communication baud rate of device.
*  After execute this command, the baudrate of device will change.
*
*  __Only customized version(ZD) have this function__
*  
* @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] iBaudRate the baud rate which to set, see the value below.
*
*  the baud rate value:
*  2400, 4800, 9600, 14400, 19200, 38400, 56000, 57600, 115200, 128000, 256000
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.1
*/
int SetBPS(int portfd, int iBaudRate);
/** @} */
/*******************************************************************************/
/** 
* @defgroup Magnetic Magnetic card operation
*
* Read/write magnetic card track data.
* 
* @{ 
*/
/*******************************************************************************/
/**
*  Sent command to read magnetic track data in ASCII format.
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] ucTrackID track id. see the value below.
*  @param[in, out] pTrack1  in: track 1 data buffer, at least 256 byte.<br>
* 							out: track 1 data(ASCII).
*  @param[in, out] pInOutLen1 in: size of track data buffer "pTrack1" <br>
* 							out: if "pInOutLen1[0]" > 0, track data "pTrack1" ok and is the size of "pTrack1"<br>
*                                if "pInOutLen1[0]" == 0, track data "pTrack1" error or blank track.
*  @param[in, out] pTrack2 	in: track 2 data buffer, at least 256 byte.<br>
* 							out: track 2 data(ASCII).
*  @param[in, out] pInOutLen2 in: size of track data buffer "pTrack2" <br>
* 							out: if "pInOutLen2[0]" > 0, track data "pTrack2" ok and is the size of "pTrack2"<br>
*                                if "pInOutLen2[0]" == 0, track data "pTrack2" error or blank track.
*  @param[in, out] pTrack3 	in: track 3 data buffer, at least 256 byte.<br>
* 							out: track 3 data(ASCII).
*  @param[in, out] pInOutLen3 in: size of track data buffer "pTrack3" <br>
* 							 out: if "pInOutLen3[0]" > 0, track data "pTrack3" ok and is the size of "pTrack3"<br>
*                                 if "pInOutLen3[0]" == 0, track data "pTrack3" error or blank track.
*  @param[in] iTimeOut  waiting timeout(ms) before read data. if < 0, return until read data, == 0, return immediately,
* 							> 0, return after waitting at most "iTimeOut" ms.
*
* ucTrackID value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 0	| not read track  |
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
int ReadMGCard(int portfd, unsigned char ucTrackID,
								    unsigned char *pTrack1,  int *pInOutLen1, 
								    unsigned char *pTrack2,  int *pInOutLen2,  
								    unsigned char *pTrack3,  int *pInOutLen3, 
								   int iTimeOut);	

/**
*  Auto read magnetic track data in ASCII format.
*  Must enable auto read track!
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in, out] pTrack1  in: track 1 data buffer, at least 256 byte.<br>
* 							out: track 1 data(ASCII).
*  @param[in, out] pInOutLen1 in: size of track data buffer "pTrack1" <br>
* 							out: if "pInOutLen1[0]" > 0, track data "pTrack1" ok and is the size of "pTrack1"<br>
*                                if "pInOutLen1[0]" == 0, track data "pTrack1" error or blank track.
*  @param[in, out] pTrack2 	in: track 2 data buffer, at least 256 byte.<br>
* 							out: track 2 data(ASCII).
*  @param[in, out] pInOutLen2 in: size of track data buffer "pTrack2" <br>
* 							out: if "pInOutLen2[0]" > 0, track data "pTrack2" ok and is the size of "pTrack2"<br>
*                                if "pInOutLen2[0]" == 0, track data "pTrack2" error or blank track.
*  @param[in, out] pTrack3 	in: track 3 data buffer, at least 256 byte.<br>
* 							out: track 3 data(ASCII).
*  @param[in, out] pInOutLen3 in: size of track data buffer "pTrack3" <br>
* 							 out: if "pInOutLen3[0]" > 0, track data "pTrack3" ok and is the size of "pTrack3"<br>
*                                 if "pInOutLen3[0]" == 0, track data "pTrack3" error or blank track.
*  @param[in] iTimeOut  waiting timeout(ms) before read data. if < 0, return until read data, == 0, return immediately,
* 							> 0, return after waitting at most "iTimeOut" ms.
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.1
*/
int AutoReadMGCard(int portfd, unsigned char *pTrack1, int *pInOutLen1, 
								unsigned char *pTrack2, int *pInOutLen2,  
								unsigned char *pTrack3,  int *pInOutLen3, 
								int iTimeOut);
/**
*  Write magnetic track data in ISO 7811 protocol format.
*  __Only universal version have this function__
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] ucTrackID track id. see the value below.
*  @param[in, out] pTrack1  in: track 1 data(ASCII).<br>
* 							out: track 1 data(ASCII) after successful written.
*  @param[in, out] pInOutLen1 in: size of track data buffer "pTrack1" which should <= 76 byte.<br>
* 							out: if "pInOutLen1[0]" > 0, track data "pTrack1" ok and is the size of "pTrack1"<br>
*                                if "pInOutLen1[0]" == 0, track data "pTrack1" error or blank track.
*  @param[in, out] pTrack2 	in: track 2 data(ASCII).<br>
* 							out: track 2 data(ASCII) after successful written..
*  @param[in, out] pInOutLen2 in: size of track data buffer "pTrack2" which should <= 37 byte.<br>
* 							out: if "pInOutLen2[0]" > 0, track data "pTrack2" ok and is the size of "pTrack2"<br>
*                                if "pInOutLen2[0]" == 0, track data "pTrack2" error or blank track.
*  @param[in, out] pTrack3 	in: track 3 data(ASCII).<br>
* 							out: track 3 data(ASCII) after successful written..
*  @param[in, out] pInOutLen3 in: size of track data buffer "pTrack3" which should <= 104 byte.<br<br>
* 							 out: if "pInOutLen3[0]" > 0, track data "pTrack3" ok and is the size of "pTrack3"<br>
*                                 if "pInOutLen3[0]" == 0, track data "pTrack3" error or blank track.
*  @param[in] iTimeOut  waiting timeout(ms) before writing data. if < 0, return until write data, == 0, return immediately,
* 							> 0, return after waitting at most "iTimeOut" ms.
*
* ucTrackID value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 1	| read track 1  |
* | 2	| read track 2  |	
* | 3	|  read track 3   |
* | 4	|  read track 1 2 |	
* | 5	|  read track 2 3 |
* | 6	|  read track 1 3 |	
* | 7	|  read track 1 2 3 |

*  @return  reference to the @ref APIReturn.
*        
*@since 0.1
*/
int WriteMGCard(int portfd, unsigned char ucTrackID,
								   unsigned char *pTrack1, int *pInOutLen1, 
								   unsigned char *pTrack2, int *pInOutLen2,  
								   unsigned char *pTrack3, int *pInOutLen3, 
								   int iTimeOut);

/**
*  Erase magnetic track.
*  __Only customized version have this function__
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*  @param[in] ucTrackID track id. see the value below.
*  @param[in] iTimeOut  waiting timeout(ms) before erase data. if < 0, return until erase data, == 0, return immediately,
* 							> 0, return after waitting at most "iTimeOut" ms.
*
* ucTrackID value:
* | value	  | 	meaning	      | 
* | ------   |------------- | 	
* | 1	| read track 1  |
* | 2	| read track 2  |	
* | 3	|  read track 3   |
* | 4	|  read track 1 2 |	
* | 5	|  read track 2 3 |
* | 6	|  read track 1 3 |	
* | 7	|  read track 1 2 3 |

*  @return  reference to the @ref APIReturn.
*        
*@since 0.1
*/
int EraseMGCard(int portfd, unsigned char ucTrackID,
								   int iTimeOut);
/**
*  Cancel read/write magnetic track operation.
*
*  @param[in] portfd the device port file description. Must not be <= 0.
*
*  @return  reference to the @ref APIReturn.
*        
*@since 0.1
*/
int CancelMGCard(int portfd);					
/** @} */
/*******************************************************************************/
#ifdef __cplusplus
}
#endif //__cplusplus

#endif //_WBT2000_H_
