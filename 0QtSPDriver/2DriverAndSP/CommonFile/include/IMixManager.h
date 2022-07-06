//IMixManager.h
#ifndef __IMIXMANAGER_H__
#define __IMIXMANAGER_H__

#include <QCoreApplication>
#include <QtCore/qglobal.h>
#include "QtTypeDef.h"
#include "ILogWrite.h"

//错误码定义
#define  ERR_MIX_INPUT_PARM     (-1) //配钞参数传递错误
#define  ERR_MIX_ALGORITHM      (-2) //配钞失败
#define  ERR_MIX_MORE_ITEM      (-3) //太多张数

//函数导出定义
#if defined(MIXMANAGER_LIBRARY)
#  define MIXMANAGERSHARED_EXPORT Q_DECL_EXPORT
#else
#  define MIXMANAGERSHARED_EXPORT Q_DECL_IMPORT
#endif

//MIX钞箱信息
typedef struct _mix_CU_infor
{
    ULONG ulValue;  // 钞箱面额
    ULONG ulCount;  // 钞箱当前剩余钞数
} MIXCUINFOR, *LPMIXCUINFOR;//LPMIXCUINFOR：钞箱信息结构

//MIX算法定义
typedef enum _tag_mix_algorithm
{
    MIX_MINIMUM_NUMBER = 1,     // 最小出钞法
    MIX_EQUAL_EMPTYING,         // 等空出钞法
    MIX_MAXIMUM_NUMBER,         // 最多面额法
    MIX_EQUAL_EMPTYING_EX,       //特殊等空出钞法，尽量从最多剩余钞数的钞箱出钞且钞箱数最少
} MIXALGORITHM; //MIXALGORITHM：配钞方式

//MIX管理器
struct  /*__declspec(novtable)*/  IMixManager
{
    //释放本对象
    virtual  void  Release() = 0;

    //功能：配钞
    // aryulResult :  [out],  配钞结果，分别对应钞箱1 ~ usSize出钞的张数
    // aryCUInfor  :  [in]   ,  钞箱信息结构数组首地址，分别对应1 ~ usSize个钞箱的信息
    // usSize         :  [in]   ,  钞箱信息结构个数（aryCUInfor 、aryulResult数组元素个数）
    // ulAmount    :  [in]   ,  配钞金额
    // alg               :  [in]   ,  配钞方式
    //ulMaxDispenseCount : [in], 最大出钞张数
    //返回值            :  成功 则返回0；其他为失败
    virtual HRESULT  MixByAlgorithm(
    ULONG *aryulResult,
    const LPMIXCUINFOR aryCUInfor,
    USHORT usSize,
    ULONG ulAmount,
    MIXALGORITHM alg,
    ULONG ulMaxDispenseCount) = 0;
};

extern "C" Q_DECL_EXPORT/*MIX_DLL_PORT*/ IMixManager  *CreateMixManager();

#endif //__IMIXMANAGER_H__
