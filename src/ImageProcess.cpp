#include "ImageProcess.h"
#include <cuda_runtime.h>
#include <npp.h>
#include <helper_cuda.h>
#include <Exceptions.h>



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

void alloc_cuda_memory(render_baton_t* baton)
{
	assert(baton != NULL);
	baton->cz = cvSize(720,576);
	baton->srcImg = cvCreateImage(baton->cz,IPL_DEPTH_8U,3);

	//BGR图像
	baton->width = baton->srcImg->width;
	baton->height = baton->srcImg->height;
	baton->widthStep = baton->srcImg->widthStep;

	cvReleaseImage(&baton->srcImg);
	//YUV图像
	baton->halfWidth = baton->width>>1;
	baton->halfHeight = baton->height>>1;
	baton->imgSize = baton->width * baton->height;
	baton->yuvBytes = baton->imgSize + (baton->imgSize>>1);

	baton->dstImg = cvCreateImage(cvSize(baton->width,baton->height), IPL_DEPTH_8U, 3);
	baton->dstImg1 = cvCreateImage(cvSize(baton->width,baton->height), IPL_DEPTH_8U, 3);

	baton->d_pBGR = nppiMalloc_8u_C3(baton->width, baton->height, &baton->bgrStep);
	baton->d_pRGB = nppiMalloc_8u_C3(baton->width, baton->height, &baton->rgbStep);
	baton->d_pHSV = nppiMalloc_8u_C3(baton->width, baton->height, &baton->hsvStep);
	baton->d_pHSV1 = nppiMalloc_8u_C3(baton->width, baton->height, &baton->hsvStep1);
	baton->d_pY = nppiMalloc_8u_C1(baton->width, baton->height, &baton->ypitch);
	baton->d_pU = nppiMalloc_8u_C1(baton->halfWidth, baton->halfHeight, &baton->upitch);
	baton->d_pV = nppiMalloc_8u_C1(baton->halfWidth, baton->halfHeight, &baton->vpitch);
	baton->d_pD = nppiMalloc_8u_C3(baton->width, baton->height, &baton->dpitch);
	baton->h_pHSV = (uchar*)malloc(baton->width*baton->height*3);
}
void free_cuda_memory(render_baton_t *baton)
{
	nppiFree(baton->d_pBGR);
	nppiFree(baton->d_pRGB);
	nppiFree(baton->d_pHSV);
	nppiFree(baton->d_pHSV1);
	nppiFree(baton->d_pY);
	nppiFree(baton->d_pU);
	nppiFree(baton->d_pV);
	nppiFree(baton->d_pD);
	free(baton->h_pHSV);
	cvReleaseImage(&baton->dstImg);
	cvReleaseImage(&baton->dstImg1);
}
IplImage* convert(uchar *pYUV, render_baton_t* baton)
{

	uchar* Y = pYUV;
	uchar* U = pYUV + baton->imgSize;
	uchar* V = U + (baton->imgSize>>2);
	checkCudaErrors( cudaSetDevice(0) );
	//	checkCudaErrors( cudaDeviceSynchronize() ); 
	clock_t stop1 = clock();
	//	cout<<"初始化分配显存："<<stop1-start<<endl;

	//图像送到显卡并且处理，处理后送回内存
	checkCudaErrors( cudaMemcpy2D(baton->d_pY, baton->ypitch, Y, baton->width,
		baton->width, baton->height,cudaMemcpyHostToDevice ) );
	checkCudaErrors( cudaMemcpy2D(baton->d_pU, baton->upitch, U, baton->halfWidth,
		baton->halfWidth, baton->halfHeight,cudaMemcpyHostToDevice ) );
	checkCudaErrors( cudaMemcpy2D(baton->d_pV, baton->vpitch, V, baton->halfWidth,
		baton->halfWidth, baton->halfHeight,cudaMemcpyHostToDevice ) );
	NppiSize oSizeROI = { baton->width, baton->height };
	const int aDstOrder[3] = {2, 1, 0};
	Npp8u * pSrc[3] = {baton->d_pY,baton->d_pU,baton->d_pV};
	int rSrcStep[3] = { baton->ypitch, baton->upitch, baton->vpitch };
	NPP_CHECK_NPP ( nppiYUV420ToRGB_8u_P3C3R(pSrc, rSrcStep, baton->d_pD, baton->dpitch, oSizeROI) );
	NPP_CHECK_NPP( nppiRGBToHSV_8u_C3R(baton->d_pD, baton->dpitch, baton->d_pHSV, baton->hsvStep, oSizeROI) );
	checkCudaErrors( cudaMemcpy2D(baton->h_pHSV, baton->width*3, baton->d_pHSV, baton->hsvStep, baton->width*3, baton->height,cudaMemcpyDeviceToHost ) );
	cudaMemcpy(baton->d_pHSV,(baton->h_pHSV),baton->width*baton->height*sizeof(uchar3),cudaMemcpyHostToDevice); 
	lightBalance(baton->d_pHSV, baton->hsvStep, baton->width, baton->height, 1,baton->d_pHSV1);
	//lightBalance(baton->d_pHSV, baton->hsvStep, baton->width, baton->height, 1,baton->d_pHSV1);
	// baton->d_pHSV1 = baton->d_pHSV;
	cudaMemcpy((baton->h_pHSV), baton->d_pHSV1, baton->width*baton->height*sizeof(uchar3),cudaMemcpyDeviceToHost);
	checkCudaErrors( cudaMemcpy2D(baton->d_pBGR,baton->bgrStep, baton->h_pHSV, baton->widthStep, 
		baton->width*3, baton->height,cudaMemcpyHostToDevice ) );
	NPP_CHECK_NPP(nppiHSVToRGB_8u_C3R(baton->d_pBGR, baton->bgrStep, baton->d_pRGB, baton->rgbStep, oSizeROI));
	NPP_CHECK_NPP(nppiSwapChannels_8u_C3R(baton->d_pRGB, baton->rgbStep, baton->d_pBGR, baton->bgrStep, oSizeROI, aDstOrder) );

	checkCudaErrors( cudaMemcpy2D(baton->dstImg->imageData, baton->widthStep, baton->d_pBGR, baton->bgrStep, 
		baton->widthStep, baton->height, cudaMemcpyDeviceToHost) );
	//checkCudaErrors( cudaDeviceSynchronize() ); 
	clock_t stop2 = clock();
	cout<<"处理所用时间:"<<stop2 - stop1<<"ms"<<endl;

	//处理结束，并且释放显存和内存。
	// cvNamedWindow("Light");
	// cvShowImage("Light",dstImg);
	// cvWaitKey();

	//转回YUV，传输编码传输
	//NPP_CHECK_NPP(nppiRGBToYUV420_8u_C3P3R(baton->d_pRGB, baton->rgbStep, pSrc, rSrcStep, oSizeROI));
	//rSrcStep 为[3]类型 需转化为 *p 然后送回内存
	//	ShowImage = dstImg
	return baton->dstImg;
	//cvReleaseImage(&dstImg);


	//	checkCudaErrors( cudaDeviceSynchronize() ); 
	//	clock_t stop3 = clock();
	//	cout<<"释放显存时间："<<stop3-stop2<<endl;


}