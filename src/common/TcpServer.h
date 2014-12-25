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
	void SetRecvCB(ServerRecvCB pfun, void *userdata);//���ý������ݻص�����
	void SetClosedCB(TcpCloseCB,void *userdata);//���ùرյĻص�����
	int  Send(const char* data, std::size_t len);//�����ַ���
	int  Send(IplImage *image);//����
	int  Send(const vector<uchar> & msg);//����Ϣ�������
	int  Send(UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp);//����
	inline void Close(){isuseraskforclosed = true;}
	inline bool IsClose(){return isclosed;}
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
	ServerRecvCB recvcb;//�������ݻص����û��ĺ���
	void *recvcb_userdata;
	TcpCloseCB closecb;
	void *closecb_userdata;
	bool isuseraskforclosed;//�û��Ƿ�������ر�
	string errmsg;
	char read_buffer[100];
	queue<IplImage*>iplImage_pool;
	queue<vector<uchar> >send_msg_pool;

public:
	ImagePool image_pool;//ͼƬ��
	string recv_msg; //resource_puid +  resource_index
	string resource_puid;//Ҫ��ȡ����ԴPUID
	int resource_index;//��Դ����
	uv_thread_t threadId;//������Ƶ���߳�ID
	float f_clip_limit;//����ǿ��
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
	//������־�ƶ���־�����ļ�����·��
	bool StartLog(const string& filePath);
	const char* GetLastErrMsg() const 
	{
		return errmsg.c_str();
	};
	int  Send(int clientid, const char* data, std::size_t len);
	int  Send(int clientid,UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp);
	int  Send(int clientid,IplImage *image);
	int  Send(int clientid,const vector<uchar>&msg);
	//�Ƿ�����Nagle�㷨
	bool SetNoDelay(bool enable);
	void SetNewConnectCB(NewConnectCB,void *userdata);
	void SetClosedCB(TcpCloseCB,void *userdata );
	void SetRecvCB(int clientid,ServerRecvCB cb, void *userdata);//���ý��ջص�������ÿ���ͻ��˸���һ��
	inline void Close(){isuseraskforclosed = true;}//�û��رշ���� 
	inline bool IsClose(){return isclosed;}
	void CloseClient(int clientid);

private:
	bool init();
	bool bind(const char* ip, int port);
	bool listen(int backlog);
	bool run(int status = UV_RUN_DEFAULT);
	void closeinl();//�ڲ�������������
private:
	static void PrepareCB(uv_prepare_t* handle);
	static void CheckCB(uv_check_t* handle);
	static void IdleCB(uv_idle_t *handle);
	static void AcceptConnection(uv_stream_t *server, int status);
	static void StartThread(void* arg);
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
	int startstatus;//�����״̬
	map<int,AcceptClient*> clients_list;//�ӿͻ�������
	

private:
	NewConnectCB newconcb;//�ص�����
	void *newconcb_userdata;
	ServerRecvCB recvcb;//�������ݻص����û��ĺ���
	TcpCloseCB closedcb;//�رպ�ص���TCPServer
	void *closedcb_userdata;

public:
	multimap<string,int> puid_client_map;//���ŵ���Ƶ��Ӧ�Ŀͻ���ID������
	uv_mutex_t mutex_puid_client;
	map<string,atomic<int> > puid_count_map;//������Ƶ��Ӧ�Ĵ���
	map<string,uv_thread_t> puid_thread_map;
	uv_mutex_t mutex_puid_count;
	map<string,void*>render_baton_map;//cudua�õ����ڴ�



};
}
#endif