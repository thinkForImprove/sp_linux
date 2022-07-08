#pragma once
#include "AutoQtHelpClass.h"
#include "md5.h"
#include "DES.h"
#include "ILogWrite.h"
#include "ICryptyData.h"
#include <string.h>

class CEncryptData : public CLogManage, public ICryptData
{
public:
    CEncryptData();
    ~CEncryptData();

    void Release();
    QByteArray GetMD5(QByteArray qaryFileName);
    void EncryptData(const QByteArray &qAryDataIn, QByteArray& qAryDataOut);
    void DecryptData(const QByteArray &qAryDataIn, QByteArray& qAryDataOut);

private:
    CAutoHex m_cAutoHex;
    CCDES m_cDes;
};
