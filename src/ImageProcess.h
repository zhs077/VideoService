#ifndef IMAGE_PROCESS_H_
#define IMAGE_PROCESS_H_
#include "myfunction.h"
#include "platform_config.h"


typedef struct render_baton_t
{
	//×ª»¯
	IplImage *srcImg;

	CvSize cz;
	//BGRÍ¼Ïñ
	int width;
	int height;
	int widthStep;
	//YUVÍ¼Ïñ
	int halfWidth ;
	int halfHeight ;
	int imgSize ;
	int yuvBytes ;
	IplImage* dstImg;
	IplImage* dstImg1;

	//·ÖÅäÏÔ´æ
	int bgrStep, rgbStep, hsvStep,hsvStep1;
	uchar* d_pBGR ;
	uchar* d_pRGB ;
	uchar* d_pHSV;
	uchar* d_pHSV1;

	int  ypitch, upitch, vpitch, dpitch;
	uchar* d_pY;
	uchar* d_pU ;
	uchar* d_pV ;
	uchar* d_pD;
	uchar* h_pHSV;

}render_baton_t;

IplImage* YUV420_To_IplImage_Opencv(unsigned char* pYUV420, int width, int height);
void alloc_cuda_memory(render_baton_t* baton);
IplImage* convert(uchar *pYUV, render_baton_t* baton);
void free_cuda_memory(render_baton_t *baton);
//void lightEnhance(unsigned char* srcImg, int srcStep, int width, int height, int ValueA);
void lightBalance(unsigned char* srcImg, int srcStep, int width, int height,int value, unsigned char* srcImg1);




#endif