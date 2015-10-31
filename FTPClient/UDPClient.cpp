
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

	/*socketHandle = socket(AF_INET, SOCK_DGRAM, 0);

	if (socketHandle <= 0)
	{
		printf("failed to create socket\n");
	}

	//sockaddr_in address;
	int ServPort = 5001;
	memset(&address, 0, sizeof(address));    
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = getAddressByHost(serverName);//"MDALIRANAEDAF"
	address.sin_port = htons(ServPort);

	//sockaddr_in myaddress;
	myaddress.sin_family = AF_INET;
	myaddress.sin_addr.s_addr = INADDR_ANY;
	myaddress.sin_port = htons((unsigned short)0);


	if (bind(handle, (const sockaddr*)&myaddress, sizeof(sockaddr_in)) < 0)
	{
		printf("failed to bind socket\n");
	}*/
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
	
	int sent_bytes = sendto(socketHandle, (char *)buffer, sizeof(req), 0, (sockaddr*)&server, sizeof(sockaddr_in));

	if (sent_bytes != sizeof(req))
	{
		printf("failed to send packet. Sent bytes %d\n", sent_bytes);
		return false;
	}

	return true;
};

Response UDPClient::RecieveResponse() 
{
	Response ptr;
	int fromLength = sizeof(server);

	//try to receive some data, this is a blocking call
	if (recvfrom(socketHandle, (char *)&ptr, BUFFER_LENGTH, 0, (struct sockaddr *) &server, &fromLength) == SOCKET_ERROR)
	{
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		return ptr;
	}

	return ptr;
};

bool UDP::SendData(Data data)
{
	return sendData(data, &server);
}

Data UDP::RecieveData()
{
	return recieveData(&server);
}