
#include <sys/types.h>
#include <iomanip>
#include "UDPClient.h"

using namespace std;

unsigned long getAddressByHost(char hostName[50])
{
	struct hostent *host;            /* Structure containing host information */

	if ((host = gethostbyname(hostName)) == NULL)
	{
		return(1);
	}

	/* Return the binary, network byte ordered address */
	return *((unsigned long *)host->h_addr_list[0]);
}

UDPClient::UDPClient() :UDP()
{

}

UDPClient::~UDPClient()
{
	
}

void UDPClient::SetHostName(char host[])
{
	strcpy(serverName, host);
}

void UDPClient::CreateConnection() 
{
	createAndBindSocketConnection(&client, 5000);

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = getAddressByHost(serverName);
	server.sin_port = htons(7000);
}

void UDPClient::CloseConnection() 
{
	closesocket(socketHandle);
}

bool UDPClient::SendRequest(Request req)
{	
	char buffer[sizeof(Request)];
	memset(buffer, '\0', sizeof(Request));
	memcpy(buffer, &req, sizeof(Request));
	
	splitAndSendAsPackets(buffer, sizeof(Request), &server);
	cout << "Request " << req.type << " sent!" << endl;
	return true;
};

bool UDPClient::SendHandshakeRequest(Handshake req)
{
	char buffer[sizeof(Handshake)];
	memset(buffer, '\0', sizeof(Handshake));
	memcpy(buffer, &req, sizeof(req));

	splitAndSendAsPackets(buffer, sizeof(Handshake), &server);

	return true;
}

Response UDPClient::RecieveResponse() 
{
	Response *ptr;
	int fromLength = sizeof(server);

	char buffer[sizeof(Response)];
	recievePacketsToBuffer(buffer, sizeof(Response), &server);
	ptr = (Response *)buffer;
	cout << "Response type " << ptr->type << " recieved!" << endl;

	return *ptr;
};

Handshake UDPClient::RecieveHandshakeResponse()
{
	Handshake *ptr;
	int fromLength = sizeof(server);

	char buffer[sizeof(Handshake)];
	recievePacketsToBuffer(buffer, sizeof(Handshake), &server);
	ptr = (Handshake *)buffer;

	return *ptr;
}

bool UDP::SendData(Data data)
{
	return sendData(data, &server);
}

Data UDP::RecieveData()
{
	return recieveData(&server);
}