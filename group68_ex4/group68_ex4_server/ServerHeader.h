#pragma once


#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include "ServerMsgUtils.h" 
#include "SocketServer.h"
#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <stdlib.h>

#define NUM_OF_WORKER_THREADS 2
#define MAX_LOOPS 2
#define MAX_PATH 256
#define ERROR_RETURN 9999
#define SERVER_ADDRESS_STR "127.0.0.1"
#define WAIT_TIME 15000
#define FILE_PATH "./GameSession.txt"
#define KILL_REQUEST 7575
#define WAIT_MAIN_ERROR 22
#define WAIT_MAIN_TIMEOUT 33


HANDLE KillerThread;
HANDLE MainServerThread;

int GlobalExitFlag;

HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
#define EXIT_THREAD 0
#define MAIN_THREAD 1

SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
HANDLE TwoHumansEvent;
HANDLE UserInGameMut;
HANDLE GameFileMut;
HANDLE FirstPlayerReady;
HANDLE SecondPlayerReady;
char *FirstPlayer;
char *SecondPlayer;
char *argv1;
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )


DWORD MainServer(char *AdressArg);
static DWORD ServiceThread(SOCKET *t_socket);
int MainMenu(Msg_t *msg, SOCKET *t_socket, char *UserName);
int VsCPU(Msg_t *msg, SOCKET *t_socket, char *UserName);
void GenerateMoveStr(int MoveNum, char *MoveStr);
void CleanupWorkerThreads(void);
int WhoWon(int Player1, int Player2);
int VsHuman(Msg_t *msg, SOCKET *t_socket, char *UserName);
DWORD WINAPI ServerKillerThread(void);

HANDLE CreateThreadSimple(LPTHREAD_START_ROUTINE StartAddress,
	LPVOID ParameterPtr,
	LPDWORD ThreadIdPtr);