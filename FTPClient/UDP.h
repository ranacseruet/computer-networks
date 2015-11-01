#include "Common.h"

public class UDP
{
protected:
	//----- Making these protected as outside of derived classes shouldn't know about this
#define PACKET_LENGTH 190
	typedef struct
	{
		Handshake hs;
		char content[PACKET_LENGTH];
		bool isLast;
		int sequence;
	}UDPPacket;
	//----- UDP packet specific settings
private:
	void printBinaryBuffer(char buffer[], int length)
	{
		for (int i = 0; i < length; i++)
		{
			cout << buffer[i];
		}
		cout << endl;
	}

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
protected:

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
		//split data into multiple packets and send
		char buffer[sizeof(Data)];
		memset(buffer, '\0', sizeof(Data));
		memcpy(buffer, &data, sizeof(Data));
		splitAndSendAsPackets(buffer, sizeof(Data), to);

		return true;
	}

	Data recieveData(sockaddr_in *from) 
	{

		Data* data;	
		char buffer[sizeof(Data)];
		recievePacketsToBuffer(buffer, sizeof(Data), from);
		data = (Data *)buffer;

		return *data;
	};

	void splitAndSendAsPackets(char *buffer, int size, sockaddr_in *to)
	{
		UDPPacket packet;

		if (PACKET_LENGTH >= size)
		{
			//can be sent in one single packet
			memset(packet.content, '\0', PACKET_LENGTH);
			memcpy(packet.content, buffer, size);
			sendUDPPacket(packet, to);
		}
		else
		{
			int numOfPacketsToToSend = (int)(size / PACKET_LENGTH) + 1;
			int dataOffset = 0;
			for (int i = 0; i < numOfPacketsToToSend; i++)
			{
				packet.sequence = i;
				memset(packet.content, '\0', PACKET_LENGTH);

				int copySize = PACKET_LENGTH;
				if ((size - dataOffset) < PACKET_LENGTH)
				{
					copySize = (size - dataOffset);
				}
				memcpy(packet.content, buffer + dataOffset, copySize);
				//copyBinaryBuffer(packet.content, buffer + dataOffset, 0, copySize);
				sendUDPPacket(packet, to);
				//cout << "Sent Packet " << i << ". Size: " << copySize << endl;
				//printBinaryBuffer(packet.content, copySize);
				dataOffset += copySize;
			}
		}
	}
	
	void recievePacketsToBuffer(char *buffer, int size, sockaddr_in *from)
	{
		UDPPacket packet;
		memset(buffer, '\0', size);
		if (PACKET_LENGTH >= size)
		{
			//can be recieved as one single packet
			packet = recieveUDPPacket(from);
			memcpy(buffer, packet.content, size);
		}
		else
		{
			int numOfPacketsToRecieve = (int)(size / PACKET_LENGTH) + 1;
			int bufferOffset = 0;
			//cout << "Waiting for Packets# " << numOfPacketsToRecieve << endl;
			for (int i = 0; i < numOfPacketsToRecieve; i++)
			{
				//recieve packets and add to data references
				memset(packet.content, '\0', PACKET_LENGTH);
				packet = recieveUDPPacket(from);
				int copySize = PACKET_LENGTH;
				if ((size - bufferOffset) < PACKET_LENGTH)
				{
					copySize = (size - bufferOffset);
				}
				memcpy((void *)(buffer + bufferOffset), packet.content, copySize);
				bufferOffset += copySize;
				//cout << "Recieved Packet " << packet.sequence <<"Size: "<<copySize<< endl;
				//printBinaryBuffer(packet.content, PACKET_LENGTH);
			}
		}
	}

public:
	virtual bool SendData(Data data);
	virtual Data RecieveData();
};