#pragma once

#define DVEFINGER_NO_VTABLE  __declspec(novtable)
//////////////////////////////////////////////////////////////////////////
#define IMGBUFFSIZE         (512*512)       // ָ��ͼ�����ݴ�С
//////////////////////////////////////////////////////////////////////////
struct DVEFINGER_NO_VTABLE IDevFinger
{
    // �ͷŽӿ�
    virtual void Release() = 0;
    // ������
    virtual long Open(const char *lpMode) = 0;
    // �ر�����
    virtual long Close() = 0;
    // ��ʼ���豸
    virtual long Init() = 0;
    // ��λ
    virtual long Reset() = 0;
    // ��ȡ�豸��Ϣ
    virtual long GetDevInfo(char *pInfo) = 0;
    // ȡ״̬
    virtual long GetStatus() = 0;
    // ȡ����ȡ
    virtual long CancelRead() = 0;
    // �ȴ��ɿ���ָ
    virtual long WaitReleaseFinger(long lTimeOut) = 0;
    // ��ȡ�ɼ�ָ��ͼ������ΪBase64���룬����ֵ-1��ʾ��ʱ
    virtual long ReadImage(const char *pRandom, char *pImage, char *pMac, long lTimeOut) = 0;
    // ָ��ͼ��תָ���������ݣ�����ΪBase64����
    virtual long GetFeatureByImage(const char *pImage, char *pFeature) = 0;
    // ָ��ͼ��תָ��ģ�����ݣ�����ΪBase64����
    virtual long GetTemplateByImage(const char *pImage1, const char *pImage2, const char *pImage3, char *pTemplate) = 0;
    // ָ�Ʊȶԣ�iSecurityLevel->��ȫ����1~5��Ĭ��Ϊ 3��������ΪBase64����
    virtual long TFVerify(const char *pTemplate, const char *pFeature, int iSecurityLevel = 3) = 0;
    // ���� BASE64 ����/����
    virtual long CryptBase64(const char *pIn, int nInLen, char *pOut, int *nOutLen, bool bCrypt = true) = 0;
    // ���ݳ������������
    virtual long GetRandom(char *pRandom, int nBit = 32) = 0;
    // ���س�����Կ������Ϊʮ��������
    virtual long LoadDFK(const char *pDevID, const char *pKeyName, const char *pKeyVer, const char *pKey) = 0;
    // ��������Կ������Ϊʮ��������
    virtual long LoadDMK(const char *pDevID, const char *pKeyName, const char *pKeyVer, const char *pKey, const char *pMAC) = 0;
    // ���ع�����Կ������Ϊʮ��������
    virtual long LoadWorkKey(const char *pDevID, int nKeyType, const char *pKeyName, const char *pKeyVer, const char *pKey, const char *pMAC) = 0;

};


extern "C" long CreateIDevFinger(LPCSTR lpDevType, IDevFinger *&p);
//////////////////////////////////////////////////////////////////////////