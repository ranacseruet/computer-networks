#include "UDPServer.h"
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

bool UDPServer::RecieveRequest(Request *req)
{
	int slen, recv_len;

	slen = sizeof(client);

	//keep listening for data
	printf("Waiting for Request...");
	//fflush(stdout);

	//try to receive some data, this is a blocking call
	if ((recv_len = recvfrom(s, (char *)req, BUFFER_LENGTH, 0, (struct sockaddr *) &client, &slen)) == SOCKET_ERROR)
	{
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		return false;
	}
	printf("recieved request. Filename: %s\n", req->filename);
	return true;
}

bool UDPServer::SendResponse(Response response)
{
	char buffer[BUFFER_LENGTH];
	memset(buffer, '\0', BUFFER_LENGTH);
	memcpy(buffer, &response, sizeof(response));

	int sent_bytes = sendto(s, (char *)buffer, sizeof(response), 0, (sockaddr*)&client, sizeof(sockaddr_in));

	if (sent_bytes != sizeof(response))
	{
		printf("failed sending response to client. Sent bytes %d\n", sent_bytes);
		return false;
	}
	printf("Sent response. Message: %s\n", response.message);

	return true;
}

/*int main(void)
{
	UDPServer* server = new UDPServer(0);
	server->run();
	int i;
	cin >> i;
	return 0;
}*/