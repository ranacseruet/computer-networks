#include <windows.h>
#include <sys/types.h>
#include <iomanip>
#include <iostream>
#include <string>
#include "Client.h"

unsigned long getAddressByHost()
{
	struct hostent *host;            /* Structure containing host information */

	if ((host = gethostbyname("MDALIRANAEDAF")) == NULL)
	{
		return(1);
	}

	/* Return the binary, network byte ordered address */
	return *((unsigned long *)host->h_addr_list[0]);
}

UDPClient::UDPClient()
{
	WSADATA WsaData;
	WSAStartup(MAKEWORD(2, 2), &WsaData);
}

UDPClient::~UDPClient()
{
	WSACleanup();
}

void UDPClient::CreateConnection() 
{
	handle = socket(AF_INET, SOCK_DGRAM, 0);

	if (handle <= 0)
	{
		printf("failed to create socket\n");
	}

	//sockaddr_in address;
	int ServPort = 5001;
	memset(&address, 0, sizeof(address));     /* Zero out structure */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = getAddressByHost();
	address.sin_port = htons(ServPort);

	//sockaddr_in myaddress;
	myaddress.sin_family = AF_INET;
	myaddress.sin_addr.s_addr = INADDR_ANY;
	myaddress.sin_port = htons((unsigned short)0);


	if (bind(handle, (const sockaddr*)&myaddress, sizeof(sockaddr_in)) < 0)
	{
		printf("failed to bind socket\n");
	}
}



bool UDPClient::SendRequest(Request req)
{	
	CreateConnection();
	char buffer[BUFFER_LENGTH];
	Container container;
	memset(container.buffer, '\0', BUFFER_LENGTH);
	memcpy(container.buffer, &req, sizeof(req));

	printf("Size: %d", sizeof(req));
	memset(buffer, '\0', BUFFER_LENGTH);
	memcpy(buffer, &req, sizeof(req));
	printf("%s", buffer);
	printf(req.filename);

	printf("Size: %d" , sizeof(buffer));
	int sent_bytes = sendto(handle, (char *)buffer, sizeof(req), 0, (sockaddr*)&address, sizeof(sockaddr_in));

	if (sent_bytes != sizeof(req))
	{
		printf("failed to send packet. Sent bytes %d\n", sent_bytes);
		return false;
	}

	return true;
};

Response UDPClient::RecieveResponse() 
{
	int numBytesRecv;
	Response ptr;
	char buffer[BUFFER_LENGTH];
	int fromLength = sizeof(address);

	//try to receive some data, this is a blocking call
	if (recvfrom(handle, (char *)&ptr, BUFFER_LENGTH, 0, (struct sockaddr *) &address, &fromLength) == SOCKET_ERROR)
	{
		printf("recvfrom() failed with error code : %d", WSAGetLastError());
		return ptr;
	}

	closesocket(handle);
	return ptr;
};


bool UDPClient::SendDatat(Data data)
{
	return true;
};


void UDPClient::run()
{
	
	Request req;
	memset(&req, '\0', sizeof(req));
	//req.type = 1;
	UDPClient::SendRequest(req);
		
	
}

/*int main(void)
{
	UDPClient* client = new UDPClient();
	client->run();
	return 0;
}*/