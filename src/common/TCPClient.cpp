#include "TCPClient.h"
using namespace uv;
#include <cassert>
#include <cstdio>
using namespace std;
TCPClient::TCPClient(void)
{
	connect_status = CONNECT_DIS;
	recvcb = NULL;
	closecb = NULL;
	isclosed = true;
	isuseraskforclosed = false;
	close_type = 0;
	int r;
	r = uv_loop_init(&loop);
	assert(r ==0);
	connect_req.data = this;
	r = uv_mutex_init(&mutex_writereq);
	assert(r == 0);
	r = uv_mutex_init(&mutex_write_buf);
	assert(r == 0);
	read_buf = uv_buf_init((char*)malloc(BUFFER_SIZE),BUFFER_SIZE);
	write_buf = uv_buf_init((char*)malloc(100),100);
	memset(recv_buffer,0,sizeof(recv_buffer));
}


TCPClient::~TCPClient(void)
{
	Close();
	uv_thread_join(&connect_thread_handle);//libuv事件循环已退出
	if (read_buf.base) 
	{
		free(read_buf.base);
		read_buf.base = NULL;
		read_buf.len = 0;
	}
	if (write_buf.base) 
	{
		free(write_buf.base);
		write_buf.base = NULL;
		write_buf.len = 0;
	}
	 uv_loop_close(&loop);
	 uv_mutex_destroy(&mutex_write_buf);
	 uv_mutex_destroy(&mutex_writereq);
	 for (auto it = writereq_list.begin(); it != writereq_list.end(); ++it)
	 {
		 free(*it);
	 }
	writereq_list.clear();


}

bool TCPClient::init()
{
	int r;
	r = uv_prepare_init(&loop,&prepare_handle);
	if (r)
	{
		return false;
	}
	prepare_handle.data = this;
	r = uv_prepare_start(&prepare_handle,PrepareCB);
	if (r)
	{
		return false;
	}
	r = uv_check_init(&loop,&check_handle);
	if (r)
	{
		return false;
	}
	check_handle.data = this;
	r = uv_check_start(&check_handle,CheckCB);
	if (r)
	{
		return false;
	}
	r = uv_idle_init(&loop,&idle_handle);
	if (r)
	{
		return false;
	}
	idle_handle.data = this;
	r = uv_idle_start(&idle_handle,IdleCB);
	if (r)
	{
		return false;
	}
	r = uv_tcp_init(&loop,&client_handle);
	if (r)
	{
		return false;
	}
	client_handle.data = this;
	isclosed = false;
	return true;
}

void TCPClient::AfterClientClose(uv_handle_t *handle)
{
	TCPClient *theclass = (TCPClient*) handle->data;
	assert(theclass);
	if (handle == (uv_handle_t *)&theclass->prepare_handle)
	{
		handle->data = 0;
	}
	if (handle == (uv_handle_t *)&theclass->client_handle)
	{
		handle->data = 0;
	}
	if (handle == (uv_handle_t *)&theclass->check_handle)
	{
		handle->data = 0;
	}
	if (handle == (uv_handle_t *)&theclass->idle_handle)
	{
		handle->data = 0;
	}
	if (theclass->prepare_handle.data == 0 
		&& theclass->client_handle.data == 0 
		&& theclass->check_handle.data == 0 
		&& theclass->idle_handle.data == 0)
	{
		theclass->isclosed = true;
		theclass->isuseraskforclosed = false;
		if (theclass->closecb) 
		{
			//通知此客户端已经关闭
			theclass->closecb(theclass->close_type,theclass->closecb_userdata);
		}

	}


}
bool TCPClient::run(int status)
{
	int r = uv_run(&loop,(uv_run_mode)status);
	if (r)
	{
		errmsg = GetUVError(r);
		fprintf(stdout,"uv_run error:%s",errmsg.c_str());
		return false;
	}
	return true;
}
void TCPClient::closeinl()
{
	if (isclosed)
	{
		return;
	}
	uv_close((uv_handle_t*)&client_handle,AfterClientClose);
	uv_close((uv_handle_t*)&prepare_handle,AfterClientClose);
	uv_close((uv_handle_t*)&idle_handle,AfterClientClose);
	uv_close((uv_handle_t*)&check_handle,AfterClientClose);
}
void TCPClient::ConnectThread(void *arg)
{
	TCPClient* theclass = (TCPClient*)arg;
	theclass->run();
}
void TCPClient::SetRecvCB(ClientRecvCB pfun, void* userdata)
{
	recvcb = pfun;
	recvcb_userdata = userdata;

}
void TCPClient::SetClosedCB(TcpCloseCB cb,void *userdata)
{
	closecb = cb;
	closecb_userdata = userdata;

}
bool TCPClient::Connect(const char* ip, int port)
{
	closeinl();
	init();
	struct sockaddr_in bind_addr;
	int r =uv_ip4_addr(ip,port,&bind_addr);
	if (r)
	{
		errmsg = GetUVError(r);
		fprintf(stdout,"uv_ip4_addr error:%s\n",errmsg.c_str());

		return false;
	}
	r = uv_tcp_connect(&connect_req,&client_handle,(sockaddr*)&bind_addr,AfterConnect);
	if (r)
	{
		errmsg = GetUVError(r);
		fprintf(stdout,"uv_tcp_connect error:%s\n",errmsg.c_str());
		return false;
	}
	r = uv_thread_create(&connect_thread_handle,ConnectThread,this);
	if (r)
	{
		errmsg = GetUVError(r);
		fprintf(stdout,"uv_thread_create error:%s\n",errmsg.c_str());
		return false;
	}
	int wait_count = 0;
	while (connect_status == CONNECT_DIS)
	{
		Sleep_i(100);
		if(++wait_count > 100) 
		{
			connect_status = CONNECT_TIMEOUT;
			break;
		}

	}
	
	return connect_status == CONNECT_FINISH;
}
void TCPClient::AllocBufferForRecv(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	TCPClient *theclass = (TCPClient*)handle->data;
	
	*buf = theclass->read_buf;
}
void TCPClient::AfterConnect(uv_connect_t* req, int status)
{
	TCPClient* theclass = (TCPClient*)req->data;
	if (status)
	{
		theclass->errmsg = GetUVError(status);
		fprintf(stdout,"status error:%s\n",theclass->errmsg.c_str());
		return;

	}
	int r = uv_read_start(req->handle,AllocBufferForRecv,AfterRecv);
	if (r)
	{
		theclass->errmsg = GetUVError(status);
		fprintf(stdout,"uv_read_start error:%s\n",theclass->errmsg.c_str());
		theclass->connect_status = CONNECT_ERROR;
		return;

	}
	else
	{
		theclass->connect_status = CONNECT_FINISH;
	}
	fprintf(stdout,"客户端:(%p) run\n",theclass);
}
void TCPClient::AfterRecv(uv_stream_t* stream,ssize_t nread,const uv_buf_t* buf)
{
	if (!stream->data)
	{
		return;
	}
	TCPClient* theclass = (TCPClient*)stream->data;
	if (nread < 0)
	{
		if (nread == UV_EOF)
		{
			 fprintf(stdout,"服务器主动断开,Client为%p\n",stream);
			
		}
		else if(nread == UV_ECONNRESET)
		{
			 fprintf(stdout,"服务器异常断开,Client为%p\n",stream);
		}
		else
		{
			   fprintf(stdout,"服务器异常断开，,Client为%p:%s\n",stream,GetUVError(nread));
		}
		theclass->close_type = -1;
		theclass->Close();
		return;
	}
	if (nread > 0)
	{
		if (theclass->recvcb)
		{
			buf->base[nread] ='\0';
			theclass->recvcb((const unsigned char*)buf->base,theclass->recvcb_userdata);
		}

	}
}


void TCPClient::PrepareCB(uv_prepare_t* handle)
{

	TCPClient *theclass = (TCPClient*)handle->data;
	//检测是否关闭
	if (theclass->isuseraskforclosed) 
	{
		theclass->closeinl();
		return;
	}
}


void TCPClient::CheckCB(uv_check_t* handle)
{

	//check阶段暂时不处理任何事情
}

void TCPClient::IdleCB(uv_idle_t* handle )
{
	
	//check阶段暂时不处理任何事情
}
int TCPClient::Send(const char* data, std::size_t len)
{
	if (!data || len ==0)
	{
		errmsg= "send data is null or len less than zero.";
		fprintf(stdout,"%s\n",errmsg.c_str());
		return 1;
	}
	uv_mutex_lock(&mutex_write_buf);
	write_buf.base = (char*)data;
	write_buf.len = len;
	uv_mutex_unlock(&mutex_write_buf);
	uv_write_t *write_req = NULL;
	uv_mutex_lock(&mutex_writereq);
	if (writereq_list.empty())
	{
		uv_mutex_unlock(&mutex_writereq);
		write_req = (uv_write_t*)malloc(sizeof(*write_req));
		write_req->data = this;
	}
	else
	{
		write_req = writereq_list.front();
		writereq_list.pop_front();
		uv_mutex_unlock(&mutex_writereq);

	}
	int r = uv_write((uv_write_t*)write_req, (uv_stream_t*)&client_handle,&write_buf, 1, AfterSend);//发送
	if (r) 
	{
		writereq_list.push_back(write_req);//发送失败，不会调用AfterSend函数，所以得回收req
		errmsg = GetUVError(r);
		fprintf(stdout,"send error.%s",errmsg.c_str());
		return 1;
	}

	return 0;
	
}

void TCPClient::AfterSend( uv_write_t *req, int status )
{
	//printf("0\n");
	//回收uv_write_t
	TCPClient *theclass = (TCPClient*)req->data;
	uv_mutex_lock(&theclass->mutex_writereq);
	theclass->writereq_list.push_back(req);
	uv_mutex_unlock(&theclass->mutex_writereq);


	//回收uv_write_t
	if (status < 0) 
	{
		//LOGE("发送数据有误:"<<GetUVError(status));
		theclass->errmsg = GetUVError(status);
		fprintf(stderr, "Write error %s\n",theclass->errmsg.c_str());

	}

}

bool TCPClient::SetNoDelay(bool enable)
{
	int r = uv_tcp_nodelay(&client_handle,enable);
	if (r)
	{
		errmsg = GetUVError(r);
		fprintf(stdout,"SetNoDelay error: %s",errmsg.c_str());
		return false;
	}
	return true;
}