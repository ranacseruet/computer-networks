#include "FTPServer.h"
#include <iostream>

using namespace std;

FTPServer::FTPServer(UDPServer* udp, FileHelper* helper, Logger* log)
{
	udpServer = udp;
	fileHelper = helper;
	logger = log;
}

void FTPServer::run()
{
	while (1)
	{
		try
		{
			Request request;

			char logMessage[100] = { '\0' };

			char serverName[100] = { '\0' };
			if (gethostname(serverName, HOSTNAME_LENGTH) != 0)
			{
				cerr << "Get the host name error,exit" << endl;
			}
			sprintf(logMessage, "Host %s Waiting For Requests...\n", serverName);
			logger->Log(logMessage);

			Handshake hs;
			hs = udpServer->recieveHandshakeRequest();
			hs.ack = hs.seq + 1;
			hs.seq = (rand() % 100) + 10;
			udpServer->sendHandshakeResponse(hs);

			request = udpServer->RecieveRequest();

			if (request.handshake.ack != hs.seq + 1)
			{
				memset(logMessage, '\0', sizeof(logMessage));
				sprintf(logMessage, "Handshake acknowledgement didn't match\n");
				logger->Log(logMessage);
				Response response;
				response.isSuccess = false;
				memset(response.message, '\0', sizeof(response.message));
				sprintf(response.message, "Handshake acknowledgement didn't match\n");
				udpServer->SendResponse(response);
				continue;
			}

			memset(logMessage, '\0', sizeof(logMessage));
			sprintf(logMessage, "Got request. Type: %d\n", request.type);
			logger->Log(logMessage);

			switch (request.type)
			{
			case REQ_LIST:
				list(request);
				break;
			case REQ_GET:
				get(request);
				break;
			case REQ_PUT:
				put(request);
				break;
			case REQ_RENAME:
				rename(request);
				break;
			case REQ_DELETE:
				del(request);
				break;
			default:
				break;
			}
		}
		catch (...)
		{
			cout << "^^^Execption occured^^^" << endl;
		}
	}
}

void FTPServer::list(Request request)
{
	Response response;
	response.type = REQ_LIST;
	memset(&response, '\0', sizeof(response));
	string filesList = fileHelper->GetListOfFiles();
	if (strlen(filesList.c_str()) <= 0)
	{
		strcpy(response.message, "No files exist in server");
		response.isSuccess = false;
	}
	else 
	{
		strcpy(response.message, filesList.c_str());
		response.isSuccess = true;
	}
	udpServer->SendResponse(response);
	logger->Log("Responsed to LIST operation\n");
}

void FTPServer::get(Request request)
{
	//1. Send success/failure response
	Response response;
	memset(&response, '\0', sizeof(response));
	response.type = REQ_GET;

	if (!fileHelper->DoesFileExist(request.filename))
	{
		strcpy(response.message, "File doesn't exist");
		response.isSuccess = false;
	}
	else
	{
		strcpy(response.message, "File found. Server is going to send data");
		response.isSuccess = true;
	}
	udpServer->SendResponse(response);
	
	if (!response.isSuccess)
	{
		//no data to send
		return;
	}

	//send data
	char dataStream[DATA_LENGTH];
	memset(dataStream, '\0', sizeof(dataStream));
	Data data;
	long pos = 0;
	while (1)
	{
		int numOfBytesRead = 0;
		memset(dataStream, '\0', DATA_LENGTH);
		memset(data.content, '\0', DATA_LENGTH);
		bool lastPacket = !fileHelper->ReadFile(request.filename, pos, dataStream, &numOfBytesRead);
		memcpy(data.content, dataStream, numOfBytesRead);
		data.isLastPacket = lastPacket;
		data.length = numOfBytesRead;
		udpServer->SendData(data);
		
		char logMessage[100] = { '\0' };
		//sprintf(logMessage, "Read And Sent %d Bytes!\n", numOfBytesRead);
		//logger->Log(logMessage);

		if (lastPacket)
		{
			break;
		}
		pos += numOfBytesRead;
	}
	char logMessage[100] = { '\0' };
	sprintf(logMessage, "File sending complete. Total Bytes: %d\n", pos);
	logger->Log(logMessage);
}

void FTPServer::put(Request request)
{
	//1. Send success/failure response
	Response response;
	memset(&response, '\0', sizeof(response));
	response.type = REQ_PUT;

	if (fileHelper->DoesFileExist(request.filename))
	{
		fileHelper->DeleteFile(request.filename);
	}
	
	strcpy(response.message, "Server is ready to recieve data");
	response.isSuccess = true;
	
	udpServer->SendResponse(response);

	if (!response.isSuccess)
	{
		//no data to send
		return;
	}

	//recieve data
	while (true)
	{
		Data data;
		data = udpServer->RecieveData();
		fileHelper->WriteFile(request.filename, data.content, data.length);
		//cout << "File data recieved:" << strlen(data.content) << " bytes" << endl;

		if (data.isLastPacket)
		{
			cout << "This was last packet" << endl;
			break;
		}
	}
	logger->Log("File data recieving complete\n");
}

void FTPServer::del(Request request)
{
	Response response;
	memset(&response, '\0', sizeof(response));
	response.type = REQ_DELETE;


	if (fileHelper->DoesFileExist(request.filename))
	{
		if (fileHelper->DeleteFile(request.filename))
		{
			response.isSuccess = true;
			strcpy(response.message, "File successfully deleted");
		}
		else
		{
			strcpy(response.message, "File exist, but couldn't be deleted");
			response.isSuccess = false;
		}
	}
	else
	{
		strcpy(response.message, "File does not exist");
		response.isSuccess = false;
	}

	udpServer->SendResponse(response);

	char logMessage[100] = { '\0' };
	sprintf(logMessage, "File successfully deleted : %s\n", request.filename);
	logger->Log(logMessage);
}

void FTPServer::rename(Request request)
{
	Response response;
	memset(&response, '\0', sizeof(response));
	response.type = REQ_RENAME;

	if (fileHelper->DoesFileExist(request.filename))
	{
		if (fileHelper->RenameFile(request.filename, request.renamedFileName))
		{
			response.isSuccess = true;
			strcpy(response.message, "File successfully renamed");
		}
		else
		{
			strcpy(response.message, "File exist, but couldn't be renamed");
			response.isSuccess = false;
		}
	}
	else
	{
		strcpy(response.message, "File does not exist");
		response.isSuccess = false;
	}

	udpServer->SendResponse(response);
	
	char logMessage[100] = { '\0' };
	sprintf(logMessage, "File Renaming Complete\n");
	logger->Log(logMessage);
}

int main(void)
{
	Request req;
	FileHelper *helper = new FileHelper("\\server_data\\");
	
	char logFilePath[FILENAME_LENGTH+100] = { '\0' };
	_getcwd(logFilePath, sizeof(logFilePath));
	helper->buildFullFilePath(logFilePath, "server_log.txt");

	Logger *logger = new Logger(logFilePath);
	UDPServer *udpServer = new UDPServer(logger);
	FTPServer *ftpServer = new FTPServer(udpServer, helper, logger);
	ftpServer->run();

	/*Data data;
	long pos = 0;
	
	data = udpServer->RecieveData();
	cout << data.content;*/

	int i;
	cin >> i;
	return 0;
}