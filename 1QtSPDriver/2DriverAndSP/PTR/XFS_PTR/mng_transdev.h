#ifndef MNG_TRANSDEV_H
#define MNG_TRANSDEV_H

/*  ================ 设备代码定义 ====================
 */
#define DEV_CARDREAD    'A'     /* A  = 读卡器      */
#define DEV_TICKETPRT   'B'     /* B    =   打票机      */
#define DEV_PZPRT       'C'     /* C  = 凭证打印机  */
#define DEV_CZPRT       'D'     /* D  = 存折打印机  */
#define DEV_CGPRT       'E'     /* E  = 存根打印机  */
#define DEV_DJPRT       'F'     /* F  = 打单机      */
#define DEV_PINPAD      'G'     /* G  = 密码键盘      */
#define DEV_UPS         'H'     /* H  = UPS               */
#define DEV_OUTNOTE     'I'     /* I  = 出钞机        */
#define DEV_INNOTE      'J'     /* J  = 进钞机        */
#define DEV_COIN        'K'     /* K  = 硬币机        */
#define DEV_SENSOR      'L'     /* L  = 传感器        */
#define DEV_CBOX        'M'     /* M  = 综合控制器  */

/*  =============== 设备状态定义 =====================
 */
/* 通用状态代码 */
#define ST_DEV_NO       '0'     /* '0'=无此设备 */
#define ST_DEV_NORMAL   '1'     /* '1'=正常 */
#define ST_DEV_ERROR    '2'     /* '2'=设备故障 */
#define ST_DEV_OFFLINE  '3'     /* '3'=设备离线 */

/* 打票机特性状态码
 * '4'=票纸少  '5'=票纸缺； '6'=碳带缺'7'=堵票  '8'=残票  '9'=出票机构故障
 */
#define ST_DEVB_TPLITTLE    '4'
#define ST_DEVB_TPNO        '5'
#define ST_DEVB_CARBNO      '6'
#define ST_DEVB_TICKETJAM   '7'
#define ST_DEVB_TICKETBAD   '8'
#define ST_DEVB_TOUTERR     '9'

/*  凭证打印机特性状态码
 *  '4'=纸少；'5'=纸缺；'9'=传送机构故障
 */
#define ST_DEVC_PAPERLITTLE '4'
#define ST_DEVC_PAPERNO     '5'
#define ST_DEVC_POUTERR     '9'

/*  存折打印机特性状态码
 *  '6'=Shutter门故障；'9'=传送机构故障
 */
#define ST_DEVD_SHUTERR    '6'
#define ST_DEVD_POUTERR    '9'

/*  存根打印机特性状态码
 *  '4'=纸少；'5'=纸缺；'6'=传送机构故障；'7'=缺纸写入文件；'8'=故障写入文件
 */
#define ST_DEVE_PAPERLITTLE '4'
#define ST_DEVE_PAPERNO     '5'
#define ST_DEVE_POUTERR     '6'
#define ST_DEVE_NP_WRITE_TO_FILE    '7'
#define ST_DEVE_HE_WRITE_TO_FILE    '8'

/*  单据打印机特性状态码
 *  '4'=纸少；'5'=纸缺；'9'=传送机构故障
 */
#define ST_DEVF_PAPERLITTLE '4'
#define ST_DEVF_PAPERNO     '5'
#define ST_DEVF_POUTERR     '9'

/*  密码键盘特性状态码
 *  '4'=密钥丢失
 */
#define ST_DEVG_LOSTKEY    '4'

/*  出钞机特性状态码
 *  '4'=钞少；'5'=缺钞；'9'=卡钞
 */
#define ST_DEVI_NOTELITTLE  '4'
#define ST_DEVI_NOTENO      '5'
#define ST_DEVI_NOTEFULL    '6'
#define ST_DEVI_NOTEJAM     '9'

/*  进钞机特性状态码
 *  '4'=钞满；'9'=卡钞
 */
#define ST_DEVJ_NOTEMORE    '4'
#define ST_DEVI_NOTEJAM     '9'

/*  传感器特性状态码
 *  '4'=报警
 */
#define ST_DEVL_ALARM       '4'
#endif // MNG_TRANSDEV_H
