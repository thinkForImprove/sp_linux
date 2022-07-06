#include "spptrdata.h"
#include "string.h"
#include "log_lib.h"
#include "ConvertXDTPath.h"
#include "ConvertXDTPath.cpp"

#include <sys/dir.h>
#include <unistd.h>
#include <sys/stat.h>

// 事件日志
#define ThisFile                    "SPBasePrinter"
#define EVENT_LOG                   "Event.log"

#define IDS_ERR_CONFIG_VALUE        "SP配置项[%s]配置错误[%s]"
#define IDS_ERR_OPENFORMPATH_ERR    "打开FORM目录[%s]失败"

#define FORMPATH                    "/etc/ndt/form/ptr_form/"
#define FORM_FILE_EXT_KEY           "ptr_formfile_ext"
#define MEDIA_FILE_EXT_KEY          "ptr_mediafile_ext"
#define FORM_FILE_DEFAULT_EXT       "wfm"
#define MEDIA_FILE_DEFAULT_EXT      "wfm"

CSPPtrData::CSPPtrData(CSPBasePrinter *pBasePrinter)
{
    m_FormFileLastChangeTime    = 0;
    m_MediaFileLastChangeTime   = 0;
    m_pBasePrinter = pBasePrinter;
}
CSPPtrData::~CSPPtrData()
{}

BOOL CSPPtrData::LoadForms()
{
    const char *const ThisModule = "DATA.LoadForms";
    //从注册表中读取FORM文件名
    string strFile = GetSPIniValue(m_pBasePrinter->GetSPName(), "default", FORMFILEVALUENAME, FORMFILEDEFAULT);
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
            log_write(EVENT_LOG, ThisFile, ThisModule, -1, IDS_ERR_CONFIG_VALUE, FORMFILEVALUENAME, strFile.c_str());
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
            log_write(EVENT_LOG, ThisFile, ThisModule, -1, IDS_ERR_OPENFORMPATH_ERR, strFile.c_str());
            return FALSE;
        }

        m_FormList.Clear();
        string strFileExt = GetSPIniValue(m_pBasePrinter->GetSPName(), "default", FORM_FILE_EXT_KEY, FORM_FILE_DEFAULT_EXT);
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
    // 从注册表中读取Media文件名
    string strFile = GetSPIniValue(m_pBasePrinter->GetSPName(), "default", MEDIAFILEVALUENAME, MEDIAFILEDEFAULT);
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
            log_write(EVENT_LOG, ThisFile, ThisModule, -1, IDS_ERR_CONFIG_VALUE, MEDIAFILEVALUENAME, strFile.c_str());
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
            log_write(EVENT_LOG, ThisFile, ThisModule, -1, IDS_ERR_OPENFORMPATH_ERR, strFile.c_str());
            return FALSE;
        }

        m_MediaList.Clear();
        string strFileExt = GetSPIniValue(m_pBasePrinter->GetSPName(), "default", MEDIA_FILE_EXT_KEY, MEDIA_FILE_DEFAULT_EXT);
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
    CSPPrinterForm *pForm = FindForm(lpszForm);
    if (!pForm)
    {
        return LFS_ERR_PTR_FORMNOTFOUND;
    }
    if (!pForm->IsLoadSucc())
    {
        return LFS_ERR_PTR_FORMINVALID;
    }
    m_LastFields.Clear();
    BOOL bRet = m_LastFields.ExtractFromForm(pForm, lpszField);
    if (!bRet)                                         //add by lvcb
    {
        return LFS_ERR_PTR_FIELDNOTFOUND;
    }

    return LFS_SUCCESS;
}
