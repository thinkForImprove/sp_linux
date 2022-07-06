#ifndef CARDREADER_ERROR_H
#define CARDREADER_ERROR_H

//读卡器错误码
#define CR_ERROR_ERROR_COMMAND            -101  // 00 读卡器发送错误命令
#define CR_ERROR_COMMAND_SEQUENCE_ERROR   -102  // 01 读卡器发送命令序列出错
#define CR_ERROR_COMMAND_DATA_ERROR       -103  // 02 读卡器命令参数出错
#define CR_ERROR_WRITE_TRACK_ERROR        -104  // 03 读卡器指定磁道未被设置写数据
#define CR_ERROR_RECEIVE_BUFFER_OVERFLOW  -105  // 04 读卡器接收缓冲区满（不支持）

#define CR_ERROR_JAM                      -106  // 10 读卡器堵卡
#define CR_ERROR_SHUTTER_ERROR            -107  // 11 读卡器门异常
#define CR_ERROR_SENSOR_ERROR             -108  // 12 读卡器传感器异常（不支持）
#define CR_ERROR_MOTOR_ERROR              -109  // 13 读卡器主机异常
#define CR_ERROR_CARD_DRAWNOUT            -110  // 14 卡处理时被拖出
#define CR_ERROR_CARD_JAM_IN_RETRIEVING   -111  // 15 读卡器回收时堵卡
#define CR_ERROR_JAM_IN_REAR_END          -112  // 16 卡堵在尾端与接触部位的连接处
#define CR_ERROR_BPI_ENCODER_ERROR        -113  // 17 读卡器 75bpi-encoder 异常（不支持）
#define CR_ERROR_POWER_DOWN               -114  // 18 读卡器断电
#define CR_ERROR_NEED_RESET               -115  // 19 读卡器需要初始化
#define CR_ERROR_ACT_ERROR                -116  // 1E 读卡器ACT错误
#define CR_ERROR_ACT_ERROR2               -117  // 1F 读卡器ACT错误
#define CR_ERROR_ACT_ERROR3               -118  // 1G 读卡器ACT错误（ACT echo is detected）
#define CR_ERROR_ACT_ERROR4               -119  // 1H 读卡器需要初始化（HOST WDT超时）

#define CR_ERROR_LONG_CARD                -120  // 20 读卡器检测到卡太长，门不能被关闭
#define CR_ERROR_SHORT_CARD               -121  // 21 读卡器检测到卡太短

#define CR_ERROR_POSITION_CHANGE          -122  // 32 读卡器检测到卡位置改变
#define CR_ERROR_EEPROM_ERROR             -123  // 33 读卡器闪存数据错误
#define CR_ERROR_NO_STRIPE_ERROR          -124  // 34 读卡器没有检测到磁条卡

#define CR_ERROR_READ_ERROR_IN_SS         -125  // 40 读卡器读错误（SS错误）
#define CR_ERROR_READ_ERROR_IN_ES         -126  // 41 读卡器读错误（ES错误）
#define CR_ERROR_READ_ERROR_IN_VRC        -127  // 42 读卡器读错误（VRC错误）
#define CR_ERROR_READ_ERROR_IN_LRC        -128  // 43 读卡器读错误（LRC错误）
#define CR_ERROR_NODETECT_MAGNETIC_STRIPE -129  // 44 读卡器读错误（无编码）
#define CR_ERROR_READ_ERROR_IN_SS_ES_LRC  -130  // 45 读卡器读错误（无数据）
#define CR_ERROR_READ_JITTER_ERROR        -131  // 46 读卡器读错误（抖动错误）
#define CR_ERROR_TRACK_READING_ERROR      -132  // 49 读卡器读磁道设置错误（指定的磁道未被读取）

#define CR_ERROR_WRITE_ERROR_IN_SS        -133  // 50 读卡器写错误（SS错误）  
#define CR_ERROR_WRITE_ERROR_IN_ES        -134  // 51 读卡器写错误（ES错误）
#define CR_ERROR_WRITE_ERROR_IN_VRC       -135  // 52 读卡器写错误（VRC错误）
#define CR_ERROR_WRITE_ERROR_IN_LRC       -136  // 53 读卡器写错误（LRC错误）
#define CR_ERROR_WRITE_ERROR_NO_ENCODE    -137  // 54 读卡器写错误（无编码）
#define CR_ERROR_WRITE_ERROR_VERIFICATION -138  // 55 读卡器写错误（数据不一致）
#define CR_ERROR_WRITE_ERROR_IN_JITTER    -139  // 56 读卡器写错误（抖动错误）
#define CR_ERROR_WRITE_ERROR_MAGNETIC_LEVEL -140  // 5B 读卡器写错误（磁等级错误）

#define CR_ERROR_CARD_TAKEOUT_RETRIEVEDED -141  // 60 读卡器超时错误（重进卡无法完全执行，卡可能被拖走）
#define CR_ERRORINSERT_TIMEOUT            -142  // 61 读卡器超时错误（等待插入时间溢出）
#define CR_ERROR_TAKEOUT_TIMEOUT          -143  // 62 读卡器超时错误（等待取卡时间溢出）
#define CR_ERROR_RETRIEVE_TIMEOUT         -144  // 63 读卡器超时错误（回收卡时间溢出）
#define CR_ERROR_CARD_HELD_BY_FORCE       -145  // 64 读卡器超时错误（初始化时检测到门口有卡）

#define CR_ERROR_FW_ERROR                 -146  // 70 读卡器固件错误（不完整的程序）
#define CR_ERROR_FW_ERROR2                -147  // 71 读卡器固件错误（在下载完成后复位初始化等待中，即未收到初始化命令）

#define CR_ERROR_ICC_CONTROL_BOARD_ERROR  -148  // 80 读卡器ICC错误（IC卡无法接收）
#define CR_ERROR_ICC_SOLENOID_ERROR       -149  // 81 读卡器ICC错误（IC卡螺线管错误）
#define CR_ERROR_ICC_ACTIVE_ERROR         -150  // 82 读卡器ICC错误（IC卡激活错误）
#define CR_ERROR_ICC_DEACTIVE_ERROR       -151  // 83 读卡器ICC错误（IC卡反激活错误）（不支持）
#define CR_ERROR_ICC_COMMUNICATION_ERROR  -152  // 84 读卡器ICC错误（IC卡通信错误）
#define CR_ERROR_ICC_FORCED_COMMAND       -153  // 85 读卡器ICC错误（IC卡接收时被强制中断）
#define CR_ERROR_ICC_RECEIVE_DATA_ERROR   -154  // 86 读卡器ICC错误（IC卡接收数据错误）
#define CR_ERROR_ICC_ATR_DATA_ERROR       -155  // 87 读卡器ICC错误（IC卡不支持，ATR数据不支持）
#define CR_ERROR_ICC_CARD_MOVEMENT        -156  // 88 读卡器ICC错误（IC卡在压下IC触点时被移动）
#define CR_ERROR_ICC_DIS_OF_VER_CODE      -157  // 89 读卡器ICC错误（IC卡验证码不一致）
#define CR_ERROR_ICC_INAP_VER_CARD        -158  // 8A 读卡器ICC错误（IC卡不适合验证卡）

#define CR_ERROR_SAM_ERROR                -159  // A0 读卡器SAM错误（SAM卡无法接收）
#define CR_ERROR_SAM_ERROR2               -160  // A2 读卡器SAM错误（SAM卡激活错误）
#define CR_ERROR_SAM_ERROR3               -161  // A4 读卡器SAM错误（SAM卡通信错误）
#define CR_ERROR_SAM_ERROR4               -162  // A5 读卡器SAM错误（SAM卡接收时被强制中断）
#define CR_ERROR_SAM_ERROR5               -163  // A6 读卡器SAM错误（SAM卡接收数据错误）
#define CR_ERROR_SAM_ERROR6               -164  // A7 读卡器SAM错误（SAM卡不支持，ATR数据不支持）
#define CR_ERROR_SAM_ERROR7               -165  // A9 读卡器SAM错误（SAM卡芯片被拔出）

#define CR_ERROR_EASD_ERROR               -166  // K0 读卡器E-ASD错误（E-ASD功能被损坏）

#endif