
#ifndef SIGNATURE_H
#define SIGNATURE_H

#ifndef bool
#define bool unsigned char
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

//设置显示器宽高
//多显示器设置显示器偏移和宽高,默认根据窗口位置自动识别所在显示器偏移和宽高
//offsetX:相对桌面原点，签名窗口所在显示器的横坐标
//offsetY:相对桌面原点，签名窗口所在显示器的纵坐标
//screenWidth:签名窗口所在显示器的宽度
//screenHeight:签名窗口所在显示器的高度
void setScreenWidthHeight(int offsetX, int offsetY, unsigned int screenWidth, unsigned int screenHeight, char *pchErrCode);
//启动签名
//#if

//多个函数同时使用

//设置签名窗口
//x:相对桌面原点，签名区域的横坐标
//y:相对桌面原点，签名区域的纵坐标
//w:签名区域的宽度
//h:签名区域的高度
void SetSignWindow(int x, int y, int w, int h, char* pchErrCode);

//设置背景色和透明度
//Transparency:(0-255),0完全透明,255不透明
//backColor:背景色;是否使用背景色
//useBackColor:1使用背景颜色,0透明背景
void SetBackColorParam(int Transparency, unsigned long backColor, int useBackColor, char* pchErrCode);

//设置签字背景图片
//photoPath:背景图片路径
//bPicAlwaysShow:true时一直显示,false写字前一直显示
void SetBackgroundPicture(char *photoPath, bool bPicAlwaysShow, char* pchErrCode);

//设置提示文本
//Left:提示文本左上角横坐标
//Top:提示文本左上角纵坐标
//string:提示文本字符串
//fontName:提示文本字体名
//fontSize:提示文本字体大小
//textColor:提示文本字体颜色
//bTxtAlwaysShow:true时一直显示,false写字前一直显示
void SetTextData(int Left, int Top, const char* string, const char* fontName, int fontSize, unsigned long textColor, bool bTxtAlwaysShow, char* pchErrCode);

//设置笔的粗细
//MaxPressurePixel:压感最大时对应的像素值
void SetPenMax(int MaxPressurePixel, char *pchErrCode);

//使用上述函数的设置值启动一个签名窗口。
void startSignatureUseSetting(char* pchErrCode);

//#else
	
//只能使用一个函数

//x:相对桌面原点，签名区域的横坐标
//y:相对桌面原点，签名区域的纵坐标
//w:签名区域的宽度
//h:签名区域的高度
void startSignature(int x, int y, int w, int h, char *pchErrCode);

//x:相对桌面原点，签名区域的横坐标
//y:相对桌面原点，签名区域的纵坐标
//w:签名区域的宽度
//h:签名区域的高度
//photoPath:图片路径
//bAlwaysShow:取值true图片一直显示，取值false写字前图片一直显示
void startSignPng(int x, int y, int w, int h, char *photoPath, bool bAlwaysShow, char *pchErrCode);

//x:相对桌面原点，签名区域的横坐标
//y:相对桌面原点，签名区域的纵坐标
//w:签名区域的宽度
//h:签名区域的高度
//PenMax:笔迹最大像素
//photoPath:图片路径
//bAlwaysShow:取值true图片一直显示，取值false写字前图片一直显示
void startSignPngPenMax(int x, int y, int w, int h, unsigned int PenMax, char *photoPath, bool bAlwaysShow, char *pchErrCode);

//x:相对桌面原点，签名区域的横坐标
//y:相对桌面原点，签名区域的纵坐标
//w:签名区域的宽度
//h:签名区域的高度
//PenMax:笔迹最大像素
//pcMsg:需要显示的字符串文字
//bAlwaysShow:取值true消息一直显示，取值false写字前一直显示
//iX:提示文本左上角横坐标
//iY:提示文本左上角纵坐标
//iFontHeight:提示文本字体大小
//ulColor:提示文本字体颜色
//pccFont:显示消息用的字体名
void startSignMsgPen(int x, int y, int w, int h, unsigned int PenMax, const char *pcMsg, bool bAlwaysShow, int iX, int iY, int iFontHeight,	unsigned long ulColor, const char *pccFont, char *pchErrCode);

//#endif


//清除签名，把窗口里面已经有的签名都清除，便于用户重新签
void clearSignature(char *pchErrCode);

//隐藏签名窗口，不影响已写入的笔迹内容
void hideSignWindow(char *pchErrCode);

//显示签名窗口，显示隐藏窗口前已写入的笔迹内容
void showSignWindow(char *pchErrCode);

//关闭签名窗口，停止签名，后面可以调用获取图片和数据函数，最后必须再调用endSignature释放资源
void closeSignWindow(char *pchErrCode);

//结束签名，释放所有资源，后面不能再调用获取图片和数据函数
void endSignature(char *pchErrCode);

//获取电子签名加密轨迹数据和图片
//psignData:明文签名轨迹数据
//psignDataLen:轨迹数据长度，输入值表示缓冲区最大长度，输出值表示轨迹数据长度。输入输出
//iIndex:主密钥号，取值1-8
//pbWorkKey:工作密钥数组
//photoPath:图片路径
void getSignature(unsigned char *psignData, long *psignDataLen, int iIndex, unsigned char *pbWorkKey, char *photoPath, char *pchErrCode);

//获取电子签名明文二进制轨迹数据
//psignData:明文签名轨迹数据
//psignDataLen:轨迹数据长度，输入值表示缓冲区最大长度，输出值表示轨迹数据长度。输入输出
void getSignData(unsigned char *psignData, long *psignDataLen, char *pchErrCode);

//获取电子签名明文文本轨迹数据
//signFilePath:文件路径
//type:1 w,h,P1024(x,y,z,timestamp;)
//type:2 w,h,1024(x,y,z/1023,deltatimems;)
//type:3 w,h,1024(x,y,z/1023;)
//type:4 w,h,Pxx,(x,y,z;)
//type:5 w,h,1024(x,y,z/1023,deltatimeus;)
void getSignDataFile(char* signFilePath, int type, char *pchErrCode);

//获取签字图片
//photoPath:图片路径
//multiple:笔迹放大倍数倍
void getPngPicture(char* photoPath, double multiple, char* pchErrCode);

//当签名区域输入的宽高比非2:1时,获取到的签字图片宽高比是2:1
//photoPath:图片路径
//multiple:笔迹放大倍数倍
void getPngPictureW2H1(char* photoPath, double multiple, char* pchErrCode);

//灌注DES主密钥
//pPriKey:密钥的数组
//number:密钥号取值1-8
void setDESPrimaryKey(char *pPriKeys, int number, char *pchErrCode);

//灌注DES主密钥
//pPriKey:密钥的数组
//iLength:取值8
//iIndex:取值1-8
void setPrimaryKey(char *pPriKey, int iLength, int iIndex, char *pchErrCode);

//复位设备
void resetDev(char *pchErrCode);

//获取设备状态
//返回	正常		YGB0000001
//		设备忙		YGB0000002
//		设备掉线	YGB0000003
void getDevStatus(char *pchErrCode);

//获取固件版本信息
//strVer:返回固件版本信息字符串地址
void getFirmwareVer(char *strVer, char *pchErrCode);

//获取电子签名加密轨迹数据
//固件3.00版本后支持;TDES2是3.01版本后支持
//psignData:加密后签名轨迹数据,前4个字节表示长度,先低后高,调用者需分配4M内存	输入输出
//iEncryptType:加密算法类型,取值DES:1, TDES3:2, SM4:3, SM2:4, TDES2:5;加密使用的Key用importKey函数设置   输入
void getSign(unsigned char *psignData, int iEncryptType, char* pchErrCode);

//灌注密钥
//固件3.01版本后支持
//pKey:密钥的数组
//iLength:取值DES:8; TDES2:16; TDES3:24; SM4:16;
//iIndex:取值1-8主密钥存储位置;
//iDecKeyNum:取值1-8解密主密钥号;
//iDecMode:(iUse为2,3时有效)取值DES:1; TDES2:2; TDES3:3; SM4:4;
//iUse:直接灌注主密钥:1; 解密后灌注主密钥:2; 轨迹数据加密密钥:3; SM2公钥:4;
void importKey(char* pKey, int iLength, int iIndex, int iDecKeyNum, int iDecMode, int iUse, char* pchErrCode);

//返回密钥校验值
//固件3.03版本后支持
//pKVC:密钥校验值
//iLength:返回pKVC的长度DES:8; TDES2:8; TDES3:8; SM4:16;
//iIndex:取值1-8主密钥存储位置;
//iEncMode:取值DES:1; TDES2:2; TDES3:3; SM4:4;
void getKeyVerificationCode(char* pKVC, int *iLength, int iIndex, int iEncMode, char* pchErrCode);

//主密钥转工作密钥(轨迹数据加密用)
//固件3.08版本后支持
//iIndex:取值1-8,MasterKey存储位置.
void masterKeyToWorkKey(int iIndex, char* pchErrCode);

//获取动态库编译日期
//strDate返回动态库编译日期信息字符串地址
void getCompileDate(char *strDate, char *pchErrCode);

#endif
