#include "ClientHeader.h"
#include "ClientMsgUtils.h"
#include "SocketClient.h"

int main(int argc, char *argv[])
{
	int MainRet = -1;
	int again = -1;
	if (argc != 4)
	{
		printf("Error in arguments");
		return 0;
	}
	MainRet = ClientMain(argv[1],argv[2], argv[3]);
	while (MainRet!=1)
	{
		while (MainRet == CONNECTION_FAIL || MainRet == CONNECTION_TIMEOUT)
		{
			again = ChooseAgain();
			if (again == 1)
			{
				MainRet = ClientMain(argv[1], argv[2], argv[3]);
			}
			else if (again == 2)
			{
				return 0;
			}
		}
	}
}