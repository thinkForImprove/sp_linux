
#include "XFSErrorConvertor.h"
#include "XFSCIM.H"
#include "XFSCDM.H"

#define THISFILE            "CXFSErrorConvertor"

//单件实例指针
CXFSErrorConvertor *CXFSErrorConvertor::m_pInstance = NULL;

#define ADD_MAP(e1, e2) AddMap(e1, e2)

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CXFSErrorConvertor::CXFSErrorConvertor()
{
    SetLogFile(LOGFILE, "CXFSErrorConvertor", "BRM");
    ADD_MAP(WFS_ERR_CDM_INVALIDCURRENCY, WFS_ERR_CIM_INVALIDCURRENCY);
    ADD_MAP(WFS_ERR_CDM_INVALIDTELLERID, WFS_ERR_CIM_INVALIDTELLERID);
    ADD_MAP(WFS_ERR_CDM_CASHUNITERROR, WFS_ERR_CIM_CASHUNITERROR);

    ADD_MAP(WFS_ERR_CDM_INVALIDDENOMINATION, -1);
    ADD_MAP(WFS_ERR_CDM_INVALIDMIXNUMBER, -1);
    ADD_MAP(WFS_ERR_CDM_NOCURRENCYMIX, -1);
    ADD_MAP(WFS_ERR_CDM_NOTDISPENSABLE, -1);

    ADD_MAP(WFS_ERR_CDM_TOOMANYITEMS, WFS_ERR_CIM_TOOMANYITEMS);
    ADD_MAP(WFS_ERR_CDM_UNSUPPOSITION, WFS_ERR_CIM_UNSUPPOSITION);
    ADD_MAP(WFS_ERR_CDM_SAFEDOOROPEN, WFS_ERR_CIM_SAFEDOOROPEN);
    ADD_MAP(WFS_ERR_CDM_SHUTTERNOTOPEN, WFS_ERR_CIM_SHUTTERNOTOPEN);
    ADD_MAP(WFS_ERR_CDM_SHUTTEROPEN, WFS_ERR_CIM_SHUTTEROPEN);
    ADD_MAP(WFS_ERR_CDM_SHUTTERCLOSED, WFS_ERR_CIM_SHUTTERCLOSED);
    ADD_MAP(WFS_ERR_CDM_INVALIDCASHUNIT, WFS_ERR_CIM_INVALIDCASHUNIT);
    ADD_MAP(WFS_ERR_CDM_NOITEMS, WFS_ERR_CIM_NOITEMS);
    ADD_MAP(WFS_ERR_CDM_EXCHANGEACTIVE, WFS_ERR_CIM_EXCHANGEACTIVE);
    ADD_MAP(WFS_ERR_CDM_NOEXCHANGEACTIVE, WFS_ERR_CIM_NOEXCHANGEACTIVE);
    ADD_MAP(WFS_ERR_CDM_SHUTTERNOTCLOSED, WFS_ERR_CIM_SHUTTERNOTCLOSED);

    ADD_MAP(WFS_ERR_CDM_PRERRORNOITEMS, -1);
    ADD_MAP(WFS_ERR_CDM_PRERRORITEMS, -1);
    ADD_MAP(WFS_ERR_CDM_PRERRORUNKNOWN, -1);

    ADD_MAP(WFS_ERR_CDM_ITEMSTAKEN, WFS_ERR_CIM_ITEMSTAKEN);

    ADD_MAP(WFS_ERR_CDM_INVALIDMIXTABLE, -1);
    ADD_MAP(WFS_ERR_CDM_OUTPUTPOS_NOT_EMPTY, -1);

    ADD_MAP(WFS_ERR_CDM_INVALIDRETRACTPOSITION, WFS_ERR_CIM_INVALIDRETRACTPOSITION);
    ADD_MAP(WFS_ERR_CDM_NOTRETRACTAREA, WFS_ERR_CIM_NOTRETRACTAREA);

    ADD_MAP(WFS_ERR_CDM_NOCASHBOXPRESENT, -1);
    ADD_MAP(WFS_ERR_CDM_AMOUNTNOTINMIXTABLE, -1);
    ADD_MAP(WFS_ERR_CDM_ITEMSNOTTAKEN, -1);
    ADD_MAP(WFS_ERR_CDM_ITEMSLEFT, -1);

    ADD_MAP(WFS_ERR_DEV_NOT_READY, WFS_ERR_CIM_CASHINACTIVE);
    ADD_MAP(-1, WFS_ERR_CIM_NOCASHINACTIVE);
    ADD_MAP(-1, WFS_ERR_CIM_POSITION_NOT_EMPTY);
}

CXFSErrorConvertor::~CXFSErrorConvertor()
{

}

CXFSErrorConvertor *CXFSErrorConvertor::GetInstance()
{
    if (m_pInstance == NULL)
    {
        m_pInstance = new CXFSErrorConvertor();
        atexit(DestoryInstance);
    }
    return m_pInstance;
}

void CXFSErrorConvertor::DestoryInstance()
{
    if (m_pInstance != NULL)
    {
        delete m_pInstance;
        m_pInstance = NULL;
    }
}

//转换错误码
//如错误码大于0或在-1~-99之间，直接返回原错误码；
//  否则查找映射，如查找到，返回对应的值；否则返回WFS_ERR_INTERNAL_ERROR
//iError：原错误码
//返回：转换后的错误码
HRESULT CXFSErrorConvertor::ConvertToXFSErrorCode(HRESULT iError, BOOL bCDM)
{
    const char *ThisModule = "ConvertToXFSErrorCode";

    //XFS通用错误
    if (iError >= 0 ||
        abs(iError) <= 99)
    {
        return iError;
    }

    int nOffset = abs(iError) / 100 * 100;
    if (bCDM)
    {
        if (nOffset == CDM_SERVICE_OFFSET)
            return iError;
        if (nOffset == CIM_SERVICE_OFFSET)
        {
            ERR_MAP_IT it = m_CIMError2CDM.find(iError);
            if (it != m_CIMError2CDM.end())
            {
                return it->second;
            }
        }
    }
    else
    {
        if (nOffset == CIM_SERVICE_OFFSET)
        {
            return iError;
        }
        if (nOffset == CDM_SERVICE_OFFSET)
        {
            ERR_MAP_IT it = m_CDMError2CIM.find(iError);
            if (it != m_CDMError2CIM.end())
            {
                return it->second;
            }
        }
    }

    Log(ThisModule, -1, "转换%s的XFS错误码(%d)失败", (bCDM ? "CDM" : "CIM"), iError);

    return WFS_ERR_INTERNAL_ERROR;
}
