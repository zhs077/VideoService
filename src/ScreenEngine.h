#pragma once
#include "platform_config.h"
#include <queue>
using namespace std;

class ReceivedImage {
public:
	ReceivedImage(UCHAR *pImg, UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp) {
		int nSize = ImgWidth * ImgHeight * 3 / 2;
		this->pImg = new UCHAR[nSize];
		memcpy(this->pImg, pImg, nSize);
		this->ImgWidth = ImgWidth;
		this->ImgHeight = ImgHeight;
		this->uiTimeStamp = uiTimeStamp;
	}

	~ReceivedImage() {
		delete[] this->pImg;
	}
public:
	UCHAR* pImg;
	UINT ImgWidth;
	UINT ImgHeight;
	UINT uiTimeStamp;
};

class ImagePool 
{
public:
	ImagePool() {
		//InitializeCriticalSection(&m_cs);
	}
	~ImagePool()
	{
		//DeleteCriticalSection(&m_cs);

	}
public:
	void push(UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp);
	auto_ptr<ReceivedImage> pop()
	{
		//EnterCriticalSection(&m_cs);
		int szSize = m_imageQueue.size();
		//int dropSize=szSize / 5;
		int dropSize=0;
		while(dropSize--) 
		{
			ReceivedImage* result = m_imageQueue.front();
			m_imageQueue.pop();
			delete result;
		}
			ReceivedImage* result;
		//if (szSize <= 0)
		//{/
				//LeaveCriticalSection(&m_cs);
			//	return auto_ptr<ReceivedImage>(NULL);
		
		//}
		result = m_imageQueue.front();
		m_imageQueue.pop();
		//LeaveCriticalSection(&m_cs);
		return auto_ptr<ReceivedImage>(result);
	}

	int  getCount() { return m_imageQueue.size(); }

	bool isEmpty() { return m_imageQueue.empty(); }
private:
	queue<ReceivedImage*> m_imageQueue;
	CRITICAL_SECTION m_cs;
};

