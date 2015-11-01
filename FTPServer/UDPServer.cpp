#include "UDPServer.h"
#include <iostream>
#include <string>

#include <windows.h>

using namespace std;

UDPServer::UDPServer(Logger* log) :UDP()
{
	logger = log;
	createAndBindSocketConnection(&server, SERVER_PORT);
}

UDPServer::~UDPServer()
{
	closesocket(socketHandle);
}

Request UDPServer::RecieveRequest()
{
	int slen, recv_len;

	slen = sizeof(client);

	//keep listening for data
	printf("Waiting for Request...");
	//fflush(stdout);

	char buffer[sizeof(Request)];
	recievePacketsToBuffer(buffer, sizeof(Request), &client);
	Request *request = (Request *)buffer;
	return *request;
}

bool UDPServer::SendResponse(Response response)
{
	char buffer[BUFFER_LENGTH];
	memset(buffer, '\0', BUFFER_LENGTH);
	memcpy(buffer, &response, sizeof(response));

	splitAndSendAsPackets(buffer, sizeof(Response), &client);
	//printf("Sent response. Message: %s\n", response.message);
	logger->Log("Sent response. Message: " + string(response.message));

	return true;
}

bool UDP::SendData(Data data)
{
	return sendData(data, &client);
}

Data UDP::RecieveData()
{
	return recieveData(&client);
}