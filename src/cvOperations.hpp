#ifndef _CV_OPERATION_HPP_94296ceb_9adc_467d_a3c9_dbb75dcd0bd5
#define _CV_OPERATION_HPP_94296ceb_9adc_467d_a3c9_dbb75dcd0bd5
#include <opencv/cv.h>
#include <opencv/highgui.h>

namespace cvo {

	inline CvPoint appendMessage(IplImage* image, TCHAR* message, CvPoint& drawPlace) {
		CvPoint nextLine(drawPlace);
		CvFont font[2];
		double hScale=1;   
		double vScale=1;    
		int lineWidth[]= { 1, 4};// 相当于写字的线条
		// 初始化字体   
		for(int i = 0; i < 2; ++ i) {
			cvInitFont(font + i,CV_FONT_HERSHEY_PLAIN, hScale,vScale,0, lineWidth[i]);//初始化字体，准备写到图片上的 
		}
		// cvPoint 为起笔的x，y坐标   
		cvPutText(image, message, drawPlace, font + 1, CV_RGB(0,0,0));//在图片中输出字符  
		cvPutText(image, message, drawPlace, font + 0, CV_RGB(255,255,255));//在图片中输出字符
		nextLine.y += 20;
		return nextLine;
	}

	inline CvPoint appendTime(IplImage* image, CvPoint& drawPlace, LPSYSTEMTIME lpSystemTime = NULL,
		LPCTSTR lpszTitle = NULL, LPCTSTR lpszFormat = _T("%04d/%02d/%02d %02d:%02d:%02d-%03d")) {

		SYSTEMTIME systemTime;
		if(NULL == lpSystemTime) {
			GetLocalTime(&systemTime);
			lpSystemTime = &systemTime;
		}
		TCHAR tszFormat[MAX_PATH];
		if(NULL == lpszTitle) {
			_stprintf_s(tszFormat, MAX_PATH - 1, _T("%s"), lpszFormat);
		} else {
			_stprintf_s(tszFormat, MAX_PATH - 1, _T("[%-15s]%s"), lpszTitle, lpszFormat);
		}
		TCHAR tszTime[MAX_PATH];
		_stprintf_s(tszTime, MAX_PATH - 1, tszFormat,
			lpSystemTime->wYear, lpSystemTime->wMonth, lpSystemTime->wDay,
			lpSystemTime->wHour, lpSystemTime->wMinute, lpSystemTime->wSecond,
			lpSystemTime->wMilliseconds);
			//tmCur.GetYear(), tmCur.GetMonth(), tmCur.GetDay(),
			//tmCur.GetHour(), tmCur.GetMinute(), tmCur.GetSecond(), tmCur.get);
		return appendMessage(image, tszTime, drawPlace);
	}

}

#endif // ! _CV_OPERATION_HPP_94296ceb_9adc_467d_a3c9_dbb75dcd0bd5