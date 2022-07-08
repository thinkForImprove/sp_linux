#ifndef READDRIVERCONFIG_H
#define READDRIVERCONFIG_H

#include <QObject>
#include <QString>
//#include "QtTypeDef.h"

class ReadDriverConfig : public QObject
{
    Q_OBJECT

public:
    explicit ReadDriverConfig(QObject *parent = 0);

    friend class UR2DriverTest;

public:
    bool LoadConfigFile();
    //获取键值
    void GetPrivateProfileString(LPCSTR lpSection, LPCSTR lpKey, LPCSTR lpDefault, char *lpRetString, int iSize, LPCSTR lpFileName);
signals:
    void SetValue(QString, QString, QString);

private:
    bool IsDataLine(QString &str);
    int FindSectionNamePos(QStringList &data, int iFromPositon);
    bool AnalysisValue(QString section, QString type, QString data, bool bHasType = false);
    bool IsListType(QString str);

};

#endif // READDRIVERCONFIG_H
