#pragma once

#define _CRT_SECURE_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS


#ifndef CLIENT_MSG_UTILS
#define CLIENT_MSG_UTILS

#include <stdio.h>
#include <string.h>
#include <winsock2.h>
#include <time.h>
#include <stdlib.h>



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
#define MAX_USERNAME 20
#define MAX_MSG_SIZE 100 //TODO - check max message size
#define SEND_STR_SIZE (MAX_PATH+MAX_USERNAME) //TODO - check max message size
#define MAX_PARAMS 4
#define MAX_PARAM_LEN MAX_USERNAME
#define MAX_MOVE_SIZE 10
#define MAX_TYPE_LEN 35

typedef struct Msg
{
	char MsgParams[MAX_PARAMS][MAX_PARAM_LEN];
	int MsgType;
	int NumOfParams;
}Msg_t;

void MsgInit(Msg_t *Msg);
int ServerMsgDecode(char *MsgStr, Msg_t *msg);
int GetParams(char **params, char *MsgStr);
void FillMsg(Msg_t *msg, int type, char **params, int NumOfParams);

#endif //CLIENT_MSG_UTILS