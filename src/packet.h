#ifndef PACKET_H
#define PACKET_H



typedef struct _NetPacket{//传输自定义数据包头结构
	int32_t version;        //封包的版本号，不同版本包的定义可能不同  :0-3
	unsigned char header;   //包头-可自定义，例如0x02                 :4
	unsigned char tail;     //包尾-可自定义，例如0x03                 :5
	unsigned char check[16];//pack data校验值-16字节的md5二进制数据   :6-21
	int32_t type;           //包数据的类型                            :22-25
	int32_t datalen;        //包数据的内容长度-不包括此包结构和包头尾 :26-29
	int32_t reserve;        //包数据保留字段-暂时不使用               :30-33
}NetPacket;
typedef void (*GetFullPacket)(const NetPacket& packethead, const  char* packetdata, void* userdata);
class Packet
{
public:
	void recvdata(const unsigned char* data, int len)
	{
		char *buffer = new  char[len+1];
 		 memset(buffer,0,strlen(buffer));
		 memcpy(buffer,data,len);
// 		 printf("%s\n",buffer);
		NetPacket netPacket;
		 packet_cb_(netPacket,buffer,packetcb_userdata_);


	}
	void SetPacketCB(GetFullPacket pfun, void *userdata)
	{
		packet_cb_ = pfun;
		packetcb_userdata_ = userdata;
	}
private:
	GetFullPacket packet_cb_;
	 void*         packetcb_userdata_;//回调函数所带的自定义数据


};
//客户端或服务器关闭的回调函数
//服务器：当clientid为-1时，表现服务器的关闭事件
//客户端：clientid无效，永远为-1
typedef void (*TcpCloseCB)(int clientid, void* userdata);

//TCPServer接收到新客户端回调给用户
typedef void (*NewConnectCB)(int clientid, void* userdata);

//TCPServer接收到客户端数据回调给用户
typedef void (*ServerRecvCB)(int clientid, void *clientdata, const  char* buf, void* serverdata);

//TCPClient接收到服务器数据回调给用户
typedef void (*ClientRecvCB)(const NetPacket& packethead, const  char* buf, void* userdata);

#endif