#include "TcpServer.h"


AcceptClient::AcceptClient(int clientId,uv_loop_t*loop)
{

	isplay = false;
	isclosed = true;
	threadId = 0;
	this->loop = loop;
	this->client_id =clientId;
	recvcb = NULL;
	this->recvcb_userdata = NULL;
	closecb = NULL;
	closecb_userdata = NULL;
	isuseraskforclosed = false;
	resource_index = 0;

	int r;
	read_buf = uv_buf_init((char*)malloc(100),100);
	//write_buf = uv_buf_init((char*)malloc(BUFFER_SIZE),BUFFER_SIZE);
	r = uv_mutex_init(&mutex_write_buf);
	assert(r == 0);
	r = uv_mutex_init(&mutex_writereq);
	assert(r == 0);
	memset(read_buffer,0,100);
	init();


}
AcceptClient::~AcceptClient()
{
	closeinl();
	while (!isclosed)
	{
		Sleep(10);


	}
	if (read_buf.base)
	{
		free(read_buf.base);
		read_buf.base = NULL;
	}
// 	if (write_buf.base)
// 	{
// 		free(write_buf.base);
// 		write_buf.base = NULL;
// 	}
	uv_mutex_destroy(&mutex_write_buf);
	uv_mutex_destroy(&mutex_writereq);

}
void AcceptClient::closeinl()
{
	if (isclosed)
	{
		return;
	}
	if(threadId !=0)
	{
	//	isplay = false;
	//	uv_thread_join(&threadId);
	//	threadId = 0;

	}
	
	uv_mutex_lock(&mutex_writereq);
	for (auto it = writereq_list.begin(); it != writereq_list.end();++it)
	{
		free(*it);
	}
	writereq_list.clear();
	uv_mutex_unlock(&mutex_writereq);
	uv_close((uv_handle_t*)&client_handle,AfterClientClose);
	uv_close((uv_handle_t*)&prepare_handle,AfterClientClose);
}
void AcceptClient::AfterClientClose( uv_handle_t *handle )
{
	AcceptClient *theclass = (AcceptClient*) handle->data;
	assert(theclass);
	if (handle == (uv_handle_t *)&theclass->prepare_handle)
	{
		handle->data = 0;

	}
	if (handle == (uv_handle_t *)&theclass->client_handle)
	{
		handle->data = 0;

	}
	if (theclass->prepare_handle.data ==0 &&
		theclass->client_handle.data == 0)
	{
		theclass->isclosed = true;
		theclass->isuseraskforclosed = false;
		if (theclass->closecb) 
		{
			//通知TCPServer此客户端已经关闭
			theclass->closecb(theclass->client_id,theclass->closecb_userdata);
		}

	}


}

void AcceptClient::PrepareCB( uv_prepare_t* handle )
{
	AcceptClient *theclass = (AcceptClient*)handle->data;
	assert(theclass);
	if (theclass->isuseraskforclosed)
	{
		theclass->closeinl();

		return;
	}
	if (theclass->isclosed)
	{
		return;
	}
	if(!theclass->iplImage_pool.empty())
	{
// 		auto_ptr<ReceivedImage> image =theclass->image_pool.pop();
// 		ReceivedImage* recvimage =image.get();
// 		if (recvimage == NULL)
// 		{
// 			return;
// 		}

// 		IplImage *img = YUV420_To_IplImage_Opencv(recvimage->pImg, recvimage->ImgWidth, recvimage->ImgHeight);
// 		assert(img);
// 		CvSize cz = cvSize(640,480);
// 		IplImage *NewImg = cvCreateImage(cz,img->depth,img->nChannels);
		uv_mutex_lock(&theclass->mutex_write_buf);
		IplImage *image = theclass->iplImage_pool.front();
		theclass->iplImage_pool.pop();
		uv_mutex_unlock(&theclass->mutex_write_buf);
		CvSize cz = cvSize(640,480);
		IplImage *NewImg = cvCreateImage(cz,image->depth,image->nChannels);
		Mat src(NewImg);
		vector<uchar> buff;//buffer for coding
		vector<int> param = vector<int>(2);
		param[0]=CV_IMWRITE_JPEG_QUALITY;
		param[1]=60;//default(95) 0-100
		imencode(".jpg",src,buff,param);
		uv_write_t *write_req = NULL;
		//uv_buf_t resbuf;
		uv_mutex_lock(&theclass->mutex_write_buf);
		theclass->write_buf.base = (char*)(&buff[0]);
		theclass->write_buf.len = buff.size();
		uv_mutex_unlock(&theclass->mutex_write_buf);
		uv_mutex_lock(&theclass->mutex_writereq);
		if (theclass->writereq_list.empty())
		{
			uv_mutex_unlock(&theclass->mutex_writereq);
			write_req = (uv_write_t*)malloc(sizeof(*write_req));
			write_req->data = theclass;
			
		}
		else
		{
			write_req = theclass->writereq_list.front();
			theclass->writereq_list.pop_front();
			uv_mutex_unlock(&theclass->mutex_writereq);

		}
		
		//uv_write_t *write_req = (uv_write_t*)malloc(sizeof(*write_req));
		int r =uv_write(write_req,(uv_stream_t*)&theclass->client_handle,&theclass->write_buf,1,AfterSend);
		if (r)
		{
			uv_mutex_lock(&theclass->mutex_writereq);
			theclass->writereq_list.push_back(write_req);//发送失败不会调用AfterSend
			uv_mutex_unlock(&theclass->mutex_writereq);
			
			 fprintf(stdout,"send error. %s-%s\n",uv_err_name(r),uv_strerror(r));
		}
	//	theclass->write_buf.base
		cvReleaseImage(&NewImg);//释放图像内存
		cvReleaseImage(&image);
	}


}
void AcceptClient::AfterSend( uv_write_t *req, int status )
{
	printf("0\n");
	//回收uv_write_t
	AcceptClient *theclass = (AcceptClient*)req->data;
	uv_mutex_lock(&theclass->mutex_writereq);
	theclass->writereq_list.push_back(req);
	uv_mutex_unlock(&theclass->mutex_writereq);
	

	//回收uv_write_t
	if (status < 0) 
	{
		//LOGE("发送数据有误:"<<GetUVError(status));
		fprintf(stderr, "Write error %s.%s\n",uv_err_name(status),uv_strerror(status));
	}

}
bool AcceptClient::init()
{
	int r;
	r = uv_prepare_init(loop,&prepare_handle);
	assert(r ==0);
	r = uv_prepare_start(&prepare_handle,PrepareCB);
	assert(r == 0);
	prepare_handle.data = this;
	r = uv_tcp_init(loop,&client_handle);
	assert(r == 0);

	client_handle.data = this;
	//启动read封装类
	//readpacket_.SetPacketCB(GetPacket,this);
	isclosed = false;
	return true;
}
// 回调数据给用户
// void AcceptClient::GetPacket( const NetPacket& packethead, const  char* packetdata, void* userdata )
// {
// 	if (!userdata)
// 	{
// 		return;
// 	}
// 	AcceptClient *theclass = (AcceptClient*)userdata;
// 
// 	if (theclass->recvcb) 
// 	{//把得到的数据回调给用户
// 		theclass->recvcb(theclass->client_id,theclass,packetdata,theclass->recvcb_userdata);
// 	}
// }
void AcceptClient::AllocBufferForRecv(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf)
{
	AcceptClient *client = (AcceptClient*)handle->data;
	assert(client);
	*buf = client->read_buf;
}
void AcceptClient::AfterRecv(uv_stream_t *handle, ssize_t nread, const uv_buf_t* buf)
{
	AcceptClient *theclass = (AcceptClient*)handle->data;//服务器的recv带的是clientdata
	assert(theclass);
	if (nread < 0)
	{
		/* Error or EOF */
		if (nread == UV_EOF) 
		{
			fprintf(stdout,"客户端(%d)主动断开\n",theclass->client_id);

		}
		else if (nread == UV_ECONNRESET) 
		{
			fprintf(stdout,"客户端(%d)异常断开\n",theclass->client_id);
			// LOGW("客户端("<<theclass->clientId<<")异常断开");
		}
		else 
		{
			fprintf(stdout,"%s\n",uv_errno_t(nread));
			// LOGW("客户端("<<theclass->client_id_<<")异常断开："<<GetUVError(nread));
		}
		theclass->Close();
		return;
	} 
	else if (0 == nread) 
	{
		/* Everything OK, but nothing read. */

	} 
	else 
	{
		if(theclass->recvcb)
		{

			memcpy(theclass->read_buffer,buf->base,nread);
			theclass->recvcb(theclass->client_id,theclass,theclass->read_buffer,theclass->recvcb_userdata);
		}
		
		//theclass->readpacket_.recvdata((const unsigned char*)buf->base,nread);//新方式-解析完包后再回调数据
	}
}
bool AcceptClient::AcceptByServer( uv_tcp_t* server )
{
	int r = uv_accept((uv_stream_t*)server,(uv_stream_t*)&client_handle);
	assert(r == 0);
	r = uv_read_start((uv_stream_t*)&client_handle,AllocBufferForRecv,AfterRecv);
	string error =uv_strerror(r);
	assert(r == 0);
	return true;

}

void AcceptClient::SetRecvCB(ServerRecvCB pfun, void* userdata)
{
	recvcb = pfun;
	recvcb_userdata = userdata;

}
void AcceptClient::SetClosedCB(TcpCloseCB cb,void *userdata)
{
	closecb = cb;
	closecb_userdata = userdata;

}
int AcceptClient::Send(const char* data, std::size_t len)
{
	uv_write_t *req = NULL;
	req = (uv_write_t*)malloc(sizeof(*req));
	write_buf.base = (char*)data;
	write_buf.len = len;
	int r = uv_write(req,(uv_stream_t*)&client_handle,&write_buf,1,AfterSend);
	return r;
}
int AcceptClient::Send(UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp)
{
	if(!isuseraskforclosed)
	{
		uv_mutex_lock(&mutex_write_buf);
		image_pool.push(pImg,ImgWidth,ImgHeight,uiTimeStamp);
		uv_mutex_unlock(&mutex_write_buf);
	}
	
	return 0;

}
int AcceptClient::Send(IplImage *image)
{
	if(!isuseraskforclosed)
	{
		uv_mutex_lock(&mutex_write_buf);
		
		iplImage_pool.push(cvCloneImage(image));
		uv_mutex_unlock(&mutex_write_buf);
	}

	return 0;

}
TcpServer::TcpServer(void)
{
	newconcb = NULL;
	newconcb_userdata = NULL;
	closedcb = NULL;
	closedcb_userdata = NULL;
	startstatus = START_DIS;
	isclosed = true;
	isuseraskforclosed = false;
	int r;
	r = uv_loop_init(&loop);
	if (r)
	{
		errmsg = uv_strerror(r);
		fprintf(stdout,"init loop error: %s\n",errmsg.c_str());
	}
	r = uv_mutex_init(&mutex_clients);
	if(r)
	{
		errmsg = uv_strerror(r);
		fprintf(stdout,"init loop error: %s\n",errmsg.c_str());
	}
	r = uv_mutex_init(&mutex_puid_client);
	if(r)
	{
		errmsg = uv_strerror(r);
		fprintf(stdout,"init loop error: %s\n",errmsg.c_str());
	}
	r = uv_mutex_init(&mutex_puid_count);
	if(r)
	{
		errmsg = uv_strerror(r);
		fprintf(stdout,"init loop error: %s\n",errmsg.c_str());
	}
}
TcpServer::~TcpServer(void)
{
	Close();
	uv_mutex_destroy(&mutex_clients);
	uv_mutex_destroy(&mutex_puid_client);
	uv_mutex_destroy(&mutex_puid_count);
	uv_loop_close(&loop);

}
void TcpServer::closeinl()
{
	if (isclosed)
	{
		return;
	}
	uv_mutex_lock(&mutex_clients);
	for (auto it = clients_list.begin();it != clients_list.end();++it)
	{
		auto client = it->second;
		client->Close();
	}
	uv_mutex_unlock(&mutex_clients);
	if (!uv_is_closing((uv_handle_t*) &server_handle)) 
	{
		uv_close((uv_handle_t*) &server_handle, AfterServerClose);
	}
	if (!uv_is_closing((uv_handle_t*) &prepare_handle)) 
	{
		uv_close((uv_handle_t*) &prepare_handle, AfterServerClose);
	}
	if (!uv_is_closing((uv_handle_t*) &idle_handle))
	{
		uv_close((uv_handle_t*) &idle_handle, AfterServerClose);
	}
	if (!uv_is_closing((uv_handle_t*) &check_handle)) 
	{
		uv_close((uv_handle_t*) &check_handle, AfterServerClose);
	}
	printf("close server\n");
}
void TcpServer::AfterServerClose(uv_handle_t *handle)
{
	TcpServer *theclass = (TcpServer*)handle->data;
	if (handle == (uv_handle_t *)&theclass->server_handle)
	{
		handle->data = 0;//赋值0，用于判断是否调用过
	}
	if (handle == (uv_handle_t *)&theclass->prepare_handle) 
	{
		handle->data = 0;//赋值0，用于判断是否调用过
	}
	if (handle == (uv_handle_t *)&theclass->check_handle)
	{
		handle->data = 0;//赋值0，用于判断是否调用过
	}
	if (handle == (uv_handle_t *)&theclass->idle_handle) 
	{
		handle->data = 0;//赋值0，用于判断是否调用过
	}
	if (theclass->server_handle.data == 0
		&& theclass->prepare_handle.data == 0
		&& theclass->check_handle.data == 0
		&& theclass->idle_handle.data == 0)
	{
		theclass->isclosed = true;
		theclass->isuseraskforclosed = false;

		if (theclass->closedcb) 
		{//通知TCPServer此客户端已经关闭
			theclass->closedcb(-1,theclass->closedcb_userdata);
		}
	}

}
void TcpServer::ClientClosed(int clientid,void *userdata)
{
	TcpServer *theclass = (TcpServer*)userdata;
	//删除clientId对应的puid
	string puid="";
	uv_mutex_lock(&theclass->mutex_puid_client);
	auto it2 = theclass->puid_client_map.begin();
	for ( ;it2 != theclass->puid_client_map.end();++it2)
	{
		if (it2->second == clientid)
		{
			puid = it2->first;
			theclass->puid_client_map.erase(it2);
			break;
			
		}
	}
	auto it3 = theclass->puid_count_map.find(puid);
	if (it3 != theclass->puid_count_map.end())
	{
		(it3->second)--;
		if (it3->second == 0)
		{
			auto it4 = theclass->puid_thread_map.find(puid);
			if (it4 != theclass->puid_thread_map.end())
			{
				uv_thread_join(&it4->second);
				theclass->puid_thread_map.erase(it4->first);
			}
		}
	}
	
	uv_mutex_unlock(&theclass->mutex_puid_client);
	uv_mutex_lock(&theclass->mutex_clients);


	auto it = theclass->clients_list.find(clientid);
	if (it != theclass->clients_list.end())
	{
		if(theclass->closedcb)
		{
			theclass->closedcb(clientid,theclass->closedcb_userdata);
		}
		
	}
	delete it->second;
	fprintf(stdout,"删除客户端：%d\n",it->first);
	theclass->clients_list.erase(it->first);
	uv_mutex_unlock(&theclass->mutex_clients);


}
void TcpServer::CheckCB( uv_check_t* handle )
{
	//TcpServer *theclass = (TcpServer*)handle->data;
	//check阶段暂时不处理任何事情
}

void TcpServer::IdleCB(uv_idle_t *handle)
{
	//TcpServer *theclass = (TcpServer*)handle->data;
	//Idle阶段暂时不处理任何事情
}


bool TcpServer::init()
{
	if (!isclosed) 
	{
		return true;
	}
	int r;
	r = uv_prepare_init(&loop,&prepare_handle);
	assert( r ==0);
	r = uv_prepare_start(&prepare_handle,PrepareCB);
	assert(r == 0);
	prepare_handle.data = this;
	r = uv_check_init(&loop, &check_handle);
	assert(r == 0);
	r = uv_check_start(&check_handle,CheckCB);
	assert(r == 0);
	check_handle.data = this;
	r = uv_idle_init(&loop,&idle_handle);
	assert(r == 0);
	r = uv_idle_start(&idle_handle,IdleCB);
	assert(r == 0);
	idle_handle.data = this;
	r = uv_tcp_init(&loop,&server_handle);
	assert(r == 0);
	server_handle.data = this;
	r = uv_tcp_nodelay(&server_handle,1);
	assert(r == 0);
	isclosed = false;
	return true;

}

void TcpServer::PrepareCB( uv_prepare_t* handle )
{
	/////////////////////////prepare阶段检测用户是否发送关闭命令
	TcpServer *theclass = (TcpServer*)handle->data;
	if (theclass->isuseraskforclosed)
	{
		theclass->closeinl();
	}

	//检测是否关闭

}
void TcpServer::AcceptConnection(uv_stream_t *server, int status)
{
	printf("AcceptConnection\n");
	if (!server->data)
	{
		return;
	}
	TcpServer *tcpServer =(TcpServer*)server->data;
	if (status)
	{
		printf("%s\n",uv_strerror(status));
		return;
	}
	int clientId = tcpServer->GetAvailaClientID();
	AcceptClient* client = NULL;
	try
	{
		client = new AcceptClient(clientId,&tcpServer->loop);//uv_close回调函数中释放
	}
	catch (bad_alloc* e)
	{
		printf("%s\n",e->what());
	}
	catch (exception* e)
	{
		printf("%s\n",e->what());
	}
	
	_ASSERTE( _CrtCheckMemory( ) );
	uv_mutex_lock(&tcpServer->mutex_clients);
	tcpServer->clients_list.insert(make_pair(clientId,client));
	uv_mutex_unlock(&tcpServer->mutex_clients);
	client->SetClosedCB(ClientClosed,tcpServer);
	if(!client->AcceptByServer((uv_tcp_t*)server)) 
	{

		tcpServer->errmsg = client->GetLastErrMsg();
		client->Close();
		return;
	}
	if (tcpServer->newconcb)
	{
		tcpServer->newconcb(clientId,tcpServer->newconcb_userdata);
	}



}
void TcpServer::SetNewConnectCB(NewConnectCB cb,void *userdata)
{
	newconcb = cb;
	newconcb_userdata = userdata;

}
void TcpServer::SetClosedCB(TcpCloseCB cb,void *userdata )
{
	closedcb = cb;
	closedcb_userdata = userdata;

}

//服务器-接收数据回调函数
void TcpServer::SetRecvCB(int clientid, ServerRecvCB cb, void *userdata)
{
	uv_mutex_lock(&mutex_clients);
	auto itfind = clients_list.find(clientid);
	if (itfind != clients_list.end()) 
	{
		itfind->second->SetRecvCB(cb,userdata);
	}
	uv_mutex_unlock(&mutex_clients);
}

int TcpServer::GetAvailaClientID() const
{
	static int s_id = 0;
	return ++s_id;
}
bool TcpServer::bind(const char* ip, int port)
{
	struct sockaddr_in bind_addr;
	int r = uv_ip4_addr(ip,port,&bind_addr);
	assert (r ==0);
	r = uv_tcp_bind(&server_handle,(const struct sockaddr*)&bind_addr,0);
	errmsg = uv_strerror(r);
	assert(r ==0);
	cout<<"server bind ip="<<ip<<", port="<<port<<endl;
	return true;

}
bool TcpServer::listen(int backlog)
{
	int r = uv_listen((uv_stream_t*)&server_handle,backlog,AcceptConnection);
	assert(r == 0);
	return true;

}

void TcpServer::StartThread( void* arg )
{
	printf("ThreadId=%p\n",uv_thread_self());
	TcpServer *theclass = (TcpServer*)arg;
	theclass->startstatus = START_FINISH;
	theclass->run();
}
bool TcpServer::run(int status)
{
	printf("server runing.\n");
	int r = uv_run(&loop,(uv_run_mode)status);
	assert(r == 0);
	return true;
}
bool TcpServer::Start(const char *ip, int port)
{
	serverport = port;
	serverip = ip;
	int r;
	if (!init())
	{
		return false;
	}
	if(!bind(ip,port))
	{
		return false;
	}
	if(!listen(10))
	{
		return false;
	}
	r = uv_thread_create(&start_threadhandle,StartThread,this);
	assert(r == 0);

	int wait_count = 0;
	while ( startstatus == START_DIS) {
#if defined (WIN32) || defined(_WIN32)
		Sleep(100);
#else
		usleep((100) * 1000);
#endif
		if(++wait_count > 100) {
			startstatus = START_TIMEOUT;
			break;
		}
	}
	return startstatus == START_FINISH;

	

}

int TcpServer::Send(int clientid, const char* data, std::size_t len)
{
	uv_mutex_lock(&mutex_clients);
	auto itfind = clients_list.find(clientid);
	if (itfind == clients_list.end())
	{

		return -1;
	}
	itfind->second->Send(data,len);
	uv_mutex_unlock(&mutex_clients);
	return 0;
}
int TcpServer::Send(int clientid,UCHAR *pImg[3], UINT ImgWidth, UINT ImgHeight, UINT uiTimeStamp)
{
	uv_mutex_lock(&mutex_clients);
	auto itfind = clients_list.find(clientid);
	if (itfind == clients_list.end())
	{
		uv_mutex_unlock(&mutex_clients);
		errmsg = "can't find cliendid ";
		errmsg += std::to_string((long long)clientid);
		printf("%s\n",errmsg.c_str());
		return -1;
	}
	itfind->second->Send(pImg,ImgWidth,ImgHeight,uiTimeStamp);
	uv_mutex_unlock(&mutex_clients);
	return 0;
}

int TcpServer::Send(int clientid,IplImage *image)
{
	uv_mutex_lock(&mutex_clients);
	auto itfind = clients_list.find(clientid);
	if (itfind == clients_list.end())
	{
		uv_mutex_unlock(&mutex_clients);
		errmsg = "can't find cliendid ";
		errmsg += std::to_string((long long)clientid);
		printf("%s\n",errmsg.c_str());
		return -1;
	}
	itfind->second->Send(image);
	uv_mutex_unlock(&mutex_clients);
	return 0;
}

void TcpServer::CloseClient(int clientid)
{
	uv_mutex_lock(&mutex_clients);
	auto itfind = clients_list.find(clientid);
	if (itfind == clients_list.end())
	{
		uv_mutex_unlock(&mutex_clients);
		errmsg = "can't find cliendid ";
		errmsg += std::to_string((long long)clientid);
		printf("%s\n",errmsg.c_str());
	
	}
	itfind->second->Close();
	uv_mutex_unlock(&mutex_clients);
	

}