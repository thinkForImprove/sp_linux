#pragma once

//////////////////////////////////////////////////////////////////////////
#define DVEPDL_NO_VTABLE  __declspec(novtable)
//////////////////////////////////////////////////////////////////////////
// ������
#define PDL_OK                              (0)     // PDL�ɹ�
#define PDL_OPEN_ERR                        (-1)    // ���豸����ʧ��
#define PDL_DEV_ERR                         (-2)    // �豸�쳣ʧ��
#define PDL_CONFIG_ERR                      (-3)    // PDL���ô��󣬻�û���ҵ������ļ�
#define PDL_FILE_ERR                        (-4)    // PDL�ļ����󣬻�û���ҵ�PDL�ļ�
#define PDL_XOR_ERR                         (-5)    // ���ֵУ��ʧ��
#define PDL_MD5_ERR                         (-6)    // MD5ֵУ��ʧ��
#define PDL_SWITCH_ERR                      (-7)    // �л�PDLģʽʧ��
#define PDL_SENDHEX_ERR                     (-8)    // ���͹̼�����ʧ��
#define PDL_ERR                             (-9)    // �����̼�ʧ��
//////////////////////////////////////////////////////////////////////////
typedef const char *LPCSTR;
//////////////////////////////////////////////////////////////////////////
struct DVEPDL_NO_VTABLE IDevPDL
{
    // �ͷŽӿ�
    virtual void Release() = 0;
    // ������
    virtual long Open(LPCSTR lpMode) = 0;
    // �ر�����
    virtual long Close() = 0;
    // �Ƿ���Ҫ����
    virtual bool IsNeedDevPDL() = 0;
    // ִ������
    virtual long UpdateDevPDL() = 0;
    // ���Ի��������Ƿ�����
    virtual long TestDevPDL() = 0;
};

extern "C" long CreateIDevPDL(LPCSTR lpDevType, IDevPDL *&p);

//////////////////////////////////////////////////////////////////////////