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
	void SetRecvCB(ServerRecvCB pfun, void *userdata);//���ý������ݻص�����
	void SetClosedCB(TcpCloseCB,void *userdata);//���ùرյĻص�����
	int Send(const char* data, std::size_t len);
	void Close(){isuseraskforclosed = true;}
	const char* GetLastErrMsg() const 
	{
		return errmsg.c_str();
	};
private:

	uv_loop_t* loop;//server������loop
	int client_id;
	uv_tcp_t client_handle;
	uv_prepare_t prepare_handle;
	bool isclosed;
	uv_buf_t read_buf;
	uv_buf_t write_buf;
	uv_mutex_t mutex_write_buf;
	std::list<uv_write_t*> writereq_list;//���õ�uv_write_t
	uv_mutex_t mutex_writereq;//����writereq_list_
	Packet readpacket_;//����buf���ݽ����ɰ�
	ServerRecvCB recvcb;//�������ݻص����û��ĺ���
	void *recvcb_userdata;
	TcpCloseCB closecb;
	void *closecb_userdata;
	bool isuseraskforclosed;//�û��Ƿ�������ر�
	string errmsg;

	
public:
	bool	 isplay;//�ӷ�������ȡ��Ƶ��
	ImagePool image_pool;//ͼƬ��
	string resource_puid;//Ҫ��ȡ����ԴPUID
	int resource_index;//��Դ����
	uv_thread_t threadId;//������Ƶ���߳�ID



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
	void SetRecvCB(int clientid,ServerRecvCB cb, void *userdata);//���ý��ջص�������ÿ���ͻ��˸���һ��
	void Close(){isuseraskforclosed = true;}//�û��رշ���� 
	bool IsClose(){return isuseraskforclosed;}
private:
	bool init();
	bool bind(const char* ip, int port);
	bool listen(int backlog);
	bool run(int status = UV_RUN_DEFAULT);
	void closeinl();//�ڲ�������������
private:
	static void PrepareCB( uv_prepare_t* handle );
	static void CheckCB( uv_check_t* handle );
	static void IdleCB(uv_idle_t *handle);
	static void AcceptConnection(uv_stream_t *server, int status);
	static void StartThread( void* arg );
	static void AfterServerClose(uv_handle_t *handle);
	static void ClientClosed(int clientid,void *userdata);//AcceptClient�رպ�ص���TCPServer
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
	int client_id;//�ͻ���id,Ωһ,��TCPServer����ֵ
	uv_tcp_t client_handle;//�ͻ��˾��
	uv_prepare_t prepare_handle;//prepare�׶�handle,���ڼ��ر����Ƿ���Ҫ��������
	uv_check_t check_handle;
	uv_idle_t idle_handle;
	uv_tcp_t server_handle;
	uv_thread_t start_threadhandle;//�����߳�
	bool isclosed;//�Ƿ��ѹر�
	bool isuseraskforclosed;//�û��Ƿ�������ر�
	string serverip;//���ӵ�IP
	int serverport;//���ӵĶ˿ں�
	int startstatus;//����״̬
	map<int,AcceptClient*> clients_list;//�ӿͻ�������
	Packet readpacket_;//����buf���ݽ����ɰ�
	NewConnectCB newconcb;//�ص�����
	void *newconcb_userdata;
private:
	ServerRecvCB recvcb;//�������ݻص����û��ĺ���
	TcpCloseCB closedcb;//�رպ�ص���TCPServer
	void *closedcb_userdata;


};

