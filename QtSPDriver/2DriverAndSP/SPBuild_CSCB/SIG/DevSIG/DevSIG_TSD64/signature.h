
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

//������ʾ�����
//����ʾ��������ʾ��ƫ�ƺͿ��,Ĭ�ϸ��ݴ���λ���Զ�ʶ��������ʾ��ƫ�ƺͿ��
//offsetX:�������ԭ�㣬ǩ������������ʾ���ĺ�����
//offsetY:�������ԭ�㣬ǩ������������ʾ����������
//screenWidth:ǩ������������ʾ���Ŀ��
//screenHeight:ǩ������������ʾ���ĸ߶�
void setScreenWidthHeight(int offsetX, int offsetY, unsigned int screenWidth, unsigned int screenHeight, char *pchErrCode);
//����ǩ��
//#if

//�������ͬʱʹ��

//����ǩ������
//x:�������ԭ�㣬ǩ������ĺ�����
//y:�������ԭ�㣬ǩ�������������
//w:ǩ������Ŀ��
//h:ǩ������ĸ߶�
void SetSignWindow(int x, int y, int w, int h, char* pchErrCode);

//���ñ���ɫ��͸����
//Transparency:(0-255),0��ȫ͸��,255��͸��
//backColor:����ɫ;�Ƿ�ʹ�ñ���ɫ
//useBackColor:1ʹ�ñ�����ɫ,0͸������
void SetBackColorParam(int Transparency, unsigned long backColor, int useBackColor, char* pchErrCode);

//����ǩ�ֱ���ͼƬ
//photoPath:����ͼƬ·��
//bPicAlwaysShow:trueʱһֱ��ʾ,falseд��ǰһֱ��ʾ
void SetBackgroundPicture(char *photoPath, bool bPicAlwaysShow, char* pchErrCode);

//������ʾ�ı�
//Left:��ʾ�ı����ϽǺ�����
//Top:��ʾ�ı����Ͻ�������
//string:��ʾ�ı��ַ���
//fontName:��ʾ�ı�������
//fontSize:��ʾ�ı������С
//textColor:��ʾ�ı�������ɫ
//bTxtAlwaysShow:trueʱһֱ��ʾ,falseд��ǰһֱ��ʾ
void SetTextData(int Left, int Top, const char* string, const char* fontName, int fontSize, unsigned long textColor, bool bTxtAlwaysShow, char* pchErrCode);

//���ñʵĴ�ϸ
//MaxPressurePixel:ѹ�����ʱ��Ӧ������ֵ
void SetPenMax(int MaxPressurePixel, char *pchErrCode);

//ʹ����������������ֵ����һ��ǩ�����ڡ�
void startSignatureUseSetting(char* pchErrCode);

//#else
	
//ֻ��ʹ��һ������

//x:�������ԭ�㣬ǩ������ĺ�����
//y:�������ԭ�㣬ǩ�������������
//w:ǩ������Ŀ��
//h:ǩ������ĸ߶�
void startSignature(int x, int y, int w, int h, char *pchErrCode);

//x:�������ԭ�㣬ǩ������ĺ�����
//y:�������ԭ�㣬ǩ�������������
//w:ǩ������Ŀ��
//h:ǩ������ĸ߶�
//photoPath:ͼƬ·��
//bAlwaysShow:ȡֵtrueͼƬһֱ��ʾ��ȡֵfalseд��ǰͼƬһֱ��ʾ
void startSignPng(int x, int y, int w, int h, char *photoPath, bool bAlwaysShow, char *pchErrCode);

//x:�������ԭ�㣬ǩ������ĺ�����
//y:�������ԭ�㣬ǩ�������������
//w:ǩ������Ŀ��
//h:ǩ������ĸ߶�
//PenMax:�ʼ��������
//photoPath:ͼƬ·��
//bAlwaysShow:ȡֵtrueͼƬһֱ��ʾ��ȡֵfalseд��ǰͼƬһֱ��ʾ
void startSignPngPenMax(int x, int y, int w, int h, unsigned int PenMax, char *photoPath, bool bAlwaysShow, char *pchErrCode);

//x:�������ԭ�㣬ǩ������ĺ�����
//y:�������ԭ�㣬ǩ�������������
//w:ǩ������Ŀ��
//h:ǩ������ĸ߶�
//PenMax:�ʼ��������
//pcMsg:��Ҫ��ʾ���ַ�������
//bAlwaysShow:ȡֵtrue��Ϣһֱ��ʾ��ȡֵfalseд��ǰһֱ��ʾ
//iX:��ʾ�ı����ϽǺ�����
//iY:��ʾ�ı����Ͻ�������
//iFontHeight:��ʾ�ı������С
//ulColor:��ʾ�ı�������ɫ
//pccFont:��ʾ��Ϣ�õ�������
void startSignMsgPen(int x, int y, int w, int h, unsigned int PenMax, const char *pcMsg, bool bAlwaysShow, int iX, int iY, int iFontHeight,	unsigned long ulColor, const char *pccFont, char *pchErrCode);

//#endif


//���ǩ�����Ѵ��������Ѿ��е�ǩ��������������û�����ǩ
void clearSignature(char *pchErrCode);

//����ǩ�����ڣ���Ӱ����д��ıʼ�����
void hideSignWindow(char *pchErrCode);

//��ʾǩ�����ڣ���ʾ���ش���ǰ��д��ıʼ�����
void showSignWindow(char *pchErrCode);

//�ر�ǩ�����ڣ�ֹͣǩ����������Ե��û�ȡͼƬ�����ݺ������������ٵ���endSignature�ͷ���Դ
void closeSignWindow(char *pchErrCode);

//����ǩ�����ͷ�������Դ�����治���ٵ��û�ȡͼƬ�����ݺ���
void endSignature(char *pchErrCode);

//��ȡ����ǩ�����ܹ켣���ݺ�ͼƬ
//psignData:����ǩ���켣����
//psignDataLen:�켣���ݳ��ȣ�����ֵ��ʾ��������󳤶ȣ����ֵ��ʾ�켣���ݳ��ȡ��������
//iIndex:����Կ�ţ�ȡֵ1-8
//pbWorkKey:������Կ����
//photoPath:ͼƬ·��
void getSignature(unsigned char *psignData, long *psignDataLen, int iIndex, unsigned char *pbWorkKey, char *photoPath, char *pchErrCode);

//��ȡ����ǩ�����Ķ����ƹ켣����
//psignData:����ǩ���켣����
//psignDataLen:�켣���ݳ��ȣ�����ֵ��ʾ��������󳤶ȣ����ֵ��ʾ�켣���ݳ��ȡ��������
void getSignData(unsigned char *psignData, long *psignDataLen, char *pchErrCode);

//��ȡ����ǩ�������ı��켣����
//signFilePath:�ļ�·��
//type:1 w,h,P1024(x,y,z,timestamp;)
//type:2 w,h,1024(x,y,z/1023,deltatimems;)
//type:3 w,h,1024(x,y,z/1023;)
//type:4 w,h,Pxx,(x,y,z;)
//type:5 w,h,1024(x,y,z/1023,deltatimeus;)
void getSignDataFile(char* signFilePath, int type, char *pchErrCode);

//��ȡǩ��ͼƬ
//photoPath:ͼƬ·��
//multiple:�ʼ��Ŵ�����
void getPngPicture(char* photoPath, double multiple, char* pchErrCode);

//��ǩ����������Ŀ�߱ȷ�2:1ʱ,��ȡ����ǩ��ͼƬ��߱���2:1
//photoPath:ͼƬ·��
//multiple:�ʼ��Ŵ�����
void getPngPictureW2H1(char* photoPath, double multiple, char* pchErrCode);

//��עDES����Կ
//pPriKey:��Կ������
//number:��Կ��ȡֵ1-8
void setDESPrimaryKey(char *pPriKeys, int number, char *pchErrCode);

//��עDES����Կ
//pPriKey:��Կ������
//iLength:ȡֵ8
//iIndex:ȡֵ1-8
void setPrimaryKey(char *pPriKey, int iLength, int iIndex, char *pchErrCode);

//��λ�豸
void resetDev(char *pchErrCode);

//��ȡ�豸״̬
//����	����		YGB0000001
//		�豸æ		YGB0000002
//		�豸����	YGB0000003
void getDevStatus(char *pchErrCode);

//��ȡ�̼��汾��Ϣ
//strVer:���ع̼��汾��Ϣ�ַ�����ַ
void getFirmwareVer(char *strVer, char *pchErrCode);

//��ȡ����ǩ�����ܹ켣����
//�̼�3.00�汾��֧��;TDES2��3.01�汾��֧��
//psignData:���ܺ�ǩ���켣����,ǰ4���ֽڱ�ʾ����,�ȵͺ��,�����������4M�ڴ�	�������
//iEncryptType:�����㷨����,ȡֵDES:1, TDES3:2, SM4:3, SM2:4, TDES2:5;����ʹ�õ�Key��importKey��������   ����
void getSign(unsigned char *psignData, int iEncryptType, char* pchErrCode);

//��ע��Կ
//�̼�3.01�汾��֧��
//pKey:��Կ������
//iLength:ȡֵDES:8; TDES2:16; TDES3:24; SM4:16;
//iIndex:ȡֵ1-8����Կ�洢λ��;
//iDecKeyNum:ȡֵ1-8��������Կ��;
//iDecMode:(iUseΪ2,3ʱ��Ч)ȡֵDES:1; TDES2:2; TDES3:3; SM4:4;
//iUse:ֱ�ӹ�ע����Կ:1; ���ܺ��ע����Կ:2; �켣���ݼ�����Կ:3; SM2��Կ:4;
void importKey(char* pKey, int iLength, int iIndex, int iDecKeyNum, int iDecMode, int iUse, char* pchErrCode);

//������ԿУ��ֵ
//�̼�3.03�汾��֧��
//pKVC:��ԿУ��ֵ
//iLength:����pKVC�ĳ���DES:8; TDES2:8; TDES3:8; SM4:16;
//iIndex:ȡֵ1-8����Կ�洢λ��;
//iEncMode:ȡֵDES:1; TDES2:2; TDES3:3; SM4:4;
void getKeyVerificationCode(char* pKVC, int *iLength, int iIndex, int iEncMode, char* pchErrCode);

//����Կת������Կ(�켣���ݼ�����)
//�̼�3.08�汾��֧��
//iIndex:ȡֵ1-8,MasterKey�洢λ��.
void masterKeyToWorkKey(int iIndex, char* pchErrCode);

//��ȡ��̬���������
//strDate���ض�̬�����������Ϣ�ַ�����ַ
void getCompileDate(char *strDate, char *pchErrCode);

#endif
