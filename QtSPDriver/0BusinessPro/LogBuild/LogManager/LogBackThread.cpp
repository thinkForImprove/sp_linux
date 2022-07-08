// LogBackThread.cpp: implementation of the CLogBackThread class.
//
//////////////////////////////////////////////////////////////////////
#include "LogWriteThread.h"
#include <QDir>
#include <stdlib.h>
//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////////
CLogBackThread::CLogBackThread()
{
    SetLogFile(LOG_BACK_NAME, "LogBackThreaed.cpp", "LOG_BACK");
}

CLogBackThread::~CLogBackThread()
{
    Exit();
}

void CLogBackThread::Exit()
{
    ThreadStop();
}

void CLogBackThread::Run()
{
    THISMODULE(__FUNCTION__);

    GetIniConfig();                             // 读配置文件

    if (m_wLogBackDelSup == 0)                  // 不支持备份/删除功能,直接退出
    {
        return;
    }

    CHAR szCurDate[8 + 1] = { 0x00 };
    QDate qCurDate;

    // 循环,根据日期检查备份删除
    while (true)
    {
        // 每次执行前,加载一次配置文件
        //AutoGetIniConfig();

        // 取 当前日期
        qCurDate = QDate::currentDate();
        memset(szCurDate, 0x00, sizeof(szCurDate));
        sprintf(szCurDate, "%4d%02d%02d", qCurDate.year(), qCurDate.month(), qCurDate.day());

        if (strlen(m_szLogBackDate) < 8 ||                  // 初始备份
            memcmp(m_szLogBackDate, szCurDate, 8) < 0)      // 当天之前未执行备份删除
        {
            if (m_wLogBackDelSup == 1)                                  // 支持备份功能
            {
                BackupLog(m_szLogRootDir, m_szLogBackDir, szCurDate);   // Log备份处理
                DeleteLog(m_szLogBackDir, szCurDate);                   // Log删除处理(备份目录)
            } else                                                      // 只支持删除功能
            {
                DeleteLog(m_szLogRootDir, szCurDate);                   // Log删除处理(Log目录)
            }

            // 备份日期写入INI, 更新当前日期变量
            Log(ThisModule, __LINE__, "CurrDate[%s]备份完成, 写入INI: [LOG_BACK_CFG]->LogBackDate = %s.",
                szCurDate, szCurDate);
            CINIWriter clWriteINI = m_clRWIni.GetWriterSection("LOG_BACK_CFG");
            clWriteINI.AddValue("LogBackDate", szCurDate);
            clWriteINI.Save();
            memcpy(m_szLogBackDate, szCurDate, strlen(szCurDate));
        }

        // 判断是否退出
        if (m_bQuitRun)
        {
            Log(ThisModule, __LINE__, "进程结束退出.");
            break;
        }

        // 休眠1分钟
        std::this_thread::sleep_for(std::chrono::milliseconds(1000 * 60 * 1));
    }

    return;
}

/*****************************************************************
 * 功能: Log备份主处理接口
 * 入参: 无
 * 回参: 无
 * 返回值: true(成功), false(失败)
*****************************************************************/
bool CLogBackThread::BackupLog(LPCSTR lpLogPath, LPCSTR lpBackPath, LPCSTR lpDate)
{
    THISMODULE(__FUNCTION__);

    QDir qtDir(lpLogPath);                                      // 源目录设置
    qtDir.setFilter(QDir::Dirs | QDir::Files);                  // 设置过滤条件: 只取目录和文件
    qtDir.setSorting(QDir::DirsFirst);                          // 设置排序: 文件夹排在前面
    QFileInfoList qtInfoList = qtDir.entryInfoList();           // 取得源目录下 目录+文件 列表

    std::string stdSource = "", stdDest = "", stdCompress = ""; // 源目录, 目的目录, 压缩文件
    std::string stdDFName = "";                                 // 截取的目录名/文件名
    CHAR szDate[32];
    int nFileCnt = 0;                                           // 处理 目录+文件 计数
    bool bIsDir = false;
    bool bIsOk = true;
    INT nRet = 0;
    CHAR szErrStr[1024] = { 0x00 };

    Log(ThisModule, __LINE__, "备份开始: CurrDate = %s, BackDate < %s, BackPath[%s]->[%s].",
        lpDate, lpDate, lpLogPath, lpBackPath);

    // 循环处理源目录下 文件列表
    while (nFileCnt < qtInfoList.size())
    {
        QFileInfo qtFileInfo = qtInfoList.at(nFileCnt);         // 取得列表数据
        nFileCnt++;                                             // 处理 目录+文件 计数 +1
        if (qtFileInfo.fileName() == "." || qtFileInfo.fileName() == "..")  // .OR..不处理
        {
            continue;
        }

        // 目录/文件, 验证日期
        if (qtFileInfo.isDir() || qtFileInfo.isFile())
        {
            // 生成该目录文件的原路径
            stdSource.clear();
            stdSource.append(QString2Char(qtFileInfo.filePath()));

            // 生成该目录文件的备份路径
            stdDest.clear();
            stdDest.append(lpBackPath);

            // 取目录文件名,用于关键字检查和时间比较
            stdDFName.clear();
            if (qtFileInfo.isDir())
            {
                stdDFName.append(QString2Char(qtFileInfo.filePath().mid(strlen(lpLogPath) + 1)));
                bIsDir = true;
            } else
            {
                stdDFName.append(QString2Char(qtFileInfo.fileName()));
                bIsDir = false;
            }

            // 组合备份路径/文件
            stdDest.append("/");
            stdDest.append(stdDFName.c_str());

            if (strlen(szErrStr) > 1)
            {
                Log(ThisModule, __LINE__, "%s", szErrStr);
                memset(szErrStr, 0x00, sizeof(szErrStr));
            }

            sprintf(szErrStr + strlen(szErrStr), "备份[%s]->[%s]: ", stdSource.c_str(), stdDest.c_str());

            // 目录文件名关键字检查(是否不需要备份删除)
            if (m_stNotBackList.IsHave((LPSTR)stdDFName.c_str()) != 0)
            {
                sprintf(szErrStr + strlen(szErrStr), "不备份(INI设置不需要备份).");
                continue;   // 不需要备份,跳出,进入下一循环
            }

            // 通用Log为YYYYMMDD格式的目录,检查是通用Log处理
            if (ChkIsDate((LPSTR)stdDFName.c_str()) == true)
            {
                // 比较目录文件与备份日期, < 备份日期时执行备份处理
                if (memcmp(stdDFName.c_str(), lpDate, 8) >= 0)
                {
                    sprintf(szErrStr + strlen(szErrStr), "不备份(FileName >= BackData[%s],不在备份日期范围内).", lpDate);
                    continue;   // 不需要备份,跳出,进入下一循环
                }
            } else  // 非通用Log,根据INI设置其他备份关键字确认是否备份
            {
                if (m_stOtherBackList.IsHave("***") == 0 &&
                    m_stOtherBackList.IsHave((LPSTR)stdDFName.c_str()) == 0)
                {
                    sprintf(szErrStr + strlen(szErrStr), "不备份(INI设定不在备份列表内).");
                    continue;   // 不需要备份,跳出,进入下一循环
                }

                // 需要备份,获取目录文件创建日期进行比较
                memset(szDate, 0x00, sizeof(szDate));
                if (GetFileCreateDate((LPSTR)stdSource.c_str(), szDate) != true)
                {
                    sprintf(szErrStr + strlen(szErrStr), "不备份(获取创建日期失败).");
                    continue;   // 获取目录文件创建日期失败,跳出,进入下一循环
                }

                // 比较目录文件创建日期与备份日期, < 备份日期时执行备份处理
                if (memcmp(szDate, lpDate, 8) >= 0)
                {
                    sprintf(szErrStr + strlen(szErrStr), "不备份(创建日期[%s] >= 备份范围日期[%s]).", szDate, lpDate);
                    continue;   // 不需要备份,跳出,进入下一循环
                }
            }

            // 1: 备份目录/文件
            if ((m_wLogBackMode & 1) == 1)
            {
                // 拷贝目录或文件
                nRet = CopySource2Dest((LPSTR)stdSource.c_str(), (LPSTR)stdDest.c_str(),
                                       qtFileInfo.isDir());
                if (nRet != 0)
                {
                    sprintf(szErrStr + strlen(szErrStr), "失败, CopySource2Dest() = %d.", nRet);
                    bIsOk = false;
                    continue;
                }
                sprintf(szErrStr + strlen(szErrStr), "完成.");
            }

            // 2: 压缩并备份
            if ((m_wLogBackMode & 2) == 2)
            {
                stdCompress.clear();
                stdCompress.append(stdDest);
                stdCompress.append(".zip");

                // 压缩目录
                if (FileAccess::linux_zip_compress(0/*压缩*/, stdCompress.c_str()/*目标文件*/,
                                                   stdSource.c_str()/*源目录*/, bIsDir) != true)
                {
                    Log(ThisModule, __LINE__, "打包备份[%s]->[%s] 失败.", stdSource.c_str(), stdCompress.c_str());
                    bIsOk = false;
                    continue;
                }
                Log(ThisModule, __LINE__, "打包备份[%s]->[%s] 完成.", stdSource.c_str(), stdCompress.c_str());
            }

            // 备份完成后是否删除源目录文件
            if (m_wLogDelSource == 1)
            {
                if (DelDirFile(stdSource.c_str()) != 0)
                {
                    Log(ThisModule, __LINE__, "删除[%s] 失败.", stdSource.c_str());
                    bIsOk = false;
                    continue;
                }
                Log(ThisModule, __LINE__, "删除[%s] 完成.", stdSource.c_str());
            }
        }
    }

    if (strlen(szErrStr) > 1)
    {
        Log(ThisModule, __LINE__, "%s", szErrStr);
        memset(szErrStr, 0x00, sizeof(szErrStr));
    }

    return bIsOk;
}

/*****************************************************************
 * 功能: Log删除处理主接口
 * 入参: lpLogPath 指定目录    bBackDel: true(备份删除方式)/false(只删除方式)
 * 回参: 无
 * 返回值: true(成功), false(失败)
*****************************************************************/
bool CLogBackThread::DeleteLog(LPCSTR lpLogPath, LPCSTR lpDate, bool bBackDel)
{
    THISMODULE(__FUNCTION__);

    char szDelDate[8+1] = { 0x00 };    // 删除日期

    // 取 当前日期 - log保存日期
    QDate qDate = QDate::fromString(lpDate, "yyyyMMdd").addDays(0 - m_wLogBackSaveDays);

    memset(szDelDate, 0x00, sizeof(szDelDate));
    sprintf(szDelDate, "%4d%02d%02d", qDate.year(), qDate.month(), qDate.day());

    Log(ThisModule, __LINE__, "删除开始: CurrDate = %s, SaveDays = %d, DelDate < %s, DelPath = %s.",
        lpDate, m_wLogBackSaveDays, szDelDate, lpLogPath);

    QDir qtDir(lpLogPath);                                      // 源目录设置
    qtDir.setFilter(QDir::Dirs | QDir::Files);                  // 设置过滤条件: 只取目录和文件
    qtDir.setSorting(QDir::DirsFirst);                          // 设置排序: 文件夹排在前面
    QFileInfoList qtInfoList = qtDir.entryInfoList();           // 取得源目录下 目录+文件 列表

    std::string stdDelete = "";                                 // 删除目录/文件
    std::string stdDFName = "";                                 // 截取的目录名/文件名
    CHAR szDate[32];
    int nFileCnt = 0;                                           // 处理 目录+文件 计数
    bool bIsOk = true;
    CHAR szErrStr[1024] = { 0x00 };

    // 循环处理源目录下 文件列表
    while (nFileCnt < qtInfoList.size())
    {
        QFileInfo qtFileInfo = qtInfoList.at(nFileCnt);         // 取得列表数据
        nFileCnt++;                                             // 处理 目录+文件 计数 +1
        if (qtFileInfo.fileName() == "." || qtFileInfo.fileName() == "..")  // .OR..不处理
        {
            continue;
        }

        if (strlen(szErrStr) > 1)
        {
            Log(ThisModule, __LINE__, "%s", szErrStr);
            memset(szErrStr, 0x00, sizeof(szErrStr));
        }

        // 生成该目录的原路径和备份路径
        stdDelete.clear();
        stdDelete.append(QString2Char(qtFileInfo.filePath()));
        sprintf(szErrStr + strlen(szErrStr), "删除[%s]: ", stdDelete.c_str());

        // 取目录名/文件名,用于时间比较
        stdDFName.clear();
        if (qtFileInfo.isDir())
        {
            stdDFName.append(QString2Char(qtFileInfo.filePath().mid(strlen(lpLogPath) + 1)));
        } else
        {
            stdDFName.append(QString2Char(qtFileInfo.fileName()));
        }

        // 目录文件名关键字检查(是否不需要删除)
        if (m_stNotBackList.IsHave((LPSTR)stdDFName.c_str()) != 0)
        {
            sprintf(szErrStr + strlen(szErrStr), "不删除(INI设定不删除).");
            continue;   // 不需要删除,跳出,进入下一循环
        }

        // 通用Log为YYYYMMDD格式的目录,检查是通用Log处理
        if (stdDFName.length() == 8 && qtFileInfo.isDir())
        {
            // 比较目录文件与删除日期, <= 删除日期时执行备份处理
            if (memcmp(stdDFName.c_str(), szDelDate, 8) > 0)
            {
                sprintf(szErrStr + strlen(szErrStr), "不删除(FileName > %s, 不在删除日期范围).", szDelDate);
                continue;   // 不需要删除,跳出,进入下一循环
            }
        } else
        if (stdDFName.length() > 8 && (stdDFName.compare(8, 4, ".zip") == 0) && qtFileInfo.isFile())
        {
            // 比较目录文件与删除日期, <= 删除日期时执行备份处理
            if (memcmp(stdDFName.substr(0, 8).c_str(), szDelDate, 8) > 0)
            {
                sprintf(szErrStr + strlen(szErrStr), "不删除(FileName > %s, 不在删除日期范围).", szDelDate);
                continue;   // 不需要删除,跳出,进入下一循环
            }
        } else  // 非通用Log,根据INI设置其他备份删除关键字确认是否删除
        {
            if (bBackDel != true)   // 只删除方式,需要检查Log根目录下那些参与删除
            {
                if (m_stOtherBackList.IsHave("***") == 0 &&
                    m_stOtherBackList.IsHave((LPSTR)stdDFName.c_str()) == 0)
                {
                    sprintf(szErrStr + strlen(szErrStr), "不删除(INI设定不在删除列表内).");
                    continue;   // 不需要删除,跳出,进入下一循环
                }
            }

            // 获取目录文件创建日期进行比较
            memset(szDate, 0x00, sizeof(szDate));
            if (GetFileCreateDate((LPSTR)stdDelete.c_str(), szDate) != true)
            {
                sprintf(szErrStr + strlen(szErrStr), "不删除(获取创建日期失败).");
                continue;   // 获取目录文件创建日期失败,跳出,进入下一循环
            }

            // 比较目录文件创建日期与备份日期, <= 备份日期时执行备份处理
            if (memcmp(szDate, szDelDate, 8) > 0)
            {
                sprintf(szErrStr + strlen(szErrStr), "不删除(创建日期[%s] > 删除范围日期[%s]).", szDate, szDelDate);
                continue;   // 不需要备份,跳出,进入下一循环
            }
        }

        // 删除目录文件
        if (DelDirFile(stdDelete.c_str()) != 0)
        {
            sprintf(szErrStr + strlen(szErrStr), "失败.");
            bIsOk = false;
            continue;
        }
        sprintf(szErrStr + strlen(szErrStr),"完成.");
    }

    if (strlen(szErrStr) > 1)
    {
        Log(ThisModule, __LINE__, "%s", szErrStr);
        memset(szErrStr, 0x00, sizeof(szErrStr));
    }

    return bIsOk;
}

/*****************************************************************
 * 功能: 拷贝目录或文件
 * 入参: lpSource: 源目录/源文件     lpDest: 目的目录/目的文件
 * 回参: 无
 * 返回值: 0成功, -1拷贝失败, -2文件数目不一致, -3文件总大小不一致
*****************************************************************/
INT CLogBackThread::CopySource2Dest(LPSTR lpSource, LPSTR lpDest, bool bIsCopyDir)
{
    if (bIsCopyDir == true)
    {
        // 源目录拷贝到备份目录
        if (FileAccess::copy_file_directory(lpSource, lpDest) != true)
        {
            return -1;
        }

        // 比较源目录与目的目录文件数目
        if (FileAccess::get_file_count(lpSource) != FileAccess::get_file_count(lpDest))
        {
            return -2;
        }

        // 比较源目录与目的目录所有文件大小
        if (FileAccess::posix_get_file_directory_size(lpSource) !=
            FileAccess::posix_get_file_directory_size(lpDest))
        {
            return -3;
        }
    } else
    {
        // 源目录拷贝到备份目录
        if (FileAccess::copy_file_directory(lpSource, lpDest, nullptr, nullptr, true, false) != true)
        {
            return -1;
        }

        // 比较源文件与目的文件大小
        if (GetFileSize(lpSource) != GetFileSize(lpDest))
        {
            return -3;
        }
    }

    return 0;
}

/*****************************************************************
 * 功能: 删除目录或文件
 * 入参: lpDirFile: 要删除的目录/源件
 * 回参: 无
 * 返回值: 0成功, -1删除失败
*****************************************************************/
INT CLogBackThread::DelDirFile(LPCSTR lpDirFile)
{
    // 删除
    if (FileAccess::delete_file_directory(lpDirFile) != true)
    {
        return -1;
    }

    // 验证删除


    return 0;
}

/*****************************************************************
 * 功能: 读配置文件
 * 入参: 无
 * 回参: 无
 * 返回值: 0成功
*****************************************************************/
int CLogBackThread::GetIniConfig()
{
    THISMODULE(__FUNCTION__);
    AutoLogFuncBeginEnd();

    CHAR szBuffer[MAX_BUFF_1024];
    INT  nTmp;
    CHAR szIniPar[MAX_BUFF_1024] = { 0x00 };

    m_clRWIni.LoadINIFile(INI_XFSLOG_PATH);                             // 加载配置文件
    CINIReader clReadINI = m_clRWIni.GetReaderSection("LOG_BACK_CFG");  // 加载备份配置项目

    // Log备份删除功能支持, 配置: 0/1/2, 缺省0
    m_wLogBackDelSup = (WORD)clReadINI.GetValue("LogBackDelSup", (DWORD)1);
    if (m_wLogBackDelSup < 0 || m_wLogBackDelSup > 2)
    {
        m_wLogBackDelSup = 0;
    }
    sprintf(szIniPar + strlen(szIniPar), "[LOG_BACK_CFG]->LogBackDelSup = %d", m_wLogBackDelSup);


    // 指定要备份的LOG目录,缺省/usr/local/LOG
    memset(m_szLogRootDir, 0x00, sizeof(m_szLogRootDir));
    strcpy(m_szLogRootDir, clReadINI.GetValue("LogRootDir", LOG_ROOT_PATH));
    if (strlen(m_szLogRootDir) < 2)
    {
        memcpy(m_szLogRootDir, LOG_ROOT_PATH, strlen(LOG_ROOT_PATH));
    }
    sprintf(szIniPar + strlen(szIniPar), ", [LOG_BACK_CFG]->LogRootDir = %s", m_szLogRootDir);

    // 指定LOG备份目录,缺省当前用户下的CFESLOG.BAK目录
    memset(m_szLogBackDir, 0x00, sizeof(m_szLogBackDir));
    strcpy(m_szLogBackDir, clReadINI.GetValue("LogBackDir", ""));
    if (strlen(m_szLogBackDir) < 1)
    {
        sprintf(m_szLogBackDir, "%s/CFESLOG.BAK", getenv("HOME"));
    }
    sprintf(szIniPar + strlen(szIniPar), ", [LOG_BACK_CFG]->LogBackDir = %s", m_szLogBackDir);

    // 已备份的日期,缺省空
    memset(szBuffer, 0x00, sizeof(szBuffer));
    strcpy(szBuffer, clReadINI.GetValue("LogBackDate", ""));
    memset(m_szLogBackDate, 0x00, sizeof(m_szLogBackDate));
    if (strlen(szBuffer) == 8)
    {
        memcpy(m_szLogBackDate, szBuffer, 8);
    }
    sprintf(szIniPar + strlen(szIniPar), ", [LOG_BACK_CFG]->LogBackDate = %s", m_szLogBackDate);

    // 备份方式,1:只备份目录, 2:目录压缩, 3:保留目录和目录压缩, 缺省3
    m_wLogBackMode = (WORD)clReadINI.GetValue("LogBackMode", (DWORD)3);
    if (m_wLogBackMode < 1 || m_wLogBackMode > 3)
    {
        m_wLogBackMode = 3;
    }
    sprintf(szIniPar + strlen(szIniPar), ", [LOG_BACK_CFG]->LogBackMode = %d", m_wLogBackMode);

    // 备份完成后是否删除源目录文件, 0:删除, 1不删除, 缺省0
    m_wLogDelSource = (WORD)clReadINI.GetValue("LogDelSource", (DWORD)0);
    if (m_wLogDelSource < 0 || m_wLogDelSource > 1)
    {
        m_wLogDelSource = 0;
    }
    sprintf(szIniPar + strlen(szIniPar), ", [LOG_BACK_CFG]->LogDelSource = %d", m_wLogDelSource);

    // 备份LOG保存天数,从当前日期算起,缺省90
    m_wLogBackSaveDays = (WORD)clReadINI.GetValue("LogBackSaveDays", (DWORD)90);
    if (m_wLogBackSaveDays < 1)
    {
        m_wLogBackSaveDays = 90;
    }
    sprintf(szIniPar + strlen(szIniPar), ", [LOG_BACK_CFG]->LogBackSaveDays = %d", m_wLogBackSaveDays);

    // 其他备份关键字列表,用于非标准Log名备份,只适用于LogRootDir指定目录下,缺省空(不备份)
    memset(szBuffer, 0x00, sizeof(szBuffer));
    strcpy(szBuffer, clReadINI.GetValue("OtherBackList", ""));
    nTmp = (INT)DataConvertor::split_string(szBuffer, ',', nullptr, 0);
    if (nTmp > 0)
    {
        char szList[nTmp][CONST_VALUE_260];
        DataConvertor::split_string(szBuffer, ',', szList, nTmp);
        for (INT i = 0; i < nTmp; i ++)
        {
            m_stOtherBackList.AddList(szList[i]);
        }
    }
    sprintf(szIniPar + strlen(szIniPar), ", [LOG_BACK_CFG]->OtherBackList = %s", szBuffer);

    // 不进行备份的关键字列表,可设置多个,以","分隔,缺省空
    memset(szBuffer, 0x00, sizeof(szBuffer));
    strcpy(szBuffer, clReadINI.GetValue("NotBackList", ""));
    nTmp = (INT)DataConvertor::split_string(szBuffer, ',', nullptr, 0);
    if (nTmp > 0)
    {
        char szList[nTmp][CONST_VALUE_260];
        DataConvertor::split_string(szBuffer, ',', szList, nTmp);
        for (INT i = 0; i < nTmp; i ++)
        {
            m_stNotBackList.AddList(szList[i]);
        }
    }
    sprintf(szIniPar + strlen(szIniPar), ", [LOG_BACK_CFG]->NotBackList = %s", szBuffer);

    Log(ThisModule, __LINE__, "INI加载: %s.", szIniPar);

    return 0;
}

/*****************************************************************
 * 功能: 自动读配置文件,用于随时需要加载的配置项
 * 入参: 无
 * 回参: 无
 * 返回值: 0成功
*****************************************************************/
int CLogBackThread::AutoGetIniConfig()
{
    CHAR szBuffer[MAX_PATH];
    INT  nTmp;

    CINIReader clReadINI = m_clRWIni.GetReaderSection("LOG_BACK_CFG");// 加载备份配置项目

    // Log备份删除功能支持, 配置: 0/1/2, 缺省0
    m_wLogBackDelSup = (WORD)clReadINI.GetValue("LogBackDelSup", (DWORD)1);
    if (m_wLogBackDelSup < 0 || m_wLogBackDelSup > 2)
    {
        m_wLogBackDelSup = 0;
    }

    // 其他备份关键字列表,用于非标准Log名备份,只适用于LogRootDir指定目录下,缺省空(不备份)
    memset(szBuffer, 0x00, sizeof(szBuffer));
    strcpy(szBuffer, clReadINI.GetValue("OtherBackList", ""));
    nTmp = (INT)DataConvertor::split_string(szBuffer, ',', nullptr, 0);
    if (nTmp > 0)
    {
        char szList[nTmp][CONST_VALUE_260];
        DataConvertor::split_string(szBuffer, ',', szList, nTmp);
        for (INT i = 0; i < nTmp; i ++)
        {
            m_stOtherBackList.AddList(szList[i]);
        }
    }

    // 不进行备份的关键字列表,可设置多个,以","分隔,缺省空
    memset(szBuffer, 0x00, sizeof(szBuffer));
    strcpy(szBuffer, clReadINI.GetValue("NotBackList", ""));
    nTmp = (INT)DataConvertor::split_string(szBuffer, ',', nullptr, 0);
    if (nTmp > 0)
    {
        char szList[nTmp][CONST_VALUE_260];
        DataConvertor::split_string(szBuffer, ',', szList, nTmp);
        for (INT i = 0; i < nTmp; i ++)
        {
            m_stNotBackList.AddList(szList[i]);
        }
    }

    return 0;
}

/*****************************************************************
 * 功能: QString转为char*
 * 入参:
 * 回参: 无
 * 返回值: char*
*****************************************************************/
char* CLogBackThread::QString2Char(QString qsStr)
{
    QByteArray qByArr;
    qByArr = qsStr.toLatin1();
    char *chr = qByArr.data();
    return chr;
}

/*****************************************************************
 * 功能: 检查字符串是否为YYYYMMDD格式
 * 入参:
 * 回参: 无
 * 返回值: true(是)/false(否)
*****************************************************************/
bool CLogBackThread::ChkIsDate(LPSTR lpStr)
{
    if (lpStr == nullptr || strlen(lpStr) != 8)
    {
        return false;
    }

    QString qStr = lpStr;
    QDateTime qDTime = QDateTime::fromString(qStr, "yyyyMMdd");
    if (qDTime.isValid() != true)
    {
        return false;
    }

    return true;
}

/*****************************************************************
 * 功能: 获取文件/目录创建时间
 * 入参:
 * 回参: 无
 * 返回值: true(是)/false(否)
*****************************************************************/
bool CLogBackThread::GetFileCreateDate(LPSTR lpName, LPSTR lpDate)
{
    if (lpName == nullptr || strlen(lpName) < 0 ||
        lpDate == nullptr)
    {
        return false;
    }

    QFileInfo qFile(lpName);
    QDateTime qDate = qFile.created();
    if (qDate.isValid() != true)
    {
        return false;
    }

    sprintf(lpDate, "%04d%02d%02d",
            qDate.date().year(), qDate.date().month(), qDate.date().day());

    return true;
}

/*****************************************************************
 * 功能: 获取文件大小
 * 入参:
 * 回参: 无
 * 返回值: -1/>-1
*****************************************************************/
ULONG CLogBackThread::GetFileSize(LPSTR lpName)
{
    if (lpName == nullptr || strlen(lpName) < 0)
    {
        return -1;
    }

    QFileInfo qFile(lpName);
    return (ULONG)qFile.size();
}

