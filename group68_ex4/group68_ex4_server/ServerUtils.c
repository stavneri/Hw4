#include "ServerHeader.h" 
#include "SocketSendRecvTools.h"


/*Main function of server*/
void MainServer(char *PortArg)
{
	int Ind;
	int Loop;
	SOCKET MainSocket = INVALID_SOCKET;
	unsigned long Address;
	SOCKADDR_IN service;
	int bindRes;
	int ListenRes;

	// Initialize Winsock.
	WSADATA wsaData;
	int StartupRes = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (StartupRes != NO_ERROR)
	{
		printf("error %ld at WSAStartup( ), ending program.\n", WSAGetLastError());
		// Tell the user that we could not find a usable WinSock DLL.                                  
		return;
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

	service.sin_family = AF_INET;
	service.sin_addr.s_addr = Address;
	service.sin_port = htons(PortArg); //The htons function converts a u_short from host to TCP/IP network byte order 
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

	// Initialize all thread handles to NULL, to mark that they have not been initialized
	for (Ind = 0; Ind < NUM_OF_WORKER_THREADS; Ind++)
		ThreadHandles[Ind] = NULL;

	printf("Waiting for a client to connect...\n"); //TODO - Erase

	for (Loop = 0; Loop < MAX_LOOPS; Loop++)
	{
		SOCKET AcceptSocket = accept(MainSocket, NULL, NULL);
		if (AcceptSocket == INVALID_SOCKET)
		{
			printf("Accepting connection with client failed, error %ld\n", WSAGetLastError());
			goto server_cleanup_3;
		}

		printf("Client Connected.\n"); //TODO - Erase

		Ind = FindFirstUnusedThreadSlot();

		if (Ind == NUM_OF_WORKER_THREADS) //no slot is available
		{
			printf("No slots available for client, dropping the connection.\n");  //TODO - Erase
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
	if (closesocket(MainSocket) == SOCKET_ERROR)
		printf("Failed to close MainSocket, error %ld. Ending program\n", WSAGetLastError());

server_cleanup_1:
	if (WSACleanup() == SOCKET_ERROR)
		printf("Failed to close Winsocket, error %ld. Ending program.\n", WSAGetLastError());
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

static void CleanupWorkerThreads()
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
	char SendStr[SEND_STR_SIZE];

	BOOL Done = FALSE;
	TransferResult_t SendRes;
	TransferResult_t RecvRes;
	int MsgType;
	char *AcceptedStr = NULL;
	char MsgParams = NULL;

	MsgParams = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == MsgParams)
	{
		printf("Error in malloc, closing thread.\n");
		closesocket(*t_socket);
		return ERROR_RETURN;
	}
	AcceptedStr = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (NULL == AcceptedStr)
	{
		printf("Error in malloc, closing thread.\n");
		closesocket(*t_socket);
		free(MsgParams);
		return ERROR_RETURN;
	}

	RecvRes = ReceiveString(&AcceptedStr, *t_socket);
	if (RecvRes == TRNS_FAILED)
	{
		printf("Service socket error while reading, closing thread.\n");
		closesocket(*t_socket);
		free(MsgParams); 
		free(AcceptedStr);
		return ERROR_RETURN;
	}
	else if (RecvRes == TRNS_DISCONNECTED)
	{
		printf("Connection closed while reading, closing thread.\n");
		closesocket(*t_socket);
		free(MsgParams);
		free(AcceptedStr);
		return ERROR_RETURN;
	}
	
	MsgType = ClientMsgDecode(AcceptedStr, MsgParams);
	if (MsgType != CLIENT_REQUEST)
	{
		printf("Wrong meassege from client, closing thread.\n");
		closesocket(*t_socket);
		free(MsgParams);
		free(AcceptedStr);
		return ERROR_RETURN;
	}

	SendRes = SendString("SERVER_MAIN_MENU", *t_socket);
	if (SendRes == TRNS_FAILED)
	{
		printf("Service socket error while writing, closing thread.\n");
		closesocket(*t_socket);
		free(MsgParams);
		free(AcceptedStr);
		return ERROR_RETURN;
	}

	/*From main menu*/
	RecvRes = ReceiveString(&AcceptedStr, *t_socket);
	if (RecvRes == TRNS_FAILED)
	{
		printf("Service socket error while reading, closing thread.\n");
		closesocket(*t_socket);
		free(MsgParams);
		free(AcceptedStr);
		return ERROR_RETURN;
	}
	else if (RecvRes == TRNS_DISCONNECTED)
	{
		printf("Connection closed while reading, closing thread.\n");
		closesocket(*t_socket);
		free(MsgParams);
		free(AcceptedStr);
		return ERROR_RETURN;
	}
	
	MsgType = ClientMsgDecode(AcceptedStr, MsgParams);
	if (MsgType == CLIENT_CPU) //TODO
	{
		VsCPU()
	}

	if (MsgType == CLIENT_VERSUS) //TODO
	{

	}
	if (MsgType == CLIENT_LEADERBOARD) //TODO
	{

	}
	if (MsgType == CLIENT_DISCONNECT) //TODO
	{

	}
	/****************************************************************************************/
	/*Till here*/
	/***************************************************************************************/







	free(AcceptedStr);

	printf("Conversation ended.\n");
	closesocket(*t_socket);
	return 0;
}



/*TODO write description and debug*/
int ClientMsgDecode(char *msg, char* MsgParams)
{
	strcpy(MsgParams, ""); //Clears old invalid contect in MsgParams
	int i, j = 0;
	char MsgType[25];

	for (i = 0; msg[i] != ':'; i++)
	{
		MsgType[i] = msg[i];
	}
	MsgType[i] = '\0';

	
	while (msg[i] != '\n')
	{
		MsgParams[j] = msg[i];
	}
	MsgParams[i] = '\0';

	if (STRINGS_ARE_EQUAL("CLIENT_REQUEST", MsgType))
	{
		return CLIENT_REQUEST;
	}
	if (STRINGS_ARE_EQUAL("CLIENT_MAIN_MENU", MsgType))
	{
		return CLIENT_MAIN_MENU;
	}
	if (STRINGS_ARE_EQUAL("CLIENT_CPU", MsgType))
	{
		return CLIENT_CPU;
	}
	if (STRINGS_ARE_EQUAL("CLIENT_VERSUS", MsgType))
	{
		return CLIENT_VERSUS;
	}
	if (STRINGS_ARE_EQUAL("CLIENT_LEADERBOARD", MsgType))
	{
		return CLIENT_LEADERBOARD;
	}
	if (STRINGS_ARE_EQUAL("CLIENT_PLAYER_MOVE", MsgType))
	{
		return CLIENT_PLAYER_MOVE;
	}
	if (STRINGS_ARE_EQUAL("CLIENT_REPLAY", MsgType))
	{
		return CLIENT_REPLAY;
	}
	if (STRINGS_ARE_EQUAL("CLIENT_REFRESH", MsgType))
	{
		return CLIENT_REFRESH;
	}
	if (STRINGS_ARE_EQUAL("CLIENT_DISCONNECT", MsgType))
	{
		return CLIENT_DISCONNECT;
	}
	return ERROR_TYPE;
}

VsCPU()