#include "ServerMsgUtils.h"
#include "ServerHeader.h" 
#include "SocketServer.h"
extern int GlobalExitFlag;


int main(int argc, char *argv[])
{
	DWORD ExitThreadId;
	DWORD MainThreadId;
	HANDLE MainThreadHandles[2];
	int RetVal = 0;

	argv1 = argv[1]; //global

	if (argc != 2)
	{
		printf("Error in arguments");
		return 0;
	}
	GlobalExitFlag = 0;
	MainThreadHandles[EXIT_THREAD] = NULL; //initial killer thread
	MainThreadHandles[EXIT_THREAD] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)ServerKillerThread,
		NULL,
		0,
		&ExitThreadId
	);

	MainThreadHandles[MAIN_THREAD] = NULL;
	MainThreadHandles[MAIN_THREAD] = CreateThread(
		NULL,
		0,
		(LPTHREAD_START_ROUTINE)MainServer,
		NULL,//&argv[1] is a global
		0,
		&MainThreadId
	);

	/* the idea is to wait forever until one of 3 things will happen:
	1. the user will quit
	2. the server will accept "exit" word
	3. an error will accure
	if "exit" was typed into server then the server will act like the state is "quit" from main menu
	and we will wait 15 sec until it will finish and close the threads*/
	DWORD WaitCode = WaitForMultipleObjects(2, MainThreadHandles, FALSE, INFINITE);
	if (WaitCode == WAIT_OBJECT_0 + EXIT_THREAD)//if exit thread is closed
	{
		GlobalExitFlag = 1;
		WaitCode = WaitForSingleObject(MainThreadHandles[MAIN_THREAD], WAIT_TIME);
		if (WaitCode != WAIT_OBJECT_0) {
			if (WaitCode == WAIT_TIMEOUT) {
				printf("TIMEOUT while waiting to main thread to close, Exit process.\n");
				RetVal = WAIT_MAIN_TIMEOUT;
				goto exit_main;
			}
			else {
				printf("Error while waiting to main thread.\n");
				RetVal = WAIT_MAIN_ERROR;
				goto exit_main;
			}
		}
	}
	else if (WaitCode == WAIT_OBJECT_0 + MAIN_THREAD) {/* main thread returned a value before "exit" was typed*/
		goto exit_main;
	}
	else {
		printf("WaitForMultipleObjects() failed. Ending program\n");
		RetVal = WAIT_MAIN_ERROR;
		goto exit_main;
	}
exit_main:
		
	RetVal += CloseHandle(MainThreadHandles[MAIN_THREAD]);
	RetVal += CloseHandle(MainThreadHandles[EXIT_THREAD]);
	return RetVal;
	
}