#include "Udp.h"
#include <iostream>
#include <string>

#include <windows.h>


using namespace std;

UDPServer::UDPServer(int port)
{

	WSADATA wsadata;
	/* Initialize Windows Socket information */
	if (WSAStartup(0x0202, &wsadata) != 0)
	{
		cerr << "Starting WSAStartup() error\n" << endl;
		exit(1);
	}

	/* Display the name of the host */
	if (gethostname(serverName, HOSTNAME_LENGTH) != 0)
	{
		cerr << "Get the host name error,exit" << endl;
		exit(1);
	}

	/* Socket Creation */
	if ((serverSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		cerr << "Socket Creation Error,exit" << endl;
		exit(1);
	}

	/* Fill-in Server Port and Address information */
	//Can be removed if proper port provided
	port = SERVER_PORT;
	
	
	memset(&serverAddr, 0, sizeof(serverAddr));      /* Zero out structure */
	serverAddr.sin_family = AF_INET;                 /* Internet address family */
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
	serverAddr.sin_port = htons(port);				/* Local port */

	/* Binding the server socket to the Port Number */
	if (::bind(serverSocket, (struct sockaddr *) &serverAddr, sizeof(serverAddr)) < 0)
	{
		cerr << "Socket Binding Error,exit" << endl;
		exit(1);
	}

	//Successfull bind, now listen for Server requests.
	if (listen(serverSocket, MAXPENDING) < 0)
	{
		cerr << "Socket Listening Error,exit" << endl;
		exit(1);
	}

	cout << "Server: " << serverName << " waiting to be contacted for requests..." << endl;
}

void UDPServer::run()
{
	//for (;;) /* Run forever */
	{
		//TODO
		Request* req = RecieveRequest();
		cout << req->type <<endl;
	}
}

UDPServer::~UDPServer()
{
	WSACleanup();
}

Request* UDPServer::RecieveRequest()
{
	while (true)
	{
		unsigned char packet_data[256];

		memset(packet_data, '\0', sizeof(packet_data));

		unsigned int max_packet_size =
			sizeof(packet_data);

		#if PLATFORM == PLATFORM_WINDOWS
				typedef int socklen_t;
		#endif

		sockaddr_in from;
		socklen_t fromLength = sizeof(from);

		int bytes = recvfrom(serverSocket,
			(char*)packet_data,
			max_packet_size,
			0,
			(sockaddr*)&from,
			&fromLength);

		unsigned int from_address =
			ntohl(from.sin_addr.s_addr);

		unsigned int from_port =
			ntohs(from.sin_port);
		if (packet_data[0] == NULL)
		{
			cout << "null data recieved" << endl;
		}
		else
		{
			Request* req;
			req = (Request *)packet_data;
			return req;
		}
	}
}

int main(void)
{
	UDPServer* server = new UDPServer(0);
	server->run();
	int i;
	cin >> i;
	return 0;
}