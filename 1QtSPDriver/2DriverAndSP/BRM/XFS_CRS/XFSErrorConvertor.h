#pragma once
#include "QtTypeDef.h"
#include "ILogWrite.h"
#include <map>
using namespace std;

//XFS错误转换辅助类
class CXFSErrorConvertor  : public CLogManage
{
    typedef map<HRESULT, HRESULT> ERR_MAP;
    typedef ERR_MAP::iterator ERR_MAP_IT;
public:
    //得到单件实例
    static CXFSErrorConvertor *GetInstance();

    //销毁单件实例
    static void DestoryInstance();

    //转换错误码
    //如错误码大于0或在-1~-99之间，直接返回原错误码；
    //  否则查找映射，如查找到，返回对应的值；否则返回WFS_ERR_INTERNAL_ERROR
    //iError：原错误码
    //返回：转换后的错误码
    HRESULT ConvertToXFSErrorCode(HRESULT iError, BOOL bCDM);

private: //私有化构造函数与析构函数，防止外部直接分配与释放
    CXFSErrorConvertor();
    virtual ~CXFSErrorConvertor();

    //增加一个映射对
    void AddMap(HRESULT e1, HRESULT e2)
    {
        m_CDMError2CIM[e1] = e2;
        m_CIMError2CDM[e2] = e1;
    }
private:
    static CXFSErrorConvertor *m_pInstance; //单件实例
    ERR_MAP m_CDMError2CIM;                 //从CDM错误到CIM错误的映射
    ERR_MAP m_CIMError2CDM;                 //从CIM错误到CDM错误的映射
};


