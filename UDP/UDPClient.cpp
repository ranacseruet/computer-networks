#include "Udp.h"
#include <windows.h>
#include <sys/types.h>


#include <iomanip>
#include <iostream>
#include <string>

unsigned long getAddressByHost()
{
	struct hostent *host;            /* Structure containing host information */

	if ((host = gethostbyname("valo-virus")) == NULL)
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



bool UDPClient::SendRequest(Request * req)
{
	int handle = socket(AF_INET, SOCK_DGRAM, 0);

	if (handle <= 0)
	{
		printf("failed to create socket\n");
		return false;
	}

	sockaddr_in address;
	int ServPort = 5001;
	memset(&address, 0, sizeof(address));     /* Zero out structure */
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = getAddressByHost();
	address.sin_port = htons(ServPort);
	



	if (bind(handle, (const sockaddr*)&address, sizeof(sockaddr_in)) < 0)
	{
		printf("failed to bind socket\n");
		return false;
	}

	int sent_bytes = sendto(handle, (const char*)req , sizeof(req),0,(sockaddr*)&address, sizeof(sockaddr_in));

	if (sent_bytes != sizeof(req))
	{
		printf("failed to send packet\n");
		return false;
	}



	return true;
};


bool UDPClient::SendDatat(Data data)
{
	return true;
};


void UDPClient::run()
{
	
	Request* req;
	req->type = 1;
	UDPClient::SendRequest(req);
		
	
}

int main(void)
{
	UDPClient* client = new UDPClient();
	client->run();
	return 0;
}