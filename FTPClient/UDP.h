#include "Common.h"

public class UDP
{
protected:
	char serverName[HOSTNAME_LENGTH];
	struct sockaddr_in server, client;
	SOCKET socketHandle;
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

	bool createAndBindSocketConnection(sockaddr_in *address)
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
		address->sin_port = htons(SERVER_PORT);

		//Bind
		if (bind(socketHandle, (struct sockaddr *)address, sizeof(*address)) == SOCKET_ERROR)
		{
			cout << "Bind failed with error code : " << WSAGetLastError();
			return false;
		}
	}

	bool SendData(Data data)
	{
		char buffer[BUFFER_LENGTH];
		memset(buffer, '\0', BUFFER_LENGTH);
		memcpy(buffer, &data, sizeof(data));

		int sent_bytes = sendto(socketHandle, (char *)buffer, sizeof(data), 0, (sockaddr*)&client, sizeof(sockaddr_in));

		if (sent_bytes != sizeof(data))
		{
			printf("failed sending data to client. Sent bytes %d\n", sent_bytes);
			return false;
		}
		//printf("Sent response. Message: %s\n", response.message);
		cout<<"Sent data. content: " <<data.content;
		return true;
	}

	Data RecieveData() {

		Data data;
		memset(&data, '\0', sizeof(data));

		int fromLength = sizeof(client);

		//try to receive some data, this is a blocking call
		if (recvfrom(socketHandle, (char *)&data, BUFFER_LENGTH, 0, (struct sockaddr *) &client, &fromLength) == SOCKET_ERROR)
		{
			printf("recvfrom() failed with error code : %d", WSAGetLastError());
			data.isLastPacket = true;
			return data;
		}
		return data;
	};
};