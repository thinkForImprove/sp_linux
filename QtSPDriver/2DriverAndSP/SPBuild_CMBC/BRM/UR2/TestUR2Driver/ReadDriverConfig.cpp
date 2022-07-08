#include "ReadDriverConfig.h"
#include <QSettings>
#include <QStringList>
#include <QtDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>
#include <QCoreApplication>
#include <assert.h>

ReadDriverConfig::ReadDriverConfig(QObject *parent) : QObject(parent)
{

}

bool ReadDriverConfig::LoadConfigFile()
{
    QString configFilePath = "UR2DriverConfig.ini";
    //    QString configFilePath = "/usr/local/ndt/bin/HCM2DriverConfig.ini";

    QFile configFile(configFilePath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        qDebug("打开配置文件失败！");
        return FALSE;
    }

    QTextStream txtInput(&configFile);
    QStringList lineStr;
    while (!txtInput.atEnd())
    {
        QString tempStr = txtInput.readLine();
        if (!IsDataLine(tempStr))
        {
            continue;
        }
        lineStr.append(tempStr);
    }

    configFile.close();

    int iSectionPos = FindSectionNamePos(lineStr, 0);
    QString ListType;//用于保存List数组的类型字w符串
    while (-1 != iSectionPos)
    {
        int iNextSectionPos = FindSectionNamePos(lineStr, iSectionPos + 1);

        for (int i = iSectionPos + 1;
             (iNextSectionPos == -1) ? i < lineStr.length() : i < iNextSectionPos;)
        {
            if (IsListType(lineStr[i]))
            {
                int iIndex = lineStr[i].indexOf("=");
                if (iIndex <= 0)
                {
                    return false;
                }
                i++;//跳过Type=List这一行
                iIndex = lineStr[i].indexOf("=");
                ListType = lineStr[i].mid(iIndex + 1);

                i++;//进入数据行
                iIndex = lineStr[i].indexOf("=");
                QString strNextType = lineStr[i].left(iIndex);
                while (i < iNextSectionPos && !strNextType.endsWith(".Type")) //出现一个新类型，即数组已结束(至少有一个数据行)
                {
                    AnalysisValue(lineStr[iSectionPos], ListType, lineStr[i], true);
                    iIndex = lineStr[++i].indexOf("=");
                    strNextType = lineStr[i].left(iIndex);
                }
            }
            else
            {
                AnalysisValue(lineStr[iSectionPos], lineStr[i], lineStr[i + 1]);
                i = i + 2;
            }
        }

        iSectionPos = iNextSectionPos;
    }

    return TRUE;
}

bool ReadDriverConfig::IsDataLine(QString &str)
{
    QString tempStr = str;
    if (tempStr.isEmpty()) //空行
    {
        return FALSE;
    }

    if ("//" == tempStr.mid(0, 2)) //注释行
    {
        return FALSE;
    }

    int index = tempStr.indexOf("//");  //去除行尾注释
    if (-1 != index)
    {
        str = str.mid(0, index);
    }

    str.replace(" ", ""); //去除所有空格

    return TRUE;
}

int ReadDriverConfig::FindSectionNamePos(QStringList &data, int iFromPositon)
{
    for (int i = iFromPositon; i < data.length(); i++)
    {
        QString temp = data[i];
        if (temp.contains("[") && temp.contains("]"))
        {
            data[i].replace("[", "");
            data[i].replace("]", "");
            return i;
        }
    }

    return -1;
}

bool ReadDriverConfig::AnalysisValue(QString section, QString type, QString data, bool bHasType)
{
    QString tempType = type;
    QString tempData = data;

    tempData.replace(".", "_");//将数据项的格式变换成QT的格式

    int i = tempType.indexOf("=");
    int j = tempData.indexOf("=");
    if ((!bHasType && (-1 == i || 0 == i || data.length() - 1 == i))
        || -1 == j || 0 == j || data.length() - 1 == j)
    {
        return FALSE;
    }

    QString strTypeRight = bHasType ? type : tempType.mid(i + 1, tempType.length());
    QString strDataLef = tempData.left(j);
    QString strDataRight = tempData.mid(j + 1, tempData.length());

    QString CtrlName = strTypeRight + "_" + section + "_" + strDataLef;

    emit SetValue(strTypeRight, CtrlName, strDataRight);

    return TRUE;
}

bool ReadDriverConfig::IsListType(QString str)
{
    int i = str.indexOf("=");
    if (i <= 0)
    {
        return false;
    }
    QString strTypeLef = str.left(i);
    QString strTypeRight = str.mid(i + 1, str.length());
    if (strTypeLef.endsWith(".Type") && strTypeRight == "List")
    {
        return true;
    }

    return false;
}

void ReadDriverConfig::GetPrivateProfileString(LPCSTR lpSection, LPCSTR lpKey,
                                               LPCSTR lpDefault, char *lpRetString,
                                               int iSize, LPCSTR lpFileName)
{
    assert(iSize >= 0);

    QFile qFile(lpFileName);
    if (!qFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        memcpy(lpRetString, lpDefault, iSize);
        return;
    }

    QTextStream txtInput(&qFile);
    QStringList lineStr;
    while (!txtInput.atEnd())
    {
        QString tempStr = txtInput.readLine();
        if (!IsDataLine(tempStr))
        {
            continue;
        }
        lineStr.append(tempStr);
    }
    qFile.close();

    int iIndex = 0;
    int iPos = 0;
    for (; iPos < lineStr.length(); iPos++)
    {
        if (lineStr[iPos].contains(lpSection) && lineStr[iPos].contains("[")
            && lineStr[iPos].contains("]"))
        {
            iIndex++;
        }
    }
    if (iIndex == 0)
    {
        memcpy(lpRetString, lpDefault, iSize);
        return;
    }

    for (int i = 0; i < lineStr.length(); i++)
    {
        if (lineStr[i].contains(lpKey))
        {
            int iTemp = lineStr[i].indexOf("=");
            if (iTemp <= 0)
            {
                continue;
            }
            QString strValue = lineStr[i].mid(iTemp + 1);
            strValue.replace(" ", "");//去除空格
            if (strValue.length() > iSize)
            {
                memcpy(lpRetString, lpDefault, iSize);
                return;
            }
            memcpy(lpRetString, qPrintable(strValue), strValue.length());
            return;
        }
    }
}

