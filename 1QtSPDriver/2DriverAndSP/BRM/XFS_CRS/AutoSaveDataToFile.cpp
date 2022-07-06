#include "AutoSaveDataToFile.h"
#include "XFS_CRS.h"
#include <assert.h>

//////////////////////////////////////////////////////////////////////
// CAutoSaveDataToFile
//////////////////////////////////////////////////////////////////////
CAutoSaveDataToFile::CAutoSaveDataToFile(XFS_CRSImp *pSP)
{
    assert(pSP != NULL);

    m_pSP = pSP;
    m_PresentInfor = m_pSP->m_PresentInfor;
    m_CashInInfor = m_pSP->m_CashInInfor;
}

CAutoSaveDataToFile::~CAutoSaveDataToFile()
{
    assert(m_pSP->m_pCUManager != NULL);

    //Save Data
    /*if
    (m_PresentInfor != m_pSP->m_PresentInfor)
    {
        m_pSP->DoPersist(m_pSP->m_strPresentFile.c_str(), FALSE);
    }
    if (m_CashInInfor != m_pSP->m_CashInInfor)
    {
        m_pSP->DoPersist(m_pSP->m_strCashInFile.c_str(), FALSE);
    }
    */
    if (m_pSP->m_pCUManager->IsDirty())
    {
        m_pSP->m_pCUManager->SaveData();
    }
}

