#ifndef TCP_CLIENT_H_
#define TCP_CLIENT_H_
#include <libuv/uv.h>
#include "TCPCommon.h"
#include "platform_config.h"
namespace uv
{
	class TCPClient
	{
	public:
		TCPClient(void);
		~TCPClient(void);
		bool Connect(const char* ip, int port);//���ӷ����
		void SetRecvCB(ClientRecvCB pfun, void *userdata);//���ý������ݻص�����
		void SetClosedCB(TcpCloseCB,void *userdata);//���ùرյĻص�����
		void Close(){isuseraskforclosed = true;}
		bool IsClose(){return isuseraskforclosed;}
		int  Send(const char* data, std::size_t len);
		//�Ƿ�����Nagle�㷨
		bool SetNoDelay(bool enable);
	private:
		bool init();
		void closeinl();
		bool run(int status = UV_RUN_DEFAULT);
		static void ConnectThread(void* arg);//������connect�߳�
	private:
		enum {
			CONNECT_TIMEOUT,
			CONNECT_FINISH,
			CONNECT_ERROR,
			CONNECT_DIS,
		};
		uv_loop_t loop;//�¼�
		uv_prepare_t prepare_handle;
		uv_idle_t idle_handle;
		uv_check_t check_handle;
		uv_connect_t connect_req;//��������
		uv_tcp_t client_handle;//�ͻ�������
		uv_buf_t read_buf;//��buf
		uv_buf_t write_buf;//дbuf

		uv_mutex_t mutex_write_buf;//����write_buf
		uv_mutex_t mutex_writereq;//����writereq_list
		list<uv_write_t*>writereq_list;
		uv_thread_t connect_thread_handle;
		ClientRecvCB recvcb;//�������ݻص����û��ĺ���
		void *recvcb_userdata;
		TcpCloseCB closecb;
		void *closecb_userdata;
		bool isclosed;//�Ƿ�ر�
		bool isuseraskforclosed;//�û��Ƿ��͹ر�
		int connect_status;//����״̬
		string errmsg;
		unsigned char recv_buffer[BUFFER_SIZE];//�յ���buffer
	private:
		static void AfterConnect(uv_connect_t* req, int status);
		static void AfterRecv(uv_stream_t* stream,ssize_t nread,const uv_buf_t* buf);
		static void AllocBufferForRecv(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
		static void PrepareCB(uv_prepare_t* handle);
		static void CheckCB(uv_check_t* handle);
		static void IdleCB(uv_idle_t* handle);
		static void AfterClientClose(uv_handle_t *handle);
		static void AfterSend( uv_write_t *req, int status);
	};
}
#endif