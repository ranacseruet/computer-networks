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

	char buffer[sizeof(Request)];
	recieveAsPacketsSR(buffer, sizeof(Request), &client);
	Request *request = (Request *)buffer;
	if (request->type != REQ_PUT)
	{
		//put request will start sending data immediately after
		//TODO what if an acknowledgement lost while put request itself???
		recieveExtraPackets(&client);
	}	
	return *request;
}

bool UDPServer::SendResponse(Response response)
{
	char buffer[sizeof(Response)];
	memset(buffer, '\0', sizeof(Response));
	memcpy(buffer, &response, sizeof(Response));

	sendAsPacketsSR(buffer, sizeof(Response), &client);
	logger->Log("Sent response.\n");

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

Handshake UDPServer::recieveHandshakeRequest()
{
	int slen, recv_len;

	slen = sizeof(client);
	char buffer[sizeof(Handshake)];
	recieveAsPacketsSR(buffer, sizeof(Handshake), &client);
	Handshake *hsReq = (Handshake *)buffer;
	
	char logMessage[100] = { '\0' };
	sprintf(logMessage, "Recieved handshake request. Seq No# %d\n", hsReq->seq);
	logger->Log(logMessage);
	recieveExtraPackets(&client);
	return *hsReq;
}

bool UDPServer::sendHandshakeResponse(Handshake hs)
{
	char buffer[sizeof(Handshake)];
	memset(buffer, '\0', sizeof(Handshake));
	memcpy(buffer, &hs, sizeof(Handshake));

	sendAsPacketsSR(buffer, sizeof(Handshake), &client);
	
	char logMessage[100] = { '\0' };
	sprintf(logMessage, "Sent handshake response. Ack# %d and Own Sequence# %d \n", hs.ack, hs.seq);
	logger->Log(logMessage);

	return true;
}