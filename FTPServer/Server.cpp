/*************************************************************************************
*								 File Name	: Server.cpp		   			 	     *
*	Usage : Handles Client request for Uploading, downloading and listing of files   *
**************************************************************************************/
#define _CRT_SECURE_NO_WARNINGS
#include <winsock.h>
#include <iostream>
#include <windows.h>
#include <time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <cstdio>
#include <fstream>
#include <process.h>
//#include <mutex>
#include <direct.h>
#include "Thread.h"
#include "server.h"

using namespace std;

//HELPER FUNCTIONS----------------
std::wstring s2ws(const std::string& s)
{
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
//--------------------

char** getListOfFiles()
{
	char cCurrentPath[FILENAME_MAX], **filesList;
	struct _stat stat_buf;
	cout << "Current Directory: " << _getcwd(cCurrentPath, sizeof(cCurrentPath)) << "\n";

	HANDLE hFind;
	WIN32_FIND_DATA data;
	std::wstring stemp = s2ws(strcat(cCurrentPath, "\\data\\*"));
	LPCWSTR rootPath = stemp.c_str();
	hFind = FindFirstFile(rootPath, &data);
	if (hFind != INVALID_HANDLE_VALUE) {
		int i = 0;
		do {
			wprintf_s(data.cFileName);
			cout << '\n';
			//TODO copy file name in the fileList array
		} while (FindNextFile(hFind, &data));
		FindClose(hFind);
	}
	else {
		cout << "invalid handle value: " << hFind;
	}
	return filesList;
}

/**
* Constructor - TcpServer
* Usage: Initialize the socket connection
*
* @arg: void
*/
TcpServer::TcpServer()
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

	cout << "Server: " << serverName << " waiting to be contacted for get/put request..." << endl;

	/* Socket Creation */
	if ((serverSock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0)
	{
		std::cerr << "Socket Creation Error,exit" << endl;
		exit(1);
	}

	/* Fill-in Server Port and Address information */
	ServerPort = REQUEST_PORT;
	memset(&ServerAddr, 0, sizeof(ServerAddr));      /* Zero out structure */
	ServerAddr.sin_family = AF_INET;                 /* Internet address family */
	ServerAddr.sin_addr.s_addr = htonl(INADDR_ANY);  /* Any incoming interface */
	ServerAddr.sin_port = htons(ServerPort);         /* Local port */

	/* Binding the server socket to the Port Number */
	if (::bind(serverSock, (struct sockaddr *) &ServerAddr, sizeof(ServerAddr)) < 0)
	{
		cerr << "Socket Binding Error,exit" << endl;
		exit(1);
	}

	//Successfull bind, now listen for Server requests.
	if (listen(serverSock, MAXPENDING) < 0)
	{
		cerr << "Socket Listening Error,exit" << endl;
		exit(1);
	}
}

/**
* Destructor - ~TcpServer
* Usage: DeAllocate the allocated memory
*
* @arg: void
*/
TcpServer::~TcpServer()
{
	WSACleanup();
}

/**
* Function - start
* Usage: Listen and handle the requests from clients
*
* @arg: void
*/
void TcpServer::start()
{
	for (;;) /* Run forever */
	{
		/* Set the size of the result-value parameter */
		clientLen = sizeof(ServerAddr);

		/* Accept the connection from client */
		if ((clientSock = accept(serverSock, (struct sockaddr *) &ClientAddr,
			&clientLen)) < 0)
		{
			cerr << "Connection Accept Failed ,exit" << endl;
			exit(1);
		}

		/* Create a Thread for new connection and run*/
		TcpThread * pt = new TcpThread(clientSock);
		pt->run();
	}
}

/*-------------------------------TcpThread Class--------------------------------*/
/**
* Function - ResolveName
* Usage: Returns the binary, network byte ordered address
*
* @arg: char []
*/
unsigned long TcpThread::ResolveName(char name[])
{
	struct hostent *host;            /* Structure containing host information */

	if ((host = gethostbyname(name)) == NULL)
	{
		std::cerr << "gethostbyname() failed" << endl;
	}

	/* Return the binary, network byte ordered address */
	return *((unsigned long *)host->h_addr_list[0]);
}

/**
* Function - msgRecv
* Usage: Returns the length of bytes in msg_ptr->buffer,which have been recevied successfully
*
* @arg: int, Msg *
*/
int TcpThread::msgRecv(int sock, Msg * msg_ptr)
{
	int rbytes, n;

	/* Check the received Message Header */
	for (rbytes = 0; rbytes<MSGHDRSIZE; rbytes += n)
	{
		if ((n = recv(sock, (char *)msg_ptr + rbytes, MSGHDRSIZE - rbytes, 0)) <= 0)
		{
			std::cerr << "Received MSGHDR Error" << endl;
			return (-1);
		}
	}

	/* Check the received Message Buffer */
	for (rbytes = 0; rbytes<msg_ptr->length; rbytes += n)
	{
		if ((n = recv(sock, (char *)msg_ptr->buffer + rbytes, msg_ptr->length - rbytes, 0)) <= 0)
		{
			std::cerr << "Recevier Buffer Error" << endl;
			return (-1);
		}
	}
	/*  Return the length of bytes in msg_ptr->buffer,which have been recevied successfully */
	return msg_ptr->length;
}

/**
* Function - msgSend
* Usage: Returns the length of bytes in msg_ptr->buffer,which have been sent out successfully
*
* @arg: int, Msg *
*/
int TcpThread::msgSend(int sock, Msg * msg_ptr)
{
	int n;
	if ((n = send(sock, (char *)msg_ptr, MSGHDRSIZE + msg_ptr->length, 0)) != (MSGHDRSIZE + msg_ptr->length))
	{
		std::cerr << "Send MSGHDRSIZE+length Error " << endl;
		return(-1);
	}
	/* Return the length of bytes in msg_ptr->buffer,which have been sent out successfully */
	return (n - MSGHDRSIZE);
}

/**
* Function - sendFileData
* Usage: Transfer the requested file to client
*
* @arg: char[]
*/
void TcpThread::sendFileData(char fName[20])
{
	Msg sendMsg;
	Resp responseMsg;
	int numBytesSent = 0;
	ifstream fileToRead;
	int result;
	struct _stat stat_buf;
	/* Lock the code section */


	memset(responseMsg.response, 0, sizeof(responseMsg));
	/* Check the file status and pack the response */
	if ((result = _stat(fName, &stat_buf)) != 0)
	{
		strcpy(responseMsg.response, "No such file");
		memset(sendMsg.buffer, '\0', BUFFER_LENGTH);
		memcpy(sendMsg.buffer, &responseMsg, sizeof(responseMsg));
		/* Send the contents of file recursively */
		if ((numBytesSent = send(serverSocket, sendMsg.buffer, sizeof(responseMsg), 0)) == SOCKET_ERROR)
		{
			cout << "Socket Error occured while sending data " << endl;
			/* Close the connection and unlock the mutex if there is a Socket Error */
			closesocket(serverSocket);

			return;
		}
		else
		{
			/* Reset the buffer */
			memset(sendMsg.buffer, '\0', sizeof(sendMsg.buffer));
		}
	}
	else
	{
		fileToRead.open(fName, ios::in | ios::binary);
		if (fileToRead.is_open())
		{
			while (!fileToRead.eof())
			{
				memset(sendMsg.buffer, '\0', BUFFER_LENGTH);
				/* Read the contents of file and write into the buffer for transmission */
				fileToRead.read(sendMsg.buffer, BUFFER_LENGTH);
				/* Transfer the content to requested client */
				if ((numBytesSent = send(serverSocket, sendMsg.buffer, BUFFER_LENGTH, 0)) == SOCKET_ERROR)
				{
					cout << "Socket Error occured while sending data " << endl;
					/* Close the connection and unlock the mutex if there is a Socket Error */
					closesocket(serverSocket);

					return;
				}
				else
				{
					/* Reset the buffer and use the buffer for next transmission */
					memset(sendMsg.buffer, '\0', sizeof(sendMsg.buffer));
				}
			}
			cout << "File transferred completely... " << endl;
		}
		fileToRead.close();
	}
	/* Close the connection and unlock the Mutex after successful transfer */
	closesocket(serverSocket);
}

/**
* Function - run
* Usage: Based on the requested operation, invokes the appropriate function
*
* @arg: void
*/
void TcpThread::run()
{
	Req *requestPtr; //Pointer to the Request Packet
	Msg receiveMsg; //send_message and receive_message format
	/*Start receiving the request */
	if (msgRecv(serverSocket, &receiveMsg) != receiveMsg.length)
	{
		cerr << "Receive Req error,exit " << endl;
		return;
	}
	requestPtr = (Req *)receiveMsg.buffer;
	/* Check the type of operation and Construct the response and send to Client */
	if (receiveMsg.type == REQ_GET)
	{
		cout << "User " << requestPtr->hostname << " requested file " << requestPtr->filename << " to be sent" << endl;
		/* Transfer the requested file to Client */
		sendFileData(requestPtr->filename);
		getListOfFiles();
	}
	else if (receiveMsg.type == REQ_LIST)
	{
		cout << "User " << requestPtr->hostname << " requested for list of files to be sent" << endl;
		getListOfFiles();
	}

}

/**
* Function - main
* Usage: Initiates the Server
*
* @arg: void
*/
int main(void)
{
	TcpServer ts;
	/* Start the server and start listening to requests */
	ts.start();
	return 0;
}
