#include "Server.h"
#include <iostream>
#include <string>

#include <windows.h>

#define BUFLEN 100

using namespace std;

UDPServer::UDPServer(int port)
{

	WSADATA wsadata;
	/* Initialize Windows Socket information */
	if (WSAStartup(0x0202, &wsadata) != 0)
	{
		cerr << "Starting WSAStartup() error\n" << endl;
		exit(1);
	}

	/* Display the name of the host */
	if (gethostname(serverName, HOSTNAME_LENGTH) != 0)
	{
		cerr << "Get the host name error,exit" << endl;
		exit(1);
	}

	/* Socket Creation */
	if ((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		cerr << "Socket Creation Error,exit" << endl;
		exit(1);
	}

	/* Fill-in Server Port and Address information */
	//Can be removed if proper port provided
	port = SERVER_PORT;
	
	
	memset(&serverAddr, 0, sizeof(serverAddr));      /* Zero out structure */
	serverAddr.sin_family = AF_INET;                 /* Internet address family */
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
	serverAddr.sin_port = htons(port);				/* Local port */

	/* Binding the server socket to the Port Number */
	if (::bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
	{
		cerr << "Socket Binding Error,exit" << endl;
		exit(1);
	}

	//Successfull bind, now listen for Server requests.
	if (listen(serverSocket, MAXPENDING) < 0)
	{
		cerr << "Socket Listening Error,exit" << endl;
		exit(1);
	}

	cout << "Server: " << serverName << " listening on port " << port << " ..." << endl;
}

void UDPServer::run()
{
	//for (;;) /* Run forever */
	{
		//TODO
		Request* req = RecieveRequest();
		cout << req->type <<endl;
	}
}

UDPServer::~UDPServer()
{
	WSACleanup();
}

Request* UDPServer::RecieveRequest()
{
	Request* req;

	SOCKET s;
	struct sockaddr_in server, si_other;
	int slen, recv_len;
	char buf[BUFLEN];
	WSADATA wsa;

	slen = sizeof(si_other);

	//Initialise winsock
	printf("\nInitialising Winsock...");
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
	{
		printf("Failed. Error Code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	printf("Initialised.\n");

	//Create a socket
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
	{
		printf("Could not create socket : %d", WSAGetLastError());
	}
	printf("Socket created.\n");

	//Prepare the sockaddr_in structure
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;
	server.sin_port = htons(5001);

	//Bind
	if (bind(s, (struct sockaddr *)&server, sizeof(server)) == SOCKET_ERROR)
	{
		printf("Bind failed with error code : %d", WSAGetLastError());
		exit(EXIT_FAILURE);
	}
	puts("Bind done");

	//keep listening for data
	while (1)
	{
		printf("Waiting for data...");
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		memset(buf, '\0', BUFLEN);

		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, buf, BUFLEN, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			break;
		}

		//print details of the client/peer and the data received
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Data: %s\n", buf);

		//now reply the client with the same data
		if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
		{
			printf("sendto() failed with error code : %d", WSAGetLastError());
			break;
		}
	}

	closesocket(s);
	WSACleanup();
	return req;
}

/*int main(void)
{
	UDPServer* server = new UDPServer(0);
	server->run();
	int i;
	cin >> i;
	return 0;
}*/