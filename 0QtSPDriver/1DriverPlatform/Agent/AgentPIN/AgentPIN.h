#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSPIN.H"
//////////////////////////////////////////////////////////////////////////
class CAgentPIN : public IAgentBase, public CLogManage
{
public:
    CAgentPIN();
    virtual ~CAgentPIN();
public:
    virtual void Release();
    // 实现命令数据拷贝
    virtual HRESULT GetInfo(DWORD dwCategory, LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    virtual HRESULT Execute(DWORD dwCommand, LPVOID lpCmdData, LPVOID &lpCopyCmdData);
protected:

    //公共
    HRESULT CMD_WFS_PIN_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFSXDATA(LPWFSXDATA lpXData, CAutoWFMFreeBuffer *autofree, LPVOID lpSourceData, LPWFSXDATA &lpCopyCmdData);
    // 查询的
    HRESULT Get_WFS_INF_PIN_KEY_DETAIL(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_PIN_FUNCKEY_DETAIL(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFS_INF_PIN_KEY_DETAILEX(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);

    // 执行的
    HRESULT Exe_WFS_CMD_PIN_CRYPT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_IMPORT_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_DERIVE_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_GET_PIN(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_LOCAL_DES(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_CREATE_OFFSET(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_LOCAL_EUROCHEQUE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_LOCAL_VISA(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_PRESENT_IDC(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_GET_PINBLOCK(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_GET_PINBLOCK_EX(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_GET_DATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_INITIALIZATION(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_LOCAL_BANKSYS(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_BANKSYS_IO(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_HSM_SET_TDATA(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_SECURE_MSG_SEND(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_SECURE_MSG_RECEIVE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_GET_JOURNAL(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_IMPORT_KEY_EX(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_ENC_IO(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_HSM_INIT(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_SECUREKEY_ENTRY(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_GENERATE_KCV(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_IMPORT_RSA_PUBLIC_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_EXPORT_RSA_ISSUER_SIGNED_ITEM(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_IMPORT_RSA_SIGNED_DES_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_GENERATE_RSA_KEY_PAIR(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_EXPORT_RSA_EPP_SIGNED_ITEM(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_LOAD_CERTIFICATE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_GET_CERTIFICATE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_REPLACE_CERTIFICATE(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_IMPORT_RSA_ENCIPHERED_PKCS7_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_EMV_IMPORT_PUBLIC_KEY(LPVOID lpCmdData, LPVOID &lpCopyCmdData);
    HRESULT Exe_WFS_CMD_PIN_DIGEST(LPVOID lpCmdData, LPVOID &lpCopyCmdData);


private:
    // 加载库
    bool LoadDll();
private:
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};

