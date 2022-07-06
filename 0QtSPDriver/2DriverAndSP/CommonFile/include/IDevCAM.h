#pragma once

//////////////////////////////////////////////////////////////////////////
// ������
#define CAM_SUCCESS          (0)
#define ERR_CONFIG           (-1)
#define ERR_OPEN_CAMER       (-2)
#define ERR_INVALID_PARAM    (-3)
#define ERR_LOAD_IMAGE       (-4)
#define ERR_FRAME            (-5)
#define ERR_IMWRITE          (-6)
#define ERR_SAVE_IMAGE       (-7)
#define ERR_OTHER            (-8)
#define ERR_TIMEOUT          (-9)

//////////////////////////////////////////////////////////////////////////
#define CAM_CAMERAS_SIZE     (8)
#define MAX_SIZE             (400)
#define MAX_CAMDATA          (4096)
//////////////////////////////////////////////////////////////////////////
enum DEVICE_STATUS
{
    DEVICE_OFFLINE                  = 0,
    DEVICE_ONLINE                   = 1,
    DEVICE_HWERROR                  = 2
};

enum MEDIA_STATUS
{
    MEDIA_OK                        = 0,
    MEDIA_HIGH                      = 1,
    MEDIA_FULL                      = 2,
    MEDIA_UNKNOWN                   = 3,
    MEDIA_NOTSUPP                   = 4
};

enum CAM_POS
{
    CAM_ROOM                        = 0,
    CAM_PERSON                      = 1,
    CAM_EXITSLOT                    = 2
};

enum CAM_STATUS
{
    STATUS_NOTSUPP                  = 0,
    STATUS_OK                       = 1,
    STATUS_INOP                     = 2,
    STATUS_UNKNOWN                  = 3
};

typedef struct tag_dev_cam_status
{
    WORD   fwDevice;
    WORD   fwMedia[CAM_CAMERAS_SIZE];
    WORD   fwCameras[CAM_CAMERAS_SIZE];
    USHORT usPictures[CAM_CAMERAS_SIZE];
    char   szErrCode[8];    // ��λ�Ĵ�����

    tag_dev_cam_status() { Clear(); }
    void Clear()
    {
        memset(this, 0x00, sizeof(tag_dev_cam_status));
        for (auto &it : fwMedia)
        {
            it = MEDIA_NOTSUPP;// ��ʼ��֧��
        }
    }
} DEVCAMSTATUS, *LPDEVCAMSTATUS;

typedef struct cam_cw_init_param    // �ƴ������ʼ��ʹ��
{
    short nModelMode;                           // ģ�ͼ��ط�ʽ, 0:�ļ����� 1: �ڴ����
    short nLivenessMode;                        // ��������, 0:����� 1: ������� 2:�ṹ�����
    short nLicenseType;                         // ��Ȩ����, 1:оƬ��Ȩ 2��hasp��Ȩ 3:��ʱ��Ȩ 4:�ƴ��������Ȩ
    char  szConfigFile[MAX_PATH];               // �㷨�����ļ�,���Բ�д��ʹ��Ĭ��
    char  szFaceDetectFile[MAX_PATH];           // �������ģ��,���Բ�д��ʹ��Ĭ��
    char  szKeyPointDetectFile[MAX_PATH];       // �ؼ�����ģ��,���Բ�д��ʹ��Ĭ��
    char  szKeyPointTrackFile[MAX_PATH];        // �ؼ������ģ��,���Բ�д��ʹ��Ĭ��
    char  szFaceQualityFile[MAX_PATH];          // ������������ģ��,���Բ�д��ʹ��Ĭ��
    char  szFaceLivenessFile[MAX_PATH];         // ����ģ��,���Բ�д��ʹ��Ĭ��
    char  szFaceKeyPointTrackFile[MAX_PATH];    // �������ģ��,���Բ�д��ʹ��Ĭ��
    short nGpu;                                 // �Ƿ�ʹ��GPU(1/ʹ��GPU,-1ʹ��CPU)
    short nMultiThread;                         // �Ƿ���̼߳��
} CAM_CW_INIT_PARAM, *LPCAM_CW_INIT_PARAM;

typedef struct tag_dev_cam_init_param
{
    WORD wOpenType;     // �򿪷�ʽ��0:��ʾ��Ŵ� 1����ʾvid pid��
    WORD wVisCamIdx;    // �ɼ���ģ��ö�����
    WORD wNisCamIdx;    // �����ģ��ö�����
    CHAR szVisVid[4+1]; // �ɼ���ģ��vid
    CHAR szVisPid[4+1]; // �ɼ���ģ��pid
    CHAR szNisVid[4+1]; // �����ģ��vid
    CHAR szNisPid[4+1]; // �����ģ��pid
    CAM_CW_INIT_PARAM stCWParam;
} CAM_INIT_PARAM, *LPCAM_INIT_PARAM;

//////////////////////////////////////////////////////////////////////////
//#define DVECAM_NO_VTABLE  __declspec(novtable)
//////////////////////////////////////////////////////////////////////////
struct /*DVECAM_NO_VTABLE*/ IDevCAM
{
    // �ͷŽӿ�
    virtual void Release() = 0;
    // ������
    virtual long Open(LPCSTR lpMode) = 0;
    // �ر�����
    virtual long Close() = 0;
    // ��λ
    virtual long Reset() = 0;
    // ��ȡ�豸��Ϣ
    virtual long GetDevInfo(char *pInfo) = 0;
    // ȡ״̬
    virtual long GetStatus(DEVCAMSTATUS &stStatus) = 0;
    // �򿪴���(���ھ��������ָʾ:1��������/0���ٴ���, X/Y����,���ڿ��)
    virtual long Display(DWORD hWnd, WORD wAction, WORD wX, WORD wY, WORD wHidth, WORD wHeight) = 0;
    // ���� ���������ַ�lpData������FileName=C:\TEMP\TEST;Text=HELLO;TextPositionX=160;TextPositionY=120;CaptureType=0����ʽ
    virtual long TakePicture(WORD wCamera, LPCSTR lpData) = 0;
    // ����(�����ļ���,ˮӡ�ִ���ͼƬ�����Ƿ��������)(1BASE64/2JPG/4BMP;T�������)
    virtual long TakePictureEx(LPSTR lpFileName, LPCSTR lpCamData, WORD wPicType, bool isContinue, WORD wTimeOut) = 0;

    // �����ʼ������ʹ��
    virtual void vSetCamInitParam(CAM_INIT_PARAM stInitParam) = 0;
};


extern "C" long CreateIDevCAM(LPCSTR lpDevType, IDevCAM *&p);
//////////////////////////////////////////////////////////////////////////