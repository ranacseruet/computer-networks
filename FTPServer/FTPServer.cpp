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
		Request request;
		bool success = udpServer->RecieveRequest(&request);
		if (!success)
		{
			//error occured
			break;
		}

		switch (request.type)
		{
		case REQ_LIST:
			list(request);
			break;
		case REQ_GET:
			get(request);
			break;
		default:
			break;
		}
	}
}

void FTPServer::list(Request request)
{
	Response response;
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
	logger->Log("Responsed to LIST operation");
}

void FTPServer::get(Request request)
{
	//1. Send success/failure response
	Response response;
	memset(&response, '\0', sizeof(response));
	if (!fileHelper->DoesFileExist(request.filename))
	{
		strcpy(response.message, "File doesn't exist");
		response.isSuccess = false;
	}
	else
	{
		strcpy(response.message, "File exist");
		response.isSuccess = true;
	}
	udpServer->SendResponse(response);
	
	if (!response.isSuccess)
	{
		//no data to send
		return;
	}

	//send data
	char dataStream[RESP_LENGTH];
	memset(dataStream, '\0', sizeof(dataStream));
	Data data;
	long pos = 0;
	while (1)
	{
		bool success = fileHelper->ReadFile(request.filename, pos, dataStream);
		strcpy(data.content, dataStream);
		data.isLastPacket = !success;
		udpServer->SendData(data);
		cout << data.content;
		if (!success)
		{
			cout << "File read completed: " << strlen(dataStream) << " bytes" << endl;
			break;
		}
		else
		{
			cout << "File read:"<< strlen(dataStream) <<" bytes" << endl;
		}
		pos += strlen(dataStream);
	}
	logger->Log("\nFile data sending complete");
}

int main(void)
{
	Request req;
	Logger *logger = new Logger("data\\server_log.txt");
	UDPServer *server = new UDPServer(logger);
	FileHelper *helper = new FileHelper("\\data\\");
	FTPServer *ftpServer = new FTPServer(server, helper, logger);
	ftpServer->run();

	int i;
	cin >> i;
	return 0;
}