

#include "ClientHeader.h"
#include "ClientMsgUtils.h"
#include "SocketClient.h"
#include "SocketClient.c"



int ClientMain(char *IPArg, char *PortArg, char *UserNameArg)
{
	SOCKET m_Socket;
	unsigned long Address;
	SOCKADDR_IN ClientService;
	int port, RetTemp = -1;
	
	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("Failed connecting to server on %s : %s", IPArg, PortArg);
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return CONNECTION_FAIL;
	}

	/* The WinSock DLL is acceptable. Proceed. */

	// Create a socket.    
	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (m_Socket == INVALID_SOCKET)
	{
		printf("Failed connecting to server on %s : %s", IPArg, PortArg);
		WSACleanup();
		return CONNECTION_FAIL;
	}

	// Bind the socket.
	/*
		For a server to accept client connections, it must be bound to a network address within the system.
		The following code demonstrates how to bind a socket that has already been created to an IP address
		and port.
		Client applications use the IP address and port to connect to the host network.
		The sockaddr structure holds information regarding the address family, IP address, and port number.
		sockaddr_in is a subset of sockaddr and is used for IP version 4 applications.
   */
   // Create a sockaddr_in object and set its values.
   // Declare variables

	Address = inet_addr(IPArg);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n", IPArg);
		WSACleanup();
		return CONNECTION_FAIL;
	}
	port = atoi(PortArg);
	ClientService.sin_family = AF_INET;
	ClientService.sin_addr.s_addr = Address;
	ClientService.sin_port = htons(port); //The htons function converts a u_short from host to TCP/IP network byte order 
									   //( which is big-endian ).
	/*
		The three lines following the declaration of sockaddr_in service are used to set up
		the sockaddr structure:
		AF_INET is the Internet address family.
		"127.0.0.1" is the local IP address to which the socket will be bound.
		The main argument is the port number to which the socket will be bound.
	*/

	// Call the bind function, passing the created socket and the sockaddr_in structure as parameters. 
	// Check for general errors.
	if (connect(m_Socket, (SOCKADDR*) &ClientService, sizeof(ClientService)) == SOCKET_ERROR) {
		printf("Failed connecting to server on %s:%s\n", IPArg, PortArg);
		WSACleanup();
		return CONNECTION_FAIL;
	}
	/*SOCKET AcceptSocket = accept(m_Socket, NULL, NULL);
	if (AcceptSocket == INVALID_SOCKET)
	{
		printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
		WSACleanup();
		return CONNECTION_FAIL;
	}*/
	printf("Connected to Server on %s:%s\n", IPArg, PortArg);
	RetTemp = ClientRequest(UserNameArg, IPArg, PortArg, m_Socket);
	if (RetTemp != 1)
	{
		WSACleanup();
		return RetTemp;
	}
	while (RetTemp == 1)
	{
		RetTemp = ClientRun(UserNameArg, m_Socket);
	}
	WSACleanup();
	return RetTemp;
}

/*Runs disconnected menu*/
/*return 1 if user chose to run again; 2 if he chose to exit*/
int ChooseAgain(void)
{
	char Meassage[256];
	printf("Choose what to do next:\n");
	printf("1. Try to reconnect\n");
	printf("2. Exit\n");
	gets_s(Meassage, sizeof(Meassage));
	if (STRINGS_ARE_EQUAL(Meassage, "1") || STRINGS_ARE_EQUAL(Meassage, "Try to reconnect") || STRINGS_ARE_EQUAL(Meassage, "1. Try to reconnect"))
	{
		return 1;
	}
	else if (STRINGS_ARE_EQUAL(Meassage, "2") || STRINGS_ARE_EQUAL(Meassage, "2. Exit") || STRINGS_ARE_EQUAL(Meassage, "Exit"))
	{
		return 2;
	}
	else
	{
		return 3;
	}
}

/*Client initial conntact with server. returns error code if fails and 1 for success*/
int ClientRequest(char *UserNameArg, char *IPArg, char *PortArg, SOCKET MainSocket)
{
	Msg_t *msg;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	int RetVal;
	char *AcceptedStr = NULL;
	char *SentStr = NULL;

	msg = (Msg_t*)malloc(sizeof(Msg_t));
	if (NULL == msg)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}
	/*AcceptedStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == AcceptedStr)
	{
		printf("Error in malloc, closing thread.\n");
		free(msg);
		return ERROR_RETURN;
	}*/
	SentStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == SentStr)
	{
		printf("Error in malloc, closing thread.\n");
		free(msg);
		free(AcceptedStr);
		return ERROR_RETURN;
	}

	strcpy(SentStr, "CLIENT_REQUEST:");
	strcat(SentStr, UserNameArg);
	strcat(SentStr, "\n");

	SendRes = SendString(SentStr, MainSocket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		free(msg);
		free(AcceptedStr);
		free(SentStr);
		return ERROR_RETURN;
	}

	
	RecvRes = ReceiveString(&AcceptedStr, MainSocket);
	//TODO add 15 sec wait
	if (RecvRes == TRNS_FAILED)
	{
		printf("Connection to server on %s:%s has been lost.", IPArg, PortArg);
		goto DealWithError;
	}
	else if (RecvRes == TRNS_DISCONNECTED)
	{
		printf("Connection to server on %s:%s has been lost.", IPArg, PortArg);
		goto DealWithError;
	}

	RetVal = ServerMsgDecode(AcceptedStr, msg);
	if (msg->MsgType == SERVER_DENIED)
	{
		printf("Server on %s:%s denied the connection request.", IPArg, PortArg);
		goto DealWithError;
	}
	if (msg->MsgType == SERVER_APPROVED)
	{
		free(msg);
		free(AcceptedStr);
		free(SentStr);
		return 1;
	}
	
	printf("Connection to server on %s:%s has been lost.", IPArg, PortArg);
	
DealWithError:
	free(msg);
	free(AcceptedStr);
	free(SentStr);
	return ERROR_RETURN;
}

int ClientRun(char *UserNameArg, SOCKET MainSocket)
{
	Msg_t *msg;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	int RetVal, MenuChoise=-1;
	char *AcceptedStr = NULL;
	char *SentStr = NULL;


	msg = (Msg_t*)malloc(sizeof(Msg_t));
	if (NULL == msg)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}
	/*AcceptedStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == AcceptedStr)
	{
		printf("Error in malloc, closing thread.\n");
		free(msg);
		return ERROR_RETURN;
	}*/
	SentStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == SentStr)
	{
		printf("Error in malloc, closing thread.\n");
		free(msg);
		free(AcceptedStr);
		return ERROR_RETURN;
	}
	while (TRUE)
		{
		RecvRes = ReceiveString(&AcceptedStr, MainSocket);
		//TODO add 15 sec wait
		if (RecvRes == TRNS_FAILED)
		{
			printf("Connection to server has been lost.\n");
			goto DealWithError;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection to server has been lost.\n");
			goto DealWithError;
		}
	
		RetVal = ServerMsgDecode(AcceptedStr, msg);
		if (msg->MsgType == SERVER_MAIN_MENU)
		{
			PrintMainMenu();
			scanf("%d", MenuChoise);
			switch (MenuChoise)
			{
				case(1):
					{
					/*VsHuman*/
					}
				case(2):
				{
					RetVal = ClientVsCPU(MainSocket);
					if (RetVal == ERROR_RETURN)
						goto DealWithError;
					else
						continue;
				}
				case(3):
				{
					/*leaderboard*/
				}
				case(4):
				{
					SendRes = SendString("EXIT_REQUEST\n", MainSocket);
					if (SendRes == TRNS_FAILED)
					{
						printf("Service socket error while writing, closing thread.\n");
						goto DealWithError;
					}
					free(msg);
					free(AcceptedStr);
					free(SentStr);
					return EXIT_REQUEST;
				}

			}
		}
	}


DealWithError:
	free(msg);
	free(AcceptedStr);
	free(SentStr);
	return ERROR_RETURN;
}


void PrintMainMenu(void)
{
	printf("Choose what to do next:\n");
	printf("1. Play against another client\n");
	printf("2. Play against the server\n");
	printf("3. View the leaderboard\n");
	printf("4. Quit\n");
}



int ClientVsCPU(SOCKET MainSocket)
{
	Msg_t *msg;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	int RetVal, MenuChoise = -1;
	char *AcceptedStr = NULL;
	char *SentStr = NULL;
	char *Move = NULL;

	msg = (Msg_t*)malloc(sizeof(Msg_t));
	if (NULL == msg)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}
	/*AcceptedStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == AcceptedStr)
	{
		printf("Error in malloc, closing thread.\n");
		free(msg);
		return ERROR_RETURN;
	}*/
	Move = (char*)malloc(MAX_MOVE_SIZE * sizeof(char));
	if (NULL == Move)
	{
		printf("Error in malloc, closing thread.\n");
		free(msg);
		free(AcceptedStr);
		return ERROR_RETURN;
	}
	SentStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == SentStr)
	{
		printf("Error in malloc, closing thread.\n");
		goto BadExit;
	}
	while (TRUE)
	{
		SendRes = SendString("CLIENT_CPU\n", MainSocket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			goto BadExit;
		}
		RecvRes = ReceiveString(&AcceptedStr, MainSocket);
		//TODO add 15 sec wait
		if (RecvRes == TRNS_FAILED)
		{
			printf("Connection to server has been lost.\n");
			goto BadExit;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection to server has been lost.\n");
			goto BadExit;
		}
		RetVal = ServerMsgDecode(AcceptedStr, msg);
		if (msg->MsgType != SERVER_PLAYER_MOVE_REQUEST || RetVal == ERROR_RETURN)
		{
			printf("Error in server.\n");
			goto BadExit;
		}
		MoveOptions(Move);
		strcpy(SentStr, "CLIENT_PLAYER_MOVE:");
		strcat(SentStr, Move);
		strcat(SentStr, "\n");
		SendRes = SendString(SentStr, MainSocket);
		if (SendRes == TRNS_FAILED)
		{
			printf("Service socket error while writing, closing thread.\n");
			goto BadExit;
		}
		RecvRes = ReceiveString(&AcceptedStr, MainSocket);
		//TODO add 15 sec wait
		if (RecvRes == TRNS_FAILED)
		{
			printf("Connection to server has been lost.\n");
			goto BadExit;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection to server has been lost.\n");
			goto BadExit;
		}
		RetVal = ServerMsgDecode(AcceptedStr, msg);
		if (msg->MsgType != SERVER_GAME_RESULTS || RetVal == ERROR_RETURN)
		{
			printf("Error in server.\n");
			goto BadExit;
		}
		printf("You played: %s\n %s played: %s\n %s won!", msg->MsgParams[2], msg->MsgParams[0], msg->MsgParams[1], msg->MsgParams[3]);
		
		RecvRes = ReceiveString(&AcceptedStr, MainSocket);
		//TODO add 15 sec wait
		if (RecvRes == TRNS_FAILED)
		{
			printf("Connection to server has been lost.\n");
			goto BadExit;
		}
		else if (RecvRes == TRNS_DISCONNECTED)
		{
			printf("Connection to server has been lost.\n");
			goto BadExit;
		}
		RetVal = ServerMsgDecode(AcceptedStr, msg);
		if (msg->MsgType != SERVER_GAME_OVER_MENU || RetVal == ERROR_RETURN)
		{
			printf("Error in server.\n");
			goto BadExit;
		}
		printf("Choose what to do next:\n 1. Play again\n 2. Return to the main menu");
		scanf("%d", MenuChoise);
		if (MenuChoise == 1)
		{
			SendRes = SendString("CLIENT_REPLAY\n", MainSocket);
			if (SendRes == TRNS_FAILED)
			{
				printf("Service socket error while writing, closing thread.\n");
				goto BadExit;
			}
			continue;
		}
		else if (MenuChoise = 2)
		{
			SendRes = SendString("CLIENT_MAIN_MENU\n", MainSocket);
			if (SendRes == TRNS_FAILED)
			{
				printf("Service socket error while writing, closing thread.\n");
				goto BadExit;
			}
			goto GoodExit;
		}
		else
		{
			goto BadExit;
		}
	}
GoodExit:
	free(msg);
	free(AcceptedStr);
	free(SentStr);
	free(Move);
	return 1;

BadExit:
	free(msg);
	free(AcceptedStr);
	free(SentStr);
	free(Move);
	return ERROR_RETURN;
}

/*gets an str, prints move options and appends option chosen to str*/
void MoveOptions(char *UpperMove)
{
	char move[10];;
	int i = 0, flag=0;
	while (flag == 0)
	{
		printf("Choose a move from the list: Rock, Paper, Scissors, Lizard or Spock:\n");
		scanf("%s", move);
		while (move[i] != '\0')
		{
			UpperMove[i] = toupper(move[i]);
			i++;
		}
		UpperMove[i] = '\n';
		if (STRINGS_ARE_EQUAL("SPOCK", UpperMove) || STRINGS_ARE_EQUAL("ROCK", UpperMove)
			|| STRINGS_ARE_EQUAL("PAPER", UpperMove) || STRINGS_ARE_EQUAL("SCISSORS", UpperMove) || STRINGS_ARE_EQUAL("LIZARD", UpperMove))
		{
			return;
		}
		flag = 1;
	}
}
