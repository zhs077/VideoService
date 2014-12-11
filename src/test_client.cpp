
#include "TcpClient.h"
using namespace uv;

void TCPCloseCB(int clientid, void* userdata)
{
	fprintf(stdout,"cliend %d close\n",clientid);
	TCPClient *client = (TCPClient *)userdata;
	client->Close();
}
void ReadCB(const unsigned char* buf, void* userdata)
{
	printf("%s\n",buf);

}
int main()
{
	TCPClient client;
	client.SetClosedCB(TCPCloseCB,&client);
	client.SetRecvCB(ReadCB,&client);

	bool b =client.Connect("127.0.0.1",8004);
	if (!b)
	{
		printf("connect error\n");
		return -1;
	}
	client.SetNoDelay(false);
	printf("connect success\n");
	client.Send("helloword",sizeof("helloword"));
	while (true)
	{
		Sleep_i(10000);

	}
	return 0;
}