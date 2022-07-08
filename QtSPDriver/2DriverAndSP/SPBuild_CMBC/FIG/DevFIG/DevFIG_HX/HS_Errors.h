#ifndef HS_ERRORS_H
#define HS_ERRORS_H

#define ERR_BREAKOFF				-13
#define ERR_COMMUNICATION			-101
#define ERR_NONE					0
#define ERR_OPENDEVICE				1
#define ERR_USBMANAGER				2
#define ERR_DEVLISTEMPTY			3
#define ERR_NOFPMODULE				4
#define ERR_WAITPERMISSION_TIMEOUT  5
#define ERR_PARAMETER				6
#define ERR_FILEOPEN				7
#define ERR_FIRMWAREFLAG			8
#define ERR_CHECKSUM				9
#define ERR_UPDATEFIRMWARE			10
#define ERR_TIMEOUT					11
#define ERR_NOWORKKEY				12
#define ERR_WSQENCODE				13
#define ERR_LICENSE					14
#define ERR_LOADLIBRARY				15
#define ERR_EXTRACTFEATURE			16
#define ERR_WSQDECODE				17
#define ERR_KEY_NOTEXIST			18
#define ERR_NOTLIVE					19
#define ERR_DEVICE_DESTROY			55
#define ERR_LICENSE_ERR				0x28
#define ERR_FINGER_REMOVE			20

#define ERR_OTHER					99
#define ERR_MALLOC					100
#define ERR_NOOPENED				101

int getErrorDescript(int errNo, char * lpErrMsg);

#endif
