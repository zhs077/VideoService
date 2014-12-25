//                            _ooOoo_   
//                           o8888888o   
//                           88" . "88   
//                           (| -_- |)   
//                            O\ = /O   
//                        ____/`---'\____   
//                      .   ' \\| |// `.   
//                       / \\||| : |||// \   
//                     / _||||| -:- |||||- \   
//                       | | \\\ - /// | |   
//                     | \_| ''\---/'' | |   
//                      \ .-\__ `-` ___/-. /   
//                   ___`. .' /--.--\ `. . __   
//                ."" '< `.___\_<|>_/___.' >'"".   
//               | | : `- \`.;`\ _ /`;.`/ - ` : | |   
//                 \ \ `-. \_ __\ /__ _/ .-` / /   
//         ======`-.____`-.___\_____/___.-`____.-'======   
//                            `=---='   
//   
//         .............................................   
//                  佛祖保佑             永无BUG  
//          佛曰:   
//                  写字楼里写字间，写字间里程序员；   
//                  程序人员写程序，又拿程序换酒钱。   
//                  酒醒只在网上坐，酒醉还来网下眠；   
//                  酒醉酒醒日复日，网上网下年复年。   
//                  但愿老死电脑间，不愿鞠躬老板前；   
//                  奔驰宝马贵者趣，公交自行程序员。   
//                  别人笑我忒疯癫，我笑自己命太贱；   
//                  不见满街漂亮妹，哪个归得程序员？  

#include "TcpServer.h"
#include "platform_config.h"
#include "C7Platform.h"
#include "minidump.h"
#include "ImageProcess.h"
#include "ErrorCode.h"
#include "configlib.h"


using namespace uv;
#define MAX_NUM_RESOURCE 100
//#define MAX_FRAME_LEN	1024*1024 //最大帧大小
TCPServer server;
Logger logger;
int index = 0;

//客户端关闭时触发
void CloseCB(int clientid, void* userdata);
//新连接到来时触发
void NewConnect(int clientid, void* userdata);
//接收数据时候触发
void ReadCB(int cliendid, void *clientdata, const  char* buf,void *serverdata);
void ThreadFun(void *arg);
//获取对应的IV视频流
int GetIV(AcceptClient *client,C7_HSTREAM &hStreamIV, HANDLE &hVARender,char *windows_name );
//启动服务
bool TcpServerStart(const char *ip, int port);
CvCapture* pCapture;
int __stdcall DecodeVideoCallBack(UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp, void *pContext)
{
// 	const char *puid = (const char *)pContext;
// 	assert(puid);
// 	if (puid == NULL)
// 	{
// 		return 0;
// 	}
// 	IplImage *pFrame=cvQueryFrame( pCapture );  
// 	if (pFrame == NULL)
// 	{
// 		return 0;
// 	}
// 	Mat src(pFrame);
// 	vector<uchar> buff;//buffer for coding
// 	vector<int> param = vector<int>(2);
// 	param[0]=CV_IMWRITE_JPEG_QUALITY;
// 	param[1]=60;//default(95) 0-100
// 	imencode(".jpg",src,buff,param);
// 	multimap<string,int>puid_client_map = server.puid_client_map;
// 	uv_mutex_lock(&server.mutex_puid_client);
// 	int num = puid_client_map.count(puid);
// 	auto it = puid_client_map.find(puid);
// 	for (int i=0; i<num;i++)
// 	{
// 
// 		server.Send(it->second,buff);
// 		++it;
// 
// 	}
// 	cvReleaseImage(&pFrame);
// 	uv_mutex_unlock(&server.mutex_puid_client);

	

	const char *puid = (const char *)pContext;
	assert(puid);
	if (puid == NULL)
	{
		return 0;
	}
	auto baton =server.render_baton_map.find(puid);
	IplImage* img = convert(pImg[0],(render_baton_t*)baton->second);
	if (img == NULL)
	{
		return 0;
	}
	

	CvSize cz = cvSize(640,480);
	IplImage *NewImg = cvCreateImage(cz,img->depth,img->nChannels);
	cvResize(img, NewImg,CV_INTER_LINEAR );
	//cvSaveImage("E:\\test2.jpg",NewImg);
	//  			 			cvReleaseImage(&img);//释放图像内存
	//IplImage *image = YUV420_To_IplImage_Opencv(pImg[0], ImgWidth, ImgHeight);
	//if (image == NULL)
	//{
		//cvReleaseImage(&image);
	//	return 0;
	//}
	//cvSaveImage("E:\\test2.jpg",image);
//	CvSize cz = cvSize(640,480);
	//IplImage *NewImg = cvCreateImage(cz,image->depth,image->nChannels);
	
	//if (NewImg == NULL)
	//{

	//	//return 0;
	//}
	//IplImage* NewImg=cvLoadImage("D:\\1.jpg",-1);
	Mat src(NewImg);
	vector<uchar> buff;//buffer for coding
	vector<int> param = vector<int>(2);
	param[0]=CV_IMWRITE_JPEG_QUALITY;
	param[1]=60;//default(95) 0-100
	imencode(".jpg",src,buff,param);
	if (buff.size() ==0)
	{
		printf("dfdffffffffffffff***********************************\n");
		exit(0);
	}
	//cvReleaseImage(&img);
	cvReleaseImage(&NewImg);
	multimap<string,int>puid_client_map = server.puid_client_map;
	uv_mutex_lock(&server.mutex_puid_client);
	int num = puid_client_map.count(puid);
	auto it = puid_client_map.find(puid);
	for (int i=0; i<num;i++)
	{
		//server.Send(it->second,pImg,ImgWidth,ImgHeight,uiTimeStamp);
		//server.Send(it->second,img);
		server.Send(it->second,buff);
		++it;

	}
	uv_mutex_unlock(&server.mutex_puid_client);
	
	//cvReleaseImage(&img);
	//delete client;
	//client->Close();
	

	//注意AcceptClient 可能已经析构了
	
//	client->image_pool.push(&pImg[0],ImgWidth,ImgHeight,uiTimeStamp);
	//client->Send(&pImg[0],ImgWidth,ImgHeight,uiTimeStamp);
	return 0;
}

int GetIV(AcceptClient *client,C7_HSTREAM &hStreamIV, HANDLE &hVARender,char *windows_name )
{
	auto it =C7Platform::resourceMap.find(client->resource_puid);
	vector<ResourceInfo> selectInfo = it->second;
	C7_HRESOURCE ResourcesArr[MAX_NUM_RESOURCE] = { NULL};
	int nResCnt = MAX_NUM_RESOURCE;
	int rv;
	rv = C7_StartStream(selectInfo[client->resource_index].hresource,ST_REALTIME,&hStreamIV);
	if (rv != NRCAP_SUCCESS)
	{
		return rv;
	}
	auto puid_count  = server.puid_count_map.find(client->recv_msg);
	hVARender =VADR_Init(100);
	rv = VADR_SetVideoDecodeCallBack(hVARender, DecodeVideoCallBack, FRAMEFMT_YUV420,(void*)puid_count->first.c_str());
	if (rv != NRCAP_SUCCESS)
	{
		C7_StopStream(hStreamIV);
		return rv;
	}
	cvNamedWindow(windows_name, CV_WINDOW_AUTOSIZE);
	HWND hPlayHwnd = (HWND)cvGetWindowHandle(windows_name);
	cvResizeWindow(windows_name, 720, 576 );
	rv= VADR_SetPlayWnd(hVARender, hPlayHwnd);
	if (rv !=NRCAP_SUCCESS)
	{
		cvDestroyWindow(windows_name);
		return rv;

	}
	//开始解码显示
	rv = VADR_StartPreview(hVARender);
	if (rv !=NRCAP_SUCCESS)
	{
		cvDestroyWindow(windows_name);
		return rv;

	}
	return 0;
	
}
int SearchVideo(AcceptClient* client)
{
	auto it =C7Platform::resourceMap.find(client->resource_puid);
	if (it == C7Platform::resourceMap.end() || client->resource_index >= it->second.size()||(it->second.at(client->resource_index).szType != RT_IV) )
	{
		return 1;
	}
	auto puid_count  = server.puid_count_map.find(client->recv_msg);
	if (puid_count == server.puid_count_map.end())
	{
		return 1;
	}
	return 0;
}
void ThreadFun(void *arg)
{
	AcceptClient* client = (AcceptClient*)arg;
	C7_HSTREAM hStreamIV = NULL;	//视频流句柄
	HANDLE hVARender = NULL;		//媒体库句柄*/
	char windows_name[100]={0};
	int rv;
	sprintf(windows_name, "windows%d", ++index);
	rv = GetIV(client,hStreamIV,hVARender,windows_name);
	if (rv != SUCCESS)
	{
		LOG_ERROR("File:%s--Line:%d: GetIV error_code: %d",__FILE__,__LINE__,rv);
		static char msg[10]={0};
		_itoa(rv,msg,10);
		client->Send(msg,strlen(msg));
		client->Close();

	}
	string tmp = client->recv_msg;//收到的消息
	render_baton_t * baton = new render_baton_t();
	baton->f_clip_limit = client->f_clip_limit;
	alloc_cuda_memory(baton);
	server.render_baton_map.insert(make_pair(tmp,baton));
	unsigned char buffer[BUFFER_SIZE]={0};
	int nLen = BUFFER_SIZE;
	int nFrameType = 0;
	int nKeyFrameFlag = 0;
	auto puid_count  = server.puid_count_map.find(client->recv_msg);
	
	while (puid_count->second >= 1)
	{
 		memset(buffer, 0, BUFFER_SIZE);
 		nLen = BUFFER_SIZE;
 		nFrameType = 0;
 		nKeyFrameFlag = 0;
 		rv = C7_ReceiveFrame(hStreamIV,(char*)buffer,&nLen,&nFrameType,&nKeyFrameFlag);
		if (nFrameType == frame_type_video)
		{
			rv = VADR_PumpVideoFrame(hVARender, buffer, nLen);

		}
 		else if(rv == NRCAP_ERROR_DC_WOULDBLOCK)
 		{
			Sleep_i(10);
 			continue;
 		}
 		else
 		{
			LOG_ERROR("File:%s--Line:%d: GetIV error_code: %d",__FILE__,__LINE__,rv);
 			break;
 
 		}
 		
 		cvWaitKey(50);

	}
	LOG_INFO("threadFun Exit");
	cvDestroyWindow(windows_name);
	

	//一定要释放资源
	if (hStreamIV != NULL)
	{
		rv = C7_StopStream(hStreamIV);
		if (rv != NRCAP_SUCCESS)
		{
			//LOG4CPLUS_ERROR(logger," Error: C7_StopStream ERROR_CODE:"<<rv);
			//return;
		}
	}
	// 停止解码显示
	rv =VADR_StopPreview(hVARender);
	if (rv != NRCAP_SUCCESS)
	{
		//LOG4CPLUS_ERROR(logger," Error: VADR_StopPreview ERROR_CODE:"<<rv);
		//return;
	}
	VADR_Close(hVARender);
	uv_mutex_lock(&server.mutex_puid_client);
	int num = server.puid_client_map.count(puid_count->first);
	auto it2 = server.puid_client_map.find(puid_count->first);
	for (int i=0; i<num;i++)
	{
		server.CloseClient(it2->second);
		++it2;
	}
	auto baton_t = server.render_baton_map.find(tmp);
	if (baton_t != server.render_baton_map.end())
	{
		free_cuda_memory((render_baton_t*)baton_t->second);
		delete baton_t->second;
		server.render_baton_map.erase(baton_t);
	}
	uv_mutex_unlock(&server.mutex_puid_client);
}
//客户端发送类似的字符串："151038402365732956,1"代表哪个ID第几个索引视频
void ReadCB(int cliendid, void *clientdata, const  char* buf,void *serverdata)
{
	
	TCPServer *server = (TCPServer*)serverdata;
	assert(server);
	AcceptClient *client = (AcceptClient*)clientdata;
	string tmp = buf;
	
	char *id = strtok((char*)buf,",");
	char *index = strtok(NULL,",");
	char *cliplimit = strtok(NULL,",");
	uv_mutex_lock(&server->mutex_puid_count);
	auto it = server->puid_count_map.find(tmp);
	if (it != server->puid_count_map.end())
	{
		++it->second;
	}
	else
	{
		server->puid_count_map.insert(make_pair(tmp,1));
	}
	uv_mutex_unlock(&server->mutex_puid_count);

	uv_mutex_lock(&server->mutex_puid_client);
	int num = server->puid_client_map.count(tmp);
	if (num == 0) //无该puid视频则创建线程获取该视频
	{
		client->recv_msg = tmp;
		client->resource_puid = id;
		client->resource_index = (index ==NULL ? 0:atoi((const char*)index));
		client->f_clip_limit = (cliplimit == NULL? 3.5:atof(cliplimit));
		server->puid_thread_map.insert(make_pair(tmp,client->threadId));
		server->puid_client_map.insert(make_pair(tmp,cliendid));

		uv_mutex_unlock(&server->mutex_puid_client);
		int rv = SearchVideo(client);
		if (rv == 0)//找到视频
		{
			uv_thread_create(&client->threadId,ThreadFun,client);
		}
		else
		{
			printf("not find video\n");
			static char msg[10]={0};
			_itoa(2,msg,10);
			client->Send(msg,strlen(msg));
			client->Close();
		}
		
	}
	else
	{
		//thread执行函数的时间不确定，需要在执行前释放锁
		server->puid_client_map.insert(make_pair(tmp,cliendid));
		uv_mutex_unlock(&server->mutex_puid_client);
	}
}
void NewConnect(int clientid, void* userdata)
{
	TCPServer * server = (TCPServer*)userdata;
	LOG_INFO("new connect:%d\n",clientid);
	server->SetRecvCB(clientid,ReadCB,server);
	
}
void CloseCB(int clientid, void* userdata)
{
	LOG_INFO("client %d close",clientid);
	//TcpServer *theclass = (TcpServer *)userdata;
}
bool TcpServerStart(const char *ip, int port)
{
	server.SetNewConnectCB(NewConnect,&server);
	server.SetClosedCB(CloseCB,&server);
	return server.Start(ip,port);

}

int main()
{
	
	MiniDump::InitMinDump();
	string current_path;
	string log_path;//log4cplus 配置文件路径
	string ini_file_path;//INI配置文件
	current_path = get_application_path(current_path);
	log_path = current_path + LOG4CPLUS_CONFIG_FILE;
	ini_file_path = current_path + RUNTIME_CONFIG_FILE;
	LogFile::SetLogFilePath(log_path);
	LOG_INFO( "*********************************************************");
	LOG_INFO("VideoService initialize...");
	char server_ip[100]={0};
	int server_port=0;
	config_read_string(ini_file_path.c_str(),"General","ServerIp",server_ip,"");
	config_read_integer(ini_file_path.c_str(),"General","ServerPort",&server_port,0);
	if(!TcpServerStart(server_ip,server_port))
	{
		LOG_ERROR("TcpServerStart Fail");
		return -1;
	}
	C7Platform c7Platform;
	C7Platform::Init(current_path);
	LOG_INFO( "*********************************************************");
	LOG_INFO("TCPService initialize success!");
	LOG_INFO("TCPService address ---> %s:%d",server_ip,server_port);

	while(true) 
	{
		Sleep_i(10000);
	}
	server.Close();
	C7Platform::UnInit();
	return 0;
}