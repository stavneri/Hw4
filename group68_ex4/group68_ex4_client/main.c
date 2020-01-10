#include "ClientHeader.h"

int main(int argc, char *argv[])
{
	int MainRet = -1;
	int again = -1;
	if (argc != 4)
	{
		printf("Error in arguments");
		return 0;
	}
	MainRet = MainClient(argv[1],argv[2], argv[3]);
	if (MainRet != 0)
	{
		while(MainRet == CONNECTION_FAIL|| MainRet == CONNECTION_TIMEOUT)
		{
			again = ChooseAgain();
			if (again == 1)
			{
				MainRet = MainClient(argv[1], argv[2]);
			}
			else if (again == 2)
			{
				return 0;
			}
		}
	}
}