#include "ServerMsgUtils.h"
#include "ServerHeader.h" 
#include "SocketServer.h"


int main(int argc, char *argv[])
{
	if (argc != 2)
	{
		printf("Error in arguments");
		return 0;
	}
	MainServer(argv[1]);
}