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
//                  ���汣��             ����BUG  
//          ��Ի:   
//                  д��¥��д�ּ䣬д�ּ������Ա��   
//                  ������Աд�������ó��򻻾�Ǯ��   
//                  ����ֻ���������������������ߣ�   
//                  ��������ո��գ����������긴�ꡣ   
//                  ��Ը�������Լ䣬��Ը�Ϲ��ϰ�ǰ��   
//                  ���۱������Ȥ���������г���Ա��   
//                  ����Ц��߯��񲣬��Ц�Լ���̫����   
//                  ��������Ư���ã��ĸ���ó���Ա��  

#include "TcpServer.h"
#include "platform_config.h"
#include "C7Platform.h"
#include "minidump.h"
#include "ImageProcess.h"
#include "ErrorCode.h"
#include "configlib.h"
using namespace uv;
#define MAX_NUM_RESOURCE 100
//#define MAX_FRAME_LEN	1024*1024 //���֡��С
TCPServer server;
Logger logger;
int index = 0;

//�ͻ��˹ر�ʱ����
void CloseCB(int clientid, void* userdata);
//�����ӵ���ʱ����
void NewConnect(int clientid, void* userdata);
//��������ʱ�򴥷�
void ReadCB(int cliendid, void *clientdata, const  char* buf,void *serverdata);
void ThreadFun(void *arg);
//��ȡ��Ӧ��IV��Ƶ��
int GetIV(AcceptClient *client,C7_HSTREAM &hStreamIV, HANDLE &hVARender,char *windows_name );
//��������
bool TcpServerStart(const char *ip, int port);

int __stdcall DecodeVideoCallBack(UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp, void *pContext)
{

	const char *puid = (const char *)pContext;
	assert(puid);
	if (puid == NULL)
	{
		return 0;
	}
	IplImage *image = YUV420_To_IplImage_Opencv(pImg[0], ImgWidth, ImgHeight);
	if (image == NULL)
	{
		cvReleaseImage(&image);
		return 0;
	}
	CvSize cz = cvSize(640,480);
	IplImage *NewImg = cvCreateImage(cz,image->depth,image->nChannels);
	if (NewImg == NULL)
	{

		return 0;
	}
	Mat src(NewImg);
	vector<uchar> buff;//buffer for coding
	vector<int> param = vector<int>(2);
	param[0]=CV_IMWRITE_JPEG_QUALITY;
	param[1]=60;//default(95) 0-100
	imencode(".jpg",src,buff,param);
	cvReleaseImage(&image);
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
	

	//ע��AcceptClient �����Ѿ�������
	
//	client->image_pool.push(&pImg[0],ImgWidth,ImgHeight,uiTimeStamp);
	//client->Send(&pImg[0],ImgWidth,ImgHeight,uiTimeStamp);
	return 0;
}

int GetIV(AcceptClient *client,C7_HSTREAM &hStreamIV, HANDLE &hVARender,char *windows_name )
{
	
	auto it =C7Platform::resourceMap.find(client->resource_puid);
	if (it == C7Platform::resourceMap.end() || (it->second.at(client->resource_index).szType != RT_IV) )
	{

		return 1;

	}
	auto puid_count  = server.puid_count_map.find(client->recv_msg);
	if (puid_count == server.puid_count_map.end())
	{
		return 1;
	}
	vector<ResourceInfo> selectInfo = it->second;
	C7_HRESOURCE ResourcesArr[MAX_NUM_RESOURCE] = { NULL};
	int nResCnt = MAX_NUM_RESOURCE;
	int rv;
	rv = C7_StartStream(selectInfo[client->resource_index].hresource,ST_REALTIME,&hStreamIV);
	if (rv != NRCAP_SUCCESS)
	{
		return rv;
		//client
		//LOG4CPLUS_ERROR(logger," Error: C7_StartStream ERROR_CODE:"<<rv);


	}
	hVARender =VADR_Init(100);
	rv = VADR_SetVideoDecodeCallBack(hVARender, DecodeVideoCallBack, FRAMEFMT_YUV420,(void*)puid_count->first.c_str());
	if (rv != NRCAP_SUCCESS)
	{

		return rv;
		//LOG4CPLUS_ERROR(logger," Error: VADR_SetVideoDecodeCallBack ERROR_CODE:"<<rv);

	}
	
	cvNamedWindow(windows_name, CV_WINDOW_AUTOSIZE);
	HWND hPlayHwnd = (HWND)cvGetWindowHandle(windows_name);
	cvResizeWindow(windows_name, 720, 576 );
	rv= VADR_SetPlayWnd(hVARender, hPlayHwnd);
	if (rv !=NRCAP_SUCCESS)
	{
		cvDestroyWindow(windows_name);
		return rv;
		//LOG4CPLUS_ERROR(logger," Error: VADR_SetPlayWnd ERROR_CODE:"<<rv);

	}
	//��ʼ������ʾ
	rv = VADR_StartPreview(hVARender);
	if (rv !=NRCAP_SUCCESS)
	{
		cvDestroyWindow(windows_name);
		return rv;
		//LOG4CPLUS_ERROR(logger," Error: VADR_StartPreview ERROR_CODE:"<<rv);

	}
	return 0;
	
}
void ThreadFun(void *arg)
{
	AcceptClient* client = (AcceptClient*)arg;
	C7_HSTREAM hStreamIV = NULL;	//��Ƶ�����
	HANDLE hVARender = NULL;		//ý�����*/
	char windows_name[100]={0};
	int rv;
	sprintf(windows_name, "windows%d", ++index);
	rv = GetIV(client,hStreamIV,hVARender,windows_name);
	if (rv != SUCCESS)
	{
		LOG_ERROR("File:%s--Line:%d: GetIV error_code: %d",__FILE__,__LINE__,rv);
		char msg[10]={0};
		client->Send(_itoa(rv,msg,10),strlen(msg));
		client->Close();
		return;
	}

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
			
 		//	LOG4CPLUS_ERROR(logger," Error: C7_ReceiveFrame ERROR_CODE:"<<rv);
 			break;
 
 		}
 		
 		cvWaitKey(50);

	}
	LOG_INFO("threadFun Exit");
	cvDestroyWindow(windows_name);
	//һ��Ҫ�ͷ���Դ
	if (hStreamIV != NULL)
	{
		rv = C7_StopStream(hStreamIV);
		if (rv != NRCAP_SUCCESS)
		{
			//LOG4CPLUS_ERROR(logger," Error: C7_StopStream ERROR_CODE:"<<rv);
			//return;
		}
	}
	// ֹͣ������ʾ
	rv =VADR_StopPreview(hVARender);
	if (rv != NRCAP_SUCCESS)
	{
		//LOG4CPLUS_ERROR(logger," Error: VADR_StopPreview ERROR_CODE:"<<rv);
		//return;
	}
	VADR_Close(hVARender);
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
//�ͻ��˷������Ƶ��ַ�����"12324,1"�����ĸ�ID�ڼ���������Ƶ
void ReadCB(int cliendid, void *clientdata, const  char* buf,void *serverdata)
{
	
	TCPServer *server = (TCPServer*)serverdata;
	assert(server);
	AcceptClient *client = (AcceptClient *)clientdata;
	string tmp = buf;
	char *id = strtok((char*)buf,",");
	char *index = strtok(NULL,",");
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
	if (num == 0) //�޸�puid��Ƶ�򴴽��̻߳�ȡ����Ƶ
	{
		client->recv_msg = tmp;
		client->resource_puid = id;
		client->resource_index = (index ==NULL ? 0:atoi((const char*)index));
		server->puid_thread_map.insert(make_pair(tmp,client->threadId));
		server->puid_client_map.insert(make_pair(tmp,cliendid));
		uv_mutex_unlock(&server->mutex_puid_client);
		uv_thread_create(&client->threadId,ThreadFun,client);

		
	}
	else
	{
		//threadִ�к�����ʱ�䲻ȷ������Ҫ��ִ��ǰ�ͷ���
		server->puid_client_map.insert(make_pair(tmp,cliendid));
		uv_mutex_unlock(&server->mutex_puid_client);
	}
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
	string log_path;//log4cplus �����ļ�·��
	string ini_file_path;//INI�����ļ�
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