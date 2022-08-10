/***************************************************************
* 文件名称： cloudwalk.h
* 文件描述： 云从摄像相关
* 版本历史信息
* 变更说明：
* 变更日期：
* 文件版本：1.0.0.1
****************************************************************/

#ifndef CLOUDWALK_H
#define CLOUDWALK_H


#include "QtTypeDef.h"

#ifndef CALL_MODE
#ifdef _WIN32
#define CALL_MODE	__stdcall
#else
#define CALL_MODE
#endif
#endif

#define	CW_FACE_LIVENESS_IS_LIVE CW_LIVENESS_IS_LIVE

//typedef int HANDLE;
//----宏定义----------------------------------------------------------------------------------
// 错误码
#define	CW_CAMERA_SUCCESS                               75002000				// 成功
#define	CW_CAMERA_PARA_INPUT_ERROR                      75002001				// 输入参数错误
#define	CW_CAMERA_NON_EXIST_ERROR						75002002				// 相机不存在
#define	CW_CAMERA_OPEN_FAIL                             75002003				// 相机打开失败
#define	CW_CAMERA_CLOSE_FAIL                            75002004				// 相机关闭失败
#define	CW_CAMERA_NON_CLOUD_WALK_CAMERA         		75002005				// 非云从相机
#define	CW_CAMERA_CHIP_LICENSE_ERROR                    75002006				// 读取芯片授权码失败
#define	CW_CAMERA_CREATE_HANDLE_ERROR               	75002007				// 创建相机句柄失败
#define	CW_CAMERA_HANDLE_NULL_ERROR                     75002008				// 相机句柄为空
#define	CW_CAMERA_NON_CAMERA_TYPE_ERROR             	75002009				// 不支持的相机类型
#define	CW_CAMERA_POINTER_NULL_ERROR                	75002010				// 参数指针为空
#define	CW_CAMERA_PARA_IMG_READ_ERROR                   75002011				// 成像参数读取失败
#define	CW_CAMERA_PARA_IMG_WRITE_ERROR                  75002012				// 成像参数设置失败
#define	CW_CAMERA_PARA_CAMERA_READ_ERROR				75002013				// 相机参数读取失败
#define	CW_CAMERA_PARA_CAMERA_WRITE_ERROR				75002014				// 相机参数设置失败
#define	CW_CAMERA_ENUM_CAMERA_ERROR                     75002015				// 枚举相机设备失败
#define	CW_CAMERA_ENUM_RESOLUTION_ERROR                 75002016				// 枚举相机分辨率失败
#define	CW_CAMERA_RUNING_ERR                            75002017				// 相机运行中
#define	CW_CAMERA_UNKNOWN_ERROR                         75002018				// 操作未知的错误
#define	CW_CAMERA_SET_SLEEP_ERR                         75002019				// 设置相机休眠失败
#define	CW_CAMERA_SET_WAKEUP_ERR                        75002020				// 设置相机唤醒失败
#define	CW_CAMERA_LOST_ERR                              75002021				// 相机设备断开
#define	CW_LIVENESS_INVALID_PARAM_ERROR                 75000108				// 输入参数错误
#define	CW_LIVENESS_INVALID_HANDLE_ERROR				75000109				// 无效的句柄
#define	CW_LIVENESS_READ_LICENSE_ERROR                  75000110				// 读取芯片授权码失败
#define	CW_LIVENESS_DETECTOR_UNINIT_ERROR				75000111				// 检测器未初始化
#define	CW_LIVENESS_UNKNOWN_ERROR                       75000112				// 未知错误catch
#define	CW_LIVENESS_FACE_DETECT_CALLBACK_ERROR			75000113				// 设置人脸检测回调函数错误
#define	CW_LIVENESS_LIVE_DETECT_CALLBACK_ERROR			75000114				// 设置活体检测回调函数错误
#define	CW_LIVENESS_LOG_MESSAGE_CALLBACK_ERROR			75000115				// 设置日志信息回调函数错误
#define	CW_LIVENESS_CAPTURE_FACE_IMAGE_ERROR			75000116				// 抠取人脸图像错误
#define	CW_LIVENESS_CREATE_COM_SERVER_ERROR				75000117				// 创建com服务错误
#define	CW_LIVENESS_REINIT_DETECTOR_ERROR				75000118				// 重复创建检测器
#define	CW_LIVENESS_DECODE_BASE64_ERROR                 75000119				// base64解码错误
#define	CW_LIVENESS_CHANAGE_FACE_ERROR                  75000120				// 检测到换脸
#define	CW_LIVENESS_INIT_DETECTOR_ERROR                 75000121				// 创建检测器错误
#define	CW_LIVENESS_MULTIPLE_FACES                      75000123				// 检测到多张人脸
#define	CW_LIVENESS_SUCCESSFUL                          26121000				// 成功or合法
#define	CW_FACE_FRAME_ERROR                             26121001				// 空图像
#define	CW_FACE_FORMAT_ERROR                            26121002				// 图像格式不支持
#define	CW_FACE_NO_FACE                                 26121003				// 没有人脸
#define	CW_FACE_ROI_ERROR                               26121004				// ROI设置失败
#define	CW_FACE_MINMAX_ERROR                            26121005				// 最小最大人脸设置失败
#define	CW_FACE_RANGE_ERROR                             26121006				// 数据范围错误
#define	CW_FACE_UNAUTHORIZED_ERROR                      26121007				// 未授权
#define	CW_FACE_UNINITIALIZED_ERROR                     26121008				// 尚未初始化
#define	CW_FACE_DETECT_MODEL_ERROR                      26121009				// 加载检测模型失败
#define	CW_FACE_KEYPOINT_MODEL_ERROR                    26121010				// 加载关键点模型失败
#define	CW_FACE_QUALITY_MODEL_ERROR                     26121011				// 加载质量评估模型失败
#define	CW_FACE_LIVENESS_MODEL_ERROR                    26121012				// 加载活体检测模型失败
#define	CW_FACE_DETECT_ERROR                            26121013				// 检测失败
#define	CW_FACE_KEYPOINT_ERROR                          26121014				// 提取关键点失败
#define	CW_FACE_ALIGN_ERROR                             26121015				// 对齐人脸失败
#define	CW_FACE_QUALITY_ERROR                           26121016				// 质量评估失败
#define	CW_FACE_LIVENESS_ERROR                          26121017				// 活体检测错误
#define	CW_FACE_TIMESTAMP_MATCH                         26121018				// 时间戳不匹配
#define	CW_LIVENESS_GET_DETECT_PARAM_ERROR				26121019				// 获取检测参数失败
#define	CW_LIVENESS_SET_DETECT_PARAM_ERROR				26121020				// 设置检测参数失败
#define	CW_LIVENESS_GET_VERSION_ERROR                   26121021				// 获取版本信息失败
#define	CW_LIVENESS_REMOVE_FILEPATH_ERROR				26121022				// 删除文件或文件夹失败
#define	CW_FACE_QUALITY_DEFAULT                         26121030				// 人脸检测默认值
#define	CW_FACE_LIVENESS_COLOR_PASS                     26121031				// color图像通过检测
#define	CW_FACE_LIVENESS_DIST_TOO_CLOSE_ERROR           26121032				// 人脸距离太近
#define	CW_LIVENESS_DISTANCE_TOO_FAR_ERR				26121033				// 人脸距离太远
#define	CW_FACE_LIVENESS_POSE_DETECT_FAIL_ERROR			26121034				// 人脸角度不满足要求
#define	CW_LIVENESS_FACE_CLARITY_DET_FAIL_ERR			26121035				// 人脸清晰度不满足要求
#define	CW_FACE_LIVENESS_VIS_EYE_CLOSE_ERROR			26121036				// 检测到闭眼，仅在设置参数时eyeopen为true且检测到闭眼
#define	CW_FACE_LIVENESS_VIS_MOUTH_OPEN_ERROR			26121037				// 检测到张嘴，仅在设置参数时mouthopen为true且检测到多人时返回
#define	CW_FACE_LIVENESS_VIS_BRIGHTNESS_EXC_ERROR		26121038				// 检测到人脸过亮，仅在设置参数时brightnessexc为true且人脸过亮时返回
#define	CW_FACE_LIVENESS_VIS_BRIGHTNESS_INS_ERROR		26121039				// 检测到人脸过暗，仅在设置参数时brightnessins为true且人脸过暗时返回
#define	CW_FACE_LIVENESS_VIS_FACE_LOW_CONF_ERROR		26121040				// 检测到人脸置信度过低，仅在设置参数时confidence为true且人脸置信度过低时返回
#define	CW_FACE_LIVENESS_VIS_OCCLUSION_ERROR            26121041				// 检测到人脸存在遮挡，仅在设置参数时occlusion为true且检测到遮挡时返回
#define	CW_FACE_LIVENESS_VIS_BLACKSPEC_ERROR			26121042				// 检测到黑框眼镜，仅在设置参数时blackspec为true且检测到黑框眼镜时返回
#define	CW_FACE_LIVENESS_VIS_SUNGLASS_ERROR				26121043				// 检测到墨镜，仅在设置参数时sunglass为true且检测到墨镜时返回
#define CW_LIVENESS_LIV_VIS_PROCEDUREMASK_ERR           26121044                // 检测到口罩,仅在设置参数时proceduremask>-1 且 检 测 到口罩时返回
#define	CW_FACE_LIVENESS_DEFAULT                        26121080				// 活体检测默认值
#define	CW_FACE_LIVENESS_IS_LIVE                        26121081				// 活体
#define	CW_FACE_LIVENESS_IS_UNLIVE_ERROR				26121082				// 非活体
#define	CW_FACE_LIVENESS_SKIN_FAILED_ERROR				26121083				// 人脸肤色检测未通过
#define	CW_FACE_LIVENESS_NO_PAIR_FACE_ERROR				26121084				// 可见光和红外人脸不匹配
#define	CW_FACE_LIVENESS_NIS_NO_FACE_ERROR				26121085				// 红外输入没有人脸
#define	CW_FACE_LIVENESS_VIS_NO_FACE_ERROR				26121086				// 可见光输入没有人脸

// 常用类型转换宏
#define isRight(lhs, rhs, cmp) (lhs) cmp (rhs)
#define CvtPointer(P, T) reinterpret_cast<T*>((P))
#define CvtValue(val, type) static_cast<type>((val))

//----枚举值定义--------------------------------------------------------------------------
// 图像格式CWLiveImageFormat枚举值
enum CWLiveImageFormat
{
    CW_IMAGE_GRAY8 = 0,         //灰度图
    CW_IMAGE_BGR888 = 1,        //bgrcolor图像
    CW_IMAGE_BGRA8888 = 2,      //bgracolor图像
    CW_IMAGE_YUV420P = 3,       //yuv420p(非yv21)
    CW_IMAGE_NV12 = 4,          //nv12
    CW_IMAGE_NV21 = 5,          //nv21
    CW_IMAGE_BINARY = 6,        //二进制数据
    CW_IMAGE_16U_DEPTH = 7,     //16bit深度图数据
    CW_RK_IR_40_50_RAW = 8,     //RK40/50基线设备ir raw data
    CW_RK_40_DEPTH_RAW = 9,     //RK40基线设备depth raw data
    CW_RK_50_DEPTH_RAW = 10     //RK50基线设备depth raw data
};

// 旋转角度CWLiveImageAngle枚举值
enum CWLiveImageAngle
{
    CW_IMAGE_ANGLE_0 = 0,   // 不旋转
    CW_IMAGE_ANGLE_90 = 1,  // 90度
    CW_IMAGE_ANGLE_180 = 2, // 180度
    CW_IMAGE_ANGLE_270 = 3  // 270度
};

// 镜像类型CWLiveImageMirror
enum CWLiveImageMirror
{
    CW_IMAGE_MIRROR_NONE = 0,   // 不镜像
    CW_IMAGE_MIRROR_HOR = 1,    // 水平镜像
    CW_IMAGE_MIRROR_VER = 2,    // 垂直镜像
    CW_IMAGE_MIRROR_HV = 3      // 垂直和水平镜像
};

// 相机运行状态CWCameraState枚举值
enum CWCameraState
{
    CW_CAMERA_STATE_UNKNOWN = 0,    // 状态未知
    CW_CAMERA_STATE_UNINIT,     // 未初始化
    CW_CAMERA_STATE_INIT,       // 初始化
    CW_CAMERA_STATE_OPENED,     // 打开
    CW_CAMERA_STATE_STOPPED,    // 停止
    CW_CAMERA_STATE_PAUSED,     // 暂停
    CW_CAMERA_STATE_LOST        // 丢失
};


//----结构体定义--------------------------------------------------------------------------
// 人脸框
typedef struct CWLiveFaceRect_t
{
    int    x;                                   // 人脸框左上角横坐标
    int    y;                                   // 人脸框左上角纵坐标
    int    width;                               // 人脸框宽度
    int    height;                              // 人脸框高度
}CWLiveFaceRect;

// 人脸质量分
typedef struct CWLiveFaceQualityScore_t
{
    float yaw_angle;                            // 人脸角度，左转为正，右转为负，范围-90~90，单位:°
    float pitch_angle;                          // 人脸角度，抬头为正，低头为负，范围-90~90,单位:°
    float roll_angle;                           // 人脸角度，顺时针为正，逆时针为负，范围-90~90,单位:°
    float face_confidence_score;                // 人脸置信度，范围0-1，分数越低，表示误检的概率越大
    float clarity_score;                        // 清晰度，越大表示越清晰，范围0-1
    float brightness_score;                     // 亮度，越大表示越亮，范围0-1
    float skin_score;                           // 肤色接近真人肤色程度，越大表示越真实，范围0-1
    float mouth_opening;                        // 张嘴分数， 越大表示越可能张嘴，范围0-1
    float left_eye_opening;                     // 左眼睁眼分数， 越大表示左眼越可能是睁眼，范围0-1
    float right_eye_opening;                    // 右眼睁眼分数， 越大表示右眼越可能是睁眼，范围0-1
    float blackframe_glass_score;               // 戴黑框眼镜置信度，越大表示戴黑框眼镜的可能性越大，范围0-1
    float sunglass_score;                       // 戴墨镜的置信分，越大表示戴墨镜的可能性越大，范围0-1
    float right_eye_occlusion_score;            // 眼睛被遮挡的置信度，越大表示眼睛越可能被遮挡，范围0-1
    float left_eye_occlusion_score;             // 眼睛被遮挡的置信度，越大表示眼睛越可能被遮挡，范围0-1
    float occlusion_score;                      // 人脸遮挡分数，分数越大，遮挡的概率越大，范围0-1
}CWLiveFaceQualityScore;

// 人脸检测信息结构
typedef struct CWLiveFaceDetectInformation_t{

    int                     track_id;           // 人脸跟踪id
    int						quality_errorcode;  // 质量检测返回码
    CWLiveFaceRect          rect;               // 人脸框坐标
    CWLiveFaceQualityScore	quality;			// 人脸质量

}CWLiveFaceDetectInformation;

// 图像尺寸CWSize结构体
struct CWSize
{
    short width;    //图像宽度
    short height;   //图像高度
};

// 图像数据CWLiveImage结构体
struct CWLiveImage
{
    char *data;                 // 图像数据
    int  data_length;           // 数据长度，非二进制图可不设
    int  width;                 // 宽, JPG等二进制图可不设
    int  height;                // 高, JPG等二进制图可不设
    CWLiveImageFormat format;   // 图像格式
    CWLiveImageAngle angle;     // 旋转角度
    CWLiveImageMirror mirror;   // 镜像
    long timestamp;             // 输入图像的进入时间戳
};

// 相机链表
#pragma pack(1)
typedef struct tagCWCameraDevice
{
    short index;    // 相机序号
    char name[32];  // 相机名称
    char vid[32];   // VID
    char pid[32];   // PID
    struct tagCWCameraDevice* next; // 链表指针
}CWCameraDevice, LPCWCameraDevice;
#pragma pack()

#pragma pack(1)
//分辨率链表
typedef struct tagCWResolution
{
    short width;
    short height;
    struct tagCWResolution* next;
}CWResolution;
#pragma pack()


// 回调函数类型定义
typedef void (*preview_func)(void* handle, const short mode, const char* frame, const short width,
                            const short height, const short channel);
typedef void (CALL_MODE *face_det_func)(void* handle, const long error_code, const long timestamp,
                            const CWLiveFaceDetectInformation* infos, const short face_num); // 人脸检测回调
typedef void (CALL_MODE *live_det_func)(void* handle, const long error_code, const float scores[],
                            const float distances[], const short face_num);// 活体检测回


// 人脸框回调结构体
struct cw_preview_observer
{
    // 回调函数类型定义
    //typedef void (*preview_func)(void* handle, const short mode, const char* frame, const short width,
    //                                    const short height, const short channel); // 人脸框设置回调
    HANDLE handle;
    preview_func func;
};

// 检测回调结构体
struct cw_detect_observer
{
    //typedef void (CALL_MODE *face_det_func)(void* handle, const long error_code, const long timestamp,
    //                                    const CWLiveFaceDetectInformation* infos, const short face_num); // 人脸检测回调
    //typedef void (CALL_MODE *live_det_func)(void* handle, const long error_code, const float scores[],
    //                                    const float distances[], const short face_num);// 活体检测回调
    HANDLE handle;
    face_det_func face_func; // 人脸检测回调
    live_det_func live_func; // 活体检测回调
};

//----回调函数定义--------------------------------------------------------------------------
// 视频流显示回调，已绘制好人脸框
void onPreviewCallback(void* handle, const short mode, const char* frame,
                       const short width, const short height, const short channel);
// 人脸检测回调
void onFaceDetCallback(void* handle, const long error_code, const long timestamp,
                       const CWLiveFaceDetectInformation* infos, const short face_num);

// 活体检测回调
void onLiveDetCallback(void* handle, const long error_code, const float* scores,
                       const float* distances, const short face_num);


//----接口函数定义-------------------------------------------------------------------------------
// 1.1 创建检测对象
typedef HANDLE  (CALL_MODE *pcwEngineCreateDetector)();
// 1.2 释放检测对象及COM服务
typedef long (CALL_MODE *pcwEngineReleaseDetector)(HANDLE handle);
// 1.3 获取COM服务及算法版本号
typedef char* (CALL_MODE *pcwEnginGetVersion)(HANDLE handle, const short mode);
// 1.4 注册相机回调回显函数
typedef long (CALL_MODE *pcwEngineRegisterPreviewHandle)(HANDLE handle, cw_preview_observer* previewObser);
// 1.5 注册检测回调函数对象
typedef long (CALL_MODE *pcwEngineRegisterDetectHandle)(HANDLE handle, cw_detect_observer* previewObser);
// 1.6 加载检测模型及设置检测参数
typedef long (CALL_MODE *pcwEngineInitDetectorParam)(HANDLE handle, short modelMode,  short livenessMode,
                                                    short licenseType, const char* configFile,
                                                    const char* faceDetectFile, const char* faceKeyPointDetectFile,
                                                    const char* faceKeyPointTrackFile, const char* faceQualityFile,
                                                    const char* faceLivenessFile, short gpu, short multiThread);
// 1.7 是否进行人脸追踪显示
typedef long (CALL_MODE *pcwEngineEnableFaceTrace)(HANDLE handle, bool enable);
// 1.9 依据相机在设备显示的序号开启相机
typedef long (CALL_MODE *pcwEngineOpenCamera)(HANDLE handle, const short deviceIndex, const short deviceMode,
                                             const short width, const short height);
// 1.10 依据相机硬件信息开启相机
typedef long (CALL_MODE *pcwEngineOpenCameraEx)(HANDLE handle, const char* vid, const char* pid,
                                               const short deviceMode, const short width, const short height);
// 1.11 关闭当前程序打开的所有相机设备
typedef long (CALL_MODE *pcwEngineCloseAllCameras)(HANDLE handle);
// 1.12 枚举当前设备上的所有相机
typedef long (CALL_MODE *pcwEngineEnumCameras)(HANDLE handle, CWCameraDevice **cameraLists);// 30-00-00-00(FT#0031)
// 1.13 枚举相机支持的所有分辨率(序号)
typedef long (CALL_MODE *pcwEngineEnumResolutions)(HANDLE handle, const short deviceIndex, CWResolution** resolutionLists);
// 1.14 枚举相机支持的所有分辨率(VIDPID)
typedef long (CALL_MODE *pcwEngineEnumResolutionsEx)(HANDLE handle, const char* vid,const char* pid,
                                                     CWResolution** resolutionLists);
// 1.15 开启活体检测
typedef long (CALL_MODE *pcwEngineStartLiveDetect)(HANDLE handle, bool isContinue);
// 1.16 停止连续活体检测
typedef long (CALL_MODE *pcwEngineStopLiveDetect)(HANDLE handle);
// 1.17 抓拍当前画面帧
typedef long (CALL_MODE *pcwEngineCaptureCurrentFrame)(HANDLE handle,       // 检测对象
                                                      const CWSize size,    // 画面帧分辨率
                                                      const bool isBase64,  // 是否编码输出
                                                      CWLiveImage* image);  // 当前图像帧信息
// 1.18 保存当前帧至指定文件路径参数
typedef long (CALL_MODE *pcwEngineSaveCurrentFrame)(HANDLE handle, const char* fileName);
// 1.19 开启语音提示功能
typedef long (CALL_MODE *pcwEngineEnableVoiceTip)(HANDLE handle, bool bEnable, const char *tipFile);// 30-00-00-00(FT#0031)
// 1.20 获取本次活体检测最佳人脸图像
typedef long (CALL_MODE *pcwEngineGetBestFace)(HANDLE handle,       // 检测对象
                                               const float ratio,   // 人脸图像放大比率，建议放大2
                                               bool isBase64,       // 是否编码输出
                                               CWLiveImage* image); // 当前最佳人脸信息，最佳人脸内存空间由内部创建，需要由内部释放
// 1.21 保存活体最佳人脸到指定路径
typedef long (CALL_MODE *pcwEngineSaveBestFace)(HANDLE handle,          // 检测对象
                                                const char* fileName,   // 图像文件全路径文件名
                                                const float ratio);     // 最佳人脸放大倍数
// 1.20 获取本次活体检测最佳人脸图像
typedef long (CALL_MODE *pcwEngineGetBestFace1)(HANDLE handle,       // 检测对象
                                                const float ratio,   // 人脸图像放大比率，建议放大2
                                                bool isBase64,       // 是否编码输出
                                                int type,            // 1可见光图像/2红外图像
                                                CWLiveImage* image); // 当前最佳人脸信息，最佳人脸内存空间由内部创建，需要由内部释放
// 1.21 保存活体最佳人脸到指定路径
typedef long (CALL_MODE *pcwEngineSaveBestFace1)(HANDLE handle,          // 检测对象
                                                 const char* fileName,   // 图像文件全路径文件名
                                                 int type,               // 1可见光图像/2红外图像
                                                 const float ratio);     // 最佳人脸放大倍数
// 1.22 通过序号得到相机状态
typedef long (CALL_MODE *pcwEngineGetCameraStatus)(HANDLE handle,           // 检测对象
                                                   const short index,       // 相机序号
                                                   CWCameraState* status);  // 相机状态
// 1.23 通过相机硬件信息VID/PID得到相机状态
typedef long (CALL_MODE *pcwEngineGetCameraStatusEx)(HANDLE handle, const char* vid, const char* pid, CWCameraState* status);

// 1.26 将图片转换成base64编码格式
typedef long (CALL_MODE *pcwEngineImage2Base64Format)(HANDLE handle,                // 检测对象
                                                      const char* imageFilePath,    // 图像全路径
                                                      std::string* strBase64);      // base64编码格式字符串
// 1.28 依照相机在设备上的序号休眠相机
typedef long (CALL_MODE *pcwEngineSleepCamera)(HANDLE handle,const short deviceIndex,bool isSleep);
// 1.29 依照相机硬件信息VID/PID休眠相机
typedef long (CALL_MODE *pcwEngineSleepCameraEx)(HANDLE handle,const char* vid,const char* pid, bool isSleep);
// 1.32 获得相机固件版本信息，依照相机在设备上的序号
typedef char* (CALL_MODE *pcwEngineGetFirmwareVersion)(HANDLE handle,               // 检测对象
                                                       const short deviceIndex);    // 相机序号
// 1.33 获得相机固件版本信息，依照相机在设备VID/PID
typedef char* (CALL_MODE *pcwEngineGetFirmwareVersionEx)(HANDLE handle, const char* vid, const char* pid);

#endif // CLOUDWALK_H
