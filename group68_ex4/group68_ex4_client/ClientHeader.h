#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>

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

/*Client message types*/
#define CLIENT_REQUEST 5500
#define CLIENT_MAIN_MENU 5501
#define CLIENT_CPU 5502
#define CLIENT_VERSUS 5503
#define CLIENT_LEADERBOARD 5504
#define CLIENT_PLAYER_MOVE 5505
#define CLIENT_REPLAY 5506
#define CLIENT_REFRESH 5507
#define CLIENT_DISCONNECT 5508

/*Server message types*/
#define SERVER_MAIN_MENU 6600
#define SERVER_APPROVED 6601
#define SERVER_DENIED 6602
#define SERVER_INVITE 6603
#define SERVER_PLAYER_MOVE_REQUEST 6604
#define SERVER_GAME_RESULTS 6605
#define SERVER_GAME_OVER_MENU 6606
#define SERVER_OPPONENT_QUIT 6607
#define SERVER_NO_OPPONENTS 6608
#define SERVER_LEADERBOARD 6609
#define SERVER_LEADERBOARD_MENU 6610

/*Move types*/
#define ROCK 7700
#define PAPER 7701
#define SCISSORS 7702
#define LIZARD 7703
#define SPOCK 7704

#define ERROR_TYPE 8800
#define	CONNECTION_FAIL 12345
#define CONNECTION_TIMEOUT 12346

#define STRINGS_ARE_EQUAL( Str1, Str2 ) ( strcmp( (Str1), (Str2) ) == 0 )

int ChooseAgain(void);
int ClientMain(char *IPArg, char *PortArg, char *UserNameArg);