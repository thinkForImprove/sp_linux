
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
  表A.2 错误代码表
错误代码说 明
-1 参数错误
-2 内存分配失败，没有分配到足够的内存
-3 用户申请的内存空间不足
-4 其他错误
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



//1 版本信息获取
int __stdcall FP_GetVersion(unsigned char code[4]);

//2	初始化操作
int __stdcall FP_Begin();

//3	1枚指纹图像特征提取
int __stdcall FP_FeatureExtract(unsigned char   cScannerType, //输入参数 指纹采集器类型代码
										   unsigned char cFingerCode,	 // 指位代码。输入参数。
										   unsigned char * pFingerImgBuf,//输入参数 指纹图像数据指针 图像格式为256(width)*360(height) 8位灰度图像 
										   unsigned char * pFeatureData); //输出参数 指纹图像特征数据指针


//5	对2个指纹特征数据进行比对，得到匹配相似度值
int __stdcall FP_FeatureMatch(unsigned char * pFeatureData1,	//输入参数 指纹特征数据1
										 unsigned char * pFeatureData2,	//输入参数 指纹特征数据2
										 float * pfSimilarity);			//输出参数 匹配相似度值0.00-1.00


//6	对指纹图像和指纹特征进行比对，得到匹配相似度值
int __stdcall FP_ImageMatch(unsigned char * pFingerImgBuf,	//输入参数 指纹图像数据指针 图像格式为256(width)*360(height)
										unsigned char * pFeatureData,	//输入参数 指纹特征数据
										float * pfSimilarity);			//输出参数 指纹图像特征数据指针

//7	对指纹图像数据进行压缩
int __stdcall FP_Compress(  unsigned char cScannerType,	//输入参数 指纹采集器类型代码
	unsigned char cEnrolResult,	
	unsigned char cFingerCode,		 // 指位代码。输入参数。
	unsigned  char * pFingerImgBuf,   //输入参数 指纹图像数据指针 图像格式为256(width)*360(height)
	int nCompressRatio,				//输入参数 压缩倍数
	unsigned char * pCompressedImgBuf,//输出参数 指纹图像压缩数据指针 空间为20K字节
	unsigned char strBuf[256]);		//输出参数 返回-4时的错误信息

//8	对指纹图像压缩数据进行复现
int __stdcall FP_Decompress (unsigned char * pCompressedImgBuf,//输入参数 指纹图像压缩数据指针 已分配好20K字节空间
										unsigned char * pFingerImgBuf,	//输入参数 指纹图像数据指针 大小256*360 字节
										unsigned char strBuf[256]);		//输出参数 返回-4时的错误信息

//9	获取指纹图像的质量值
int __stdcall FP_GetQualityScore (unsigned char * pFingerImgBuf,//输入参数 指纹图像数据指针 图像格式为256(width)*360(height)
											 unsigned char  * pnScore);//输出参数 图像质量值 00H - 64H


//10 生成"未注册"指纹特征数据
int __stdcall FP_GenFeatureFromEmpty1(
					unsigned char   cScannerType,  //输入参数 指纹采集器类型代码
					unsigned char	cFingerCode,	 // 指位代码。输入参数。
					unsigned char * pFeatureData  //输出参数 已由调用者分配512字节空间
);

//10 生成"未注册"指纹特征数据
int __stdcall FP_GenFeatureFromEmpty2(unsigned char cFingerCode, unsigned char * pFeatureData);//输出参数 已由调用者分配512字节空间

//11 生成"未注册"指纹图像压缩数据
//int __stdcall FP_GenCompressDataFromEmpty(unsigned char * pCompressedImgBuf);//输出参数 已由调用者分配20K字节空间
//

//12 结束操作
int __stdcall FP_End();

#ifdef __cplusplus
}
#endif

#endif//..#ifndef __ID_FPR_H__



