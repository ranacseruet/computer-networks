#include "Common.h"

public class UDP
{
protected:
	//----- Making these protected as outside of derived classes shouldn't know about this
#define PACKET_LENGTH 190
	typedef enum
	{
		PACKET_DATA=1,
		PACKET_ACK=2,
		PACKET_DUP=3
	} PacketType;

	typedef struct
	{
		Handshake hs;
		char content[PACKET_LENGTH];
		int sequence;
		PacketType type;
		bool retrying;
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
			return p;
		}
		return p;
	}

	bool sendPacketRecieptACK(UDPPacket p, sockaddr_in *from)
	{
		if (p.type != PACKET_ACK)
		{
			//get acknowledgement
			UDPPacket ackPack;
			ackPack.type = PACKET_ACK;
			ackPack.retrying = false;
			ackPack.sequence = p.sequence + 1;
			if (!sendUDPPacket(ackPack, from))
			{
				cout << "Couldn't send the acknowledgement!" << endl;
				false;
			}
			else
			{
				cout << "Sent packet reciept acknowledgement" << endl;
			}
		}
		return true;
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

	bool recievePacketRecieptACK(UDPPacket p, sockaddr_in *to)
	{
		if (p.type == PACKET_ACK || p.retrying == true)
		{
			//no need to retry this type of packet
			return true;
		}

		struct timeval tv;
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		fd_set readfds;
		FD_ZERO(&readfds);
		FD_SET(socketHandle, &readfds);

		int ret = select(socketHandle + 1, &readfds, NULL, NULL, &tv);

		if (ret > 0)
		{
			//has data to read
			//get acknowledgement
			UDPPacket ackPack = recieveUDPPacket(to);

			if (ackPack.type == PACKET_ACK && ackPack.sequence == p.sequence + 1)
			{
				//success ack
				//cout << "Got and successfull acknowledgement!" << endl;
				return true;
			}
			else
			{
				//recieved wrong data, try again
				cout << "Wrong acknoledgement." << endl;
				return false;
			}
		}
		else if (ret == 0)
		{
			//timed out
			cout << "Timeout occured. Resending packet." << endl;
			return false;
		}
		else
		{
			//select error
			cout << "Timer Error: "<<ret<<endl;
			return false;
		}
	}


	bool sendUDPPacketReliably(UDPPacket p, sockaddr_in *to)
	{
		int attempt = 5;
		do
		{
			if (attempt < 5)
			{
				cout << "resending packet#"<<p.sequence << endl;
			}
			sendUDPPacket(p, to);
			attempt--;
			if (attempt < 0)
			{
				cout << "Max attempt failed. Give up" <<endl;
				break;
			}
		} while (!recievePacketRecieptACK(p, to));		
		return !(attempt < 0);
	}

	UDPPacket recieveUDPPacketReiably(int oldSeq, sockaddr_in *from)
	{
		UDPPacket packet;
		//discard duplicate packets in loop
		bool duplicate = false;
		do
		{
			if (duplicate)
			{
				cout << "recieved duplicate packet" << endl;
			}
			packet = recieveUDPPacket(from);
			duplicate = true;
		} while (packet.sequence == oldSeq);

		sendPacketRecieptACK(packet, from);
		return packet;
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
		packet.type = PACKET_DATA;
		packet.retrying = false;

		if (PACKET_LENGTH >= size)
		{
			packet.sequence = 0;
			//can be sent in one single packet
			memset(packet.content, '\0', PACKET_LENGTH);
			memcpy(packet.content, buffer, size);
			//TODO check packet status, if false return error
			if (!sendUDPPacketReliably(packet, to))
			{
				cout << "reliable send of packet failed" << endl;
			}
			cout << "Sent single Packet " << ". Seq#" << packet.sequence << endl;
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
				//TODO check packet status, if false return error
				if (!sendUDPPacketReliably(packet, to))
				{
					cout << "reliable send of packet failed" << endl;
				}
				cout << "Sent Packet " << i << ". Seq#" << packet.sequence << endl;
				//printBinaryBuffer(packet.content, copySize);
				dataOffset += copySize;
			}
		}
	}
	
	void recievePacketsToBuffer(char *buffer, int size, sockaddr_in *from)
	{
		UDPPacket packet;
		memset(buffer, '\0', size);
		int oldSeq = -1;
		if (PACKET_LENGTH >= size)
		{
			//can be recieved as one single packet
			//discarding duplicate packets
			packet = recieveUDPPacketReiably(oldSeq, from);
			cout << "Recieved packet with seq# " << packet.sequence << endl;
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
				packet = recieveUDPPacketReiably(oldSeq, from);

				oldSeq = packet.sequence;

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