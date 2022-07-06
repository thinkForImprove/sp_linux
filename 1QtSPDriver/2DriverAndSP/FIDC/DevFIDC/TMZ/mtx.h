#ifndef MTX_H
#define MTX_H

#ifdef __cplusplus
extern "C" {
#endif

	/* 设备函数组 */
	int open_usb_device(int vid,int pid,int proid);
	int open_serial_device(char *serial_path,int baudrate,int devid,int proid);
	int get_device_version(char *device_version,int *verlen);
	int get_so_version(char *so_version,int *rlen);
	int device_beep(int nMsec,int nMsec_end,int nTime);
	int read_device_snr(int nSnrLen,char *snr_data);
	int write_device_snr(int nSnrLen,char *snr_data);
	int close_device();

	/* 非接CPU卡 */
	int picc_status();
	int picc_poweron(int nMode,unsigned char *sSnr,unsigned char *sAtr, int *nAtrLen);
	int picc_poweroff();
	int picc_apdu(unsigned char *sCmd, int nCmdLen, unsigned char *sResp, int *nRespLen);

	/* 接触CPU卡 */
	int icc_status(int nCardSet);
	int icc_reset(int nCardSet, unsigned char *sAtr, int *nAtrLen);
	int icc_reset_buad(int nCardSet, int nBaud,unsigned char *sAtr, int *nAtrLen);
	int icc_poweron(int nCardSet, unsigned char *sAtr, int *nAtrLen);
	int icc_poweron_buad(int nCardSet, int nBaud, unsigned char *sAtr, int *nAtrLen);
	int icc_poweroff(int nCardSet);
	int icc_apdu(int nCardSet,unsigned char *sCmd, int nCmdLen, unsigned char *sResp, int *nRespLen);

	int contact_select(unsigned char nCardType);
	int contact_verify(unsigned char *nCardType);
	/* 4442卡 */
	int sle4442_is42(unsigned char* sCardState);
	int sle4442_read(unsigned char nAddr,unsigned short nDLen,unsigned char* sRecData);
	int sle4442_write(unsigned char nAddr,unsigned short nWLen,unsigned char* sWriteData);
	int sle4442_pwd_read(unsigned char* sKey);
	int sle4442_pwd_check(unsigned char* sKey);
	int sle4442_pwd_modify(unsigned char* sKey);
	int sle4442_probit_read(unsigned char* nLen,unsigned char* sProBitData);
	int sle4442_probit_write(unsigned char nAddr,unsigned short nWLen,unsigned char* sProBitData);
	int sle4442_errcount_read(unsigned char* nErrCount);
	/* 4428卡 */
	int sle4428_is28(unsigned char* sCardState);
	int sle4428_read(unsigned short nAddr,unsigned short nDLen,unsigned char* sRecData);
	int sle4428_write(unsigned short nAddr,unsigned short nWLen,unsigned char* sWriteData);
	int sle4428_pwd_read(unsigned char* sKey);
	int sle4428_pwd_check(unsigned char* sKey);
	int sle4428_pwd_modify(unsigned char* sKey);
	int sle4428_probit_read(unsigned short nAddr,unsigned short nDLen,unsigned char* sRecData);
	int sle4428_probit_write(unsigned short nAddr,unsigned short nWLen,unsigned char* sWriteData);
	int sle4428_errcount_read(unsigned char* nErrCount);

	/* M1卡 */
	int rf_reset();
	int rf_card(unsigned char nMode,unsigned char *sSnr);
	int rf_authentication_key(unsigned char nMode,unsigned char nBlockaddr,unsigned char *sNkey);
	int rf_read(unsigned char nAdr,unsigned char *sReadData);
	int rf_write(unsigned char nAdr,unsigned char *sWriteData);
	int rf_initval(unsigned char nAdr,unsigned long ulValue);
	int rf_readval(unsigned char nAdr,unsigned long *ulValue);
	int rf_increment(unsigned char nAdr,unsigned long ulValue);
	int rf_decrement(unsigned char nAdr,unsigned long ulValue);
	int rf_transfer(unsigned char nAdr);
	int rf_restore(unsigned char nAdr);
	int rf_terminal();

	/*  磁条卡 */
		
	/**
	 * [set_magnetic_mode 设置磁条卡模式]
	 * @param  nmode [
	 * mode参数设置：
			0x00：模拟键盘输出模式；
			0x01：命令输出模式；
			0x40: 开关输出模式，启动磁条刷卡。发送此命令后，清空缓存数据，开始等待磁条刷卡；
			0x41: 开关输出模式，关闭磁条刷卡。发送此命令后，清空缓存数据，结束等待磁条刷卡；
			当mode最高位1时，磁条数据去掉‘；‘和'?’起始结束符后输出。也就是下面参数值，磁条数据去掉起始和结束符后输出：
		0x80:模拟键盘输出模式;
			0x81:命令输出模式；
			0xC0:开关输出模式，启动磁条刷卡。发送此命令后，清空缓存数据，开始等待磁条刷卡；
			0xC1:开关输出模式，关闭磁条刷卡。发送此命令后，清空缓存数据，结束等待磁条刷卡；
			为了保证向下兼容，默认mode=0x01; 设置成功后，掉电保存。
					 ]
	 * @return       [0:正确，其他错误]
	 */
	int set_magnetic_mode(int nmode);
	int magnetic_read(int timeout, int* track1_len, int* track2_len, int* track3_len, char* track1_data, char* track2_data, char* track3_data);

	/* 二代证 */
	int idcard_moduleid(int *ModeIDLen,char *ModeID);
	int idcard_uid(int *UIDLen,char *UID);

	/**
	 * [idcard_read_base 读取身份证原始数据]
	 * @param  read_finger_data [0：不读指纹信息，1：读取指纹信息]
	 * @param  textIDCMsg       [返回身份证的文字数据信息]
	 * @param  textIDCMsgLen    [文字数据信息长度]
	 * @param  photoIDCMsg      [返回身份证的照片数据信息]
	 * @param  photoIDCMsgLen   [照片数据信息长度]
	 * @param  fingerIDCMsg     [返回身份证的指纹数据信息]
	 * @param  fingerIDCMsgLen  [指纹数据信息长度]
	 * @return                  [0:正确，其他错误]
	 */
	int idcard_read_base(
		int read_finger_data,
		unsigned char *textIDCMsg,
		unsigned int  *textIDCMsgLen,
		unsigned char *photoIDCMsg,
		unsigned int  *photoIDCMsgLen,
		unsigned char *fingerIDCMsg,
		unsigned int  *fingerIDCMsgLen
	);
	
	/**
	 * [parse_idcard_text 解析身份证件中的文字数据]
	 * @param  sCodeFormat   [按照什么编码解析文字信息，例如:"UTF-8//TRANSLIT//IGNORE"]
	 * @param  textIDCMsg    [身份证件中的文字数据]
	 * @param  textIDCMsgLen [文字数据长度]
	 * @param  cardType      [
	 * 返回的证件类型：
	 *  0 中国大陆身份证：
	 *  姓名|性别|民族|出生日期|住址|公民身份证号码|签发机关|有效期起始日期|有效期截止日期|预留数据|
	 *  1 外国人永久居住证：
	 *  英文姓名|性别|永久居留证号码|国籍所在地区|中文姓名|有效期起始日期|有效期截止日期|出生日期|证件版本号|当次申请受理机关代码|证件类型标识|预留数据|
	 *  2 港澳台居民居住证：
	 *  姓名|性别|出生日期|住址|公民身份证号码|签发机关|有效期起始日期|有效期截止日期|通行证号码|签发次数|证件类型标识|
	 *                       ]
	 * @param  cardMessage   [解析的证件信息，各项数据以 '|' 号分隔]
	 * @return               [0:正确，其他错误]
	 */
	int parse_idcard_text(
		char *sCodeFormat,
		unsigned char *textIDCMsg,
		unsigned int  textIDCMsgLen,
		int  *cardType,
		char *cardMessage
	);

	/**
	 * [parse_idcard_photo 解析身份证件中的照片数据]
	 * @param  photoIDCMsg          [身份证件中的照片数据]
	 * @param  photoIDCMsgLen       [身份证件中的照片数据的长度]
	 * @param  photo_save_full_path [需要保存文件全路径，例如："/home/zp.bmp"]
	 * @param  photo_sava_status    [0：照片保存失败，1：照片保存成功]
	 * @param  photo_base64         [照片文件的base64码]
	 * @param  photo_base64_len     [base64码长度]
	 * @return                      [0:正确，其他错误]
	 */
	int parse_idcard_photo(
		unsigned char *photoIDCMsg,
		unsigned int  photoIDCMsgLen,
		char *photo_save_full_path,
		char *photo_base64, 
		long  *photo_base64_len
	);

	/* 工具函数 */
	char* mystrupr(char *str/*,int len*/);
	char* mystrlwr(char *str/*,int len*/);
	void _splitpath(char *path,char *dir, char *fname, char *ext);
	int  _makepath(char *path,char *dir, char *fname, char *ext);
	int bmp_generate(unsigned char *pSrcBmpdata,char *pBMPFile,int Width ,int Height);
	int img_generate(unsigned char *pSrcBmpdata,char *pBMPFile,int Width ,int Height,int ColorType/*0:RGB,1:BGR*/);
	int code_convert(char *from_charset,char *to_charset,char *inbuf,size_t inlen,char *outbuf,size_t outlen);
	int hex_asc(unsigned char *hex,unsigned char *asc,unsigned long length);
	int asc_hex(unsigned char *asc,unsigned char *hex,unsigned long length);
	long base64_hex(unsigned char *base64,unsigned char *hex,unsigned long base64len);
	long hex_base64(unsigned char *hex,unsigned char *base64,unsigned long hexlen);
	
	/**
	 * [wlt_2_rgb 将读取的二代证照片数据，解析为RGB数据]
	 * @param  wlt_data             [wlt文件数据,1024字节]
	 * @param  rgb_data             [解析的RGB数据，102*126*3字节，可根据需求生成BMP或者JPG，图像数据BGR格式，需要将B、R值互换。]
	 * @param  VendorCode			[厂商代码]
	 * @param  bsava_photo          [是否生成bmp图片。 0: 不生成   1: 生成]
	 * @param  photo_save_full_path [bmp文件全路径，例如："/home/zp.bmp"]
	 * @return                      [0:正确，其他错误,  -102:调用libwlt.so的解码函数unpack出错]
	 */
	int wlt_2_rgb(char *wlt_data, char *rgb_data,int VendorCode, int bsava_photo, char *photo_save_full_path);

	/**
	 * [get_base64_data 获取照片文件的base64码]
	 * @param  photo_save_full_path [bmp文件全路径，例如："/home/zp.bmp"]
	 * @param  base64               [照片文件的base64码]
	 * @param  base64_len           [base64码长度]
	 * @return                      [0:正确，其他错误]
	 */
	int get_base64_data(char *photo_save_full_path,char *base64,long *base64_len);

	int get_errmsg(int errcode,char *errmsg);
	/* 日志函数 */
	int enabled_log(char *cSavePath,char *cLogFileName,int iMaxSize,int iKeepingDate);
	int disenabled_log();

#ifdef __cplusplus
}
#endif

#endif
