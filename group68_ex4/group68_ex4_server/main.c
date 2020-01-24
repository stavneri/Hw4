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
	KillerThread = NULL; //initial killer thread
	KillerThread = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ServerKillerThread,
		NULL,
		0,
		NULL
	);
	if (GlobalExitFlag)
		CloseHandle(KillerThread);
	MainServerThread = NULL;
	MainServerThread = CreateThread( 
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)MainServer,
		&argv[1],
		0,
		NULL
	);
	if (GlobalExitFlag)
		CloseHandle(MainServerThread);
}