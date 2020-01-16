#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <stdlib.h>

#define SERVER_ADDRESS_STR "127.0.0.1"
#define NUM_OF_WORKER_THREADS 2
#define MAX_LOOPS 2
#define MAX_PATH 256
#define MAX_USERNAME 20
#define MAX_MSG_SIZE 100 //TODO - check max message size
#define SEND_STR_SIZE (MAX_PATH+MAX_USERNAME) //TODO - check max message size
#define ERROR_RETURN 9999

HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];


#define EXIT_REQUEST 9876
#define ERROR_TYPE 8800
#define	CONNECTION_FAIL 12345
#define CONNECTION_TIMEOUT 12346

#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

int ChooseAgain(void);
int ClientMain(char *IPArg, char *PortArg, char *UserNameArg);
int ClientRequest(char *UserNameArg, char *IPArg, char *PortArg, SOCKET MainSocket);
void PrintMainMenu(void);
void MoveOptions(char *UpperMove);
int ClientVsCPU(SOCKET MainSocket);