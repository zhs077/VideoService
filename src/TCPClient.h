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
		bool Connect(const char* ip, int port);//连接服务端
		void SetRecvCB(ClientRecvCB pfun, void *userdata);//设置接收数据回调函数
		void SetClosedCB(TcpCloseCB,void *userdata);//设置关闭的回调函数
		void Close(){isuseraskforclosed = true;}
		bool IsClose(){return isuseraskforclosed;}
		int  Send(const char* data, std::size_t len);
		//是否启用Nagle算法
		bool SetNoDelay(bool enable);
	private:
		bool init();
		void closeinl();
		bool run(int status = UV_RUN_DEFAULT);
		static void ConnectThread(void* arg);//真正的connect线程
	private:
		enum {
			CONNECT_TIMEOUT,
			CONNECT_FINISH,
			CONNECT_ERROR,
			CONNECT_DIS,
		};
		uv_loop_t loop;//事件
		uv_prepare_t prepare_handle;
		uv_idle_t idle_handle;
		uv_check_t check_handle;
		uv_connect_t connect_req;//连接请求
		uv_tcp_t client_handle;//客户端连接
		uv_buf_t read_buf;//读buf
		uv_buf_t write_buf;//写buf

		uv_mutex_t mutex_write_buf;//控制write_buf
		uv_mutex_t mutex_writereq;//控制writereq_list
		list<uv_write_t*>writereq_list;
		uv_thread_t connect_thread_handle;
		ClientRecvCB recvcb;//接收数据回调给用户的函数
		void *recvcb_userdata;
		TcpCloseCB closecb;
		void *closecb_userdata;
		bool isclosed;//是否关闭
		bool isuseraskforclosed;//用户是否发送关闭
		int connect_status;//连接状态
		string errmsg;
		unsigned char recv_buffer[BUFFER_SIZE];//收到的buffer
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