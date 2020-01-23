#include "ServerMsgUtils.h"
#include "ServerHeader.h" 
#include "SocketServer.h"



/*Main function of server*/
void MainServer(char *PortArg)
{
	int Ind, port;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;


	remove(FILE_PATH); // Just in case
	FirstPlayer = (char*)malloc(MAX_USERNAME * sizeof(char));
	if (NULL == FirstPlayer)
	{
		printf("Error in malloc, closing thread.\n");
		return ERROR_RETURN;
	}
	SecondPlayer = (char*)malloc(MAX_USERNAME * sizeof(char));
	if (NULL == SecondPlayer)
	{
		printf("Error in malloc, closing thread.\n");
		free(FirstPlayer);
		return ERROR_RETURN;
	}

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
	}

	GameFileMut = CreateMutex(
		NULL,	/* default security attributes */
		FALSE,	/* initially not owned */
		NULL);	/* unnamed mutex */
	if (NULL == GameFileMut)
	{
		printf("Error Generating Mutex.\n");
		goto server_cleanup_2;
	}
	/* The WinSock DLL is acceptable. Proceed. */

	// Create a socket.    
	MainSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MainSocket == INVALID_SOCKET)
	{
		printf("Error at socket( ): %ld\n", WSAGetLastError());
		goto server_cleanup_1;
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

	Address = inet_addr(SERVER_ADDRESS_STR);
	if (Address == INADDR_NONE)
	{
		printf("The string \"%s\" cannot be converted into an ip address. ending program.\n",
			SERVER_ADDRESS_STR);
		goto server_cleanup_2;
	}
	
	UserInGameMut = CreateMutex(
		NULL,	/* default security attributes */
		FALSE,	/* initially not owned */
		NULL);	/* unnamed mutex */
	if (NULL == UserInGameMut)
	{
		printf("Error Generating Mutex.\n");
		goto server_cleanup_2;
	}

	TwoHumansEvent = CreateEvent(
		NULL, /* default security attributes */
		TRUE,       /* manual-reset event */
		FALSE,      /* initial state is non-signaled */
		"OtherPlayerIsIn");         /* name */				/* named */

	if (TwoHumansEvent == NULL)
	{
		CloseHandle(UserInGameMut);
		printf("Error Generating Event.\n");
		goto server_cleanup_2;
	}

	SecondPlayerReady = CreateEvent(
		NULL, /* default security attributes */
		TRUE,       /* manual-reset event */
		FALSE,      /* initial state is non-signaled */
		"SecondPlayerReady");         /* name */				/* named */

	if (SecondPlayerReady == NULL)
	{
		CloseHandle(UserInGameMut);
		printf("Error Generating Event.\n");
		goto server_cleanup_2;
	}
	port = atoi(PortArg);
	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(port); //The htons function converts a u_short from host to TCP/IP network byte order 
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
	bindRes = bind(MainSocket, (SOCKADDR*)&service, sizeof(service));
	if (bindRes == SOCKET_ERROR)
	{
		printf("bind( ) failed with error %ld. Ending program\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Listen on the Socket.
	ListenRes = listen(MainSocket, SOMAXCONN);
	if (ListenRes == SOCKET_ERROR)
	{
		printf("Failed listening on socket, error %ld.\n", WSAGetLastError());
		goto server_cleanup_2;
	}

	// Initialize all thread handles to NULL
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;

	//Waiting for a client to connect
	while(TRUE)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}

		
		//Client Connected

		Ind = FindFirstUnusedThreadSlot();

		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			//No slots available for client, dropping the connection
			closesocket(AcceptSocket); //Closing the socket, dropping the connection.
		}
		else
		{
			ThreadInputs[Ind] = AcceptSocket; // shallow copy: don't close 
											  // AcceptSocket, instead close 
											  // ThreadInputs[Ind] when the
											  // time comes.
			ThreadHandles[Ind] = CreateThread(
				NULL,
				0,
				(LPTHREAD_START_ROUTINE)ServiceThread,
				&(ThreadInputs[Ind]),
				0,
				NULL
			);
		}
	} // for ( Loop = 0; Loop < MAX_LOOPS; Loop++ )

server_cleanup_3:
	
	CleanupWorkerThreads();

server_cleanup_2:
	CloseHandle(GameFileMut);
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

	
server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
	free(FirstPlayer);
	free(SecondPlayer);
	return 0;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

static int FindFirstUnusedThreadSlot()
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] == NULL)
			break;
		else
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], 0);

			if (Res == WAIT_OBJECT_0) // this thread finished running
			{
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
		}
	}

	return Ind;
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

void CleanupWorkerThreads(void)
{
	int Ind;

	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
	{
		if (ThreadHandles[Ind] != NULL)
		{
			// poll to check if thread finished running:
			DWORD Res = WaitForSingleObject(ThreadHandles[Ind], INFINITE);

			if (Res == WAIT_OBJECT_0)
			{
				closesocket(ThreadInputs[Ind]);
				CloseHandle(ThreadHandles[Ind]);
				ThreadHandles[Ind] = NULL;
				break;
			}
			else
			{
				printf("Waiting for thread failed. Ending program\n");
				return;
			}
		}
	}
}

/*oOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoOoO*/

//Service thread is the thread that opens for each successful client connection and "talks" to the client.
static DWORD ServiceThread(SOCKET *t_socket)
{
	while (TRUE)
	{
		Msg_t *msg=NULL;
		char UserName[20];
		TransferResult_t SendRes;
		TransferResult_t RecvRes;
		int RetVal;
		char *AcceptedStr = NULL;
	
		msg = (Msg_t*)malloc(sizeof(Msg_t));
		if (NULL == msg)
		{
			printf("Error in malloc, closing thread.\n");
			goto DealWithError1;
		}
		while (TRUE)
		{
			AcceptedStr = NULL;
			RecvRes = ReceiveString(&AcceptedStr, *t_socket);
			if (RecvRes == TRNS_FAILED)
			{
				printf("Service socket error while reading, closing thread.\n");
				goto DealWithError3;
			}
			else if (RecvRes == TRNS_DISCONNECTED)
			{
				printf("Connection closed while reading, closing thread.\n");
				goto DealWithError3;
			}
			RetVal = ClientMsgDecode(AcceptedStr, msg);
			if (RetVal == ERROR_RETURN)
			{
				printf("Wrong meassege from client, closing thread.\n");
				goto DealWithError3;
			}
			if (msg->MsgType != CLIENT_REQUEST)
			{
				printf("Wrong meassege from client, closing thread.\n");
				goto DealWithError3;
			}
			if (RetVal == 1)
			{
				SendRes = SendString("SERVER_APPROVED\n", *t_socket);
				if (SendRes == TRNS_FAILED)
				{
					printf("Service socket error while writing, closing thread.\n");
					goto DealWithError1;
				}
				break;
			}
		}
		

		strcpy(UserName, msg->MsgParams[0]);
		printf("%s connected\n", UserName);
		RetVal = MainMenu(msg, t_socket,UserName);
		if (RetVal == ERROR_RETURN)
		{
			goto DealWithError3;
		}
	

		/*Dealing with errors*/

	DealWithError3:
		free(AcceptedStr);
	DealWithError2:
		free(msg);
	DealWithError1:
		closesocket(*t_socket);
		printf("%s disconnected\n", UserName);
		return 0;
	}
}

/*Gets player moves and reurns player number of the winner, 0 for tie*/
int WhoWon(int Player1, int Player2)
{
	if (Player1 == Player2)
	{
		return 0;
	}
	switch (Player1)
	{
	case(SPOCK):
	{
		if (Player2 == PAPER || Player2 == LIZARD)
			return 2;
		return 1;
	}
	case(ROCK):
	{
		if (Player2 == PAPER || Player2 == SPOCK)
			return 2;
		return 1;
	}
	case(PAPER):
	{
		if (Player2 == SCISSORS || Player2 == LIZARD)
			return 2;
		return 1;
	}
	case(SCISSORS):
	{
		if (Player2 == SPOCK || Player2 == ROCK)
			return 2;
		return 1;
	}
	case(LIZARD):
	{
		if (Player2 == SCISSORS || Player2 == ROCK)
			return 2;
		return 1;
	}
	return ERROR_RETURN;
	}
}

static DWORD ServerKillerThread(void)
{
	char Kill[5];
	while (TRUE)
	{
		scanf(Kill, "%s");
		if (STRINGS_ARE_EQUAL(Kill, "quit"))
		{
			return KILL_REQUEST;
		}
	}
}