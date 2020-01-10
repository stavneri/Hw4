#include "ClientHeader.h" 
#include "SocketClientSendRecvTools.h"

SOCKET m_socket;

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
	if (connect(m_socket, (SOCKADDR*)&ClientService, sizeof(ClientService)) == SOCKET_ERROR) {
		printf("Failed connecting to server on %s : %s", IPArg, PortArg);
		WSACleanup();
		return CONNECTION_FAIL;
	}
	
	RetTemp = ClientRequest(UserNameArg, IPArg, PortArg);
	if (RetTemp == CONNECTION_FAIL || RetTemp == CONNECTION_TIMEOUT)
	{
		WSACleanup();
		return RetTemp;
	}

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

int ClientRequest(char *UserNameArg, char *IPArg, char *PortArg)
{
	TransferResult_t Res;
	char *Meassge = NULL;
	Meassge = (char*)malloc(MAX_MSG_SIZE * sizeof(char));
	if (Meassge == NULL)
	{
		printf("Connection to server on %s : %s has been lost.", IPArg, PortArg);
		return CONNECTION_TIMEOUT;
	}
	strcpy(Meassge, "CLIENT_REQUEST:");
	strcat(Meassge, UserNameArg);
	Res = SendString(Meassge, m_socket);
	if (Res == TRNS_FAILED)
	{
		printf("Connection to server on %s : %s has been lost.", IPArg, PortArg);
		return CONNECTION_TIMEOUT;
	}
	strcpy(Meassge, "");
	Res = ReceiveString(&Meassge, m_socket);
	//TODO add 15 sec wait
	if (RecvRes == TRNS_FAILED)
	{
		printf("Connection to server on %s : %s has been lost.", IPArg, PortArg);
		return CONNECTION_TIMEOUT;
	}
	else if (RecvRes == TRNS_DISCONNECTED)
	{
		printf("Connection to server on %s : %s has been lost.", IPArg, PortArg);
		return CONNECTION_TIMEOUT;
	}
	if STRINGS_ARE_EQUAL(Meassge, "SERVER_DENIED")
	{
		printf("Server on %s : %s denied the connection request.", IPArg, PortArg);
		return CONNECTION_FAIL;
	}
	if STRINGS_ARE_EQUAL(Meassge, "SERVER_APPROVED")
	{
		return 0;
	}
	printf("Connection to server on %s : %s has been lost.", IPArg, PortArg);
	return CONNECTION_TIMEOUT;
}