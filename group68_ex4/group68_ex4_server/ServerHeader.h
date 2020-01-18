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


HANDLE ThreadHandles[NUM_OF_WORKER_THREADS];
SOCKET ThreadInputs[NUM_OF_WORKER_THREADS];
HANDLE TwoHumansEvent;
HANDLE UserInGameMut;
HANDLE GameFileMut;
char *FirstPlayer;
char *SecondPlayer;
#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )


void MainServer(char *AdressArg);
static DWORD ServiceThread(SOCKET *t_socket);
int MainMenu(Msg_t *msg, SOCKET *t_socket, char *UserName);
int VsCPU(Msg_t *msg, SOCKET *t_socket, char *UserName);
void GenerateMoveStr(int MoveNum, char *MoveStr);
void CleanupWorkerThreads(void);
int WhoWon(int Player1, int Player2);
int VsHuman(Msg_t *msg, SOCKET *t_socket, char *UserName);