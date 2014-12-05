#ifndef PACKET_H
#define PACKET_H



// class Packet
// {
// public:
// 	void recvdata(const unsigned char* data, int len)
// 	{
// 		char *buffer = new  char[len+1];
//  		 memset(buffer,0,strlen(buffer));
// 		 memcpy(buffer,data,len);
// // 		 printf("%s\n",buffer);
// 		// packet_cb_(netPacket,buffer,packetcb_userdata_);
// 
// 
// 	}
// 	void SetPacketCB(GetFullPacket pfun, void *userdata)
// 	{
// 		packet_cb_ = pfun;
// 		packetcb_userdata_ = userdata;
// 	}
// private:
// 	GetFullPacket packet_cb_;
// 	 void* packetcb_userdata_;//回调函数所带的自定义数据
// 
// 
// };
//客户端或服务器关闭的回调函数
//服务器：当clientid为-1时，表现服务器的关闭事件
//客户端：clientid无效，永远为-1
typedef void (*TcpCloseCB)(int clientid, void* userdata);

//TCPServer接收到新客户端回调给用户
typedef void (*NewConnectCB)(int clientid, void* userdata);

//TCPServer接收到客户端数据回调给用户
typedef void (*ServerRecvCB)(int clientid, void *clientdata, const  char* buf, void* serverdata);

//TCPClient接收到服务器数据回调给用户
//typedef void (*ClientRecvCB)(const NetPacket& packethead, const  char* buf, void* userdata);

#endif