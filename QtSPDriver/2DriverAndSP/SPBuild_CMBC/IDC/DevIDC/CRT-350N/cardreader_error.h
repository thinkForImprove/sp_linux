#ifndef CARDREADER_ERROR_H
#define CARDREADER_ERROR_H

//读卡器错误码
#define CR_ERROR_UNDEFINED_COMMAND               -101  // 00 读卡器接收到没有定义的命令
#define CR_ERROR_COMMAND_DATA_ERROR              -102  // 01 读卡器命令参数错误
#define CR_ERROR_COMMAND_UNABLE_EXEC             -103  // 02 读卡器命令无法执行（如在读卡器内无卡时执行读卡命令）
#define CR_ERROR_HARDWARE_UNAVAILABLE            -104  // 03 读卡器执行命令需要的功能(硬件)不可用或有故障
#define CR_ERROR_DATA_ERROR_IN_CMD               -105  // 04 读卡器命令中的数据错误
#define CR_ERROR_NO_RELEASE_CONTACT_MOVE         -106  // 05 移动卡时没有释放触点
#define CR_ERROR_NO_KEY                          -107  // 06 没有加密功能需要的密钥

#define CR_ERROR_JAM                             -108  // 10 读卡器堵卡
#define CR_ERROR_SHUTTER_ERROR                   -109  // 11 读卡器门异常
#define CR_ERROR_SENSOR_ERROR                    -110  // 12 读卡器传感器异常
#define CR_ERROR_LONG_CARD                       -111  // 13 读卡器检测到卡太长，门不能被关闭
#define CR_ERROR_SHORT_CARD                      -112  // 14 读卡器检测到卡太短
#define CR_ERROR_WRITE_FRAM_ERROR                -113  // 15 读卡器FRAM错误
#define CR_ERROR_POSITION_CHANGE                 -114  // 16 卡位置移动
#define CR_ERROR_CARD_JAM_IN_RETRIEVING          -115  // 17 重进卡时卡堵塞
#define CR_ERROR_SW1_SW2_ERROR                   -116  // 18 SW1, SW2 错误
#define CR_ERROR_CARD_NO_INSERTED_BACK           -117  // 19 卡没有从后端插入

#define CR_ERROR_READ_TRACK_PARITY_ERROR         -118  // 20 读磁卡错误 (奇偶校验错)
#define CR_ERROR_READ_TRACK_ERROR                -119  // 21 读磁卡错误
#define CR_ERROR_WRITE_TRACK_ERROR               -120  // 22 写磁卡错误
#define CR_ERROR_READ_NO_DATA_ERROR              -121  // 23 读磁卡错误 (没有数据内容，只有 STX 起始符，ETX 结束符和 LRC)
#define CR_ERROR_READ_NO_TRACK                   -122  // 24 读磁卡错误 (没有磁条或没有编码-空白轨道)
#define CR_ERROR_WRITE_CHECK_ERROR               -123  // 25 写磁卡校验错误 (品质错误)
#define CR_ERROR_READ_ERROR_IN_SS                -124  // 26 读磁卡错误（没有 SS）
#define CR_ERROR_READ_ERROR_IN_ES                -125  // 27 读卡器读错误（ES错误)
#define CR_ERROR_READ_ERROR_IN_LRC               -126  // 28 读卡器读错误（LRC错误）
#define CR_ERROR_WRITE_ERROR_VERIFICATION        -127  // 29 读卡器写错误（数据不一致）

#define CR_ERROR_POWER_FAILURE                   -128  // 30 读卡器电源掉电
#define CR_ERROR_DSR_IS_OFF                      -129  // 31 读卡器DSR 信号为 OFF

#define CR_ERROR_BEFORE_RETRACT_PULL             -130  // 40 吞卡时卡拔走
#define CR_ERROR_IC_CONTACT_SENNOR_ERROR         -131  // 41 IC触点或触点传感器错误
#define CR_ERROR_UNABLE_REACH_IC_POSITION        -132  // 43 无法走到 IC 卡位
#define CR_ERROR_ENFORCE_EJECT_CARD              -133  // 45 卡机强制弹卡
#define CR_ERROR_RETRIEVE_TIMEOUT                -134  // 46 前端卡未在指定时间内取走

#define CR_ERROR_COUNT_OVERFLOW                  -135  // 50 回收卡计数溢出
#define CR_ERROR_MOTOR_ERROR                     -136  // 51 马达错误
#define CR_ERROR_DIGITAL_DECODING_READ           -137  // 53 数字解码读错误
#define CR_ERROR_TANMPER_MOVE_ERROR              -138  // 54 防盗钩移动错误
#define CR_ERROR_TANMPER_BEEN_SET                -139  // 55 防盗钩已经设置，命令不能执行
#define CR_ERROR_CHIP_TEST_SENSOR_ERROR          -140  // 56 芯片检测传感器错误
#define CR_ERROR_TANMPER_IS_MOVING               -141  // 5B 防盗钩正在移动

#define CR_ERROR_ICC_OR_SAM_VCC_ABNORMAL         -142  // 60 IC 卡或 SAM 卡 Vcc 条件异常
#define CR_ERROR_ICC_OR_SAM_ATR_COMM_ERROR       -143  // 61 IC 卡或 SAM 卡 ATR 通讯错误
#define CR_ERROR_ICC_OR_SAM_ACTIVE_ATR_INVALID   -144  // 62 IC 卡或 SAM 卡在当前激活条件下 ATR 无效
#define CR_ERROR_ICC_OR_SAM_COMM_NO_RESPONSE     -145  // 63 IC 卡或 SAM 卡通讯过程中无响应
#define CR_ERROR_ICC_OR_SAM_COMM_ERROR           -146  // 64 IC 卡或 SAM 卡通讯错误（除无响应外）
#define CR_ERROR_ICC_OR_SAM_CARD_INACTIVATED     -147  // 65 IC 卡或 SAM 卡未激活
#define CR_ERROR_ICC_OR_SAM_NOT_SUPPORT          -148  // 66 IC 卡或 SAM 卡不支持（仅对于非 EMV 激活）
#define CR_ERROR_ICC_OR_SAM_NOT_SUPPORT_EMV      -149  // 69 IC 卡或 SAM 卡不支持（仅对于 EMV 激活）

#define CR_ERROR_ESU_AND_IC_COMM_ERROR           -150  // 76 ESU模块和卡机通讯错误
#define CR_ERROR_ESU_NO_CONNECTION               -151  // 95 ESU 模块损坏或无连接
#define CR_ERROR_ESU_OVERCURRENT                 -152  // 99 ESU 模块过流
#define CR_ERROR_NO_RECEIVE_INIT_CMD             -153  // B0 未接收到初始化命令

#endif // CARDREADER_ERROR_H
