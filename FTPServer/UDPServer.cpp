#include "Server.h"
#include <iostream>
#include <string>

#include <windows.h>

using namespace std;

UDPServer::UDPServer(int port)
{
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
}

void UDPServer::run()
{
	//for (;;) /* Run forever */
	{
		//TODO
		//Request* req = RecieveRequest();
		//cout << req->type <<endl;
	}
}

UDPServer::~UDPServer()
{
	closesocket(s);
	WSACleanup();
}

void UDPServer::RecieveRequest(Request *req)
{
	int slen, recv_len;

	slen = sizeof(si_other);

	//keep listening for data
	while (1)
	{
		printf("Waiting for data... header size: %d", sizeof(req));
		fflush(stdout);

		//clear the buffer by filling null, it might have previously received data
		char buffer[BUFFER_LENGTH];
		memset(buffer, '\0', BUFFER_LENGTH);
		//try to receive some data, this is a blocking call
		if ((recv_len = recvfrom(s, (char *)req, BUFFER_LENGTH, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			break;
		}
		
		printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
		printf("Filename: %s\n", req->filename);
		printf("Request type: %d\n",req->type);
	}
}

/*int main(void)
{
	UDPServer* server = new UDPServer(0);
	server->run();
	int i;
	cin >> i;
	return 0;
}*/