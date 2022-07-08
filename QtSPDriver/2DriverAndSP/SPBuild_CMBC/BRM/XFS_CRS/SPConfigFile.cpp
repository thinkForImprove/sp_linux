#include "SPConfigFile.h"
#include <sys/stat.h>
#include <sys/types.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include "XFSAPI.H"


#define ERR_STRING_VALUE            "ERR~!@#"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CSPConfigFile::CSPConfigFile()
{

}

CSPConfigFile::~CSPConfigFile()
{

}

int CSPConfigFile::Load(LPCSTR lpszFileName)
{
    m_bLoad = m_configfile.LoadINIFile(lpszFileName);
    return m_bLoad ? WFS_SUCCESS : WFS_ERR_INTERNAL_ERROR;
}

LPCSTR CSPConfigFile::GetString(LPCSTR lpszKeyName, LPCSTR lpszValueName, LPCSTR lpszDefault)
{
    if (!m_bLoad)
        return "";
    CINIReader cINI = m_configfile.GetReaderSection(lpszKeyName);
    return (LPCSTR)cINI.GetValue(lpszValueName, lpszDefault);
}

//设置字串值
//lpszKeyName：键名，INI文件中［］中的内容
//lpszValueName：值名，INI文件中等号前的内容
//lpszValue：要写入的值
BOOL CSPConfigFile::SetString(LPCSTR lpszKeyName, LPCSTR lpszValueName, LPCSTR lpszValue)
{
    if (!m_bLoad)
        return FALSE;
    CINIWriter cINI = m_configfile.GetWriterSection(lpszKeyName);
    cINI.SetValue(lpszValueName, lpszValue);
    return TRUE;
}

int CSPConfigFile::GetInt(LPCSTR lpszKeyName, LPCSTR lpszValueName, int nDefault)
{
    if (!m_bLoad)
        return -1;
    char szBuff[256];
    sprintf(szBuff, "%d", nDefault);
    CINIReader cINI = m_configfile.GetReaderSection(lpszKeyName);
    return (int)cINI.GetValue(lpszValueName, szBuff);
}
