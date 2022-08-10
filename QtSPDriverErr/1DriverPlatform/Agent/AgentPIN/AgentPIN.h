#pragma once
#include "IAgentBase.h"
#include "QtTypeInclude.h"
#include "XFSPIN.H"
#include "XFSPINCHN.H"                          //30-00-00-00(FS#0003)
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
    virtual HRESULT GetInfoOut(DWORD dwCategory, LPVOID lpQueryDetails, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT ExecuteOut(DWORD dwCommand, LPVOID lpCmdData, LPWFSRESULT &lpCopyCmdData);
    virtual HRESULT CopyEventStruct(UINT uMsgID, DWORD dwEventID, LPVOID lpData, LPWFSRESULT &lpResult);
protected:

    //公共
    HRESULT CMD_WFS_PIN_NODATA(LPVOID lpQueryDetails, LPVOID &lpCopyCmdData);
    HRESULT Get_WFSXDATA(LPWFSXDATA lpXData, CAutoWFMFreeBuffer *autofree, LPVOID lpSourceData, LPWFSXDATA &lpCopyCmdData);
    HRESULT Get_String(LPCSTR lpString, CAutoWFMFreeBuffer *autofree, LPVOID lpSourceData, LPSTR &lpCopyCmdData);
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

    //Manager申请出参内存
    //信息命令
    HRESULT Fmt_WFSPINSTATUS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINCAPS(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINKEYDETAIL(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINFUNCKEYDETAIL(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINKEYDETAILEX(LPVOID lpData, LPWFSRESULT &lpResult);
    //执行命令
    HRESULT Fmt_WFSPINDATA(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINENCIO(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINKCV(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINSTARTKEYEXCHANGE(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINEXPORTRSAISSUERSIGNEDITEMOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINIMPORTRSASIGNEDDESKEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINEXPORTRSAEPPSIGNEDITEMOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINENTRY(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPINIMPORTRSAPUBLICKEYOUTPUT(LPVOID lpData, LPWFSRESULT &lpResult);
    //事件
    HRESULT Fmt_WFSPINKEY(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSPININIT(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSHWERROR(LPWFSRESULT &lpResult, LPVOID lpData);
    HRESULT Fmt_WFSDEVSTATUS(LPWFSRESULT &lpResult, LPVOID lpData);

    HRESULT Fmt_WFSXDATA(LPVOID lpData, LPWFSRESULT &lpResult);
    HRESULT Fmt_WFSXDATA(CAutoSIMFreeBuffer &_auto, const LPWFSXDATA lpXData, LPWFSXDATA &lpNewXData, LPWFSRESULT &lpResult);
private:
    // 加载库
    bool LoadDll();
    HRESULT Fmt_ExtraStatus(LPWFSRESULT &lpResult, LPSTR &lpszNewExtra, LPCSTR lpszOldExtra);
private:
    CQtDLLLoader<IWFMShareMenory> m_pIWFM;
};

