#include "IDCForm.h"
#include <assert.h>
#include <QFileInfo>

static const char *ThisFile = "IDCForm";
static const char *g_error_log[] =
{
    "GetFormFileName(SP=%s)失败",
    "fullname(%s)失败",
    "_stat(%s)失败，文件可能不存在",
    "读取文件(%s)节名失败",
    "读取文件%s节%s失败",
    "读字段""%s""失败",
    "字段""%s""没有定义",
    "字段数据非法(POS%d%+d,POS%d%+d), 总的分隔数(%d)",
    "字段数据(%d,%d)超出范围(Len = %d)",
    "字段数据(%d,%d)-(%d,%d)超出范围(Len = %d)",
    "FORM ""%s""需要的TRACK%d数据没有提供",
    "第%d磁道数据状态不正常(%d)",
    "ComputeFieldData(%s)失败",
    "字段""%s""数据没有提供",
    "FORM ""%s""算法""%s""需要的磁道数据不齐全(TRACK1=%d,TRACK2=%d,TRACK3=%d)",
    "执行%s参数0x%08X失败(%s)",
    "得到信息%s参数0x%08X失败(%s)",
    "GetIDCForm(%s)失败(%d)",
    "AcceptAndReadTrack(%d)失败(%s)",
    "SPParseData(%s)失败(%s)",
    "调用WriteTrack(%d)失败(%s)",
    "调用ChipIO(%d,%d,0x%08X)失败(%s)",
    "无效的FORM名(%s)",
    "FORM(%s)没有定义",
    "FORM(%s)没有正确定义",
    "FORM (%s)动作为%d而要求的动作是%d",
    "调用LoadField(%s)失败",
    "LoadForm(%s)失败",
    "LoadTracksFields(%s)失败",
    "WriteFieldData(%s, TRACK%d, %s)失败"
};

static CLogManage gclog;
void LogWrite(const char *ThisModule, int Err, int fmt, ...)
{
    if (!(fmt >= 0 && fmt <= 30))
        return;

    gclog.SetLogFile(LOGFILE, ThisFile, "IDC");
    char buf[4096] = {0};
    const char *fmtbuf = g_error_log[fmt];;

    va_list vl;
    va_start(vl, fmt);
    vsprintf(buf, fmtbuf, vl);
    va_end(vl);
    gclog.Log(ThisModule, Err, buf);
}


CIDCForm::CIDCForm()
{
    m_FormFileLastChangeTime = 0;
    ZeroMemory(&m_LastForm, sizeof(m_LastForm));
}

CIDCForm::~CIDCForm()
{

}

HRESULT CIDCForm::CheackFormInvalid(FL &FormList, const char *pFormName, DWORD dwAction)
{
    const char *const ThisModule = "CheackFormInvalid";
    HRESULT hRes = WFS_SUCCESS;

    SP_IDC_FORM *pForm = NULL;
    hRes = SPGetIDCForm(FormList, pFormName, pForm, dwAction);
    if (hRes != WFS_SUCCESS)
    {
        LogWrite(ThisModule, -1, IDS_GET_FORM_FAILED, pFormName, hRes);
        return hRes;
    }

    string strFormName = pForm->FormName; // just test

    for (int iTrack = 0; iTrack < 3 && hRes == WFS_SUCCESS; iTrack++)
    {
        FFLIT itField;
        for (itField = pForm->TracksFields[iTrack].begin(); itField != pForm->TracksFields[iTrack].end() && hRes == WFS_SUCCESS; itField++)
        {
            int iIndices0, iIndices1, iOffsets0, iOffsets1;
            iIndices0 = (*itField)->FieldSepIndices[0];
            iIndices1 = (*itField)->FieldSepIndices[1];
            if (iIndices0 > iIndices1)
            {
                hRes = WFS_ERR_IDC_FORMINVALID;
                LogWrite(ThisModule, -1, IDS_GET_FORM_FAILED, pFormName, hRes);
            }
            iOffsets0 = (*itField)->nOffsets[0];
            if (iOffsets0 == 0)
            {
                iOffsets0++;
            }
            iOffsets1 = (*itField)->nOffsets[1];
            if (iIndices0 == iIndices1)
            {
                if (((iOffsets0 > 0 && iOffsets1 > 0) || (iOffsets0 < 0 && iOffsets1 < 0)) &&
                    (iOffsets0 > iOffsets1))
                {
                    hRes = WFS_ERR_IDC_FORMINVALID;
                    LogWrite(ThisModule, -1, IDS_GET_FORM_FAILED, pFormName, hRes);
                }
            }
        }
    }

    return hRes;
}


//功能：在FORM列表查找指字名字的FORM
//输入：pData       : 读卡器数据
//      szFormName  : 要查找的FORM名
//返回：成功，FORM地址；否则NULL。
SP_IDC_FORM *CIDCForm::FLFind(FL &FormList, LPCSTR szFormName)
{
    FLIT itForm;
    for (itForm = FormList.begin(); itForm != FormList.end(); itForm++)
    {
        if ((*itForm)->FormName == szFormName)
            return (*itForm);
    }
    return nullptr;
}

HRESULT CIDCForm::SPGetIDCForm(FL &FormList, const char *pFormName, SP_IDC_FORM *&pForm, DWORD dwAction)
{
    const char *const ThisModule = "SPGetIDCForm";
    HRESULT hRes = WFS_SUCCESS;
    pForm = NULL;
    if (!pFormName || !pFormName[0])
    {
        LogWrite(ThisModule, -1, IDS_FORM_NAME_INVALID, pFormName);
        hRes = WFS_ERR_IDC_FORMNOTFOUND;
    }

    if (hRes == WFS_SUCCESS)
    {
        pForm = FLFind(FormList, pFormName);
        if (!pForm)
        {
            LogWrite(ThisModule, -1, IDS_FORM_NOT_FOUND, pFormName);
            hRes = WFS_ERR_IDC_FORMNOTFOUND;
        }
        else if (!pForm->bLoadSucc)
        {
            LogWrite(ThisModule, -1, IDS_FORM_INVALID, pFormName);
            hRes = WFS_ERR_IDC_FORMINVALID;
        }
        else if (pForm->fwAction != dwAction)
        {
            LogWrite(ThisModule, -1, IDS_FORM_RW_ACTION_ERROR, pFormName, pForm->fwAction, dwAction);
            hRes = WFS_ERR_IDC_FORMINVALID;
        }
    }

    return hRes;
}

//功能：转换磁道索引为数据源选项
//输入：iTrack, 磁道索引
//返回：选项。
DWORD CIDCForm::TrackIndexToDataSourceOption(int iTrack)
{
    switch (iTrack)
    {
    case 0:  return WFS_IDC_TRACK1;
    case 1:  return WFS_IDC_TRACK2;
    default: return WFS_IDC_TRACK3;
    }
}


//功能：根据pValue提供的三个磁道成功与否值，和*pp指向的磁道算法
//      计算整个算法的值
//输入：pp     : 指向字串的指针
//      pValue : 数组
//      bValue : 计算结果
//返回：true，成功，否则，*pp指向的不是一个合法表达式。
BOOL CIDCForm::ComputeTracks(const char **pp, int *pValue, BOOL &bValue)
{
    bValue = true;
    if (! **pp)
        return false;
    BOOL bValue1 = false, bValue2 = false, bOpAnd = false;
    //读第一个值
    if (**pp >= '1' && **pp <= '3')
    {
        bValue1 = pValue[ **pp - '1'];
        (*pp)++;
    }
    else if (**pp == '(')
    {
        (*pp)++;
        if (!ComputeTracks(pp, pValue, bValue1))
            return false;
        if (**pp != ')')
            return false;
        (*pp)++;
    }
    else
        return false;

    while (1)
    {
        //读操作符
        if (**pp == 0 || **pp == ')')
        {
            bValue = bValue1;
            return true;
        }
        if (**pp != '&' && **pp != '|')
            return false;
        if (**pp == '&')
            bOpAnd = true;
        else
            bOpAnd = false;
        (*pp)++;
        //读第二个值
        if (**pp >= '1' && **pp <= '3')
        {
            bValue2 = pValue[ **pp - '1'];
            (*pp)++;
        }
        else if (**pp == '(')
        {
            (*pp)++;
            if (!ComputeTracks(pp, pValue, bValue2))
                return false;
            if (**pp != ')')
                return false;
            (*pp)++;
        }
        else
            return false;
        //计算值
        if (bOpAnd)
            bValue = bValue1 & bValue2;
        else
            bValue = bValue1 | bValue2;
        //查看是否结束
        if (**pp == 0)
            return true;
        if (**pp == ')')
            return true;
        bValue1 = bValue;
    }
}

//功能：根据szTracks的磁道算法计算第iTrack是否需要
//输入：szTrack: 磁道算法字串
//      iTrack : 磁道索引
//返回：true，需要，否则，不一定需要。
bool CIDCForm::NeedTrack(const char *szTracks, int iTrack)
{
    //assert(iTrack >= 0 && iTrack < 3);
    int nValues[3] = {1, 1, 1};
    nValues[iTrack] = 0;
    BOOL bSucc;
    if (!ComputeTracks(&szTracks, nValues, bSucc) || !bSucc)
        return true;
    return false;
}


//功能：是否对FORM进行安全检测
//输入：pFomr     : FORM
//      dwOption  : 选项
//返回：true, FALSE。
BOOL CIDCForm::IsDoSecureCheck(SP_IDC_FORM *pForm, DWORD dwOption)
{
    if (dwOption & WFS_IDC_TRACK1 && pForm->bSecures[0])
        return true;
    if (dwOption & WFS_IDC_TRACK2 && pForm->bSecures[1])
        return true;
    if (dwOption & WFS_IDC_TRACK3 && pForm->bSecures[2])
        return true;
    return false;
}

//功能：转换磁道算法为选项
//输入：szTracks : 磁道算法字串
//返回：选项。
DWORD CIDCForm::TracksToDataSourceOption(const char *szTracks)
{
    DWORD dwOption = 0;
    if (strchr(szTracks, '1'))
        dwOption |= WFS_IDC_TRACK1;
    if (strchr(szTracks, '2'))
        dwOption |= WFS_IDC_TRACK2;
    if (strchr(szTracks, '3'))
        dwOption |= WFS_IDC_TRACK3;
    return dwOption;
}

//功能：生成分隔列表
//输入：pTrack : 磁道数据
//      nLen  : 磁道数据的长度
//      cSep  : 分隔字符
//      SepList : 返回分隔列表
//返回：无。
void CIDCForm::GenSepList(const char *pTrack, int nLen, char cSep, vector<int> &SepList)
{
    const char *pStart = pTrack;
    SepList.push_back(-1);
    while (1)
    {
        const char *pTemp = strchr(pStart, cSep);
        if (!pTemp)
            break;
        SepList.push_back(pTemp - pTrack);
        pStart = pTemp + 1;
    }
    SepList.push_back(nLen);
}

//功能：计算字段信息
//输入：pByte : 磁道数据
//      nLen  : 磁道数据的长度
//      Indicex : 索引数组
//      nOffsets: 偏移数组
//      SepList : 分隔列表
//      nFieldOffset : 返回字段偏移
//      nFieldLen : 返回字段长度
//返回：成功与否。
bool CIDCForm::ComputeFieldInfo(LPBYTE lpByte, int nLen, long *Indices, long *nOffsets,
                                vector<int> &SepList, long &nFieldOffset, long &nFieldLen)
{
    const char *const ThisModule = "ComputeFieldInfo";
    //assert(Indices[0] >= 0);
    int nStart0 = 0;
    if (Indices[0] == END_SEP_INDEX)
    {
        nStart0 = SepList.back();
    }
    else if (Indices[0] < SepList.size())
    {
        nStart0 = SepList[Indices[0]];
    }
    else
    {
        LogWrite(ThisModule, -1, IDS_FIELD_DATA_INVALID, Indices[0], nOffsets[0], Indices[1], nOffsets[1], SepList.size());
        return false;
    }
    int nOffset0 = nOffsets[0];
    //////////////////////////////////////////////////////////////////////////
    //  if(nOffset0 >= 0)
    //      nStart0++;
    if (nOffset0 == 0)
    {
        nOffset0++;
    }
    //////////////////////////////////////////////////////////////////////////
    if (nStart0 + nOffset0 < 0 || nStart0 + nOffset0 >= nLen)
    {
        LogWrite(ThisModule, -1, IDS_FIELD_DATA_OUT_OF_BOUND, nStart0, nOffset0, nLen);
        return false;
    }

    //assert(Indices[1] >= 0);
    int nStart1 = 0;
    if (Indices[1] == END_SEP_INDEX)
    {
        nStart1 = SepList.back();
    }
    else if (Indices[1] < SepList.size())
    {
        nStart1 = SepList[Indices[1]];
    }
    else
    {
        LogWrite(ThisModule, -1, IDS_FIELD_DATA_INVALID, Indices[0], nOffsets[0], Indices[1], nOffsets[1], SepList.size());
        return false;
    }
    int nOffset1 = nOffsets[1];
    //////////////////////////////////////////////////////////////////////////
    //  if(nOffset1 >= 0)
    //      nStart1++;
    //////////////////////////////////////////////////////////////////////////
    if (nStart1 + nOffset1 < 0 || nStart1 + nOffset1 >= nLen || nStart1 + nOffset1 < nStart0 + nOffset0)
    {
        LogWrite(ThisModule, -1, IDS_FIELD_DATA_OUT_OF_BOUND1, nStart0, nOffset0, nStart1, nOffset1, nLen);
        return false;
    }
    nFieldLen = nStart1 + nOffset1 - nStart0 - nOffset0 + 1;
    nFieldOffset = nStart0 + nOffset0;
    return true;
}


//功能：从文件中装入FORM定义数据
//输入：SPName : SP名
//      pData  : 读卡器数据
//返回：成功与否。
bool CIDCForm::LoadFormFile(LPCSTR SPName, FL &FormList)
{
    const char *const ThisModule = "LoadFormFile";
    //从注册表中读取FORM文件名
    string FormFileName = GetFormFileName(SPName);
    gclog.Log(ThisModule, 1, "LoadFormFullName :%s", FormFileName.c_str());
    if (FormFileName.empty())
    {
        LogWrite(ThisModule, -1, IDS_GET_FORM_FILE_NAME_FAILED, SPName);
        return false;
    }

    //仅当文件修改后再次装载
    /* struct _stat st;
     if(_stat(FullName, &st) < 0)
     {
         LogWrite(ThisModule, -1, IDS_STAT_FAILED, FullName);
         return false;
     }
     */
    QFileInfo info(FormFileName.c_str());
    if (!info.exists())
    {
        LogWrite(ThisModule, -1, IDS_STAT_FAILED, FormFileName.c_str());
        return false;
    }
    QDateTime qTime = info.created();
    if (m_FormFileLastChangeTime >= qTime.toTime_t())
        return true;

    //读取所有节名
    char totalsection[100][100];
    int count;
    if (!read_all_section(totalsection, count, FormFileName.c_str()))
    {
        ClearFormList(FormList);
        LogWrite(ThisModule, -1, IDS_READ_SECTIONS_FAILED, FormFileName.c_str());
        return false;
    }

    ClearFormList(FormList);

    CMultiString SectionNames;
    for (int i = 0; i < count; i++)
        SectionNames.Add(totalsection[i]);

    char buf[4096] = {0};
    for (int i = 0; i < count; i++)
    {
        if (!read_every_section_KeyValue_data(SectionNames.GetAt(i), buf, sizeof(buf), FormFileName.c_str()))
            continue;

        SP_IDC_FORM *pForm = new SP_IDC_FORM;
        FormList.push_back(pForm);
        const char *pFormName = SectionNames.GetAt(i);
        if (!pForm->Load(pFormName, buf))
        {
            LogWrite(ThisModule, -1, IDS_LOAD_FORM_FAILED, pFormName);
            continue;
        }

        if (!pForm->bLoadSucc)
            continue;

        //加入ALL字段
        AddFieldALL(pForm);
        //查找每个字段
        if (!FillTracksFields(pForm))
        {
            LogWrite(ThisModule, -1, IDS_FILL_TRACKS_FIELDS_FAILED, pForm->FormName.c_str());
        }
    }

    m_FormNames = SectionNames;
    m_FormFileLastChangeTime = qTime.toTime_t();

    return true;
}


//功能：把pForm的每个磁道的字段的地址加入到TracksFields中
//输入：pForm : FORM
//返回：成功与否。
BOOL CIDCForm::FillTracksFields(SP_IDC_FORM *pForm)
{
    const char *const ThisModule = "FillTracksFields";
    //查找每个字段
    for (int iTrack = 0; iTrack < 3; iTrack++)
    {
        for (int iField = 0; iField < pForm->szTrackFields[iTrack].GetCount(); iField++)
        {
            if (qstricmp(pForm->szTrackFields[iTrack].GetAt(iField), "SECURE") == 0)
            {
                pForm->bSecures[iTrack] = true;
                continue;
            }
            FFLIT itField;
            const char *pFieldName = pForm->szTrackFields[iTrack].GetAt(iField);
            for (itField = pForm->FieldList.begin(); itField != pForm->FieldList.end(); itField++)
            {
                if ((*itField)->FieldName == pFieldName)
                {
                    break;
                }
            }
            if (itField == pForm->FieldList.end())
            {
                LogWrite(ThisModule, -1, IDS_FIELD_NOT_DEFINED, pForm->FormName.c_str(), pForm->szTrackFields[iTrack].GetAt(iField));
                pForm->bLoadSucc = false;
                return false;
            }
            pForm->TracksFields[iTrack].push_back(*itField);
        }

        if (!pForm->bLoadSucc)
            break;
    }

    return true;
}

//功能：加入一个字段名为ALL的字段
//输入：pForm : FORM
//返回：成功与否。
inline void CIDCForm::AddFieldALL(SP_IDC_FORM *pForm)
{
    SP_IDC_FORM_FIELD *pField = new SP_IDC_FORM_FIELD;
    pField->FieldName           = "ALL";
    pField->FieldSepIndices[0]  = 0;
    pField->FieldSepIndices[1]  = END_SEP_INDEX;
    pField->nOffsets[0]         = 0;
    pField->nOffsets[1]         = -1;
    pForm->FieldList.push_back(pField);
}


//功能：从文件中装入FORM定义数据
//输入：pData : 数据
//返回：无。
void CIDCForm::ClearFormList(FL &FormList)
{
    FLIT itForm;
    for (itForm = FormList.begin(); itForm != FormList.end(); itForm++)
    {
        delete (*itForm);
    }
    FormList.clear();
}


//功能：从注册表中读出FORM文件名
//输入：SPName : SP名
//返回：FORM文件名。
string CIDCForm::GetFormFileName(LPCSTR SPName)
{
    QString strFullName;
    LPCSTR szFomName = m_cXfsReg.GetValue(m_strSPName.c_str(), FORMFILEVALUENAME, FORMFILEDEFAULT);
    strFullName.sprintf("%s%s", FORMPATH, szFomName);
    return strFullName.toStdString();
}


//功能：查找字段数据
//输入：FieldData : 字段数据数组
//      FieldName : 字段名
//返回：成功，字段值；否则，NULL。
LPCSTR CIDCForm::SPFindFieldName(const CMultiString &FieldData, LPCSTR FieldName)
{
    int i;
    for (i = 0; i < FieldData.GetCount(); i++)
    {
        LPCSTR ps = FieldData.GetAt(i);
        LPCSTR pd = FieldName;
        while (*pd && *ps && *ps++ == *pd++);
        if (!*pd && *ps == '=')
        {
            return ps + 1;
        }
    }
    return NULL;
}

//功能：形成磁道数据并写磁道
//输入：pData : 数据
//      pWrite : 参数
//返回：LFS_SUCCESS, 成功，否则，失败。
bool CIDCForm::WriteFieldData(LPWFSIDCCARDDATA pCardData, SP_IDC_FORM_FIELD *pField, vector<int> &SepList,
                              BOOL bDefault, char cFieldSeparator, CMultiString &InputData)
{
    const char *const ThisModule = "WriteFieldData";

    LPBYTE lpByte = pCardData->lpbData;
    int nLen = pCardData->ulDataLength;
    long nFieldLen = 0;
    long nFieldOffset = 0;
    if (!ComputeFieldInfo(lpByte, nLen, pField->FieldSepIndices, pField->nOffsets, SepList, nFieldOffset, nFieldLen))
    {
        LogWrite(ThisModule, -1, IDS_COMPUTE_FIELD_FAILED, pField->FieldName.c_str());
        return false;
    }

    LPCSTR pFieldData = SPFindFieldName(InputData, pField->FieldName.c_str());
    if (!pFieldData)
    {
        if (bDefault)
            return true;
        LogWrite(ThisModule, -1, IDS_NO_FIELD_DATA, pField->FieldName.c_str());
        return false;
    }
    int nDataLen = strlen(pFieldData);
    if (nDataLen > 0)
    {
        if (pField->FieldName == "ALL")
        {
            if (pCardData->ulDataLength < nDataLen)
            {
                delete [] pCardData->lpbData;
                pCardData->lpbData = new BYTE[nDataLen + 1];
            }
            pCardData->ulDataLength = nDataLen;
            memcpy(lpByte, pFieldData, nDataLen);
            SepList.clear();
            GenSepList((char *)pCardData->lpbData, pCardData->ulDataLength, cFieldSeparator, SepList);
        }
        else
            memcpy(lpByte + nFieldOffset, pFieldData, nDataLen <= nFieldLen ? nDataLen : nFieldLen);
    }

    return true;
}

//功能：压缩磁道算法字段，去掉TRACK
//输入：szOld  : 原字串
//      szNew  : 转换后的字串
//返回：true, 成功，否则，原字串有非法字符。
BOOL CompressTracks(const char *szOld, char *szNew)
{
    const char *ps = szOld;
    char *pd = szNew;
    while (*ps)
    {
        if (*ps == '|' || *ps == '&' || *ps == '(' || *ps == ')')
            *pd++ = *ps++;
        else if (*ps == ' ')
            ps++;
        else if (*ps == 'T' && ps[1] == 'R' && ps[2] == 'A' && ps[3] == 'C' && ps[4] == 'K' && ps[5] <= '3' && ps[5] >= '1')
        {
            *pd = ps[5];
            pd++;
            ps += 6;
        }
        else
            return false;
    }
    *pd = 0;

    return true;
}

//功能：从pFormName和pBuf中装入Form数据
//输入：pFormName: FORM名
//      pBuf : FORM数据字串
//返回：成功与否。
bool SP_IDC_FORM::Load(const char *pFormName, const char *pBuf)
{
    const char *const ThisModule = "FORM.Load";

    this->FormName  = pFormName;
    this->bLoadSucc = true;
    CMultiString Section(pBuf);
    BOOL bTrack1 = false, bTrack2 = false;
    BOOL bRead = false;
    for (int j = 0; j < Section.GetCount(); j++)
    {
        LPCSTR p = Section.GetAt(j);
        int len = strlen(p);
        if (len < 2)
            continue;
        if (strncmp(p, "//", 2) == 0)
            continue;
        CMultiString Pair;
        if (!GetNameAndValue((char *)p, Pair))
            continue;
        assert(Pair.GetCount() == 2);
        LPCSTR pName = Pair.GetAt(0);
        LPCSTR pValue = Pair.GetAt(1);
        if ((bTrack1 = qstricmp(pName, "FIELDSEPT1") == 0) ||
            (bTrack2 = qstricmp(pName, "FIELDSEPT2") == 0) ||
            (qstricmp(pName, "FIELDSEPT3") == 0))
        {
            if (bTrack1)
                this->cFieldSeparatorTracks[0] = pValue[0];
            else if (bTrack2)
                this->cFieldSeparatorTracks[1] = pValue[0];
            else
                this->cFieldSeparatorTracks[2] = pValue[0];
        }
        else if ((bRead = qstricmp(pName, "READ") == 0) ||
                 (qstricmp(pName, "WRITE") == 0))
        {
            this->sTracks   =  pValue;
            this->fwAction      = WFS_IDC_ACTIONREAD;
            if (!bRead)
                this->fwAction = WFS_IDC_ACTIONWRITE;
            CompressTracks(pValue, this->szTracks);
        }
        else if ((bTrack1 = (qstricmp(pName, "TRACK1") == 0)) ||
                 (bTrack2 = (qstricmp(pName, "TRACK2") == 0)) ||
                 ((qstricmp(pName, "TRACK3") == 0)))
        {
            CMultiString *pms = NULL;
            if (bTrack1)
                pms = &this->szTrackFields[0];
            else if (bTrack2)
                pms = &this->szTrackFields[1];
            else
                pms = &this->szTrackFields[2];
            *pms = Split((char *)pValue, " \t\r\n,");
        }
        else if (qstricmp(pName, "DEFAULT") == 0)
        {
            this->sDefault = pValue;
        }
        else
        {
            SP_IDC_FORM_FIELD *pField = new SP_IDC_FORM_FIELD;
            FieldList.push_back(pField);
            if (!pField->Load(pName, pValue))
            {
                LogWrite(ThisModule, -1, IDS_LOAD_FIELD_FAILED, pName, pValue);
                this->bLoadSucc = false;
                return false;
            }
        }
    }

    return true;
}

//------------------ SP_IDC_FORM_FIELD实现 ----------------------
//功能：从pName和pValue中装入字段数据
//输入：pName  : 字段名
//      pValue : 字段的值字串
//返回：成功与否。
bool SP_IDC_FORM_FIELD::Load(const char *pName, const char *pValue)
{
    const char *const ThisModule = "FIELD.Load";
    this->FieldName = pName;
    CMultiString ms = Split((char *)pValue, ",");
    if (ms.GetCount() < 2)
    {
        LogWrite(ThisModule, -1, IDS_READ_FIELD_FAILED, this->FieldName.c_str());
        return false;
    }
    for (int i = 0; i < 2; i++)
    {
        char *pTemp = (char *)ms.GetAt(i);
        char cPlus = '+';
        if (!strchr(pTemp, cPlus))
            cPlus = '-';
        SkipSpace(pTemp);
        char *pStart = pTemp;
        BOOL bEndTrack = false;
        if (qstrnicmp(pStart, "FIELDSEPPOS", 11) == 0)
        {
            if (!isdigit(pStart[11]))
            {
                LogWrite(ThisModule, -1, IDS_READ_FIELD_FAILED, this->FieldName.c_str());
                return false;
            }
        }
        else if (qstrnicmp(pStart, "ENDTRACK", 8) == 0)
        {
            bEndTrack = true;
        }
        else
        {
            LogWrite(ThisModule, -1, IDS_READ_FIELD_FAILED, this->FieldName.c_str());
            return false;
        }
        char *pFirstBlank = nullptr;
        if (!ReadToChar(pTemp, cPlus, pFirstBlank))
        {
            LogWrite(ThisModule, -1, IDS_READ_FIELD_FAILED, this->FieldName.c_str());
            return false;
        }
        if (pFirstBlank)
            *pFirstBlank = 0;
        *pTemp = 0;
        pTemp++;
        SkipSpace(pTemp);
        if (bEndTrack)
            this->FieldSepIndices[i] = END_SEP_INDEX;
        else
            this->FieldSepIndices[i] = atol(pStart + 11);
        this->nOffsets[i] = (cPlus == '+' ? 1 : -1) * atol(pTemp);
    }

    return true;
}

