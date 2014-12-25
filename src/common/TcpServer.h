#ifndef TCP_SERVER_H_
#define TCP_SERVER_H_

#include <libuv/uv.h>
#include "TCPCommon.h"
#include "platform_config.h"
#include "ScreenEngine.h"
#include "LogFile.h"

namespace uv
{


class AcceptClient
{
public:
	AcceptClient(int clientId,uv_loop_t*loop);
	virtual ~AcceptClient();
	bool AcceptByServer(uv_tcp_t *server);
	void SetRecvCB(ServerRecvCB pfun, void *userdata);//设置接收数据回调函数
	void SetClosedCB(TcpCloseCB,void *userdata);//设置关闭的回调函数
	int  Send(const char* data, std::size_t len);//发送字符串
	int  Send(IplImage *image);//废弃
	int  Send(const vector<uchar> & msg);//将消息放入队列
	int  Send(UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp);//废弃
	inline void Close(){isuseraskforclosed = true;}
	inline bool IsClose(){return isclosed;}
	const char* GetLastErrMsg() const 
	{
		return errmsg.c_str();
	};
private:

	uv_loop_t* loop;//server所带的loop
	int client_id;
	uv_tcp_t client_handle;
	uv_prepare_t prepare_handle;
	bool isclosed;
	uv_buf_t read_buf;
	uv_buf_t write_buf;
	uv_mutex_t mutex_write_buf;
	std::list<uv_write_t*> writereq_list;//可用的uv_write_t
	uv_mutex_t mutex_writereq;//控制writereq_list_
	ServerRecvCB recvcb;//接收数据回调给用户的函数
	void *recvcb_userdata;
	TcpCloseCB closecb;
	void *closecb_userdata;
	bool isuseraskforclosed;//用户是否发送命令关闭
	string errmsg;
	char read_buffer[100];
	queue<IplImage*>iplImage_pool;
	queue<vector<uchar> >send_msg_pool;

public:
	ImagePool image_pool;//图片池
	string recv_msg; //resource_puid +  resource_index
	string resource_puid;//要获取的资源PUID
	int resource_index;//资源索引
	uv_thread_t threadId;//接收视频的线程ID
	float f_clip_limit;//光亮强度
private:
	bool init();
	void closeinl();
private:
	static void AllocBufferForRecv(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
	static void AfterRecv(uv_stream_t *client, ssize_t nread, const uv_buf_t* buf);
	static void PrepareCB( uv_prepare_t* handle );
	static void AfterSend( uv_write_t *req, int status);
	static void AfterClientClose( uv_handle_t *handle );


};


class TCPServer
{
public:
	TCPServer(void);
	~TCPServer(void);

	bool Start(const char *ip, int port);
	//开启日志制定日志配置文件所在路径
	bool StartLog(const string& filePath);
	const char* GetLastErrMsg() const 
	{
		return errmsg.c_str();
	};
	int  Send(int clientid, const char* data, std::size_t len);
	int  Send(int clientid,UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp);
	int  Send(int clientid,IplImage *image);
	int  Send(int clientid,const vector<uchar>&msg);
	//是否启用Nagle算法
	bool SetNoDelay(bool enable);
	void SetNewConnectCB(NewConnectCB,void *userdata);
	void SetClosedCB(TcpCloseCB,void *userdata );
	void SetRecvCB(int clientid,ServerRecvCB cb, void *userdata);//设置接收回调函数，每个客户端各有一个
	inline void Close(){isuseraskforclosed = true;}//用户关闭服务端 
	inline bool IsClose(){return isclosed;}
	void CloseClient(int clientid);

private:
	bool init();
	bool bind(const char* ip, int port);
	bool listen(int backlog);
	bool run(int status = UV_RUN_DEFAULT);
	void closeinl();//内部真正的清理函数
private:
	static void PrepareCB(uv_prepare_t* handle);
	static void CheckCB(uv_check_t* handle);
	static void IdleCB(uv_idle_t *handle);
	static void AcceptConnection(uv_stream_t *server, int status);
	static void StartThread(void* arg);
	static void AfterServerClose(uv_handle_t *handle);
	static void ClientClosed(int clientid,void *userdata);//AcceptClient关闭后回调给TCPServer
	int GetAvailaClientID()const;


private:
	enum 
	{
		START_TIMEOUT,
		START_FINISH,
		START_ERROR,
		START_DIS
	};
	uv_loop_t loop;
	string errmsg;
	uv_mutex_t mutex_clients;
	int client_id;//客户端id,惟一,由TCPServer负责赋值
	uv_tcp_t client_handle;//客户端句柄
	uv_prepare_t prepare_handle;//prepare阶段handle,用于检测关闭与是否需要发送数据
	uv_check_t check_handle;
	uv_idle_t idle_handle;
	uv_tcp_t server_handle;
	uv_thread_t start_threadhandle;//启动线程
	bool isclosed;//是否已关闭
	bool isuseraskforclosed;//用户是否发送命令关闭
	string serverip;//连接的IP
	int serverport;//连接的端口号
	int startstatus;//服务端状态
	map<int,AcceptClient*> clients_list;//子客户端链接
	

private:
	NewConnectCB newconcb;//回调函数
	void *newconcb_userdata;
	ServerRecvCB recvcb;//接收数据回调给用户的函数
	TcpCloseCB closedcb;//关闭后回调给TCPServer
	void *closedcb_userdata;

public:
	multimap<string,int> puid_client_map;//播放的视频对应的客户端ID、索引
	uv_mutex_t mutex_puid_client;
	map<string,atomic<int> > puid_count_map;//播放视频对应的次数
	map<string,uv_thread_t> puid_thread_map;
	uv_mutex_t mutex_puid_count;
	map<string,void*>render_baton_map;//cudua用到的内存



};
}
#endif