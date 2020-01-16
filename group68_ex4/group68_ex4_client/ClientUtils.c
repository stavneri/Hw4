#include "SocketClient.h"
#include "ClientHeader.h"
#include "ClientMsgUtils.h"


int ClientMain(char *IPArg, char *PortArg, char *UserNameArg)
{
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN ClientService;
	int RetTemp = -1;
	
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
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (MainSocket == INVALID_SOCKET)
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

	ClientService.sin_family = AF_INET;
	ClientService.sin_addr.s_addr = Address;
	ClientService.sin_port = htons(PortArg); //The htons function converts a u_short from host to TCP/IP network byte order 
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
	if (connect(MainSocket, (SOCKADDR*)&ClientService, sizeof(ClientService)) == SOCKET_ERROR) {
		printf("Failed connecting to server on %s:%s", IPArg, PortArg);
		WSACleanup();
		return CONNECTION_FAIL;
	}
	Printf("Connected to Server on %s:%s", IPArg, PortArg);
	RetTemp = ClientRequest(UserNameArg, IPArg, PortArg, MainSocket);
	if (RetTemp != 1)
	{
		WSACleanup();
		return RetTemp;
	}
	while (RetTemp == 1)
	{
		RetTemp = ClientRun(UserNameArg, MainSocket);
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
int ClientRequest(char *UserNameArg, char *IPArg, char *PortArg, SOCKET *MainSocket)
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
	AcceptedStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == AcceptedStr)
	{
		printf("Error in malloc, closing thread.\n");
		free(msg);
		return ERROR_RETURN;
	}
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
	SendRes = SendString(&SentStr, MainSocket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		free(msg);
		free(AcceptedStr);
		free(SentStr);
		return ERROR_RETURN;
	}

	
	RetVal = ReceiveString(&AcceptedStr, MainSocket);
	//TODO add 15 sec wait
	if (RetVal == TRNS_FAILED)
	{
		printf("Connection to server on %s:%s has been lost.", IPArg, PortArg);
		goto DealWithError;
	}
	else if (RetVal == TRNS_DISCONNECTED)
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

ClientRun(char *UserNameArg, SOCKET MainSocket)
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
	AcceptedStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == AcceptedStr)
	{
		printf("Error in malloc, closing thread.\n");
		free(msg);
		return ERROR_RETURN;
	}
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
		RetVal = ReceiveString(&AcceptedStr, MainSocket);
		//TODO add 15 sec wait
		if (RetVal == TRNS_FAILED)
		{
			printf("Connection to server has been lost.\n");
			goto DealWithError;
		}
		else if (RetVal == TRNS_DISCONNECTED)
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
					/*VsCPU*/
				}
				case(3):
				{
					/*leaderboard*/
				}
				case(4):
				{
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