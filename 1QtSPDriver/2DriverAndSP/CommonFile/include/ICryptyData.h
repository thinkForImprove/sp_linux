#ifndef QTTYPEDEF_H
#define QTTYPEDEF_H

#include "QtTypeDef.h"

struct  ICryptData
{
    virtual void Release() = 0;
    virtual QByteArray GetMD5(QByteArray qaryFileName) = 0;
    virtual void EncryptData(const QByteArray &qAryDataIn, QByteArray& qAryDataOut) = 0;
    virtual void DecryptData(const QByteArray &qAryDataIn, QByteArray& qAryDataOut) = 0;
};

extern "C" int CreateCryptData(ICryptData *&pInst);
#endif
