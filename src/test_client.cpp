//
//#include "tcpclient.h"
//using namespace uv;
//
//void tcpclosecb(int clientid, void* userdata)
//{
//	fprintf(stdout,"cliend %d close\n",clientid);
//	tcpclient *client = (tcpclient *)userdata;
//	client->close();
//}
//void readcb(const unsigned char* buf, void* userdata)
//{
//	printf("%s\n",buf);
//
//}
//int main()
//{
//	tcpclient client;
//	client.setclosedcb(tcpclosecb,&client);
//	client.setrecvcb(readcb,&client);
//
//	bool b =client.connect("127.0.0.1",8004);
//	if (!b)
//	{
//		printf("connect error\n");
//		return -1;
//	}
//	client.setnodelay(false);
//	printf("connect success\n");
//	client.send("helloword",sizeof("helloword"));
//	while (true)
//	{
//		sleep_i(10000);
//
//	}
//	return 0;
//}