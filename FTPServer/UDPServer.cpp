#include "UDPServer.h"
#include <iostream>
#include <string>

#include <windows.h>

using namespace std;

UDPServer::UDPServer(Logger* log) :UDP()
{
	logger = log;
	createAndBindSocketConnection(&server);
}

UDPServer::~UDPServer()
{
	closesocket(socketHandle);
}

bool UDPServer::RecieveRequest(Request *req)
{
	int slen, recv_len;

	slen = sizeof(client);

	//keep listening for data
	printf("Waiting for Request...");
	//fflush(stdout);

	//try to receive some data, this is a blocking call
	if ((recv_len = recvfrom(socketHandle, (char *)req, BUFFER_LENGTH, 0, (struct sockaddr *) &client, &slen)) == SOCKET_ERROR)
	{
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		return false;
	}
	//char* type = itoa(req->type, type, 10);
	logger->Log("Recieved a request. Request Type: ");
	return true;
}

bool UDPServer::SendResponse(Response response)
{
	char buffer[BUFFER_LENGTH];
	memset(buffer, '\0', BUFFER_LENGTH);
	memcpy(buffer, &response, sizeof(response));

	int sent_bytes = sendto(socketHandle, (char *)buffer, sizeof(response), 0, (sockaddr*)&client, sizeof(sockaddr_in));

	if (sent_bytes != sizeof(response))
	{
		printf("failed sending response to client. Sent bytes %d\n", sent_bytes);
		return false;
	}
	//printf("Sent response. Message: %s\n", response.message);
	logger->Log("Sent response. Message: " + string(response.message));

	return true;
}