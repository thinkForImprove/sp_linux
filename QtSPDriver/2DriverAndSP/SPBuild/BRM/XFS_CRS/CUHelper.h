//CUHelper.h
#ifndef CU_HELPER_H
#define CU_HELPER_H

//钞箱操作辅助类
//该类为SP主类服务，主要以LPVOID类型屏蔽CDM和CIM接口差异
class CCUHelper
{
public:
    CCUHelper(BOOL bCDM, LPVOID pCUInterface)
    {
        m_bCDM = bCDM;
        m_pCUInterface = pCUInterface;
    }

    ~CCUHelper() {}

    //得到是否为CDM类型
    inline BOOL IsCDM() const
    {
        return m_bCDM;
    }

    //得到LCU
    inline LPVOID GetLCUByNumber(USHORT usNumber) const
    {
        return m_bCDM ? (LPVOID)((ICDMCUInterface *)m_pCUInterface)->GetCDMLCUByNumber(usNumber) :
               (LPVOID)((ICIMCUInterface *)m_pCUInterface)->GetCIMLCUByNumber(usNumber);
    }

    //以XFS格式设置钞箱
    inline long SetByXFSFormat(LPVOID pCUInfo) const
    {
        return m_bCDM ? ((ICDMCUInterface *)m_pCUInterface)->SetByXFSCDMFormat((LPWFSCDMCUINFO)pCUInfo) :
               ((ICIMCUInterface *)m_pCUInterface)->SetByXFSCIMFormat((LPWFSCIMCASHINFO)pCUInfo);
    }

    //得到LCU的个数
    inline USHORT GetCountOfLCU() const
    {
        return m_bCDM ? ((ICDMCUInterface *)m_pCUInterface)->GetCountOfLCU() :
               ((ICIMCUInterface *)m_pCUInterface)->GetCountOfLCU();
    }

    //得到PU接口
    inline LPVOID GetPCUInterface(LPVOID pLCU) const
    {
        return m_bCDM ? (LPVOID)((ICDMLCUInfor *)pLCU)->GetCDMPCUInterface() :
               (LPVOID)((ICIMLCUInfor *)pLCU)->GetCIMPCUInterface();
    }

    //设置交换状态
    inline long SetCUExchangeState(LPVOID pLCU, BOOL bEx) const
    {
        return m_bCDM ? ((ICDMLCUInfor *)pLCU)->SetExchangeState(bEx) :
               ((ICIMLCUInfor *)pLCU)->SetExchangeState(bEx);
    }

    //得到物理钞箱的索引
    inline USHORT GetPCUIndex(LPVOID pPCU) const
    {
        return m_bCDM ? ((ICDMPhysicalCU *)pPCU)->GetIndex() :
               ((ICIMPhysicalCU *)pPCU)->GetIndex();
    }

    //得到钞箱的类型
    inline DWORD GetLCUType(LPVOID pLCU) const
    {
        return m_bCDM ? ((ICDMLCUInfor *)pLCU)->GetType() :
               ((ICIMLCUInfor *)pLCU)->GetType();
    }

private:
    BOOL m_bCDM;            //是否为CDM
    LPVOID m_pCUInterface;  //保存钞箱接口
};

#endif //CU_HELPER_H
