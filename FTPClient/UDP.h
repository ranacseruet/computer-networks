#include "Common.h"

public class UDP
{
protected:

//----- Making these protected as outside of derived classes shouldn't know about this
#define PACKET_LENGTH 180
	typedef struct
	{
		Handshake hs;
		char content[PACKET_LENGTH];
		bool isLast;
	}UDPPacket;
//----- UDP packet specific settings


	char serverName[HOSTNAME_LENGTH];
	struct sockaddr_in server, client;
	SOCKET socketHandle;

	UDP(void)
	{
		WSADATA wsa;
		if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		{
			cout << "Failed. Error Code : " << WSAGetLastError();
			exit(EXIT_FAILURE);
		}
	}

	~UDP(void)
	{
		WSACleanup();
	}

	bool createAndBindSocketConnection(sockaddr_in *address, int port)
	{
		//Create a socket
		if ((socketHandle = socket(AF_INET, SOCK_DGRAM, 0)) == INVALID_SOCKET)
		{
			cout << "Could not create socket. Error code : " << WSAGetLastError();
			return false;
		}

		//Prepare the sockaddr_in structure
		address->sin_family = AF_INET;
		address->sin_addr.s_addr = INADDR_ANY;
		address->sin_port = htons(port);

		//Bind
		if (bind(socketHandle, (struct sockaddr *)address, sizeof(*address)) == SOCKET_ERROR)
		{
			cout << "Bind failed with error code : " << WSAGetLastError();
			return false;
		}
	}

	bool sendData(Data data, sockaddr_in *to)
	{
		UDPPacket packet;
		memset(packet.content, '\0', PACKET_LENGTH);
		memcpy(packet.content, &data, sizeof(data));

		sendUDPPacket(packet, to);

		return true;
	}

	Data recieveData(sockaddr_in *from) 
	{

		Data* data;
		UDPPacket packet = recieveUDPPacket(from);
		data = (Data *)packet.content;
		return *data;
	};

	UDPPacket recieveUDPPacket(sockaddr_in *from)
	{
		UDPPacket p;

		memset(&p, '\0', sizeof(p));

		int fromLength = sizeof(sockaddr_in);

		//try to receive some data, this is a blocking call
		if (recvfrom(socketHandle, (char *)&p, sizeof(UDPPacket), 0, (struct sockaddr *)from, &fromLength) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			p.isLast = true;
			return p;
		}

		return p;
	}

	bool sendUDPPacket(UDPPacket p, sockaddr_in *to)
	{
		char buffer[sizeof(UDPPacket)];
		memset(buffer, '\0', sizeof(UDPPacket));
		memcpy(buffer, &p, sizeof(p));

		int sent_bytes = sendto(socketHandle, (char *)buffer, sizeof(UDPPacket), 0, (sockaddr*)to, sizeof(sockaddr_in));

		if (sent_bytes != sizeof(p))
		{
			printf("failed sending data to. Sent bytes %d\n", sent_bytes);
			return false;
		}
		return true;
	}

public:
	virtual bool SendData(Data data);
	virtual Data RecieveData();
};