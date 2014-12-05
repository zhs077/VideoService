
#include "TcpServer.h"
#include "platform_config.h"
#include "C7Platform.h"
#include "minidump.h"
#include "ImageProcess.h"
#define MAX_NUM_RESOURCE 100
#define MAX_FRAME_LEN	1024*1024 //最大帧大小
TcpServer server;
int index = 0;
HANDLE hVARender;		//媒体库句柄*/

int __stdcall DecodeVideoCallBack(UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp, void *pContext)
{

	const char *puid = (const char *)pContext;
	assert(puid);
	if (puid == NULL)
	{
		return 0;
	}
	IplImage *img = YUV420_To_IplImage_Opencv(pImg[0], ImgWidth, ImgHeight);
	if (img == NULL)
	{
		return 0;
	}
	multimap<string,int>puid_client_map = server.puid_client_map;
	uv_mutex_lock(&server.mutex_puid_client);
	int num = puid_client_map.count(puid);
	auto it = puid_client_map.find(puid);

	for (int i=0; i<num;i++)
	{
		//server.Send(it->second,pImg,ImgWidth,ImgHeight,uiTimeStamp);
		server.Send(it->second,img);
		++it;

	}
	uv_mutex_unlock(&server.mutex_puid_client);
	cvReleaseImage(&img);
	//delete client;
	//client->Close();
	

	//注意AcceptClient 可能已经析构了
	
//	client->image_pool.push(&pImg[0],ImgWidth,ImgHeight,uiTimeStamp);
	//client->Send(&pImg[0],ImgWidth,ImgHeight,uiTimeStamp);
	return 0;
}
void ThreadFun(void *arg)
{
	AcceptClient* client = (AcceptClient*)arg;
	auto it =C7Platform::resourceMap.find(client->resource_puid);
	if (it == C7Platform::resourceMap.end())
	{
		return;
	}
	auto puid_count  = server.puid_count_map.find(client->resource_puid);
	if (puid_count == server.puid_count_map.end())
	{
		return;
	}
	vector<ResourceInfo> selectInfo = it->second;
	C7_HRESOURCE ResourcesArr[MAX_NUM_RESOURCE] = { NULL };
	int nResCnt = MAX_NUM_RESOURCE;
	int rv;
// 	int rv = C7_ForkPUResource(selectInfo[0].hresource,ResourcesArr,&nResCnt);
// 	if (rv != NRCAP_SUCCESS)
// 	{
// 		//LOG4CPLUS_ERROR(logger," Error: C7_ForkPUResource ERROR_CODE:"<<rv);
// 		return;
// 
// 	}
	C7_HSTREAM hStreamIV;	//视频流句柄
	rv = C7_StartStream(selectInfo[1].hresource,ST_REALTIME,&hStreamIV);
	if (rv != NRCAP_SUCCESS)
	{
		//LOG4CPLUS_ERROR(logger," Error: C7_StartStream ERROR_CODE:"<<rv);
		exit(-1);

	}
	hVARender =VADR_Init(100);
	rv = VADR_SetVideoDecodeCallBack(hVARender, DecodeVideoCallBack, FRAMEFMT_YUV420,(void*)puid_count->first.c_str());
	if (rv != NRCAP_SUCCESS)
	{
		//LOG4CPLUS_ERROR(logger," Error: VADR_SetVideoDecodeCallBack ERROR_CODE:"<<rv);
		return ;
	}
	char windows_name[100]={0};
	sprintf(windows_name, "windows%d", ++index);
	cvNamedWindow(windows_name, CV_WINDOW_AUTOSIZE);
	HWND hPlayHwnd = (HWND)cvGetWindowHandle(windows_name);
	cvResizeWindow(windows_name, 720, 576 );
	rv= VADR_SetPlayWnd(hVARender, hPlayHwnd);
	if (rv !=NRCAP_SUCCESS)
	{
		//LOG4CPLUS_ERROR(logger," Error: VADR_SetPlayWnd ERROR_CODE:"<<rv);
		return ;
	}
	//开始解码显示
	rv = VADR_StartPreview(hVARender);
	if (rv !=NRCAP_SUCCESS)
	{
		//LOG4CPLUS_ERROR(logger," Error: VADR_StartPreview ERROR_CODE:"<<rv);
		return ;
	}
	unsigned char buffer[MAX_FRAME_LEN]={0};
	int nLen = MAX_FRAME_LEN;
	int nFrameType = 0;
	int nKeyFrameFlag = 0;
	
	while (puid_count->second >=1)
	{
 		memset(buffer, 0, MAX_FRAME_LEN);
 		nLen = MAX_FRAME_LEN;
 		nFrameType = 0;
 		nKeyFrameFlag = 0;
 		rv = C7_ReceiveFrame(hStreamIV,(char*)buffer,&nLen,&nFrameType,&nKeyFrameFlag);
 		if(rv == NRCAP_ERROR_DC_WOULDBLOCK)
 		{
 			continue;
 		}
 		if (rv != NRCAP_SUCCESS)
 		{
 		//	LOG4CPLUS_ERROR(logger," Error: C7_ReceiveFrame ERROR_CODE:"<<rv);
 			break;
 
 		}
 		if (nFrameType == frame_type_video)
 		{
 			rv = VADR_PumpVideoFrame(hVARender, buffer, nLen);
 
 		}
 		cvWaitKey(50);

	}
	printf("ThreadFun Exit\n");
	cvDestroyWindow(windows_name);
	//一定要释放资源
	if (hStreamIV != NULL)
	{
		rv = C7_StopStream(hStreamIV);
		if (rv != NRCAP_SUCCESS)
		{
			//LOG4CPLUS_ERROR(logger," Error: C7_StopStream ERROR_CODE:"<<rv);
			return;
		}
	}
	// 停止解码显示
	//rv =VADR_StopPreview(hVARender);
	if (rv != NRCAP_SUCCESS)
	{
		//LOG4CPLUS_ERROR(logger," Error: VADR_StopPreview ERROR_CODE:"<<rv);
		return;
	}
	//VADR_Close(hVARender);
	//C7_Close(C7Platform::session);
	//C7_Terminate();
	uv_mutex_lock(&server.mutex_puid_client);
	int num = server.puid_client_map.count(puid_count->first);
	auto it2 = server.puid_client_map.find(puid_count->first);

	for (int i=0; i<num;i++)
	{
		
		server.CloseClient(it2->second);
		++it2;
	}
	uv_mutex_unlock(&server.mutex_puid_client);
	

}
void ReadCB(int cliendid, void *clientdata, const  char* buf,void *serverdata)
{
	
	TcpServer *server = (TcpServer*)serverdata;
	assert(server);
	AcceptClient *client = (AcceptClient *)clientdata;


	
	uv_mutex_lock(&server->mutex_puid_count);
	auto it = server->puid_count_map.find(buf);
	if (it != server->puid_count_map.end())
	{
		++it->second;
	}
	else
	{
		server->puid_count_map.insert(make_pair(buf,1));
	}
	uv_mutex_unlock(&server->mutex_puid_count);

	uv_mutex_lock(&server->mutex_puid_client);
	int num = server->puid_client_map.count(buf);
	if (num == 0) //无该puid视频创建线程获取视频
	{
		client->resource_puid = buf;
		client->isplay = true;
		uv_thread_create(&client->threadId,ThreadFun,client);
		server->puid_thread_map.insert(make_pair(buf,client->threadId));
		
	}
	server->puid_client_map.insert(make_pair(buf,cliendid));
	uv_mutex_unlock(&server->mutex_puid_client);
	
	
	/*
	AcceptClient* client = (AcceptClient*)userdata;
	assert(client);
	//151038402365732956
	if (strcmp((const char*)buf,"0") != 0)
	{
			client->resource_puid = buf;
			client->isplay = true;
			uv_thread_create(&client->threadId,ThreadFun,client);
	}
	else
	{
	
		//client->ready = false;
	}
	*/

	
}
void NewConnect(int clientid, void* userdata)
{
	TcpServer * server = (TcpServer*)userdata;
	fprintf(stdout,"new connect:%d\n",clientid);
	server->SetRecvCB(clientid,ReadCB,server);
	
}
void CloseCB(int clientid, void* userdata)
{
	fprintf(stdout,"client %d close\n",clientid);
	TcpServer *theclass = (TcpServer *)userdata;
}

  

int main()
{
	MiniDump::InitMinDump();
	string current_path;
	current_path = get_application_path(current_path);
	C7Platform c7Platform;
	C7Platform::Init(current_path);
	
	server.SetNewConnectCB(NewConnect,&server);
	server.SetClosedCB(CloseCB,&server);
	server.Start("127.0.0.1",9003);

	//Sleep(12000);
	while(true) 
	{
		Sleep(10000);
	}
	server.Close();
	C7Platform::UnInit();
	return 0;
}