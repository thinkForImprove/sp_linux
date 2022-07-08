#include "CryptyData.h"

#define Key "CFES20200429TUCY"

CEncryptData::CEncryptData()
{
    SetLogFile(LOGFILE, "CEncryptData", "UR");
}

CEncryptData::~CEncryptData()
{

}

void CEncryptData::Release()
{

}

QByteArray CEncryptData::GetMD5(QByteArray qaryFileName)
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();
    MD5 md5;
    string strmd5 = md5.DigestFile(qaryFileName.constData());
    QByteArray vtHexMD5;
    m_cAutoHex.Bcd2Hex((const BYTE*)strmd5.c_str(), strmd5.size(), vtHexMD5);
    return vtHexMD5;
}

void CEncryptData::EncryptData(const QByteArray &qAryDataIn, QByteArray& qAryDataOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    ULONG ulLen = 0;
    ULONG ulDataLen = 0;
    do
    {
        if(qAryDataIn.size() - ulLen > 1024)
            ulDataLen = 1024;
        else
            ulDataLen = qAryDataIn.size() - ulLen;

        QByteArray qAryData = qAryDataIn.mid(ulLen, ulDataLen);
        ulLen += ulDataLen;
        QByteArray vtHexIn;
        m_cAutoHex.Bcd2Hex((const BYTE*)qAryData.constData(), qAryData.size(), vtHexIn);
        char szDataBuff[10240] = {0};
        m_cDes.gDes((char*)vtHexIn.constData(), Key, szDataBuff);
        QByteArray vtHexOutData;
        m_cAutoHex.Hex2Bcd(szDataBuff, vtHexOutData);
        qAryDataOut += vtHexOutData;

        if(ulLen >= qAryDataIn.size())
        {
            break;
        }
    }while(true);

    return;
}

void CEncryptData::DecryptData(const QByteArray &qAryDataIn, QByteArray& qAryDataOut)
{
    THISMODULE(__FUNCTION__);
    //AutoLogFuncBeginEnd();

    ULONG ulLen = 0;
    ULONG ulDataLen = 0;
    do
    {
        if(qAryDataIn.size() - ulLen > 1024)
            ulDataLen = 1024;
        else
            ulDataLen = qAryDataIn.size() - ulLen;

        QByteArray qAryData = qAryDataIn.mid(ulLen, ulDataLen);
        ulLen += ulDataLen;
        QByteArray vtHexIn;
        m_cAutoHex.Bcd2Hex((const BYTE*)qAryData.constData(), qAryData.size(), vtHexIn);
        char szDataBuff[10240] = {0};
        m_cDes.gUndes((char*)vtHexIn.constData(), Key, szDataBuff);
        QByteArray vtHexOutData;
        m_cAutoHex.Hex2Bcd(szDataBuff, vtHexOutData);
        qAryDataOut += vtHexOutData;

        if(ulLen >= qAryDataIn.size())
        {
            break;
        }
    }while(true);

    return;
}

