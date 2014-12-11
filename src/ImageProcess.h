#ifndef IMAGE_PROCESS_H_
#define IMAGE_PROCESS_H_
#include "myfunction.h"
#include "platform_config.h"



IplImage* YUV420_To_IplImage_Opencv(unsigned char* pYUV420, int width, int height)
{
	if (!pYUV420)
	{
		return NULL;
	}

	IplImage *yuvimage,*rgbimg,*yimg,*uimg,*vimg,*uuimg,*vvimg;

	int nWidth = width;
	int nHeight = height;
	rgbimg = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,3);
	yuvimage = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,3);

	yimg = cvCreateImageHeader(cvSize(nWidth, nHeight),IPL_DEPTH_8U,1);
	uimg = cvCreateImageHeader(cvSize(nWidth/2, nHeight/2),IPL_DEPTH_8U,1);
	vimg = cvCreateImageHeader(cvSize(nWidth/2, nHeight/2),IPL_DEPTH_8U,1);

	uuimg = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,1);
	vvimg = cvCreateImage(cvSize(nWidth, nHeight),IPL_DEPTH_8U,1);

	cvSetData(yimg,pYUV420, nWidth);
	cvSetData(uimg,pYUV420+nWidth*nHeight, nWidth/2);
	cvSetData(vimg,pYUV420+long(nWidth*nHeight*1.25), nWidth/2);
	cvResize(uimg,uuimg,CV_INTER_LINEAR);
	cvResize(vimg,vvimg,CV_INTER_LINEAR);

	cvMerge(yimg,uuimg,vvimg,NULL,yuvimage);
	cvCvtColor(yuvimage,rgbimg,CV_YCrCb2RGB);

	cvReleaseImage(&uuimg);
	cvReleaseImage(&vvimg);
	cvReleaseImageHeader(&yimg);
	cvReleaseImageHeader(&uimg);
	cvReleaseImageHeader(&vimg);

	cvReleaseImage(&yuvimage);

	if (!rgbimg)
	{
		return NULL;
	}

	return rgbimg;
}



class CFpsCounter {
public:
	CFpsCounter(int nMaxSecond)
		: m_nMaxSecond(nMaxSecond)
		, m_nThisSecond(0)
		, m_nThisSecondImg(0)
		, m_nLastSecond(0)
		, m_nLastSecondImg(0) {
			
	}
	

public:
	void inc() {
		
		int nTime = time(NULL);
		if(nTime != m_nThisSecond) {
			m_nLastSecond = m_nThisSecond;
			m_nLastSecondImg = m_nThisSecondImg;
			m_nThisSecondImg = 1;
			m_nThisSecond = nTime;
			findAndInsertIfNotExists(m_mapTimeToFps, m_nLastSecond)
				= m_nLastSecondImg;
		} else {
			++ m_nThisSecondImg;
		}
		
	}

public:
	int getFps() {
		return m_nLastSecondImg;
	}

	int getAverageFps() {
		int t = time(NULL) - m_nMaxSecond;
		int total = 0;
		while(m_mapTimeToFps.begin() != m_mapTimeToFps.end()) {
			if(m_mapTimeToFps.begin()->first < t)
				m_mapTimeToFps.erase(m_mapTimeToFps.begin());
			else break;
		}
		for(map< int, int>::iterator itor = m_mapTimeToFps.begin();
			m_mapTimeToFps.end() != itor; ++ itor)
		{
			total += itor->second;
		}
		return total / m_nMaxSecond;
	}

private:
	map< int, int> m_mapTimeToFps;
	 int m_nMaxSecond;
	 int m_nThisSecond;
	 int m_nThisSecondImg;
	 int m_nLastSecond;
	 int m_nLastSecondImg;
	 	CRITICAL_SECTION m_cs;
	
};

class CVideoCalculator
	: public CFpsCounter
{
public:
	CVideoCalculator(void);
	~CVideoCalculator(void);

public:
	void onReceivedImage();

public:
	int getFps();

public:
	int getAverageFps();

private:
	//CFpsCounter m_fpsCounter;
};
#endif