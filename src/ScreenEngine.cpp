
#include "ScreenEngine.h"
#include "platform_config.h"

//////////////////////////////////////////////////////////////////////////
void ImagePool::push(UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp) {
	EnterCriticalSection(&m_cs);
	//calculator.onReceivedImage();
	m_imageQueue.push(new ReceivedImage(pImg[0], ImgWidth, ImgHeight, uiTimeStamp));
	LeaveCriticalSection(&m_cs);
}



