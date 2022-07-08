
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the ID_FPR_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// ID_FPR_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#ifndef __ID_FPR_H__
#define __ID_FPR_H__

#if defined(__linux__)
#define __stdcall
#endif
//////////////////////////////////////////////////////////////////////////
/*
  ��A.2 ��������
�������˵ ��
-1 ��������
-2 �ڴ����ʧ�ܣ�û�з��䵽�㹻���ڴ�
-3 �û�������ڴ�ռ䲻��
-4 ��������
*/
#define		ID_FPR_Err_InvalidParam			(-1)
#define		ID_FPR_Err_FailToAllocate		(-2)
#define		ID_FPR_Err_FailToProcess		(-3)
#define		ID_FPR_Err_Others				(-9)
#define		ID_FPR_LOADDLL_FAIL			(-38)
#define		ID_FPR_NOFUNC	(-40)

#ifdef __cplusplus
extern "C" {

#endif



//1 �汾��Ϣ��ȡ
int __stdcall FP_GetVersion(unsigned char code[4]);

//2	��ʼ������
int __stdcall FP_Begin();

//3	1öָ��ͼ��������ȡ
int __stdcall FP_FeatureExtract(unsigned char   cScannerType, //������� ָ�Ʋɼ������ʹ���
										   unsigned char cFingerCode,	 // ָλ���롣���������
										   unsigned char * pFingerImgBuf,//������� ָ��ͼ������ָ�� ͼ���ʽΪ256(width)*360(height) 8λ�Ҷ�ͼ�� 
										   unsigned char * pFeatureData); //������� ָ��ͼ����������ָ��


//5	��2��ָ���������ݽ��бȶԣ��õ�ƥ�����ƶ�ֵ
int __stdcall FP_FeatureMatch(unsigned char * pFeatureData1,	//������� ָ����������1
										 unsigned char * pFeatureData2,	//������� ָ����������2
										 float * pfSimilarity);			//������� ƥ�����ƶ�ֵ0.00-1.00


//6	��ָ��ͼ���ָ���������бȶԣ��õ�ƥ�����ƶ�ֵ
int __stdcall FP_ImageMatch(unsigned char * pFingerImgBuf,	//������� ָ��ͼ������ָ�� ͼ���ʽΪ256(width)*360(height)
										unsigned char * pFeatureData,	//������� ָ����������
										float * pfSimilarity);			//������� ָ��ͼ����������ָ��

//7	��ָ��ͼ�����ݽ���ѹ��
int __stdcall FP_Compress(  unsigned char cScannerType,	//������� ָ�Ʋɼ������ʹ���
	unsigned char cEnrolResult,	
	unsigned char cFingerCode,		 // ָλ���롣���������
	unsigned  char * pFingerImgBuf,   //������� ָ��ͼ������ָ�� ͼ���ʽΪ256(width)*360(height)
	int nCompressRatio,				//������� ѹ������
	unsigned char * pCompressedImgBuf,//������� ָ��ͼ��ѹ������ָ�� �ռ�Ϊ20K�ֽ�
	unsigned char strBuf[256]);		//������� ����-4ʱ�Ĵ�����Ϣ

//8	��ָ��ͼ��ѹ�����ݽ��и���
int __stdcall FP_Decompress (unsigned char * pCompressedImgBuf,//������� ָ��ͼ��ѹ������ָ�� �ѷ����20K�ֽڿռ�
										unsigned char * pFingerImgBuf,	//������� ָ��ͼ������ָ�� ��С256*360 �ֽ�
										unsigned char strBuf[256]);		//������� ����-4ʱ�Ĵ�����Ϣ

//9	��ȡָ��ͼ�������ֵ
int __stdcall FP_GetQualityScore (unsigned char * pFingerImgBuf,//������� ָ��ͼ������ָ�� ͼ���ʽΪ256(width)*360(height)
											 unsigned char  * pnScore);//������� ͼ������ֵ 00H - 64H


//10 ����"δע��"ָ����������
int __stdcall FP_GenFeatureFromEmpty1(
					unsigned char   cScannerType,  //������� ָ�Ʋɼ������ʹ���
					unsigned char	cFingerCode,	 // ָλ���롣���������
					unsigned char * pFeatureData  //������� ���ɵ����߷���512�ֽڿռ�
);

//10 ����"δע��"ָ����������
int __stdcall FP_GenFeatureFromEmpty2(unsigned char cFingerCode, unsigned char * pFeatureData);//������� ���ɵ����߷���512�ֽڿռ�

//11 ����"δע��"ָ��ͼ��ѹ������
//int __stdcall FP_GenCompressDataFromEmpty(unsigned char * pCompressedImgBuf);//������� ���ɵ����߷���20K�ֽڿռ�
//

//12 ��������
int __stdcall FP_End();

#ifdef __cplusplus
}
#endif

#endif//..#ifndef __ID_FPR_H__



