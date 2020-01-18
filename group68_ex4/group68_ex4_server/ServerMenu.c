#include "ServerMsgUtils.h"
#include "ServerHeader.h" 
#include "SocketServer.h"

/*Runs server main menu. gets a msg to append and t_socket. return ERROR_RETURN for fail, 1 for success*/
int MainMenu(Msg_t *msg, SOCKET *t_socket, char *UserName)
{
	int RetVal = 1;
	int RetValB = 1;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	char *AcceptedStr = NULL;
	/*AcceptedStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == AcceptedStr)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}*/
	while (TRUE)
	{
		SendRes = SendString("SERVER_MAIN_MENU\n", *t_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			goto DealWithError1;
		}
		AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, *t_socket); //Get choice from client
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			goto DealWithError1;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			goto DealWithError1;
		}

		RetValB = ClientMsgDecode(AcceptedStr, msg); //msg from str to msg struct
		if (RetVal == ERROR_RETURN)
		{
			printf("Wrong meassege from client, closing thread.\n");
			goto DealWithError1;
		}
		if (msg->MsgType != CLIENT_CPU && msg->MsgType != CLIENT_VERSUS && msg->MsgType != CLIENT_LEADERBOARD && msg->MsgType != CLIENT_DISCONNECT)
		{
			printf("Wrong meassege from client, closing thread.\n");
			goto DealWithError1;
		}


		if (msg->MsgType == CLIENT_CPU)
		{
			RetValB = VsCPU(msg, t_socket, UserName);
			if (RetValB == ERROR_RETURN)
			{
				goto DealWithError1;
			}
			continue;
		}
		/*else if (msg->MsgType == CLIENT_VERSUS) //TODO
		{
			RetValB = VsHuman();
			if (RetValB ==ERROR_RETURN)
			{
				goto DealWithError1;
			}
			continue;
		}
		else if (msg->MsgType == CLIENT_LEADERBOARD) //TODO
		{
			RetValB = ShowLeaderboard();
			if (RetValB == ERROR_RETURN)
			{
				goto DealWithError1;
			}
			continue;
		}*/
		else if (msg->MsgType == CLIENT_DISCONNECT) //TODO
		{
			goto CleanExit;
			RetVal = 1;
		}
	}

DealWithError1:
	RetVal = ERROR_RETURN;
CleanExit:
	free(AcceptedStr);


	return RetVal;
}


/*Play against the server, gets msg to append. returns ERROR_RETURN for fail, 1 for success*/
int VsCPU(Msg_t *msg, SOCKET *t_socket, char *UserName)
{
	int RetVal = 1;
	int RetValB = 1;
	int random, CPUMove, winner;
	char *CPUMoveStr = NULL;
	int PlayerMoveInt = -1;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	char *AcceptedStr = NULL;
	char *SentStr = NULL;
	/*AcceptedStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == AcceptedStr)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}*/
	SentStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == SentStr)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}
	CPUMoveStr = (char*)malloc(MAX_MOVE_SIZE * sizeof(char));
	if (NULL == CPUMoveStr)
	{
		printf("Error in malloc, closing thread.\n");
		free(SentStr);
		return ERROR_RETURN;
	}
	while (TRUE)
	{
		srand(time(NULL));
		random = rand() % 5;
		CPUMove = ROCK + random;
		GenerateMoveStr(CPUMove, CPUMoveStr);
		SendRes = SendString("SERVER_PLAYER_MOVE_REQUEST\n", *t_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			goto DealWithError1;
		}
		AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, *t_socket); //Get choice from client
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			goto DealWithError1;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			goto DealWithError1;
		}
		RetValB = ClientMsgDecode(AcceptedStr, msg); //msg from str to msg struct
		if (msg->MsgType != CLIENT_PLAYER_MOVE)
		{
			printf("Wrong meassege from client, closing thread.\n");
			goto DealWithError1;
		}

		PlayerMoveInt = GenerateMoveInt(msg->MsgParams[0]);

		winner = WhoWon(CPUMove, PlayerMoveInt);
		strcpy(SentStr, "SERVER_GAME_RESULTS:Server;\0");
		strcat(SentStr, CPUMoveStr);
		strcat(SentStr, ";");
		strcat(SentStr, msg->MsgParams[0]);
		strcat(SentStr, ";");
		switch (winner)
		{
		case(1):
			strcat(SentStr, "Server");
			break;
		case(2):
			strcat(SentStr, UserName);
			break;
		case(0):
			strcat(SentStr, "TIE");
			break;
		}
		strcat(SentStr, "\n");
		SendRes = SendString(SentStr, *t_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			goto DealWithError1;
		}
		SendRes = SendString("SERVER_GAME_OVER_MENU\n", *t_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			goto DealWithError1;
		}

		AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, *t_socket); //Get choice from client
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			goto DealWithError1;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			goto DealWithError1;
		}
		RetValB = ClientMsgDecode(AcceptedStr, msg); //msg from str to msg struct
		if (msg->MsgType == CLIENT_REPLAY)
		{
			continue;
		}
		else if (msg->MsgType == CLIENT_MAIN_MENU)
		{
			goto CleanExit;
		}
		else
		{
			goto DealWithError1;
		}
	}

	DealWithError1:
		RetVal = ERROR_RETURN;
	CleanExit:
		free(AcceptedStr);
		free(SentStr);
		free(CPUMoveStr);
		return RetVal;
}



/*Gets move number and appends the str*/
void GenerateMoveStr(int MoveNum, char *MoveStr)
{
	switch (MoveNum)
	{
	case ROCK:
		strcpy(MoveStr, "ROCK");
		break;
	case PAPER:
		strcpy(MoveStr, "PAPER");
		break;
	case SCISSORS:
		strcpy(MoveStr, "SCISSORS");
		break;
	case LIZARD:
		strcpy(MoveStr, "LIZARD");
		break;
	case SPOCK:
		strcpy(MoveStr, "SPOCK");
		break;
	}
}

/*Gets move str and returns move num*/
int GenerateMoveInt(char *MoveStr)
{
	if(STRINGS_ARE_EQUAL(MoveStr, "ROCK"))
		return ROCK;
	if (STRINGS_ARE_EQUAL(MoveStr, "PAPER"))
		return PAPER;
	if (STRINGS_ARE_EQUAL(MoveStr, "SCISSORS"))
		return SCISSORS;
	if (STRINGS_ARE_EQUAL(MoveStr, "LIZARD"))
		return LIZARD;
	if (STRINGS_ARE_EQUAL(MoveStr, "SPOCK"))
		return SPOCK;
}