#include "ISNImageParser.h"
//#include "INIReader.h"
#include "opencv/cv.h"
#include "opencv/highgui.h"
#include "opencv2/imgcodecs/imgcodecs.hpp"
#include <string.h>
#include <QFile>
using namespace cv;


static int iOffsetPerChar[3][10] = {{0, 1, 0, 0, 0, -1, -2, 0, -1, 0}, {0, 1, -1, 0, 0, 0, -1, 0, 0, 0}, {0, 1, -1, 0, 0, -1, 0, 0, 0, 0}};

extern "C" bool SaveImageFile(const char *lpszSrcFile, const char *lpszDesFile)
{
    QFile File(lpszSrcFile);
    if (File.exists())
    {
        IplImage *pSrcImage = cvLoadImage(lpszSrcFile, CV_LOAD_IMAGE_UNCHANGED);//默认为将打开的图片转换为3通道
        if (pSrcImage == nullptr)
        {
            return false;
        }
        //Crash,No reason
//        int ret = cvSaveImage(lpszDesFile, pSrcImage);
        bool bRet = cv::imwrite(lpszDesFile, cvarrToMat(pSrcImage));
        cvReleaseImage(&pSrcImage);
//        return ret >= 0;
        return bRet;
    }
    return false;
}

extern "C" long SNIP_Parser(const char *lpszImageFile, unsigned int uiValue, unsigned int uMode, TImageSNo &Data)
{
    //0（默认）表示白底黑子，1表示黑底白字
    if (uMode == 1)
    {
        memset(Data.SNo, 0xFF, sizeof(TImgSNoData) * 12);
    }
    else
    {
        memset(Data.SNo, 0, sizeof(TImgSNoData) * 12);
    }


    if (lpszImageFile == nullptr)
    {
        // 非法参数
        return -1;
    }

    IplImage *pSrcImage = cvLoadImage(lpszImageFile, CV_LOAD_IMAGE_UNCHANGED);

    if (pSrcImage == nullptr)
    {
        return -3;
    }
    IplImage *pGrayImage = nullptr;
    IplImage *pBinaryImage = nullptr;
    IplImage *pBinaryImage1 = nullptr;
    // 转为灰度图
    pGrayImage =  cvCreateImage(cvGetSize(pSrcImage), IPL_DEPTH_8U, 1);
    if (pGrayImage == nullptr)
    {
        return -3;
    }
    cvCvtColor(pSrcImage, pGrayImage, CV_BGR2GRAY);

    // 创建二值图
    pBinaryImage1 = cvCreateImage(cvGetSize(pGrayImage), IPL_DEPTH_8U, 1);
    if (pBinaryImage1 == nullptr)
    {
        return -3;
    }
    pBinaryImage = cvCreateImage(cvGetSize(pGrayImage), IPL_DEPTH_8U, 1);
    if (pBinaryImage == nullptr)
    {
        return -3;
    }

    int blocksize = 4 * 2 + 3;     //计算阈值的像素邻域大小3,5,7...
    int param = 4;                      //常量, 范围取[-50,50]

    cvAdaptiveThreshold(pGrayImage,
                        pBinaryImage1,
                        255,
                        CV_ADAPTIVE_THRESH_MEAN_C,
                        CV_THRESH_BINARY, blocksize, param
                       );

    //邻域平均滤波
    cvSmooth(pBinaryImage1, pBinaryImage, CV_BLUR, 3, 3, 0, 0);

    CvSize cv = cvGetSize(pSrcImage);
    // 数据行数
    unsigned int dwLines = (cv.height > 32) ? 32 : cv.height;
    // 每个字符的像素个数（平均值）
    unsigned int dwPixelsPerChar = (cv.width / 10 > 32) ? 32 : (cv.width / 10);

    unsigned int iMaxValue = 98;
    unsigned int dwOffsetTotal = 0;
    int k;
    switch (uiValue)
    {
    case 10: k = 0; break;
    case 100: k = 1; break;
    case 50: k = 2; break;
    default:  k = 1;  break;
    }

    Data.Num = 10;
    Data.height = (unsigned short)dwLines;
    Data.width = (unsigned short)dwPixelsPerChar;//每个字符的宽度
    unsigned int i, j, m;
    for (i = 0; i < dwLines; i++)
    {
        dwOffsetTotal = 0;
        for (j = 0; j < 10; j++)
        {
            dwOffsetTotal += iOffsetPerChar[k][j];
            for (m = 0; m < dwPixelsPerChar; m++)
            {
                unsigned int iIndex = pBinaryImage->widthStep * (dwLines - (1 + i)) + dwOffsetTotal + m + 2;
                if (iIndex < 0 || iIndex > (pBinaryImage->widthStep * pBinaryImage->height))
                {
                    continue;
                }
                unsigned int uiMask = 0x00000001;
                if (pBinaryImage->imageData[iIndex] < iMaxValue)
                {
                    if (uMode == 1)
                    {
                        Data.SNo[j].Data[m] ^= (uiMask << i);
                    }
                    else
                    {
                        Data.SNo[j].Data[m] |= (uiMask << i);
                    }
                }
            }
            dwOffsetTotal += dwPixelsPerChar;
        }
    }

    cvReleaseImage(&pSrcImage);
    cvReleaseImage(&pGrayImage);
    cvReleaseImage(&pBinaryImage);
    cvReleaseImage(&pBinaryImage1);

    return 0;
}
