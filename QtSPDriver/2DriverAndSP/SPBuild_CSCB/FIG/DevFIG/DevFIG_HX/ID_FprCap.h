#ifndef ID_FPRCAP_H
#define ID_FPRCAP_H

#if defined(__linux__)
#define __stdcall
#endif

#ifdef __cplusplus
extern "C" {
#endif

int __stdcall LIVESCAN_Init(void);
int __stdcall LIVESCAN_Close(void);
int __stdcall LIVESCAN_GetChannelCount(void);
int __stdcall LIVESCAN_SetBright(int nChannel,int nBright);
int __stdcall LIVESCAN_SetContrast(int nChannel,int nContrast);
int __stdcall LIVESCAN_GetBright(int nChannel,int *pnBright);
int __stdcall LIVESCAN_GetContrast(int nChannel,int *pnContrast);
int __stdcall LIVESCAN_GetMaxImageSize(int nChannel,int *pnWidth,int *pnHeight);
int __stdcall LIVESCAN_GetCaptWindow(int nChannel,int *pnOriginX,int *pnOriginY,int *pnWidth,int *pnHeight);
int __stdcall LIVESCAN_SetCaptWindow(int nChannel,int nOriginX,int nOriginY,int nWidth,int nHeight);
int __stdcall LIVESCAN_Setup(void);
int __stdcall LIVESCAN_BeginCapture(int nChannel);
int __stdcall LIVESCAN_GetFPRawData(int nChannel,unsigned char *pRawData);
int __stdcall LIVESCAN_GetFPBmpData(int nChannel,unsigned char *pBmpData);
int __stdcall LIVESCAN_EndCapture(int nChannel);
int __stdcall LIVESCAN_IsSupportSetup(void);
int __stdcall LIVESCAN_GetVersion(void);
int __stdcall LIVESCAN_GetDesc(char pszDesc[1024]);
int __stdcall LIVESCAN_GetErrorInfo(int nErrorNo,char pszErrorInfo[256]);
int __stdcall LIVESCAN_SetBufferEmpty(unsigned char *pImageData, long imageLength);
int __stdcall LIVESCAN_PlaySound(int nChannel, int cSoundCode1, int cSoundCode2);

#ifdef __cplusplus
}
#endif

#endif

