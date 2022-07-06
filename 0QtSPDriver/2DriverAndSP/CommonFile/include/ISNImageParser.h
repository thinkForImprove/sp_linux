#ifndef ISNIMAGEPARSER_H
#define ISNIMAGEPARSER_H

#include "FSNDefine.h"

/*
    功能：根据冠字号图片文件生成人行定义的点阵数据
    参数：[in] lpszImageFile : 冠字号图片文件名，文件格式为"bmp"或"jpg"
          [in] uiValue : 纸币面额
          [out] Data : 人行定义的点阵数据
    返回值：0---成功；其他---失败。
*/
extern "C" long SNIP_Parser(const char *lpszImageFile, unsigned int uiValue, unsigned int uMode, TImageSNo &Data);
extern "C" bool SaveImageFile(const char *lpszSrcFile, const char *lpszDesFile);

#endif // ISNIMAGEPARSER_H
