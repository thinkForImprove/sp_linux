﻿/******************************************************************************
*                                                                             *
* xfssiu.h      WOSA/XFS - definitions                                        *
*               for the Sensors and Indicators Unit - services                *
*                                                                             *
*               Version 3.01 (16/11/01)                                       *
*                                                                             *
******************************************************************************/
 
#ifndef __INC_XFSSIU__H
#define __INC_XFSSIU__H

// #ifdef __cplusplus
// extern "C" {
// #endif

#include "XFSAPI.H"

/* be aware of alignment */
#pragma pack (push, 1)

/* values of WFSSIUCAPS.wClass */

/* XFS_COMMON.H 中定义
#ifdef QT_WIN_LINUX_XFS

#define     WFS_SERVICE_CLASS_SIU               (8)
#define     WFS_SERVICE_CLASS_VERSION_SIU       0x0303

#else

#define     WFS_SERVICE_CLASS_SIU               (2)
#define     WFS_SERVICE_CLASS_VERSION_SIU       0x0001

#endif
*/

#define     WFS_SERVICE_CLASS_NAME_SIU          "SIU"
#define     SIU_SERVICE_OFFSET                  (WFS_SERVICE_CLASS_SIU * 100)


/* SIU Info Commands */

#define     WFS_INF_SIU_STATUS                  (SIU_SERVICE_OFFSET + 1)
#define     WFS_INF_SIU_CAPABILITIES            (SIU_SERVICE_OFFSET + 2)


/* SIU Command Verbs */

#define     WFS_CMD_SIU_ENABLE_EVENTS           (SIU_SERVICE_OFFSET + 1)
#define     WFS_CMD_SIU_SET_PORTS               (SIU_SERVICE_OFFSET + 2)
#define     WFS_CMD_SIU_SET_DOOR                (SIU_SERVICE_OFFSET + 3)
#define     WFS_CMD_SIU_SET_INDICATOR           (SIU_SERVICE_OFFSET + 4)
#define     WFS_CMD_SIU_SET_AUXILIARY           (SIU_SERVICE_OFFSET + 5)
#define     WFS_CMD_SIU_SET_GUIDLIGHT           (SIU_SERVICE_OFFSET + 6)
#define     WFS_CMD_SIU_RESET                   (SIU_SERVICE_OFFSET + 7)
#define     WFS_CMD_SIU_POWER_SAVE_CONTROL      (SIU_SERVICE_OFFSET + 8)

/* SIU Messages */

#define     WFS_SRVE_SIU_PORT_STATUS            (SIU_SERVICE_OFFSET + 1)
#define     WFS_EXEE_SIU_PORT_ERROR             (SIU_SERVICE_OFFSET + 2)
#define     WFS_SRVE_SIU_POWER_SAVE_CHANGE      (SIU_SERVICE_OFFSET + 3)

/* Values of WFSSIUSTATUS.fwDevice */

#define     WFS_SIU_DEVONLINE                   WFS_STAT_DEVONLINE
#define     WFS_SIU_DEVOFFLINE                  WFS_STAT_DEVOFFLINE
#define     WFS_SIU_DEVPOWEROFF                 WFS_STAT_DEVPOWEROFF
#define     WFS_SIU_DEVNODEVICE                 WFS_STAT_DEVNODEVICE
#define     WFS_SIU_DEVHWERROR                  WFS_STAT_DEVHWERROR
#define     WFS_SIU_DEVUSERERROR                WFS_STAT_DEVUSERERROR
#define     WFS_SIU_DEVBUSY                     WFS_STAT_DEVBUSY


/* Size and max index of fwSensors array */

#define     WFS_SIU_SENSORS_SIZE                (32)
#define     WFS_SIU_SENSORS_MAX                 (WFS_SIU_SENSORS_SIZE - 1)


/* Size and max index of fwDoors array */

#define     WFS_SIU_DOORS_SIZE                  (16)
#define     WFS_SIU_DOORS_MAX                   (WFS_SIU_DOORS_SIZE - 1)


/* Size and max index of fwIndicators array */

#define     WFS_SIU_INDICATORS_SIZE             (16)
#define     WFS_SIU_INDICATORS_MAX              (WFS_SIU_INDICATORS_SIZE - 1)


/* Size max index of fwAuxiliaries array */

#define     WFS_SIU_AUXILIARIES_SIZE            (16)
#define     WFS_SIU_AUXILIARIES_MAX             (WFS_SIU_AUXILIARIES_SIZE - 1)


/* Size and max index of fwGuidLights array */

#if defined(SET_BANK_CMBC) | defined(SET_BANK_CSCB) | defined(SET_BANK_SXXH) | defined(SET_BANK_JIANGNAN) || defined(SET_BANK_JINDIAN)
    #define     WFS_SIU_GUIDLIGHTS_SIZE             (32)
#else
    #define     WFS_SIU_GUIDLIGHTS_SIZE             (16)
#endif
#define     WFS_SIU_GUIDLIGHTS_MAX              (WFS_SIU_GUIDLIGHTS_SIZE - 1)


/* Indices of WFSSIUSTATUS.fwSensors [...]
          WFSSIUCAPS.fwSensors [...]
          WFSSIUENABLE.fwSensors [...]
          WFSSIUPORTEVENT.wPortIndex
          WFSSIUPORTERROR.wPortIndex */

#define     WFS_SIU_OPERATORSWITCH              (0)
#define     WFS_SIU_TAMPER                      (1)
#define     WFS_SIU_INTTAMPER                   (2)
#define     WFS_SIU_SEISMIC                     (3)
#define     WFS_SIU_HEAT                        (4)
#define     WFS_SIU_PROXIMITY                   (5)
#define     WFS_SIU_AMBLIGHT                    (6)
#define     WFS_SIU_ENHANCEDAUDIO               (7)
#define     WFS_SIU_BOOT_SWITCH                 (8)
#define     WFS_SIU_CONSUMER_DISPLAY            (9)
#define     WFS_SIU_OPERATOR_CALL_BUTTON        (10)
#define     WFS_SIU_HANDSETSENSOR               (11)
#define     WFS_SIU_GENERALINPUTPORT            (12)


/* Indices of WFSSIUSTATUS.fwDoors [...]
           WFSSIUCAPS.fwDoors [...]
           WFSSIUENABLE.fwDoors [...]
           WFSSIUSETPORT.fwDoors [...]
           WFSSIUSETDOORS.wDoor
           WFSSIUPORTEVENT.wPortIndex
           WFSSIUPORTERROR.wPortIndex */

#define     WFS_SIU_CABINET                     (0)
#define     WFS_SIU_SAFE                        (1)
#define     WFS_SIU_VANDALSHIELD                (2)
#define     WFS_SIU_CABINET_FRONT               (3)
#define     WFS_SIU_CABINET_REAR                (4)
#define     WFS_SIU_CABINET_LEFT                (5)
#define     WFS_SIU_CABINET_RIGHT               (6)

/* Indices of WFSSIUSTATUS.fwIndicators [...]
           WFSSIUCAPS.fwIndicators [...]
           WFSSIUENABLE.fwIndicators [...]
           WFSSIUSETPORT.wIndicators [...]
           WFSSIUSETINDICATORS.wIndicator
           WFSSIUPORTEVENT.wPortIndex
           WFSSIUPORTERROR.wPortIndex */

#define     WFS_SIU_OPENCLOSE                   (0)
#define     WFS_SIU_FASCIALIGHT                 (1)
#define     WFS_SIU_AUDIO                       (2)
#define     WFS_SIU_HEATING                     (3)
#define     WFS_SIU_CONSUMER_DISPLAY_BACKLIGHT  (4)
#define     WFS_SIU_SIGNAGEDISPLAY              (5)
#define     WFS_SIU_TRANSINDICATOR              (6)
#define     WFS_SIU_GENERALOUTPUTPORT           (7)


/* Indices of WFSSIUSTATUS.fwAuxiliaries [...]
           WFSSIUCAPS.fwAuxiliaries [...]
           WFSSIUENABLE.fwAuxiliaries [...]
           WFSSIUSETPORT.wAuxiliaries [...]
           WFSSIUSETAUXILIARIES.wAuxiliary
           WFSSIUPORTEVENT.wPortIndex
           WFSSIUPORTERROR.wPortIndex */

#define     WFS_SIU_VOLUME                      (0)
#define     WFS_SIU_UPS                         (1)
#define     WFS_SIU_REMOTE_STATUS_MONITOR       (2)
#define     WFS_SIU_AUDIBLE_ALARM               (3)
#define     WFS_SIU_ENHANCEDAUDIOCONTROL        (4)


/* Indices of WFSSIUSTATUS.fwGuidLights [...]
           WFSSIUCAPS.fwGuidLights [...]
           WFSSIUENABLE.fwGuidLights [...]
           WFSSIUSETPORT.wGuidLights [...]
           WFSSIUSETGUIDLIGHTS.wGuidLight
           WFSSIUPORTEVENT.wPortIndex
           WFSSIUPORTERROR.wPortIndex */

#define     WFS_SIU_CARDUNIT                    (0)     //读卡器灯（或发卡机）
#define     WFS_SIU_PINPAD                      (1)     //密码键盘灯
#define     WFS_SIU_NOTESDISPENSER              (2)     //出钞指示灯
#define     WFS_SIU_COINDISPENSER               (3)     //硬币取款灯/射频卡指示灯
#define     WFS_SIU_RECEIPTPRINTER              (4)     //凭条打印机灯
#define     WFS_SIU_PASSBOOKPRINTER             (5)     //存折指示灯
#define     WFS_SIU_ENVDEPOSITORY               (6)     //强存/刷磁设备
#define     WFS_SIU_CHEQUEUNIT                  (7)
#define     WFS_SIU_ENVSCANNER                  (7)     //票据扫描/支票处理单元
#define     WFS_SIU_BILLACCEPTOR                (8)     //存款模块灯
#define     WFS_SIU_ENVDISPENSER                (9)     //票据发售UKey
#define     WFS_SIU_DOCUMENTPRINTER             (10)    //A4 激光打印机灯
#define     WFS_SIU_COINACCEPTOR                (11)    //硬币存款/身份证指示灯
#define     WFS_SIU_SCANNER                     (12)    //扫描仪器
#if defined(SET_BANK_CMBC)
#define     LFS_SIU_ICCARDUNIT                  (13)	//接触式IC读卡器
#define     LFS_SIU_RFCARDUNIT                  (14)	//非接触式IC卡读卡器
#define     LFS_SIU_TRACKCARDUNIT               (15)	//磁条刷卡器
#define     LFS_SIU_IDCARDREADER                (16)	//身份证阅读器
#define     LFS_SIU_FINGER                      (17)	//指纹仪
#define     LFS_SIU_KEYDISPENSER                (18)	//发key模块出口
#define     LFS_SIU_FORCEACCEPTOR               (19)	//强存模块入口
#define     LFS_SIU_BUNDLEDISPENSE              (20)	//整把取款出口
#define     LFS_SIU_CODESCANNER                 (21)	//二维码扫描器
#define     LFS_SIU_HIGHCAMERA                  (22)	//高拍仪
#elif defined(SET_BANK_CSCB)
#define     WFS_SIU_CHECKSCANNER                (13)	//二维码阅读器指示灯
#define     WFS_SIU_FINGERPRINT                 (14)	//指纹仪灯
#define     WFS_SIU_USBCONTROL                  (15)	//USB 通断电控制
#define     WFS_SIU_CAMERA                      (16)	//摄像头补光灯
#define     WFS_SIU_PASSBOOKREADER              (17)	//刷存折指示灯
#define     WFS_SIU_CARDBOX                     (18)	//发卡盒指示灯
#define     WFS_SIU_RFCARDUNIT                  (19)	//非接读卡器
#define     WFS_SIU_IDCARDREADER                (20)	//身份证阅读模块灯
#define     WFS_SIU_SCANNER                     (24)	//硬币取款模块指示灯
#define     WFS_SIU_CHEQUEUNIT                  (25)    //氛围灯
#define     WFS_SIU_BILLPRINTER                 (26)    //发票打印机
#define     WFS_SIU_HIGHCAMERA                  (27)    //高拍仪指示灯
#elif defined(SET_BANK_SXXH)
#define     WFS_SIU_FINGERPRINT                 (13)	//指纹仪灯
#define     WFS_SIU_DEPOSITRECPREADER           (14)	//存单读取
#define     WFS_SIU_DEPOSITRECPPRINTER          (15)	//存单打印
#elif defined(SET_BANK_JIANGNAN)
#define     WFS_SIU_CHECKSCANNER                (13)	//二维码阅读器指示灯
#define     WFS_SIU_FINGERPRINT                 (14)	//指纹仪灯
#define     WFS_SIU_USBCONTROL                  (15)	//USB 通断电控制
#define     WFS_SIU_CAMERA                      (16)	//摄像头补光灯
#define     WFS_SIU_HIGHCAMERA                  (17)    //高拍仪指示灯
#define     WFS_SIU_CARDBOX                     (18)    //发卡盒指示灯
#define     WFS_SIU_PASSBOOKREADER              (19)	//刷存折指示灯
#define     WFS_SIU_ENVELOPEDEPOSITORY          (20)    //
#define     WFS_SIU_UKEY                        (21)    //UKey
#define     WFS_SIU_ATMOSPHERE                  (22)    //
#define     WFS_SIU_IDCARDREADER                (23)    //身份证阅读模块灯
#else
#define     WFS_SIU_UKEY                        (12)    //UKey
#define     WFS_SIU_FINGER	                (14)    // 新增加指纹灯
#endif

/* Values of WFSSIUSTATUS.fwSensors [...]
          WFSSIUSTATUS.fwDoors [...]
          WFSSIUSTATUS.fwIndicators [...]
          WFSSIUSTATUS.fwAuxiliaries [...]
          WFSSIUSTATUS.fwGuidLights [...]
          WFSSIUCAPS.fwSensors [...]
          WFSSIUCAPS.fwDoors [...]
          WFSSIUCAPS.fwIndicators [...]
          WFSSIUCAPS.fwAuxiliaries [...]
          WFSSIUCAPS.fwGuidLights [...] */

#define     WFS_SIU_NOT_AVAILABLE               (0x0000)
#define     WFS_SIU_AVAILABLE                   (0x0001)


/* Values of WFSSIUSTATUS.fwSensors [WFS_SIU_OPERATORSWITCH]
          WFSSIUCAPS.fwSensors [WFS_SIU_OPERATORSWITCH]
          WFSSIUPORTEVENT.fwPortStatus
          WFSSIUPORTERROR.fwPortStatus */

#define     WFS_SIU_RUN                         (0x0001)
#define     WFS_SIU_MAINTENANCE                 (0x0002)
#define     WFS_SIU_SUPERVISOR                  (0x0004)


/* Values of WFSSIUSTATUS.fwDoors [...]
          WFSSIUSTATUS.fwIndicators [WFS_SIU_OPENCLOSE]
          WFSSIUCAPS.fwDoors [...]
          WFSSIUCAPS.fwIndicators [WFS_SIU_OPENCLOSE]
          WFSSIUSETPORT.fwDoors [...]
          WFSSIUSETPORT.fwIndicators [WFS_SIU_OPENCLOSE]
          WFSSIUSETDOOR.wDoor
          WFSSIUSETINDICATOR.wCommand
          WFSSIUPORTEVENT.wPortStatus
          WFSSIUPORTERROR.wPortStatus */

#define     WFS_SIU_CLOSED                      (0x0001)
#define     WFS_SIU_OPEN                        (0x0002)
#define     WFS_SIU_LOCKED                      (0x0004)
#define     WFS_SIU_BOLTED                      (0x0008)
#define     WFS_SIU_SERVICE                     (0x0010)
#define     WFS_SIU_KEYBOARD                    (0x0020)
#define     WFS_SIU_AJAR                        (0x0040)
#define     WFS_SIU_JAMMED                      (0x0080)


/* Values of WFSSIUSTATUS.fwIndicators [WFS_SIU_AUDIO]
          WFSSIUSETPORT.fwIndicators [WFS_SIU_AUDIO]
          WFSSIUSETINDICATOR.wCommand
          WFSSIUPORTEVENT.wPortStatus
          WFSSIUPORTERROR.wPortStatus */

#define     WFS_SIU_KEYPRESS                    (0x0002)
#define     WFS_SIU_EXCLAMATION                 (0x0004)
#define     WFS_SIU_WARNING                     (0x0008)
#define     WFS_SIU_ERROR                       (0x0010)
#define     WFS_SIU_CRITICAL                    (0x0020)

/* Values of WFSSIUSTATUS.wSensors [WFS_SIU_CONSUMER_DISPLAY]
            WFSSIUPORTEVENT.wPort_Status
            WFSSIUPORTERROR.wPort_Status */

#define       WFS_SIU_DISPLAY_ERROR                      (0x0004)

/* Flags for WFSSIUSTATUS.wIndicators [WFS_SIU_TRANSINDICATOR]
            WFSSIUSETPORT.wIndicators [WFS_SIU_TRANSINDICATOR]
            WFSSIUSETINDICATOR.wCommand
            WFSSIUPORTEVENT.wPort_Status[WFS_SIU_TRANSINDICATOR]
            WFSSIUPORTERROR.wPort_Status[WFS_SIU_TRANSINDICATOR] */

#define       WFS_SIU_LAMP1                              (0x0001)
#define       WFS_SIU_LAMP2                              (0x0002)
#define       WFS_SIU_LAMP3                              (0x0004)
#define       WFS_SIU_LAMP4                              (0x0008)
#define       WFS_SIU_LAMP5                              (0x0010)
#define       WFS_SIU_LAMP6                              (0x0020)
#define       WFS_SIU_LAMP7                              (0x0040)
#define       WFS_SIU_LAMP8                              (0x0080)
#define       WFS_SIU_LAMP9                              (0x0100)
#define       WFS_SIU_LAMP10                             (0x0200)
#define       WFS_SIU_LAMP11                             (0x0400)
#define       WFS_SIU_LAMP12                             (0x0800)
#define       WFS_SIU_LAMP13                             (0x1000)
#define       WFS_SIU_LAMP14                             (0x2000)
#define       WFS_SIU_LAMP15                             (0x4000)
#define       WFS_SIU_LAMP16                             (0x8000)

/* Values of WFSSIUSTATUS.fwAuxiliaries [WFS_SIU_REMOTE_STATUS_MONITOR]
          WFSSIUSETPORT.fwAuxiliaries [WFS_SIU_REMOTE_STATUS_MONITOR]
          WFSSIUSETAUXILIARY.fwCommand
          WFSSIUPORTEVENT.wPortStatus
          WFSSIUPORTERROR.wPortStatus */

#define     WFS_SIU_GREEN_LED_ON                (0x0001)
#define     WFS_SIU_GREEN_LED_OFF               (0x0002)
#define     WFS_SIU_AMBER_LED_ON                (0x0004)
#define     WFS_SIU_AMBER_LED_OFF               (0x0008)
#define     WFS_SIU_RED_LED_ON                  (0x0010)
#define     WFS_SIU_RED_LED_OFF                 (0x0020)

/* Values of WFSSIUSTATUS.fwAuxiliaries [WFS_SIU_ENHANCEDAUDIOCONTROL]
          WFSSIUSETPORT.fwAuxiliaries [WFS_SIU_ENHANCEDAUDIOCONTROL]
          WFSSIUSETAUXILIARY.fwCommand
          WFSSIUPORTEVENT.wPortStatus
          WFSSIUPORTERROR.wPortStatus */

#define     WFS_SIU_PUBLICAUDIO_MANUAL          (0x0001)
#define     WFS_SIU_PUBLICAUDIO_AUTO            (0x0002)
#define     WFS_SIU_PUBLICAUDIO_SEMI_AUTO       (0x0004)
#define     WFS_SIU_PRIVATEAUDIO_MANUAL         (0x0008)
#define     WFS_SIU_PRIVATEAUDIO_AUTO           (0x0010)
#define     WFS_SIU_PRIVATEAUDIO_SEMI_AUTO      (0x0020)



/* Values of WFSSIUSTATUS.fwSensors [...]
          WFSSIUSTATUS.fwIndicators [...]
          WFSSIUSTATUS.fwAuxiliaries [...]
          WFSSIUSTATUS.fwGuidLights [...]
          WFSSIUCAPS.fwSensors [...]
          WFSSIUCAPS.fwIndicators [...]
          WFSSIUCAPS.fwGuidLights [...]
          WFSSIUSETPORT.fwIndicators [...]
          WFSSIUSETPORT.fwAuxiliaries [...]
          WFSSIUSETPORT.fwGuidLights [...]
          WFSSIUSETINDICATORS.fwCommand [...]
          WFSSIUSETAUXILIARY.fwCommand [...]
          WFSSIUSETGUIDLIGHTS.fwCommand [...]
          WFSSIUPORTEVENT.wPortStatus
          WFSSIUPORTERROR.wPortStatus */

#define     WFS_SIU_OFF                         (0x0001)
#define     WFS_SIU_ON                          (0x0002)
#define     WFS_SIU_SLOW_FLASH                  (0x0004)
#define     WFS_SIU_MEDIUM_FLASH                (0x0008)
#define     WFS_SIU_QUICK_FLASH                 (0x0010)
#define     WFS_SIU_CONTINUOUS                  (0x0080)

/* Flags for WFSSIUSTATUS.wSensors [WFS_SIU_GENERALINPUTPORT]
            WFSSIUSTATUS.wIndicators [WFS_SIU_GENERALOUTPUTPORT]
            WFSSIUSETPORT.wIndicators [WFS_SIU_GENERALOUTPUTPORT]
            WFSSIUSETINDICATOR.wCommand
            WFSSIUPORTEVENT.wPort_Status[WFS_SIU_GENERALINPUTPORT]
            WFSSIUPORTEVENT.wPort_Status[WFS_SIU_GENERALOUTPUTPORT]
            WFSSIUPORTERROR.wPort_Status[WFS_SIU_GENERALINPUTPORT]
            WFSSIUPORTERROR.wPort_Status[WFS_SIU_GENERALOUTPUTPORT] */

#define       WFS_SIU_GPP1                               (0x0001)
#define       WFS_SIU_GPP2                               (0x0002)
#define       WFS_SIU_GPP3                               (0x0004)
#define       WFS_SIU_GPP4                               (0x0008)
#define       WFS_SIU_GPP5                               (0x0010)
#define       WFS_SIU_GPP6                               (0x0020)
#define       WFS_SIU_GPP7                               (0x0040)
#define       WFS_SIU_GPP8                               (0x0080)
#define       WFS_SIU_GPP9                               (0x0100)
#define       WFS_SIU_GPP10                              (0x0200)
#define       WFS_SIU_GPP11                              (0x0400)
#define       WFS_SIU_GPP12                              (0x0800)
#define       WFS_SIU_GPP13                              (0x1000)
#define       WFS_SIU_GPP14                              (0x2000)
#define       WFS_SIU_GPP15                              (0x4000)
#define       WFS_SIU_GPP16                              (0x8000)

/* Values of WFSSIUSTATUS.fwSensors [WFS_SIU_PROXIMITY]
          WFSSIUSTATUS.fwSensors [WFS_SIU_ENHANCEDCONTROL]
          WFSSIUPORTEVENT.wPortStatus
          WFSSIUPORTERROR.wPortStatus */

#define     WFS_SIU_PRESENT                     (0x0001)
#define     WFS_SIU_NOT_PRESENT                 (0x0002)   

/* Values of WFSSIUSTATUS.wSensors [WFS_SIU_HANDSETSENSOR] */

#define       WFS_SIU_OFF_THE_HOOK                       (0x0001)
#define       WFS_SIU_ON_THE_HOOK                        (0x0002)
/* Values of WFSSIUCAPS.fwSensors [WFS_SIU_ENHANCEDAUDIO] */

#define     WFS_SIU_MANUAL                     (0x0001)
#define     WFS_SIU_AUTO                       (0x0002)   
#define     WFS_SIU_SEMI_AUTO                  (0X0004)


/* Values of WFSSIUSTATUS.fwSensors [WFS_SIU_AMBLIGHT]
          WFSSIUCAPS.fwSensors [WFS_SIU_AMBLIGHT]
          WFSSIUPORTEVENT.fwPortStatus
          WFSSIUPORTERROR.fwPortStatus */

#define     WFS_SIU_VERY_DARK                   (0x0001)
#define     WFS_SIU_DARK                        (0x0002)
#define     WFS_SIU_MEDIUM_LIGHT                (0x0004)
#define     WFS_SIU_LIGHT                       (0x0008)
#define     WFS_SIU_VERY_LIGHT                  (0x0010)


/* Values of WFSSIUSTATUS.fwAuxiliaries [WFS_SIU_UPS]
             WFSSIUCAPS.fwAuxiliaries [WFS_SIU_UPS]
             WFSSIUPORTEVENT.wPortStatus
             WFSSIUPORTERROR.wPortStatus */

#define WFS_SIU_LOW                             (0x0002)
#define WFS_SIU_ENGAGED                         (0x0004)
#define WFS_SIU_POWERING                        (0x0008)
#define WFS_SIU_RECOVERED                       (0x0010)


/* Values of WFSSIUCAPS.fwType */

#define     WFS_SIU_SENSORS                     (0x0001)
#define     WFS_SIU_DOORS                       (0x0002)
#define     WFS_SIU_INDICATORS                  (0x0004)
#define     WFS_SIU_AUXILIARIES                 (0x0008)
#define     WFS_SIU_GUIDLIGHTS                  (0x0010)

/* Values of WFSSIUCAPS.fwAuxiliaries [WFS_SIU_ENHANCEDAUDIOCONTROL] */
#define     WFS_SIU_HEADSET_DETECTION		(0x0001)
#define     WFS_SIU_MODE_CONTROLLABLE      	(0x0002)

/* Values of WFSSIUENABLE.fwSensors [...]
          WFSSIUENABLE.fwDoors [...]
          WFSSIUENABLE.fwIndicators [...]
          WFSSIUENABLE.fwAuxiliaries [...]
          WFSSIUENABLE.fwGuidLights [...]
          WFSSIUSETPORTS.fwDoors [...]
          WFSSIUSETPORTS.fwIndicators [...]
          WFSSIUSETPORTS.fwAuxiliaries [...]
          WFSSIUSETPORTS.fwGuidLights [...] */

#define     WFS_SIU_NO_CHANGE                   (0x0000)
#define     WFS_SIU_ENABLE_EVENT                (0x0001)
#define     WFS_SIU_DISABLE_EVENT               (0x0002)


/* Values of WFSSIUSETPORTS.fwDoors [...]
          WFSSIUSETDOORS.fwCommand [...] */

#define     WFS_SIU_BOLT                        (0x0001)
#define     WFS_SIU_UNBOLT                      (0x0002)


/* Values of WFSSIUSETPORTS.fwAuxiliaries [WFS_SIU_UPS]
      WFSSIUSETAUXILIARY.wAuxiliary [WFS_SIU_UPS] */

#define WFS_SIU_ENGAGE                          (0x0001)
#define WFS_SIU_DISENGAGE                       (0x0002)


/* XFS SIU Errors */ 

#define     WFS_ERR_SIU_INVALID_PORT            (-(SIU_SERVICE_OFFSET + 1))
#define     WFS_ERR_SIU_SYNTAX                  (-(SIU_SERVICE_OFFSET + 2))
#define     WFS_ERR_SIU_PORT_ERROR              (-(SIU_SERVICE_OFFSET + 3))
#define     WFS_ERR_SIU_POWERSAVETOOSHORT       (-(SIU_SERVICE_OFFSET + 4))

/*=================================================================*/
/* SIU Info Command Structures and variables */
/*=================================================================*/

typedef struct _wfs_siu_status
{
    WORD            fwDevice;
    WORD            fwSensors [WFS_SIU_SENSORS_SIZE];
    WORD            fwDoors [WFS_SIU_DOORS_SIZE];
    WORD            fwIndicators [WFS_SIU_INDICATORS_SIZE];
    WORD            fwAuxiliaries [WFS_SIU_AUXILIARIES_SIZE];
    WORD            fwGuidLights [WFS_SIU_GUIDLIGHTS_SIZE];
    LPSTR           lpszExtra;
	USHORT          usPowerSaveRecoveryTime;
} WFSSIUSTATUS, * LPWFSSIUSTATUS;

typedef struct _wfs_siu_caps
{
    WORD            wClass;
    WORD            fwType;
    WORD            fwSensors [WFS_SIU_SENSORS_SIZE];
    WORD            fwDoors [WFS_SIU_DOORS_SIZE];
    WORD            fwIndicators [WFS_SIU_INDICATORS_SIZE];
    WORD            fwAuxiliaries [WFS_SIU_AUXILIARIES_SIZE];
    WORD            fwGuidLights [WFS_SIU_GUIDLIGHTS_SIZE];
    LPSTR           lpszExtra;
 	BOOL            bPowerSaveControl;
} WFSSIUCAPS, * LPWFSSIUCAPS;


/*=================================================================*/
/* SIU Execute Command Structures */
/*=================================================================*/

typedef struct _wfs_siu_enable
{
    WORD            fwSensors [WFS_SIU_SENSORS_SIZE];
    WORD            fwDoors [WFS_SIU_DOORS_SIZE];
    WORD            fwIndicators [WFS_SIU_INDICATORS_SIZE];
    WORD            fwAuxiliaries [WFS_SIU_AUXILIARIES_SIZE];
    WORD            fwGuidLights [WFS_SIU_GUIDLIGHTS_SIZE];
    LPSTR           lpszExtra;
} WFSSIUENABLE, * LPWFSSIUENABLE;


typedef struct _wfs_siu_set_ports
{
    WORD            fwDoors [WFS_SIU_DOORS_SIZE];
    WORD            fwIndicators [WFS_SIU_INDICATORS_SIZE];
    WORD            fwAuxiliaries [WFS_SIU_AUXILIARIES_SIZE];
    WORD            fwGuidLights [WFS_SIU_GUIDLIGHTS_SIZE];
    LPSTR           lpszExtra;
} WFSSIUSETPORTS, * LPWFSSIUSETPORTS;


typedef struct _wfs_siu_set_door
{
    WORD            wDoor;
    WORD            fwCommand;
} WFSSIUSETDOOR, * LPWFSSIUSETDOOR;


typedef struct _wfs_siu_set_indicator
{
    WORD            wIndicator;
    WORD            fwCommand;
} WFSSIUSETINDICATOR, * LPWFSSIUSETINDICATOR;


typedef struct _wfs_siu_set_auxiliary
{
    WORD            wAuxiliary;
    WORD            fwCommand;
} WFSSIUSETAUXILIARY, * LPWFSSIUSETAUXILIARY;


typedef struct _wfs_siu_set_guidlight
{
    WORD            wGuidLight;
    WORD            fwCommand;
} WFSSIUSETGUIDLIGHT, * LPWFSSIUSETGUIDLIGHT;

typedef struct _wfs_siu_power_save_control
{
     USHORT            usMaxPowerSaveRecoveryTime;
} WFSSIUPOWERSAVECONTROL, *LPWFSSIUPOWERSAVECONTROL;

/*=================================================================*/
/* SIU Message Structures */
/*=================================================================*/

typedef struct _wfs_siu_port_event
{
    WORD            wPortType;
    WORD            wPortIndex;
    WORD            wPortStatus;
    LPSTR           lpszExtra;
} WFSSIUPORTEVENT, * LPWFSSIUPORTEVENT;


typedef struct _wfs_siu_port_error
{
    WORD            wPortType;
    WORD            wPortIndex;
    HRESULT         PortError;
    WORD            wPortStatus;
    LPSTR           lpszExtra;
} WFSSIUPORTERROR, * LPWFSSIUPORTERROR;

typedef struct _wfs_siu_power_save_change
{
     USHORT            usPowerSaveRecoveryTime;
} WFSSIUPOWERSAVECHANGE, *LPWFSSIUPOWERSAVECHANGE;
/* restore alignment */
#pragma pack (pop)

// #ifdef __cplusplus
// }       /*extern "C"*/
// #endif

#endif  /* __INC_XFSSIU__H */

