#include "PTRData.h"
#include <string.h>
#include <sys/dir.h>
#include <unistd.h>
#include <sys/stat.h>

// 事件日志
#define ThisFile                    "CSPPtrData"

#define IDS_ERR_CONFIG_VALUE        "SP配置项[%s]配置错误[%s]"
#define IDS_ERR_OPENFORMPATH_ERR    "打开FORM目录[%s]失败"

#ifdef Q_OS_WIN32
#define FORMPATH                    "C:/CFES/FORM/PTR"
#else
#define FORMPATH                    "/usr/local/CFES/DATA/FORM"
#endif

#define FORM_FILE_EXT_KEY           "ptr_formfile_ext"
#define MEDIA_FILE_EXT_KEY          "ptr_mediafile_ext"
#define FORM_FILE_DEFAULT_EXT       "wfm"
#define MEDIA_FILE_DEFAULT_EXT      "wfm"

CSPPtrData::CSPPtrData(LPCSTR lpLogicalName)
{
    m_FormFileLastChangeTime    = 0;
    m_MediaFileLastChangeTime   = 0;

    m_cXfsReg.SetLogicalName(lpLogicalName);
    m_strFormName  = m_cXfsReg.GetValue("CONFIG", FORMFILEVALUENAME, FORMFILEDEFAULT);
    m_strMediaName = m_cXfsReg.GetValue("CONFIG", MEDIAFILEVALUENAME, MEDIAFILEDEFAULT);
    m_strFormKey = m_cXfsReg.GetValue("CONFIG", FORM_FILE_EXT_KEY, FORM_FILE_DEFAULT_EXT);
    m_strMediaKey = m_cXfsReg.GetValue("CONFIG", MEDIA_FILE_EXT_KEY, MEDIA_FILE_DEFAULT_EXT);

    SetLogFile(LOGFILE, ThisFile, "PTR");
}
CSPPtrData::~CSPPtrData()
{}

BOOL CSPPtrData::LoadForms()
{
    const char *const ThisModule = "DATA.LoadForms";
    //从注册表中读取FORM文件名
    string strFile = m_strFormName;
    BOOL bIsDir = FALSE;
    if (strFile.empty())
    {
        strFile = FORMPATH;
        bIsDir = TRUE;
    }
    else
    {
        struct stat st;
        int iRes = lstat(strFile.c_str(), &st);
        if (0 > lstat(strFile.c_str(), &st))
        {
            Log(ThisModule, -1, IDS_ERR_CONFIG_VALUE, FORMFILEVALUENAME, strFile.c_str());
            return FALSE;
        }
        else
        {
            bIsDir = S_ISDIR(st.st_mode);
        }
    }

    if (bIsDir)
    {
        if (strFile[strFile.length() - 1] != '/') strFile.append(1, '/');

        DIR *pDir;
        pDir = opendir(strFile.c_str());
        if (NULL == pDir)
        {
            Log(ThisModule, -1, IDS_ERR_OPENFORMPATH_ERR, strFile.c_str());
            return FALSE;
        }

        m_FormList.Clear();
        string strFileExt = m_strFormKey;
        struct dirent *pDirent;
        struct stat statbuf;
        while (NULL != (pDirent = readdir(pDir)))
        {
            string strFileFullName = strFile + pDirent->d_name;
            lstat(strFileFullName.c_str(), &statbuf);
            if (!S_ISDIR(statbuf.st_mode) && strlen(pDirent->d_name) > strFileExt.length()) // 只关注文件，且文件名长度大于后缀长度
            {
                string strSubFile(pDirent->d_name);
                if (strSubFile.substr(strSubFile.length() - strFileExt.length(), strFileExt.length()).compare(strFileExt) == 0) // 检查文件后缀
                {
                    m_FormList.Load(strFileFullName.c_str(), true);
                }
            }
        }
        closedir(pDir);
    }
    else
    {
        m_FormList.Clear();
        m_FormList.Load(strFile.c_str(), true);
    }

    return TRUE;
}
BOOL CSPPtrData::LoadMedias()
{
    const char *const ThisModule = "DATA.LoadMedia";
    string strFile = m_strMediaName;
    BOOL bIsDir = FALSE;
    if (strFile.empty())
    {
        strFile = FORMPATH;
        bIsDir = TRUE;
    }
    else
    {
        struct stat st;
        if (0 > lstat(strFile.c_str(), &st))
        {
            Log(ThisModule, -1, IDS_ERR_CONFIG_VALUE, MEDIAFILEVALUENAME, strFile.c_str());
            return FALSE;
        }
        else
        {
            bIsDir = S_ISDIR(st.st_mode);
        }
    }

    if (bIsDir)
    {
        if (strFile[strFile.length() - 1] != '/') strFile.append(1, '/');

        DIR *pDir;
        pDir = opendir(strFile.c_str());
        if (NULL == pDir)
        {
            Log(ThisModule, -1, IDS_ERR_OPENFORMPATH_ERR, strFile.c_str());
            return FALSE;
        }

        m_MediaList.Clear();
        string strFileExt = m_strMediaKey;
        struct dirent *pDirent;
        struct stat statbuf;
        while (NULL != (pDirent = readdir(pDir)))
        {
            string strFileFullName = strFile + pDirent->d_name;
            lstat(strFileFullName.c_str(), &statbuf);
            if (!S_ISDIR(statbuf.st_mode) && strlen(pDirent->d_name) > strFileExt.length()) // 只关注文件，且文件名长度大于后缀长度
            {
                string strSubFile(pDirent->d_name);
                if (strSubFile.substr(strSubFile.length() - strFileExt.length(), strFileExt.length()).compare(strFileExt) == 0) // 检查文件后缀
                {
                    m_MediaList.Load(strFileFullName.c_str(), false);
                }
            }
        }
        closedir(pDir);
    }
    else
    {
        m_MediaList.Clear();
        m_MediaList.Load(strFile.c_str(), false);
    }

    return TRUE;
}

HRESULT CSPPtrData::FindField(LPCSTR lpszForm, LPCSTR lpszField)
{
    THISMODULE(__FUNCTION__);
    CSPPrinterForm *pForm = FindForm(lpszForm);
    if (!pForm)
    {
        Log(ThisModule, -1, "Form[%s] not found", lpszForm);
        return WFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!pForm->IsLoadSucc())
    {
        Log(ThisModule, -1, "Form[%s] load failed", lpszForm);
        return WFS_ERR_PTR_FORMINVALID;
    }
    m_LastFields.Clear();
    BOOL bRet = m_LastFields.ExtractFromForm(pForm, lpszField);
    if (!bRet)
    {
        Log(ThisModule, -1, "field[%s] not found", lpszField);
        return WFS_ERR_PTR_FIELDNOTFOUND;
    }

    return WFS_SUCCESS;
}
