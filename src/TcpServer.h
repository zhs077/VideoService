#pragma once
#include <libuv/uv.h>
#include <string>
#include <cassert>
#include <iostream>
#include <map>
#include <list>
#include <atomic>
#include "packet.h"
#include "ScreenEngine.h"
using namespace std;
#ifndef BUFFER_SIZE
#define BUFFER_SIZE (1024*1024)
#endif
class AcceptClient
{
public:
	AcceptClient(int clientId,uv_loop_t*loop);
	virtual ~AcceptClient();
	bool AcceptByServer( uv_tcp_t *server );
	void SetRecvCB(ServerRecvCB pfun, void *userdata);//设置接收数据回调函数
	void SetClosedCB(TcpCloseCB,void *userdata);//设置关闭的回调函数
	int Send(const char* data, std::size_t len);
	void Close(){isuseraskforclosed = true;}
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
	Packet readpacket_;//接受buf数据解析成包
	ServerRecvCB recvcb;//接收数据回调给用户的函数
	void *recvcb_userdata;
	TcpCloseCB closecb;
	void *closecb_userdata;
	bool isuseraskforclosed;//用户是否发送命令关闭
	string errmsg;

	
public:
	bool	 isplay;//从服务器获取视频流
	ImagePool image_pool;//图片池
	string resource_puid;//要获取的资源PUID
	int resource_index;//资源索引
	uv_thread_t threadId;//接收视频的线程ID



private:
	bool init();
	void closeinl();
private:
	static void AllocBufferForRecv(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
	static void AfterRecv(uv_stream_t *client, ssize_t nread, const uv_buf_t* buf);
	static void GetPacket( const NetPacket& packethead, const  char* packetdata, void* userdata );
	static void PrepareCB( uv_prepare_t* handle );
	static void AfterSend( uv_write_t *req, int status);
	static void AfterClientClose( uv_handle_t *handle );

	
};
class TcpServer
{
public:
	TcpServer(void);
	~TcpServer(void);

	bool Start(const char *ip, int port);
	const char* GetLastErrMsg() const 
	{
		return errmsg.c_str();
	};
	int  Send(int clientid, const char* data, std::size_t len);
	void SetNewConnectCB(NewConnectCB,void *userdata);
	void SetClosedCB(TcpCloseCB,void *userdata );
	void SetRecvCB(int clientid,ServerRecvCB cb, void *userdata);//设置接收回调函数，每个客户端各有一个
	void Close(){isuseraskforclosed = true;}//用户关闭服务端 
	bool IsClose(){return isuseraskforclosed;}
private:
	bool init();
	bool bind(const char* ip, int port);
	bool listen(int backlog);
	bool run(int status = UV_RUN_DEFAULT);
	void closeinl();//内部真正的清理函数
private:
	static void PrepareCB( uv_prepare_t* handle );
	static void CheckCB( uv_check_t* handle );
	static void IdleCB(uv_idle_t *handle);
	static void AcceptConnection(uv_stream_t *server, int status);
	static void StartThread( void* arg );
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
	int startstatus;//连接状态
	map<int,AcceptClient*> clients_list;//子客户端链接
	Packet readpacket_;//接受buf数据解析成包
	NewConnectCB newconcb;//回调函数
	void *newconcb_userdata;
private:
	ServerRecvCB recvcb;//接收数据回调给用户的函数
	TcpCloseCB closedcb;//关闭后回调给TCPServer
	void *closedcb_userdata;


};

