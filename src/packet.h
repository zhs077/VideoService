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
// 	 void* packetcb_userdata_;//�ص������������Զ�������
// 
// 
// };
//�ͻ��˻�������رյĻص�����
//����������clientidΪ-1ʱ�����ַ������Ĺر��¼�
//�ͻ��ˣ�clientid��Ч����ԶΪ-1
typedef void (*TcpCloseCB)(int clientid, void* userdata);

//TCPServer���յ��¿ͻ��˻ص����û�
typedef void (*NewConnectCB)(int clientid, void* userdata);

//TCPServer���յ��ͻ������ݻص����û�
typedef void (*ServerRecvCB)(int clientid, void *clientdata, const  char* buf, void* serverdata);

//TCPClient���յ����������ݻص����û�
//typedef void (*ClientRecvCB)(const NetPacket& packethead, const  char* buf, void* userdata);

#endif