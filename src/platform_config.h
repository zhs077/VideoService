
#ifndef _PLATFORM_CINFIG_H_
#define  _PLATFORM_CINFIG_H_

//如果是nodejs环境请加上
//#define NODEJS

#include <vector>
#include <map>
#include <list>
#include <queue>
#include<cassert>
#include <utility>
#include<string>
#include<cstring>
#include<cstdio>
#include<cstdlib>
#include<iostream>
#include<sstream>
#include <iomanip>
using namespace std;


#if defined (__unix) || defined (__linux__) || defined(__QNX__) || defined (_AIX)
	#ifndef OS_LINUX
	#define OS_LINUX
	#endif
#elif defined (_WIN32) || defined (__WIN32) || defined (WIN32) || defined (__WIN32__)
	#ifndef OS_WIN32
	#define OS_WIN32
	#endif
#endif


#if defined  OS_LINUX
	#define strncpy_s(dest,num,src,count) strncpy(dest,src,count)
	#define sprintf_s  snprintf
	#include<iconv.h>
	int gb2312_to_utf8(char *in, char *out, size_t size);
	int utf8_to_gb2312(const char* sIn,char* sOut,size_t size);


#elif defined OS_WIN32 
#include <windows.h>
/**************** 运行时需要的lib ****************/
#pragma comment(lib,"cudart.lib")
#pragma comment(lib, "nppi.lib")

#pragma comment(lib,"ImageConvertP.lib")
#pragma comment(lib,"ws2_32.lib")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"libuv.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"libconfig.lib")
#ifdef _DEBUG
#pragma comment(lib,"log4cplusD.lib")
#pragma comment(lib,"opencv_calib3d249d.lib")
#pragma comment(lib,"opencv_contrib249d.lib")
#pragma comment(lib,"opencv_core249d.lib")
#pragma comment(lib,"opencv_features2d249d.lib")
#pragma comment(lib,"opencv_flann249d.lib")
#pragma comment(lib,"opencv_gpu249d.lib")
#pragma comment(lib,"opencv_highgui249d.lib")
#pragma comment(lib,"opencv_imgproc249d.lib")
#pragma comment(lib,"opencv_legacy249d.lib")
#pragma comment(lib,"opencv_ml249d.lib")
#pragma comment(lib,"opencv_objdetect249d.lib")
#pragma comment(lib,"opencv_video249d.lib")
#else
#pragma comment(lib,"log4cplus.lib")
#pragma comment(lib,"opencv_calib3d249.lib")
#pragma comment(lib,"opencv_contrib249.lib")
#pragma comment(lib,"opencv_core249.lib")
#pragma comment(lib,"opencv_features2d249.lib")
#pragma comment(lib,"opencv_flann249.lib")
#pragma comment(lib,"opencv_gpu249.lib")
#pragma comment(lib,"opencv_highgui249.lib")
#pragma comment(lib,"opencv_imgproc249.lib")
#pragma comment(lib,"opencv_legacy249.lib")
#pragma comment(lib,"opencv_ml249.lib")
#pragma comment(lib,"opencv_objdetect249.lib")
#pragma comment(lib,"opencv_video249.lib")
#endif // _DEBUG

#pragma comment(lib,"CRAudioEnc.lib")
#pragma comment(lib,"Nrcapc7.lib")
#pragma comment(lib,"PCCall.lib")
#pragma comment(lib,"PlayAudio.lib")
#pragma comment(lib,"VARender.lib")
#pragma comment(lib,"VAStorage.lib")







#endif

/**************** CDUA ****************/

/**************** LOG4CPLUS ****************/
#include <log4cplus/logger.h>
#include <log4cplus/fileappender.h>
#include <log4cplus/layout.h>
#include <log4cplus/configurator.h>
#include <log4cplus/helpers/stringhelper.h>
#include <log4cplus/loggingmacros.h>
using namespace log4cplus;
/**************** 第三方视频库 ****************/
#include <nrcapc7/Nrcapc7.h>
#include <nrcapc7/NrcapError.h>
#include <nrcapc7/VARender.h>
/**************** OpenCV库 ****************/
#include <opencv/cv.h>
#include <opencv/highgui.h>
using namespace cv;

/**************** 运行时配置文件 ****************/
#ifndef RUNTIME_CONFIG_FILE_SETUP
#define RUNTIME_CONFIG_FILE_SETUP
#define LOG4CPLUS_CONFIG_FILE "log4cplus.conf"
#define RUNTIME_CONFIG_FILE   "VideoService.ini"
#define VERSION_CONFIG_FILE   "CarAnalysis.ver.ini"
#define PID_CONFIG_FILE       "CarAnalysis.pid"
#endif
/**************** 公共函数****************/
string get_application_path(string &str);
string ExtractFilePath(string filename);
void Sleep_i(int mseconds);


typedef  uchar UCHAR;

typedef  unsigned int UINT;

#endif


//extern "C" 

//void lightBalance(unsigned char* srcImg, int srcStep, int width, int height, unsigned char* srcImg1);
//void lightEnhance(unsigned char* srcImg, int srcStep, int width, int height, int ValueA);
void lightBalance(unsigned char* srcImg, int srcStep, int width, int height,int threshold);
