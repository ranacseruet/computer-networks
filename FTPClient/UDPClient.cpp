
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
	createAndBindSocketConnection(&client, 0);

	memset(&server, 0, sizeof(server));
	server.sin_family = AF_INET;
	server.sin_addr.s_addr = getAddressByHost(serverName);
	server.sin_port = htons(SERVER_PORT);
}

void UDPClient::CloseConnection() 
{
	closesocket(socketHandle);
}

bool UDPClient::SendRequest(Request req)
{	
	char buffer[BUFFER_LENGTH];
	memset(buffer, '\0', BUFFER_LENGTH);
	memcpy(buffer, &req, sizeof(req));
	
	splitAndSendAsPackets(buffer, sizeof(Request), &server);

	return true;
};

Response UDPClient::RecieveResponse() 
{
	Response *ptr;
	int fromLength = sizeof(server);

	char buffer[sizeof(Response)];
	recievePacketsToBuffer(buffer, sizeof(Response), &server);
	ptr = (Response *)buffer;

	return *ptr;
};

bool UDP::SendData(Data data)
{
	return sendData(data, &server);
}

Data UDP::RecieveData()
{
	return recieveData(&server);
}