//////////////////////////////////////////////////////////////////////////////
// ┬ис?ф┬ис??┬ис?┐с??с?╗= ┬ис?х┬иУЪ?с?Ё?с??с?╣с?ис?Бс?дс??с?Цс??с??с?ф┬ис??┬ис?┐┬ис?д┬ис??┬ис??┬иqd
//  NAME   = HWSIUDEF.CPP
//  Т?ЦС╗?   = 2014/08/04
//  №╝Х№╝▓№╝┤ = 40-21-00-01
// HISTORY = HWSIU,40-00-00-07,2010/07/01 : TS-EA45
// HISTORY = HWSIU,40-00-01-00,2010/08/04 : IOMCж?ют?│с?│с?╝с??тц?Т?┤
// HISOTRY = HWSIU,40-00-01-01,2010/08/16 : с?│с?╝с??с?Бс?│с??С┐?ТГБ
// HISTORY = HWSIU,40-00-01-02,2010/08/18 : MTCс?│с?╝с??тц?Т??т?ду??У┐йт?а
// History = HWSIU,40-01-00-04(DS1#0004),2010/11/01 : by BHH-lixw for У╝Ют║дУеГт??
// History = HWSIU,40-02-00-09(DS1#0009),2010/12/20 : by BHH-wangxt
// HISTORY = HWSIU,40-05-00-00,2011/05/25 BHH(Lixw)   40-05-00-08(DS1#0008) **
// HISTORY = HWSIU,40-06-00-00,2011/08/25 BHH-GUOJY   40-06-00-01(DS1#0001) **
// HISTORY  = HWSIU,40-06-00-00,2011/08/25 BHH-GUOJY  40-06-00-01(DS1#0002)
// HISTORY = HWSIU,40-09-00-00,2011/10/28 BHH-GUOJY   40-09-00-01(DS1#0001) **
// HISTORY = HWSIU,40-09-01-00,2012/01/13 BHH-liwei   40-09-01-01(UT#00007) **
// History = HWSIU,40-16-05-00,2013/11/28 by BHH-guojy  40-16-05-01(MRT3#001) for UPS т?╣т║?    **
// HISTORY = HWSIU,40-21-00-00,2014/07/22 HISOL(Kuwayama) 40-21-00-00(FS#0001)
// HISTORY = HWSIU,40-21-00-01,2014/08/04 HISOL(Kuwayama) 40-21-00-01(PG#0001)
//////////////////////////////////////////////////////////////////////////////
#ifndef HWSIUDEF_H
#define HWSIUDEF_H

// For Timer
enum EN_TIMER_ID                // TimerID
{
    HEADSENSOR_TIMER            = 1001,                 // Heat sensor      //15-00-05-04
    SEISMICSENSOR_TIMER         = 1002,                 // Seismic sensor   //15-00-05-04
    FANCONTROL_TIMER            = 1003,                 // FAN Control      // STD40-00-00-00
    ENV_INTE_TEMPER_TIMER       = 1004,                 // EnvironmentLog Interval[Temperature]
    ENV_INTE_HUMID_TIMER        = 1005,                 // EnvironmentLog Interval[Humidity]
    ENV_INTE_DUST_TIMER         = 1006,                 // EnvironmentLog Interval[Dust]
    POST_COMPLETE               = 1007,                 //40-09-01-01(UT#00007)
    PROXIMITYSENSOR_TIMER       = 1008,                 //Proximity sensor
};
enum EN_TIMER_TIMEROUT          // Timer time out value
{
    HEADSENSOR_TIMEROUT                 = 100,                  // Heat sensor      //15-00-05-04
    SEISMICSENSOR_TIMEROUT              = 100,                  // Seismic sensor   //15-00-05-04
    PROXIMITYSENSOR_TIMEROUT            = 3000,                 // Proximity sensor //15-00-05-04
    FANCONTROL_TIMEROUT                 = 30 * 1000,            // FAN control time out = 30(sec)   // STD40-00-00-00
    ENV_INTE_TEMPER_DEFAULT_TIMEROUT    = 10 * 60 * 1000,       // EnvironmentLog Interval[Temperature] (Default: 10min)
    ENV_INTE_HUMID_DEFAULT_TIMEROUT     = 10 * 60 * 1000,       // EnvironmentLog Interval[Humidity]    (Default: 10min)
    ENV_INTE_DUST_DEFAULT_TIMEROUT      = 10 * 60 * 1000,       // EnvironmentLog Interval[Dust]        (Default: 10min)
};
// For Environment LOG
enum EN_ENV_LOG_TYPE
{
    EN_ENV_LOG_TEMPERATURE  = 1,        // temperature info // STD40-00-00-00
    EN_ENV_LOG_HUMIDITY,                // Humidity info    // STD40-00-00-00
    EN_ENV_LOG_DUST,                    // Dust info        // STD40-00-00-00
};

//CSTV7001----------------------------------------------------------Begin
#define EVENT_TIMEOUT       1
#define EVENT_FORMATERROR   2
#define EVENT_REJECT        3
#define EVENT_REJECTOVER    4
#define EVENT_ACCEPT        5
#define EVENT_DATA          6

#define POWEROFF_MAXRETRY   1
#define POLING_BASE         100
#define SET_BASE            200
#define POWEROFF_TIME       "045"
#define POWERRESTART_TIME   "0"
#define COMMRETRY_TIME      3
#define COMMRETRY_DELAY     100

#define OPEN                    (0x0011)                                        //40-02-00-16(DS1#0016)
#define RESET                   (0x0013)                                        //40-02-00-16(DS1#0016)

// ---- IOMC command time out ---- STD40-00-00-00
enum EN_IOMC_TIME_OUT
{
    EN_IOMCTOUT_MIN                         = 10,   // Minimum Time out
    //  EN_IOMCTOUT_STOP                        = 10,   // CMD[0x0000]
    EN_IOMCTOUT_VERSION_SENSE               = 10,   // CMD[0x0100]
    EN_IOMCTOUT_DATA_WRITE                  = 60,   // CMD[0x0600]
    EN_IOMCTOUT_RESET                       = 10,   // CMD[0x0700]
    EN_IOMCTOUT_SENSE_STATUS                = 10,   // CMD[0x0C00]
    //  EN_IOMCTOUT_CUSTOMER_SENSE              = 10,   // CMD[0x5000]
    EN_IOMCTOUT_FLICKER_CONTROL             = 10,   // CMD[0x3500]
    EN_IOMCTOUT_GENERAL_IO                  = 10,   // CMD[0xA300]
    EN_IOMCTOUT_STATUS_DISPLAY_LAMP         = 10,   // CMD[0x4A00]
    EN_IOMCTOUT_BACKLIGHT_CONTROL           = 10,   // CMD[0xB300]
    EN_IOMCTOUT_DATA_READ                   = 30,   // CMD[0x0500]
    EN_IOMCTOUT_LOG_GET                     = 40,   // CMD[0x0D00] STD40-00-00-01
    EN_IOMCTOUT_TEMPERATURE_SENSER_READ     = 10,   // CMD[0x1300]
    EN_IOMCTOUT_ENVIRONMENT_SENSER_READ     = 10,   // CMD[0x2400]
    EN_IOMCTOUT_FAN_CONTROL                 = 10,   // CMD[0x1800]
    EN_IOMCTOUT_GET_FAN_RPM                 = 10,   // CMD[0x1900]
    EN_IOMCTOUT_SERIAL_NO_WRITE             = 30,   // CMD[0x1A00]
    EN_IOMCTOUT_SERIAL_NO_READ              = 30,   // CMD[0x1B00]
    EN_IOMCTOUT_BUZZER                      = 30,   // CMD[0x7D00]
    EN_IOMCTOUT_GET_POWERINF                = 10,   // CMD[0x9100]          //40-21-00-00(FS#0001)
    // PDL(IOMC)
    EN_IOMCTOUT_PDL_CHECK                   = 30,   // CMD[0x0300]
    EN_IOMCTOUT_PDL_START                   = 100,  // CMD[0x0301] STD40-00-00-01
    EN_IOMCTOUT_PDL_DATA_TRANSMISSION       = 30,   // CMD[0x0302]
    EN_IOMCTOUT_PDL_END                     = 100,  // CMD[0x0303] STD40-00-00-01

    EN_IOMCTOUT_MAX                         = 60,   // Maximum Time out
};
enum EN_IOMC_CMD
{
    //  EN_IOMC_CMD_STOP                        = 0x0000,   // CMD[0x0000]
    EN_IOMC_CMD_VERSION_SENSE               = 0x0001,   // CMD[0x0100]
    EN_IOMC_CMD_DATA_WRITE                  = 0x0006,   // CMD[0x0600]
    EN_IOMC_CMD_RESET                       = 0x0007,   // CMD[0x0700]
    EN_IOMC_CMD_SENSE_STATUS_IDLE           = 0x000C,   // CMD[0x0C00]
    EN_IOMC_CMD_SENSE_STATUS_CONSTRAINT     = 0x010C,   // CMD[0x0C01]
    //  EN_IOMC_CMD_CUSTOMER_SENSE              = 0x0050,   // CMD[0x5000]
    EN_IOMC_CMD_FLICKER_CONTROL             = 0x0035,   // CMD[0x3500]
    EN_IOMC_CMD_GENERAL_IO                  = 0x00A3,   // CMD[0xA300]
    EN_IOMC_CMD_STATUS_DISPLAY_LAMP         = 0x004A,   // CMD[0x4A00]
    EN_IOMC_CMD_BACKLIGHT_CONTROL           = 0x00B3,   // CMD[0xB300]
    EN_IOMC_CMD_DATA_READ                   = 0x0005,   // CMD[0x0500]
    EN_IOMC_CMD_LOG_GET_NOT_CLEAR           = 0x000D,   // CMD[0x0D00]
    EN_IOMC_CMD_LOG_GET_CLEAR               = 0x010D,   // CMD[0x0D01]
    EN_IOMC_CMD_LOG_GET_ALL_LOG_CLEAR       = 0xFF0D,   // CMD[0x0DFF]
    EN_IOMC_CMD_TEMPERATURE_SENSER_READ     = 0x0013,   // CMD[0x1300]
    EN_IOMC_CMD_ENVIRONMENT_SENSER_READ     = 0x0024,   // CMD[0x2400]
    EN_IOMC_CMD_FAN_CONTROL                 = 0x0018,   // CMD[0x1800]
    EN_IOMC_CMD_GET_FAN_RPM                 = 0x0019,   // CMD[0x1900]
    EN_IOMC_CMD_SERIAL_NO_WRITE             = 0x001A,   // CMD[0x1A00]
    EN_IOMC_CMD_SERIAL_NO_READ              = 0x001B,   // CMD[0x1B00]
    EN_IOMC_CMD_BUZZER                      = 0x017D,   // CMD[0x7D00]
    EN_IOMC_CMD_GET_POWERINF                = 0x0091,   // CMD[0x9100]          //40-21-00-00(FS#0001)
    // PDL(IOMC)
    EN_IOMC_CMD_PDL_CHECK                   = 0x0003,   // CMD[0x0300]
    EN_IOMC_CMD_PDL_START                   = 0x0103,   // CMD[0x0301]
    EN_IOMC_CMD_PDL_DATA_TRANSMISSION       = 0x0203,   // CMD[0x0302]
    EN_IOMC_CMD_PDL_END                     = 0x0303,   // CMD[0x0303]
    // PDL(PowerUnit)
    EN_IOMC_CMD_POW_PDL_CHECK               = 0x0099,   // CMD[0x9900]
    EN_IOMC_CMD_POW_PDL_START               = 0x0199,   // CMD[0x9901]
    EN_IOMC_CMD_POW_PDL_DATA_TRANSMISSION   = 0x0299,   // CMD[0x9902]
    EN_IOMC_CMD_POW_PDL_END                 = 0x0399,   // CMD[0x9903]
};

enum EN_IOMC_CMD_ID
{
    EN_IOMC_CMDID_CMDCODE                   = 0x0100,   // Command code
    EN_IOMC_CMDID_ACT                       = 0x0200,   // ACT
    EN_IOMC_CMDID_CHAINCODE                 = 0x0500,   // Chanin code
    EN_IOMC_CMDID_PDLDAT                    = 0x0600,   // PDL write data
    EN_IOMC_CMDID_TARGET                    = 0x0700,   // Targetting

    EN_IOMC_CMDID_DATETIMESET               = 0x4D07,   // CMD[0x0600]-Date & Time setting
    EN_IOMC_CMDID_FLKCYCLE                  = 0x4007,   // CMD[0x0600]-Filcker Cycle
    EN_IOMC_CMDID_LMCLEDCYCLE               = 0x4807,   // CMD[0x0600]-LMC-LED Cycle
    EN_IOMC_CMDID_STLMPCYCLE                = 0x4907,   // CMD[0x0600]-Status Lamp Cycle
    //  EN_IOMC_CMDID_LMCBZRCYCLE               = 0x4107,   // CMD[0x0600]-LMC buzzer cycle
    EN_IOMC_CMDID_EARPHONEMUTE              = 0x4607,   // CMD[0x0600]-earphone mute setting
    EN_IOMC_CMDID_CUDWNFANCTRL              = 0x5E07,   // CMD[0x0600]-Fan action after CU shutdown.
    EN_IOMC_CMDID_POWERCUT                  = 0x6007,   // CMD[0x0600]-power cut setting

    EN_IOMC_CMDID_SERIALNUM_WRITE           = 0x0107,   // CMD[0x1A00]-Serial No. Write
    EN_IOMC_CMDID_FLKCTRL                   = 0x1107,   // CMD[0x3500]-Flicker control

    EN_IOMC_CMDID_OPL_BACKLIGHT             = 0x4B07,   // CMD[0xB300]-OPL Backlight control
    EN_IOMC_CMDID_SPL_BACKLIGHT             = 0x4C07,   // CMD[0xB300]-SPL Backlight control

    EN_IOMC_CMDID_ENVIRONMENT_SENSER_READ   = 0x0107,   // CMD[0x2400]
    EN_IOMC_CMDID_FAN_CONTROL               = 0x0107,   // CMD[0x1800]
    EN_IOMC_CMDID_ALARMPOWERONTIME          = 0x4507,
    EN_IOMC_CMDID_ALARMFLGENABLE            = 0x5107,
    EN_IOMC_CMDID_POWERINF                  = 0x5407,   // CMD[0x0754]          //40-21-00-00(FS#0001)
    EN_IOMC_CMDID_ALMPOWERON_ENABLE         = 0x2111,   // CMD[0x0754]          //40-21-00-00(FS#0001)
    EN_IOMC_CMDID_ALMPOWERON_SETDATE        = 0x2211,   // CMD[0x0754]          //40-21-00-00(FS#0001)
};

enum EN_IOMC_PACKET_LNG
{
    EN_IOMC_PKTLNG_COMMAND                  = 0x0400,   // Command packet size = 4byte

    EN_IOMC_PKTLNG_SERIALNUM_WRITE          = 0x1C00,   // Serial Nu.Write data packet size = 28byte
    EN_IOMC_PKTLNG_FLKCTRL                  = 0x1200,   // Flicker display data packet size = 18byte
    EN_IOMC_PKTLNG_LCD_BACKLIGHT            = 0x0400,   // OPL/ SPL backlight level set =  4byte
    EN_IOMC_PKTLNG_ENVIRONMENT_SENSER_READ  = 0x0400,   // Environment sensors read size =  4byte

    EN_IOMC_PKTLNG_FAN_CONTROL              = 0x0600,   // OPL/ SPL backlight level set =  4byte
    EN_IOMC_PKTLNG_CUDWNFANCTRL             = 0x0400,   // OPL/ SPL backlight level set =  4byte
    EN_IOMC_PKTLNG_LOG_GET                  = 0x0400,   // LOG GET  =  4byte
    EN_IOMC_PKTLING_EXTEND_CONTROL          = 0x2200,   // Т??у?ет?║т??1-8 = 34byte

};

enum EN_IOMC_LOGGET_ACT
{
    EN_IOMC_LOGGETACT_MTC                   = 0x0002,   // MTC
    EN_IOMC_LOGGETACT_CMNDRES               = 0x0004,   // Command res trace
    EN_IOMC_LOGGETACT_WDT_TIMEOUT           = 0x0006,   // WDT TIMEOUT
    EN_IOMC_LOGGETACT_EEPROM_DUMP           = 0x0010,   // EEPROM dump
    EN_IOMC_LOGGETACT_ACCESSLOG             = 0x0011,   // ACCESS log
    EN_IOMC_LOGGETACT_SELFCHECK             = 0x0013,   // Self Check log
    EN_IOMC_LOGGETACT_MC_ERR                = 0x0020,   // MC Error log

};

enum EN_IOMC_RES_ID
{
    EN_IOMC_MTC_ID                  = 0x8300,
    EN_IOMC_MS_ID                   = 0x8600,   // MS

};



#define DEFVOLUME   600     //с?цс?хс??с??┬ис??┬ис?д┬ис?Ц┬и┬ис??с??с??с?Ф00
#define LNG_CAPSEXTRA    512
#define LNG_STATEXTRA    2048
#define LNG_ERRDETAIL    43   // STD40-00-00-00
#define USB_OPEN_TIMER   10   //60  
#define USB_OPEN_REQSZ   4


#define IOMC_CNTL_SZ                 8
#define IOMC_DATE_DATA_SZ           10
#define IOMC_PDL_REBOOT_TIMER       10000  // с??с?Ъс??с??с??с??┬ис?й┬ис??┬и№╝?
#define IOMC_MTC_SZ                  4
#define IOMC_MTC_CHAR_SZ             4      // 40-00-01-02
#define IOMC_PDL_REBOOT_TIMER_H8    70000   //40-06-00-01(DS1#0001)

//IOMC Flicker comand
#define IOMC_FLK_LED_NOCHANGE       0x00        // for IOMC. Flicker Nothing is done.
#define IOMC_FLK_LED_OFF            0x10        // for IOMC. Flicker off
#define IOMC_FLK_LED_ON             0x11        // for IOMC. Flicker on(continuous)
#define IOMC_FLK_LED_FLASH_CYCLE_1  0x01        // for IOMC. Flicker flash(flash cycle1)
#define IOMC_FLK_LED_FLASH_CYCLE_2  0x02        // for IOMC. Flicker flash(flash cycle2)
#define IOMC_FLK_LED_FLASH_CYCLE_3  0x03        // for IOMC. Flicker flash(flash cycle3)
#define IOMC_FLK_LED_FLASH_CYCLE_4  0x04        // for IOMC. Flicker flash(flash cycle4)
#define IOMC_FLK_LED_FLASH_CYCLE_5  0x05        // for IOMC. Flicker flash(flash cycle5)
#define IOMC_FLK_LED_FLASH_CYCLE_6  0x06        // for IOMC. Flicker flash(flash cycle6)
#define IOMC_FLK_LED_FLASH_CYCLE_7  0x07        // for IOMC. Flicker flash(flash cycle7)
#define IOMC_FLK_LED_FLASH_CYCLE_8  0x08        // for IOMC. Flicker flash(flash cycle8)

#define IOMC_FLK_INDEX_SLOW_ON      0                                           //40-05-00-08(DS1#0008)
#define IOMC_FLK_INDEX_SLOW_OFF     (IOMC_FLK_INDEX_SLOW_ON+1)                  //40-05-00-08(DS1#0008)
#define IOMC_FLK_INDEX_MEDIUM_ON    (IOMC_FLK_INDEX_SLOW_ON+2)                  //40-05-00-08(DS1#0008)
#define IOMC_FLK_INDEX_MEDIUM_OFF   (IOMC_FLK_INDEX_SLOW_ON+3)                  //40-05-00-08(DS1#0008)
#define IOMC_FLK_INDEX_QUICK_ON     (IOMC_FLK_INDEX_SLOW_ON+4)                  //40-05-00-08(DS1#0008)
#define IOMC_FLK_INDEX_QUICK_OFF    (IOMC_FLK_INDEX_SLOW_ON+5)                  //40-05-00-08(DS1#0008)

//С╝аТёЪт?е Т??у?ет?║т??1-8
#define IOMC_SENS_CONTINUOUS        0x00         // Output continue
#define IOMC_SENS_OFF               0x02         // Output stop
#define IOMC_SENS_ON                0x01         // Output begin



//IOMC Т??у?ет?║т??1-8 device
#define IOMC_SENSOR_ID_FC        4  // ж?▓ж??ж?╝ТБ?Тх?т??тЁ?С╝аТёЪт?е               = Т??у?ет?║т?? 5
#define IOMC_SENSOR_ID_SO        2  // т╝?ж?еТБ?Тх?т??Т??т??т?║т╝?ж?еС┐Ат?и           = Т??у?ет?║т?? 3
#define IOMC_SENSOR_ID_SC        5  // тЁ│ж?еТБ?Тх?т??Т??т??т?║тЁ│ж?еС┐Ат?и           = Т??у?ет?║т?? 6
#define IOMC_SENSOR_ID_LA        1  // Т??Т??Тю║СИ?у?хС┐Ат?и                     = Т??у?ет?║т?? 2

//IOMC Environment Sensor comand
#define IOMC_ENV_SNS_TYPE_TEMPER    0x00;   // Temperature      STD40-00-00-00
#define IOMC_ENV_SNS_TYPE_HUMIDITY  0x01;   // Humidity sensor  STD40-00-00-00
#define IOMC_ENV_SNS_TYPE_DUST      0x02;   // Dust sensor      STD40-00-00-00

//IOMC InitialSet umcomplete Error //STD40-00-01-01
#define IOMC_SYOKIERR_WC_SETPORTS               "1101001"   // с?Юс?╝с??у?ХТЁ?УеГт??т?ду??
#define IOMC_SYOKIERR_WC_SETAUXILIARY           "1101002"   // с?Юс?╝с??у?ХТЁ?УеГт??т?ду??
#define IOMC_SYOKIERR_WC_SETGUIDLIGHT           "1101003"   // с?гс?цс??с?Ес?цс??УеГт??т?ду??
#define IOMC_SYOKIERR_WC_RESET                  "1101004"   // с?фс?╗с??с??с?│с?ъс?│с??
#define IOMC_SYOKIERR_VAP_SELFCHECK             "1101005"   // с?╗с?Фс??с??с?Дс??с??т?ду??№╝?Т?Ат╝хс?│с?ъс?│с??№╝?
#define IOMC_SYOKIERR_VAP_SET_TRACEABILITY      "1101006"   // с??с?гс?╝с?хс??с?фс??с?БТ?Ёта?УеГт??т?ду??№╝?Т?Ат╝хс?│с?ъс?│с??№╝?
#define IOMC_SYOKIERR_VAP_GET_TRACEABILITY      "1101007"   // с??с?гс?╝с?хс??с?фс??с?БТ?Ёта?т??тЙ?т?ду??№╝?Т?Ат╝хс?│с?ъс?│с??№╝?
#define IOMC_SYOKIERR_VAP_OUTPUT_ACCESSLOG      "1101008"   // с?бс??с?╗с?╣с?Гс??т?║т??т?ду??№╝?Т?Ат╝хс?│с?ъс?│с??№╝?
#define IOMC_SYOKIERR_VAP_STORE_LOGGING         "1101009"   // ух?Уе?с?Гс??УеГт??№╝?Т?Ат╝хс?│с?ъс?│с??№╝?
#define IOMC_SYOKIERR_VAP_CLEAR_LOGGING         "110100A"   // ух?Уе?с?Гс??с??с?фс?б№╝?Т?Ат╝хс?│с?ъс?│с??№╝?
#define IOMC_SYOKIERR_VAP_UPDATE_LOGDATA        "110100B"   // ух?Уе?с?Гс??Т?┤Т??№╝?Т?Ат╝хс?│с?ъс?│с??№╝?
#define IOMC_SYOKIERR_VAP_SET_BRIGHTNESS        "110100C"   // У╝Ют║дУеГт??№╝?Т?Ат╝хс?│с?ъс?│с??№╝?   //40-01-00-04(DS1#0004)
#define IOMC_SYOKIERR_SET_AUTOSTARTUPTIME       "110100D"   // У?фт??Ухит??Т??ж??УеГт??т?ду??
#define IOMC_SYOKIERR_WC_SETINDICATOR           "110100E"   // с?Юс?╝с??у?ХТЁ?УеГт??т?ду??

//IOMC Fan Control comand
#define  IOMC_FAN_CONT  0x00    // FAN continue value for IOMC parameter
#define  IOMC_FAN_STOP  0x01    // FAN stop value for IOMC parameter
#define  IOMC_FAN_MOVE  0x02    // FAN move value for IOMC parameter

#define TEMPERA_MAX  127            //MAX Temperature STD40-00-00-00
#define TEMPERA_MIN  -127           //MIN Temperature STD40-00-00-00

// с?╗с?│с?хТ?Ёта?
// index(Т?Цт?бс?╗с?│с?╣Т?Ёта?)
#define INDEX_OPERATOR_SW  0        // С┐?т?Ас?Гс?╝ STD40-00-00-00
#define INDEX_SILENTALRM_SW 0       // с?хс?цс?гс?│с??с?бс?Ес?╝с?аSTD40-00-00-00
#define INDEX_SILENTALRM_SW_LATCH 2                                             //40-02-00-09(DS1#0009)
#define INDEX_SEIS_SENS    0        // Т??т??ТцюуЪЦ STD40-00-00-00
#define INDEX_SEIS_SENS_LATCH 2                                                 //40-02-00-09(DS1#0009)
#define INDEX_HEAT_SENS    0        // ТИЕт║д STD40-00-00-00
#define INDEX_HEAT_SENS_LATCH 2                                                 //40-02-00-09(DS1#0009)
#define INDEX_FRONT_DOOR   1        // т??Т?? STD40-00-00-00
#define INDEX_REAR_DOOR    1        // тЙ?Т?? STD40-00-00-00
#define INDEX_VDM_SW       2        // жАДт?бТцюуЪЦ STD40-00-00-00
#define INDEX_VDM_SW_LATCH 2                                                    //40-02-00-09(DS1#0009)
#define INDEX_UNIT_SW      3        // с?дс??с??с??т??Сй?уй? STD40-00-00-00
// index(С╗?т?ас?╗с?│с?╣Т?Ёта?)
#define INDEX_HEADSET_SENS   0      // с?цс?цс??с?Ес?│Т?┐тЁЦ STD40-00-00-00
#define INDEX_MCU_SENS       2      // с?Фс?╝с??т?БСИ?ТГБТцюуЪЦ STD40-00-00-00
#define INDEX_MCU_SENS_LATCH 4                                                  //40-02-00-09(DS1#0009)
#define INDEX_PROM_SENS      2      // С║║Сй?ТёЪуЪЦ STD40-00-00-00
#define INDEX_PROM_SENS_LATCH 4                                                 //40-02-00-09(DS1#0009)
#define INDEX_FSCK_SENS      2      // ж??ж?╝ТБ?Тх?(т??тЁ?) STD40-00-00-00
#define INDEX_FSCK_SENS_LATCH 4                                                 //40-02-00-09(DS1#0009)
#define INDEX_HDDE_SENS 2           // Т??ж?еТБ?уЪЦ STD40-00-00-00
#define INDEX_HDDE_SENS_LATCH 4                                                 //40-02-00-09(DS1#0009)
#define INDEX_SUDO_SENS 2           // т╝?ж?ет??Т??ТБ?Тх? STD40-00-00-00
#define INDEX_SUDO_SENS_LATCH 4                                                 //40-02-00-09(DS1#0009)
#define INDEX_SUDC_SENS 2           // тЁ│ж?ет??Т??ТБ?Тх? STD40-00-00-00
#define INDEX_SUDC_SENS_LATCH 4                                                 //40-02-00-09(DS1#0009)
#define INDEX_POWERFAIL_SENS 6      // ACт?юж?╗ STD40-00-00-00
#define INDEX_POWERFAIL_SENS_LATCH  6                                           //40-02-00-09(DS1#0009)
#define INDEX_POWER_SW       6      // ж?╗Т║?SWс?фс?? STD40-00-00-00
#define INDEX_POWERFAN       6      // ж?╗Т║?с??с?Ас?│ STD40-00-00-00
#define INDEX_LMC_SW         7      // LMC Switch1 STD40-00-00-00
#define INDEX_CABINET_DOORS_ERROR  2// т??тЙ?Т??у??тИИж??ж??                          //40-33-05-01(FS#0001)
#define INDEX_SAFE_DOOR_ERROR  2    // ж??т║ФТ??у??тИИж??ж??                         //40-33-05-01(FS#0001)

//IOMC Flicker device
#define IOMC_FLK_ID_FL_00        0
#define IOMC_FLK_ID_FL_01        1
#define IOMC_FLK_ID_FL_02        2
#define IOMC_FLK_ID_FL_03        3
#define IOMC_FLK_ID_FL_PB        4  // Passbook Slot Lamp                 = Flicker 5
#define IOMC_FLK_ID_FL_BAR       5  // Т??Т??Тю║Т??уц║у??                       = Flicker 6
#define IOMC_FLK_ID_FL_06        6
#define IOMC_FLK_ID_FL_07        7
#define IOMC_FLK_ID_LMP_CS       8  // Cash Slot Flicker Lamp             = Flicker 9
#define IOMC_FLK_ID_LMP_EPP      9  // EPP Lamp/Passbook FlickerLamp      = Flicker 10
#define IOMC_FLK_ID_ICRW_FLK    10  // т╝║т?Хж??т?АТ??уц║у??                     = Flicker 11
#define IOMC_FLK_ID_FL_RT       11  // Receipt Slot Flicker Lamp          = Flicker 12
#define IOMC_FLK_ID_FL_CS       12  // У║ФС╗йУ??Т??уц║у??                         = Flicker 13        
#define IOMC_FLK_ID_FL_CD       13  // Card Slot Flicker Lamp             = Flicker 14
#define IOMC_FLK_ID_FL_14       14

// bit
// INDEX(Т?Цт?бс?╗с?│с?╣Т?Ёта?) 0
#define MASK_OPERATOR_SW        0x02        // С┐?т?Ас?Гс?╝ STD40-00-00-00
#define MASK_SILENTALRM_SENS    0x20    // с?хс?цс?гс?│с??с?бс?Ес?╝с?аSTD40-00-00-00
#define MASK_SILENTALRM_SENS_LATCH 0x20                                         //40-02-00-09(DS1#0009)
#define MASK_SEIS_SENS          0x40        // Т??т??ТцюуЪЦ STD40-00-00-00
#define MASK_SEIS_SENS_LATCH    0x40                                                //40-02-00-09(DS1#0009)
#define MASK_HEAT_SENS          0x80        // ТИЕт║д STD40-00-00-00
#define MASK_HEAT_SENS_LATCH    0x80                                                //40-02-00-09(DS1#0009)
// INDEX(Т?Цт?бс?╗с?│с?╣Т?Ёта?) 1
#define MASK_REAR_DOOR          0x01        // тЙ?Т?? STD40-00-00-00
#define MASK_FRONT_DOOR         0x04        // т??Т?? STD40-00-00-00

// INDEX(Т?Цт?бс?╗с?│с?╣Т?Ёта?) 2
#define MASK_VDM_SW       0x08      // жАДт?бТцюуЪЦ STD40-00-00-00
#define MASK_VDM_SW_LATCH 0x10                                                  //40-02-00-09(DS1#0009)
// INDEX(Т?Цт?бс?╗с?│с?╣Т?Ёта?) 3
#define MASK_SAFE_DOOR    0x01      // ж??т║ФТ??т??Сй?уй? STD40-00-00-00
#define MASK_SMJ_SW       0x04      // JPR/ SPR/ MCUт??Сй?уй? STD40-00-00-00
#define MASK_PASSBOOK_SENS 0x40     // PASSBOOK т??Сй?уй? STD40-00-00-00
#define MASK_PASSBOOK_DOOR 0x80     // PASSBOOK Т?? STD40-00-00-00

// INDEX(С╗?т?ас?╗с?│с?╣Т?Ёта?) 0
#define MASK_HEADSET_SENS 0x10      // с?цс?цс??с?Ес?│Т?┐тЁЦ STD40-00-00-00

// INDEX(С╗?т?ас?╗с?│с?╣Т?Ёта?) 2
#define MASK_MCU_SENS          0x40// с?Фс?╝с??т?БСИ?ТГБТцюуЪЦ STD40-00-00-00
#define MASK_MCU_SENS_LATCH    0X40                                            //40-02-00-09(DS1#0009)
#define MASK_PROM_SENS         0x02// С║║Сй?ТёЪуЪЦSTD40-00-00-00
#define MASK_PROM_SENS_LATCH   0x02                                            //40-02-00-09(DS1#0009)
#define MASK_FSCK_SENS         0x10// ж??ж?╝ТБ?Тх?(т??тЁ?) STD40-00-00-00
#define MASK_FSCK_SENS_LATCH   0x10                                            //40-02-00-09(DS1#0009)
#define MASK_HDDE_SENS         0x20// Т??ж?еТБ?уЪЦ STD40-00-00-00
#define MASK_HDDE_SENS_LATCH   0x20                                            //40-02-00-09(DS1#0009)
#define MASK_SUDO_SENS         0x04// т╝?ж?ет??Т??ТБ? STD40-00-00-00
#define MASK_SUDO_SENS_LATCH   0x04                                            //40-02-00-09(DS1#0009)
#define MASK_SUDC_SENS         0x01//тЁ│ж?ет??Т??ТБ?Тх? STD40-00-00-00
#define MASK_SUDC_SENS_LATCH   0x01                                            //40-02-00-09(DS1#0009)

#define MASK_CABINET_DOOR_ERROR 0x08// т??тЙ?Т??у??тИИж??                              //40-33-05-01(FS#0001)
#define MASK_SAFE_DOOR_ERROR    0X04// ж??т║ФТ??у??тИИж??                             //40-33-05-01(FS#0001)

// INDEX(С╗?т?ас?╗с?│с?╣Т?Ёта?) 6
#define MASK_POWER_SW       0x01    // ж?╗Т║?SWс?фс?? STD40-00-00-00
#define MASK_POWERFAIL_SENS 0x80    // ACт?юж?╗ STD40-00-00-00                  //40-02-00-09(DS1#0016)
#define MASK_POWERFAIL_SENS_LATCH   0x40                                        //40-02-00-09(DS1#0016)

// INDEX(С╗?т?ас?╗с?│с?╣Т?Ёта?) 7
#define MASK_LMC_SW         0x01    // LMC Switch1 STD40-00-00-00
#define MASK_LMC_LATCH_SENS 0x02    // LMC Switch1(с?Ес??с??) STD40-00-00-00

//#define SIU_USB_OPEN_ERROR       1 // USB┬ис??┬ис??с??┬ис?Ъ┬ис?Дс??с?и┬ис?Е┬иУЪ?Ь?ёТ??С╝?
//#define SIU_USB_DRIVER_ERROR     2 // USB┬ис??┬ис??с??┬ис?ЪР??с?Гс?╗?
//#define SIU_IOMC_RESPONSE_ERROR  3 // IOMC┬иУ??с?ЁУ?ёс?ЁУЪ?с??┬ис??Р??с?Гс?╗?

// с??с??с?иТг╣тЪ?Р??┬ис??┬ис??с??с?и┬ис?й┬ис??
#define    NORMALPROC              0       // с??с??с?╗ж?│тЪ?Р??с??с??┬ис??┬ис?д┬ис?Ц┬иТ?┐с?ЁТ??№╝Ё?
#define    PDLREBOOT               2       // с??с?Ъс??с??с??с??┬ис??с?ес??с?и┬ис??с?╣с???

//с?Гс??Т?Ёта?
#define IOMC_ACCESSLOG_MAX_SIZE    128*1024
#define IOMC_SELFCHECKLOG_MAX_SIZE 64*1024
#define IOMC_MTCLOG_MAX_SIZE       64*1024
#define IOMC_COMRES_MAX_SIZE       64*1024
#define IOMC_WDTTIMEOUT_MAX_SIZE   64*1024
#define IOMC_EEPROMDUMP_MAX_SIZE   64*1024
#define IOMC_MCERRLOG_MAX_SIZE   64*1024

//Log Getс?│с?ъс?│с??
#define IOMC_LOG_GET_MAX_PACKET_SIZE    4096




// №╝?№╝ц№╝гж?бж?Б
// №╝?№╝ц№╝гТ?Ис??УЙ╝с?┐с??с?╝с?┐Тю?тцДс?хс?цс?║                     // STD40-00-00-00
#define     PDL_WRITEDATASIZE_MAX    4096            // STD40-00-00-00

#define     PDL_INFO_FILE_NAME   "IO_SETUP.INI"  // STD40-00-00-00

#define Rsp_PDLbu_Dousachuu         0x40    // №╝Г№╝│№╝?(1с??с?цс??у??)№╝?№╝?№╝ц№╝гж?ет??СйюСИГ    //STD40-00-00-00

// №╝?№╝ц№╝гс?фс??с?Ес?цт?ъТ??                                                                  //
#define    PDL_RETRY_KAISUU            1                                               //STD40-00-00-00

// №╝?№╝ц№╝гж?бж?Бт??уЙЕ
#define    PDL_FIRST_BLOCK        0xE000       // PDLуе?т?Ц№╝?тЁ?жаГ  // STD40-00-00-00
#define    PDL_MIDDLE_BLOCK       0xE001       // PDLуе?т?Ц№╝?СИГж??  // STD40-00-00-00
#define    PDL_LAST_BLOCK         0xEFFF       // PDLуе?т?Ц№╝?Тю?ух?  // STD40-00-00-00
#define    PDL_ADDRESS_VALID      0x0000       // с?бс??с?гс?╣Т??т??с??с?Ес??№╝?Т?Ис??УЙ╝с?┐тЁ?жаГс?бс??с?гс?╣Тю?т?╣ // STD40-00-00-00
#define    PDL_ADDRESS_INVALID    0xFFFF       // с?бс??с?гс?╣Т??т??с??с?Ес??№╝?Т?Ис??УЙ╝с?┐тЁ?жаГс?бс??с?гс?╣уёАт?╣ // STD40-00-00-00

#define wVxDIDIS    0x3106
#define wAPINo      0x0001
#define wSTID       RAS_HWSIU

// с?фс??с?иж?│т║ГТ?╗У?Ю?
#define KYO_HT2845   1
#define KYO_HT2845CN 2  // 2003.01.23. ADD
#define KYO_HT2845WL 3  // 2003.01.23. ADD
#define KYO_HT2845S  4  // 2004.04.14. ADD
#define KYO_HT2845V  5
#define KYO_HTCZ5000    6                                       //13-00-00-03   
#define KYO_HT2845W  7                                          //15-00-00-14  
#define KYO_TSEA45   8                                          // 30-00-00-00 

//т??Сй?уй?т??С╣?                                                   //13-00-00-03
#define WFS_SIU_SENSORS_JPR     WFS_SIU_SENSORS_MAX             //13-00-00-03
#define WFS_SIU_SENSORS_SPR     WFS_SIU_SENSORS_MAX-1           //13-00-00-03
#define WFS_SIU_SENSORS_MCU     WFS_SIU_SENSORS_MAX-2           //13-00-00-03
#define WFS_SIU_SENSORS_VDMSW   WFS_SIU_SENSORS_MAX-3           //15-00-03-27
//-----------------------------------------------------------------------

//-----------------------------------------------------------------------

// SPLс??с?Цс??с?ф(с??Р??с??с??с?фс??/с??с??с??с?фс??с??с?╣с??с??)
#define KYO_REAR     0  // SPLс??с?фс??с??с??с??с??с?фс??с??с??
#define KYO_FRONT    1  // SPLс??с?Цс??с??с??Р??с??с??с?фс??с??с??
//#define KYO_UNKNOWN  255// SPLс??с?Цс??с??с??Р??с??с??с??с??с??с??с?цс??с?┐с??с??

#define NORMAL_END "0000000"

#define  MAX_VOL        (DWORD)65535        // с?цс?хс?┐с?и┬ис??┬ис??с??с?и┬ис??с??с?│с??с?ёс??с??

#define SIU_VRT_COUNT    9  // ┬ис?Ъс??с?и┬ис??┬ис??Уъ?У┐ЮТ?╗С╝?№╝?Т?╗у??Т?╗тИиТ?╗Тх?у┤Дс?╣у??у?│с??┬?Р??с??с??с?е
#define SIU_VRT_DETAIL  10  // ┬ис?Ъс??с?и┬ис??┬ис??Уъ?У┐ЮТ?╗С╝?№╝?Т?╗у??Т?╗тИиТ?╗Тх?У??Т??тъас??с?ес??с??у??у?│с??┬?Р??с??с??с?е

// USBТЪ┤жЁ?ж?фУй┤т??жЁ? с??ж?ф-т?цжИБТЪ┤жЁ?
static  const   long    USB_RtnCd_OK1          = 0x00000000 ;   // с?┐с?фс?╗ж?│Т?╗л?ж?Й?
static  const   long    USB_RtnCd_OK2          = 0x33000000 ;   // с?┐с?фс?╗ж?│Т?╗л?ж?Й?
static  const   long    USB_RtnCd_MSK_ERRLVL   = 0xFF000000 ;   // с??ж?фжИБт?║УхджЁ?т?║уЁ?Т╝?тц?с??с??
static  const   long    USB_RtnCd_MSK_WinAK1   = 0x20000000 ;   // Win32API/AK1уЁ?Т╝?тц?с??с??


// qJyс?Ас?ф0|с?йс?БSCOMqУ??yУ?Юu`|{yqd{т?цynqжЁ?zjxУ??qт?АzТГ╗|с?Цс?АУ??qСИ?qЬ?йУ??qТЪ┤qс?бс??
#define  VXDPOST_HWSIU_PNC                     AP_MSG + 16199      //VXDPOST_HWSIU_PNC
#define  VXDPOST_HWSIU_TIMEOUT                 AP_MSG + 16198      //VXDPOST_HWSIU_TIMEOUT
#define  VXDPOST_HWSIU_PIO                     AP_MSG + 16197      //VXDPOST_HWSIU_PIO
#define  VXDPOST_HWSIU_USB                     AP_MSG + 16196      //VXDPOST_HWSIU_USB
#define  VXDPOST_HWSIU_PDLCHECK                AP_MSG + 16195      //VXDPOST_HWSIU_PDLCHECK
#define  VXDPOST_HWSIU_PDLSTART                AP_MSG + 16194      //VXDPOST_HWSIU_PDLSTART
#define  VXDPOST_HWSIU_PDLSEND                 AP_MSG + 16193      //VXDPOST_HWSIU_PDLSEND
#define  VXDPOST_HWSIU_PDLEND                  AP_MSG + 16192      //VXDPOST_HWSIU_PDLEND

#define  IOMC_DEVICE_NAME                  "IOMC"      //IOMC device name STD40-00-00-00
#define  PWRSPLY_DEVICE_NAME               "PWRSPLY"   //PWRSPLY device name    //40-21-00-01(PG#0001)
#define  DEF_EP_FILESUU               10       //IOMC EPс??File STD40-00-00-00


#define    SARCHKEY_SIZ     64   //STD40-00-00-00

//ух?Уе?с?Гс??с?│с??ID
#define  ETC_IOMC_SUPPLY_POWER          1101       //IOMCж?╗Т║?СЙ?ухдТ??ж?? STD40-00-00-07
#define  ETC_IOMC_POWER_ON              1102       //ж?╗Т║?Т??тЁЦТ??ж?? STD40-00-00-07
//40-06-00-01(DS1#0002) #define  ETC_FAN_ACT                    1103       //с??с?Ас?│с??т??СйюТ??ж??с??у┤?Уе?с??с?? 40-03-00-06(DS1#0006)

//с?цс??с?│с??с?Гс??
#define TASKNAME                "HWSIU"                             //STD40-00-00-06

#define ERRMSG_USB_EPDOWN       "EP DOWN"                           //STD40-00-00-06
#define ERRMSG_USB_LPDOWN       "LP DOWN"                           //STD40-00-00-06
#define ERRMSG_USB_TIMEOUT      "COMMAND TIME OUT"                  //STD40-00-00-06
#define ERRMSG_RCVDATA_ERR      "Receive Data error"                //STD40-00-00-06

#define ERRMSG_PDL_FAILURE          "PDL FAILURE"                   //STD40-00-00-06
#define ERRMSG_PDL_FILE_NOT_EXIST   "PDL FILE NOT EXIST"            //STD40-00-00-06
#define ERRMSG_ACCESS_LOG_OPEN_FAIL "ACCESS LOG FILE OPEN FAIL"     //STD40-00-00-06
#define ERRMSG_ACCESS_LOG_WRITE_FAIL "ACCESS LOG FILE WRITE FAIL"   //STD40-00-00-06
#define ERRMSG_REGEDIT_CREATE_FAIL   "REGEDIT_CREATE_FAIL"                  //40-09-00-01(DS1#0001)
#define ERRMSG_REGEDIT_DATA_SET_FAIL "REGEDIT DATA SET FAIL:BOARDVERSION"   //40-09-00-01(DS1#0001)
#define ERRMSG_WFS_SIU_POWERING      "SIU UPS POWERING"                     //40-16-05-01(MRT3#001)
#define ERRMSG_WFS_SIU_LOW           "SIU UPS LOW"                          //40-16-05-01(MRT3#001)

#define CHANNEL_0                       0                                   //40-21-00-01(PG#0001)
#define CHANNEL_1                       1                                   //40-21-00-01(PG#0001)

#endif

