#ifndef PACKET_H
#define PACKET_H



typedef struct _NetPacket{//�����Զ������ݰ�ͷ�ṹ
	int32_t version;        //����İ汾�ţ���ͬ�汾���Ķ�����ܲ�ͬ  :0-3
	unsigned char header;   //��ͷ-���Զ��壬����0x02                 :4
	unsigned char tail;     //��β-���Զ��壬����0x03                 :5
	unsigned char check[16];//pack dataУ��ֵ-16�ֽڵ�md5����������   :6-21
	int32_t type;           //�����ݵ�����                            :22-25
	int32_t datalen;        //�����ݵ����ݳ���-�������˰��ṹ�Ͱ�ͷβ :26-29
	int32_t reserve;        //�����ݱ����ֶ�-��ʱ��ʹ��               :30-33
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
	 void*         packetcb_userdata_;//�ص������������Զ�������


};
//�ͻ��˻�������رյĻص�����
//����������clientidΪ-1ʱ�����ַ������Ĺر��¼�
//�ͻ��ˣ�clientid��Ч����ԶΪ-1
typedef void (*TcpCloseCB)(int clientid, void* userdata);

//TCPServer���յ��¿ͻ��˻ص����û�
typedef void (*NewConnectCB)(int clientid, void* userdata);

//TCPServer���յ��ͻ������ݻص����û�
typedef void (*ServerRecvCB)(int clientid, void *clientdata, const  char* buf, void* serverdata);

//TCPClient���յ����������ݻص����û�
typedef void (*ClientRecvCB)(const NetPacket& packethead, const  char* buf, void* userdata);

#endif