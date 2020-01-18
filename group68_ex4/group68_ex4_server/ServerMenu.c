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
		else if (msg->MsgType == CLIENT_VERSUS)
		{
			RetValB = VsHuman(msg, t_socket, UserName);
			if (RetValB ==ERROR_RETURN)
			{
				goto DealWithError1;
			}
			continue;
		}
		/*else if (msg->MsgType == CLIENT_LEADERBOARD) //TODO
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


/*the server, gets msg to append. returns ERROR_RETURN for fail, 1 for success*/
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

/*Starts play vs other player sequens. gets socket, msg type and username. returns 1 or 0 for success and ERROR_RETURN for fail*/
int VsHuman(Msg_t *msg, SOCKET *t_socket, char *UserName)
{
	DWORD FirstPlayerDword = NULL;
	DWORD VsHumenDword = NULL;
	DWORD OtherPlayerIn = NULL;
	int IAmFirst = -1;
	int RetVal = 0;
	int FirstGame = TRUE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	char *AcceptedStr = NULL;
	char *SentStr = NULL;
	
	
	while (TRUE) {
		FirstPlayerDword = WaitForSingleObject(UserInGameMut, 1); //first user catches semaphor
		if (FirstPlayerDword == WAIT_FAILED || FirstPlayerDword == WAIT_ABANDONED)
		{
			printf("Error in Mutex, closing thread.\n");
			return ERROR_RETURN;
		}
		if (FirstPlayerDword == WAIT_OBJECT_0)
		{
			IAmFirst = TRUE;
			strcpy(FirstPlayer, UserName);
			OtherPlayerIn = WaitForSingleObject(TwoHumansEvent, (2 * WAIT_TIME)); //Wait for second player
			if (OtherPlayerIn == WAIT_FAILED || OtherPlayerIn == WAIT_ABANDONED)
			{
				printf("Error in Event, closing thread.\n");
				return ERROR_RETURN;
			}
			if (OtherPlayerIn == WAIT_TIMEOUT)
			{
				if (FirstGame == TRUE)
					SendRes = SendString("SERVER_NO_OPPONENTS", *t_socket);
				else
				{
					SentStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
					if (NULL == SentStr)
					{
						printf("Error in malloc, closing thread.\n");
						return ERROR_RETURN;
					}
					strcpy(SentStr, "SERVER_OPPONENT_QUIT");
					if (IAmFirst == TRUE)
						strcat(SentStr, SecondPlayer);
					else
						strcat(SentStr, FirstPlayer);
					strcat(SentStr, "\n");
					SendRes = SendString(SentStr, *t_socket);
				}
				if (SendRes == TRNS_FAILED)
				{
					printf("Service socket error while writing, closing thread.\n");
					free(SentStr);
					return ERROR_RETURN;
				}
				free(SentStr);
				return 0;
			}
			if (OtherPlayerIn == WAIT_OBJECT_0)
			{
				RetVal = PlayVsHuman(msg, t_socket, IAmFirst);
				if (RetVal == ERROR_RETURN)
				{
					return ERROR_RETURN;
				}
			}
		}
		if (FirstPlayerDword == WAIT_TIMEOUT)
		{
			IAmFirst = FALSE;
			BOOL EventBool = FALSE;
			EventBool = SetEvent(TwoHumansEvent);
			if (EventBool == FALSE)
			{
				printf("Event error while setting, closing thread.\n");
				return ERROR_RETURN;
			}
			strcpy(SecondPlayer, UserName);
			RetVal = PlayVsHuman(msg, t_socket, IAmFirst);
			if (RetVal == ERROR_RETURN)
			{
				return ERROR_RETURN;
			}
		}
		FirstGame = FALSE;
		SendRes = SendString("SERVER_GAME_OVER_MENU\n", *t_socket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			return ERROR_RETURN;
		}

		AcceptedStr = NULL;
		RecvRes = ReceiveString(&AcceptedStr, *t_socket); //Get choice from client
		if (RecvRes == TRNS_FAILED)
		{
			printf("Service socket error while reading, closing thread.\n");
			return ERROR_RETURN;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection closed while reading, closing thread.\n");
			return ERROR_RETURN;
		}
		BOOL RealeaseBool = FALSE;
		RetVal = ClientMsgDecode(AcceptedStr, msg); //msg from str to msg struct
		if (msg->MsgType == CLIENT_REPLAY)
		{
			continue;
		}
		else if (msg->MsgType == CLIENT_MAIN_MENU)
		{
			if (IAmFirst == TRUE)
			{
				RealeaseBool= ReleaseMutex(UserInGameMut); //first user Releases mutex
				if (RealeaseBool == FALSE)
				{
					printf("Error releasing mutex. closing thread\n");
					return ERROR_RETURN;
				}
			}
			else
			{
				RealeaseBool = ResetEvent(UserInGameMut); //first user Releases mutex
				if (RealeaseBool == FALSE)
				{
					printf("Error releasing mutex. closing thread\n");
					return ERROR_RETURN;
				}
				return 1;
			}
		}
		else
		{

			return ERROR_RETURN;
		}
		return 1;
	}
}


PlayVsHuman(Msg_t *msg, SOCKET *t_socket, int IAmFirst)
{
	int RetVal = 1;
	int RetValB = 1;
	int VsMove, winner;
	char *VsMoveStr = NULL;
	int PlayerMoveInt = -1;
	int VsMoveInt = -1;
	DWORD WaitForFile = NULL;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	FILE *GameFile;
	char *AcceptedStr = NULL;
	char *SentStr = NULL;
	SentStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == SentStr)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}
	strcpy(SentStr, "SERVER_INVITE:");
	if (IAmFirst == TRUE)
	{
		strcat(SentStr, SecondPlayer);
	}
	else
	{
		strcat(SentStr, FirstPlayer);
	}
	strcat(SentStr, "\n");
	SendRes = SendString(SentStr, *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		free(SentStr);
		return ERROR_RETURN;
	}

	SendRes = SendString("SERVER_PLAYER_MOVE_REQUEST", *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		free(SentStr);
		return ERROR_RETURN;
	}
	Sleep(100);
	AcceptedStr = NULL;
	RecvRes = ReceiveString(&AcceptedStr, *t_socket); //Get choice from client
	if (RecvRes == TRNS_FAILED)
	{
		printf("Service socket error while reading, closing thread.\n");
		free(SentStr);
		return ERROR_RETURN;
	}
	else if (RecvRes == TRNS_DISCONNECTED)
	{
		printf("Connection closed while reading, closing thread.\n");
		free(SentStr);
		return ERROR_RETURN;
	}
	RetValB = ClientMsgDecode(AcceptedStr, msg); //msg from str to msg struct
	if (msg->MsgType != CLIENT_PLAYER_MOVE)
	{
		printf("Wrong meassege from client, closing thread.\n");
		free(SentStr);
		return ERROR_RETURN;
	}
	VsMoveStr = (char*)malloc(MAX_MOVE_SIZE * sizeof(char));
	if (NULL == VsMoveStr)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}
	/*Mutex on game file so users don't write together*/
	WaitForFile = WaitForSingleObject(GameFileMut, INFINITE);
	if (WaitForFile != WAIT_OBJECT_0)
	{
		printf("Error in file mutex. closing thread.\n");
		free(SentStr);
		free(VsMoveStr);
		return ERROR_RETURN;
	}
	GameFile = fopen(FILE_PATH, "r");
	if (NULL == GameFile) //no file yet
	{
		GameFile = fopen(FILE_PATH, "w");
		if (NULL == GameFile)
		{
			printf("Error in writing to file. closing thread.\n");
			free(SentStr);
			free(VsMoveStr);
			return ERROR_RETURN;
		}
		fprintf(GameFile, msg->MsgParams[0]);
		fclose(GameFile);
		RetVal = ReleaseMutex(GameFileMut);
		if (RetVal == FALSE)
		{
			printf("Error in writing to file. closing thread.\n");
			free(SentStr);
			free(VsMoveStr);
			return ERROR_RETURN;
		}
		Sleep(10); // to make sure other user catches file sem
		WaitForFile = WaitForSingleObject(GameFileMut, INFINITE);
		if (WaitForFile != WAIT_OBJECT_0)
		{
			printf("Error in file mutex. closing thread.\n");
			free(SentStr);
			free(VsMoveStr);
			return ERROR_RETURN;
		}
		GameFile = fopen(FILE_PATH, "r");
		if (NULL == GameFile)
		{
			printf("Error in writing to file. closing thread.\n");
			free(SentStr);
			free(VsMoveStr);
			return ERROR_RETURN;
		}
		fgets(VsMoveStr, (MAX_MOVE_SIZE * sizeof(char)), GameFile); //jumps to right line
		fgets(VsMoveStr, (MAX_MOVE_SIZE * sizeof(char)), GameFile); //get other players move
		fclose(GameFile);
		remove(FILE_PATH);
		RetVal = ReleaseMutex(GameFileMut);
		if (RetVal == FALSE)
		{
			printf("Error in writing to file. closing thread.\n"); //write my move
			free(SentStr);
			free(VsMoveStr);
			return ERROR_RETURN;
		}
	}
	else //I am second to write
	{
		fgets(VsMoveStr, (MAX_MOVE_SIZE * sizeof(char)), GameFile); //get other players move
		fclose(GameFile);
		GameFile = fopen(FILE_PATH, "w");
		if (NULL == GameFile)
		{
			printf("Error in writing to file. closing thread.\n");
			free(SentStr);
			free(VsMoveStr);
			return ERROR_RETURN;
		}
		fprintf(GameFile, msg->MsgParams[0]); //write my move
		fclose(GameFile);
		RetVal = ReleaseMutex(GameFileMut);
		if (RetVal == FALSE)
		{
			printf("Error in writing to file. closing thread.\n");
			free(SentStr);
			free(VsMoveStr);
			return ERROR_RETURN;
		}
	}


	PlayerMoveInt = GenerateMoveInt(msg->MsgParams[0]);
	VsMoveInt = GenerateMoveInt(VsMoveStr);

	winner = WhoWon(VsMoveInt, PlayerMoveInt);
	strcpy(SentStr, "SERVER_GAME_RESULTS:");
	if (IAmFirst == TRUE)
	{
		strcat(SentStr, SecondPlayer);
	}
	else
	{
		strcat(SentStr, FirstPlayer);
	}
	strcat(SentStr, ";");
	strcat(SentStr, VsMoveStr);
	strcat(SentStr, ";");
	strcat(SentStr, msg->MsgParams[0]);
	strcat(SentStr, ";");
	switch (winner)
	{
	case(1):
		if (IAmFirst == TRUE)
		{
			strcat(SentStr, SecondPlayer);
		}
		else
		{
			strcat(SentStr, FirstPlayer);
		}
		break;
	case(2):
		if (IAmFirst == TRUE)
		{
			strcat(SentStr, FirstPlayer);
		}
		else
		{
			strcat(SentStr, SecondPlayer);
		}
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
		free(SentStr);
		free(VsMoveStr);
		return ERROR_RETURN;
	}
	free(SentStr);
	free(VsMoveStr);
	return 1;
}
