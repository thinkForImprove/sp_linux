// SPConfigFile.h: interface for the CSPConfigFile class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_SPCONFIGFILE_H__00B40E07_D1BE_438F_8107_4DC17B46B4BE__INCLUDED_)
#define AFX_SPCONFIGFILE_H__00B40E07_D1BE_438F_8107_4DC17B46B4BE__INCLUDED_

#include "QtTypeDef.h"
#include "INIFileReader.h"

//SP配置文件读写辅助类
//该类由_tag_brm_config_param调用，主要屏蔽配置文件类型，屏蔽以后配置文件格式修改的影响
class CSPConfigFile
{
public:
    //得到整数值
    //lpszKeyName：键名，INI文件中［］中的内容
    //lpszValueName：值名，INI文件中等号前的内容
    //nDefault：缺省值，如读配置失败或转换失败，返回该值
    int GetInt(LPCSTR lpszKeyName, LPCSTR lpszValueName, int nDefault);

    //得到字串值，返回字串应在下一次调用GetString前拷贝出来
    //lpszKeyName：键名，INI文件中［］中的内容
    //lpszValueName：值名，INI文件中等号前的内容
    //lpszDefault：缺省值，如读配置失败，返回该值
    LPCSTR GetString(LPCSTR lpszKeyName, LPCSTR lpszValueName, LPCSTR lpszDefault);

    //设置字串值
    //lpszKeyName：键名，INI文件中［］中的内容
    //lpszValueName：值名，INI文件中等号前的内容
    //lpszValue：要写入的值
    BOOL SetString(LPCSTR lpszKeyName, LPCSTR lpszValueName, LPCSTR lpszValue);

    //从配置文件装载配置项
    //先从ETCDIR下装载，失败时从当前目录装载
    //lpszFileName：不带路径的文件名
    //失败返回WFS_ERR_INTERNAL_ERROR
    int Load(LPCSTR lpszFileName);

    CSPConfigFile();
    virtual ~CSPConfigFile();

private:
    CINIFileReader m_configfile;
    bool           m_bLoad;
};

#endif // !defined(AFX_SPCONFIGFILE_H__00B40E07_D1BE_438F_8107_4DC17B46B4BE__INCLUDED_)
