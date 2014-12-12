#ifndef TCP_COMMON_H_
#define TCP_COMMON_H_
#include <string>
#ifndef BUFFER_SIZE
#define BUFFER_SIZE (1024*1024) //最大缓冲区
#endif


inline std::string GetUVError(int r)
{
	std::string err;
	err = uv_err_name(r);
	err +=":";
	err += uv_strerror(r);
	return err;
}
//客户端或服务器关闭的回调函数
//服务器：当clientid为-1时，表现服务器的关闭事件
//客户端：clientid无效，永远为-1，
typedef void (*TcpCloseCB)(int clientid, void* userdata);
//TCPServer接收到新客户端回调给用户
typedef void (*NewConnectCB)(int clientid, void* userdata);
//TCPServer接收到客户端数据回调给用户
typedef void (*ServerRecvCB)(int clientid, void *clientdata, const  char* buf, void* serverdata);
//TCPClient接收到服务器数据回调给用户
typedef void (*ClientRecvCB)( const unsigned char* buf, void* userdata);
#endif