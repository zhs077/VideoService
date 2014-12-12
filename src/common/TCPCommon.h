#ifndef TCP_COMMON_H_
#define TCP_COMMON_H_
#include <string>
#ifndef BUFFER_SIZE
#define BUFFER_SIZE (1024*1024) //��󻺳���
#endif


inline std::string GetUVError(int r)
{
	std::string err;
	err = uv_err_name(r);
	err +=":";
	err += uv_strerror(r);
	return err;
}
//�ͻ��˻�������رյĻص�����
//����������clientidΪ-1ʱ�����ַ������Ĺر��¼�
//�ͻ��ˣ�clientid��Ч����ԶΪ-1��
typedef void (*TcpCloseCB)(int clientid, void* userdata);
//TCPServer���յ��¿ͻ��˻ص����û�
typedef void (*NewConnectCB)(int clientid, void* userdata);
//TCPServer���յ��ͻ������ݻص����û�
typedef void (*ServerRecvCB)(int clientid, void *clientdata, const  char* buf, void* serverdata);
//TCPClient���յ����������ݻص����û�
typedef void (*ClientRecvCB)( const unsigned char* buf, void* userdata);
#endif