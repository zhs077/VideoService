
#include "ScreenEngine.h"
#include "platform_config.h"



//#pragma comment(lib,"ImgProcess.lib") //引用库文件

//////////////////////////////////////////////////////////////////////////

// IplImage* YUV420_To_IplImage_Opencv(unsigned char* pYUV420, int width, int height)
// {
// 	if (!pYUV420)
// 	{
// 		return NULL;
// 	}
// 
// 	IplImage *yuvimage,*rgbimg,*yimg,*uimg,*vimg,*uuimg,*vvimg;
// 
// 	int nWidth = width;
// 	int nHeight = height;
// 	rgbimg = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,3);
// 	yuvimage = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,3);
// 
// 	yimg = cvCreateImageHeader(cvSize(nWidth, nHeight),IPL_DEPTH_8U,1);
// 	uimg = cvCreateImageHeader(cvSize(nWidth/2, nHeight/2),IPL_DEPTH_8U,1);
// 	vimg = cvCreateImageHeader(cvSize(nWidth/2, nHeight/2),IPL_DEPTH_8U,1);
// 
// 	uuimg = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,1);
// 	vvimg = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,1);
// 
// 	cvSetData(yimg,pYUV420, nWidth);
// 	cvSetData(uimg,pYUV420+nWidth*nHeight, nWidth/2);
// 	cvSetData(vimg,pYUV420+long(nWidth*nHeight*1.25), nWidth/2);
// 	cvResize(uimg,uuimg,CV_INTER_LINEAR);
// 	cvResize(vimg,vvimg,CV_INTER_LINEAR);
// 
// 	cvMerge(yimg,uuimg,vvimg,NULL,yuvimage);
// 	cvCvtColor(yuvimage,rgbimg,CV_YCrCb2RGB);
// 
// 	cvReleaseImage(&uuimg);
// 	cvReleaseImage(&vvimg);
// 	cvReleaseImageHeader(&yimg);
// 	cvReleaseImageHeader(&uimg);
// 	cvReleaseImageHeader(&vimg);
// 
// 	cvReleaseImage(&yuvimage);
// 
// 	if (!rgbimg)
// 	{
// 		return NULL;
// 	}
// 
// 	return rgbimg;
// }
//////////////////////////////////////////////////////////////////////////
void ImagePool::push(UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp) {
	EnterCriticalSection(&m_cs);
	//calculator.onReceivedImage();
	m_imageQueue.push(new ReceivedImage(pImg[0], ImgWidth, ImgHeight, uiTimeStamp));
	LeaveCriticalSection(&m_cs);
}



