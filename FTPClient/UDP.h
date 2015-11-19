#include "Common.h"

//-----Only to be used by UDP class internally
#define PACKET_LENGTH 512
typedef struct
{
	Handshake handshake;
	char content[PACKET_LENGTH];
}UDPPacket;
//----- UDP packet specific settings


public class UDP
{
private:

	//Recieve a single UDP packet
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

	//Send single UDP packet
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

	//Send acknowledgement of recieved packet
	bool sendPacketRecieptACK(UDPPacket p, sockaddr_in *from)
	{
		//send acknowledgement
		UDPPacket ackPack;
		ackPack.handshake.seq = -1;
		ackPack.handshake.ack = p.handshake.seq;
		if (!sendUDPPacket(ackPack, from))
		{
			cout << "Couldn't send the acknowledgement!" << endl;
			return false;
		}
		else
		{
			//cout << "Sent packet reciept acknowledgement #" << ackPack.sequence << endl;
		}
		return true;
	}

	//recieve acknowledgement of sent packet
	bool recievePacketRecieptACK(UDPPacket p, sockaddr_in *to)
	{
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

			if (ackPack.handshake.ack == p.handshake.seq)
			{
				//success ack
				//cout << "Got acknowledgement for packet #" << p.sequence << endl;
				return true;
			}
			else
			{
				//recieved wrong data, try again
				cout << "Wrong acknoledgement for packet#"<<p.handshake.seq << endl;
				return false;
			}
		}
		else if (ret == 0)
		{
			//timed out
			cout << "ACK Timeout occured for packet #" << p.handshake.seq << endl;
			return false;
		}
		else
		{
			//select error
			cout << "Timer Error: "<<ret<<endl;
			return false;
		}
	}

	int sendingSequenceNo = 0;
	//Send UDP packet RELIABLY, reliable version of sendUDPPacket()
	bool sendUDPPacketReliably(UDPPacket p, sockaddr_in *to)
	{
		int attempt = MAX_TRIES;
		p.handshake.ack = -1;
		p.handshake.seq = sendingSequenceNo;
		sendingSequenceNo = ++sendingSequenceNo % ((2 * WINDOW_SIZE) + 1);
		do
		{
			if (attempt < MAX_TRIES)
			{
				cout << "!!!Resending Packet. Seq#" << p.handshake.seq << endl;
			}
			sendUDPPacket(p, to);
			if (recievePacketRecieptACK(p, to))
			{
				cout << ">>>Sent Packet. Seq#" << p.handshake.seq << endl;
				break;
			}
		} while (attempt-- > 0);
		return (attempt >0);
	}

	int recievedSequenceNo = -1;
	//Recieve UDP packet RELIABLY, reliable version of recieveUDPPacket()
	UDPPacket recieveUDPPacketReiably(sockaddr_in *from)
	{
		UDPPacket packet;
		while (true)
		{
			//cout << "<<<<<<< Recieving Packet "<< endl;
			memset(&packet, '\0', sizeof(UDPPacket));
			packet = recieveUDPPacket(from);
			sendPacketRecieptACK(packet, from);
			if (packet.handshake.seq != recievedSequenceNo)
			{
				recievedSequenceNo = packet.handshake.seq;
				cout << "<<<Recieved Packet " << packet.handshake.seq<< endl;
				break;
			}
			else
			{
				cout << "!!!Recieved Duplicate Packet " << packet.handshake.seq << ". Discarding!!!" << endl;
			}
		}
		return packet;
	}

protected:

	char serverName[HOSTNAME_LENGTH];
	struct sockaddr_in server, client;
	SOCKET socketHandle;

	bool sendAsPackets(char *buffer, int size, sockaddr_in *to)
	{
		UDPPacket packet;
		//window size 1
		//cout << "spliting as multiple packets and send" << endl;
		int numOfPacketsToToSend = (int)(size / PACKET_LENGTH);
		if (numOfPacketsToToSend <= 0) {
			numOfPacketsToToSend = 1;
		}
		int dataOffset = 0;
		for (int i = 0; i < numOfPacketsToToSend; i++)
		{
			memset(packet.content, '\0', PACKET_LENGTH);
			//window size 1
			int copySize = PACKET_LENGTH;
			if ((size - dataOffset) < PACKET_LENGTH)
			{
				copySize = (size - dataOffset);
			}
			memcpy(packet.content, buffer + dataOffset, copySize);
			//TODO check packet status, if false return error
			if (!sendUDPPacketReliably(packet, to))
			{
				cout << "reliable send of packet failed! exiting." << endl;
				return false;
			}
			dataOffset += copySize;
		}
	}
	
	void recieveAsPackets(char *buffer, int size, sockaddr_in *from)
	{
		UDPPacket packet;
		memset(buffer, '\0', size);
		//window size 1
		int numOfPacketsToRecieve = (int)(size / PACKET_LENGTH);
		if (numOfPacketsToRecieve <= 0) {
			numOfPacketsToRecieve = 1;
		}
		int bufferOffset = 0;
		//cout << "Waiting for Packets# " << numOfPacketsToRecieve << endl;
		for (int i = 0; i < numOfPacketsToRecieve; i++)
		{
			//recieve packets and add to data references
			memset(packet.content, '\0', PACKET_LENGTH);
			packet = recieveUDPPacketReiably(from);

			int recievedContentLength = PACKET_LENGTH;
			if ((size - bufferOffset) < PACKET_LENGTH)
			{
				recievedContentLength = (size - bufferOffset);
			}
			memcpy((void *)(buffer + bufferOffset), packet.content, recievedContentLength);
			bufferOffset += recievedContentLength;
			//cout << "<<<Recieved Packet " << packet.handshake.seq <<"Size: "<<recievedContentLength<< endl;
		}
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
		return sendAsPackets(buffer, sizeof(Data), to);
	}

	Data recieveData(sockaddr_in *from)
	{

		Data* data;
		char buffer[sizeof(Data)];
		recieveAsPackets(buffer, sizeof(Data), from);
		data = (Data *)buffer;

		return *data;
	};

public:
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
	virtual bool SendData(Data data);
	virtual Data RecieveData();
};